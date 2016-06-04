/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*=============================================================================
	SerialMIDIDevice.cpp
	
=============================================================================*/

#include "SerialMIDIDevice.h"
#include "SerialMIDIDriverBase.h"
#include <fcntl.h>
#include <sys/ioctl.h>


#if DEBUG
	//#define DUMP_INPUT 1
	//#define DUMP_OUTPUT 1
	//#define VERBOSE 1
#endif

// _________________________________________________________________________________________
//	Dump
//
#if DUMP_INPUT || DUMP_OUTPUT
static void Dump(const char *label, const Byte *buffer, ByteCount len)
{
	printf("%-5.5s: ", label);
	ByteCount i = 0;
	while (true) {
		printf("%02X ", buffer[i]);
		if (++i >= len) break;
		if (i % 32 == 0)
			printf("\n        ");
	}
	printf("\n");
}
#endif

// _________________________________________________________________________________________
//	SerialMIDIDevice::SerialMIDIDevice
//
SerialMIDIDevice::SerialMIDIDevice(	SerialMIDIDriverBase *	driver,
									SerialDevice *			serialDevice,
									MIDIDeviceRef			midiDevice) :
	MIDIDriverDevice(midiDevice),
	mDriver(driver),
	mSerialDevice(serialDevice),
	mWriteQueueGuard("SerialMIDIDevice.mWriteQueueGuard"),
	mOutputRunningStatus(0),
	mLastRunningStatusClearedTime(0),
	mInputParser(this)
{
	mDeathPillPipe[0] = mDeathPillPipe[1] = -1;
}

// _________________________________________________________________________________________
//	SerialMIDIDevice::Initialize
//
bool	SerialMIDIDevice::Initialize()
{
	SInt32			clockMode, nports; 
	struct 			termios	options;
	UInt32			receiveLatency = 650;	// microseconds (was: 1000)
	int				status;
	bool			isAppleKext = !strcmp(mSerialDevice->GetBSDDevice(), "/dev/cu.serial");

	// the property is the clock divisor, 1 or 32 (default)
	if (MIDIObjectGetIntegerProperty(mMIDIDevice, kSerDevProperty_ExtClockDivide, &clockMode) == noErr && clockMode == 1)
		clockMode = isAppleKext ? k1XClock_ForAppleKext : BEXT1;
	else
		clockMode = isAppleKext ? k32XClock_ForAppleKext : BEXT32;

	if (MIDIObjectGetIntegerProperty(mMIDIDevice, kSerDevProperty_NumberF5Ports, &nports) == noErr && nports > 1)
		mUseF5 = true;
	else
		mUseF5 = false;
	mOutputPort = 0xFF;

	mWriteBuf.Allocate(this, 128);
	mReadBuf.Allocate(this, 128);

	mPortFD = open(mSerialDevice->GetBSDDevice(), O_RDWR | O_NOCTTY | O_NDELAY);
	require(mPortFD != -1, errexit);

	// Restore read blocking O_NDELAY
	require_noerr(fcntl(mPortFD, F_SETFL, 0), errexit);  
	
	// Save current port options for later restore 
	require_noerr(tcgetattr(mPortFD, &mSavedTTYAttrs), errexit); 			    
	
	// Configure serial port options
	options = mSavedTTYAttrs;

	options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);	// This line must be before the cflag |= line
	options.c_cflag |= (CS8 | CLOCAL);	
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;
	/* man termios
	DOESN'T WORK
	Case C: MIN = 0, TIME > 0
     In this case, since MIN = 0, TIME no longer represents an inter-byte
     timer.  It now serves as a read timer that is activated as soon as the
     read function is processed.  A read is satisfied as soon as a single byte
     is received or the read timer expires.  Note that in this case if the
     timer expires, no bytes are returned.  If the timer does not expire, the
     only way the read can be satisfied is if a byte is received.  In this
     case the read will not block indefinitely waiting for a byte; if no byte
     is received within TIME*0.1 seconds after the read is initiated, the read
     returns a value of zero, having read no data.  If data is in the buffer
     at the time of the read, the timer is started as if data had been
     received immediately after the read.
	*/
