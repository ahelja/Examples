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
	DataSource.cpp
    Created by James E McCartney on Mon Aug 26 2002.

=============================================================================*/

#include "DataSource.h"
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <algorithm>

#define VERBOSE 0

//////////////////////////////////////////////////////////////////////////////////////////


const UInt16 kPositionModeMask = 3;

DataSource::DataSource(Boolean inCloseOnDelete)
	: mCloseOnDelete(inCloseOnDelete)
{
}

DataSource::~DataSource()
{
}

SInt64 DataSource::CalcOffset(	UInt16 positionMode, 
								SInt64 positionOffset,
								SInt64 currentOffset,
								SInt64 size)
{
	SInt64 newOffset = 0;
	switch (positionMode & kPositionModeMask) {
		case fsAtMark : newOffset = currentOffset; break;
		case fsFromStart : newOffset = positionOffset; break;
		case fsFromLEOF : newOffset = size + positionOffset; break;
		case fsFromMark : newOffset = positionOffset + currentOffset; break;
	}
	return newOffset;
}

Boolean DataSource::EqualsCurrentOffset(
					UInt16 positionMode, 
					SInt64 positionOffset)
{
	positionMode = positionMode & kPositionModeMask;
	if (positionMode == fsAtMark) return true;
	if (positionMode == fsFromMark && positionOffset == 0) return true;
	SInt64 pos;
	OSStatus err = GetPos(pos);
	if (err) return false;
	if (positionMode == fsFromStart && positionOffset == pos) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

MacFile_DataSource::MacFile_DataSource( SInt16 inForkRefNum, SInt8 inPermissions, Boolean inCloseOnDelete)
	: DataSource(inCloseOnDelete), mFileNum(inForkRefNum), mPermissions(inPermissions)
{
}

MacFile_DataSource::~MacFile_DataSource()
{
	if (mCloseOnDelete) FSClose(mFileNum);
}


OSStatus MacFile_DataSource::GetSize(SInt64& outSize) const
{
	outSize = -1; // in case of error
	OSStatus err = FSGetForkSize(mFileNum, &outSize);
	return err;
}

OSStatus MacFile_DataSource::GetPos(SInt64& outPos) const
{
	return FSGetForkPosition(mFileNum, &outPos);
}

OSStatus MacFile_DataSource::SetSize(SInt64 inSize)
{
	return FSSetForkSize(mFileNum, fsFromStart, inSize);
}


OSStatus MacFile_DataSource::ReadBytes(
					UInt16 positionMode, 
					SInt64 positionOffset, 
					ByteCount requestCount, 
					void *buffer, 
					ByteCount* actualCount)
{
	if (!buffer) return paramErr;
	return FSReadFork(mFileNum, positionMode, positionOffset, requestCount, buffer, actualCount);
}

OSStatus MacFile_DataSource::WriteBytes(
					UInt16 positionMode, 
					SInt64 positionOffset, 
					ByteCount requestCount, 
					const void *buffer, 
					ByteCount* actualCount)
{
	if (!buffer) return paramErr;
	return FSWriteFork(mFileNum, positionMode, positionOffset, requestCount, buffer, actualCount);
}

//////////////////////////////////////////////////////////////////////////////////////////

#define NO_CACHE 0

OSStatus Cached_DataSource::ReadFromHeaderCache(
					SInt64 offset, 
					ByteCount requestCount,
					void *buffer, 
					ByteCount* actualCount)
{
	OSStatus err = noErr;
	ByteCount theActualCount = 0;

#if VERBOSE	
	printf("read from header %lld %lu   %lld %lu\n", offset, requestCount, 0LL, mHeaderCacheSize);
#endif

	if (!mHeaderCache) 
	{
		mHeaderCache = (UInt8*)calloc(1, mHeaderCacheSize);
		err = mDataSource->ReadBytes(fsFromStart, 0, mHeaderCacheSize, mHeaderCache, &mHeaderCacheSize);
		if (err == eofErr) err = noErr;
		if (err) return err;
	}
	
	ByteCount firstPart = std::min((SInt64)requestCount, (SInt64)mHeaderCacheSize - offset);
	ByteCount secondPart = requestCount - firstPart;
	
	memcpy(buffer, mHeaderCache + offset, firstPart);
	theActualCount = firstPart;
	
	if (secondPart) {
		ByteCount secondPartActualCount = 0;
		err = mDataSource->ReadBytes(fsFromStart, mHeaderCacheSize, secondPart, (char*)buffer + firstPart, &secondPartActualCount);
		theActualCount += secondPartActualCount;
	}
	
	if (actualCount) *actualCount = theActualCount;
	mOffset = offset + theActualCount;
	
	return err;
}

OSStatus Cached_DataSource::ReadBytes(
					UInt16 positionMode, 
					SInt64 positionOffset, 
					ByteCount requestCount, 
					void *buffer, 
					ByteCount* actualCount)
{
	OSStatus err = noErr;
	SInt64 size;
	ByteCount theActualCount = 0;

	if (!buffer) return paramErr;

	if ((positionMode & kPositionModeMask) != fsFromLEOF) size = 0; // not used in this case
	else 
	{
		err = GetSize(size);
		if (err) return err;
	}

	SInt64 offset = CalcOffset(positionMode, positionOffset, mOffset, size);
	if (offset < 0) return posErr;
	
	if (offset < mHeaderCacheSize) {
		return ReadFromHeaderCache(offset, requestCount, buffer, actualCount);
	}

#if NO_CACHE
	err = mDataSource->ReadBytes(positionMode, positionOffset, requestCount, buffer, &theActualCount);
	mOffset = offset + theActualCount;
#else

	SInt64 cacheEnd = mBodyCacheOffset + mBodyCacheCurSize;
	if (mBodyCache && requestCount < mBodyCacheSize && offset >= mBodyCacheOffset && offset < cacheEnd)
	{
		if (offset + requestCount <= cacheEnd) 
		{
			// request is entirely within cache
#if VERBOSE	
			printf("request is entirely within cache %lld %lu   %lld %lu\n", offset, requestCount, mBodyCacheOffset, mBodyCacheCurSize);
#endif
			memcpy(buffer, mBodyCache + (offset - mBodyCacheOffset), requestCount);
			theActualCount = requestCount;
		}
		else
		{
			// part of request is within cache. copy, read next cache block, copy.
#if VERBOSE	
			printf("part of request is within cache %lld %lu   %lld %lu\n", offset, requestCount, mBodyCacheOffset, mBodyCacheCurSize);
#endif
			
			// copy first part.
			ByteCount firstPart = cacheEnd - offset;
			ByteCount secondPart = requestCount - firstPart;
#if VERBOSE	
			printf("memcpy   offset %lld  mBodyCacheOffset %lld  offset - mBodyCacheOffset %lld  firstPart %lu   requestCount %lu\n", 
						offset, mBodyCacheOffset, offset - mBodyCacheOffset, firstPart, requestCount);
#endif
			memcpy(buffer, mBodyCache + (offset - mBodyCacheOffset), firstPart);
			
			// read new block
			mBodyCacheOffset += mBodyCacheSize;
			err = mDataSource->ReadBytes(fsFromStart, mBodyCacheOffset, mBodyCacheSize, mBodyCache, &mBodyCacheCurSize);
			if (err == eofErr) err = noErr;
			if (err) return err;

			// copy second part
			secondPart = std::min(secondPart, mBodyCacheCurSize);
			if (secondPart) memcpy((char*)buffer + firstPart, mBodyCache, secondPart);
			theActualCount = firstPart + secondPart;
		}
	}
	else 
	{
		if (requestCount > mBodyCacheSize)
		{
#if VERBOSE	
			printf("large request %lld %lu   %lld %lu\n", offset, requestCount, mBodyCacheOffset, mBodyCacheCurSize);
#endif
			// the request is larger than we normally cache, just do a read and don't cache.
			err = mDataSource->ReadBytes(positionMode, positionOffset, requestCount, buffer, &theActualCount);
			mOffset = offset + theActualCount;
		}
		else
		{
			// request is outside cache. read new block.
#if VERBOSE	
			printf("request is outside cache %lld %lu   %lld %lu\n", offset, requestCount, mBodyCacheOffset, mBodyCacheCurSize);
#endif
			if (!mBodyCache) 
			{
				mBodyCache = (UInt8*)calloc(1, mBodyCacheSize);
#if VERBOSE	
				printf("alloc mBodyCache %08X\n", mBodyCache);
#endif
			}
			mBodyCacheOffset = offset;
			err = mDataSource->ReadBytes(fsFromStart, mBodyCacheOffset, mBodyCacheSize, mBodyCache, &mBodyCacheCurSize);
#if VERBOSE	
			printf("read %08X %d    mBodyCacheOffset %lld   %lu %lu\n", err, err, mBodyCacheOffset, mBodyCacheSize, mBodyCacheCurSize);
#endif
			if (err == eofErr) err = noErr;
			if (err) return err;

			theActualCount = std::min(requestCount, mBodyCacheCurSize);
			memcpy(buffer, mBodyCache, theActualCount);
		}
		
	}

#endif
	if (actualCount) *actualCount = theActualCount;
#if VERBOSE	
	printf("<<read err %d  actualCount %lu\n", err, *actualCount);
#endif
	return err;
}

OSStatus Cached_DataSource::WriteBytes(
					UInt16 positionMode, 
					SInt64 positionOffset, 
					ByteCount requestCount, 
					const void *buffer, 
					ByteCount* actualCount)
{
	OSStatus err = noErr;
	SInt64 size;

	if (!buffer) return paramErr;
	
	if ((positionMode & kPositionModeMask) != fsFromLEOF) size = 0; // not used in this case
	else 
	{
		err = GetSize(size);
		if (err) return err;
	}

	SInt64 offset = CalcOffset(positionMode, positionOffset, mOffset, size);
	if (offset < 0) return posErr;
	
	if (mHeaderCache && offset < mHeaderCacheSize) 
	{
		// header cache write through
		ByteCount firstPart = std::min((SInt64)requestCount, (SInt64)mHeaderCacheSize - offset);
#if VERBOSE	
		printf("header cache write through %lu %lu\n", mHeaderCacheSize, firstPart);
#endif
		memcpy(mHeaderCache + offset, buffer, firstPart);
	}
		
#if VERBOSE	
	printf("write %lld %lu    %lld %d %lld\n", offset, requestCount, mOffset, positionMode, positionOffset);
#endif

	SInt64 cacheEnd = mBodyCacheOffset + mBodyCacheCurSize;
	if (mBodyCache && offset >= mBodyCacheOffset && offset < cacheEnd)
	{
		// body cache write through
		ByteCount firstPart = std::min((SInt64)requestCount, cacheEnd - offset);
#if VERBOSE	
		printf("body cache write through %lld %lu  %lld %lu\n", mBodyCacheOffset, mBodyCacheCurSize, offset, firstPart);
#endif
		memcpy(mBodyCache + (offset - mBodyCacheOffset), buffer, firstPart);
	}
	
	ByteCount theActualCount;
	err = mDataSource->WriteBytes(positionMode, positionOffset, requestCount, buffer, &theActualCount);
	
	mOffset = offset + theActualCount;
	if (actualCount) *actualCount = theActualCount;
	
	return err;
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

Seekable_DataSource::Seekable_DataSource(	void * inRefCon,
							AudioFile_ReadProc					inReadFunc, 
							AudioFile_WriteProc					inWriteFunc, 
							AudioFile_GetSizeProc				inGetSizeFunc,
							AudioFile_SetSizeProc				inSetSizeFunc)
	: DataSource(false), mRefCon(inRefCon), mReadFunc(inReadFunc), mWriteFunc(inWriteFunc), 
		mSizeFunc(inGetSizeFunc), mSetSizeFunc(inSetSizeFunc)
{
}

Seekable_DataSource::~Seekable_DataSource()
{
}


OSStatus Seekable_DataSource::GetSize(SInt64& outSize) const
{
	if (!mSizeFunc) return kAudioFileOperationNotSupportedError;
	outSize = (*mSizeFunc)(mRefCon);
	return noErr;
}

OSStatus Seekable_DataSource::SetSize(SInt64 inSize)
{
	if (!mSetSizeFunc) return kAudioFileOperationNotSupportedError;
	return (*mSetSizeFunc)(mRefCon, inSize);
}

OSStatus Seekable_DataSource::ReadBytes(	
							UInt16 positionMode, 
							SInt64 positionOffset, 
							ByteCount requestCount, 
							void *buffer, 
							ByteCount* actualCount)
{
	OSStatus err;
	
	if (!mReadFunc) return kAudioFileOperationNotSupportedError;
	if (!buffer) return paramErr;

	SInt64 size;
	positionMode &= kPositionModeMask;
	if (positionMode != fsFromLEOF) size = 0; // not used in this case
	else 
	{
		err = GetSize(size);
		if (err) return err;
	}
	
	SInt64 offset = CalcOffset(positionMode, positionOffset, mOffset, size);
	if (offset < 0) return posErr;
	
	ByteCount theActualCount = 0;
	err = (*mReadFunc)(mRefCon, offset, requestCount, buffer, &theActualCount);
	if (actualCount) *actualCount = theActualCount;
	mOffset = offset + theActualCount;
	return err;
}

					
OSStatus Seekable_DataSource::WriteBytes(
							UInt16 positionMode, 
							SInt64 positionOffset, 
							ByteCount requestCount, 
							const void *buffer, 
							ByteCount* actualCount)
{
	OSStatus err;
	
	if (!mWriteFunc) return kAudioFileOperationNotSupportedError;
	if (!buffer) return paramErr;

	SInt64 size;
	positionMode &= kPositionModeMask;
	if (positionMode != fsFromLEOF) size = 0; // not used in this case
	else 
	{
		err = GetSize(size);
		if (err) return err;
	}

	SInt64 offset = CalcOffset(positionMode, positionOffset, mOffset, size);
	if (offset < 0) return posErr;
	
	ByteCount theActualCount;
	err = (*mWriteFunc)(mRefCon, offset, requestCount, buffer, &theActualCount);
	if (err) return err;
	if (actualCount) *actualCount = theActualCount;
	mOffset = offset + theActualCount;
	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////////
