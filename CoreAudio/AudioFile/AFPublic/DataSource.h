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
	DataSource.h
    Created by James E McCartney on Mon Aug 26 2002.

=============================================================================*/

#ifndef __DataSource_h__
#define __DataSource_h__

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreServices/CoreServices.h>
	#include <AudioToolbox/AudioFile.h>
#else
	#include <ConditionalMacros.h>
	#include <CoreServices.h>
	#include "AudioFile.h"
#endif
#include <stdlib.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////////////////

class DataSource
{
public:
	DataSource(Boolean inCloseOnDelete);
	virtual ~DataSource();
		
	virtual OSStatus GetSize32(UInt32& outSize) const
	{
		SInt64 size64;
		OSStatus err = GetSize(size64);
		if (err) return err;
		if (size64 > 0x00000000FFFFffffLL) return kAudioFileDoesNotAllow64BitDataSizeError;
		outSize = size64;
		return noErr;
	}
	
	virtual OSStatus GetSize(SInt64& outSize) const=0;
	
	virtual OSStatus SetSize(SInt64 inSize)=0;
	
	virtual OSStatus GetPos(SInt64& outPos) const=0; 
	
	/* non seekable data sources should use fsAtMark for the positionMode (or fsFromMark with offset zero, 
		or fsFromStart with offset equal to the current position in the stream, in other words no seeking from the 
		current position is allowed.)
	*/
	
	virtual OSStatus ReadBytes(	UInt16 positionMode, 
								SInt64 positionOffset, 
								ByteCount requestCount, 
								void *buffer, 
								ByteCount* actualCount)=0;
						
	virtual OSStatus WriteBytes(UInt16 positionMode, 
								SInt64 positionOffset, 
								ByteCount requestCount, 
								const void *buffer, 
								ByteCount* actualCount)=0;
	
	virtual void SetCloseOnDelete(Boolean inFlag) { mCloseOnDelete = inFlag; }
	
	virtual Boolean CanSeek() const=0;
	virtual Boolean CanGetSize() const=0;
	virtual Boolean CanSetSize() const=0;
	virtual Boolean CanRead() const=0;
	virtual Boolean CanWrite() const=0;
	
protected:
	Boolean mCloseOnDelete;
	
	/* utility method */
	SInt64 CalcOffset(	UInt16 positionMode, 
						SInt64 positionOffset,
						SInt64 currentOffset,
						SInt64 size);
						
	Boolean EqualsCurrentOffset(
					UInt16 positionMode, 
					SInt64 positionOffset);
};

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

/*
	Initialize with a Macintosh file fork ref num as obtained from FSOpenFork.
*/
class MacFile_DataSource : public DataSource
{
	SInt16 mFileNum;
	SInt8 mPermissions;
	
public:

	MacFile_DataSource(	SInt16 inForkRefNum, SInt8 inPermissions, Boolean inCloseOnDelete);
	virtual ~MacFile_DataSource();
	
	virtual OSStatus GetSize(SInt64& outSize) const;
	virtual OSStatus GetPos(SInt64& outPos) const; 
	
	virtual OSStatus SetSize(SInt64 inSize);
	
	virtual OSStatus ReadBytes(	UInt16 positionMode, 
								SInt64 positionOffset, 
								ByteCount requestCount, 
								void *buffer, 
								ByteCount* actualCount);
						
	virtual OSStatus WriteBytes(UInt16 positionMode, 
								SInt64 positionOffset, 
								ByteCount requestCount, 
								const void *buffer, 
								ByteCount* actualCount);
	
	virtual Boolean CanSeek() const { return true; }
	virtual Boolean CanGetSize() const { return true; }
	virtual Boolean CanSetSize() const { return true; }
	
	virtual Boolean CanRead() const { return mPermissions & fsRdPerm; }
	virtual Boolean CanWrite() const { return mPermissions & fsWrPerm; }
};

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////

/*
	A wrapper that caches the wrapped source's header.
*/
class Cached_DataSource : public DataSource
{
	DataSource* mDataSource;
	UInt8* mHeaderCache;
	ByteCount mHeaderCacheSize;
	UInt8* mBodyCache;
	ByteCount mBodyCacheSize;
	ByteCount mBodyCacheCurSize;
	SInt64 mBodyCacheOffset;
	SInt64 mOffset;
	Boolean mOwnDataSource;
	
public:

	Cached_DataSource(DataSource* inDataSource, UInt32 inHeaderCacheSize = 4096, UInt32 inBodyCacheSize = 32768, Boolean inOwnDataSource = true)
					: DataSource(false), 
					mDataSource(inDataSource), mHeaderCache(0), mHeaderCacheSize(inHeaderCacheSize), 
					mBodyCache(0), mBodyCacheSize(inBodyCacheSize), mBodyCacheCurSize(0), mBodyCacheOffset(-1), 
					mOffset(0),
					mOwnDataSource(inOwnDataSource)
				{
				}
				
	virtual ~Cached_DataSource()
				{
					free(mHeaderCache);
					free(mBodyCache);
					if (mOwnDataSource) delete mDataSource;
				}

	
	virtual OSStatus GetSize(SInt64& outSize) const { return mDataSource->GetSize(outSize); }
	virtual OSStatus GetPos(SInt64& outPos) const { return mDataSource->GetPos(outPos); } 
	
	virtual OSStatus SetSize(SInt64 inSize) { return mDataSource->SetSize(inSize); }
	
	virtual OSStatus ReadBytes(		UInt16 positionMode, 
									SInt64 positionOffset, 
									ByteCount requestCount, 
									void *buffer, 
									ByteCount* actualCount);
						
	virtual OSStatus WriteBytes(	UInt16 positionMode, 
									SInt64 positionOffset, 
									ByteCount requestCount, 
									const void *buffer, 
									ByteCount* actualCount);
	
	OSStatus ReadFromHeaderCache(	SInt64 offset, 
									ByteCount requestCount,
									void *buffer, 
									ByteCount* actualCount);
	
	virtual Boolean CanSeek() const { return mDataSource->CanSeek(); }
	virtual Boolean CanGetSize() const { return mDataSource->CanGetSize(); }
	virtual Boolean CanSetSize() const { return mDataSource->CanSetSize(); }
	
	virtual Boolean CanRead() const { return mDataSource->CanRead(); }
	virtual Boolean CanWrite() const { return mDataSource->CanWrite(); }
};

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////

typedef OSStatus (*AudioFile_ReadProc)(
								void * inRefCon,
								SInt64 inPosition, 
								ByteCount requestCount, 
								void *buffer, 
								ByteCount* actualCount);

typedef OSStatus (*AudioFile_WriteProc)(
								void * inRefCon,
								SInt64 inPosition, 
								ByteCount requestCount, 
								const void *buffer, 
								ByteCount* actualCount);
								
typedef SInt64 (*AudioFile_GetSizeProc)(void * inRefCon);

typedef OSStatus (*AudioFile_SetSizeProc)(void * inRefCon, SInt64 inSize);

//////////////////////////////////////////////////////////////////////////////////////////


/* This class calls user supplied routines on demand. */

class Seekable_DataSource : public DataSource
{
	void * mRefCon;
	SInt64 mOffset;
	
	AudioFile_ReadProc mReadFunc;
	AudioFile_WriteProc mWriteFunc;
	AudioFile_GetSizeProc mSizeFunc;
	AudioFile_SetSizeProc mSetSizeFunc;
	
public:
	Seekable_DataSource(	void * 								inRefCon,
							AudioFile_ReadProc					inReadFunc, 
							AudioFile_WriteProc					inWriteFunc, 
							AudioFile_GetSizeProc				inGetSizeFunc,
							AudioFile_SetSizeProc				inSetSizeFunc
					);
	
	virtual ~Seekable_DataSource();
	
	virtual OSStatus GetSize(SInt64& outSize) const;
	virtual OSStatus GetPos(SInt64& outPos) const { outPos = mOffset; return noErr; }; 
	
	virtual OSStatus SetSize(SInt64 inSize);
	
	virtual OSStatus ReadBytes(	UInt16 positionMode, 
								SInt64 positionOffset, 
								ByteCount requestCount, 
								void *buffer, 
								ByteCount* actualCount);
						
	virtual OSStatus WriteBytes(UInt16 positionMode, 
								SInt64 positionOffset, 
								ByteCount requestCount, 
								const void *buffer, 
								ByteCount* actualCount);

	virtual Boolean CanSeek() const { return true; }
	virtual Boolean CanGetSize() const { return mSizeFunc != 0; }
	virtual Boolean CanSetSize() const { return mSetSizeFunc != 0; }
	virtual Boolean CanRead() const { return mReadFunc != 0; }
	virtual Boolean CanWrite() const { return mWriteFunc != 0; }
};

//////////////////////////////////////////////////////////////////////////////////////////

#endif