//	options.c_cc[ VMIN ] = 1;			
//	options.c_cc[ VTIME ] = 0;

	options.c_ispeed = clockMode;
	options.c_ospeed = clockMode;
	verify_noerr(cfsetispeed(&options, clockMode));
	verify_noerr(cfsetospeed(&options, clockMode));
	
	// Set port attributes
	require_noerr(tcsetattr(mPortFD, TCSANOW, &options), errexit);
	
	require_noerr(pipe(mDeathPillPipe), errexit);

	status = ioctl(mPortFD, IOSSDATALAT, &receiveLatency);	// Sets the receive latency (in microseconds) 

	SetUpEndpoints(true);
	mStopRequested = false;

	mWriteThread = new WriteThread(this);
	mWriteThread->Start();
	while (!mWriteThread->IsRunning())
		usleep(100);

	mReadThread = new ReadThread(this);
	mReadThread->Start();
	
	if (mUseF5)
		StartOrStopF5(true);
	
	mDriver->StartInterface(this);

	// I/O is now running.  Do driver-specific initialization.
	// Here, the driver can do things like send MIDI to the interface to
	// configure it.
	return true;
	
errexit:
	return false;
}

// _________________________________________________________________________________________
//	SerialMIDIDevice::~SerialMIDIDevice
//
SerialMIDIDevice::~SerialMIDIDevice()
{
	char X;
	
	SetUpEndpoints(false);

	if (mUseF5)
		StartOrStopF5(false);

	mDriver->StopInterface(this);

	mStopRequested = true;
	
	int status = tcsetattr(mPortFD, TCSANOW, &mSavedTTYAttrs);
#if DEBUG
	if ((status != -1) && (status !=0))			
		// Can return -1 when no device found and port already reset
		printf("Error resetting port attributes while closing driver\n");
#endif
	close(mPortFD); 
	
	// wake write thread up so that it will notice stop has been requested, and then exit
	mWriteQueueGuard.Notify(); // unblock write queue thread

	// unblock read thread
	X = 'X';
	verify(write(mDeathPillPipe[1], &X, 1) == 1);
	
	if (mReadThread)
		mReadThread->StopAndDelete();

	if (mWriteThread)
		mWriteThread->StopAndDelete();
	
	if (mDeathPillPipe[0] != -1)
		close(mDeathPillPipe[0]);
	if (mDeathPillPipe[1] != -1)
		close(mDeathPillPipe[1]);
}

// __________________________________________________________________________________________________

void	SerialMIDIDevice::StartOrStopF5(bool start)
{
	{
		CAMutex::Locker lock(mWriteQueueGuard);
		MIDIPacket pkt;
		WriteQueueElem wqe;
		
		pkt.timeStamp = 0;
		pkt.length = 0;
		
		wqe.packet.Create(&pkt);
		wqe.portNum = start ? 0x7F : 0x00;
		wqe.bytesSent = 0;
		mWriteQueue.push_back(wqe);
	}
	mWriteQueueGuard.Notify();
}

// __________________________________________________________________________________________________

void	SerialMIDIDevice::RunReadThread()
{
	fd_set fdset;
	int bad = mDeathPillPipe[0];
	int nfds = std::max(mPortFD, bad) + 1;

	FD_ZERO(&fdset);

	while (true) {
		FD_SET(mPortFD, &fdset);
		FD_SET(bad, &fdset);
		verify(select(nfds, &fdset, NULL, NULL, NULL) > 0);
		
		if (mStopRequested || FD_ISSET(bad, &fdset))
			break;
		
		if (FD_ISSET(mPortFD, &fdset)) {			
			ssize_t bytesReceived = read(mPortFD, mReadBuf, sizeof(mReadBuf));
			if (bytesReceived == -1 || mStopRequested)
				break;
			if (bytesReceived > 0)
				HandleInput(mReadBuf, bytesReceived);
		}
	}
}

