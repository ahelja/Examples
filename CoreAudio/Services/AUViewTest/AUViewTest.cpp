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
	AUViewTest.cpp
	
=============================================================================*/

#include "AUViewTest.h"
#include "XDebugging.h"
#include "AUEditWindow.h"
#include <AudioToolbox/AudioToolbox.h>


AUViewTest::AUViewTest() : XApp(CFSTR("main"))
{
	BuildGraph();
	BuildSequence();
}

void	AUViewTest::BuildGraph()
{
	RequireNoErr(NewAUGraph(&mGraph));

	RequireNoErr(AUGraphOpen(mGraph));
	
	// DLS synth -> multitap delay -> output

	AUNode synthNode, delayNode, outputNode;
	ComponentDescription desc;

	// create nodes
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	// DLS MusicDevice
	desc.componentType = kAudioUnitType_MusicDevice; 
	desc.componentSubType = kAudioUnitSubType_DLSSynth;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	RequireNoErr(AUGraphNewNode(mGraph, &desc, 0, NULL, &synthNode));

	// our test multitap delay
	// we look for the multi-delay, if we don't find it, use the OS installed delay	
	desc.componentType = kAudioUnitType_Effect; 
	desc.componentSubType = 'asmd';
	desc.componentManufacturer = 'Acme';
	
	if (FindNextComponent (NULL, &desc) == NULL) {
			// we didn't find the multi-tap, load the system one		
		desc.componentType = kAudioUnitType_Effect;
		desc.componentSubType = kAudioUnitSubType_Delay;
		desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	}
	
	RequireNoErr(AUGraphNewNode(mGraph, &desc, 0, NULL, &delayNode));
	
	// default output unit
	desc.componentType = kAudioUnitType_Output; 
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;

	RequireNoErr(AUGraphNewNode(mGraph, &desc, 0, NULL, &outputNode));


	// connect nodes
	RequireNoErr(AUGraphConnectNodeInput(	mGraph,
											synthNode, 0,
											delayNode, 0 ));

	RequireNoErr(AUGraphConnectNodeInput(	mGraph,
											delayNode, 0,
											outputNode, 0 ));
	
	RequireNoErr(AUGraphInitialize(mGraph));
	
	AudioUnit theSynth, theDelay, theOutput;
	RequireNoErr(AUGraphGetNodeInfo(mGraph, synthNode, NULL, NULL, NULL, &theSynth));
	RequireNoErr(AUGraphGetNodeInfo(mGraph, delayNode, NULL, NULL, NULL, &theDelay));
	RequireNoErr(AUGraphGetNodeInfo(mGraph, outputNode, NULL, NULL, NULL, &theOutput));
	
	mTargetUnit = theDelay;
	mSynth = theSynth;
	mSynthNode = synthNode;
	
	RequireNoErr(AUGraphStart(mGraph));
}

static int myrand(int range)
{
	return random() % range;
}

enum { kNumScales = 4 };
Byte gScales[kNumScales][8] = {
	// major 7
	{ 4, 3, 4, 1, 0 },
	// minor 7
	{ 3, 4, 3, 2, 0 },
	// 1 2 #4 5
	{ 2, 4, 1, 5, 0 },
	// whole tone
	//{ 2, 2, 2, 2, 2, 2, 0 },
	// 1 2 4 5 7
	{ 2, 3, 2, 3, 2, 0 }
};

void	AUViewTest::BuildSequence()
{
	MusicSequence sequence;
	MusicTrack track, tempoTrack;
	MusicPlayer player;
	
	RequireNoErr(
		NewMusicSequence(&sequence));

	RequireNoErr(
		MusicSequenceGetTempoTrack(sequence, &tempoTrack));
	RequireNoErr(
		MusicTrackNewExtendedTempoEvent(tempoTrack, 0, 120.));

	RequireNoErr(
		MusicSequenceNewTrack(sequence, &track));
	
	MIDIChannelMessage chmsg;
	// program change channel 0: piano
	chmsg.status = 0xC0;
	chmsg.data1 = 0;
	chmsg.data2 = 0;
	chmsg.reserved = 0;
	RequireNoErr(
		MusicTrackNewMIDIChannelEvent(track, 0., &chmsg));
	// program change channel 1: electric piano
	chmsg.status = 0xC1;
	chmsg.data1 = 5;
	chmsg.data2 = 0;
	chmsg.reserved = 0;
	RequireNoErr(
		MusicTrackNewMIDIChannelEvent(track, 0., &chmsg));
	
	// pan channel 1 hard left
	chmsg.status = 0xB0;
	chmsg.data1 = 10;
	chmsg.data2 = 0;
	chmsg.reserved = 0;
	RequireNoErr(
		MusicTrackNewMIDIChannelEvent(track, 0., &chmsg));
	// pan channel 2 hard right
	chmsg.status = 0xB1;
	chmsg.data1 = 10;
	chmsg.data2 = 127;
	chmsg.reserved = 0;
	RequireNoErr(
		MusicTrackNewMIDIChannelEvent(track, 0., &chmsg));

	// fill track with randomly-generated notes
	MusicTimeStamp t = 0;
	const float kDuration = 0.20, kInterval = 0.25;
	int channel = 0;
	while (t < 1000.) {
		int scaleNum = myrand(kNumScales);
		Byte *scale = gScales[scaleNum];
		int root = 32 + myrand(12);
		int nTimes = myrand(5);
		#define kMaxNote 96
		
		for (int i = 0; i < nTimes; ++i) {
			int note = root;
			// up
			Byte *p = scale;
			while (note < kMaxNote) {
				MIDINoteMessage msg;
				msg.channel = channel;
				msg.note = note;
				msg.velocity = 96 + myrand(16);
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3
				msg.releaseVelocity = 0;
#else
				msg.reserved = 0;
#endif
				msg.duration = kDuration;
		
				RequireNoErr(
					MusicTrackNewMIDINoteEvent(track, t, &msg));
				t += kInterval;
				note += *p++;
				if (*p == 0) p = scale;
			}
			channel = 1 - channel;
		}
	}
	RequireNoErr(
		MusicSequenceSetAUGraph(sequence, mGraph));
	RequireNoErr(
		MusicTrackSetDestNode(track, mSynthNode));
	
	RequireNoErr(
		NewMusicPlayer(&player));
	RequireNoErr(
		MusicPlayerSetSequence(player, sequence));
	
	RequireNoErr(
		MusicPlayerStart(player));

}

bool	AUViewTest::DoCommand(UInt32 command)
{
	switch (command) {
	case 'CUST':
	case 'GENE':
		new AUEditWindowController(this, mMainNib, mTargetUnit, command == 'GENE');
		break;
	default:
		return XApp::DoCommand(command);
	}
	return true;
}

// ______________________________________________________________________________

int main(int argc, char* argv[])
{
	AUViewTest *app = new AUViewTest;
	
	app->Initialize();
	app->Run();
}

