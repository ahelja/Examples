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
/*==================================================================================================
	AREngine.cpp

==================================================================================================*/

//==================================================================================================
//	Includes
//==================================================================================================

//	Self Include
#include "AREngine.h"

//	Local Includes
#include "ARDebug.h"
#include "ARDevice.h"

//	System Incluces
#include <libkern/sysctl.h>
#include <IOKit/audio/IOAudioControl.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/audio/IOAudioLevelControl.h>
#include <IOKit/audio/IOAudioSelectorControl.h>
#include <IOKit/audio/IOAudioToggleControl.h>
#include <IOKit/audio/IOAudioTypes.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOTimerEventSource.h>

//==================================================================================================
//	Constants
//==================================================================================================

#define DESCRIPTION_KEY				"Description"
#define BLOCK_SIZE_KEY				"BlockSize"
#define NUM_BLOCKS_KEY				"NumBlocks"
#define NUM_STREAMS_KEY				"NumStreams"
#define FORMATS_KEY					"Formats"
#define SAMPLE_RATES_KEY			"SampleRates"

#define INITIAL_SAMPLE_RATE			44100
#define BLOCK_SIZE					512
#define NUM_BLOCKS					32
#define NUM_STREAMS					1

//==================================================================================================
//	AREngine
//==================================================================================================

OSDefineMetaClassAndStructors(AREngine, IOAudioEngine)

bool	AREngine::init(OSDictionary* inProperties)
{
	bool theAnswer = false;
	
	//	figure out if altivec is present
	UInt32 theValue = 0;
	UInt32 theValueSize = sizeof(UInt32);
	int theError = 1;
#if __ppc__
	theError = sysctlbyname("hw.optional.altivec", &theValue, &theValueSize, NULL, 0);
#elif __i386__
	theError = sysctlbyname("hw.optional.sse3", &theValue, &theValueSize, NULL, 0);
#else
	#error	unknown processor
#endif
	mHasVectorUnit = theError == 0;
	
	if(IOAudioEngine::init(inProperties))
	{
		//	get the number of blocks
		mNumberBlocks = NUM_BLOCKS;
		OSNumber* theOSNumber = OSDynamicCast(OSNumber, getProperty(NUM_BLOCKS_KEY));
		if(theOSNumber != NULL)
		{
			mNumberBlocks = theOSNumber->unsigned32BitValue();
		}
		
		//	get the block size
		mBlockSize = BLOCK_SIZE;
		theOSNumber = OSDynamicCast(OSNumber, getProperty(BLOCK_SIZE_KEY));
		if(theOSNumber != NULL)
		{
			mBlockSize = theOSNumber->unsigned32BitValue();
		}
		
		theAnswer = true;
	}

	return theAnswer;
}

void	AREngine::free()
{
	//	free the input buffer, but only if it was allocated separately
	if((mInputBuffer != NULL) && (mInputBuffer != mOutputBuffer))
	{
		IOFreeAligned(mInputBuffer, mInputBufferSize);
	}
	mInputBuffer = NULL;

	//	free the output buffer
	if(mOutputBuffer != NULL)
	{
		IOFreeAligned(mOutputBuffer, mOutputBufferSize);
		mOutputBuffer = NULL;
	}
	
	//	let the super class do it's work
	IOAudioEngine::free();
}