// __________________________________________________________________________________________________

void	SerialMIDIDevice::RunWriteThread()
{
	ssize_t	numBytes;	// Number of bytes read or written
	mWriteQueueGuard.Lock();
			
	while (true) {
		// wait for the signal that there's something to send
		mWriteQueueGuard.Wait();

		if (mStopRequested)
			break;
		
		MIDITimeStamp now = CAHostTimeBase::GetCurrentTime();
		if (CAHostTimeBase::ConvertToNanos(now - mLastRunningStatusClearedTime) > 100000000) { // 100 ms
			mLastRunningStatusClearedTime = now;
			mOutputRunningStatus = 0;
		}
		
		// now that we're awake, keep running until we've exhausted the queue
		while (mWriteQueue.begin() != mWriteQueue.end()) {
			ByteCount msglen = PrepareOutput(mWriteQueue, mWriteBuf, mWriteBuf.Size());
			if (msglen > 0) {
				mWriteQueueGuard.Unlock();
#if DUMP_OUTPUT
				Dump("OUT", mWriteBuf, msglen);
#endif
				numBytes = write(mPortFD, mWriteBuf, msglen);
				mWriteQueueGuard.Lock();
			}
		}
		if (mStopRequested)
			break;
	}
	mWriteQueueGuard.Unlock();
}


// _________________________________________________________________________________________
//	SerialMIDIDevice::HandleInput
//
void	SerialMIDIDevice::HandleInput(Byte *readBuf, ByteCount bytesReceived)
{
	UInt64 now = CAHostTimeBase::GetCurrentTime();
#if DUMP_INPUT
	if (readBuf[0] != 0xFE || bytesReceived > 1)
		Dump("IN", readBuf, bytesReceived);
#endif
	mInputParser.FeedBytes(now, readBuf, bytesReceived);
}

// _________________________________________________________________________________________
//	SerialMIDIDevice::Send
//
void	SerialMIDIDevice::Send(const MIDIPacketList *pktlist, int portNumber)
{
	{
		CAMutex::Locker lock(mWriteQueueGuard);
		const MIDIPacket *srcpkt = pktlist->packet;
		for (int i = pktlist->numPackets; --i >= 0; ) {
			WriteQueueElem wqe;

			wqe.packet.Create(srcpkt);
#if DUMP_OUTPUT
			Dump("SEND", wqe.packet.Data(), wqe.packet.Length());
#endif
			if (portNumber == mNumEntities - 1)
				wqe.portNum = 0;				// the broadcast/realtime entity
			else
				wqe.portNum = portNumber + 1;	// 0-based entity => 1-based F5 cable
			wqe.bytesSent = 0;
			mWriteQueue.push_back(wqe);
			
			srcpkt = MIDIPacketNext(srcpkt);
		}
	}
	mWriteQueueGuard.Notify();
}

