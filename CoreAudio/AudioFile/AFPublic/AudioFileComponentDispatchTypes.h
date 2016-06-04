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
	AudioFileComponentDispatchTypes.h
	
=============================================================================*/

#if !defined(__AudioFileComponentDispatchTypes_h__)
#define __AudioFileComponentDispatchTypes_h__

//=============================================================================
//	Includes
//=============================================================================

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif



#define	AudioFileComponentStandardGluePBFields	\
	UInt8 componentFlags; \
	UInt8 componentParamSize; \
	SInt16 componentWhat;

//=============================================================================


#if	!TARGET_OS_WIN32
struct AudioFileComponentCreateGluePB
{
	AudioFileComponentStandardGluePBFields;
	FSRef								*outNewFileRef;
	UInt32								inFlags;
	const AudioStreamBasicDescription	*inFormat;
	CFStringRef							inFileName;
	const FSRef							*inParentRef; 
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentCreateGluePB
{
	AudioFileComponentStandardGluePBFields;
	const FSRef							*inParentRef; 
	CFStringRef							inFileName;
	const AudioStreamBasicDescription	*inFormat;
	UInt32								inFlags;
	FSRef								*outNewFileRef;
};
#endif
typedef struct AudioFileComponentCreateGluePB	AudioFileComponentCreateGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentInitializeGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32								inFlags;
	const AudioStreamBasicDescription	*inFormat;
	const FSRef							*inFileRef;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentInitializeGluePB
{
	AudioFileComponentStandardGluePBFields;
	const FSRef							*inFileRef;
	const AudioStreamBasicDescription	*inFormat;
	UInt32								inFlags;
};
#endif
typedef struct AudioFileComponentInitializeGluePB	AudioFileComponentInitializeGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentOpenGluePB
{
	AudioFileComponentStandardGluePBFields;
	SInt32  							inRefNum; 
	SInt32  							inPermissions; 
	const FSRef							*inFileRef;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentOpenGluePB
{
	AudioFileComponentStandardGluePBFields;
	const FSRef							*inFileRef; 
	SInt32  							inPermissions; 
	SInt32  							inRefNum; 
};
#endif
typedef struct AudioFileComponentOpenGluePB	AudioFileComponentOpenGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentOpenWithCallbacksGluePB
{
	AudioFileComponentStandardGluePBFields;
	AudioFile_SetSizeProc				inSetSizeFunc;
	AudioFile_GetSizeProc				inGetSizeFunc;
	AudioFile_WriteProc					inWriteFunc; 
	AudioFile_ReadProc					inReadFunc; 
	void *								inRefCon; 
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentOpenWithCallbacksGluePB
{
	AudioFileComponentStandardGluePBFields;
	void *								inRefCon; 
	AudioFile_ReadProc					inReadFunc; 
	AudioFile_WriteProc					inWriteFunc; 
	AudioFile_GetSizeProc				inGetSizeFunc;
	AudioFile_SetSizeProc				inSetSizeFunc;
};
#endif
typedef struct AudioFileComponentOpenWithCallbacksGluePB	AudioFileComponentOpenWithCallbacksGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentInitializeWithCallbacksGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32								inFlags;
	const AudioStreamBasicDescription	*inFormat;
	UInt32								inFileType;
	AudioFile_SetSizeProc				inSetSizeFunc;
	AudioFile_GetSizeProc				inGetSizeFunc;
	AudioFile_WriteProc					inWriteFunc; 
	AudioFile_ReadProc					inReadFunc; 
	void *								inRefCon; 
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentInitializeWithCallbacksGluePB
{
	AudioFileComponentStandardGluePBFields;
	void *								inRefCon; 
	AudioFile_ReadProc					inReadFunc; 
	AudioFile_WriteProc					inWriteFunc; 
	AudioFile_GetSizeProc				inGetSizeFunc;
	AudioFile_SetSizeProc				inSetSizeFunc;
	UInt32								inFileType;
	const AudioStreamBasicDescription	*inFormat;
	UInt32								inFlags;
};
#endif
typedef struct AudioFileComponentInitializeWithCallbacksGluePB	AudioFileComponentInitializeWithCallbacksGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentCloseGluePB
{
	AudioFileComponentStandardGluePBFields;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentCloseGluePB
{
	AudioFileComponentStandardGluePBFields;
};
#endif
typedef struct AudioFileComponentCloseGluePB	AudioFileComponentCloseGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentOptimizeGluePB
{
	AudioFileComponentStandardGluePBFields;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentOptimizeGluePB
{
	AudioFileComponentStandardGluePBFields;
};
#endif
typedef struct AudioFileComponentOptimizeGluePB	AudioFileComponentOptimizeGluePB;



#if	!TARGET_OS_WIN32
struct AudioFileComponentReadBytesGluePB
{
	AudioFileComponentStandardGluePBFields;
	void							*outBuffer;
	UInt32							*ioNumBytes; 
	SInt64							*inStartingByte; 
	UInt32							inUseCache;
	AudioFileComponent				inComponent;
};
#else
struct AudioFileComponentReadBytesGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32							inUseCache;
	SInt64							*inStartingByte; 
	UInt32							*ioNumBytes; 
	void							*outBuffer;
};
#endif
typedef struct AudioFileComponentReadBytesGluePB	AudioFileComponentReadBytesGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentWriteBytesGluePB
{
	AudioFileComponentStandardGluePBFields;
	const void						*inBuffer;
	UInt32							*ioNumBytes; 
	SInt64							*inStartingByte; 
	UInt32							inUseCache;
	AudioFileComponent				inComponent;
};
#else
struct AudioFileComponentWriteBytesGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32							inUseCache;
	SInt64							*inStartingByte; 
	UInt32							*ioNumBytes; 
	const void						*inBuffer;
};
#endif
typedef struct AudioFileComponentWriteBytesGluePB	AudioFileComponentWriteBytesGluePB;



#if	!TARGET_OS_WIN32
struct AudioFileComponentReadPacketsGluePB
{
	AudioFileComponentStandardGluePBFields;
	void							*outBuffer;
	UInt32							*ioNumPackets; 
	SInt64							*inStartingPacket; 
	AudioStreamPacketDescription	*outPacketDescriptions;
	UInt32							*outNumBytes;
	UInt32							inUseCache;
	AudioFileComponent				inComponent;
};
#else
struct AudioFileComponentReadPacketsGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32							inUseCache;
	UInt32							*outNumBytes;
	AudioStreamPacketDescription	*outPacketDescriptions;
	SInt64							*inStartingPacket; 
	UInt32							*ioNumPackets; 
	void							*outBuffer;
};
#endif
typedef struct AudioFileComponentReadPacketsGluePB	AudioFileComponentReadPacketsGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentWritePacketsGluePB
{
	AudioFileComponentStandardGluePBFields;
	const void						*inBuffer;
	UInt32							*ioNumPackets; 
	SInt64							*inStartingPacket; 
	AudioStreamPacketDescription	*inPacketDescriptions;
	UInt32							inNumBytes;
	UInt32							inUseCache;
	AudioFileComponent				inComponent;
};
#else
struct AudioFileComponentWritePacketsGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32							inUseCache;
	UInt32							inNumBytes;
	AudioStreamPacketDescription	*inPacketDescriptions;
	SInt64							*inStartingPacket; 
	UInt32							*ioNumPackets; 
	const void						*inBuffer;
};
#endif
typedef struct AudioFileComponentWritePacketsGluePB	AudioFileComponentWritePacketsGluePB;


//=============================================================================


#if	!TARGET_OS_WIN32
struct AudioFileComponentGetPropertyInfoGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32*								outWritable;
	UInt32*								outPropertySize;
	AudioFileComponentPropertyID		inPropertyID;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentGetPropertyInfoGluePB
{
	AudioFileComponentStandardGluePBFields;
	AudioFileComponentPropertyID		inPropertyID;
	UInt32*								outPropertySize;
	UInt32*								outWritable;
};
#endif
typedef struct AudioFileComponentGetPropertyInfoGluePB	AudioFileComponentGetPropertyInfoGluePB;

#if	!TARGET_OS_WIN32
struct AudioFileComponentGetPropertyGluePB
{
	AudioFileComponentStandardGluePBFields;
	void*								outPropertyData;
	UInt32*								ioPropertyDataSize;
	AudioFileComponentPropertyID		inPropertyID;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentGetPropertyGluePB
{
	AudioFileComponentStandardGluePBFields;
	AudioFileComponentPropertyID		inPropertyID;
	UInt32*								ioPropertyDataSize;
	void*								outPropertyData;
};
#endif
typedef struct AudioFileComponentGetPropertyGluePB	AudioFileComponentGetPropertyGluePB;

#if	!TARGET_OS_WIN32
struct AudioFileComponentSetPropertyGluePB
{
	AudioFileComponentStandardGluePBFields;
	const void*							inPropertyData;
	UInt32								inPropertyDataSize;
	AudioFileComponentPropertyID		inPropertyID;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentSetPropertyGluePB
{
	AudioFileComponentStandardGluePBFields;
	AudioFileComponentPropertyID		inPropertyID;
	UInt32								inPropertyDataSize;
	const void*							inPropertyData;
};
#endif
typedef struct AudioFileComponentSetPropertyGluePB	AudioFileComponentSetPropertyGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentCountUserDataGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32*								outNumberItems;
	UInt32								inUserDataID;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentCountUserDataGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32								inUserDataID;
	UInt32*								outNumberItems;
};
#endif
typedef struct AudioFileComponentCountUserDataGluePB	AudioFileComponentCountUserDataGluePB;

#if	!TARGET_OS_WIN32
struct AudioFileComponentGetUserDataSizeGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32*								outUserDataSize;
	UInt32								inIndex;
	UInt32								inUserDataID;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentGetUserDataSizeGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32								inUserDataID;
	UInt32								inIndex;
	UInt32*								outUserDataSize;
};
#endif
typedef struct AudioFileComponentGetUserDataSizeGluePB	AudioFileComponentGetUserDataSizeGluePB;

#if	!TARGET_OS_WIN32
struct AudioFileComponentGetUserDataGluePB
{
	AudioFileComponentStandardGluePBFields;
	void*								outUserData;
	UInt32*								ioUserDataSize;
	UInt32								inIndex;
	UInt32								inUserDataID;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentGetUserDataGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32								inUserDataID;
	UInt32								inIndex;
	UInt32*								ioUserDataSize;
	void*								outUserData;
};
#endif
typedef struct AudioFileComponentGetUserDataGluePB	AudioFileComponentGetUserDataGluePB;

#if	!TARGET_OS_WIN32
struct AudioFileComponentSetUserDataGluePB
{
	AudioFileComponentStandardGluePBFields;
	const void*							inUserData;
	UInt32								inUserDataSize;
	UInt32								inIndex;
	UInt32								inUserDataID;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentSetUserDataGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32								inUserDataID;
	UInt32								inIndex;
	UInt32								inUserDataSize;
	const void*							inUserData;
};
#endif
typedef struct AudioFileComponentSetUserDataGluePB	AudioFileComponentSetUserDataGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentRemoveUserDataGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32								inIndex;
	UInt32								inUserDataID;
	AudioFileComponent					inComponent;
};
#else
struct AudioFileComponentRemoveUserDataGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32								inUserDataID;
	UInt32								inIndex;
};
#endif
typedef struct AudioFileComponentSetUserDataGluePB	AudioFileComponentSetUserDataGluePB;