bool	AREngine::initHardware(IOService* inProvider)
{
	bool theAnswer = false;
	
	if(IOAudioEngine::initHardware(inProvider))
	{
		IOAudioSampleRate theInitialSampleRate = { 0, 0 };
		UInt32 theNumberChannels = 0;
		
		//	create the streams
		if(CreateStreams(&theInitialSampleRate, &theNumberChannels) && (theInitialSampleRate.whole != 0))
		{
			CreateControls(theNumberChannels);
			
			//	figure out how long each block is in microseconds
			mBlockTimeoutMicroseconds = 1000000 * mBlockSize / theInitialSampleRate.whole;
			
			setSampleRate(&theInitialSampleRate);
			
			// Set the number of sample frames in each buffer
			setNumSampleFramesPerBuffer(mBlockSize * mNumberBlocks);
			
			//	set up the timer
			IOWorkLoop* theWorkLoop = getWorkLoop();
			if(theWorkLoop != NULL)
			{
				mTimerEventSource = IOTimerEventSource::timerEventSource(this, TimerFired);
				if(mTimerEventSource != NULL)
				{
					theWorkLoop->addEventSource(mTimerEventSource);
					theAnswer = true;
				}
			}
			
			//	set the safety offset
			//	note that due to cache issues, it probably isn't wise to leave the safety offset at 0,
			//	we set it to 4 here, just to be safe.
			setSampleOffset(4);
			
			//	set up the time stamp generator
			mTimeStampGenerator.SetSampleRate(theInitialSampleRate.whole);
			mTimeStampGenerator.SetFramesPerRingBuffer(mBlockSize * mNumberBlocks);
			
			//	nate that the rate scalar is a 4.28 fixed point number
			//	this means that each incremnt is 1/2^28
			mTimeStampGenerator.SetRateScalar(1UL << 28);
			
			//	set the maximum jitter
//			AbsoluteTime theMaximumJitter = { 0, 0 };
//			nanoseconds_to_absolutetime(5ULL * 1000ULL, &theMaximumJitter);
//			mTimeStampGenerator.SetMaximumJitter(theMaximumJitter.lo);
		}
	}
	
	return theAnswer;
}