// _________________________________________________________________________________________
// SerialMIDIDriverBase::SerialMIDIPrepareOutput
//
// WriteQueue is an STL list of WriteQueueElem's to be transmitted, presumably containing
// at least one element.
// Copy into a buffer, destBuf, with a size of bufSize, with outgoing data as a standard
// MIDI stream.  Return the number of bytes written.
ByteCount	SerialMIDIDevice::PrepareOutput(				WriteQueue &		writeQueue, 			
															Byte *				destBuf, 
															ByteCount 			bufSize)
{
	Byte *dest = destBuf, *destend = dest + bufSize - 5;
		// Subtract from destend so that we never separate data bytes from their
		// preceding status bytes (except for sysex).  This is only to simplify
		// the loop below, so we don't have to check against destend for every 
		// byte we write to destBuf.

	while (!writeQueue.empty() && dest < destend) {		
		WriteQueue::iterator wqit = writeQueue.begin();
		WriteQueueElem *wqe = &(*wqit);
		Byte *dataStart = wqe->packet.Data();
		Byte *src = dataStart + wqe->bytesSent;
		Byte *srcend = dataStart + wqe->packet.Length();

		if (mUseF5 && wqe->portNum != mOutputPort) {
			*dest++ = 0xF5;
			*dest++ = mOutputPort = wqe->portNum;
			mOutputRunningStatus = 0;
		}
		if (src >= srcend) {
			// source packet completely emptied
			wqe->packet.Dispose();
			writeQueue.erase(wqit);
			continue;
		}
		while (src < srcend && dest < destend) {
			Byte c = *src++;
			
			// We can assume that any valid status byte is followed by the
			// correct number of data bytes (except sysex which can be followed
			// by ANY number of data bytes).  That's CoreMIDI's rule about
			// how MIDI is represented in MIDIPacketLists.
			switch (c >> 4) {
			// data bytes, presumably inside sysex
			case 0x0: case 0x1: case 0x2: case 0x3:
			case 0x4: case 0x5: case 0x6: case 0x7:
				*dest++ = c;
				break;
			
			// channel message status bytes followed by 2 data bytes
			case 0x8:	// note off
			case 0x9:	// note on
			case 0xA:	// channel pressure poly
			case 0xB:	// control change
			case 0xE:	// pitch bend
				if (c != mOutputRunningStatus)
					*dest++ = mOutputRunningStatus = c;
				*dest++ = *src++;
				*dest++ = *src++;
				break;
				
			// channel message status bytes followed by 1 data byte
			case 0xC:	// program change
			case 0xD:	// channel pressure mono
				if (c != mOutputRunningStatus)
					*dest++ = mOutputRunningStatus = c;
				*dest++ = *src++;
				break;
				
			// system message status bytes
			case 0xF:
				switch (c) {
				// common: 0 or N data bytes (clears running status)
				case 0xF0:	// sysex start (subsequent data bytes handled above)
				case 0xF6:	// tune request (0 data bytes)
				case 0xF7:	// end of exclusive
					mOutputRunningStatus = 0;
					// fall
				// realtime (does NOT clear running status, NOT cableized)
				case 0xF8:	// clock
				case 0xFA:	// start
				case 0xFB:	// continue
				case 0xFC:	// stop
				case 0xFE:	// active sensing
				case 0xFF:	// system reset
					*dest++ = c;
					break;
				
				// 1 data byte
				case 0xF1:	// MTC
				case 0xF3:	// song select
					mOutputRunningStatus = 0;
					*dest++ = c;
					*dest++ = *src++;
					break;
				// 2 data bytes
				case 0xF2:	// song pointer (2)
					mOutputRunningStatus = 0;
					*dest++ = c;
					*dest++ = *src++;
					*dest++ = *src++;
					break;
				default:
					// illegal MIDI message! advance until we find a status byte
					mOutputRunningStatus = 0;
					while (src < srcend && !(*src & 0x80))
						++src;
					break;
				}
				break;
			}
		
			if (src >= srcend) {
				// source packet completely emptied
				wqe->packet.Dispose();
				writeQueue.erase(wqit);
				if (writeQueue.empty())
					break;
			} else
				wqe->bytesSent = src - dataStart;
		}
	}
	return dest - destBuf;
}

void		SerialMIDIDevice::InputParser::EmitPacketList(Byte cable, const MIDIPacketList &pktlist)
{
	if (cable < mSerialMIDIDevice->mNumEntities) {
		// cable 0 -> the realtime/broadcast entity, which is the last one
		// other cable numbers are 1-based and must be decremented to map starting with entity 0
#if DUMP_INPUT
		char label[] = "IN0";
		label[2] += cable;
		Dump(label, pktlist.packet[0].data, pktlist.packet[0].length);
#endif
		if (cable == 0)
			cable = mSerialMIDIDevice->mNumEntities - 1;
		else
			cable -= 1;
		MIDIReceived(mSerialMIDIDevice->mSources[cable], &pktlist);
	}
}
