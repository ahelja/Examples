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
	AudioFileObject.h
	
=============================================================================*/


#ifndef _AudioFileObject_H_
#define _AudioFileObject_H_

#include <TargetConditionals.h>

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreServices/CoreServices.h>
	#include <CoreAudio/CoreAudioTypes.h>
	#include <AudioToolbox/AudioFile.h>
#else
	#include "CoreAudioTypes.h"
	#include "AudioFile.h"
#endif

#include "CACFDictionary.h"
#include "DataSource.h"
#include <vector>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// some files encode these as upper case
enum {
  kUpperCase_IMACompression               = 'IMA4', /*IMA 4:1*/
  kUpperCase_ULawCompression              = 'ULAW', /*µLaw 2:1*/
  kUpperCase_ALawCompression              = 'ALAW', /*aLaw 2:1*/
  
  kUpperCase_Float32					  = 'FL32',
  kUpperCase_Float64					  = 'FL64'
};

enum
{
	// in what header is this defined? what is it?
	kGSM = 'agsm', 
	kUpperCase_GSM = 'GSM '
};

#define	kPackedBESInt	(kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsBigEndian | kLinearPCMFormatFlagIsPacked)
#define	kPackedLESInt	(kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked)
#define kPackedBEFloat	(kLinearPCMFormatFlagIsFloat | kLinearPCMFormatFlagIsBigEndian | kLinearPCMFormatFlagIsPacked)
#define kPackedLEFloat	(kLinearPCMFormatFlagIsFloat | kLinearPCMFormatFlagIsPacked)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct  AudioStreamPacketDescriptionExtended : AudioStreamPacketDescription
{
    SInt64  mFrameOffset; // this is the sum of the mVariableFramesInPacket up to this point so we can binary search.
};
typedef struct AudioStreamPacketDescriptionExtended AudioStreamPacketDescriptionExtended;

// used to make a packet table for formats that need one (i.e. MP3 data)
typedef std::vector<AudioStreamPacketDescriptionExtended> PacketTable;

