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
#include "RenderSin.h"

// THESE values can be read from your data source
// they're used to tell the DefaultOutputUnit what you're giving it
Float64			sSampleRate = 48000;
SInt32			sNumChannels = 2;

SInt32			sWhichFormat = kAsFloat;


UInt32			sSinWaveFrameCount = 0; // this keeps track of the number of frames you render

double			sAmplitude = 0.25;
double			sToneFrequency = 440.;

UInt32 theFormatID = kAudioFormatLinearPCM;

// these are set based on which format is chosen
UInt32 theFormatFlags = 0;
UInt32 theBytesInAPacket = 0;
UInt32 theBitsPerChannel = 0;
UInt32 theBytesPerFrame = 0;

// these are the same regardless of format
UInt32 theFramesPerPacket = 1; // this shouldn't change




UInt32 sampleNextPrinted = 0;//print first time (UInt32)gSampleRate;

// REALLY should have a different version for each of the 3 bit depths...

// simulates reading the UInt data from the disk
// returns the adjusted frame count
void RenderSin (UInt32 				startingFrameCount, 
				UInt32 				inFrames, 
				void*				inBuffer, 
				double 				inSampleRate, 
				double 				amplitude, 
				double 				frequency,
				int					inOutputFormat) 
{
	double j = startingFrameCount;
	double cycleLength = inSampleRate / frequency;

	// mNumberBuffers is the same as the kNumChannels
	
	for (UInt32 frame = 0; frame < inFrames; ++frame) 
	{
		// generate inFrames 32-bit floats
		Float32 nextFloat = sin(j / cycleLength * (M_PI * 2.0)) * amplitude;

		switch (inOutputFormat) {
			case kAsFloat:
				static_cast<Float32*>(inBuffer)[frame] = nextFloat;
				break;
			case kAs16Bit:
				static_cast<SInt16*>(inBuffer)[frame] = static_cast<SInt16>(nextFloat * 32768. + 0.5);
				break;
				
			case kAs24Bit:
				static_cast<UInt32*>(inBuffer)[frame] = static_cast<UInt32>(nextFloat * 8388608. + 0.5);
				break;
		}
		
		j += 1.0;
		if (j > cycleLength)
			j -= cycleLength;
	}
			
	
	if (startingFrameCount >= sampleNextPrinted) {
		printf ("Current Slice: inFrames=%ld, startingFrameCount=%ld\n", inFrames, startingFrameCount);
		sampleNextPrinted += (int)inSampleRate;
	}
}



char* usageStr = "usage: [-d 16,24,32] where 32 is float, [-c numChannels], [-s sampleRate in Hz] [-a amplitude (0 - 1)], [-f freuency Hz]";

int ParseArgsAndSetup (int argc, const char* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		const char* str = argv[i];
		if (!strcmp ("-h", str)) {
			printf ("%s\n", usageStr);
			return -1;
		}
		else if (!strcmp ("-d", str)) {
			sscanf (argv[++i], "%ld", &sWhichFormat);
		} 
		else if (!strcmp ("-c", str)) {
			sscanf (argv[++i], "%ld", &sNumChannels);
		}
		else if (!strcmp ("-s", str)) {
			sscanf (argv[++i], "%lf", &sSampleRate);
		}
		else if (!strcmp ("-a", str)) {
			sscanf (argv[++i], "%lf", &sAmplitude);
		}
		else if (!strcmp ("-f", str)) {
			sscanf (argv[++i], "%lf", &sToneFrequency);
		} else {
			printf ("%s\n", usageStr);
			return -1;
		}
	}
	
	printf ("generating sin wave at %f Hz, %f amplitude\n", sToneFrequency, sAmplitude);
	
	switch (sWhichFormat) {
		case kAsFloat:
			theFormatFlags =  kLinearPCMFormatFlagIsFloat 
								| kLinearPCMFormatFlagIsBigEndian
								| kLinearPCMFormatFlagIsPacked
								| kAudioFormatFlagIsNonInterleaved;
			theBytesPerFrame = theBytesInAPacket = 4;
			theBitsPerChannel = 32;
			break;
		
		case kAs16Bit:
			theFormatFlags =  kLinearPCMFormatFlagIsSignedInteger 
								| kLinearPCMFormatFlagIsBigEndian
								| kLinearPCMFormatFlagIsPacked
								| kAudioFormatFlagIsNonInterleaved;
			theBytesPerFrame = theBytesInAPacket = 2;
			theBitsPerChannel = 16;		
			break;
			
		case kAs24Bit:
			theFormatFlags =  kLinearPCMFormatFlagIsSignedInteger 
								| kLinearPCMFormatFlagIsBigEndian
								| kAudioFormatFlagIsNonInterleaved;
			theBytesPerFrame = theBytesInAPacket = 4;
			theBitsPerChannel = 24;
			break;
		
		default:
			printf ("unknown format\n");
			return -1;
	}
	
	return 0;
}
