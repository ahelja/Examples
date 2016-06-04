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
	AUOutputBase.cpp
	
=============================================================================*/

#include "AUOutputBase.h"


AUOutputBase::AUOutputBase(			AudioUnit						inInstance, 
									UInt32							numInputs,
									UInt32							numOutputs,
									UInt32							numGroups) :
		AUBase(inInstance, numInputs, numOutputs, numGroups)
{
}

ComponentResult		AUOutputBase::Start()
{
	return noErr;
}

ComponentResult		AUOutputBase::Stop()
{
	return noErr;
}




#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

#if	TARGET_API_MAC_OS8 || TARGET_API_MAC_OSX
struct AudioOutputUnitStartStopGluePB {
	unsigned char                  componentFlags;
	unsigned char                  componentParamSize;
	short                          componentWhat;
	AudioUnit                      ci;
};
#elif	TARGET_OS_WIN32
struct AudioOutputUnitStartStopGluePB {
	unsigned char                  componentFlags;
	unsigned char                  componentParamSize;
	short                          componentWhat;
};
#else
	#error	Platform not supported
#endif

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

ComponentResult	AUOutputBase::ComponentEntryDispatch(ComponentParameters *params, AUOutputBase *This)
{
	if (This == NULL) return paramErr;

	ComponentResult result;
	
	switch (params->what) {
	case kAudioOutputUnitStartSelect:
		{
			//AudioOutputUnitStartStopGluePB *p = (AudioOutputUnitStartStopGluePB *)params;
			result = This->Start();
		}
		break;

	case kAudioOutputUnitStopSelect:
		{
			//AudioOutputUnitStartStopGluePB *p = (AudioOutputUnitStartStopGluePB *)params;
			result = This->Stop();
		}
		break;

	default:
		result = AUBase::ComponentEntryDispatch(params, This);
		break;
	}
	
	return result;
}