inline bool operator < (const AudioStreamPacketDescriptionExtended& a, const AudioStreamPacketDescriptionExtended& b)
{
	return a.mFrameOffset < b.mFrameOffset;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class AudioFileObject
{
private:
	SInt64							mNumBytes;				// total bytes of audio data in the audio file
	SInt64							mNumPackets;			// total frames of audio data in the audio file
	AudioStreamBasicDescription		mDataFormat;		// format of the audio data
	SInt64							mDataOffset;		// position if the file where audio data begins
	UInt32							mIsOptimized;		// 1 if there is nothing in the file following the audio data, 0 if there is
	UInt32							mFileType;			// file type of the audio file (i.e. AIFF, WAVE, etc.)			
	FSRef 							mFSRef;				// FSRef of the file passed to AudioFileOpen or AudioFileCreate
	SInt16							mFileNum;			// Ref num of the file after opening within Audio File 
	SInt8							mPermissions;		// file permissions indicated by the caller, passed by AudioFileOpen or set with SetProperty function
	Boolean							mIsInitialized;		// has the AudioFileObject for this file been intialized?
	DataSource						*mDataSource;
    UInt32							mMaximumPacketSize;
    PacketTable						*mPacketTable;
	UInt32							mDeferSizeUpdates;
	Boolean							mNeedsSizeUpdate;
	Boolean							mFirstSetFormat;
	
public:    
  
	AudioFileObject (UInt32 inFileType)
		: mNumBytes(0),
		  mNumPackets(0),
		  mDataOffset(0),
		  mIsOptimized(1),
		  mFileType(inFileType),
		  mFileNum(0),
		  mPermissions(0),
		  mIsInitialized (false),
		  mDataSource(0),
          mMaximumPacketSize(0),
          mPacketTable(NULL),
		  mDeferSizeUpdates(1),
		  mNeedsSizeUpdate(false),
		  mFirstSetFormat(true)
		  {
			memset(&mDataFormat, 0, sizeof(mDataFormat));
			memset(&mFSRef, 0, sizeof(mFSRef));
		  }
	
	virtual ~AudioFileObject();

	
/* Public API Function Implementation */
	// The DoSomething() versions of these functions are wrappers that perform a standard prologue.
	// The Something() methods are those which should be overridden in the subclasses.
	
	OSStatus DoCreate(				const FSRef							*inFileRef,
									CFStringRef							inFileName,
									const AudioStreamBasicDescription	*inFormat,
									UInt32								inFlags,
									FSRef								*outNewFileRef);
                                
	virtual OSStatus Create(		const FSRef							*inFileRef,
									CFStringRef							inFileName,
									const AudioStreamBasicDescription	*inFormat,
									FSRef								*outNewFileRef);
                                
	OSStatus DoOpen(				const FSRef		*inFileRef, 
									SInt8  			inPermissions,
									SInt16			inRefNum);
									
	virtual OSStatus Open(			const FSRef		*inFileRef, 
									SInt8  			inPermissions,
									SInt16			inRefNum);
									
	OSStatus DoInitialize(			const FSRef							*inFileRef,
									const AudioStreamBasicDescription	*inFormat,
									UInt32								inFlags);
	
	virtual OSStatus Initialize(	const FSRef							*inFileRef,
									const AudioStreamBasicDescription	*inFormat,
									UInt32								inFlags);
	
	OSStatus DoOpenWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc);
				
	OSStatus DoInitializeWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc,
				UInt32								inFileType,
				const AudioStreamBasicDescription	*inFormat,
				UInt32								inFlags);
				
	virtual OSStatus OpenFromDataSource(SInt8 inPermissions);
						
	virtual OSStatus InitializeDataSource(const AudioStreamBasicDescription	*inFormat, UInt32 inFlags);
	
	OSStatus DoClose();
	
	virtual OSStatus Close();
	
	OSStatus DoOptimize();
	
	virtual OSStatus Optimize();
	
	virtual OSStatus ReadBytes(		Boolean			inUseCache,
									SInt64			inStartingByte, 
									UInt32			*ioNumBytes, 
									void			*outBuffer);
	
	virtual OSStatus WriteBytes(	Boolean			inUseCache,
									SInt64			inStartingByte, 
									UInt32			*ioNumBytes, 
									const void		*inBuffer);
	
	virtual OSStatus ReadPackets(	Boolean							inUseCache,
									UInt32							*outNumBytes,
									AudioStreamPacketDescription	*outPacketDescriptions,
									SInt64							inStartingPacket, 
									UInt32  						*ioNumPackets, 
									void							*outBuffer);
	
	virtual OSStatus WritePackets(	Boolean							inUseCache,
									UInt32							inNumBytes,
									AudioStreamPacketDescription	*inPacketDescriptions,
									SInt64							inStartingPacket, 
									UInt32  						*ioNumPackets, 
									const void						*inBuffer);
