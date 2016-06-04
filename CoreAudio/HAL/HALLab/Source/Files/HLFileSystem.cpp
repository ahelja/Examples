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
	HLFileSystem.cpp

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

#include "HLFileSystem.h"
#include "HLFileSystemObject.h"
#include "HLFileSystemObjectFactory.h"
#include "HLDirectory.h"
#include "HLFile.h"
#include "HLAudioFile.h"
#include "CACFString.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include "HLFileSystemCFStringConstants.h"
#include <algorithm>
#include <unistd.h>

//=============================================================================
//	HLFileSystem
//=============================================================================

void	HLFileSystem::Initialize()
{
	HLFileSystemCFStringConstants::Initialize();
	AddStandardObjectFactories();
}

bool	HLFileSystem::ObjectExists(const FSRef& inFSRef)
{
	//	getting the name of the file is enough to figure out if it exists or not
	HFSUniStr255 theUniStr255;
	OSStatus theError = FSGetCatalogInfo(&inFSRef, kFSCatInfoNone, NULL, &theUniStr255, NULL, NULL);
	ThrowIf((theError != 0) && (theError != fnfErr), CAException(theError), "HLFileSystem::Exists: couldn't get the catalog info");
	return theError == 0;
}

bool	HLFileSystem::ObjectExists(const char* inPath)
{
	//	the process of making an FSRef out of the path will determine whether or not it exists
	FSRef theFSRef;
	return MakeFSRefFromPath(inPath, theFSRef);
}

bool	HLFileSystem::ObjectExists(CFStringRef inPath)
{
	CACFString theCFString(inPath, false);
	UInt32 theStringSize = PATH_MAX;
	char theString[PATH_MAX];
	theCFString.GetCString(theString, theStringSize);
	return ObjectExists(theString);
}

HLFileSystemObject*	HLFileSystem::AllocateObject(const FSRef& inFSRef)
{
	HLFileSystemObject* theAnswer = NULL;
	
	//	first check to see if it's a directory
	HLFileSystemObjectFactory* theDirectoryFactory = GetDefaultDirectoryFactory();
	if(theDirectoryFactory->ObjectIsA_FSRef(inFSRef))
	{
		theAnswer = theDirectoryFactory->Allocate_FSRef(inFSRef);
	}
	
	//	now look in the factory list to see if anybody claims this file
	if((theAnswer == NULL) && (sObjectFactoryList != NULL))
	{
		ObjectFactoryList::iterator theIterator = sObjectFactoryList->begin();
		while((theIterator != sObjectFactoryList->end()) && (theAnswer == NULL))
		{
			HLFileSystemObjectFactory* theObjectFactory = *theIterator;

			if(theObjectFactory->ObjectIsA_FSRef(inFSRef))
			{
				theAnswer = theObjectFactory->Allocate_FSRef(inFSRef);
			}
			
			std::advance(theIterator, 1);
		}
	}
	
	//	if no factory claims this file, use the default file factory
	if(theAnswer == NULL)
	{
		HLFileSystemObjectFactory* theFileFactory = GetDefaultFileFactory();
		theAnswer = theFileFactory->Allocate_FSRef(inFSRef);
	}
	
	return theAnswer;
}

HLFileSystemObject*	HLFileSystem::AllocateObject(const char* inPath)
{
	FSRef theFSRef;
	bool theFileExists = MakeFSRefFromPath(inPath, theFSRef);
	ThrowIf(!theFileExists, CAException(fnfErr), "HLFileSystem::AllocateObject: couldn't allocate the object");
	return AllocateObject(theFSRef);
}

HLFileSystemObject*	HLFileSystem::AllocateObject(CFStringRef inPath)
{
	CACFString theCFString(inPath, false);
	UInt32 theStringSize = PATH_MAX;
	char theString[PATH_MAX];
	theCFString.GetCString(theString, theStringSize);
	return AllocateObject(theString);
}

