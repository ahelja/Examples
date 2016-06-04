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
	HLFile.cpp

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

#include "HLFile.h"
#include "HLFileSystem.h"
#include "HLDirectory.h"
#include "CAAutoDisposer.h"
#include "CACFString.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include <stdlib.h>

//=============================================================================
//	HLFile
//=============================================================================

HLFile::HLFile(const FSRef& inFSRef)
:
	HLFileSystemObject(inFSRef),
	mRefNum(-1),
	mOpenForReading(false),
	mOpenForWriting(false),
	mOpenCount(0),
	mPrepared(false)
{
	//	set the current fork to be the data for;
	FSGetDataForkName(&mCurrentForkName);
}

HLFile::~HLFile()
{
	//	close the file if it's open
	if(mOpenForReading || mOpenForWriting)
	{
		FSCloseFork(mRefNum);
	}
}

bool	HLFile::IsFile() const
{
	return true;
}

bool	HLFile::IsDirectory() const
{
	return false;
}

bool	HLFile::IsVolume() const
{
	return false;
}

bool	HLFile::IsOpenForReading() const
{
	return mOpenForReading;
}

bool	HLFile::IsOpenForWriting() const
{
	return mOpenForWriting;
}

void	HLFile::Open(bool inForReading, bool inForWriting)
{
	if(mOpenCount == 0)
	{
		//	only actully open the file the first time
		
		//	save off the permissions
		mOpenForReading = inForReading;
		mOpenForWriting = inForWriting;
		
		//	translate the permisions
		SInt8 thePermissions = 0;
		if(mOpenForReading && mOpenForWriting)
		{
			thePermissions = fsRdWrPerm;
		}
		else if(mOpenForReading)
		{
			thePermissions = fsRdPerm;
		}
		else if(mOpenForWriting)
		{
			thePermissions = fsWrPerm;
		}
		else
		{
			DebugMessage("HLFile::Open: have to specify the access mode");
			throw CAException(paramErr);
		}
		
		//	open the current fork
		OSStatus theError = FSOpenFork(&mFSRef, mCurrentForkName.length, mCurrentForkName.unicode, thePermissions, &mRefNum);
		ThrowIfError(theError, CAException(theError), "HLFile::GetPosition: couldn't open the fork");
	}
	else
	{
		//	file is already open, so it's an error if someone tries to add permissions
		ThrowIf((mOpenForReading && !mOpenForWriting) && inForWriting, CAException(fBsyErr), "HLFile::Open: can't add write permissions");
		ThrowIf((!mOpenForReading && mOpenForWriting) && inForReading, CAException(fBsyErr), "HLFile::Open: can't add read permissions");
	}

	//	increment the open count
	++mOpenCount;
}

void	HLFile::Close()
{
	if(mOpenForWriting)
	{
		Flush();
	}
	
	if(mOpenCount > 0)
	{
		//	only close the file if it is open
		
		//	decrement the open count
		--mOpenCount;
		
		if(mOpenCount == 0)
		{
			//	no one wants the file open, so really close it
			OSStatus theError = FSCloseFork(mRefNum);
			
			//	reset the refnum
			mRefNum = -1;
			
			//	clear the permissions
			mOpenForReading = false;
			mOpenForWriting = false;
			
			//	check for errors
			ThrowIfError(theError, CAException(theError), "HLFile::Close: couldn't close the fork");
		}
	}
}

void	HLFile::Prepare()
{
	//	- invoked for files that have content already in the appropriate format
	//	- this or PrepareNew() should be called immediately after creating the object
	//	  but Open() may be called previously for optimization purposes
	//	- should throw an exception if the content isn't in proper format
	mPrepared = true;
}

void	HLFile::PrepareNew(const void* /*inData*/, UInt32 /*inDataSize*/)
{
	//	- invoked for files that should be newly initialized with the given data
	//	- this or Prepare() should be called immediately after creating the object
	//	  but Open() may be called previously for optimization purposes
	//	- should only throw an exception if some file system related problem occurs
	mPrepared = true;
}

void	HLFile::Flush(bool /*inOptimizeLayout*/)
{
	ThrowIf(!mOpenForWriting, CAException(fnOpnErr), "HLFile::Flush: file isn't open");
	
	FSFlushFork(mRefNum);
}

