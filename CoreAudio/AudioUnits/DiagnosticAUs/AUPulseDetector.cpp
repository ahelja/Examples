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
	AUPulseDetector.cpp
	
=============================================================================*/
#include "AUEffectBase.h"
#include "AUPulseDetectorVersion.h"

#include "AUPulseShared.h"

#if AU_DEBUG_DISPATCHER
	#include "AUDebugDispatcher.h"
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AUPulseDetector

static const AUChannelInfo sChannels[1] = { {1, 1} };

enum {
	kDoPulseDetection	= 0,
	kPulseRestTime		= 1,
	kPulseThreshold 	= 2,
	kPulseLength 		= 3,
	kWritePulseStats	= 4
};

const static float kPulseThresholdDefault = 0.5;
const static float kPulseThresholdMin = 0.4;
const static float kPulseThresholdMax = 1.;

const static float kPulseLengthDefault = 8.;
const static float kPulseLengthMin = 4.;
const static float kPulseLengthMax = 32.;

const static float kPulseRestTimeDefault = 5;
const static float kPulseRestTimeMin = 1;
const static float kPulseRestTimeMax = 60;

const static float kDoPulseDetectionDefault = 0;
const static float kDoPulseDetectionMin = 0;
const static float kDoPulseDetectionMax = 1;

class AUPulseDetector : public AUEffectBase
{
public:
								AUPulseDetector(AudioUnit component);
								virtual ~AUPulseDetector ();
	
	virtual AUKernelBase *		NewKernel() { return new AUPulseDetectorKernel(this); }
	
	virtual ComponentResult		GetPropertyInfo (AudioUnitPropertyID			inID,
												AudioUnitScope					inScope,
												AudioUnitElement				inElement,
												UInt32 &						outDataSize,
												Boolean &						outWritable)
	{
		if (inScope == kAudioUnitScope_Global) {
			switch (inID) {
				case kAUPulseMetricsPropertyID:
					outDataSize = sizeof (AUPulseMetrics);
					outWritable = false;
					return noErr;
				default:
					break;
			}
		}
		return AUEffectBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
	}

	virtual ComponentResult		GetProperty(		AudioUnitPropertyID inID,
													AudioUnitScope 		inScope,
													AudioUnitElement 	inElement,
													void *				outData );
	
	virtual	bool				SupportsTail () { return true; }

	virtual ComponentResult 	SetParameter(			AudioUnitParameterID			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														Float32							inValue,
														UInt32							inBufferOffsetInFrames);
	
	virtual UInt32				SupportedNumChannels (	const AUChannelInfo** 			outInfo)
								{
									if (outInfo) *outInfo = sChannels;
									return sizeof (sChannels) / sizeof (AUChannelInfo);
								}

	virtual	ComponentResult		GetParameterInfo(	AudioUnitScope			inScope,
                                                    AudioUnitParameterID	inParameterID,
													AudioUnitParameterInfo	&outParameterInfo	);

	virtual int					GetNumCustomUIComponents () { return 1; }

	virtual void				GetUIComponentDescs (ComponentDescription* inDescArray)
	{
		ComponentDescription desc;
		desc.componentType = 'auvw';
		desc.componentSubType = 'puls';
		desc.componentManufacturer = 'appl';
		desc.componentFlags = 0;
		desc.componentFlagsMask = 0;  
		*inDescArray = desc;//{ 'auvw', 'puls', 'appl', 0, 0 };
	}
	
	virtual ComponentResult	Version() { return kAUPulseDetectorVersion; }

	virtual ComponentResult Render(	AudioUnitRenderActionFlags 			&ioActionFlags,
											const AudioTimeStamp &		inTimeStamp,
											UInt32						nFrames)
	{
		mRenderTime = inTimeStamp;
		return AUEffectBase::Render (ioActionFlags, inTimeStamp, nFrames);
	}


protected:
	class AUPulseDetectorKernel : public AUKernelBase		// most real work happens here
	{
	public:
		AUPulseDetectorKernel(AUPulseDetector *inAudioUnit )
			: AUKernelBase(inAudioUnit),
			  mParentObject (inAudioUnit)
		{
			Reset();
			mParentObject->mChildObject = this;
		}

		virtual ~AUPulseDetectorKernel() 
		{ 
			mParentObject->mChildObject = NULL; 
		}
		
// Required overides for the process method for this effect
		// processes one channel of interleaved samples
		virtual void 		Process(	const Float32 	*inSourceP,
										Float32		 	*inDestP,
										UInt32 			inFramesToProcess,
										UInt32			inNumChannels,
                                        bool			&ioSilence);
		
