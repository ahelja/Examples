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
	AudioFileComponentBase.cpp
	
=============================================================================*/

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioToolbox/AudioFileComponent.h>
#else
	#include "AudioFileComponent.h"
#endif

#include "AudioFileComponentBase.h"
#include "AudioFileComponentDispatchTypes.h"

#ifndef __defined_kAudioFileRemoveUserDataSelect__
	enum {
		kAudioFileRemoveUserDataSelect				= 0x0018
	};
#endif

//----------------------------------------------------------------------------------------

static OSStatus AFAPI_ReadBytesFDF(
								void								*inComponentStorage,
								Boolean			inUseCache,
								SInt64			inStartingByte, 
								UInt32			*ioNumBytes, 
								void			*outBuffer)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_ReadBytes(inUseCache, inStartingByte, ioNumBytes, outBuffer);
}


static OSStatus AFAPI_WriteBytesFDF(
								void								*inComponentStorage,
								Boolean			inUseCache,
								SInt64			inStartingByte, 
								UInt32			*ioNumBytes, 
								const void		*inBuffer)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_WriteBytes(inUseCache, inStartingByte, ioNumBytes, inBuffer);
}


static OSStatus AFAPI_ReadPacketsFDF(
								void							*inComponentStorage,
								Boolean							inUseCache,
								UInt32							*outNumBytes,
								AudioStreamPacketDescription	*outPacketDescriptions,
								SInt64							inStartingPacket, 
								UInt32							*ioNumPackets, 
								void							*outBuffer)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_ReadPackets(inUseCache, outNumBytes, outPacketDescriptions, 
		inStartingPacket, ioNumPackets, outBuffer);
}

static OSStatus AFAPI_WritePacketsFDF(
								void							*inComponentStorage,
								Boolean							inUseCache,
								UInt32							inNumBytes,
								AudioStreamPacketDescription	*inPacketDescriptions,
								SInt64							inStartingPacket, 
								UInt32							*ioNumPackets, 
								const void						*inBuffer)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_WritePackets(inUseCache, inNumBytes, inPacketDescriptions, 
		inStartingPacket, ioNumPackets, inBuffer);
}

static OSStatus AFAPI_GetPropertyInfoFDF(
								void					*inComponentStorage,
								AudioFilePropertyID		inPropertyID,
								UInt32					*outDataSize,
								UInt32					*isWritable)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_GetPropertyInfo(inPropertyID, outDataSize, isWritable);
}

static OSStatus AFAPI_GetPropertyFDF(
								void					*inComponentStorage,
								AudioFilePropertyID		inPropertyID,
								UInt32					*ioDataSize,
								void					*ioData)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_GetProperty(inPropertyID, ioDataSize, ioData);
}


static OSStatus AFAPI_SetPropertyFDF(
								void					*inComponentStorage,
								AudioFilePropertyID		inPropertyID,
								UInt32					inDataSize,
								const void				*inData)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_SetProperty(inPropertyID, inDataSize, inData);
}

static OSStatus AFAPI_CountUserDataFDF(
								void					*inComponentStorage,
								UInt32					inUserDataID,
								UInt32					*outNumberItems)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_CountUserData(inUserDataID, outNumberItems);
}

static OSStatus AFAPI_GetUserDataSizeFDF(
								void					*inComponentStorage,
								UInt32					inUserDataID,
								UInt32					inIndex,
								UInt32					*outDataSize)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_GetUserDataSize(inUserDataID, inIndex, outDataSize);
}

static OSStatus AFAPI_GetUserDataFDF(
								void					*inComponentStorage,
								UInt32					inUserDataID,
								UInt32					inIndex,
								UInt32					*ioUserDataSize,
								void					*outUserData)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_GetUserData(inUserDataID, inIndex, ioUserDataSize, outUserData);
}

