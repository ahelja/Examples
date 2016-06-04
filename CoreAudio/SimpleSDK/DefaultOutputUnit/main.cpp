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
        File:			UsingDefaultNoAC.cpp
		
		Description:	DefaultOutputUnit
						
						This is a command line tool that generates a sine wave at the specified frequency
						
						It uses the DefaultOutputUnit, so it will continue to generate this sine wave:
						- if the user changes the destination audio device for the default output
						- if that device has a different stream format than the previous one
						
        Author:			Doug Wyatt, William Stewart
*/

#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <unistd.h>
#include <AudioUnit/AudioUnit.h>

#include <math.h>

#include "RenderSin.h"

AudioUnit	gOutputUnit;


OSStatus	MyRenderer(void 				*inRefCon, 
				AudioUnitRenderActionFlags 	*ioActionFlags, 
				const AudioTimeStamp 		*inTimeStamp, 
				UInt32 						inBusNumber, 
				UInt32 						inNumberFrames, 
				AudioBufferList 			*ioData)

{
	RenderSin (sSinWaveFrameCount, 
				inNumberFrames,  
				ioData->mBuffers[0].mData, 
				sSampleRate, 
				sAmplitude, 
				sToneFrequency, 
				sWhichFormat);

		//we're just going to copy the data into each channel
	for (UInt32 channel = 1; channel < ioData->mNumberBuffers; channel++)
		memcpy (ioData->mBuffers[channel].mData, ioData->mBuffers[0].mData, ioData->mBuffers[0].mDataByteSize);

	sSinWaveFrameCount += inNumberFrames;
		
	return noErr;
}

// ________________________________________________________________________________
//
// CreateDefaultAU
//
void	CreateDefaultAU()
{
	OSStatus err = noErr;

	// Open the default output unit
	ComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	Component comp = FindNextComponent(NULL, &desc);
	if (comp == NULL) { printf ("FindNextComponent\n"); return; }
	
	err = OpenAComponent(comp, &gOutputUnit);
	if (comp == NULL) { printf ("OpenAComponent=%ld\n", err); return; }

	// Set up a callback function to generate output to the output unit
    AURenderCallbackStruct input;
	input.inputProc = MyRenderer;
	input.inputProcRefCon = NULL;

	err = AudioUnitSetProperty (gOutputUnit, 
								kAudioUnitProperty_SetRenderCallback, 
								kAudioUnitScope_Input,
								0, 
								&input, 
								sizeof(input));
	if (err) { printf ("AudioUnitSetProperty-CB=%ld\n", err); return; }
    
}

// ________________________________________________________________________________
//
// TestDefaultAU
//
void	TestDefaultAU()
{
	OSStatus err = noErr;
    
	// We tell the Output Unit what format we're going to supply data to it
	// this is necessary if you're providing data through an input callback
	// AND you want the DefaultOutputUnit to do any format conversions
	// necessary from your format to the device's format.
	AudioStreamBasicDescription streamFormat;
		streamFormat.mSampleRate = sSampleRate;		//	the sample rate of the audio stream
		streamFormat.mFormatID = theFormatID;			//	the specific encoding type of audio stream
		streamFormat.mFormatFlags = theFormatFlags;		//	flags specific to each format
		streamFormat.mBytesPerPacket = theBytesInAPacket;	
		streamFormat.mFramesPerPacket = theFramesPerPacket;	
		streamFormat.mBytesPerFrame = theBytesPerFrame;		
		streamFormat.mChannelsPerFrame = sNumChannels;	
		streamFormat.mBitsPerChannel = theBitsPerChannel;	

	printf("Rendering source:\n\t");
	printf ("SampleRate=%f,", streamFormat.mSampleRate);
	printf ("BytesPerPacket=%ld,", streamFormat.mBytesPerPacket);
	printf ("FramesPerPacket=%ld,", streamFormat.mFramesPerPacket);
	printf ("BytesPerFrame=%ld,", streamFormat.mBytesPerFrame);
	printf ("BitsPerChannel=%ld,", streamFormat.mBitsPerChannel);
	printf ("ChannelsPerFrame=%ld\n", streamFormat.mChannelsPerFrame);
	
	err = AudioUnitSetProperty (gOutputUnit,
							kAudioUnitProperty_StreamFormat,
							kAudioUnitScope_Input,
							0,
							&streamFormat,
							sizeof(AudioStreamBasicDescription));
	if (err) { printf ("AudioUnitSetProperty-SF=%4.4s, %ld\n", (char*)&err, err); return; }
	
    // Initialize unit
	err = AudioUnitInitialize(gOutputUnit);
	if (err) { printf ("AudioUnitInitialize=%ld\n", err); return; }
    
	Float64 outSampleRate;
	UInt32 size = sizeof(Float64);
	err = AudioUnitGetProperty (gOutputUnit,
							kAudioUnitProperty_SampleRate,
							kAudioUnitScope_Output,
							0,
							&outSampleRate,
							&size);
	if (err) { printf ("AudioUnitSetProperty-GF=%4.4s, %ld\n", (char*)&err, err); return; }

	// Start the rendering
	// The DefaultOutputUnit will do any format conversions to the format of the default device
	err = AudioOutputUnitStart (gOutputUnit);
	if (err) { printf ("AudioOutputUnitStart=%ld\n", err); return; }
			
			// we call the CFRunLoopRunInMode to service any notifications that the audio
			// system has to deal with
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 2, false);