		virtual void		Reset () 
		{ 
			mWhichMode = kEstablishMode; 
			mTotalMeasurements = 0;
			mTotalMeasurementsSquared = 0;
			mNumMeasurements = 0;
			mMaxTime = 0;
			mMinTime = 0xFFFFFFFF; 
			mLastMeasurement = 0;
			mLastFrames = 0;
			mParentObject->ClearPulseTS();
		}
	
		Float64 			SampleTime () 
		{ 
			return mParentObject->mRenderTime.mSampleTime; 
		}
		
	public: //state variables...
		enum {
			kDetectMode,
			kCleanMode,
			kEstablishMode
		};
		
		UInt32 mWhichMode;
		Float64 mPulseStartTime;
		SInt32 mDoneClean;

		Float64 mTotalMeasurements;
		Float64 mTotalMeasurementsSquared;
		UInt32  mLastMeasurement;
		UInt32  mNumMeasurements;
		UInt32  mMaxTime;
		UInt32  mMinTime;
		UInt32  mLastFrames;
		bool	mWasSuccessful;
		
		AUPulseDetector* mParentObject;
	};
	
	AudioTimeStamp mRenderTime;
	AUPulseDetectorKernel* mChildObject;

	enum {
		kPulseTSSize = 256
	};
	
	struct PulseTS {
		Float64 start;
		Float64 length;
	};
	
	PulseTS *mPulseTimeStats;
	UInt32  mCurrentPTSIndex;

public:
	void			ClearPulseTS ()
	{
		memset (mPulseTimeStats, 0, (sizeof(PulseTS) * kPulseTSSize));
		mCurrentPTSIndex = 0;
	}
	
	void			WritePulseTS ();