static OSStatus AFAPI_SetUserDataFDF(
								void					*inComponentStorage,
								UInt32					inUserDataID,
								UInt32					inIndex,
								UInt32					inUserDataSize,
								const void				*inUserData)
{
	AudioFileComponentBase* obj = (AudioFileComponentBase*)inComponentStorage;
	return obj->AFAPI_SetUserData(inUserDataID, inIndex, inUserDataSize, inUserData);
}

//----------------------------------------------------------------------------------------


AudioFileComponentBase::AudioFileComponentBase(ComponentInstance inInstance)
	: ComponentBase(inInstance)
{
}

AudioFileComponentBase::~AudioFileComponentBase()
{
}

AudioFileObjectComponentBase::AudioFileObjectComponentBase(ComponentInstance inInstance)
	: AudioFileComponentBase(inInstance), mAudioFileObject(0)
{
	// derived class should create an AudioFileObject here and if NULL, the AudioFormat as well.
}

AudioFileObjectComponentBase::~AudioFileObjectComponentBase()
{
	delete mAudioFileObject;
}

OSStatus AudioFileObjectComponentBase::AFAPI_Create(
								const FSRef							*inParentRef, 
                                CFStringRef							inFileName,
                                const AudioStreamBasicDescription	*inFormat,
                                UInt32								inFlags,
                                FSRef								*outNewFileRef)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->DoCreate(inParentRef, inFileName, inFormat, inFlags, outNewFileRef);
}

								
OSStatus AudioFileObjectComponentBase::AFAPI_Initialize(
									const FSRef							*inFileRef,
                                    const AudioStreamBasicDescription	*inFormat,
                                    UInt32								inFlags)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->DoInitialize(inFileRef, inFormat, inFlags);
}

								
OSStatus AudioFileObjectComponentBase::AFAPI_Open(
									const FSRef		*inFileRef, 
									SInt8  			inPermissions,
									SInt16			inRefNum)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->DoOpen(inFileRef, inPermissions, inRefNum);
}


OSStatus AudioFileObjectComponentBase::AFAPI_OpenWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->DoOpenWithCallbacks(inRefCon, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc);
}


OSStatus AudioFileObjectComponentBase::AFAPI_InitializeWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc,
				UInt32								inFileType,
				const AudioStreamBasicDescription	*inFormat,
				UInt32								inFlags)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->DoInitializeWithCallbacks(inRefCon, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc, 
											inFileType, inFormat, inFlags);
}

									
OSStatus AudioFileObjectComponentBase::AFAPI_Close()
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->DoClose();
}

OSStatus AudioFileObjectComponentBase::AFAPI_Optimize()
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->DoOptimize();
}

OSStatus AudioFileObjectComponentBase::AFAPI_ReadBytes(		
											Boolean			inUseCache,
											SInt64			inStartingByte, 
											UInt32			*ioNumBytes, 
											void			*outBuffer)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->ReadBytes(inUseCache, inStartingByte, ioNumBytes, outBuffer);
}


OSStatus AudioFileObjectComponentBase::AFAPI_WriteBytes(		
											Boolean			inUseCache,
											SInt64			inStartingByte, 
											UInt32			*ioNumBytes, 
											const void		*inBuffer)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->WriteBytes(inUseCache, inStartingByte, ioNumBytes, inBuffer);
}




