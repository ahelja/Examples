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
/* AudioData.c */
#include <stdlib.h>
#include <math.h>

#include "AudioData.h"

// Note: assumes stereo... should generalize for multichannel

#define ACCEPTABLE_ERROR	0.1
// ACCEPTABLE_ERROR is the fraction of a sample that we can allow give or take on when making our loop.
// 0.1 seems reasonable: doesn't require too much memory, and should be imperceptible to users.

// public function implementation
AudioData*		AudioDataNew (UInt32 inWaveType, Float64 inSampleRate, Float32 inFrequency, Float32 inAmplitude)
{
    AudioData*	retVal;
    Float32		samplesPerCycle, cyclesPerSample;
    UInt32		numberOfCyclesToCreate;
    UInt32		numberOfFramesNeeded;
    UInt32		i;
    Float32		positionInCycle;
    Float32		calculatedSample;
    
    // create new object
    retVal = (AudioData*)malloc (sizeof(AudioData));
    
    // cap amplitude
    if (inAmplitude < -1.0) inAmplitude = -1.0;
    if (inAmplitude > 1.0) inAmplitude = 1.0;
    
    // profile audio data that we need to build
    samplesPerCycle = (Float32)inSampleRate / inFrequency;
    numberOfCyclesToCreate = 1;		// have to have at least one (obviously)
    while ( ((samplesPerCycle * (Float32)numberOfCyclesToCreate) - (UInt32)(samplesPerCycle * (Float32)numberOfCyclesToCreate)) > ACCEPTABLE_ERROR ) {
        numberOfCyclesToCreate++;
    };
    
    numberOfFramesNeeded = (UInt32)(samplesPerCycle * (Float32)numberOfCyclesToCreate);
    
    // set object properties & allocate memory for PCM data
    retVal->mPlaybackHead = 0;
    retVal->mDataSizeInFrames = numberOfFramesNeeded;
    retVal->mData = (Float32*)malloc (retVal->mDataSizeInFrames * 2 * sizeof(Float32));
    
    // setup for rendering
    cyclesPerSample = (Float32)1.0 / samplesPerCycle;
    positionInCycle = 0;
    calculatedSample = 0;
    
    // render sample data
    for (i = 0; i < retVal->mDataSizeInFrames; i++) {	// for each sample we're rendering...
        
        switch (inWaveType) {
            case SINE_WAVE:
                calculatedSample = (Float32)sin (positionInCycle * 2 * M_PI);
                break;
            case SQUARE_WAVE:
                if (positionInCycle < 0.5) calculatedSample = -1.0;
                else calculatedSample = 1.0;
                break;
            case SAWTOOTH_WAVE:
                calculatedSample = (positionInCycle * 2.0) - 1.0;
                break;
            case TRIANGLE_WAVE:
                if (positionInCycle < 0.5) calculatedSample = (positionInCycle * 4.0) - 1.0;
                else calculatedSample = ((1.0 - positionInCycle) * 4.0) - 1.0;
                break;
        }
        
        calculatedSample *= inAmplitude;
        retVal->mData[i * 2] = calculatedSample;
        retVal->mData[i * 2 + 1] = calculatedSample;
        
        positionInCycle += cyclesPerSample;
        if (positionInCycle >= 1.0) positionInCycle -= 1.0;
    }
    
    // return new object
    return retVal;
}

void			AudioDataDestroy (AudioData* inAD)
{
    free (inAD->mData);
    free (inAD);
}

void			AudioDataMovePlaybackHead (AudioData* inAD, SInt32 framesToMove)
{
    inAD->mPlaybackHead = ((SInt32)inAD->mPlaybackHead + framesToMove) % inAD->mDataSizeInFrames;
}

void			AudioDataZeroPlaybackHead (AudioData* inAD)
{
    inAD->mPlaybackHead = 0;
}

/*
 */
