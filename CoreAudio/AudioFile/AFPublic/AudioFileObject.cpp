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
	AudioFileObject.cpp
	
=============================================================================*/

#include "AudioFileObject.h"
#include "CADebugMacros.h"
#include <algorithm>

#define USE_CACHED_DATASOURCE 1

//////////////////////////////////////////////////////////////////////////////////////////////////////////

AudioFileObject::~AudioFileObject()
{
	delete mDataSource;
	DeletePacketTable();
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::DoCreate(		
									const FSRef							*inFileRef,
									CFStringRef							inFileName,
									const AudioStreamBasicDescription	*inFormat,
									UInt32								inFlags,
									FSRef								*outNewFileRef)
{
	// common prologue
	if (!IsDataFormatValid(inFormat))
	{
		return kAudioFileUnsupportedDataFormatError;
	}
	if (!IsDataFormatSupported(inFormat)) 
	{
		return kAudioFileUnsupportedDataFormatError;
	}
	SetPermissions(fsRdWrPerm);
	
	// call virtual method for particular format.
	return Create(inFileRef, inFileName, inFormat, outNewFileRef);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::Create(		
									const FSRef							*inFileRef,
									CFStringRef							inFileName,
									const AudioStreamBasicDescription	*inFormat,
									FSRef								*outNewFileRef)
{
	OSStatus err;
		
	FSRef fsRef;
	err = CreateDataFile(inFileRef, inFileName, &fsRef);
    FailIf (err != noErr, Bail, "CreateDataFile failed");
	
	SetFSRef(&fsRef);
	if (outNewFileRef) *outNewFileRef = fsRef;

	err = SetDataFormat(inFormat);
    FailIf (err != noErr, Bail, "SetDataFormat failed");
	
	SInt16 refNum;
    err = FSOpenFork (&fsRef, 0, NULL, fsRdWrPerm, &refNum);
    FailIf (err != noErr, Bail, "FSOpenFork failed");

	err = OpenFile(fsRdWrPerm, refNum);
    FailIf (err != noErr, Bail, "FSOpenFork failed");
	
    mIsInitialized = false;
	
Bail:
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::DoOpen(			
									const FSRef		*inFileRef, 
									SInt8  			inPermissions,
									SInt16			inRefNum)
{		
	SetPermissions(inPermissions);
	return Open(inFileRef, inPermissions, inRefNum);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::Open(			
									const FSRef		*inFileRef, 
									SInt8  			inPermissions,
									SInt16			inRefNum)
{		
	SetFSRef(inFileRef);
	
	OSStatus err = OpenFile(inPermissions, inRefNum);
    FailIf (err != noErr, Bail, "FSOpenFork failed");
	
	err = OpenFromDataSource(inPermissions);
	
Bail:
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::DoOpenWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc)
{		
	SetPermissions(fsRdWrPerm);
	DataSource* dataSource = new Seekable_DataSource(inRefCon, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc);
	SetDataSource(dataSource);
	return OpenFromDataSource(fsRdWrPerm);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::DoInitializeWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc,
                UInt32								inFileType,
				const AudioStreamBasicDescription	*inFormat,
				UInt32								inFlags)
{		
	DataSource* dataSource = new Seekable_DataSource(inRefCon, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc);
	if (!dataSource->CanWrite()) return permErr;
	dataSource->SetSize(0);
	SetDataSource(dataSource);
	SetPermissions(fsRdWrPerm);

	OSStatus err = SetDataFormat(inFormat);
	if (err) return err;
	
	return InitializeDataSource(inFormat, inFlags);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
							
OSStatus AudioFileObject::OpenFromDataSource(SInt8  			/*inPermissions*/)
{		
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					
OSStatus AudioFileObject::DoInitialize(	
									const FSRef							*inFileRef,
									const AudioStreamBasicDescription	*inFormat,
									UInt32			inFlags)
{
	SetFSRef(inFileRef);
	SetPermissions(fsRdWrPerm);
	return Initialize(inFileRef, inFormat, inFlags);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					
OSStatus AudioFileObject::Initialize(	
									const FSRef							*inFileRef,
									const AudioStreamBasicDescription	*inFormat,
									UInt32								inFlags)
{
	OSStatus err = noErr;
	
	SetFSRef(inFileRef);
	
	SInt16 refNum;
    err = FSOpenFork (inFileRef, 0, NULL, fsRdWrPerm, &refNum);
    FailIf (err != noErr, Bail, "FSOpenFork failed");

	err = OpenFile(fsRdWrPerm, refNum);
    FailIf (err != noErr, Bail, "FSOpenFork failed");

	GetDataSource()->SetSize(0);

	err = SetDataFormat(inFormat);
    FailIf (err != noErr, Bail, "SetDataFormat failed");

	InitializeDataSource(inFormat, inFlags);
	
Bail:	
	return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
							
OSStatus AudioFileObject::InitializeDataSource(const AudioStreamBasicDescription	*inFormat, UInt32 /*inFlags*/)
{		
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::DoClose()
{
	OSStatus err = UpdateSizeIfNeeded();
	if (err) return err;
	
	return Close();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::Close()
{
    try {
		delete mDataSource;
		mDataSource = 0;
	} catch (OSStatus err) {
		return err;
	} catch (...) {
		return kAudioFileUnspecifiedError;
	}
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


OSStatus AudioFileObject::Optimize()
{
	// default is that nothing needs to be done. This happens to be true for Raw, SD2 and NeXT/Sun types.
	SetIsOptimized(true); 
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


OSStatus AudioFileObject::DoOptimize()
{
	if (!CanWrite()) return kAudioFilePermissionsError;

	OSStatus err = UpdateSizeIfNeeded();
	if (err) return err;
	
	if (IsOptimized()) return noErr;

	err = Optimize();
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::UpdateNumBytes(SInt64 inNumBytes)	
{
    OSStatus err = noErr;
	if (inNumBytes != GetNumBytes()) {
		SetNumBytes(inNumBytes);
        
        // #warning " this will not work for vbr formats"
		SetNumPackets(GetNumBytes() / mDataFormat.mBytesPerPacket);
		SizeChanged();
	}
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::UpdateNumPackets(SInt64 inNumPackets)	
{
    OSStatus err = noErr;
	if (inNumPackets != GetNumPackets()) {
		SetNumPackets(inNumPackets);
        
        // #warning " this will not work for vbr formats"
		SetNumBytes(GetNumPackets() * mDataFormat.mBytesPerFrame);
		
		SizeChanged();
	}
	return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::PacketToFrame(SInt64 inPacket, SInt64& outFirstFrameInPacket)
{
	if (mDataFormat.mFramesPerPacket == 0)
	{
		PacketTable* packetTable = GetPacketTable();
		if (!packetTable)
			return kAudioFileInvalidPacketOffsetError;
			
		if (inPacket < 0 || inPacket >= packetTable->size())
			return kAudioFileInvalidPacketOffsetError;
			
		outFirstFrameInPacket = (*packetTable)[inPacket].mFrameOffset;
	}
	else
	{
		outFirstFrameInPacket = inPacket * mDataFormat.mFramesPerPacket;
	}
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::FrameToPacket(SInt64 inFrame, SInt64& outPacket, UInt32& outFrameOffsetInPacket)
{
	if (mDataFormat.mFramesPerPacket == 0)
	{
		PacketTable* packetTable = GetPacketTable();
		if (!packetTable)
			return kAudioFileInvalidPacketOffsetError;
			
		// search packet table
		AudioStreamPacketDescriptionExtended pext;
		memset(&pext, 0, sizeof(pext));
		pext.mFrameOffset = inFrame;
		PacketTable::iterator iter = std::lower_bound(packetTable->begin(), packetTable->end(), pext);
		
		if (iter == packetTable->end())
			return kAudioFileInvalidPacketOffsetError;
		
		outPacket = iter - packetTable->begin();
		outFrameOffsetInPacket = inFrame - iter->mFrameOffset;
	}
	else
	{
		outPacket = inFrame / mDataFormat.mFramesPerPacket;
		outFrameOffsetInPacket = inFrame % mDataFormat.mFramesPerPacket;
	}
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::ReadBytes(		
								Boolean			inUseCache,
								SInt64			inStartingByte, 
								UInt32			*ioNumBytes, 
								void			*outBuffer)
{
    OSStatus		err = noErr;
    UInt16			mode = fsFromStart;
	SInt64 			fileOffset = mDataOffset + inStartingByte;
    bool			readingPastEnd = false;
	
    FailWithAction((ioNumBytes == NULL) || (outBuffer == NULL), err = paramErr, 
		Bail, "invalid num bytes parameter");

	//printf("inStartingByte %lld  GetNumBytes %lld\n", inStartingByte, GetNumBytes());

	if (inStartingByte >= GetNumBytes()) 
	{
		*ioNumBytes = 0;
		return eofErr;
	}

	if ((fileOffset + *ioNumBytes) > (GetNumBytes() + mDataOffset)) 
	{
		*ioNumBytes = (GetNumBytes() + mDataOffset) - fileOffset;
		readingPastEnd = true;
	}
	//printf("fileOffset %lld  mDataOffset %lld  readingPastEnd %d\n", fileOffset, mDataOffset, readingPastEnd);

    if (!inUseCache)
        mode |= noCacheMask;
	
    err = GetDataSource()->ReadBytes(mode, fileOffset, 
			(unsigned long) *ioNumBytes, (Ptr) outBuffer, (ByteCount *) ioNumBytes);

	if (readingPastEnd && err == noErr)
		err = eofErr;

Bail:
    return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::WriteBytes(	
								Boolean			inUseCache,
								SInt64			inStartingByte, 
								UInt32			*ioNumBytes, 
								const void		*inBuffer)
{
    OSStatus		err = noErr;
    UInt16			mode = fsFromStart;
    Boolean			extendingTheAudioData = false;

	if (!CanWrite()) return kAudioFilePermissionsError;

    FailWithAction((ioNumBytes == NULL) || (inBuffer == NULL), err = kAudioFileUnspecifiedError, Bail, "invalid parameters");
	if (!CanWrite()) return kAudioFilePermissionsError; 

    // Do not try to write to a postion greater than 32 bits for some file types
    // see if starting byte + ioNumBytes is greater than 32 bits
    // if so, see if file type supports this and bail if not
    err = IsValidFilePosition(inStartingByte + *ioNumBytes);
    FailIf(err != noErr, Bail, "invalid file position");
    
    if (inStartingByte + *ioNumBytes > GetNumBytes())
        extendingTheAudioData = true;
    
    // if file is not optimized, then do not write data that would overwrite chunks following the sound data chunk
    FailWithAction(	extendingTheAudioData && !IsOptimized(), 
                    err = kAudioFileNotOptimizedError, Bail, "Can't write more data until the file is optimized");

    if (!inUseCache)
        mode |= noCacheMask;
    
    err = GetDataSource()->WriteBytes(mode, mDataOffset + inStartingByte, (unsigned long) *ioNumBytes, 
                        (const void *)inBuffer, (ByteCount *) ioNumBytes);
    FailIf(err != noErr, Bail, "couldn't write new data");
    
    if ((inStartingByte + *ioNumBytes) > GetNumBytes())
    {
        SInt64		nuEOF;						// Get the total bytes of audio data
        SInt64		nuByteTotal;

		err = GetDataSource()->GetSize(nuEOF);
		FailIf(err != noErr, Bail, "GetSize failed");
            
        // only update the data size if the audio data grows
        if (extendingTheAudioData)
        {
            nuByteTotal = nuEOF - mDataOffset;
            err = UpdateNumBytes(nuByteTotal);
        }
    }
    
Bail:
    return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::ReadPackets(	
								Boolean							inUseCache,
								UInt32							*outNumBytes,
								AudioStreamPacketDescription	*outPacketDescriptions,
								SInt64							inStartingPacket, 
								UInt32  						*ioNumPackets, 
								void							*outBuffer)
{
	// This only works with CBR. To suppport VBR you must override.
    OSStatus		err = noErr;
    
    FailWithAction(outBuffer == NULL, err = paramErr, Bail, "NULL buffer");
	
    FailWithAction((ioNumPackets == NULL) || (*ioNumPackets < 1), err = paramErr, Bail, "invalid num packets parameter");
    
	{
		UInt32			byteCount = *ioNumPackets * mDataFormat.mBytesPerPacket;
		SInt64			startingByte = inStartingPacket * mDataFormat.mBytesPerPacket;
			
		err = ReadBytes (inUseCache, startingByte, &byteCount, outBuffer);
		if ((err == noErr) || (err == eofErr))
		{
			if (byteCount != (*ioNumPackets * mDataFormat.mBytesPerPacket))
			{
				*ioNumPackets = byteCount / mDataFormat.mBytesPerPacket;
				byteCount = *ioNumPackets * mDataFormat.mBytesPerPacket;
			}

			if (outNumBytes)
				*outNumBytes = byteCount;
			
			if (err == eofErr)
				err = noErr;
		}
	}
Bail:
    return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::WritePackets(	
								Boolean							inUseCache,
								UInt32							inNumBytes,
								AudioStreamPacketDescription	*inPacketDescriptions,
								SInt64							inStartingPacket, 
								UInt32  						*ioNumPackets, 
								const void						*inBuffer)
{
	// This only works with CBR. To suppport VBR you must override.
    OSStatus		err = noErr;
    SInt64			startingByte;
    UInt32			byteCount;
    
    FailWithAction((ioNumPackets == NULL) || (inBuffer == NULL), err = kAudioFileUnspecifiedError, Bail, "invalid parameter");

    byteCount = *ioNumPackets * mDataFormat.mBytesPerPacket;
    startingByte = inStartingPacket * mDataFormat.mBytesPerPacket;

    err = WriteBytes(inUseCache, startingByte, &byteCount, inBuffer);
    FailIf (err != noErr, Bail, "Write Bytes Failed");

    if (byteCount != (*ioNumPackets * mDataFormat.mBytesPerPacket))
        *ioNumPackets = byteCount / mDataFormat.mBytesPerPacket;

Bail:
    return (err);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetMagicCookieDataSize(
												UInt32					*outDataSize,
												UInt32					*isWritable)
{
	if (outDataSize)  *outDataSize = 0;
	if (isWritable)  *isWritable = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetMagicCookieData(
												UInt32					*ioDataSize,
												void					*ioPropertyData)
{
	*ioDataSize = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetMagicCookieData(	UInt32					/*inDataSize*/,
												const void				*inPropertyData)
{
	return kAudioFileInvalidChunkError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetMarkerListSize(
												UInt32					*outDataSize,
												UInt32					*isWritable)
{
	if (outDataSize) *outDataSize = 0;
	if (isWritable) *isWritable = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetMarkerList(
												UInt32					*ioDataSize,
												AudioFileMarkerList*	/*ioPropertyData*/)
{
	*ioDataSize = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetMarkerList(	UInt32						/*inDataSize*/,
											const AudioFileMarkerList*  /*inPropertyData*/)
{
	return kAudioFileUnsupportedPropertyError;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetRegionListSize(
												UInt32					*outDataSize,
												UInt32					*isWritable)
{
	if (outDataSize) *outDataSize = 0;
	if (isWritable) *isWritable = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetRegionList(
												UInt32					*ioDataSize,
												AudioFileRegionList		*ioPropertyData)
{
	*ioDataSize = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetRegionList(	UInt32						/*inDataSize*/,
											const AudioFileRegionList*   /*inPropertyData*/)
{
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetChannelLayoutSize(
												UInt32					*outDataSize,
												UInt32					*isWritable)
{
	if (outDataSize) *outDataSize = 0;
	if (isWritable) *isWritable = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetChannelLayout(
												UInt32						*ioDataSize,
												AudioChannelLayout*			/*ioPropertyData*/)
{
	*ioDataSize = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetChannelLayout(	UInt32						/*inDataSize*/,
											const AudioChannelLayout*   /*inPropertyData*/)
{
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetInfoDictionarySize(		UInt32						*outDataSize,
									UInt32						*isWritable)
{
	if (outDataSize) *outDataSize = sizeof(CFDictionaryRef);
	if (isWritable) *isWritable = 0;
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
												
OSStatus AudioFileObject::GetInfoDictionary(CACFDictionary  *infoDict)
{
	// calculate duration
	AudioStreamBasicDescription		ASBD = GetDataFormat();
	if (ASBD.mFramesPerPacket != 0)
	{
		Float64     fl = (GetNumPackets() * ASBD.mFramesPerPacket) / ASBD.mSampleRate;
		
		#if TARGET_OS_MAC
			CFLocaleRef						currentLocale = CFLocaleGetSystem();
			CFNumberFormatterRef			numberFormatter = NULL;
			numberFormatter = CFNumberFormatterCreate(kCFAllocatorDefault, currentLocale, kCFNumberFormatterDecimalStyle);
			CFStringRef cfStr = CFNumberFormatterCreateStringWithValue( kCFAllocatorDefault, numberFormatter, 
																		kCFNumberFloat64Type, &fl);
			
			infoDict->AddString(CFSTR(kAFInfoDictionary_ApproximateDurationInSeconds), cfStr);
			CFRelease(cfStr);
			CFRelease(numberFormatter);
		#else
			//	Apparently, there is no CFLocale.h on Windows, so use the sprintf
			char theCString[512];
			sprintf(theCString, "%.3f", fl);
			CFStringRef theString = CFStringCreateWithCString(NULL, theCString, kCFStringEncodingASCII);
			if(theString != NULL)
			{
				infoDict->AddString(CFSTR(kAFInfoDictionary_ApproximateDurationInSeconds), theString);
				CFRelease(theString);
			}
		#endif
	}
	
	/*
		For now, assume that any ASBD that has zero in the frames per packet field has been subclassed for this
		method. i.e. A CAF file has a frame count in one of it's chunks.
		
		MP3 has been subclassed because it guesstimates a duration so the entire file does not 
		need to be parsed in order to calculate the total frames.
	*/
	
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
												
OSStatus AudioFileObject::SetInfoDictionary(CACFDictionary *infoDict)
{
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetPropertyInfo	(	
                                        AudioFilePropertyID		inPropertyID,
                                        UInt32					*outDataSize,
                                        UInt32					*isWritable)
{
    OSStatus		err = noErr;
    UInt32			writable = 0;
        
    switch (inPropertyID)
    {
		case kAudioFilePropertyDeferSizeUpdates :
            if (outDataSize) *outDataSize = sizeof(UInt32);
            writable = 1;
            break;

        case kAudioFilePropertyFileFormat:
            if (outDataSize) *outDataSize = sizeof(UInt32);
            writable = 0;
            break;

        case kAudioFilePropertyDataFormat:
            if (outDataSize) *outDataSize = sizeof(AudioStreamBasicDescription);
            writable = 1;
            break;
            
		case 'pkub' :
        case kAudioFilePropertyIsOptimized:
        case kAudioFilePropertyMaximumPacketSize:
            if (outDataSize) *outDataSize = sizeof(UInt32);
            writable = 0;
            break;

        case kAudioFilePropertyAudioDataByteCount:
        case kAudioFilePropertyAudioDataPacketCount:
            writable = 1;
            if (outDataSize) *outDataSize = sizeof(SInt64);
            break;

		case kAudioFilePropertyDataOffset:
            writable = 0;
            if (outDataSize) *outDataSize = sizeof(SInt64);
            break;

		case kAudioFilePropertyMagicCookieData:            
            err = GetMagicCookieDataSize(outDataSize, &writable);
			break;

		case kAudioFilePropertyMarkerList :
            err = GetMarkerListSize(outDataSize, &writable);			
			break;
			
		case kAudioFilePropertyRegionList :
            err = GetRegionListSize(outDataSize, &writable);			
			break;
			
		case kAudioFilePropertyChannelLayout :
            err = GetChannelLayoutSize(outDataSize, &writable);			
			break;

		case kAudioFilePropertyPacketToFrame :
		case kAudioFilePropertyFrameToPacket :
            if (outDataSize) *outDataSize = sizeof(AudioFramePacketTranslation);
            writable = 0;
			break;

        case kAudioFilePropertyInfoDictionary :
            err = GetInfoDictionarySize(outDataSize, &writable);			
            break;

        default:
            writable = 0;
            err = kAudioFileUnsupportedPropertyError;
            break;
    }

    if (isWritable)
        *isWritable = writable;
    return (err);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus	AudioFileObject::GetProperty(
                                    AudioFilePropertyID		inPropertyID,
                                    UInt32					*ioDataSize,
                                    void					*ioPropertyData)
{
    OSStatus		err = noErr;
	UInt32			neededSize;
    UInt32			writable;
	
    switch (inPropertyID)
    {
        case kAudioFilePropertyFileFormat:
            FailWithAction(*ioDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
				
            *(UInt32 *) ioPropertyData = GetFileType();
            break;

        case kAudioFilePropertyDataFormat:
            FailWithAction(*ioDataSize != sizeof(AudioStreamBasicDescription), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
				
            memcpy(ioPropertyData, &mDataFormat, sizeof(AudioStreamBasicDescription));
            break;
		case kAudioFilePropertyDataOffset:
            FailWithAction(*ioDataSize != sizeof(SInt64), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(SInt64 *) ioPropertyData = mDataOffset;
			break;
        case kAudioFilePropertyIsOptimized:
            FailWithAction(*ioDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(UInt32 *) ioPropertyData = mIsOptimized;
            break;

        case kAudioFilePropertyAudioDataByteCount:
            FailWithAction(*ioDataSize != sizeof(SInt64), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(SInt64 *)ioPropertyData = GetNumBytes();
            break;

        case kAudioFilePropertyAudioDataPacketCount:
            FailWithAction(*ioDataSize != sizeof(SInt64), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(SInt64 *)ioPropertyData = GetNumPackets();
            break;

		case 'pkub' :
        case kAudioFilePropertyMaximumPacketSize:
            FailWithAction(*ioDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(UInt32 *)ioPropertyData = mMaximumPacketSize;
            break;


         case kAudioFilePropertyMagicCookieData:            
            
			err = GetMagicCookieData(ioDataSize, ioPropertyData);
            break;

		case kAudioFilePropertyMarkerList :
            err = GetMarkerList(ioDataSize, static_cast<AudioFileMarkerList*>(ioPropertyData));			
			break;
			
		case kAudioFilePropertyRegionList :
            err = GetRegionList(ioDataSize, static_cast<AudioFileRegionList*>(ioPropertyData));			
			break;
			
		case kAudioFilePropertyChannelLayout :
			err = GetChannelLayoutSize(&neededSize, &writable);
            FailIf(err, Bail, "GetChannelLayoutSize failed");
            FailWithAction(*ioDataSize != neededSize, err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
			
            err = GetChannelLayout(ioDataSize, static_cast<AudioChannelLayout*>(ioPropertyData));			
			break;
						
		case kAudioFilePropertyDeferSizeUpdates :
            FailWithAction(*ioDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
				
            *(UInt32 *) ioPropertyData = DeferSizeUpdates();
            break;

		case kAudioFilePropertyPacketToFrame : 
		{
			AudioFramePacketTranslation afpt;
            FailWithAction(*ioDataSize != sizeof(AudioFramePacketTranslation), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
			
			err = PacketToFrame(afpt.mPacket, afpt.mFrame);
			break;
		}	
		case kAudioFilePropertyFrameToPacket :
		{
			AudioFramePacketTranslation afpt;
            FailWithAction(*ioDataSize != sizeof(AudioFramePacketTranslation), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
			err = FrameToPacket(afpt.mFrame, afpt.mPacket, afpt.mFrameOffsetInPacket);
			break;
		}

        case kAudioFilePropertyInfoDictionary :
		{
            FailWithAction(*ioDataSize != sizeof(CFDictionaryRef), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			CACFDictionary		afInfoDictionary(true);

            err = GetInfoDictionary(&afInfoDictionary);			
            
			if (!err)
			{
				*(CFMutableDictionaryRef *)ioPropertyData = afInfoDictionary.CopyCFMutableDictionary();
			}
            break;
		}
			
       default:
            err = kAudioFileUnsupportedPropertyError;			
            break;
    }

Bail:
    return (err);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus	AudioFileObject::SetProperty(
                                    AudioFilePropertyID		inPropertyID,
                                    UInt32					inDataSize,
                                    const void				*inPropertyData)
{
    OSStatus		err = noErr;

    switch (inPropertyID)
    {
        case kAudioFilePropertyDataFormat:
            FailWithAction(inDataSize != sizeof(AudioStreamBasicDescription), 
				err = kAudioFileBadPropertySizeError, Bail, "Incorrect data size");
            err = UpdateDataFormat((AudioStreamBasicDescription *) inPropertyData);
		break;

        case kAudioFilePropertyAudioDataByteCount: {
            FailWithAction(inDataSize != sizeof(SInt64), err = kAudioFileBadPropertySizeError, Bail, "Incorrect data size");
            SInt64 numBytes = *(SInt64 *) inPropertyData;
			if (numBytes > GetNumBytes()) {
				// can't use this to increase data size.
				return kAudioFileOperationNotSupportedError;
			}
			UInt32 saveDefer = DeferSizeUpdates();
			SetDeferSizeUpdates(0); // force an update.
			err = UpdateNumBytes(numBytes);
			SetDeferSizeUpdates(saveDefer);
		} break;

		case kAudioFilePropertyAudioDataPacketCount: {
			SInt64 numPackets = *(SInt64 *) inPropertyData;
			if (numPackets > GetNumPackets()) {
				// can't use this to increase data size.
				return kAudioFileOperationNotSupportedError;
			}
			err = UpdateNumPackets(numPackets);
		} break;
		
		case kAudioFilePropertyMagicCookieData:            
			err = SetMagicCookieData(inDataSize, inPropertyData);
			break;


		case kAudioFilePropertyMarkerList :
            err = SetMarkerList(inDataSize, static_cast<const AudioFileMarkerList*>(inPropertyData));			
			break;
			
		case kAudioFilePropertyRegionList :
            err = SetRegionList(inDataSize, static_cast<const AudioFileRegionList*>(inPropertyData));			
			break;
			
		case kAudioFilePropertyChannelLayout :
            err = SetChannelLayout(inDataSize, static_cast<const AudioChannelLayout*>(inPropertyData));			
			break;

		case kAudioFilePropertyDeferSizeUpdates :
            FailWithAction(inDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            SetDeferSizeUpdates(*(UInt32 *) inPropertyData);
            break;

        case kAudioFilePropertyInfoDictionary :
		{
            FailWithAction(inDataSize != sizeof(CFDictionaryRef), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			// pass the SetInfoDictionary a CACFDictionary object made with the provided CFDictionaryRef
			// Let the caller release their own CFObject so pass false for th erelease parameter
			CACFDictionary		afInfoDictionary(*(CFDictionaryRef *)inPropertyData, false);
            err = SetInfoDictionary(&afInfoDictionary);			
            
            break;
		}
			
        default:
            err = kAudioFileUnsupportedPropertyError;			
		break;
    }

Bail:
    return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetDataFormat(const AudioStreamBasicDescription* inStreamFormat)
{
	OSStatus err = noErr;
	
	if (!IsDataFormatSupported(inStreamFormat)) 
		return kAudioFileUnsupportedDataFormatError;
	
	UInt32 prevBytesPerPacket = mDataFormat.mBytesPerPacket;
	
	mDataFormat = *inStreamFormat;
	
	// if CBR and bytes per packet changes, we need to change the number of packets we think we have.
	if (!mFirstSetFormat && mDataFormat.mBytesPerPacket && mDataFormat.mBytesPerPacket != prevBytesPerPacket)
	{
		SInt64 numPackets = GetNumBytes() / mDataFormat.mBytesPerPacket;
		SetNumPackets(numPackets);
		SetMaximumPacketSize(mDataFormat.mBytesPerPacket);

		SizeChanged();
	}
	
	mFirstSetFormat = false;
	
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::UpdateDataFormat(const AudioStreamBasicDescription* inStreamFormat)
{
	if (!IsDataFormatSupported(inStreamFormat)) return kAudioFileUnsupportedDataFormatError;
	return SetDataFormat(inStreamFormat);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Boolean AudioFileObject::IsDataFormatValid(AudioStreamBasicDescription const* inDesc)
{
	if (inDesc->mFormatID == kAudioFormatLinearPCM)
	{
		if (inDesc->mSampleRate < 0.)
			return false;
			
		if (inDesc->mFramesPerPacket != 1)
			return false;
			
		if (inDesc->mBytesPerFrame != inDesc->mBytesPerPacket)
			return false;
			
		// [3605260] we assume here that a packet is an integer number of frames.
		UInt32 minimumBytesPerPacket = (inDesc->mBitsPerChannel * inDesc->mChannelsPerFrame + 7) / 8;
		if (inDesc->mBytesPerPacket < minimumBytesPerPacket) 
			return false;
	}
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void AudioFileObject::SetDataSource(DataSource* inDataSource)	
{
	if (mDataSource != inDataSource) {
		delete mDataSource;
		mDataSource = inDataSource;
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::OpenFile(SInt8 inPermissions, SInt16 inRefNum)
{
	OSStatus err = noErr;

#if USE_CACHED_DATASOURCE
	SetDataSource(new Cached_DataSource(new MacFile_DataSource(inRefNum, inPermissions, true)));
#else
	SetDataSource(new MacFile_DataSource(inRefNum, inPermissions, true));
#endif
	
	SetFileNum(inRefNum);
	mPermissions = inPermissions;

	return err;
}

OSStatus AudioFileObject::CreateDataFile(const FSRef* inParentFileRef, CFStringRef inFileName, FSRef *outRef)
{
    OSStatus		err = noErr;
    UniChar			uniName[ 255 ];
	CFRange			cfRange;
	cfRange.length = CFStringGetLength (inFileName);
	cfRange.location = 0;

	CFStringGetCharacters (inFileName, cfRange, uniName);

	// create the new file in the directory indicated by the inParentFileRef param
	err = FSCreateFileUnicode (inParentFileRef, (UniCharCount) cfRange.length, (const UniChar *) uniName, 
							kFSCatInfoNone, NULL, outRef, NULL);
	FailIf (err != noErr, Bail, "FSCreateFileUnicode failed");
	
Bail:
	return err;
}

OSStatus AudioFileObject::CreateResourceFile(const FSRef* inParentFileRef, CFStringRef inFileName, FSRef *outRef)
{
    OSStatus		err = noErr;
    UniChar			uniName[ 255 ];
	CFRange			cfRange;
	cfRange.length = CFStringGetLength (inFileName);
	cfRange.location = 0;

	CFStringGetCharacters (inFileName, cfRange, uniName);

	FSSpec				nuSpec;
	HFSUniStr255 		theResourceForkName;

	FSGetResourceForkName(&theResourceForkName);

	// create the new file in the directory indicated by the inParentFileRef param
	err = FSCreateResourceFile(inParentFileRef, (UniCharCount) cfRange.length, (const UniChar *) uniName, kFSCatInfoNone,
								NULL, theResourceForkName.length, theResourceForkName.unicode, outRef,  &nuSpec); 
	FailIf (err != noErr, Bail, "FSCreateFileUnicode failed");
	
Bail:
	return err;
}

OSStatus AudioFileObject::SizeChanged()
{
	OSStatus err = noErr;
	if (mPermissions & fsWrPerm) 
	{
		if (DeferSizeUpdates())
			SetNeedsSizeUpdate(true);
		else
			err = UpdateSize();
	}
	return err;
}

OSStatus AudioFileObject::UpdateSizeIfNeeded()
{		
	if (GetNeedsSizeUpdate()) 
	{
		OSStatus err = UpdateSize();
		if (err) return err;
		SetNeedsSizeUpdate(false);
	}
	return noErr;
}

OSStatus AudioFileObject::CountUserData(	UInt32					/*inUserDataID*/,
											UInt32*					/*outNumberItems*/)
{
	return kAudioFileOperationNotSupportedError;
}

OSStatus AudioFileObject::GetUserDataSize(  UInt32					/*inUserDataID*/,
											UInt32					/*inIndex*/,
											UInt32*					/*outDataSize*/)
{
	return kAudioFileOperationNotSupportedError;
}
											
OSStatus AudioFileObject::GetUserData(		UInt32					/*inUserDataID*/,
											UInt32					/*inIndex*/,
											UInt32*					/*ioDataSize*/,
											void*					/*ioUserData*/)
{
	return kAudioFileOperationNotSupportedError;
}
											
OSStatus AudioFileObject::SetUserData(		UInt32					/*inUserDataID*/,
											UInt32					/*inIndex*/,
											UInt32					/*inDataSize*/,
											const void*				/*inUserData*/)
{
	return kAudioFileOperationNotSupportedError;
}
											
OSStatus AudioFileObject::RemoveUserData(	UInt32					/*inUserDataID*/,
											UInt32					/*inIndex*/)
{
	return kAudioFileOperationNotSupportedError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