bool	AREngine::CreateStreams(IOAudioSampleRate* outInitialSampleRate, UInt32* outNumberChannels)
{
	//	set the return values
	bool theAnswer = true;
	*outNumberChannels = 0;
	
	//	set up some local variables
	OSArray* theFormatArray = NULL;
	OSArray* theSampleRateArray = NULL;
	OSString* theOSString = NULL;
	UInt32 theNumberStreams = NUM_STREAMS;
	OSNumber* theOSNumber = NULL;
	
	//	get the array of formats
	theFormatArray = OSDynamicCast(OSArray, getProperty(FORMATS_KEY));
	FailIfNULLWithAction(theFormatArray, theAnswer = false, Done, "AREngine::CreateStreams: Couldn't get the format array");
	
	//	get the array of sample rates
	theSampleRateArray = OSDynamicCast(OSArray, getProperty(SAMPLE_RATES_KEY));
	FailIfNULLWithAction(theSampleRateArray, theAnswer = false, Done, "AREngine::CreateStreams: Couldn't get the sample rate array");
	
	//	get the description
	theOSString = OSDynamicCast(OSString, getProperty(DESCRIPTION_KEY));
	if(theOSString != NULL)
	{
		setDescription(theOSString->getCStringNoCopy());
	}
	
	//	get the number of streams
	theOSNumber = OSDynamicCast(OSNumber, getProperty(NUM_STREAMS_KEY));
	if(theOSNumber != NULL)
	{
		theNumberStreams = theOSNumber->unsigned32BitValue();
	}
	
	//	make the streams
	for(UInt32 theStreamNumber = 0; theStreamNumber < theNumberStreams; ++theStreamNumber)
	{
		//	initialize some local variables
		bool theResult = false;
		UInt32 theMaxBitWidth = 0;
		UInt32 theMaxNumberChannels = 0;
		IOAudioStream* theInputStream = NULL;
		IOAudioStream* theOutputStream = NULL;
		OSCollectionIterator* theFormatIterator = NULL;
		OSCollectionIterator* theSampleRateIterator = NULL;
		OSDictionary* theFormatDictionary = NULL;
		IOAudioSampleRate theSampleRate = { 0, 0 };
		IOAudioStreamFormat theInitialFormat;
		bool theInitialFormatSet = false;
		char theInputStreamName[32];
		char theOutputStreamName[32];
		UInt32 theStreamBufferSize = 0;

		//	allocate and initialize the input stream
		if(theNumberStreams > 1)
		{
			sprintf(theInputStreamName, "Input Stream #%ld", theStreamNumber + 1);
		}
		else
		{
			sprintf(theInputStreamName, "Input Stream");
		}
		theInputStream = new IOAudioStream;
		FailIfNULLWithAction(theInputStream, theAnswer = false, Error, "AREngine::CreateStreams: couldn't create the input stream");
		theResult = theInputStream->initWithAudioEngine(this, kIOAudioStreamDirectionInput, *outNumberChannels + 1, theInputStreamName);
		FailIfWithAction(!theResult, theAnswer = false, Error, "AREngine::CreateStreams: couldn't initialize the input stream");

		//	allocate and initialize the output stream
		if(theNumberStreams > 1)
		{
			sprintf(theOutputStreamName, "Output Stream #%ld", theStreamNumber + 1);
		}
		else
		{
			sprintf(theOutputStreamName, "Output Stream");
		}
		theOutputStream = new IOAudioStream;
		FailIfNULLWithAction(theOutputStream, theAnswer = false, Error, "AREngine::CreateStreams: couldn't create the output stream");
		theResult = theOutputStream->initWithAudioEngine(this, kIOAudioStreamDirectionOutput, *outNumberChannels + 1, theOutputStreamName);
		FailIfWithAction(!theResult, theAnswer = false, Error, "AREngine::CreateStreams: couldn't initialize the output stream");

		//	make an iterator for the format array
		theFormatIterator = OSCollectionIterator::withCollection(theFormatArray);
		FailIfNULLWithAction(theFormatIterator, theAnswer = false, Error, "AREngine::CreateStreams: couldn't create the format iterator");
		
		//	make an iterator for the sample rate array
		theSampleRateIterator = OSCollectionIterator::withCollection(theSampleRateArray);
		FailIfNULLWithAction(theSampleRateIterator, theAnswer = false, Error, "AREngine::CreateStreams: couldn't create the sample rate iterator");

		//	iterate through the formats
		theFormatIterator->reset();
		theFormatDictionary = (OSDictionary*)theFormatIterator->getNextObject();
		while(theFormatDictionary != NULL)
		{
			//	make sure we have a dictionary
			if(OSDynamicCast(OSDictionary, theFormatDictionary) != NULL)
			{
				//	convert the dictionary into something we can deal with
				IOAudioStreamFormat theFormat;
				FailIfNULLWithAction(IOAudioStream::createFormatFromDictionary(theFormatDictionary, &theFormat), theAnswer = false, Error, "AREngine::CreateStreams: couldn't make a format out of the dictionary");
				
				//	make sure the initial format is set
				if(!theInitialFormatSet)
				{
					theInitialFormat = theFormat;
				}
				
				//	iterate through the sample rates
				theSampleRateIterator->reset();
				theOSNumber = (OSNumber*)theSampleRateIterator->getNextObject();
				while(theOSNumber != NULL)
				{
					//	make sure we have a number
					if(OSDynamicCast(OSNumber, theOSNumber) != NULL)
					{
						//	get the sample rate
						theSampleRate.whole = theOSNumber->unsigned32BitValue();
						
						//	make sure the initial sample rate is set
						if(outInitialSampleRate->whole == 0)
						{
							outInitialSampleRate->whole = theSampleRate.whole;
						}

						//	add the format to the input stream
						theInputStream->addAvailableFormat(&theFormat, &theSampleRate, &theSampleRate);
						
						//	add the format to the output stream
						theOutputStream->addAvailableFormat(&theFormat, &theSampleRate, &theSampleRate);
						
						//	track a few things
						theMaxNumberChannels = (theFormat.fNumChannels > theMaxNumberChannels) ? theFormat.fNumChannels : theMaxNumberChannels;
						theMaxBitWidth = (theFormat.fBitWidth > theMaxBitWidth) ? theFormat.fBitWidth : theMaxBitWidth;
					}
					
					//	go to the next sample rate
					theOSNumber = (OSNumber*)theSampleRateIterator->getNextObject();
				}
			}
			
			//	go to the next format
			theFormatDictionary = (OSDictionary*)theFormatIterator->getNextObject();
		}
		
		//	calculate the size of the stream buffer
		theStreamBufferSize = mBlockSize * mNumberBlocks * theMaxNumberChannels * theMaxBitWidth / 8;
		
		//	allocate the buffers if necessary
		if(mOutputBuffer == NULL)
		{
			//	calculate the size
			mOutputBufferSize = theStreamBufferSize * theNumberStreams;
			
			//	allocate the output buffer
			mOutputBuffer = (void*)IOMallocAligned(mOutputBufferSize, PAGE_SIZE);
			FailIfNULLWithAction(mOutputBuffer, theAnswer = false, Error, "AREngine::CreateStreams: couldn't allocate the output buffer");
			
			//	the input size is the same as the output size
			mInputBufferSize = mOutputBufferSize;
			
			//	allocate the input buffer
			mInputBuffer = mOutputBuffer;
		}
		
		//	set some info about the stream
		theInputStream->setTerminalType(INPUT_UNDEFINED);
		theOutputStream->setTerminalType(OUTPUT_UNDEFINED);
		
		//	set the initial stream formats
		theInputStream->setFormat(&theInitialFormat, false);
		theOutputStream->setFormat(&theInitialFormat, false);
		
		//	set the data buffer for the streams
		theInputStream->setSampleBuffer(&((UInt8*)mInputBuffer)[theStreamBufferSize * theStreamNumber], theStreamBufferSize);
		theOutputStream->setSampleBuffer(&((UInt8*)mOutputBuffer)[theStreamBufferSize * theStreamNumber], theStreamBufferSize);
		
		//	add the streams to the engine
		addAudioStream(theInputStream);
		theInputStream->release();
		theInputStream = NULL;

		addAudioStream(theOutputStream);
		theOutputStream->release();
		theOutputStream = NULL;

		theFormatIterator->release();
		theFormatIterator = NULL;
		
		theSampleRateIterator->release();
		theSampleRateIterator = NULL;

		*outNumberChannels += theMaxNumberChannels;

		continue;

Error:
		if(theInputStream)
		{
			theInputStream->release();
		}

		if(theOutputStream)
		{
			theOutputStream->release();
		}

		if(theFormatIterator)
		{
			theFormatIterator->release();
		}

		if(theSampleRateIterator)
		{
			theSampleRateIterator->release();
		}

		goto Done;
	}
	
Done:
	return theAnswer;
}

