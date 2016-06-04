/*	Copyright: 	й Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple╒s
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
	ACAppleIMA4Decoder.cpp

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

#include "ACAppleIMA4Decoder.h"
#include "ACCodecDispatch.h"
#include "CAStreamBasicDescription.h"
#include "CASampleTools.h"
#include "CADebugMacros.h"
#include "CABundleLocker.h"

#if TARGET_OS_WIN32
	#include "CAWin32StringResources.h"
#endif

//=============================================================================
//	ACAppleIMA4Decoder
//=============================================================================

ACAppleIMA4Decoder::ACAppleIMA4Decoder()
:
	ACAppleIMA4Codec(kInputBufferPackets * kIMA4PacketBytes)
{
	//еее	One issue to talk about here is how do we represent the fact that this
	//еее	decoder doesn't care about the number of channels or the sample rate?
	//еее	For now, I've implemented it by specifying 0 for the values that don't
	//еее	matter. The issue is that 0 can also mean that the value is unknown or
	//еее	doesn't apply.
	//еее
	//еее	Below are examples of both usages, in both the available input and output
	//еее	formats. Since the number of channels is 0 (because this decoder doesn't
	//еее	care), a lot of the dependant values, like bytes per frame, are unknowable.
	//еее
	//еее	The reason why this is an issue is that code that tries to figure out how
	//еее	to hook this decoder up to other converters is going to have to be very
	//еее	careful about being strict enough with format matching that it can always
	//еее	be correct, but not too strict that it treats the situations thus created
	//еее	as bad matches or, worse yet, an error.

	//	This decoder only takes an Apple IMA4 stream as it's input
	CAStreamBasicDescription theInputFormat(kAudioStreamAnyRate, kAudioFormatAppleIMA4, kIMA4PacketBytes, kFramesPerPacket, 0, 0, 0, 0);
	AddInputFormat(theInputFormat);

	//	set our initial input format to mono Apple IMA4 at a 44100 sample rate
	mInputFormat.mSampleRate = 44100;
	mInputFormat.mFormatID = kAudioFormatAppleIMA4;
	mInputFormat.mFormatFlags = 0;
	mInputFormat.mBytesPerPacket = kIMA4PacketBytes;
	mInputFormat.mFramesPerPacket = kFramesPerPacket;
	mInputFormat.mBytesPerFrame = 0;
	mInputFormat.mChannelsPerFrame = 1;
	mInputFormat.mBitsPerChannel = 0;
	
	//	This decoder produces either 16 bit native endian signed integer or
	//	32 bit native endian Core Audio floats as it's output,
	//	but can handle any sample rate and any number of channels
	CAStreamBasicDescription theOutputFormat1(kAudioStreamAnyRate, kAudioFormatLinearPCM, 0, 1, 0, 0, 16, kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked);
	AddOutputFormat(theOutputFormat1);

	CAStreamBasicDescription theOutputFormat2(kAudioStreamAnyRate, kAudioFormatLinearPCM, 0, 1, 0, 0, 32, kAudioFormatFlagsNativeFloatPacked);
	AddOutputFormat(theOutputFormat2);
	
	//	set our intial output format to mono 32 bit native endian Core Audio floats at a 44100 sample rate
	mOutputFormat.mSampleRate = 44100;
	mOutputFormat.mFormatID = kAudioFormatLinearPCM;
	mOutputFormat.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	mOutputFormat.mBytesPerPacket = 4;
	mOutputFormat.mFramesPerPacket = 1;
	mOutputFormat.mBytesPerFrame = 4;
	mOutputFormat.mChannelsPerFrame = 1;
	mOutputFormat.mBitsPerChannel = 32;
	
	//	initialize our channel state
	InitializeChannelStateList();
}

ACAppleIMA4Decoder::~ACAppleIMA4Decoder()
{
}

void	ACAppleIMA4Decoder::GetProperty(AudioCodecPropertyID inPropertyID, UInt32& ioPropertyDataSize, void* outPropertyData)
{	
	switch(inPropertyID)
	{
		case kAudioCodecPropertyNameCFString:
		{
			if (ioPropertyDataSize != sizeof(CFStringRef)) CODEC_THROW(kAudioCodecBadPropertySizeError);
			
			CABundleLocker lock;
			CFStringRef name = CFCopyLocalizedStringFromTableInBundle(CFSTR("Apple IMA4 decoder"), CFSTR("CodecNames"), GetCodecBundle(), CFSTR(""));
			*(CFStringRef*)outPropertyData = name;
			break; 
		}
		default:
			ACAppleIMA4Codec::GetProperty(inPropertyID, ioPropertyDataSize, outPropertyData);
	}
}

void	ACAppleIMA4Decoder::SetCurrentInputFormat(const AudioStreamBasicDescription& inInputFormat)
{
	if(!mIsInitialized)
	{
		//	check to make sure the input format is legal
		if(inInputFormat.mFormatID != kAudioFormatAppleIMA4)
		{
#if VERBOSE
			DebugMessage("ACAppleIMA4Decoder::SetFormats: only support Apple IMA for output");
#endif
			CODEC_THROW(kAudioCodecUnsupportedFormatError);
		}
		
		//	tell our base class about the new format
		ACAppleIMA4Codec::SetCurrentInputFormat(inInputFormat);
	}
	else
	{
		CODEC_THROW(kAudioCodecStateError);
	}
}

void	ACAppleIMA4Decoder::SetCurrentOutputFormat(const AudioStreamBasicDescription& inOutputFormat)
{
	if(!mIsInitialized)
	{
		//	check to make sure the output format is legal
		if(	(inOutputFormat.mFormatID != kAudioFormatLinearPCM) ||
			!( ( (inOutputFormat.mFormatFlags == kAudioFormatFlagsNativeFloatPacked) &&
			     (inOutputFormat.mBitsPerChannel == 32) ) ||
			   ( (inOutputFormat.mFormatFlags == (kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked) ) &&
			     (inOutputFormat.mBitsPerChannel == 16) ) ) )
		{
#if VERBOSE
			DebugMessage("ACAppleIMA4Decoder::SetFormats: only supports either 16 bit native endian signed integer or 32 bit native endian Core Audio floats for output");
#endif
			CODEC_THROW(kAudioCodecUnsupportedFormatError);
		}
		
		//	tell our base class about the new format
		ACAppleIMA4Codec::SetCurrentOutputFormat(inOutputFormat);
	}
	else
	{
		CODEC_THROW(kAudioCodecStateError);
	}
}

UInt32	ACAppleIMA4Decoder::ProduceOutputPackets(void* outOutputData, UInt32& ioOutputDataByteSize, UInt32& ioNumberPackets, AudioStreamPacketDescription* outPacketDescription)
{
	//	setup the return value, by assuming that everything is going to work
	UInt32 theAnswer = kAudioCodecProduceOutputPacketSuccess;
	
	if(!mIsInitialized)
		CODEC_THROW(kAudioCodecStateError);
	
	//	Note that the decoder doesn't suffer from the same problem the encoder
	//	does with not having enough data for a packet, since the encoded data
	//	is always going to be in whole packets.
	
	//	clamp the number of packets to produce based on what is available in the input buffer
	UInt32 inputPacketSize = mInputFormat.mChannelsPerFrame * kIMA4PacketBytes;
	UInt32 numberOfInputPackets = GetUsedInputBufferByteSize() / inputPacketSize;
	if (ioNumberPackets < numberOfInputPackets)
	{
		numberOfInputPackets = ioNumberPackets;
	}
	else if (ioNumberPackets > numberOfInputPackets)
	{
		ioNumberPackets = numberOfInputPackets;
		
		//	this also means we need more input to satisfy the request so set the return value
		theAnswer = kAudioCodecProduceOutputPacketNeedsMoreInputData;
	}
	
	UInt32 inputByteSize = numberOfInputPackets * inputPacketSize;
	
	if(ioNumberPackets > 0)
	{
		//	make sure that there is enough space in the output buffer for the encoded data
		//	it is an error to ask for more output than you pass in buffer space for
		UInt32	theOutputByteSize = ioNumberPackets * kFramesPerPacket * mOutputFormat.mBytesPerFrame;
		ThrowIf(ioOutputDataByteSize < theOutputByteSize, static_cast<ComponentResult>(kAudioCodecNotEnoughBufferSpaceError), "ACAppleIMA4Decoder::ProduceOutputPackets: not enough space in the output buffer");
		
		//	set the return value
		ioOutputDataByteSize = theOutputByteSize;
		
		//	decode the input data for each channel
		Byte* theInputData = GetBytes(inputByteSize);
		if (mOutputFormat.mBitsPerChannel == 16)
		{
			SInt16* theOutputData = reinterpret_cast<SInt16*>(outOutputData);
			ChannelStateList::iterator theIterator = mChannelStateList.begin();
			for(UInt32 theChannelIndex = 0; theChannelIndex < mOutputFormat.mChannelsPerFrame; ++theChannelIndex)
			{
				//printf("->DecodeChannel %d %d %d %08X %08X\n", mOutputFormat.mChannelsPerFrame, theChannelIndex, ioNumberPackets, theInputData, theOutputData);
				DecodeChannelSInt16(*theIterator, mOutputFormat.mChannelsPerFrame, theChannelIndex, ioNumberPackets, theInputData, theOutputData);
				std::advance(theIterator, 1);
			}
			ConsumeInputData(inputByteSize);
		}
		else
		{
			float* theOutputData = reinterpret_cast<float*>(outOutputData);
			ChannelStateList::iterator theIterator = mChannelStateList.begin();
			for(UInt32 theChannelIndex = 0; theChannelIndex < mOutputFormat.mChannelsPerFrame; ++theChannelIndex)
			{
				//printf("->DecodeChannel %d %d %d %08X %08X\n", mOutputFormat.mChannelsPerFrame, theChannelIndex, ioNumberPackets, theInputData, theOutputData);
				DecodeChannelCAFloat(*theIterator, mOutputFormat.mChannelsPerFrame, theChannelIndex, ioNumberPackets, theInputData, theOutputData);
				std::advance(theIterator, 1);
			}
			ConsumeInputData(inputByteSize);
			//printf("inputByteSize == %lu, ioOutputDataByteSize = %lu\n", inputByteSize, ioOutputDataByteSize);
		}
	}
	else
	{
		//	set the return value since we're not actually doing any work
		ioOutputDataByteSize = 0;
	}
	
	if((theAnswer == kAudioCodecProduceOutputPacketSuccess) && (GetUsedInputBufferByteSize() >= inputPacketSize))
	{
		//	we satisfied the request, and there's at least one more full packet of data we can decode
		//	so set the return value
		theAnswer = kAudioCodecProduceOutputPacketSuccessHasMore;
	}
		
	return theAnswer;
}


void	ACAppleIMA4Decoder::DecodeChannelSInt16(ChannelState& ioChannelState, UInt32 inNumberChannels, UInt32 inDecodeChannel, UInt32 inNumberPacketsToDecode, const Byte* inInputData, SInt16* outOutputData)
{
	//	This decoder can only decode one channel at a time.
	//	Each channel in a packet of frames is encoded separately and
	//	the resulting channel packets are interleaved in channel order.

	//	We need to figure out how to skip through the input and output buffers
	//	and point at the appropriate place in the data to start off
	UInt32	theInputStride	= (inNumberChannels - 1) * kIMA4PacketBytes + 2;
	Byte*	theInputData	= const_cast<Byte*>(inInputData) + (inDecodeChannel * kIMA4PacketBytes);
	UInt32	theOutputStride	= inNumberChannels;

	SInt32 theDifference;
	SInt32 theCode = 0;
	UInt32 theTemporaryInputData = 0;					/* initialize so warnings go away */

	SInt16*	theOutputData	= const_cast<SInt16*>(outOutputData) + inDecodeChannel;

	if (inNumberPacketsToDecode == 0)
		return;

	CheckState(theInputData, ioChannelState);						/* make sure state predictors match stream */
	theInputData += 2;										/* skip first predictor */

	SInt32 thePredictedSample = ioChannelState.mPredictedSample;					/* continue where we left off last time */
	SInt32	theStepTableIndex = ioChannelState.mStepTableIndex;
	SInt32	theStep = sStepTable[theStepTableIndex];

	for (; inNumberPacketsToDecode > 0; --inNumberPacketsToDecode)
	{
		//printf("inNumberPacketsToDecode %d\n", inNumberPacketsToDecode);
		for(UInt32 theNumberSamplesLeft = kFramesPerPacket; theNumberSamplesLeft > 0; --theNumberSamplesLeft)
		{
			if	(theNumberSamplesLeft & 1)						/* two samples per input char */
				theCode = theTemporaryInputData >> 4;
			else
			{
				//printf("IN %02X\n", *theInputData & 255);
				theTemporaryInputData = *theInputData++;					/* buffer two ADPCM nibbles */
				theCode = theTemporaryInputData & 0x0F;
			}
			
			theDifference = 0;								/* compute new sample estimate thePredictedSample */
			if (theCode & 4)
				theDifference += theStep;
			if (theCode & 2)
				theDifference += theStep >> 1;
			if (theCode & 1)
				theDifference += theStep >> 2;
				
			theDifference += theStep >> 3;
			if (theCode & 8)
				theDifference = -theDifference;

			thePredictedSample += theDifference;

			//	check for overflow
			if(thePredictedSample > 32767)
			{
				thePredictedSample = 32767;
			}
			else if(thePredictedSample < -32768)
			{
				thePredictedSample = -32768;
			}

			//printf("   theCode %02X  theDifference %d\n", theCode, theDifference);
			//printf("OUT %02X %d   \n", thePredictedSample & 255, thePredictedSample);

			*theOutputData = thePredictedSample;
			theOutputData += theOutputStride;
			
			theStepTableIndex += sIndexTable[theCode];				/* compute new stepsize step */
			if (theStepTableIndex < 0)
				theStepTableIndex = 0;
			else if (theStepTableIndex > 88)
				theStepTableIndex = 88;
			theStep = sStepTable[theStepTableIndex];
			//printf("   theStep %d  theStepTableIndex %d\n", theStep, theStepTableIndex);
		}

		theInputData += theInputStride;
	}

	ioChannelState.mPredictedSample = thePredictedSample;
	ioChannelState.mStepTableIndex = theStepTableIndex;
}

