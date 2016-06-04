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
/*
	An effect unit will work on the canonical format - ie. PCM Float32
	Its input and output formats are equivalent - it does NO transformation of
	the format of its input to its output.
	
	It assumes that there will only ever be one input bus and one output bus.
*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	DebugAU.cpp
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "AUBase.h"
#include "DebugAUVersion.h"

#include <AudioUnit/AudioUnitProperties.h>

#include "AUDebugDispatcher.h"

static const AUChannelInfo sChannels[1] = { {-1, -1} };

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____DebugAU

/*
	virtual ComponentResult		PrepareInstrument(MusicDeviceInstrumentID inInstrument) = 0;

	virtual ComponentResult		ReleaseInstrument(MusicDeviceInstrumentID inInstrument) = 0;

	virtual ComponentResult		StartNote(		MusicDeviceInstrumentID 	inInstrument, 
												MusicDeviceGroupID 			inGroupID, 
												NoteInstanceID 				&outNoteInstanceID, 
												UInt32 						inOffsetSampleFrame, 
												const MusicDeviceNoteParams &inParams) = 0;

	virtual ComponentResult		StopNote(		MusicDeviceGroupID 			inGroupID, 
												NoteInstanceID 				inNoteInstanceID, 
												UInt32 						inOffsetSampleFrame) = 0;
*/

class DebugAU : public AUBase
{
public:
								DebugAU(AudioUnit component);
	virtual						~DebugAU () 
								{ 
									delete mDebugDispatcher; 
								}    

	virtual bool				StreamFormatWritable(	AudioUnitScope					scope,
														AudioUnitElement				element)
								{
									return IsInitialized() ? false : element == 0;
								}
																
	virtual	bool				SupportsTail () 
								{ 
									return true; 
								}

	virtual UInt32				SupportedNumChannels (const	AUChannelInfo** 				outInfo)
								{
									if (outInfo) *outInfo = sChannels;
									return sizeof (sChannels) / sizeof (AUChannelInfo);
								}

	virtual ComponentResult		Initialize();

	virtual ComponentResult		Version() 
								{ 
									return kDebugAUVersion; 
								}

	virtual ComponentResult 	Render(	AudioUnitRenderActionFlags	& ioActionFlags,
										const AudioTimeStamp		& inTimeStamp,
										UInt32						nFrames);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

COMPONENT_ENTRY(DebugAU)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	DebugAU::DebugAU
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DebugAU::DebugAU(AudioUnit component)
	: AUBase(component, 1, 1)
{
	mDebugDispatcher = new AUDebugDispatcher (this);
}

ComponentResult		DebugAU::Initialize()
{
		// get our current numChannels for input and output
	UInt32 auNumInputChan = GetInput(0)->GetStreamFormat().mChannelsPerFrame;
	UInt32 auNumOutputChan = GetOutput(0)->GetStreamFormat().mChannelsPerFrame;

	if (auNumInputChan != auNumOutputChan) 
		return kAudioUnitErr_FormatNotSupported;

    return noErr;
}

// we can use render here as we know that we're only rendering on bus zero.
ComponentResult 	DebugAU::Render(	AudioUnitRenderActionFlags	& ioActionFlags,
										const AudioTimeStamp		& inTimeStamp,
										UInt32						nFrames)
{
	if (!HasInput(0))
		return kAudioUnitErr_NoConnection;

	AUInputElement *theInput = GetInput(0);
	ComponentResult result = theInput->PullInput(ioActionFlags, inTimeStamp, 0 /* element */, nFrames);
	
	if (result == noErr)
	{
		AUOutputElement *theOutput = GetOutput(0);	// throws if error
		theInput->CopyBufferContentsTo (theOutput->GetBufferList());
	}
	
	return result;
}
