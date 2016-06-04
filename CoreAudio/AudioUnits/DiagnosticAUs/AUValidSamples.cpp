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
	AUValidSamples.cpp
	
=============================================================================*/
#include "AUEffectBase.h"
#include "AUValidSamplesVersion.h"
#include "CAHostTimeBase.h"

#include "AUValidSamplesShared.h"

template <class ITEM>
class LockFreeFIFO
{
public:
	LockFreeFIFO(UInt32 inMaxSize)
		: mReadIndex(0), mWriteIndex(0)
	{
		//assert(IsPowerOfTwo(inMaxSize));
		mItems = new ITEM[inMaxSize];
		mMask = inMaxSize - 1;
	}
	
	~LockFreeFIFO()
	{
		delete [] mItems;
	}
	
	ITEM* WriteItem()
	{
		UInt32 nextWriteIndex = (mWriteIndex + 1) & mMask;
		if (nextWriteIndex == mReadIndex) return NULL;
		return &mItems[mWriteIndex];
	}
	
	ITEM* ReadItem()
	{
		if (mReadIndex == mWriteIndex) return NULL;
		return &mItems[mReadIndex];
	}
	
		// the CompareAndSwap will always succeed. We use CompareAndSwap because it calls the PowerPC sync instruction,
		// plus any processor bug workarounds for various CPUs.
	void AdvanceWritePtr() { CompareAndSwap(mWriteIndex, (mWriteIndex + 1) & mMask, (UInt32*)&mWriteIndex); }
	void AdvanceReadPtr()  { CompareAndSwap(mReadIndex,  (mReadIndex  + 1) & mMask, (UInt32*)&mReadIndex); }
	
private:
	
	volatile UInt32 mReadIndex, mWriteIndex;
	UInt32 mMask;
	ITEM *mItems;
	
	LockFreeFIFO()
		: mReadIndex(0), mWriteIndex(0), mItems(NULL) {}
};


/*
	ITEM *item = fifo->WriteItem(); // get pointer to item in queue.
	if (item)
	{
		// ...put stuff in item...
		fifo->AdvanceWritePtr();
	}

...

	ITEM *item;
	while ((item = fifo->ReadItem()) != NULL)
	{
		// ...get stuff from item...
		fifo->AdvanceReadPtr();
	}
*/

#if AU_DEBUG_DISPATCHER
	#include "AUDebugDispatcher.h"
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AUValidSamples

enum {
	kZapInvalidValues = 0,
	kDoReport = 1,
	kValidNominalRange	= 2,
};

const static int kDoReportDefault = 1;
const static int kDoReportMin = 0;
const static int kDoReportMax = 1;

const static int kValidNominalRangeDefault = 0;
const static int kValidNominalRangeMin = 0;
const static int kValidNominalRangeMax = 1;

const static int kZapInvalidValuesDefault = 1;
const static int kZapInvalidValuesMin = 0;
const static int kZapInvalidValuesMax = 1;

const static CFStringRef kDoReportParamName = CFSTR("Report Invalid Samples");
const static CFStringRef kValidNominalRangeParamName = CFSTR("Report Samples Outside Range (-1 < 1)");
const static CFStringRef kZapInvalidValuesParamName = CFSTR("Zap Invalid Values");

//static bool sLocalized = false;

static const int kMaxNumInfo = 64;

class AUValidSamples : public AUEffectBase
{
public:
								AUValidSamples(AudioUnit component);
								virtual ~AUValidSamples ();
								
	virtual AUKernelBase *		NewKernel() { return new AUValidSamplesKernel(this); }

	virtual ComponentResult		Initialize();
		
	virtual	bool				SupportsTail () { return true; }

	virtual	ComponentResult		GetParameterInfo(	AudioUnitScope			inScope,
                                                    AudioUnitParameterID	inParameterID,
													AudioUnitParameterInfo	&outParameterInfo	);
	
