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
	MIDIParser.h
	
=============================================================================*/

#ifndef __MIDIParser_h__
#define __MIDIParser_h__

#include <CoreMIDI/CoreMIDI.h>

// feed it bytes, it will emit 1-packet MIDIPacketLists
class MIDIParser {
public:
	MIDIParser(bool parseF5);
	virtual ~MIDIParser() { }

	void				FeedBytes(MIDITimeStamp t, Byte *ptr, int nbytes);

protected:
	void				EmitPacket() {
							if (mPacketList.packet[0].data[0] == 0xF5) {
								mCurrentCable = mPacketList.packet[0].data[1];
								return;
							}
							EmitPacketList(mCurrentCable, mPacketList);
						}
	virtual void		EmitPacketList(Byte cable, const MIDIPacketList &pktlist) = 0;
							// note that with F5 protocol, realtime is not cableized and
							// will arrive on cable 0 (???)
	
	bool				mParseF5;
	Byte				mCurrentCable;	// last F5 xx message, 0 if none
	Byte				mRunningStatus;	// 0 if none
	int					mDataRequired;	// -1 if in sysex else number of data bytes
										// expected in order to complete a message
										// or 0 if expecting a status byte or a legal
										// running status data byte
	MIDIPacketList		mPacketList;
};

#endif // __MIDIParser_h__
