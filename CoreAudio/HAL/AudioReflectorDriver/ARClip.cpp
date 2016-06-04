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
	ARClip.cpp

==================================================================================================*/

//==================================================================================================
//	Includes
//==================================================================================================

//	Local Includes
#include "ARDebug.h"
#include "AREngine.h"
#include "PCMBlitterLibDispatch.h"

//	System Includes
#include <IOKit/IOLib.h>
#include <IOKit/audio/IOAudioStream.h>

//==================================================================================================
//	ARClip
//==================================================================================================

IOReturn AREngine::clipOutputSamples(const void* inMixBuffer, void* outTargetBuffer, UInt32 inFirstFrame, UInt32 inNumberFrames, const IOAudioStreamFormat* inFormat, IOAudioStream* /*inStream*/)
{
	//	figure out what sort of blit we need to do
	if((inFormat->fSampleFormat == kIOAudioStreamSampleFormatLinearPCM) && inFormat->fIsMixable)
	{
		//	it's mixable linear PCM, which means we will be calling a blitter, which works in samples not frames
		Float32* theMixBuffer = (Float32*)inMixBuffer;
		UInt32 theFirstSample = inFirstFrame * inFormat->fNumChannels;
		UInt32 theNumberSamples = inNumberFrames * inFormat->fNumChannels;
	
		if(inFormat->fNumericRepresentation == kIOAudioStreamNumericRepresentationSignedInt)
		{
			//	it's some kind of signed integer, which we handle as some kind of even byte length
			bool nativeEndianInts;
			#if TARGET_RT_BIG_ENDIAN
				nativeEndianInts = (inFormat->fByteOrder == kIOAudioStreamByteOrderBigEndian);
			#else
				nativeEndianInts = (inFormat->fByteOrder == kIOAudioStreamByteOrderLittleEndian);
			#endif
			
			switch(inFormat->fBitWidth)
			{
				case 8:
					{
						DebugMessage("AREngine::clipOutputSamples: can't handle signed integers with a bit width of 8 at the moment");
					}
					break;
				
				case 16:
					{
						SInt16* theTargetBuffer = (SInt16*)outTargetBuffer;
						if (nativeEndianInts)
							Float32ToNativeInt16(mHasVectorUnit, &(theMixBuffer[theFirstSample]), &(theTargetBuffer[theFirstSample]), theNumberSamples);
						else
							Float32ToSwapInt16(mHasVectorUnit, &(theMixBuffer[theFirstSample]), &(theTargetBuffer[theFirstSample]), theNumberSamples);
					}
					break;
				
				case 24:
					{
						UInt8* theTargetBuffer = (UInt8*)outTargetBuffer;
						if (nativeEndianInts)
							Float32ToNativeInt24(mHasVectorUnit, &(theMixBuffer[theFirstSample]), &(theTargetBuffer[3*theFirstSample]), theNumberSamples);
						else
							Float32ToSwapInt24(mHasVectorUnit, &(theMixBuffer[theFirstSample]), &(theTargetBuffer[3*theFirstSample]), theNumberSamples);
					}
					break;
				
				case 32:
					{
						SInt32* theTargetBuffer = (SInt32*)outTargetBuffer;
						if (nativeEndianInts)
							Float32ToNativeInt32(mHasVectorUnit, &(theMixBuffer[theFirstSample]), &(theTargetBuffer[theFirstSample]), theNumberSamples);
						else
							Float32ToSwapInt32(mHasVectorUnit, &(theMixBuffer[theFirstSample]), &(theTargetBuffer[theFirstSample]), theNumberSamples);
					}
					break;
				
				default:
					DebugMessageN1("AREngine::clipOutputSamples: can't handle signed integers with a bit width of %d", inFormat->fBitWidth);
					break;
				
			}
		}
		else if(inFormat->fNumericRepresentation == kIOAudioStreamNumericRepresentationIEEE754Float)
		{
			//	it is some kind of floating point format
		#if TARGET_RT_BIG_ENDIAN
			if((inFormat->fBitWidth == 32) && (inFormat->fBitDepth == 32) && (inFormat->fByteOrder == kIOAudioStreamByteOrderBigEndian))
		#else
			if((inFormat->fBitWidth == 32) && (inFormat->fBitDepth == 32) && (inFormat->fByteOrder == kIOAudioStreamByteOrderLittleEndian))
		#endif
			{
				//	it's Float32, so we are just going to copy the data
				Float32* theTargetBuffer = (Float32*)outTargetBuffer;
				memcpy(&(theTargetBuffer[theFirstSample]), &(theMixBuffer[theFirstSample]), theNumberSamples * sizeof(Float32));
			}
			else
			{
				DebugMessageN2("AREngine::clipOutputSamples: can't handle floats with a bit width of %d, bit depth of %d, and/or the given byte order", inFormat->fBitWidth, inFormat->fBitDepth);
			}
		}
	}
	else
	{
		//	it's not linear PCM or it's not mixable, so just copy the data into the target buffer
		SInt8* theMixBuffer = (SInt8*)inMixBuffer;
		SInt8* theTargetBuffer = (SInt8*)outTargetBuffer;
		UInt32 theFirstByte = inFirstFrame * (inFormat->fBitWidth / 8) * inFormat->fNumChannels;
		UInt32 theNumberBytes = inNumberFrames * (inFormat->fBitWidth / 8) * inFormat->fNumChannels;
		memcpy(&(theTargetBuffer[theFirstByte]), &(theMixBuffer[theFirstByte]), theNumberBytes);
	}

	return kIOReturnSuccess;
}