	virtual ComponentResult		GetPropertyInfo (AudioUnitPropertyID			inID,
												AudioUnitScope					inScope,
												AudioUnitElement				inElement,
												UInt32 &						outDataSize,
												Boolean &						outWritable)
	{
		if (inScope == kAudioUnitScope_Global) {
			switch (inID) {
				case kAUValidSamples_InvalidSamplesPropertyID:
					outDataSize = sizeof (VSInfo) * kMaxNumInfo + sizeof(UInt32);
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

	virtual int					GetNumCustomUIComponents () { return 1; }

	virtual void				GetUIComponentDescs (ComponentDescription* inDescArray)
	{
		ComponentDescription desc;
		desc.componentType = kAudioUnitCarbonViewComponentType;
		desc.componentSubType = 'vsmp';
		desc.componentManufacturer = 'appl';
		desc.componentFlags = 0;
		desc.componentFlagsMask = 0;  
		*inDescArray = desc;
	}

	virtual ComponentResult	Version() { return kAUValidSamplesVersion; }

	
protected:
	class AUValidSamplesKernel : public AUKernelBase		// most real work happens here
	{
	public:
		AUValidSamplesKernel(AUValidSamples *inAudioUnit )
			: AUKernelBase(inAudioUnit),
			  mParent (*inAudioUnit)
		{
		}

		virtual ~AUValidSamplesKernel() {}
		
// Required overides for the process method for this effect
		// processes one channel of interleaved samples
		virtual void 		Process(	const Float32 	*inSourceP,
										Float32		 	*inDestP,
										UInt32 			inFramesToProcess,
										UInt32			inNumChannels,
                                        bool			&ioSilence);
		
		virtual void		Reset ()
		{
		}
		
	private:
		AUValidSamples &mParent; 
	};

	LockFreeFIFO<VSInfo> 	*mSampleInfo;
	Float64					mLastTime;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

COMPONENT_ENTRY(AUValidSamples)

#pragma mark ____AUValidSamples

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUValidSamples::AUValidSamples
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUValidSamples::AUValidSamples(AudioUnit component)
	: AUEffectBase(component),
	  mSampleInfo(NULL),
	  mLastTime(0)
{
	CreateElements();
		
		//load code... by calling fabs
	Globals()->UseIndexedParameters ((int)fabs(3));
	Globals()->SetParameter (kDoReport, kDoReportDefault);
	Globals()->SetParameter (kZapInvalidValues, kZapInvalidValuesDefault);
	Globals()->SetParameter (kValidNominalRange, kValidNominalRangeDefault);
	
#if AU_DEBUG_DISPATCHER
	mDebugDispatcher = new AUDebugDispatcher (this);
#endif
}

AUValidSamples::~AUValidSamples ()
{
#if AU_DEBUG_DISPATCHER
	delete mDebugDispatcher;
#endif
	delete mSampleInfo;
}

ComponentResult		AUValidSamples::Initialize()
{
	ComponentResult result = AUEffectBase::Initialize();
	if (result) return result;

	delete mSampleInfo;
	mSampleInfo = new LockFreeFIFO<VSInfo> (GetMaxFramesPerSlice() * GetNumberOfChannels());
		//load code...
	mLastTime = CAHostTimeBase::GetCurrentTimeInNanos() * 0.000000001;
	return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUPulseDetector::GetParameterInfo
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult		AUValidSamples::GetParameterInfo(	AudioUnitScope			inScope,
                                                        AudioUnitParameterID	inParameterID,
                                                        AudioUnitParameterInfo	&outParameterInfo )
{
	outParameterInfo.flags = 	kAudioUnitParameterFlag_IsWritable
						+		kAudioUnitParameterFlag_IsReadable;
    
    if (inScope == kAudioUnitScope_Global) 
	{
        switch(inParameterID)
        {
            case kDoReport:
                AUBase::FillInParameterName (outParameterInfo, kDoReportParamName, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
				outParameterInfo.minValue = kDoReportMin;
				outParameterInfo.maxValue = kDoReportMax;
				outParameterInfo.defaultValue = kDoReportDefault;
				return noErr;

            case kValidNominalRange:
                AUBase::FillInParameterName (outParameterInfo, kValidNominalRangeParamName, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
				outParameterInfo.minValue = kValidNominalRangeMin;
				outParameterInfo.maxValue = kValidNominalRangeMax;
				outParameterInfo.defaultValue = kValidNominalRangeDefault;
				return noErr;

            case kZapInvalidValues:
                AUBase::FillInParameterName (outParameterInfo, kZapInvalidValuesParamName, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
				outParameterInfo.minValue = kZapInvalidValuesMin;
				outParameterInfo.maxValue = kZapInvalidValuesMax;
				outParameterInfo.defaultValue = kZapInvalidValuesDefault;
				return noErr;
		}
	}

	return AUEffectBase::GetParameterInfo (inScope, inParameterID, outParameterInfo);
}


ComponentResult		AUValidSamples::GetProperty(	AudioUnitPropertyID inID,
													AudioUnitScope 		inScope,
													AudioUnitElement 	inElement,
													void *				outData )
{
	if (inScope == kAudioUnitScope_Global) 
	{
		switch (inID) {
			case kAUValidSamples_InvalidSamplesPropertyID:
			{
				VSInfoList *list = (VSInfoList*)(outData);				
				VSInfo *item;
				int i = 0;
				int temp;
				while ((item = mSampleInfo->ReadItem()) != NULL) {
					memcpy (&list->data[i], item, sizeof(VSInfo));
					mSampleInfo->AdvanceReadPtr();
					temp = ++i % kMaxNumInfo;
					i = temp;
				}
				list->numEntries = i;
			}
			return noErr;
			
			default: break;
		}
	}
	return AUEffectBase::GetProperty (inID, inScope, inElement, outData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AUValidSamplesKernel
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUValidSamples::AUValidSamples::Process
//
//		delay samples from input to specified outputs
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AUValidSamples::AUValidSamplesKernel::Process(const Float32 	*inSourceP,
											Float32		 			*inDestP,
											UInt32 					inFramesToProcess,
											UInt32					inNumChannels,
											bool					&ioSilence )
{
	long nSampleFrames = inFramesToProcess;
	const float *sourceP = inSourceP;
	float *destP = inDestP;
	
	bool zap = (bool)GetParameter(kZapInvalidValues); 
	bool badRange = (bool)GetParameter(kValidNominalRange);
	bool doReport = (bool)GetParameter(kDoReport);
	bool badNumDetected = false;
	
	while (nSampleFrames-- > 0)
	{
		float  input = *sourceP;
		sourceP += inNumChannels;
		
#if 0
		static int counter = 0;
		if (((++counter % 44100) == 0))
			input = 1e-16;//0./0.;
#endif
				
		float absx = fabs(input);
		
				// a bad number!
		if (!(absx > 1e-15 && absx < 1e15))
		{
			if (!(absx == 0))
			{
				if (doReport) {
					VSInfo* item = mParent.mSampleInfo->WriteItem();
					badNumDetected = true;
					if (item) {
						item->sample = inFramesToProcess - nSampleFrames - 1;
						item->value = input;
						item->channel = GetChannelNum ();
						mParent.mSampleInfo->AdvanceWritePtr();
					}
				}
				if (zap)
					input = 0;
			}
		} 
		else if (badRange && !(input < 1.0 && input >= -1.0)) 
		{
			if (doReport) 
			{
				VSInfo* item = mParent.mSampleInfo->WriteItem();
				badNumDetected = true;
				if (item) {
					item->sample = inFramesToProcess - nSampleFrames - 1;
					item->value = input;
					item->channel = GetChannelNum ();
					mParent.mSampleInfo->AdvanceWritePtr();
				}
			}
			if (zap) {
				if (input < -1.0)
					input = -1.;
				else
					input = 0.9999999f;
			}
		}
		
		// output		
		*destP = input;
		destP += inNumChannels;
	}

	if (badNumDetected) {
			// we have to throttle this or its bad
		Float64 secs = CAHostTimeBase::GetCurrentTimeInNanos() * 0.000000001;
		Float64 then = mParent.mLastTime + 0.1;
		if (secs > then) { // add 100msecs to lastTime
			mParent.PropertyChanged (kAUValidSamples_InvalidSamplesPropertyID, kAudioUnitScope_Global, 0);
			mParent.mLastTime = secs;
		}
	}
}