HLFileSystemObject*	HLFileSystem::CreateObject(const FSRef& inParentFSRef, CFStringRef inName, UInt32 inObjectType, const void* inObjectData, UInt32 inObjectDataSize)
{
	//	get a factory for the given object type
	HLFileSystemObjectFactory* theObjectFactory = FindObjectFactory(inObjectType);
	
	//	use it to create the file
	HLFileSystemObject* theObject = theObjectFactory->Create_FSRef(inParentFSRef, inName, inObjectData, inObjectDataSize);
	
	//	set the file type if we need to
	if(theObject != NULL)
	{
		UInt32 theObjectType = theObject->GetType();
		if((theObjectType == 0) && (inObjectType != 0) && (inObjectType != HLDirectory::kObjectType))
		{
			theObject->SetTypeOnFile(inObjectType);
		}
	}
	
	return theObject;
}

HLFileSystemObject*	HLFileSystem::CreateObject(const char* inPath, bool inCreateParentDirectories, UInt32 inObjectType, const void* inObjectData, UInt32 inObjectDataSize)
{	
	//	get a factory for the given object type
	HLFileSystemObjectFactory* theObjectFactory = FindObjectFactory(inObjectType);
	
	//	use it to create the file
	HLFileSystemObject* theObject = theObjectFactory->Create_Path(inPath, inCreateParentDirectories, inObjectData, inObjectDataSize);
	
	//	set the file type if we need to
	if(theObject != NULL)
	{
		UInt32 theObjectType = theObject->GetType();
		if((theObjectType == 0) && (inObjectType != 0) && (inObjectType != HLDirectory::kObjectType))
		{
			theObject->SetTypeOnFile(inObjectType);
		}
	}
	
	return theObject;
}

HLFileSystemObject*	HLFileSystem::CreateObject(CFStringRef inPath, bool inCreateParentDirectories, UInt32 inObjectType, const void* inObjectData, UInt32 inObjectDataSize)
{
	CACFString theCFString(inPath, false);
	UInt32 theStringSize = PATH_MAX;
	char theString[PATH_MAX];
	theCFString.GetCString(theString, theStringSize);
	return CreateObject(theString, inCreateParentDirectories, inObjectType, inObjectData, inObjectDataSize);
}

void	HLFileSystem::DeleteObject(const FSRef& inFSRef)
{
	OSStatus theError = FSDeleteObject(&inFSRef);
	ThrowIfError(theError, CAException(theError), "HLFileSystem::Delete: couldn't delete the object");
}

void	HLFileSystem::DeleteObject(const char* inPath)
{
	//	make an FSRef for the path
	FSRef theFSRef;
	bool theFileExists = MakeFSRefFromPath(inPath, theFSRef);
	ThrowIf(!theFileExists, CAException(fnfErr), "HLFileSystem::Delete: the object doesn't exist");
	DeleteObject(theFSRef);
}

void	HLFileSystem::DeleteObject(CFStringRef inPath)
{
	CACFString theCFString(inPath, false);
	UInt32 theStringSize = PATH_MAX;
	char theString[PATH_MAX];
	theCFString.GetCString(theString, theStringSize);
	DeleteObject(theString);
}

void	HLFileSystem::SwapObjects(const FSRef& inFSRef1, const FSRef& inFSRef2)
{
	OSStatus theError = FSExchangeObjects(&inFSRef1, &inFSRef2);
	ThrowIfError(theError, CAException(theError), "HLFileSystem::SwapObjects: couldn't swap the objects");
}

void	HLFileSystem::SwapObjects(const char* inPath1, const char* inPath2)
{
	//	make an FSRef for the paths
	FSRef theFSRef1;
	bool theFileExists = MakeFSRefFromPath(inPath1, theFSRef1);
	ThrowIf(!theFileExists, CAException(fnfErr), "HLFileSystem::SwapObjects: the first object doesn't exist");
	
	FSRef theFSRef2;
	theFileExists = MakeFSRefFromPath(inPath2, theFSRef2);
	ThrowIf(!theFileExists, CAException(fnfErr), "HLFileSystem::SwapObjects: the second object doesn't exist");
	
	SwapObjects(theFSRef1, theFSRef2);
}