IOReturn AREngine::convertInputSamples(const void* inSourceBuffer, void* outTargetBuffer, UInt32 inFirstFrame, UInt32 inNumberFrames, const IOAudioStreamFormat* inFormat, IOAudioStream* /*inStream*/)
{
	//	figure out what sort of blit we need to do
	if((inFormat->fSampleFormat == kIOAudioStreamSampleFormatLinearPCM) && inFormat->fIsMixable)
	{
		//	it's linear PCM, which means the target is Float32 and we will be calling a blitter, which works in samples not frames
		Float32* theTargetBuffer = (Float32*)outTargetBuffer;
		UInt32 theFirstSample = inFirstFrame * inFormat->fNumChannels;
		UInt32 theNumberSamples = inNumberFrames * inFormat->fNumChannels;
	
		if(inFormat->fNumericRepresentation == kIOAudioStreamNumericRepresentationSignedInt)
		{
			//	it's some kind of signed integer, which we handle as some kind of even byte length
			bool nativeEndianInts;
			#if TARGET_RT_BIG_ENDIAN
				nativeEndianInts = (inFormat->fByteOrder == kIOAudioStreamByteOrderBigEndian);
			#else
				nativeEndianInts = (inFormat->fByteOrder == kIOAudioStreamByteOrderLittleEndian);
			#endif
			
			switch(inFormat->fBitWidth)
			{
				case 8:
					{
						DebugMessage("AREngine::convertInputSamples: can't handle signed integers with a bit width of 8 at the moment");
					}
					break;
				
				case 16:
					{
						SInt16* theSourceBuffer = (SInt16*)inSourceBuffer;
						if (nativeEndianInts)
							NativeInt16ToFloat32(mHasVectorUnit, &(theSourceBuffer[theFirstSample]), theTargetBuffer, theNumberSamples);
						else
							SwapInt16ToFloat32(mHasVectorUnit, &(theSourceBuffer[theFirstSample]), theTargetBuffer, theNumberSamples);
					}
					break;
				
				case 24:
					{
						UInt8* theSourceBuffer = (UInt8*)inSourceBuffer;
						if (nativeEndianInts)
							NativeInt24ToFloat32(mHasVectorUnit, &(theSourceBuffer[3*theFirstSample]), theTargetBuffer, theNumberSamples);
						else
							SwapInt24ToFloat32(mHasVectorUnit, &(theSourceBuffer[3*theFirstSample]), theTargetBuffer, theNumberSamples);
					}
					break;
				
				case 32:
					{
						SInt32* theSourceBuffer = (SInt32*)inSourceBuffer;
						if (nativeEndianInts)
							NativeInt32ToFloat32(mHasVectorUnit, &(theSourceBuffer[theFirstSample]), theTargetBuffer, theNumberSamples);
						else
							SwapInt32ToFloat32(mHasVectorUnit, &(theSourceBuffer[theFirstSample]), theTargetBuffer, theNumberSamples);
					}
					break;
				
				default:
					DebugMessageN1("AREngine::convertInputSamples: can't handle signed integers with a bit width of %d", inFormat->fBitWidth);
					break;
				
			}
		}
		else if(inFormat->fNumericRepresentation == kIOAudioStreamNumericRepresentationIEEE754Float)
		{
			//	it is some kind of floating point format
		#if TARGET_RT_BIG_ENDIAN
			if((inFormat->fBitWidth == 32) && (inFormat->fBitDepth == 32) && (inFormat->fByteOrder == kIOAudioStreamByteOrderBigEndian))
		#else
			if((inFormat->fBitWidth == 32) && (inFormat->fBitDepth == 32) && (inFormat->fByteOrder == kIOAudioStreamByteOrderLittleEndian))
		#endif
			{
				//	it's Float32, so we are just going to copy the data
				Float32* theSourceBuffer = (Float32*)inSourceBuffer;
				memcpy(theTargetBuffer, &(theSourceBuffer[theFirstSample]), theNumberSamples * sizeof(Float32));
			}
			else
			{
				DebugMessageN2("AREngine::convertInputSamples: can't handle floats with a bit width of %d, bit depth of %d, and/or the given byte order", inFormat->fBitWidth, inFormat->fBitDepth);
			}
		}
	}
	else
	{
		//	it's not linear PCM or it's not mixable, so just copy the data into the target buffer
		SInt8* theSourceBuffer = (SInt8*)inSourceBuffer;
		UInt32 theFirstByte = inFirstFrame * (inFormat->fBitWidth / 8) * inFormat->fNumChannels;
		UInt32 theNumberBytes = inNumberFrames * (inFormat->fBitWidth / 8) * inFormat->fNumChannels;
		memcpy(outTargetBuffer, &(theSourceBuffer[theFirstByte]), theNumberBytes);
	}

	return kIOReturnSuccess;
}