void	AREngine::CreateControls(UInt32 inNumberChannels)
{
	for(UInt32 theChannelID = 0; theChannelID <= inNumberChannels; ++theChannelID)
	{
		IOAudioControl* theControl;
		IOAudioSelectorControl* theSelectorControl;
		char theChannelName[32];
		if(theChannelID > 0)
		{
			sprintf(theChannelName, "Channel %lu", theChannelID);
		}
		else
		{
			strcpy(theChannelName, kIOAudioControlChannelNameAll);
		}
		
		//	clock source selector
		if(theChannelID == 0)
		{
			theSelectorControl = IOAudioSelectorControl::create(0, theChannelID, theChannelName, 0, kIOAudioSelectorControlSubTypeClockSource, kIOAudioControlUsageInput);
			if(theSelectorControl != NULL)
			{
				theSelectorControl->addAvailableSelection(0, "Internal");
				theSelectorControl->addAvailableSelection(1, "External 1");
				theSelectorControl->addAvailableSelection(2, "External 2");
				theSelectorControl->addAvailableSelection(3, "External 3");
				theSelectorControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theSelectorControl);
				theSelectorControl->release();
				theSelectorControl = NULL;
			}
		}
		
		//	output volume
		theControl = IOAudioLevelControl::createVolumeControl(65535, 0, 65535, (-22 << 16) + (32768), 0, theChannelID, theChannelName, 0, kIOAudioControlUsageOutput);
		if(theControl != NULL)
		{
			theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
			addDefaultAudioControl(theControl);
			theControl->release();
			theControl = NULL;
		}
		
		//	output mute
		theControl = IOAudioToggleControl::createMuteControl(false, theChannelID, theChannelName, 0, kIOAudioControlUsageOutput);
		if(theControl != NULL)
		{
			theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
			addDefaultAudioControl(theControl);
			theControl->release();
			theControl = NULL;
		}
		
		//	output solo
		if(theChannelID != 0)
		{
			theControl = IOAudioToggleControl::create(false, theChannelID, theChannelName, 0, kIOAudioToggleControlSubTypeSolo, kIOAudioControlUsageOutput);
			if(theControl != NULL)
			{
				theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theControl);
				theControl->release();
				theControl = NULL;
			}
		}
		
		//	output data source
		if(theChannelID != 0)
		{
			theSelectorControl = IOAudioSelectorControl::create(0, theChannelID, theChannelName, 0, kIOAudioSelectorControlSubTypeOutput, kIOAudioControlUsageOutput);
			if(theSelectorControl != NULL)
			{
				theSelectorControl->addAvailableSelection(0, "Source 1");
				theSelectorControl->addAvailableSelection(1, "Source 2");
				theSelectorControl->addAvailableSelection(2, "Source 3");
				theSelectorControl->addAvailableSelection(3, "Source 4");
				theSelectorControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theSelectorControl);
				theSelectorControl->release();
				theSelectorControl = NULL;
			}
		}

		//	output line level
		if(theChannelID != 0)
		{
			theSelectorControl = IOAudioSelectorControl::create(kIOAudioSelectorControlSubTypeChannelLevelMinus10dBV, theChannelID, theChannelName, 0, kIOAudioSelectorControlSubTypeChannelNominalLineLevel, kIOAudioControlUsageOutput);
			if(theSelectorControl != NULL)
			{
				theSelectorControl->addAvailableSelection(kIOAudioSelectorControlSubTypeChannelLevelPlus4dBu, "+4dBu");
				theSelectorControl->addAvailableSelection(kIOAudioSelectorControlSubTypeChannelLevelMinus10dBV, "-10dBV");
				theSelectorControl->addAvailableSelection(kIOAudioSelectorControlSubTypeChannelLevelMinus20dBV, "-20dBV");
				theSelectorControl->addAvailableSelection(kIOAudioSelectorControlSubTypeChannelLevelMicLevel, "Mic");
				theSelectorControl->addAvailableSelection(kIOAudioSelectorControlSubTypeChannelLevelInstrumentLevel, "Instrument");
				theSelectorControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theSelectorControl);
				theSelectorControl->release();
				theSelectorControl = NULL;
			}
		}

		//	input volume
		theControl = IOAudioLevelControl::createVolumeControl(65535, 0, 65535, (-22 << 16) + (32768), 0, theChannelID, theChannelName, 0, kIOAudioControlUsageInput);
		if(theControl != NULL)
		{
			theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
			addDefaultAudioControl(theControl);
			theControl->release();
			theControl = NULL;
		}

		//	input mute
		theControl = IOAudioToggleControl::createMuteControl(false, theChannelID, theChannelName, 0, kIOAudioControlUsageInput);
		if(theControl != NULL)
		{
			theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
			addDefaultAudioControl(theControl);
			theControl->release();
			theControl = NULL;
		}
		
		//	input solo
		if(theChannelID != 0)
		{
			theControl = IOAudioToggleControl::create(false, theChannelID, theChannelName, 0, kIOAudioToggleControlSubTypeSolo, kIOAudioControlUsageInput);
			if(theControl != NULL)
			{
				theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theControl);
				theControl->release();
				theControl = NULL;
			}
		}
		
		//	input data source
		if(theChannelID != 0)
		{
			theSelectorControl = IOAudioSelectorControl::create(0, theChannelID, theChannelName, 0, kIOAudioSelectorControlSubTypeInput, kIOAudioControlUsageInput);
			if(theSelectorControl != NULL)
			{
				theSelectorControl->addAvailableSelection(0, "Source 1");
				theSelectorControl->addAvailableSelection(1, "Source 2");
				theSelectorControl->addAvailableSelection(2, "Source 3");
				theSelectorControl->addAvailableSelection(3, "Source 4");
				theSelectorControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theSelectorControl);
				theSelectorControl->release();
				theSelectorControl = NULL;
			}
		}

		//	input line level
		if(theChannelID != 0)
		{
			theSelectorControl = IOAudioSelectorControl::create(kIOAudioSelectorControlSubTypeChannelLevelMinus10dBV, theChannelID, theChannelName, 0, kIOAudioSelectorControlSubTypeChannelNominalLineLevel, kIOAudioControlUsageInput);
			if(theSelectorControl != NULL)
			{
				theSelectorControl->addAvailableSelection(kIOAudioSelectorControlSubTypeChannelLevelPlus4dBu, "+4dBu");
				theSelectorControl->addAvailableSelection(kIOAudioSelectorControlSubTypeChannelLevelMinus10dBV, "-10dBV");
				theSelectorControl->addAvailableSelection(kIOAudioSelectorControlSubTypeChannelLevelMinus20dBV, "-20dBV");
				theSelectorControl->addAvailableSelection(kIOAudioSelectorControlSubTypeChannelLevelMicLevel, "Mic");
				theSelectorControl->addAvailableSelection(kIOAudioSelectorControlSubTypeChannelLevelInstrumentLevel, "Instrument");
				theSelectorControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theSelectorControl);
				theSelectorControl->release();
				theSelectorControl = NULL;
			}
		}

		//	play through volume
		theControl = IOAudioLevelControl::createVolumeControl(65535, 0, 65535, (-22 << 16) + (32768), 0, theChannelID, theChannelName, 0, kIOAudioControlUsagePassThru);
		if(theControl != NULL)
		{
			theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
			addDefaultAudioControl(theControl);
			theControl->release();
			theControl = NULL;
		}

		//	play through on/off
		theControl = IOAudioToggleControl::createMuteControl(true, theChannelID, theChannelName, 0, kIOAudioControlUsagePassThru);
		if(theControl != NULL)
		{
			theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
			addDefaultAudioControl(theControl);
			theControl->release();
			theControl = NULL;
		}
		
		//	play through solo
		if(theChannelID != 0)
		{
			theControl = IOAudioToggleControl::create(false, theChannelID, theChannelName, 0, kIOAudioToggleControlSubTypeSolo, kIOAudioControlUsagePassThru);
			if(theControl != NULL)
			{
				theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theControl);
				theControl->release();
				theControl = NULL;
			}
		}
		
		//	play through data destination
		if(theChannelID != 0)
		{
			theSelectorControl = IOAudioSelectorControl::create(0, theChannelID, theChannelName, 0, kIOAudioSelectorControlSubTypeDestination, kIOAudioControlUsagePassThru);
			if(theSelectorControl != NULL)
			{
				theSelectorControl->addAvailableSelection(0, "Destination 1");
				theSelectorControl->addAvailableSelection(1, "Destination 2");
				theSelectorControl->addAvailableSelection(2, "Destination 3");
				theSelectorControl->addAvailableSelection(3, "Destination 4");
				theSelectorControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theSelectorControl);
				theSelectorControl->release();
				theSelectorControl = NULL;
			}
		}

		//	LFE volume
		if(theChannelID == 0)
		{
			theControl = IOAudioLevelControl::create(65535, 0, 65535, (-22 << 16) + (32768), 0, theChannelID, theChannelName, 0, kIOAudioLevelControlSubTypeLFEVolume, kIOAudioControlUsageOutput);
			if(theControl != NULL)
			{
				theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theControl);
				theControl->release();
				theControl = NULL;
			}
		}

		//	LFE mute
		if(theChannelID == 0)
		{
			theControl = IOAudioToggleControl::create(false, theChannelID, theChannelName, 0, kIOAudioToggleControlSubTypeLFEMute, kIOAudioControlUsageOutput);
			if(theControl != NULL)
			{
				theControl->setValueChangeHandler((IOAudioControl::IntValueChangeHandler)IntegerControlChangeHandler, this);
				addDefaultAudioControl(theControl);
				theControl->release();
				theControl = NULL;
			}
		}
	}
}

