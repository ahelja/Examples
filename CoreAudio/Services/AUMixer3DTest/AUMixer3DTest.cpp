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
	AUMixer3DTest.cpp
	
=============================================================================*/


#include "AUMixer3DTest.h"
#include "XDebugging.h"
#include "AUEditWindow.h"
#include <AudioToolbox/AudioUnitUtilities.h>
#include <AudioToolbox/AudioFile.h>

#include "AUMixer3DView.h"

#include "AudioFilePlay.h"

#include <stdio.h>

#include <vector>
using namespace std;


//static const int kDesiredSpeakerConfig = kSpeakerConfiguration_5_0;
static const int kDesiredSpeakerConfig = kSpeakerConfiguration_Stereo;


#define EXIT_RESULT(str) 			\
		if (result) {				\
			printf ("Error in: %s,%ld\n", str, result); \
			exit (result); 			\
		}

// You need to set up the files that will be used for this project
// First define a base level directory from which the sound files can be found
#define SOUND_DIR		"/Full/Path/To/Sound/Dir/"

// Then these files are relative pathways to that directory
// the first three files are loaded first and are required...
#define file0   "Samples/hyper-reality/refreshing drink";
#define file1   "Free/helicopter.aif";
#define file2   "Free/footsteps2.aif";

// these are optional if you don't have them it doesn't matter...
#define file3   "Ethnic/ethnic4";
#define file4   "Ethnic/ethnic5";
#define file5   "Ethnic/ethnic3";
#define file6   "Ethnic/ethnic11";
#define file7  	"Ethnic/ethnic8";
#define file8   "Ethnic/ethnic13";
#define file9   "Samples/hyper-reality/crunched up paper";
#define file10  "Ethnic/ethnic21";
#define file11  "Ethnic/ethnic22";
#define file12  "Ethnic/ethnic20";


static void ReadTestFiles();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AUComponentDescription

// a convenience wrapper for ComponentDescription

#include <CoreAudio/CoreAudioTypes.h>

class AUComponentDescription : public ComponentDescription
{
public:
	AUComponentDescription()
	{
		 	componentType = 0;
			componentSubType = 0;
			componentManufacturer = 0;
			componentFlags = 0;
			componentFlagsMask = 0;
	};
	
			
	AUComponentDescription(	OSType			inType,
							OSType			inSubType,
							OSType			inManufacturer = 0,
							unsigned long	inFlags = 0,
							unsigned long	inFlagsMask = 0 )
	{
		 	componentType = inType;
			componentSubType = inSubType;
			componentManufacturer = inManufacturer;
			componentFlags = inFlags;
			componentFlagsMask = inFlagsMask;
	};