void	HLFileSystem::SwapObjects(CFStringRef inPath1, CFStringRef inPath2)
{
	CACFString theCFString1(inPath1, false);
	UInt32 theStringSize1 = PATH_MAX;
	char theString1[PATH_MAX];
	theCFString1.GetCString(theString1, theStringSize1);
	
	CACFString theCFString2(inPath2, false);
	UInt32 theStringSize2 = PATH_MAX;
	char theString2[PATH_MAX];
	theCFString2.GetCString(theString2, theStringSize2);
	
	SwapObjects(theString1, theString2);
}

bool	HLFileSystem::MakeFSRefFromPath(const char* inPath, FSRef& outFSRef)
{
	Boolean isDirectory;
	OSStatus theError = FSPathMakeRef(reinterpret_cast<const UInt8*>(inPath), &outFSRef, &isDirectory);
	ThrowIf((theError != 0) && (theError != fnfErr), CAException(theError), "HLFileSystem::MakeFSRefFromPath: couldn't make the FSRef");
	return theError == 0;
}

void	HLFileSystem::NormalizePath(const char* inPath, char* outNormalizedPath, UInt32 inMaxNormalizedPathLength)
{
	const char* thePath = inPath;
	
	//	first make sure we're dealing with an absolute path
	if(IsRelativePath(thePath))
	{
		MakeAbsolutePath(thePath, outNormalizedPath, inMaxNormalizedPathLength);
	}
	else
	{
		strncpy(outNormalizedPath, thePath, inMaxNormalizedPathLength - 1);
	}
	
	//	now lop off any trailing delimters
	UInt32 thePathLength = strlen(outNormalizedPath);
	while((thePathLength > 0) && (outNormalizedPath[thePathLength - 1] == kPathDelimiter[0]))
	{
		outNormalizedPath[thePathLength - 1] = 0;
		--thePathLength;
	}
}

CFStringRef	HLFileSystem::CopyNameFromPath(const char* inPath)
{
	//	the end is the last character
	UInt32 theFileNameEnd = strlen(inPath) - 1;
	
	//	find the start
	UInt32 theFileNameStart = theFileNameEnd;
	while((theFileNameStart > 0) && (inPath[theFileNameStart - 1] != kPathDelimiter[0]))
	{
		--theFileNameStart;
	}
	
	//	copy the name
	char theName[PATH_MAX];
	memcpy(theName, &inPath[theFileNameStart], theFileNameEnd - theFileNameStart + 1);
	theName[theFileNameEnd - theFileNameStart + 1] = 0;
	
	//	make a CFString from it
	return CFStringCreateWithCString(NULL, theName, kCFStringEncodingASCII);
}

void	HLFileSystem::CopyParentPathFromPath(const char* inPath, char* outParentPath, UInt32 inMaxParentPathLength)
{
	//	the end is just before the last delimiter
	UInt32 theParentPathEnd = strlen(inPath) - 1;
	while((theParentPathEnd > 0) && (inPath[theParentPathEnd - 1] != kPathDelimiter[0]))
	{
		--theParentPathEnd;
	}
	
	//	this puts us at the beginning of the name, so skip back 2 characters for the end of the parent path
	theParentPathEnd -= 2;
	
	//	make sure we don't fall off the end
	if((theParentPathEnd + 1) > inMaxParentPathLength)
	{
		theParentPathEnd = inMaxParentPathLength - 1;
	}
	
	//	copy the parent path
	memcpy(outParentPath, inPath, theParentPathEnd + 1);
	outParentPath[theParentPathEnd + 1] = 0;
}

bool	HLFileSystem::IsRelativePath(const char* inPath)
{
	return inPath[0] != kPathDelimiter[0];
}

void	HLFileSystem::MakeAbsolutePath(const char* inRelativePath, char* outAbsolutePath, UInt32 inMaxAbsolutePathLength)
{
	//  This method should be rewritten to use glob(3)
	
	//	get the path to the current working directory
	getcwd(outAbsolutePath, inMaxAbsolutePathLength);
	
	//	append the path delimiter
	ThrowIf((strlen(outAbsolutePath) + kPathDelimiterLength) > (inMaxAbsolutePathLength - 1), CAException(paramErr), "HLFileSystem::MakeAbsolutePath: path is too long to add the path delimiter");
	strcat(outAbsolutePath, kPathDelimiter);
	
	//	append the relative path
	ThrowIf((strlen(outAbsolutePath) + strlen(inRelativePath)) > (inMaxAbsolutePathLength - 1), CAException(paramErr), "HLFileSystem::MakeAbsolutePath: path is too long");
	strcat(outAbsolutePath, inRelativePath);
}