/* Property Support */

	virtual OSStatus GetPropertyInfo	(	
                                        AudioFilePropertyID		inPropertyID,
                                        UInt32					*outDataSize,
                                        UInt32					*isWritable);
										
	virtual OSStatus GetProperty	(	AudioFilePropertyID		inPropertyID,
										UInt32					*ioDataSize,
										void					*ioPropertyData);
										
	virtual OSStatus SetProperty	(	AudioFilePropertyID		inPropertyID,
										UInt32					inDataSize,
										const void				*inPropertyData);
										
	UInt32 GetFileType() const { return mFileType; }
	void SetFileType(UInt32 inFileType) { mFileType = inFileType; }
	
	// this will set the format in memory only.
	virtual OSStatus SetDataFormat(const AudioStreamBasicDescription* inStreamFormat);
	
	// this will update the format info on disk and in memory.
	virtual OSStatus UpdateDataFormat(const AudioStreamBasicDescription* inStreamFormat);
	
	const AudioStreamBasicDescription &GetDataFormat() const { return mDataFormat; }
	
	virtual OSStatus UpdateSize() { return noErr; }
	UInt32	DeferSizeUpdates() { return mDeferSizeUpdates; }
	void SetDeferSizeUpdates(UInt32 inFlag) { mDeferSizeUpdates = inFlag; }
	OSStatus UpdateSizeIfNeeded();
	OSStatus SizeChanged();
	
	Boolean IsOptimized() const { return mIsOptimized != 0; }
	void SetIsOptimized(Boolean inIsOptimized) { mIsOptimized = inIsOptimized ? 1 : 0; }

	virtual SInt64 GetNumBytes() { return mNumBytes; }
	virtual void SetNumBytes(SInt64 inNumBytes) { mNumBytes = inNumBytes; }

	// this will update the header size info on disk
	OSStatus UpdateNumBytes(SInt64 inNumBytes);
	
	virtual SInt64 GetNumPackets(){ return mNumPackets; }
	virtual void SetNumPackets(SInt64 inNumPackets) { mNumPackets = inNumPackets; }
	
	// this will update the header size info on disk
	OSStatus UpdateNumPackets(SInt64 inNumPackets);

	virtual OSStatus PacketToFrame(SInt64 inPacket, SInt64& outFirstFrameInPacket);
	virtual OSStatus FrameToPacket(SInt64 inFrame, SInt64& outPacket, UInt32& outFrameOffsetInPacket);
	
	
	virtual OSStatus GetMagicCookieDataSize(	UInt32					*outDataSize,
												UInt32					*isWritable);
	                      
	virtual OSStatus GetMagicCookieData(		UInt32					*ioDataSize,
												void					*ioPropertyData);
	                      
	virtual OSStatus SetMagicCookieData(		UInt32					inDataSize,
												const void				*inPropertyData);

	virtual OSStatus GetMarkerListSize(			UInt32						*outDataSize,
												UInt32						*isWritable);
	                      
	virtual OSStatus GetMarkerList(				UInt32						*ioDataSize,
												AudioFileMarkerList			*ioMarkerList);
	                      
	virtual OSStatus SetMarkerList(				UInt32						inDataSize,
												const AudioFileMarkerList	*inMarkerList);


	virtual OSStatus GetRegionListSize(			UInt32						*outDataSize,
												UInt32						*isWritable);
	                      
	virtual OSStatus GetRegionList(				UInt32						*ioDataSize,
												AudioFileRegionList			*ioRegionList);
	                      
	virtual OSStatus SetRegionList(				UInt32						inDataSize,
												const AudioFileRegionList	*inRegionList);

	virtual OSStatus GetChannelLayoutSize(		UInt32						*outDataSize,
												UInt32						*isWritable);
	                      
	virtual OSStatus GetChannelLayout(			UInt32						*ioDataSize,
												AudioChannelLayout			*ioChannelLayout);
	                      
	virtual OSStatus SetChannelLayout(			UInt32						inDataSize,
												const AudioChannelLayout	*inChannelLayout);

	virtual OSStatus GetInfoDictionarySize(		UInt32						*outDataSize,
												UInt32						*isWritable);
												
	virtual OSStatus GetInfoDictionary(			CACFDictionary  *infoDict);
												
	virtual OSStatus SetInfoDictionary(			CACFDictionary  *infoDict);

	virtual OSStatus CountUserData(			UInt32					inUserDataID,
											UInt32					*outNumberItems);
											
	virtual OSStatus GetUserDataSize(		UInt32					inUserDataID,
											UInt32					inIndex,
											UInt32					*outDataSize);
											
	virtual OSStatus GetUserData(			UInt32					inUserDataID,
											UInt32					inIndex,
											UInt32					*ioDataSize,
											void					*ioUserData);
											
	virtual OSStatus SetUserData(			UInt32					inUserDataID,
											UInt32					inIndex,
											UInt32					inDataSize,
											const void				*inUserData);

	virtual OSStatus RemoveUserData(		UInt32					inUserDataID,
											UInt32					inIndex);

	Boolean CanRead() const { return mPermissions & fsRdPerm; }
	Boolean CanWrite() const { return mPermissions & fsWrPerm; }

	void SetPermissions(SInt8 inPermissions) { mPermissions = inPermissions; }
		
/* Other Helper Methods: (some may not be necessary depending on how things are refactored) */
	
	OSStatus OpenFile(SInt8 inPermissions, SInt16 inRefNum);
	
	OSStatus CreateDataFile(const FSRef* inParentFileRef, CFStringRef inFileName, FSRef *outRef);
	
	OSStatus CreateResourceFile(const FSRef* inParentFileRef, CFStringRef inFileName, FSRef *outRef);

	virtual Boolean IsDataFormatSupported(const AudioStreamBasicDescription	*inFormat) = 0;
	virtual Boolean IsDataFormatValid(const AudioStreamBasicDescription	*inFormat);

	virtual OSStatus IsValidFilePosition(SInt64 /*position*/) { return noErr; }