	AUComponentDescription(const ComponentDescription &inDescription )
	{
		*(ComponentDescription*)this = inDescription;
	};
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	FeedAudioData
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const int kNumSourceFiles = 13;

AudioFilePlayID gFilePlayer[kNumSourceFiles];

AudioUnit gTargetAudioUnit = 0;
UInt32 gInputBusNumber = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SetInput
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SetInput(int n)
{
    gInputBusNumber = n;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMixer3DTest::AUMixer3DTest
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUMixer3DTest::AUMixer3DTest() : XApp(CFSTR("AUMixer3DTest"))
{
    // initialize to use default edit view (may override later on)
	mViewComponentDesc.componentType = kAudioUnitCarbonViewComponentType;
	mViewComponentDesc.componentSubType = kAUCarbonViewSubType_Generic;
	mViewComponentDesc.componentManufacturer = kAudioUnitManufacturer_Apple;
	mViewComponentDesc.componentFlags = 0;
	mViewComponentDesc.componentFlagsMask = 0;

	TestMixer(kDesiredSpeakerConfig);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMixer3DTest::TestMixer
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AUMixer3DTest::TestMixer(UInt32 inSpeakerConfiguration )
{
	OSStatus result = noErr;
	
	RequireNoErr(NewAUGraph(&mGraph));

	AUNode outputNode;
	AUNode mixerNode;
	
	AudioUnit mixer3D;
	AudioUnit output;


	AUComponentDescription 	output_desc		(		
												kAudioUnitType_Output,
												kAudioUnitSubType_DefaultOutput,
												kAudioUnitManufacturer_Apple
												);

	AUComponentDescription 	mixer_desc		(		
												kAudioUnitType_Mixer,
												kAudioUnitSubType_3DMixer,
												kAudioUnitManufacturer_Apple
												);



	RequireNoErr(AUGraphNewNode(mGraph, &output_desc, 0, NULL, &outputNode));
	RequireNoErr(AUGraphNewNode(mGraph, &mixer_desc, 0, NULL, &mixerNode));

	RequireNoErr(AUGraphConnectNodeInput(	mGraph,
											mixerNode, 0,
											outputNode, 0 ));

	RequireNoErr(AUGraphOpen(mGraph));
	
	// now, get the 3D mixer and set its output format to what we want...
	result = AUGraphGetNodeInfo(mGraph, mixerNode, NULL, NULL, NULL, &mixer3D  );
	result = AUGraphGetNodeInfo(mGraph, outputNode, NULL, NULL, NULL, &output  );

	// setup the speaker configuration for the mixer
	result = AudioUnitSetProperty(	mixer3D,
							kAudioUnitProperty_SpeakerConfiguration,
							kAudioUnitScope_Output,
							0,
							&inSpeakerConfiguration,
							sizeof(inSpeakerConfiguration) );

	// turn on metering
	UInt32 meteringMode = 1;
	result = AudioUnitSetProperty(	mixer3D,
							kAudioUnitProperty_MeteringMode,
							kAudioUnitScope_Global,
							0,
							&meteringMode,
							sizeof(meteringMode) );



	RequireNoErr(AUGraphInitialize(mGraph));
	
	mTargetUnit = mixer3D;
	gTargetAudioUnit = mixer3D;
	mViewComponentDesc.componentSubType = '3dmx';

	ReadTestFiles();	
		
	// configure the 3D mixer rendering algorithm depending on
	// if we're output on a stereo system or 5.1 surround
	UInt32 spatAlgo = kSpatializationAlgorithm_HRTF;
	//UInt32 spatAlgo = kSpatializationAlgorithm_EqualPowerPanning;
	//UInt32 spatAlgo = kSpatializationAlgorithm_SphericalHead;
	//UInt32 spatAlgo = kSpatializationAlgorithm_SphericalHead;
	//UInt32 spatAlgo = 6 /*kSpatializationAlgorithm_StereoPassThrough*/;
	
	
	for (unsigned int i = 0; i < 3; i++)
	{
			// we can mix and match spatialization algorithms on the inputs
		if (inSpeakerConfiguration == kSpeakerConfiguration_5_1)
		{
			//if (i)
			//	spatAlgo = kSpatializationAlgorithm_SoundField;
			//else
				spatAlgo = kSpatializationAlgorithm_VectorBasedPanning;
		}
	
		spatAlgo = kSpatializationAlgorithm_HRTF;
		AFP_SetDestination (gFilePlayer[i], mixer3D, i);
		AFP_Connect (gFilePlayer[i]);

		
		result = AudioUnitSetProperty(	mixer3D,
										kAudioUnitProperty_SpatializationAlgorithm,
										kAudioUnitScope_Input,
										i,
										&spatAlgo,
										sizeof(spatAlgo) );
		
#if 0
		Float64 atten = 0.3;
		result = AudioUnitSetProperty(	mixer3D,
										kAudioUnitProperty_3DMixerDistanceAtten,
										kAudioUnitScope_Input,
										i,
										&atten,
										sizeof(atten) );
#else
		MixerDistanceParams distanceParams;
		distanceParams.mReferenceDistance = 1.0;
		distanceParams.mMaxDistance = 1000.0;
		distanceParams.mMaxAttenuation = 60.0;
		
		
		result = AudioUnitSetProperty(	mixer3D,
										kAudioUnitProperty_3DMixerDistanceParams,
										kAudioUnitScope_Input,
										i,
										&distanceParams,
										sizeof(distanceParams) );

#endif





		result = AudioUnitSetParameter(mixer3D,
					2 /* distance */,
					kAudioUnitScope_Input,
					i,
					1000.0,
					0 );
	}
 
	// start out with a very quiet master gain
	//
	AudioUnitSetParameter(mixer3D, k3DMixerParam_Gain, kAudioUnitScope_Output, 0, -20.0, 0);


	RequireNoErr(AUGraphStart(mGraph));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMixer3DTest::DoCommand
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool	AUMixer3DTest::DoCommand(UInt32 command)
{
	switch (command) {
	case kHICommandNew:
		DoNew();
		break;
	default:
		return XApp::DoCommand(command);
	}
	return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMixer3DTest::DoNew
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AUMixer3DTest::DoNew()
{
	new AUEditWindowController(this, mMainNib, mTargetUnit, mViewComponentDesc );
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	main
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int main(int argc, char* argv[])
{
	AUMixer3DTest	&app = *new AUMixer3DTest;

/*
	ComponentRegistrar<ReverbView, kAudioUnitCarbonViewComponentType, 
									'rvrb', 
									kAudioUnitManufacturer_Apple >();
*/
	ComponentRegistrar<AUMixer3DView, kAudioUnitCarbonViewComponentType, 
									'3dmx', 
									kAudioUnitManufacturer_Apple >();
	

	app.Initialize();
	app.DoNew();
	app.Run();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SetFileInput
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SetFileInput(int n)
{
	if (gFilePlayer[n] == 0)
		return;
		
	Boolean isConnected;
	RequireNoErr(AFP_IsConnected (gFilePlayer[n], &isConnected));	
	if (isConnected)
		return;
	
	for (int i = 0; i < kNumSourceFiles; ++i) 
	{
		Boolean isConnected;
		RequireNoErr(AFP_IsConnected (gFilePlayer[i], &isConnected));	
		if (isConnected) {
			UInt32 busNumber;
			RequireNoErr(AFP_GetInfo (gFilePlayer[i], NULL, &busNumber, NULL));
			if (busNumber == gInputBusNumber) {
				RequireNoErr(AFP_Disconnect (gFilePlayer[i]));
				break;
			}
		}
	}
		
	RequireNoErr(AFP_SetDestination (gFilePlayer[n], 
										gTargetAudioUnit, 
										gInputBusNumber));
	RequireNoErr(AFP_Connect(gFilePlayer[n]));
}

static void CreateFilePlay (const FSRef *ref, AudioFilePlayID* outPlayer)
{
	RequireNoErr(NewAudioFilePlayID (ref, outPlayer));
	RequireNoErr(AFP_SetLooping (*outPlayer, TRUE));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	ReadTestFiles
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void ReadTestFiles()
{
	FSRef ref;
	const UInt8* p = (const UInt8*)SOUND_DIR file0;
	printf ("loaded: %s\n", p);
	OSStatus result = FSPathMakeRef (p, &ref, NULL);
		EXIT_RESULT("FSPathMakeRef")
	CreateFilePlay (&ref, &gFilePlayer[0]);
		
	p = (const UInt8*)SOUND_DIR file1;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
		EXIT_RESULT("FSPathMakeRef")
	CreateFilePlay (&ref, &gFilePlayer[1]);

	p = (const UInt8*)SOUND_DIR file2;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
		EXIT_RESULT("FSPathMakeRef")
	CreateFilePlay (&ref, &gFilePlayer[2]);

// optional files
	p = (const UInt8*)SOUND_DIR file3;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
	if (result) {
		gFilePlayer[3] = 0;
		printf ("Can't load file: %ld\n", result);
	} else
		CreateFilePlay (&ref, &gFilePlayer[3]);

	p = (const UInt8*)SOUND_DIR file4;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
	if (result) {
		gFilePlayer[4] = 0;
		printf ("Can't load file: %ld\n", result);
	} else
		CreateFilePlay (&ref, &gFilePlayer[4]);
	
	p = (const UInt8*)SOUND_DIR file5;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
	if (result) {
		gFilePlayer[5] = 0;
		printf ("Can't load file: %ld\n", result);
	} else
		CreateFilePlay (&ref, &gFilePlayer[5]);

	p = (const UInt8*)SOUND_DIR file6;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
	if (result) {
		gFilePlayer[6] = 0;
		printf ("Can't load file: %ld\n", result);
	} else
		CreateFilePlay (&ref, &gFilePlayer[6]);
	
	p = (const UInt8*)SOUND_DIR file7;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
	if (result) {
		gFilePlayer[7] = 0;
		printf ("Can't load file: %ld\n", result);
	} else
		CreateFilePlay (&ref, &gFilePlayer[7]);
	
	p = (const UInt8*)SOUND_DIR file8;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
	if (result) {
		gFilePlayer[8] = 0;
		printf ("Can't load file: %ld\n", result);
	} else
		CreateFilePlay (&ref, &gFilePlayer[8]);
	
	p = (const UInt8*)SOUND_DIR file9;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
	if (result) {
		gFilePlayer[9] = 0;
		printf ("Can't load file: %ld\n", result);
	} else
		CreateFilePlay (&ref, &gFilePlayer[9]);
	
	p = (const UInt8*)SOUND_DIR file10;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
	if (result) {
		gFilePlayer[10] = 0;
		printf ("Can't load file: %ld\n", result);
	} else
		CreateFilePlay (&ref, &gFilePlayer[10]);
	
	p = (const UInt8*)SOUND_DIR file11;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
	if (result) {
		gFilePlayer[11] = 0;
		printf ("Can't load file: %ld\n", result);
	} else
		CreateFilePlay (&ref, &gFilePlayer[11]);
	
	p = (const UInt8*)SOUND_DIR file12;
	printf ("loaded: %s\n", p);
	result = FSPathMakeRef (p, &ref, NULL);
	if (result) {
		gFilePlayer[12] = 0;
		printf ("Can't load file: %ld\n", result);
	} else
		CreateFilePlay (&ref, &gFilePlayer[12]);
}