const char		HLFileSystem::kPathDelimiter[] = "/";
const UInt32	HLFileSystem::kPathDelimiterLength = 1;

void	HLFileSystem::AddObjectFactory(HLFileSystemObjectFactory* inObjectFactory)
{
	if(sObjectFactoryList == NULL)
	{
		sObjectFactoryList = new ObjectFactoryList;
	}
	ObjectFactoryList::iterator theIterator = std::find(sObjectFactoryList->begin(), sObjectFactoryList->end(), inObjectFactory);
	if(theIterator == sObjectFactoryList->end())
	{
		sObjectFactoryList->push_back(inObjectFactory);
	}
}

void	HLFileSystem::RemoveObjectFactory(HLFileSystemObjectFactory* inObjectFactory)
{
	if(inObjectFactory != NULL)
	{
		ObjectFactoryList::iterator theIterator = std::find(sObjectFactoryList->begin(), sObjectFactoryList->end(), inObjectFactory);
		if(theIterator != sObjectFactoryList->end())
		{
			sObjectFactoryList->erase(theIterator);
		}
	}
}

HLFileSystemObjectFactory*	HLFileSystem::GetDefaultFileFactory()
{
	if(sDefaultFileFactory == NULL)
	{
		sDefaultFileFactory = new HLFileFactory(HLFile::kObjectType, NULL, false);;
	}
	return sDefaultFileFactory;
}

HLFileSystemObjectFactory*	HLFileSystem::GetDefaultDirectoryFactory()
{
	if(sDefaultDirectoryFactory == NULL)
	{
		sDefaultDirectoryFactory = new HLDirectoryFactory();
	}
	return sDefaultDirectoryFactory;
}

HLFileSystemObjectFactory*	HLFileSystem::FindObjectFactory(UInt32 inFileType)
{
	HLFileSystemObjectFactory* theAnswer = NULL;
	
	//	first, check the list
	if(sObjectFactoryList != NULL)
	{
		ObjectFactoryList::iterator theIterator = sObjectFactoryList->begin();
		while((theIterator != sObjectFactoryList->end()) && (theAnswer == NULL))
		{
			HLFileSystemObjectFactory* theObjectFactory = *theIterator;
			if(theObjectFactory->GetObjectType() == inFileType)
			{
				theAnswer = theObjectFactory;
			}
			std::advance(theIterator, 1);
		}
	}
	
	//	not in the list? then use the appropriate default factory
	if(theAnswer == NULL)
	{
		//	unless they asked for the directory factory
		if(inFileType == HLDirectory::kObjectType)
		{
			theAnswer = GetDefaultDirectoryFactory();
		}
		else
		{
			//	they're getting the file factory
			theAnswer = GetDefaultFileFactory();
		}
	}
	
	return theAnswer;
}

void	HLFileSystem::AddStandardObjectFactories()
{
	AddObjectFactory(new HLAudioFileFactory(kAudioFileAIFFType, HLFileSystemCFStringConstants::sAIFFNameExtension));
	AddObjectFactory(new HLAudioFileFactory(kAudioFileAIFCType, HLFileSystemCFStringConstants::sAIFFNameExtension));
	AddObjectFactory(new HLAudioFileFactory(kAudioFileWAVEType, HLFileSystemCFStringConstants::sWAVNameExtension));
	AddObjectFactory(new HLAudioFileFactory(kAudioFileSoundDesigner2Type, HLFileSystemCFStringConstants::sSD2NameExtension));
}

HLFileSystem::ObjectFactoryList*	HLFileSystem::sObjectFactoryList = NULL;
HLFileSystemObjectFactory*			HLFileSystem::sDefaultFileFactory = NULL;
HLFileSystemObjectFactory*			HLFileSystem::sDefaultDirectoryFactory = NULL;