/* Accessors: */
	Boolean IsInitialized() const { return mIsInitialized; }
	void SetInitialized(Boolean inFlag) { mIsInitialized = inFlag; }
		
	SInt16 GetFileNum() const { return mFileNum; }
	
	DataSource* GetDataSource() const { return mDataSource; }
	void SetDataSource(DataSource *inDataSource);

	void SetFileNum(SInt16 inFileNum) { mFileNum = inFileNum; }
	
	FSRef GetFSRef() const { return mFSRef; }
	void SetFSRef(const FSRef* inRef) { mFSRef = *inRef; }
    
    UInt32	GetMaximumPacketSize () const {return mMaximumPacketSize;}
    void	SetMaximumPacketSize (const UInt32 inPacketSize) { mMaximumPacketSize = inPacketSize; }
    	
	SInt64 GetDataOffset() const { return mDataOffset; }	
	void SetDataOffset(SInt64 inOffset) { mDataOffset = inOffset; }	
    
	// I like this idiom better than DoesPacketTableExist+NewPacketTable:
	PacketTable* GetPacketTable(Boolean inCreateIt = false)
	{
		if (!mPacketTable && inCreateIt) 
			mPacketTable = new PacketTable;
		return mPacketTable;
	}
	void ClearPacketTable() 
	{
		DeletePacketTable();
		GetPacketTable(true);
	}
	
    void AppendPacket(AudioStreamPacketDescription &inPacket) 
		{
			PacketTable* packetTable = GetPacketTable(true);
			UInt32 numFramesInPacket = mDataFormat.mFramesPerPacket ? mDataFormat.mFramesPerPacket : inPacket.mVariableFramesInPacket;
			
			AudioStreamPacketDescriptionExtended pext;
			memset(&pext, 0, sizeof(pext));
			pext.mStartOffset = inPacket.mStartOffset;
			pext.mDataByteSize = inPacket.mDataByteSize;
			pext.mVariableFramesInPacket = inPacket.mVariableFramesInPacket;
			pext.mFrameOffset = numFramesInPacket + (packetTable->size() ? packetTable->back().mFrameOffset : 0);
			
			packetTable->push_back(pext); 
			if (inPacket.mDataByteSize > mMaximumPacketSize) 
				mMaximumPacketSize = inPacket.mDataByteSize;
		}
    void DeletePacketTable() { delete mPacketTable; mPacketTable = NULL;}
    SInt64	GetPacketCount() { return mPacketTable ? mPacketTable->size() : 0; }
    OSStatus GetPacketDescriptions(UInt32   inStartingPacket, UInt32   *ioDataSize, AudioStreamPacketDescription    *outPacketDescriptions)
        {
			if (outPacketDescriptions == NULL) return kAudioFileUnspecifiedError;
            if (mPacketTable)
			{
				// only get as many packet descriptions as can fit in outPacketDescriptions
				UInt32	count = *ioDataSize / sizeof(AudioStreamPacketDescription);
				if (count + inStartingPacket  > GetPacketCount()) count = GetPacketCount() - inStartingPacket;
					
				*ioDataSize = 0;
				for (UInt32 i = inStartingPacket; i < (count + inStartingPacket); i++)
				{
					AudioStreamPacketDescription    curPacket =  (*mPacketTable)[i];
					outPacketDescriptions[i].mStartOffset = curPacket.mStartOffset - GetDataOffset();
					outPacketDescriptions[i].mVariableFramesInPacket = curPacket.mVariableFramesInPacket;
					outPacketDescriptions[i].mDataByteSize = curPacket.mDataByteSize;
					*ioDataSize += sizeof(AudioStreamPacketDescription);
				}
			}
			else
				*ioDataSize = 0;

            return noErr;
        }
	
	Boolean GetNeedsSizeUpdate() const { return mNeedsSizeUpdate; }
	void SetNeedsSizeUpdate(Boolean inNeedsSizeUpdate) { mNeedsSizeUpdate = inNeedsSizeUpdate; }
	
/* debug */
//	virtual void PrintFile (FILE* inFile, const char *indent) = 0;
	
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline Boolean cfstrcmpi(CFStringRef a, CFStringRef b)
{
	// case insensitive CFString compare
	return CFStringCompare(a, b, kCFCompareCaseInsensitive) == kCFCompareEqualTo;
}

CFBundleRef GetAudioToolboxBundle();


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#endif