#if	!TARGET_OS_WIN32
struct AudioFileComponentDataIsThisFormatGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32							*outResult;
	AudioFile_SetSizeProc			inSetSizeFunc;
	AudioFile_GetSizeProc			inGetSizeFunc;
	AudioFile_WriteProc				inWriteFunc; 
	AudioFile_ReadProc				inReadFunc; 
	void							*inRefCon; 
	AudioFileComponent				inComponent;
};
#else
struct AudioFileComponentDataIsThisFormatGluePB
{
	AudioFileComponentStandardGluePBFields;
	void							*inRefCon; 
	AudioFile_ReadProc				inReadFunc; 
	AudioFile_WriteProc				inWriteFunc; 
	AudioFile_GetSizeProc			inGetSizeFunc;
	AudioFile_SetSizeProc			inSetSizeFunc;
	UInt32							*outResult;
};
#endif
typedef struct AudioFileComponentDataIsThisFormatGluePB	AudioFileComponentDataIsThisFormatGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentFileIsThisFormatGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32							*outResult;
	SInt32							inFileRefNum; 
	AudioFileComponent				inComponent;
};
#else
struct AudioFileComponentFileIsThisFormatGluePB
{
	AudioFileComponentStandardGluePBFields;
	SInt32							inFileRefNum;
	UInt32							*outResult;
};
#endif
typedef struct AudioFileComponentFileIsThisFormatGluePB	AudioFileComponentFileIsThisFormatGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentExtensionIsThisFormatGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32							*outResult;
	CFStringRef 					inExtension; 
	AudioFileComponent				inComponent;
};
#else
struct AudioFileComponentExtensionIsThisFormatGluePB
{
	AudioFileComponentStandardGluePBFields;
	CFStringRef 					inExtension;
	UInt32							*outResult;
};
#endif
typedef struct AudioFileComponentExtensionIsThisFormatGluePB AudioFileComponentExtensionIsThisFormatGluePB;