void	ACAppleIMA4Decoder::DecodeChannelCAFloat(ChannelState& ioChannelState, UInt32 inNumberChannels, UInt32 inDecodeChannel, UInt32 inNumberPacketsToDecode, const Byte* inInputData, float* outOutputData)
{
	//	This decoder can only decode one channel at a time.
	//	Each channel in a packet of frames is encoded separately and
	//	the resulting channel packets are interleaved in channel order.

	//	We need to figure out how to skip through the input and output buffers
	//	and point at the appropriate place in the data to start off
	UInt32	theInputStride	= (inNumberChannels - 1) * kIMA4PacketBytes + 2;
	Byte*	theInputData	= const_cast<Byte*>(inInputData) + (inDecodeChannel * kIMA4PacketBytes);
	UInt32	theOutputStride	= inNumberChannels;

	SInt32 theDifference;
	SInt32 theCode = 0;
	UInt32 theTemporaryInputData = 0;					/* initialize so warnings go away */
	float * tempFloatPtr;
	unsigned long tempULong = 0;
	float tempFloat;

	float*	theOutputData	= const_cast<float*>(outOutputData) + inDecodeChannel;

	if (inNumberPacketsToDecode == 0)
		return;

	CheckState(theInputData, ioChannelState);						/* make sure state predictors match stream */
	theInputData += 2;										/* skip first predictor */

	SInt32 thePredictedSample = ioChannelState.mPredictedSample;					/* continue where we left off last time */
	SInt32	theStepTableIndex = ioChannelState.mStepTableIndex;
	SInt32	theStep = sStepTable[theStepTableIndex];

	for (; inNumberPacketsToDecode > 0; --inNumberPacketsToDecode)
	{
		//printf("inNumberPacketsToDecode %d\n", inNumberPacketsToDecode);
		for(UInt32 theNumberSamplesLeft = kFramesPerPacket; theNumberSamplesLeft > 0; --theNumberSamplesLeft)
		{
			if	(theNumberSamplesLeft & 1)						/* two samples per input char */
				theCode = theTemporaryInputData >> 4;
			else
			{
				//printf("IN %02X\n", *theInputData & 255);
				theTemporaryInputData = *theInputData++;					/* buffer two ADPCM nibbles */
				theCode = theTemporaryInputData & 0x0F;
			}
			
			theDifference = 0;								/* compute new sample estimate thePredictedSample */
			if (theCode & 4)
				theDifference += theStep;
			if (theCode & 2)
				theDifference += theStep >> 1;
			if (theCode & 1)
				theDifference += theStep >> 2;
				
			theDifference += theStep >> 3;
			if (theCode & 8)
				theDifference = -theDifference;

			thePredictedSample += theDifference;

			//	check for overflow
			if(thePredictedSample > 32767)
			{
				thePredictedSample = 32767;
			}
			else if(thePredictedSample < -32768)
			{
				thePredictedSample = -32768;
			}

			//printf("   theCode %02X  theDifference %d\n", theCode, theDifference);
			//printf("OUT %02X %d   \n", thePredictedSample & 255, thePredictedSample);

			// Cheaper than floating point division
			tempFloat = (float)(thePredictedSample);
			tempFloatPtr = &tempFloat;
			if (tempFloat != 0.0 && tempFloat != -0.0)
			{
				tempULong = *((unsigned long *)tempFloatPtr);
				tempULong -= 0x07800000;
				tempFloatPtr = (float *)(&tempULong);
			}
			*theOutputData = *tempFloatPtr;
			theOutputData += theOutputStride;
			
			theStepTableIndex += sIndexTable[theCode];				/* compute new stepsize step */
			if (theStepTableIndex < 0)
				theStepTableIndex = 0;
			else if (theStepTableIndex > 88)
				theStepTableIndex = 88;
			theStep = sStepTable[theStepTableIndex];
			//printf("   theStep %d  theStepTableIndex %d\n", theStep, theStepTableIndex);
		}

		theInputData += theInputStride;
	}

	ioChannelState.mPredictedSample = thePredictedSample;
	ioChannelState.mStepTableIndex = theStepTableIndex;
}