// REALLY after you're finished playing STOP THE AUDIO OUTPUT UNIT!!!!!!	
// but we never get here because we're running until the process is nuked...	
	verify_noerr (AudioOutputUnitStop (gOutputUnit));
	
    err = AudioUnitUninitialize (gOutputUnit);
	if (err) { printf ("AudioUnitUninitialize=%ld\n", err); return; }
}

void CloseDefaultAU ()
{
	// Clean up
	CloseComponent (gOutputUnit);
}

void CreateArgListFromString (int *ioArgCount, char **ioArgs, char *inString)
{
    int i, length;
    length = strlen (inString);
    
    // prime for first argument
    ioArgs[0] = inString;
    *ioArgCount = 1;
    
    // get subsequent arguments
    for (i = 0; i < length; i++) {
        if (inString[i] == ' ') {
            inString[i] = 0;		// terminate string
            ++(*ioArgCount);		// increment count
            ioArgs[*ioArgCount - 1] = inString + i + 1;	// set next arg pointer
        }
    }
}

// ________________________________________________________________________________
//
// TestDefaultAU
//
int main(int argc, const char * argv[])
{
    CreateDefaultAU();
    
    if (argc == 1) {
        // case for no arguments specified: batch test run
        Float64 sampleRateList[] = {	
										8000.0, 11025.0, 16000.0, 22050.0, 32000.0, 44100.0, 48000.0, 64000.0,
                                        88200.0, 96000.0, 192000.0, 320000.0, 55339.75 
									};
        UInt32	sampleRateListLength = sizeof(sampleRateList) / sizeof(Float64);
        UInt32	channelCountList[] = {	2, 4, 8	};
        UInt32	channelCountListLength = sizeof(channelCountList) / sizeof(UInt32);
        UInt32	bitDepthList[] = {	16, 24, 32	};
        UInt32	bitDepthListLength = sizeof(bitDepthList) / sizeof(UInt32);
        
        UInt32 i, j, k;
        char testString[1024];
        
        int internalArgc;
        char* internalArgv[256];
        OSStatus result;
        
        for (i = 0; i < bitDepthListLength; i++) {
            for (j = 0; j < channelCountListLength; j++) {
                for (k = 0; k < sampleRateListLength; k++) {
                    sprintf (testString, "executable -d %lu -c %lu -s %f", bitDepthList[i], channelCountList[j], sampleRateList[k]);
                    printf ("# Testing at %lu bits, with %lu channels, at %fHz.\n", bitDepthList[i], channelCountList[j], sampleRateList[k]);
                    
                    CreateArgListFromString (&internalArgc, internalArgv, testString);
                    if (ParseArgsAndSetup (internalArgc, (const char**)internalArgv)) return -1;
                    TestDefaultAU();
                    
                    // reset unit
                    result = AudioUnitReset (gOutputUnit, kAudioUnitScope_Input, 0);
                    if (result != noErr) {
                        printf ("Error %d while trying to reset output unit.\n", (int)result);
                        exit(-1);
                    }
                }
            }
        }
    } else {
        // case for arguments specified: run specified test
        if (ParseArgsAndSetup (argc, argv)) return -1;
        TestDefaultAU();
    }
    
    CloseDefaultAU();
    
    return 0;
}
