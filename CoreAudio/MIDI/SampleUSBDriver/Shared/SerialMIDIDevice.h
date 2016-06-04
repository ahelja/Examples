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
	SerialMIDIDevice.h
	
=============================================================================*/

#ifndef __SerialMIDIDevice_h__
#define __SerialMIDIDevice_h__

#include "MIDIDriverDevice.h"
#include "SerialDevice.h"
#include <termios.h> 	
#include "XThread.h"
#include "MIDIParser.h"
#include "CAGuard.h"
#include "CAHostTimeBase.h"

class SerialMIDIDriverBase;

// _________________________________________________________________________________________
//	SerialMIDIDevice
// 
//	This class is the runtime state for one serial MIDI device.
class SerialMIDIDevice : public MIDIDriverDevice {
public:
					SerialMIDIDevice(	SerialMIDIDriverBase *	driver,
										SerialDevice *			usbDevice,
										MIDIDeviceRef			midiDevice);
	
	virtual			~SerialMIDIDevice();
	
	// we have two-stage construction, so that overridden virtual methods 
	// are correctly dispatched to subclasses
	virtual bool	Initialize();
						// return true for success
	
	virtual void	Send(const MIDIPacketList *pktlist, int portNumber);
	
	virtual void			HandleInput(	Byte *				readBuf,
											ByteCount 			readBufSize);
							// some serial bytes arrived, parse into a MIDIPacketList and
							// call MIDIReceived

	virtual ByteCount		PrepareOutput(	WriteQueue &		writeQueue,
											Byte *				destBuf,
											UInt32				size);
							// dequeue from WriteQueue into a MIDI stream, return
							// number of bytes dequeued.  Called with the queue mutex locked.

	void					RunReadThread();
	void					RunWriteThread();
	
	void					StartOrStopF5(bool start);
	
	// Leave data members public, so they may be accessed directly by driver methods.
	
	SerialMIDIDriverBase *		mDriver;
	SerialDevice *				mSerialDevice;
	
	// I/O state
	int							mPortFD;
	int							mDeathPillPipe[2];
	struct termios				mSavedTTYAttrs;
	bool						mStopRequested;
	
	IOBuffer					mReadBuf, mWriteBuf;
	
	CAGuard						mWriteQueueGuard;
	WriteQueue					mWriteQueue;

	// MIDI parse state
	bool						mUseF5;
	Byte						mOutputPort;
	Byte						mOutputRunningStatus;
	MIDITimeStamp				mLastRunningStatusClearedTime;
									// used to force periodic reassertion of running status
	
	// -------------
	class IOThread : public XThread {
	public:
		IOThread(SerialMIDIDevice *rs) :
			mSerialMIDIDevice(rs)
		{
		}
		
		virtual void	Start() {
							XThread::Start();
							UInt64 computeTime = CAHostTimeBase::ConvertFromNanos(250000);  // 250 us
							SetTimeConstraints(	0, 				// period - 0 means not periodic
												computeTime,
												2 * computeTime, 
												true 			// preemptible
												);
						}
	protected:
		SerialMIDIDevice *mSerialMIDIDevice;
	};
		
	class ReadThread : public IOThread {
	public:
		ReadThread(SerialMIDIDevice *rs) : IOThread(rs) { }
	
		virtual void	Run() {
			mSerialMIDIDevice->RunReadThread();
		}
	};
	
	class WriteThread : public IOThread {
	public:
		WriteThread(SerialMIDIDevice *rs) : IOThread(rs) { }
	
		virtual void	Run() { 
			UInt64 computeTime = CAHostTimeBase::ConvertFromNanos(250000);	// 250 us
			SetTimeConstraints(	0, 				// period - 0 means not periodic
								computeTime,
								2 * computeTime, 
								true 			// preemptible
								);
			mSerialMIDIDevice->RunWriteThread();
		}
	};
	
	class InputParser : public MIDIParser {
	public:
		InputParser(SerialMIDIDevice *rs) :
			MIDIParser(true),
			mSerialMIDIDevice(rs)
		{
		} 	// handle F5
		virtual void		EmitPacketList(Byte cable, const MIDIPacketList &pktlist);
	protected:
		SerialMIDIDevice *mSerialMIDIDevice;
	};
	
	ReadThread *				mReadThread;
	WriteThread *				mWriteThread;
	InputParser					mInputParser;
};

#endif // __SerialMIDIDevice_h__