OSString*	AREngine::getGlobalUniqueID()
{
    return OSString::withCString("Audio_Reflector");
}

IOReturn	AREngine::performAudioEngineStart()
{
	//	reset the time stamp generator
	mTimeStampGenerator.Reset();
	
	//	set the current block to the last one before the wrap around
	mCurrentBlock = mNumberBlocks - 1;
	
	//	start the timer, the first time stamp will be taken when it goes off
	mTimerEventSource->setTimeoutUS(mBlockTimeoutMicroseconds);
	
	return kIOReturnSuccess;
}

IOReturn	AREngine::performAudioEngineStop()
{
	mTimerEventSource->cancelTimeout();
	
	return kIOReturnSuccess;
}

UInt32	AREngine::getCurrentSampleFrame()
{
	return mCurrentBlock * mBlockSize;
}

IOReturn	AREngine::performFormatChange(IOAudioStream* inStream, const IOAudioStreamFormat* inNewFormat, const IOAudioSampleRate* inNewSampleRate)
{
	//	set up the time stamp generator
	if(inNewSampleRate != NULL)
	{
		mTimeStampGenerator.SetSampleRate(inNewSampleRate->whole);
	}

	//	When the format of a stream changes, we have to synch the corresponding stream in the other direction
	//	so that the reflection works without munging channels.
	if(inNewFormat != NULL)
	{
		//	we're going to change a lot of stuff, including controls
		beginConfigurationChange();
		
		//	get the starting channel for the stream
		UInt32 theStartingChannel = inStream->getStartingChannelID();
		
		//	get the direction for the stream
		IOAudioStreamDirection theDirection = inStream->getDirection();
		
		//	flip the direction
		theDirection = (theDirection == kIOAudioStreamDirectionOutput) ? kIOAudioStreamDirectionInput : kIOAudioStreamDirectionOutput;
		
		//	look up the stream in the opposite direction
		IOAudioStream* theOppositeStream = getAudioStream(theDirection, theStartingChannel);
		
		//	tell the opposite stream to change format too
		if(theOppositeStream != NULL)
		{
			theOppositeStream->setFormat(inNewFormat, false);
		}
		
		//	we're done changing stuff
		completeConfigurationChange();
	}
	
	return kIOReturnSuccess;
}