	void			DetectedPulse (Float64 startTime, Float64 duration)
	{
		mPulseTimeStats[mCurrentPTSIndex].start = startTime;
		mPulseTimeStats[mCurrentPTSIndex].length = duration;
		
		mCurrentPTSIndex = (++mCurrentPTSIndex % kPulseTSSize);
		
		PropertyChanged (kAUPulseMetricsPropertyID, kAudioUnitScope_Global, 0);
	}
	
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

COMPONENT_ENTRY(AUPulseDetector)

#pragma mark ____AUPulseDetector

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUPulseDetector::AUPulseDetector
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUPulseDetector::AUPulseDetector(AudioUnit component)
	: AUEffectBase(component),
	  mChildObject(NULL)
{
	CreateElements();
	
	CAStreamBasicDescription monoDesc;
	monoDesc.SetCanonical (1, false);
	monoDesc.mSampleRate = 44100.;
	
	GetOutput(0)->SetStreamFormat(monoDesc);
	GetInput(0)->SetStreamFormat(monoDesc);

	Globals()->UseIndexedParameters (5);
	Globals()->SetParameter (kPulseThreshold, kPulseThresholdDefault);
	Globals()->SetParameter (kPulseLength, kPulseLengthDefault);
	Globals()->SetParameter (kPulseRestTime, kPulseRestTimeDefault);
	Globals()->SetParameter (kDoPulseDetection, kDoPulseDetectionDefault);
	Globals()->SetParameter (kWritePulseStats, 0);

	mPulseTimeStats = new PulseTS[kPulseTSSize];
	
#if AU_DEBUG_DISPATCHER
	mDebugDispatcher = new AUDebugDispatcher (this);
#endif
}

AUPulseDetector::~AUPulseDetector () 
{ 			
	delete [] mPulseTimeStats;
#if AU_DEBUG_DISPATCHER
	delete mDebugDispatcher; 
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUPulseDetector::GetParameterInfo
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult		AUPulseDetector::GetParameterInfo(	AudioUnitScope			inScope,
                                                        AudioUnitParameterID	inParameterID,
                                                        AudioUnitParameterInfo	&outParameterInfo )
{
	outParameterInfo.flags = 	kAudioUnitParameterFlag_IsWritable
						+		kAudioUnitParameterFlag_IsReadable;
    
    if (inScope == kAudioUnitScope_Global) 
	{
        switch(inParameterID)
        {
            case kPulseLength:
                AUBase::FillInParameterName (outParameterInfo, CFSTR("Pulse Length"), false);
				AUBase::HasClump (outParameterInfo, 2);				
				outParameterInfo.unit = kAudioUnitParameterUnit_SampleFrames;
				outParameterInfo.minValue = kPulseLengthMin;
				outParameterInfo.maxValue = kPulseLengthMax;
				outParameterInfo.defaultValue = kPulseLengthDefault;
				return noErr;
            
			case kPulseThreshold:
                AUBase::FillInParameterName (outParameterInfo, CFSTR("Pulse Threshold"), false);
				AUBase::HasClump (outParameterInfo, 2);				
                outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
                outParameterInfo.minValue = kPulseThresholdMin;
                outParameterInfo.maxValue = kPulseThresholdMax;
                outParameterInfo.defaultValue = kPulseThresholdDefault;
				return noErr;

			case kPulseRestTime:
                AUBase::FillInParameterName (outParameterInfo, CFSTR("Time Between Pulses"), false);
				AUBase::HasClump (outParameterInfo, 2);				
                outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
                outParameterInfo.minValue = kPulseRestTimeMin;
                outParameterInfo.maxValue = kPulseRestTimeMax;
                outParameterInfo.defaultValue = kPulseRestTimeDefault;
				return noErr;

			case kDoPulseDetection:
                AUBase::FillInParameterName (outParameterInfo, CFSTR("Do Pulse Detection"), false);
				AUBase::HasClump (outParameterInfo, 1);				
                outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
                outParameterInfo.minValue = kDoPulseDetectionMin;
                outParameterInfo.maxValue = kDoPulseDetectionMax;
                outParameterInfo.defaultValue = kDoPulseDetectionDefault;
				return noErr;
				
			case kWritePulseStats:
				outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable;
                AUBase::FillInParameterName (outParameterInfo, CFSTR("Write Pulse Time Stamps"), false);
				AUBase::HasClump (outParameterInfo, 3);
                outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
                outParameterInfo.minValue = 0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = 0;
				return noErr;
		}
	}


	
	return AUEffectBase::GetParameterInfo (inScope, inParameterID, outParameterInfo);
}

ComponentResult		AUPulseDetector::GetProperty(	AudioUnitPropertyID inID,
													AudioUnitScope 		inScope,
													AudioUnitElement 	inElement,
													void *				outData )
{
	if (inScope == kAudioUnitScope_Global) 
	{
		switch (inID) 
		{
			case kAUPulseMetricsPropertyID:
			{
				if (!mChildObject) return kAudioUnitErr_InvalidPropertyValue; //we don't have any data

				AUPulseMetrics metrics;
				AUPulseDetectorKernel &child = *mChildObject;
				
				metrics.isValid = child.mWasSuccessful;
				
				if (metrics.isValid) 
				{
					metrics.min = child.mMinTime;
					metrics.max = child.mMaxTime;
					
					UInt32 numMeasures = child.mNumMeasurements;
					UInt32 lastMeasure = child.mLastMeasurement;
					
					if (numMeasures > 1) {
						Float64 sumMeasures = child.mTotalMeasurements;
						Float64 sumMeasuresSquared = child.mTotalMeasurementsSquared;
						
						metrics.mean = sumMeasures / numMeasures;
						
							//	stdDev = (sum of Xsquared -((sum of X)*(sum of X)/N)) / (N-1))
						metrics.stdDev = sqrt ((sumMeasuresSquared - ((sumMeasures * sumMeasures) / numMeasures)) / (numMeasures-1));
					} else {
						metrics.mean = lastMeasure;
						metrics.stdDev = 0;
					}
					metrics.numMeasurements = numMeasures;
					metrics.lastPulseTime = lastMeasure;
					metrics.numFrames = child.mLastFrames;
				} else {
					metrics.min = metrics.max = 0;
					metrics.mean = metrics.stdDev = 0;
					metrics.numMeasurements = 0;
					metrics.lastPulseTime = metrics.numFrames = 0;
				}
				
				*((AUPulseMetrics*)outData) = metrics;
			}
			return noErr;
			
			default:
				break;
		}
	}
	return AUEffectBase::GetProperty (inID, inScope, inElement, outData);
}

ComponentResult 	AUPulseDetector::SetParameter(		AudioUnitParameterID			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														Float32							inValue,
														UInt32							inBufferOffsetInFrames)
{
	if (inID == kWritePulseStats && inValue > 0.5) {
		WritePulseTS();
		return noErr;
	}

	bool wasOff = false;
	if (inID == kPulseRestTime && mChildObject) {
		mChildObject->mDoneClean = 0;
		mChildObject->mWhichMode = AUPulseDetector::AUPulseDetectorKernel::kCleanMode;
	}
	if (inID == kDoPulseDetection)
		wasOff = GetParameter (kDoPulseDetection) == 0.0;
		
	ComponentResult result = AUBase::SetParameter (inID, inScope, inElement, inValue, inBufferOffsetInFrames);
		
			// establish a new pulse
	if (inID == kDoPulseDetection && mChildObject) {
		if (GetParameter (kDoPulseDetection) && wasOff)
			mChildObject->mWhichMode = AUPulseDetector::AUPulseDetectorKernel::kEstablishMode;
	}
	
	return result;
}

void		AUPulseDetector::WritePulseTS ()
{
	static int currentWriteFile = 1;
	int index = mCurrentPTSIndex;

	if (index == 0) {
		if (mPulseTimeStats[0].length == 0) {
			printf ("No pulse stats to write\n");
			return;
		}
		index = kPulseTSSize;
	}
	
	static char str[1024];
	sprintf (str, "/tmp/au-pulse-ts-%d.txt", currentWriteFile++);
	
	FILE * pFile = fopen (str,"wt");
	if (pFile != NULL) {
		printf ("Writing %d stats to %s\n", index, str);
		fprintf (pFile, "Start Time (Samples)\tDuration\n");
		for (int i = 0; i < index; ++i) {
			fprintf (pFile, "%.0f\t%.0f\n", mPulseTimeStats[i].start, mPulseTimeStats[i].length);
		}
		
		fclose (pFile);
	} else 
		printf ("Can't create file:%s\n", str);
	
	ClearPulseTS();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AUPulseDetectorKernel

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUPulseDetector::AUPulseDetectorKernel::Process
//
//		pass-through audio
//		do spike detection
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AUPulseDetector::AUPulseDetectorKernel::Process(const Float32 	*inSourceP,
                                                    Float32		 	*inDestP,
                                                    UInt32 			inFramesToProcess,
                                                    UInt32			inNumChannels,
                                                    bool			&ioSilence )
{
	if (GetParameter (kDoPulseDetection) == 0) {
		return;
	}
	
	switch (mWhichMode)
	{
		case kDetectMode:
		{
			Float64 now = SampleTime();
			Float64 sampleRate = GetSampleRate();

			if ((now - mPulseStartTime) > (sampleRate * GetParameter (kPulseRestTime))) {
				mDoneClean = 0;
				mWhichMode = kCleanMode;
				mWasSuccessful = false;
				mParentObject->PropertyChanged (kAUPulseMetricsPropertyID, kAudioUnitScope_Global, 0);
				break;
			}

			float pulseThreshold = GetParameter (kPulseThreshold);
			
			for (unsigned int i = 0; i < inFramesToProcess; ++i) 
			{
				Float32 inputSample = inSourceP[i];
				
				if(fabs(inputSample) >= pulseThreshold) {
					mLastMeasurement = UInt32(now + i - mPulseStartTime);
					
					mTotalMeasurements += mLastMeasurement;
					mTotalMeasurementsSquared += pow (mLastMeasurement, 2);
					mNumMeasurements++;
					
					if (mLastMeasurement > mMaxTime)
						mMaxTime = mLastMeasurement;
					if (mLastMeasurement < mMinTime)
						mMinTime = mLastMeasurement;
						
					mDoneClean = 0;
					mWhichMode = kCleanMode;
					
					mLastFrames = inFramesToProcess;
					mWasSuccessful = true;
					mParentObject->DetectedPulse (mPulseStartTime, mLastMeasurement);
					break;
				}				
			}
			memset (inDestP, 0, (inFramesToProcess * sizeof(Float32)));
		}
		break;

		case kCleanMode:
		{
			if (mDoneClean == 0) {
				float secs = GetParameter (kPulseRestTime);
				mDoneClean = SInt32(secs * GetSampleRate());
			}
			
			memset (inDestP, 0, (inFramesToProcess * sizeof(Float32)));
			ioSilence = true;
			mDoneClean -= inFramesToProcess;
			if (mDoneClean <= 0)
				mWhichMode = kEstablishMode;
		}
		break;
		
		case kEstablishMode:
		{
			memset (inDestP, 0, (inFramesToProcess * sizeof(Float32)));
			unsigned int pulseLength = (unsigned int)GetParameter (kPulseLength);
			if (pulseLength > inFramesToProcess)
				pulseLength = inFramesToProcess;

			for (unsigned int i = (inFramesToProcess - pulseLength); i < inFramesToProcess; ++i)
				inDestP[i] = 1.0;
			
			ioSilence = false;
			mWhichMode = kDetectMode;
			mPulseStartTime = SampleTime() + inFramesToProcess - pulseLength;
		}
		break;
	}
}
