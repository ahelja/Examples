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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	XMIDIClient.cpp
//

#include "XMIDIClient.h"
#include "XDebugging.h"
#include <CoreAudio/HostTime.h>

//#define DUMP_OUTPUT 1

#if DUMP_OUTPUT
#define Print64(x) (long)(x >> 32), (long)(x & 0xFFFFFFFF)
#define NSMS(x) (long)((x / kNanosPerMillisecond) & 0x7FFFFFF)		// nanos to millis

void	MIDIPacketListDump(const MIDIPacketList *pktlist)
{
	const MIDIPacket *pkt = pktlist->packet;
	UInt64 nowHW = AudioGetHWTime();
	UInt64 nowNS = AudioHWTimeToNanoseconds(nowHW);
	for (unsigned int i = 0; i < pktlist->numPackets; ++i) {
		//printf("%2d  %08lX%08lX: ", i, Print64(pkt->timeStamp));
		printf("%10ld %10ld: ", NSMS(nowNS), NSMS(AudioHWTimeToNanoseconds(pkt->timeStamp)));
		for (unsigned int j = 0; j < pkt->length; ++j)
			printf("%02X ", pkt->data[j]);
		printf("\n");
		pkt = MIDIPacketNext(pkt);
	}
}
#else
	#define MIDIPacketListDump(x)
#endif


XMIDIClient::XMIDIClient(CFStringRef name)
{
	OSStatus result = MIDIClientCreate (name, NotifyProc, this, &mClient);
		if (result) throw result;
}

XMIDIClient::~XMIDIClient()
{
	if (mClient)
		MIDIClientDispose(mClient);
}

void	XMIDIDestination::Set(MIDIEndpointRef endpt)
{
	mName[0] = 0;
	mType = kEndpt;
	mEndpt = endpt;
	if (endpt) {
		CFStringRef cfname;
			
		if (!MIDIObjectGetStringProperty(endpt, kMIDIPropertyName, &cfname)) {
			CFStringGetCString(cfname, mName, sizeof(mName), 0);
			CFRelease(cfname);
		}
	}
}

void	XMIDIDestination::Set(MIDIReadProc proc, void *refCon)
{
	mType = kReadProc;
	mProc = proc;
	mRefCon = refCon;
}

void	XMIDIDestination::SendPacketList(const MIDIPacketList *pktlist, MIDIPortRef outPort)
{
#if DUMP_OUTPUT
	MIDIPacketListDump(pktlist);
#endif
	if (mType == kEndpt) {
		if (mEndpt != NULL)
			MIDISend(outPort, mEndpt, pktlist);
	} else {
		(*mProc)(pktlist, mRefCon, NULL);
	}
}

const Byte *	XMIDIClient::NextMIDIEvent(const MIDIPacket *pkt, const Byte *event)
{
	const Byte *end = &pkt->data[pkt->length];
	if (event < &pkt->data[0] || event >= end)
		return end;
	
	Byte c = *event;
	switch ((c & 0xF0) >> 4) 
	{
		default:	// data byte -- assume in sysex
			while ((*++event & 0x80) == 0 && event < end)
				;
			break;
		case 0x8:
		case 0x9:
		case 0xA:
		case 0xB:
		case 0xE:
			event += 3;
			break;
		case 0xC:
		case 0xD:
			event += 2;
			break;
		case 0xF:
			switch (c) {
			case 0xF0:
				while ((*++event & 0x80) == 0 && event < end)
					;
				break;
			case 0xF1:
			case 0xF3:
				event += 2;
				break;
			case 0xF2:
				event += 3;
				break;
			default:
				++event;
			}
			break;
	}
	return (event >= end) ? end : event;
}

