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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AudioFilePlayer.h
//
#include "AudioFilePlayer.h"

#define TRY		try {

#define CATCH				\
	}						\
	catch (OSStatus stat) 	\
	{						\
		return stat;		\
	}						\
	catch (...) 			\
	{						\
		return -1;			\
	}						\
	return noErr;

OSStatus NewAudioFilePlayID (const FSRef	*inFileRef, AudioFilePlayID* outFilePlayID)
{
	TRY
	
	if (!inFileRef || !outFilePlayID) return paramErr;
	
	AudioFilePlayer* player = new AudioFilePlayer (*inFileRef);
	*outFilePlayID = reinterpret_cast<OpaqueFilePlayObj*>(player);
	
	CATCH
}

OSStatus DisposeAudioFilePlayID (AudioFilePlayID 	inFilePlayID)
{
	TRY
	
	delete reinterpret_cast<AudioFilePlayer*>(inFilePlayID);
	
	CATCH
}

OSStatus AFP_SetDestination (AudioFilePlayID 		inFilePlayID,
									AudioUnit 				inDestUnit, 
									UInt32					inDestBusNumber)
{
	TRY
	
	if (!inFilePlayID || !inDestUnit) return paramErr;

	reinterpret_cast<AudioFilePlayer*>(inFilePlayID)->SetDestination (inDestUnit, 
															inDestBusNumber);
	CATCH
}

OSStatus AFP_SetLooping (AudioFilePlayID 			inFilePlayID,
									Boolean 				inShouldLoop)
{
	TRY

	if (!inFilePlayID) return paramErr;
	
	reinterpret_cast<AudioFilePlayer*>(inFilePlayID)->SetLooping (inShouldLoop ? true : false);
	
	CATCH
}
	
OSStatus AFP_SetNotifier (AudioFilePlayID 			inFilePlayID,
									AudioFilePlayNotifier 	inNotifier, 
									void*					inUserData)
{
	TRY

	if (!inFilePlayID) return paramErr;
	
	reinterpret_cast<AudioFilePlayer*>(inFilePlayID)->SetNotifier (inNotifier, inUserData);
	
	CATCH
}
	
OSStatus AFP_Connect (AudioFilePlayID 		inFilePlayID)
{
	TRY

	if (!inFilePlayID) return paramErr;
	
	reinterpret_cast<AudioFilePlayer*>(inFilePlayID)->Connect();
	
	CATCH
}

OSStatus AFP_Disconnect (AudioFilePlayID 	inFilePlayID)
{
	TRY

	if (!inFilePlayID) return paramErr;
	
	reinterpret_cast<AudioFilePlayer*>(inFilePlayID)->Disconnect();
	
	CATCH
}

OSStatus AFP_IsConnected (AudioFilePlayID 	inFilePlayID, Boolean	*outIsConnected)
{
	TRY

	if (!inFilePlayID || !outIsConnected) return paramErr;
	
	*outIsConnected = reinterpret_cast<AudioFilePlayer*>(inFilePlayID)->IsConnected();
	
	CATCH
}

OSStatus AFP_GetInfo (AudioFilePlayID 	inFilePlayID,
							AudioUnit			*outDestUnit,
							UInt32				*outBusNumber,
							AudioConverterRef 	*outConverter)
{
	TRY

	if (!inFilePlayID) return paramErr;
	
	AudioFilePlayer* player = reinterpret_cast<AudioFilePlayer*>(inFilePlayID);
	
	if (player->GetDestUnit() == 0)
		return kAudioFilePlay_PlayerIsUninitialized;
		
	if (outDestUnit)
		*outDestUnit = player->GetDestUnit();

	if (outBusNumber)
		*outBusNumber = player->GetBusNumber();

	if (outConverter)
		*outConverter = player->GetAudioConverter();
	
	CATCH
}

OSStatus AFP_Print (AudioFilePlayID 			inFilePlayID)
{
	TRY

	if (!inFilePlayID) return paramErr;
	
	reinterpret_cast<AudioFilePlayer*>(inFilePlayID)->Print();
	
	CATCH
}


void PrintStreamDesc (AudioStreamBasicDescription *inDesc)
{
	if (!inDesc) {
		printf ("Can't print a NULL desc!\n");
		return;
	}
	
	printf ("- - - - - - - - - - - - - - - - - - - -\n");
	printf ("  Sample Rate:%f\n", inDesc->mSampleRate);
	printf ("  Format ID:%s\n", (char*)&inDesc->mFormatID);
	printf ("  Format Flags:%lX\n", inDesc->mFormatFlags);
	printf ("  Bytes per Packet:%ld\n", inDesc->mBytesPerPacket);
	printf ("  Frames per Packet:%ld\n", inDesc->mFramesPerPacket);
	printf ("  Bytes per Frame:%ld\n", inDesc->mBytesPerFrame);
	printf ("  Channels per Frame:%ld\n", inDesc->mChannelsPerFrame);
	printf ("  Bits per Channel:%ld\n", inDesc->mBitsPerChannel);
	printf ("- - - - - - - - - - - - - - - - - - - -\n");
}