#if	!TARGET_OS_WIN32
struct AudioFileComponentGetGlobalInfoSizeGluePB
{
	AudioFileComponentStandardGluePBFields;
	UInt32							*outPropertyDataSize;
	const void						*inSpecifier;
	UInt32							inSpecifierSize;
	AudioFileComponentPropertyID	inPropertyID;
	AudioFileComponent				inComponent;
};
#else
struct AudioFileComponentGetGlobalInfoSizeGluePB
{
	AudioFileComponentStandardGluePBFields;
	AudioFileComponentPropertyID	inPropertyID;
	UInt32							inSpecifierSize;
	const void						*inSpecifier;
	UInt32							*outPropertyDataSize;
};
#endif
typedef struct AudioFileComponentGetGlobalInfoSizeGluePB AudioFileComponentGetGlobalInfoSizeGluePB;

#if	!TARGET_OS_WIN32
struct AudioFileComponentGetGlobalInfoGluePB
{
	AudioFileComponentStandardGluePBFields;
	void							*outPropertyData;
	UInt32							*ioPropertyDataSize;
	const void						*inSpecifier;
	UInt32							inSpecifierSize;
	AudioFileComponentPropertyID	inPropertyID;
	AudioFileComponent				inComponent;
};
#else
struct AudioFileComponentGetGlobalInfoGluePB
{
	AudioFileComponentStandardGluePBFields;
	AudioFileComponentPropertyID	inPropertyID;
	UInt32							inSpecifierSize;
	const void						*inSpecifier;
	UInt32							*ioPropertyDataSize;
	void							*outPropertyData;
};
#endif
typedef struct AudioFileComponentGetGlobalInfoGluePB AudioFileComponentGetGlobalInfoGluePB;



#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif


//=============================================================================
//=============================================================================

#endif