OSStatus AudioFileObjectComponentBase::AFAPI_ReadPackets(		
											Boolean							inUseCache,
											UInt32							*outNumBytes,
											AudioStreamPacketDescription	*outPacketDescriptions,
											SInt64							inStartingPacket, 
											UInt32  						*ioNumPackets, 
											void							*outBuffer)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->ReadPackets(inUseCache, outNumBytes, outPacketDescriptions,
												inStartingPacket, ioNumPackets, outBuffer);
}

									
OSStatus AudioFileObjectComponentBase::AFAPI_WritePackets(	
											Boolean							inUseCache,
											UInt32							inNumBytes,
											AudioStreamPacketDescription	*inPacketDescriptions,
											SInt64							inStartingPacket, 
											UInt32  						*ioNumPackets, 
											const void						*inBuffer)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->WritePackets(inUseCache, inNumBytes, inPacketDescriptions,
												inStartingPacket, ioNumPackets, inBuffer);
}


									
OSStatus AudioFileObjectComponentBase::AFAPI_GetPropertyInfo(	
											AudioFilePropertyID		inPropertyID,
											UInt32					*outDataSize,
											UInt32					*isWritable)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->GetPropertyInfo(inPropertyID, outDataSize, isWritable);
}

										
OSStatus AudioFileObjectComponentBase::AFAPI_GetProperty(		
											AudioFilePropertyID		inPropertyID,
											UInt32					*ioPropertySize,
											void					*ioPropertyData)
{
	OSStatus err = noErr;
	
	if (!ioPropertyData) return paramErr;
	
	if (!mAudioFileObject) return paramErr;
	err = mAudioFileObject->GetProperty(inPropertyID, ioPropertySize, ioPropertyData);
	return err;
}

									
OSStatus AudioFileObjectComponentBase::AFAPI_SetProperty(		
											AudioFilePropertyID		inPropertyID,
											UInt32					inPropertySize,
											const void				*inPropertyData)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->SetProperty(inPropertyID, inPropertySize, inPropertyData);
}


OSStatus AudioFileObjectComponentBase::AFAPI_CountUserData(		
											UInt32				inUserDataID,
											UInt32				*outNumberItems)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->CountUserData(inUserDataID, outNumberItems);
}

OSStatus AudioFileObjectComponentBase::AFAPI_GetUserDataSize(		
											UInt32				inUserDataID,
											UInt32				inIndex,
											UInt32				*outUserDataSize)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->GetUserDataSize(inUserDataID, inIndex, outUserDataSize);
}

OSStatus AudioFileObjectComponentBase::AFAPI_GetUserData(		
											UInt32				inUserDataID,
											UInt32				inIndex,
											UInt32				*ioUserDataSize,
											void				*outUserData)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->GetUserData(inUserDataID, inIndex, ioUserDataSize, outUserData);
}

OSStatus AudioFileObjectComponentBase::AFAPI_SetUserData(		
											UInt32				inUserDataID,
											UInt32				inIndex,
											UInt32				inUserDataSize,
											const void			*inUserData)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->SetUserData(inUserDataID, inIndex, inUserDataSize, inUserData);
}

OSStatus AudioFileObjectComponentBase::AFAPI_RemoveUserData(		
											UInt32				inUserDataID,
											UInt32				inIndex)
{
	if (!mAudioFileObject) return paramErr;
	return mAudioFileObject->RemoveUserData(inUserDataID, inIndex);
}


OSStatus AudioFileComponentBase::AFAPI_GetGlobalInfoSize(		
											AudioFilePropertyID		inPropertyID,
											UInt32					inSpecifierSize,
											const void*				inSpecifier,
											UInt32					*outPropertySize)
{
	OSStatus err = noErr;
		
	switch (inPropertyID)
	{
		case kAudioFileComponent_CanRead :
		case kAudioFileComponent_CanWrite :
			*outPropertySize = sizeof(UInt32);
			break;

		case kAudioFileComponent_FileTypeName :
			*outPropertySize = sizeof(CFStringRef);
			break;

		case kAudioFileComponent_ExtensionsForType :
			*outPropertySize = sizeof(CFArrayRef);
			break;
			
		case kAudioFileComponent_AvailableFormatIDs :
		{
			UInt32 size = 0xFFFFFFFF;
			err = GetAudioFileFormatBase()->GetAvailableFormatIDs(&size, NULL);
			if (!err) *outPropertySize = size;
			break;
		}
		case kAudioFileComponent_HFSTypeCodesForType :
		{
			UInt32 size = 0xFFFFFFFF;
			err = GetAudioFileFormatBase()->GetHFSCodes(&size, NULL);
			if (!err) *outPropertySize = size;
			break;
		}
		case kAudioFileComponent_AvailableStreamDescriptionsForFormat :
			{
				if (inSpecifierSize != sizeof(UInt32)) return paramErr;
				UInt32 inFormatID = *(UInt32*)inSpecifier;
				err = GetAudioFileFormatBase()->GetAvailableStreamDescriptions(inFormatID, outPropertySize, NULL);
			}
			break;
			
		case kAudioFileComponent_FastDispatchTable :
			*outPropertySize = sizeof(AudioFileFDFTable);
			break;
			
		default:
			err = kAudioFileUnsupportedPropertyError;
	}
	return err;
}