IOReturn	AREngine::IntegerControlChangeHandler(IOService* inTarget, IOAudioControl* inControl, SInt32 inOldValue, SInt32 inNewValue)
{
	IOReturn theAnswer = kIOReturnBadArgument;
	
	//	validate the engine
	AREngine* theEngine = OSDynamicCast(AREngine, inTarget);
	if(theEngine != NULL)
	{
		//	validate the control
		if(inControl != NULL)
		{
			//	figure out what kind of control we have here

			//	dispatch to the appropriate handler
			theAnswer = kIOReturnSuccess;
		}
	}
	
	return theAnswer;
}

void	AREngine::TimerFired(OSObject* inTarget, IOTimerEventSource* inSender)
{
	//	validate the engine
	AREngine* theEngine = OSDynamicCast(AREngine, inTarget);
	if(theEngine != NULL)
	{
		//	go to the next block
		theEngine->mCurrentBlock += 1;
		
		//	check to see if we have wrapped around
		if(theEngine->mCurrentBlock >= theEngine->mNumberBlocks)
		{
			//	we have
			theEngine->mCurrentBlock = 0;
			
			//	get the next time stamp
			AbsoluteTime theTimeStamp = { 0, 0 };
			theEngine->mTimeStampGenerator.GetNextTimeStamp(theEngine->status->fCurrentLoopCount, &theTimeStamp);
			
			//	tell the family about it
			theEngine->takeTimeStamp(true, &theTimeStamp);
		}
		
		//	set the timer to go off in one block
		inSender->setTimeoutUS(theEngine->mBlockTimeoutMicroseconds);
	}
}
