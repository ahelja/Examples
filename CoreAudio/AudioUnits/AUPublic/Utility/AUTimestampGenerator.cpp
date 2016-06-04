/*	Copyright: 	� Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
	AUTimestampGenerator.cpp
	
=============================================================================*/

#include "AUTimestampGenerator.h"
#include "CAMath.h"

#if DEBUG
static double DebugHostTime(const AudioTimeStamp &ts)
{
	static UInt64 baseHostTime = 0;
	if (!(ts.mFlags & kAudioTimeStampHostTimeValid))
		return -1.;
	if (baseHostTime == 0)
		baseHostTime = ts.mHostTime;
	return double(SInt64(ts.mHostTime) - SInt64(baseHostTime)) / CAHostTimeBase::GetFrequency();
}
#endif

void	AUTimestampGenerator::AddOutputTime(const AudioTimeStamp &inTimeStamp, Float64 expectedDeltaFrames, double outputSampleRate, double rateScalarAdj)
{
	mLastOutputTime = mCurrentOutputTime;
	mCurrentOutputTime = inTimeStamp;
	if (!(mCurrentOutputTime.mFlags & kAudioTimeStampHostTimeValid) && (mLastOutputTime.mFlags & kAudioTimeStampHostTimeValid)) {
		// no host time here but we had one last time, interpolate one
		double rateScalar = (mCurrentOutputTime.mFlags & kAudioTimeStampRateScalarValid) ? mCurrentOutputTime.mRateScalar : 1.0;
		Float64 deltaSamples = mCurrentOutputTime.mSampleTime - mLastOutputTime.mSampleTime;
		mCurrentOutputTime.mHostTime = mLastOutputTime.mHostTime + 
			UInt64(CAHostTimeBase::GetFrequency() * deltaSamples * rateScalar / outputSampleRate);
		mCurrentOutputTime.mFlags |= kAudioTimeStampHostTimeValid;
#if DEBUG
		if (mVerbosity > 1)
			printf("synthesized host time: %.3f (%.3f + %.f smp @ %.f Hz, rs %.3f\n", DebugHostTime(mCurrentOutputTime), DebugHostTime(mLastOutputTime), deltaSamples, outputSampleRate, rateScalar);
#endif
	}
	if (rateScalarAdj != 1.0) {
		if (mCurrentOutputTime.mFlags & kAudioTimeStampRateScalarValid)
			mCurrentOutputTime.mRateScalar *= rateScalarAdj;
		else {
			mCurrentOutputTime.mRateScalar = rateScalarAdj;
			mCurrentOutputTime.mFlags |= kAudioTimeStampRateScalarValid;
		}
	}
	
	if (mFirstTime) {
		mFirstTime = false;
		mDiscontinuous = false;
		if (!mStartInputAtZero)
			mNextInputSampleTime = mCurrentOutputTime.mSampleTime;
	} else {
		mDiscontinuous = fnotequal(mCurrentOutputTime.mSampleTime, mNextOutputSampleTime);
#if DEBUG
		if (mVerbosity > 1)
			if (mDiscontinuous)
				printf("%-12.12s: *** DISCONTINUOUS, got %.f, expected %.f\n", mDebugName, mCurrentOutputTime.mSampleTime, mNextOutputSampleTime);
#endif
	}
	mNextOutputSampleTime = mCurrentOutputTime.mSampleTime + expectedDeltaFrames;
}

const AudioTimeStamp &	AUTimestampGenerator::GenerateInputTime(Float64 framesToAdvance, double inputSampleRate)
{
	double inputSampleTime;
	
	mCurrentInputTime.mFlags = kAudioTimeStampSampleTimeValid;
	double rateScalar = 1.0;
	
	// propagate rate scalar
	if (mCurrentOutputTime.mFlags & kAudioTimeStampRateScalarValid) {
		mCurrentInputTime.mFlags |= kAudioTimeStampRateScalarValid;
		mCurrentInputTime.mRateScalar = rateScalar = mCurrentOutputTime.mRateScalar;
	}
	
	// propagate host time and sample time
	if (mCurrentOutputTime.mFlags & kAudioTimeStampHostTimeValid) {
		mCurrentInputTime.mFlags |= kAudioTimeStampHostTimeValid;
		mCurrentInputTime.mHostTime = mCurrentOutputTime.mHostTime;
		if (mDiscontinuous && (mLastOutputTime.mFlags & kAudioTimeStampHostTimeValid)) {
			// we had a discontinuous output time, need to resync by interpolating 
			// a sample time that is appropriate to the host time
			UInt64 deltaHostTime = mCurrentOutputTime.mHostTime - mLastOutputTime.mHostTime;
			double deltaSeconds = double(deltaHostTime) / CAHostTimeBase::GetFrequency();
			// samples/second * seconds = samples
			double deltaSamples = floor(inputSampleRate / rateScalar * deltaSeconds + 0.5);
			double lastInputSampleTime = mCurrentInputTime.mSampleTime;
			inputSampleTime = lastInputSampleTime + deltaSamples;
#if DEBUG
			if (mVerbosity > 1)
				printf("%-12.12s: adjusted input time: %.0f -> %.0f (SR=%.3f, rs=%.3f)\n", mDebugName, lastInputSampleTime, inputSampleTime, inputSampleRate, rateScalar);
#endif
			mDiscontinuous = false;
		} else
			inputSampleTime = mNextInputSampleTime;
	} else
		// we don't know the host time, so we can't do much
		inputSampleTime = mNextInputSampleTime;

	// propagate word clock
	if (mCurrentOutputTime.mFlags & kAudioTimeStampWordClockTimeValid) {
		mCurrentInputTime.mFlags |= kAudioTimeStampWordClockTimeValid;
		mCurrentInputTime.mWordClockTime = mCurrentOutputTime.mWordClockTime;
	}
	
	// propagate SMPTE time
	if (mCurrentOutputTime.mFlags & kAudioTimeStampSMPTETimeValid) {
		mCurrentInputTime.mFlags |= kAudioTimeStampSMPTETimeValid;
		mCurrentInputTime.mSMPTETime = mCurrentOutputTime.mSMPTETime;
	}
	
	// store the input sample time and expected next input time
	mCurrentInputTime.mSampleTime = inputSampleTime;
	mNextInputSampleTime = inputSampleTime + framesToAdvance;

#if DEBUG
	if (mVerbosity > 0) {
		printf("%-12.12s: out = %8.0f (%10.3fs)  in = %8.0f  (%10.3fs)  delta = %8.0f  advance = %8.0f\n", mDebugName, mCurrentOutputTime.mSampleTime, DebugHostTime(mCurrentOutputTime), inputSampleTime, DebugHostTime(mCurrentInputTime), mCurrentOutputTime.mSampleTime - inputSampleTime, framesToAdvance);
	}
#endif
	return mCurrentInputTime;
}