OSStatus AudioFileComponentBase::AFAPI_GetGlobalInfo(		
											AudioFilePropertyID		inPropertyID,
											UInt32					inSpecifierSize,
											const void*				inSpecifier,
											UInt32					*ioPropertySize,
											void					*ioPropertyData)
{
	OSStatus err = noErr;
	
	if (!ioPropertyData) return paramErr;
	
	switch (inPropertyID)
	{
		case kAudioFileComponent_CanRead :
			{
				if (*ioPropertySize != sizeof(UInt32)) return paramErr;
				UInt32* flag = (UInt32*)ioPropertyData;
				*flag = GetAudioFileFormatBase()->CanRead();
			}
			break;
			
		case kAudioFileComponent_CanWrite :
			{
				if (*ioPropertySize != sizeof(UInt32)) return paramErr;
				UInt32* flag = (UInt32*)ioPropertyData;
				*flag = GetAudioFileFormatBase()->CanWrite();
			}
			break;
			
		case kAudioFileComponent_FileTypeName :
			{
				if (*ioPropertySize != sizeof(CFStringRef)) return paramErr;
				CFStringRef* name = (CFStringRef*)ioPropertyData;
				GetAudioFileFormatBase()->GetFileTypeName(name);
			}
			break;

		case kAudioFileComponent_ExtensionsForType :
			{
				if (*ioPropertySize != sizeof(CFArrayRef)) return paramErr;
				CFArrayRef* array = (CFArrayRef*)ioPropertyData;
				GetAudioFileFormatBase()->GetExtensions(array);
			}
			break;

		case kAudioFileComponent_HFSTypeCodesForType :
			{
				err = GetAudioFileFormatBase()->GetHFSCodes(ioPropertySize, ioPropertyData);
			}
			break;
			
		case kAudioFileComponent_AvailableFormatIDs :
			{
				err = GetAudioFileFormatBase()->GetAvailableFormatIDs(ioPropertySize, ioPropertyData);
			}
			break;
			
		case kAudioFileComponent_AvailableStreamDescriptionsForFormat :
			{
				if (inSpecifierSize != sizeof(UInt32)) return paramErr;
				UInt32 inFormatID = *(UInt32*)inSpecifier; 
				err = GetAudioFileFormatBase()->GetAvailableStreamDescriptions(inFormatID, ioPropertySize, ioPropertyData);
			}
			break;
			
		case kAudioFileComponent_FastDispatchTable :
			{
				if (*ioPropertySize != sizeof(AudioFileFDFTable)) return paramErr;
				AudioFileFDFTable *table = (AudioFileFDFTable*)ioPropertyData;
				table->mComponentStorage = (void*)this;
				table->mReadBytesFDF = &AFAPI_ReadBytesFDF;
				table->mWriteBytesFDF = &AFAPI_WriteBytesFDF;
				table->mReadPacketsFDF = &AFAPI_ReadPacketsFDF;
				table->mWritePacketsFDF = &AFAPI_WritePacketsFDF;
				table->mGetPropertyInfoFDF = &AFAPI_GetPropertyInfoFDF;
				table->mGetPropertyFDF = &AFAPI_GetPropertyFDF;
				table->mSetPropertyFDF = &AFAPI_SetPropertyFDF;
				table->mCountUserDataFDF = &AFAPI_CountUserDataFDF;
				table->mGetUserDataSizeFDF = &AFAPI_GetUserDataSizeFDF;
				table->mGetUserDataFDF = &AFAPI_GetUserDataFDF;
				table->mSetUserDataFDF = &AFAPI_SetUserDataFDF;
			}
			break;
			
		default:
			err = kAudioFileUnsupportedPropertyError;
	}
	return err;
}



