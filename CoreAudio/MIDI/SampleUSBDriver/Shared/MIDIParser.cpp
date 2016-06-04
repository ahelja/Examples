/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
	MIDIParser.cpp
	
=============================================================================*/

#include "MIDIParser.h"

#define kMaxPacketLength	sizeof(mPacket.data)
#define kInSysEx			(-1)
#define mPacket				mPacketList.packet[0]

MIDIParser::MIDIParser(bool parseF5)
{
	mPacketList.numPackets = 1;	// always!
	mCurrentCable = 0;
	mRunningStatus = 0;
	mDataRequired = 0;
	mParseF5 = parseF5;
}

void	MIDIParser::FeedBytes(MIDITimeStamp t, Byte *ptr, int nbytes)
{
	while (--nbytes >= 0) {
		Byte c = *ptr++;

		if (c & 0x80) {
			// status byte
			if (c >= 0xF8) {
				// realtime message
				switch (c) {
				case 0xF8:		// clock
				case 0xFA:		// start
				case 0xFB:		// continue
				case 0xFC:		// stop
				case 0xFF:		// system reset
					// realtime message - interrupts any message in progress
					// without altering state
					{
						MIDIPacketList realtimePacketList;
						realtimePacketList.numPackets = 1;
						realtimePacketList.packet[0].timeStamp = t;
						realtimePacketList.packet[0].length = 1;
						realtimePacketList.packet[0].data[0] = c;
						EmitPacketList(0, realtimePacketList);
					}
					break;
				default:
					// illegal or ignored (active sensing)
					break;
				}
			} else {
				// non-realtime status byte -- always begins packet, sets mDataRequired and mRunningStatus
				if (mDataRequired == kInSysEx) {
					// any status byte terminates sysex, but always write an F7 regardless
					// of whether one was sent (as per recommended practices)
					mPacket.data[mPacket.length++] = 0xF7;
					EmitPacket();
					if (c == 0xF7)
						return;			// nothing more to do
					// other status bytes start a new packet
				}
				
				// hopefully we got all the data bytes required by a previous status
				// byte because we're reinitializing the packet!
				
				// store the status byte
				mPacket.timeStamp = t;
				mPacket.length = 1;
				mPacket.data[0] = c;
				
				if (c < 0xF0) {
					// channel message
					mRunningStatus = c;
					mDataRequired = ((c & 0xE0) == 0xC0) ? 1 : 2;
				} else {
					// system message
					mRunningStatus = 0;		// which means no running status
					switch (c) {
					case 0xF0:		// SOX
						mDataRequired = kInSysEx;
						break;
					case 0xF1:		// MTC quarter frame
					case 0xF3:		// song select, 1 data byte
						mDataRequired = 1;
						break;
					case 0xF2:		// song ptr
						mDataRequired = 2;
						break;
					case 0xF6:		// tune request, 0 data bytes
						EmitPacket();
						mDataRequired = 0;
						break;
					case 0xF5:		// cable or undefined
						if (mParseF5) {
							mDataRequired = 1;
						} // else fall
					case 0xF4:		// undefined
					case 0xF7:		// EOX handled above
						break;
					}
				}
			}
		} else {
			// data byte
			int x = mDataRequired;
			if (x > 0) {
				// awaiting data bytes to complete a message
				mPacket.data[mPacket.length++] = c;
				if (--mDataRequired == 0)
					EmitPacket();
			} else if (x < 0) {
				// in sysex
				x = mPacket.length++;
				mPacket.data[x] = c;
				if (x == kMaxPacketLength) {
					EmitPacket();
					mPacket.length = 0;
				} else if (x == 0)
					mPacket.timeStamp = t;	// first byte in continuation of big sysex
			} else {
				if ((x = mRunningStatus) != 0) {
					// running status
					mPacket.timeStamp = t;
					mPacket.length = 2;
					mPacket.data[0] = x;		// running status
					mPacket.data[1] = c;		// the byte that just arrived
					if ((x & 0xE0) == 0xC0)
						EmitPacket();
					else
						mDataRequired = 1;
				}
				// else drop the data byte, syntax error
			}
		}
	}
}