SInt64	HLFile::GetRawByteSize() const
{
	ThrowIf(!mOpenForReading && !mOpenForWriting, CAException(fnOpnErr), "HLFile::GetByteSize: file isn't open");
	
	SInt64 theAnswer = 0;
	
	//	get the size
	OSStatus theError = FSGetForkSize(mRefNum, &theAnswer);
	ThrowIfError(theError, CAException(theError), "HLFile::GetByteSize: couldn't get the fork size");
	
	//	return the size
	return theAnswer;
}

void	HLFile::SetRawByteSize(SInt64 inSize)
{
	ThrowIf(!mOpenForWriting, CAException(fnOpnErr), "HLFile::SetByteSize: file isn't open");
	
	//	set the size
	OSStatus theError = FSSetForkSize(mRefNum, fsFromStart, inSize);
	ThrowIfError(theError, CAException(theError), "HLFile::SetByteSize: couldn't set the fork size");
}

void	HLFile::ReadRawBytes(SInt64 inOffset, UInt32& ioNumberBytes, void* outData, bool inCache)
{
	ThrowIf(!mOpenForReading, CAException(fnOpnErr), "HLFile::ReadBytes: file isn't open");
	
	//	figure out the positioning mask in light of the caching request
	UInt16 thePositionMode = fsFromStart;
	if(!inCache)
	{
		thePositionMode += noCacheMask;
	}
	
	//	read the data
	UInt32 theActualBytesRead = 0;
	OSStatus theError = FSReadFork(mRefNum, thePositionMode, inOffset, ioNumberBytes, outData, &theActualBytesRead);
	
	//	set the return value
	ioNumberBytes = theActualBytesRead;

	//	check for errors
	ThrowIfError(theError, CAException(theError), "HLFile::ReadBytes: couldn't read from the fork");
}

void	HLFile::WriteRawBytes(SInt64 inOffset, UInt32& ioNumberBytes, void* inData, bool inCache)
{
	ThrowIf(!mOpenForWriting, CAException(fnOpnErr), "HLFile::WriteBytes: file isn't open");
	
	//	figure out the positioning mask in light of the caching request
	UInt16 thePositionMode = fsFromStart;
	if(!inCache)
	{
		thePositionMode += noCacheMask;
	}
	
	//	write the data
	UInt32 theActualBytesWritten = 0;
	OSStatus theError = FSWriteFork(mRefNum, thePositionMode, inOffset, ioNumberBytes, inData, &theActualBytesWritten);
	
	//	set the return value
	ioNumberBytes = theActualBytesWritten;

	//	check for errors
	ThrowIfError(theError, CAException(theError), "HLFile::WriteBytes: couldn't write to the fork");
}

//=============================================================================
//	HLFileFactory
//=============================================================================

HLFileFactory::HLFileFactory(UInt32 inObjectType, CFStringRef inNameExtension, bool inUsesResourceFork)
:
	HLFileSystemObjectFactory(inObjectType, inNameExtension),
	mUsesResourceFork(inUsesResourceFork)
{
}

HLFileFactory::~HLFileFactory()
{
}

bool	HLFileFactory::ObjectIsA_FSRef(const FSRef& inFSRef) const
{
	bool theAnswer = false;
	
	//	make a file system object so we can query a few things
	HLFileSystemObject theFileSystemObject(inFSRef);
	
	//	first check the file name extension
	if(mNameExtension != NULL)
	{
		CACFString theExtension(theFileSystemObject.CopyNameExtension());
		if(theExtension.IsValid())
		{
			theAnswer = CFEqual(mNameExtension, theExtension.GetCFString());
		}
	}
	
	//	now check the file type
	if(!theAnswer && (mObjectType != 0))
	{
		theAnswer = mObjectType == theFileSystemObject.GetType();
	}
	
	return theAnswer;
}

bool	HLFileFactory::ObjectIsA_Path(const char* inPath) const
{
	FSRef theFSRef;
	bool theFileExists = HLFileSystem::MakeFSRefFromPath(inPath, theFSRef);
	ThrowIf(!theFileExists, CAException(fnfErr), "HLFileFactory::ObjectIsA: the object doesn't exist");
	return ObjectIsA_FSRef(theFSRef);
}
	