ComponentResult AudioFileComponentBase::ComponentEntryDispatch(ComponentParameters* inParameters, AudioFileComponentBase* inThis)
{
	ComponentResult	result = noErr;
	if (inThis == NULL) return paramErr;
	
	try
	{
		switch (inParameters->what)
		{						
			case kComponentCanDoSelect:
				switch (*((SInt16*)&inParameters->params[1]))
				{
					case kAudioFileCreateSelect:
					case kAudioFileOpenSelect:
					case kAudioFileInitializeSelect:
					case kAudioFileOpenWithCallbacksSelect:
					case kAudioFileInitializeWithCallbacksSelect:
					case kAudioFileCloseSelect:
					case kAudioFileOptimizeSelect:
					case kAudioFileReadBytesSelect:
					case kAudioFileWriteBytesSelect:
					case kAudioFileReadPacketsSelect:
					case kAudioFileWritePacketsSelect:
					case kAudioFileGetPropertyInfoSelect:
					case kAudioFileGetPropertySelect:
					case kAudioFileSetPropertySelect:
					case kAudioFileExtensionIsThisFormatSelect:
					case kAudioFileDataIsThisFormatSelect:
					case kAudioFileFileIsThisFormatSelect:
					case kAudioFileGetGlobalInfoSizeSelect:
					case kAudioFileGetGlobalInfoSelect:
					
					case kAudioFileCountUserDataSelect:
					case kAudioFileGetUserDataSizeSelect:
					case kAudioFileGetUserDataSelect:
					case kAudioFileSetUserDataSelect:
					case kAudioFileRemoveUserDataSelect:
						result = 1;
						break;
					default:
						result = ComponentBase::ComponentEntryDispatch(inParameters, inThis);
				};
				break;
					
				case kAudioFileCreateSelect:
					{
						AudioFileComponentCreateGluePB* pb = (AudioFileComponentCreateGluePB*)inParameters;
						
						result = inThis->AFAPI_Create(pb->inParentRef, pb->inFileName, pb->inFormat, pb->inFlags, pb->outNewFileRef);
					}
					break;
				case kAudioFileOpenSelect:
					{
						AudioFileComponentOpenGluePB* pb = (AudioFileComponentOpenGluePB*)inParameters;
						
						result = inThis->AFAPI_Open(pb->inFileRef, pb->inPermissions, pb->inRefNum);
					}
					break;
				case kAudioFileInitializeSelect:
					{
						AudioFileComponentInitializeGluePB* pb = (AudioFileComponentInitializeGluePB*)inParameters;
						
						result = inThis->AFAPI_Initialize(pb->inFileRef, pb->inFormat, pb->inFlags);
					}
					break;
				case kAudioFileOpenWithCallbacksSelect:
					{
						AudioFileComponentOpenWithCallbacksGluePB* pb = (AudioFileComponentOpenWithCallbacksGluePB*)inParameters;
						
						result = inThis->AFAPI_OpenWithCallbacks(pb->inRefCon, pb->inReadFunc, pb->inWriteFunc, 
													pb->inGetSizeFunc, pb->inSetSizeFunc);
					}
					break;
				case kAudioFileInitializeWithCallbacksSelect:
					{
						AudioFileComponentInitializeWithCallbacksGluePB* pb = (AudioFileComponentInitializeWithCallbacksGluePB*)inParameters;
						
						result = inThis->AFAPI_InitializeWithCallbacks(pb->inRefCon, pb->inReadFunc, pb->inWriteFunc, 
													pb->inGetSizeFunc, pb->inSetSizeFunc,
													pb->inFileType, pb->inFormat, pb->inFlags);
					}
					break;
				case kAudioFileCloseSelect:
					{
						//AudioFileComponentCloseGluePB* pb = (AudioFileComponentCloseGluePB*)inParameters;
						
						result = inThis->AFAPI_Close();
					}
					break;
				case kAudioFileOptimizeSelect:
					{
						//AudioFileComponentOptimizeGluePB* pb = (AudioFileComponentOptimizeGluePB*)inParameters;
						
						result = inThis->AFAPI_Optimize();
					}
					break;
				case kAudioFileReadBytesSelect:
					{
						AudioFileComponentReadBytesGluePB* pb = (AudioFileComponentReadBytesGluePB*)inParameters;
						
						result = inThis->AFAPI_ReadBytes(pb->inUseCache, *pb->inStartingByte, pb->ioNumBytes,
							pb->outBuffer);
					}
					break;
				case kAudioFileWriteBytesSelect:
					{
						AudioFileComponentWriteBytesGluePB* pb = (AudioFileComponentWriteBytesGluePB*)inParameters;
						
						result = inThis->AFAPI_WriteBytes(pb->inUseCache, *pb->inStartingByte, pb->ioNumBytes,
							pb->inBuffer);
					}
					break;
				case kAudioFileReadPacketsSelect:
					{
						AudioFileComponentReadPacketsGluePB* pb = (AudioFileComponentReadPacketsGluePB*)inParameters;
						
						result = inThis->AFAPI_ReadPackets(pb->inUseCache, pb->outNumBytes, pb->outPacketDescriptions, 
							*pb->inStartingPacket, pb->ioNumPackets, pb->outBuffer);
					}
					break;
				case kAudioFileWritePacketsSelect:
					{
						AudioFileComponentWritePacketsGluePB* pb = (AudioFileComponentWritePacketsGluePB*)inParameters;
						
						result = inThis->AFAPI_WritePackets(pb->inUseCache, pb->inNumBytes, pb->inPacketDescriptions, 
							*pb->inStartingPacket, pb->ioNumPackets, pb->inBuffer);
					}
					break;
					
				case kAudioFileGetPropertyInfoSelect:
					{
						AudioFileComponentGetPropertyInfoGluePB* pb = (AudioFileComponentGetPropertyInfoGluePB*)inParameters;
						
						result = inThis->AFAPI_GetPropertyInfo(pb->inPropertyID, pb->outPropertySize, pb->outWritable);
					}
					break;
					
				case kAudioFileGetPropertySelect:
					{
						AudioFileComponentGetPropertyGluePB* pb = (AudioFileComponentGetPropertyGluePB*)inParameters;
						
						result = inThis->AFAPI_GetProperty(pb->inPropertyID, pb->ioPropertyDataSize, pb->outPropertyData);
					}
					break;
				case kAudioFileSetPropertySelect:
					{
						AudioFileComponentSetPropertyGluePB* pb = (AudioFileComponentSetPropertyGluePB*)inParameters;
						
						result = inThis->AFAPI_SetProperty(pb->inPropertyID, pb->inPropertyDataSize, pb->inPropertyData);
					}
					break;
					
				case kAudioFileGetGlobalInfoSizeSelect:
					{
						AudioFileComponentGetGlobalInfoSizeGluePB* pb = (AudioFileComponentGetGlobalInfoSizeGluePB*)inParameters;
						
						result = inThis->AFAPI_GetGlobalInfoSize(pb->inPropertyID, pb->inSpecifierSize, pb->inSpecifier,
							pb->outPropertyDataSize);
					}
					break;
				case kAudioFileGetGlobalInfoSelect:
					{
						AudioFileComponentGetGlobalInfoGluePB* pb = (AudioFileComponentGetGlobalInfoGluePB*)inParameters;
						
						result = inThis->AFAPI_GetGlobalInfo(pb->inPropertyID, pb->inSpecifierSize, pb->inSpecifier,
							pb->ioPropertyDataSize, pb->outPropertyData);
					}
					break;
					
				case kAudioFileExtensionIsThisFormatSelect:
					{
						AudioFileComponentExtensionIsThisFormatGluePB* pb = (AudioFileComponentExtensionIsThisFormatGluePB*)inParameters;
						
						AudioFileFormatBase* aff = inThis->GetAudioFileFormatBase();
						if (!aff) return paramErr;
						
						UInt32 res = aff->ExtensionIsThisFormat(pb->inExtension);
						if (pb->outResult) *pb->outResult = res;
					}
					break;
				case kAudioFileDataIsThisFormatSelect:
					{
						AudioFileComponentDataIsThisFormatGluePB* pb = (AudioFileComponentDataIsThisFormatGluePB*)inParameters;
						
						AudioFileFormatBase* aff = inThis->GetAudioFileFormatBase();
						if (!aff) return paramErr;
						
						UncertainResult res = aff->DataIsThisFormat(pb->inRefCon, 
									pb->inReadFunc, pb->inWriteFunc, pb->inGetSizeFunc, pb->inSetSizeFunc);
						if (pb->outResult) *pb->outResult = res;
					}
					break;
				case kAudioFileFileIsThisFormatSelect:
					{
						AudioFileComponentFileIsThisFormatGluePB* pb = (AudioFileComponentFileIsThisFormatGluePB*)inParameters;
						
						AudioFileFormatBase* aff = inThis->GetAudioFileFormatBase();
						if (!aff) return paramErr;
						
						UncertainResult res = aff->FileIsThisFormat(pb->inFileRefNum);
						if (pb->outResult) *pb->outResult = res;
					}
					break;

				case kAudioFileCountUserDataSelect:
					{
						AudioFileComponentCountUserDataGluePB* pb = (AudioFileComponentCountUserDataGluePB*)inParameters;
						
						result = inThis->AFAPI_CountUserData(pb->inUserDataID, pb->outNumberItems);
					}
					break;
					
				case kAudioFileGetUserDataSizeSelect:
					{
						AudioFileComponentGetUserDataSizeGluePB* pb = (AudioFileComponentGetUserDataSizeGluePB*)inParameters;
						
						result = inThis->AFAPI_GetUserDataSize(pb->inUserDataID, pb->inIndex, pb->outUserDataSize);
					}
					break;
					
				case kAudioFileGetUserDataSelect:
					{
						AudioFileComponentGetUserDataGluePB* pb = (AudioFileComponentGetUserDataGluePB*)inParameters;
						
						result = inThis->AFAPI_GetUserData(pb->inUserDataID, pb->inIndex, 
										pb->ioUserDataSize, pb->outUserData);
					}
					break;
					
				case kAudioFileSetUserDataSelect:
					{
						AudioFileComponentSetUserDataGluePB* pb = (AudioFileComponentSetUserDataGluePB*)inParameters;
						
						result = inThis->AFAPI_SetUserData(pb->inUserDataID, pb->inIndex, 
										pb->inUserDataSize, pb->inUserData);
					}
					break;
	
				case kAudioFileRemoveUserDataSelect:
					{
						AudioFileComponentRemoveUserDataGluePB* pb = (AudioFileComponentRemoveUserDataGluePB*)inParameters;
						
						result = inThis->AFAPI_RemoveUserData(pb->inUserDataID, pb->inIndex);
					}
					break;
	
		
				default:
					result = ComponentBase::ComponentEntryDispatch(inParameters, inThis);
					break;
		}
	}
	COMPONENT_CATCH
	return result;
} 
	
