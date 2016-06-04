/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
	HLDirectory.cpp

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

#include "HLDirectory.h"
#include "HLFileSystem.h"
#include "CAAutoDisposer.h"
#include "CACFString.h"
#include "CADebugMacros.h"
#include "CAException.h"

//=============================================================================
//	HLDirectory
//=============================================================================

HLDirectory::HLDirectory(const FSRef& inFSRef)
:
	HLFileSystemObject(inFSRef),
	mItemsFetched(false),
	mNumberItems(0),
	mItems(NULL)
{
}

HLDirectory::~HLDirectory()
{
	delete mItems;
}

bool	HLDirectory::IsFile() const
{
	return false;
}

bool	HLDirectory::IsDirectory() const
{
	return true;
}

bool	HLDirectory::IsVolume() const
{
	return false;
}

UInt32	HLDirectory::GetNumberItems()
{
	FetchItems();
	return mItems->size();
}

void	HLDirectory::GetItem(UInt32 inIndex, FSRef& outFSRef)
{
	FetchItems();
	
	ItemList::iterator theIterator = mItems->begin();
	std::advance(theIterator, inIndex);
	
	ThrowIf(theIterator == mItems->end(), CAException(paramErr), "HLDirectory::GetItem: index out of range");
	
	memcpy(&outFSRef, &(*theIterator), sizeof(FSRef));
}

HLFileSystemObject*	HLDirectory::GetItem(UInt32 inIndex)
{
	FSRef theFSRef;
	GetItem(inIndex, theFSRef);
	
	return HLFileSystem::AllocateObject(theFSRef);
}

void	HLDirectory::FetchItems()
{
	if(!mItemsFetched)
	{
		mItems = new ItemList;
		
		//	make an FSIterator for this directory
		FSIterator theFSIterator;
		OSStatus theError = FSOpenIterator(&mFSRef, kFSIterateFlat, &theFSIterator);
		ThrowIfError(theError, CAException(theError), "HLDirectory::FetchItems: couldn't allocate the FSIterator");
		
		try
		{
			//	iterate through the items in the directory and get the FSRefs
			do
			{
				FSRef theFSRef;
				UInt32 theNumberItemsFetched = 0;
				
				theError = FSGetCatalogInfoBulk(theFSIterator, 1, &theNumberItemsFetched, NULL, kFSCatInfoNone, NULL, &theFSRef, NULL, NULL);
				ThrowIf((theError != 0) && (theError != errFSNoMoreItems), CAException(theError), "HLDirectory::FetchItems: couldn't get the catalog info");
				
				if(theError != errFSNoMoreItems)
				{
					mItems->push_back(theFSRef);
				}
			}
			while(theError != errFSNoMoreItems);
		
			mItemsFetched = true;
		}
		catch(...)
		{
			FSCloseIterator(theFSIterator);
			throw;
		}
	
		//	close the iterator
		FSCloseIterator(theFSIterator);
	}
}

//=============================================================================
//	HLDirectoryFactory
//=============================================================================

HLDirectoryFactory::HLDirectoryFactory()
:
	HLFileSystemObjectFactory(HLDirectory::kObjectType, NULL)
{
}

HLDirectoryFactory::~HLDirectoryFactory()
{
}

bool	HLDirectoryFactory::ObjectIsA_FSRef(const FSRef& inFSRef) const
{
	FSCatalogInfo theCatalogInfo;
	OSStatus theError = FSGetCatalogInfo(&inFSRef, kFSCatInfoNodeFlags, &theCatalogInfo, NULL, NULL, NULL);
	ThrowIfError(theError, CAException(theError), "HLDirectoryFactory::ObjectIsA: couldn't get the catalog info");
	return (theCatalogInfo.nodeFlags & kFSNodeIsDirectoryMask) != 0;
}

bool	HLDirectoryFactory::ObjectIsA_Path(const char* inPath) const
{
	FSRef theFSRef;
	bool theFileExists = HLFileSystem::MakeFSRefFromPath(inPath, theFSRef);
	ThrowIf(!theFileExists, CAException(fnfErr), "HLDirectoryFactory::ObjectIsA: the object doesn't exist");
	return ObjectIsA_FSRef(theFSRef);
}
	
HLFileSystemObject*	HLDirectoryFactory::Allocate_FSRef(const FSRef& inFSRef)
{
	return new HLDirectory(inFSRef);
}

HLFileSystemObject*	HLDirectoryFactory::Allocate_Path(const char* inPath)
{
	FSRef theFSRef;
	bool theFileExists = HLFileSystem::MakeFSRefFromPath(inPath, theFSRef);
	ThrowIf(!theFileExists, CAException(fnfErr), "HLDirectoryFactory::Allocate: the object doesn't exist");
	return Allocate_FSRef(theFSRef);
}
	
HLFileSystemObject*	HLDirectoryFactory::Create_FSRef(const FSRef& inParentFSRef, CFStringRef inName, const void* /*inData*/, UInt32 /*inDataSize*/)
{
	CACFString theDirectoryName(inName, false);
	UInt32 theDirectoryNameLength = 255;
	UniChar	theDirectoryNameString[255];
	theDirectoryName.GetUnicodeString(theDirectoryNameString, theDirectoryNameLength);
	
	//	create the directory
	FSRef theFSRef;
	OSStatus theError = FSCreateDirectoryUnicode(&inParentFSRef, theDirectoryNameLength, theDirectoryNameString, kFSCatInfoNone, NULL, &theFSRef, NULL, NULL);
	ThrowIfError(theError, CAException(theError), "HLDirectoryFactory::Create: couldn't create the directory");
	
	return Allocate_FSRef(theFSRef);
}

HLFileSystemObject*	HLDirectoryFactory::Create_Path(const char* inPath, bool inCreateParentDirectories, const void* inData, UInt32 inDataSize)
{
	//	can't make the directory if it already exists
	ThrowIf(HLFileSystem::ObjectExists(inPath), CAException(dupFNErr), "HLDirectoryFactory::Create: the directory already exists");
	
	//	normalize the path
	char theNormalizedPath[PATH_MAX];
	HLFileSystem::NormalizePath(inPath, theNormalizedPath, PATH_MAX);
	
	//	extract the directory name
	CACFString theDirectoryName(HLFileSystem::CopyNameFromPath(theNormalizedPath));
	
	//	extract the parent path
	char theParentPath[PATH_MAX];
	HLFileSystem::CopyParentPathFromPath(theNormalizedPath, theParentPath, PATH_MAX);
	
	//	make an FSRef for the parent path
	FSRef theParentFSRef;
	bool theParentExists = HLFileSystem::MakeFSRefFromPath(theParentPath, theParentFSRef);
	if(!theParentExists && inCreateParentDirectories)
	{
		//	the parent directory doesn't exist, but we're allowed to create it
		CAAutoDelete<HLFileSystemObject> theObject(Create_Path(theParentPath, inCreateParentDirectories, NULL, 0));
		theObject->GetFSRef(theParentFSRef);
	}
	else if(!theParentExists)
	{
		//	don't have an FSRef for the parent, so we have to bail
		DebugMessage("HLFileSystemObject::CreateDirectory: couldn't make an FSRef for the parent");
		throw CAException(fnfErr);
	}
	
	//	we have the parent FSRef and the CFString name, so create the directory
	return Create_FSRef(theParentFSRef, theDirectoryName.GetCFString(), inData, inDataSize);
}