HLFileSystemObject*	HLFileFactory::Allocate_FSRef(const FSRef& inFSRef)
{
	HLFile* theFile = CreateObject(inFSRef);
	theFile->Prepare();
	return theFile;
}

HLFileSystemObject*	HLFileFactory::Allocate_Path(const char* inPath)
{
	FSRef theFSRef;
	bool theFileExists = HLFileSystem::MakeFSRefFromPath(inPath, theFSRef);
	ThrowIf(!theFileExists, CAException(fnfErr), "HLFileFactory::Allocate: the object doesn't exist");
	return Allocate_FSRef(theFSRef);
}
	
HLFileSystemObject*	HLFileFactory::Create_FSRef(const FSRef& inParentFSRef, CFStringRef inName, const void* inData, UInt32 inDataSize)
{
	OSStatus theError = 0;
	
	CACFString theFileName(inName, false);
	UInt32 theFileNameLength = 255;
	UniChar	theFileNameString[255];
	theFileName.GetUnicodeString(theFileNameString, theFileNameLength);
	
	//	create the file on disk
	FSRef theFSRef;
	if(!mUsesResourceFork)
	{
		theError = FSCreateFileUnicode(&inParentFSRef, theFileNameLength, theFileNameString, kFSCatInfoNone, NULL, &theFSRef, NULL);
		ThrowIfError(theError, CAException(theError), "HLFileFactory::CreateFile: couldn't create the file");
	}
	else
	{
		HFSUniStr255 theForkName;
		theError = FSGetResourceForkName(&theForkName);
		ThrowIfError(theError, CAException(theError), "HLFileFactory::CreateFile: couldn't get the name of the resource fork");
		
		theError = FSCreateResourceFile(&inParentFSRef, theFileNameLength, theFileNameString, kFSCatInfoNone, NULL, theForkName.length, theForkName.unicode, &theFSRef, NULL);
		ThrowIfError(theError, CAException(theError), "HLFileFactory::CreateFile: couldn't create the file");
	}
	
	//	make the object
	HLFile* theFile = CreateObject(theFSRef);
	theFile->PrepareNew(inData, inDataSize);
	return theFile;
}

HLFileSystemObject*	HLFileFactory::Create_Path(const char* inPath, bool inCreateParentDirectories, const void* inData, UInt32 inDataSize)
{
	//	can't make the file if it already exists
	ThrowIf(HLFileSystem::ObjectExists(inPath), CAException(dupFNErr), "HLFileFactory::CreateFile: the file already exists");
	
	//	normalize the path
	char theNormalizedPath[PATH_MAX];
	HLFileSystem::NormalizePath(inPath, theNormalizedPath, PATH_MAX);
	
	//	extract the file name
	CACFString theFileName(HLFileSystem::CopyNameFromPath(theNormalizedPath));
	
	//	extract the parent path
	char	theParentPath[PATH_MAX];
	HLFileSystem::CopyParentPathFromPath(theNormalizedPath, theParentPath, PATH_MAX);
	
	//	make an FSRef for the parent path
	FSRef theParentFSRef;
	bool theParentExists = HLFileSystem::MakeFSRefFromPath(theParentPath, theParentFSRef);
	if(!theParentExists && inCreateParentDirectories)
	{
		//	the parent directory doesn't exist, but we're allowed to create it
		CAAutoDelete<HLFileSystemObject> theObject(HLFileSystem::CreateObject(theParentPath, inCreateParentDirectories, HLDirectory::kObjectType, NULL, 0));
		theObject->GetFSRef(theParentFSRef);
	}
	else if(!theParentExists)
	{
		//	don't have an FSRef for the parent, so we have to bail
		DebugMessage("HLFileFactory::CreateFile: couldn't make an FSRef for the parent");
		throw CAException(fnfErr);
	}
	
	//	we have the parent FSRef and the CFString name, so create the file
	return Create_FSRef(theParentFSRef, theFileName.GetCFString(), inData, inDataSize);
}

HLFile*	HLFileFactory::CreateObject(const FSRef& inFSRef)
{
	return new HLFile(inFSRef);
}