UInt32	ACAppleIMA4Decoder::GetVersion() const
{
	return kIMA4adecVersion;
}

extern "C"
ComponentResult	ACAppleIMA4DecoderEntry(ComponentParameters* inParameters, ACAppleIMA4Decoder* inThis)
{	
	return	ACCodecDispatch(inParameters, inThis);
}


const SInt32 kPredTolerance = 0x007F;
const UInt32 kIndexMask = 0x007F;
const UInt32 kPredictorMask = 0xFF80;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ACAppleIMA4Decoder::CheckState(const Byte *inInputData, ChannelState& ioChannelState)
{
	SInt16 s = EndianS16_BtoN(*((short *) inInputData));
	SInt16 theStepTableIndex = s & kIndexMask;					// get stored index
	s &= kPredictorMask;					// get stored thePredictedSample
	SInt32 thePredictedSample = s;							// make sure it gets sign-extended!

	if (theStepTableIndex == ioChannelState.mStepTableIndex)				// indexes match
	{
		SInt32 theDifference = thePredictedSample - ioChannelState.mPredictedSample;	// calculate theDifference between state and stored value
		if (theDifference < 0)
			theDifference = -theDifference;
		if (theDifference <= kPredTolerance)			// is difference greater than tolerance?
			return;							// no, so state is good
	}

	ioChannelState.mPredictedSample = thePredictedSample;			// use stored state
	ioChannelState.mStepTableIndex = theStepTableIndex;
}

void ACAppleIMA4Decoder::FixFormats()
{
	mInputFormat.mFramesPerPacket = 64;
	mInputFormat.mBytesPerPacket = mInputFormat.mChannelsPerFrame * 34;
	mInputFormat.mBytesPerFrame = 0;
}


