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
	HLFileSystem.h

=============================================================================*/
#if !defined(__HLFileSystem_h__)
#define __HLFileSystem_h__

//=============================================================================
//	Includes
//=============================================================================

#include <CoreAudio/CoreAudioTypes.h>
#include <CoreServices/CoreServices.h>
#include <vector>

//=============================================================================
//	Types
//=============================================================================

class	HLFileSystemObject;
class	HLFileSystemObjectFactory;

//=============================================================================
//	HLFileSystem
//
//	Polymorphic routines for managing file system objects
//=============================================================================

class HLFileSystem
{

//	Construction/Destruction
public:
	static void							Initialize();

//	Object Manipulation
public:
	static bool							ObjectExists(const FSRef& inFSRef);
	static bool							ObjectExists(const char* inPath);
	static bool							ObjectExists(CFStringRef inPath);
	
	static HLFileSystemObject*			AllocateObject(const FSRef& inFSRef);
	static HLFileSystemObject*			AllocateObject(const char* inPath);
	static HLFileSystemObject*			AllocateObject(CFStringRef inPath);
	
	static HLFileSystemObject*			CreateObject(const FSRef& inParentFSRef, CFStringRef inName, UInt32 inObjectType, const void* inObjectData, UInt32 inObjectDataSize);
	static HLFileSystemObject*			CreateObject(const char* inPath, bool inCreateParentDirectories, UInt32 inObjectType, const void* inObjectData, UInt32 inObjectDataSize);
	static HLFileSystemObject*			CreateObject(CFStringRef inPath, bool inCreateParentDirectories, UInt32 inObjectType, const void* inObjectData, UInt32 inObjectDataSize);
	
	static void							DeleteObject(const FSRef& inFSRef);
	static void							DeleteObject(const char* inPath);
	static void							DeleteObject(CFStringRef inPath);
	
	static void							SwapObjects(const FSRef& inObject1, const FSRef& inObject2);
	static void							SwapObjects(const char* inObject1, const char* inObject2);
	static void							SwapObjects(CFStringRef inObject1, CFStringRef inObject2);
	
//	Path Manipulation Utilities
public:
	static bool							MakeFSRefFromPath(const char* inPath, FSRef& outFSRef);
	static void							NormalizePath(const char* inPath, char* outNormalizedPath, UInt32 inMaxNormalizedPathLength);
	static CFStringRef					CopyNameFromPath(const char* inPath);
	static void							CopyParentPathFromPath(const char* inPath, char* outParentPath, UInt32 inMaxParentPathLength);
	static bool							IsRelativePath(const char* inPath);
	static void							MakeAbsolutePath(const char* inRelativePath, char* outAbsolutePath, UInt32 inMaxAbsolutePathLength);

	static const char					kPathDelimiter[];
	static const UInt32					kPathDelimiterLength;

//	Factory Routines
public:
	static void							AddObjectFactory(HLFileSystemObjectFactory* inObjectFactory);
	static void							RemoveObjectFactory(HLFileSystemObjectFactory* inObjectFactory);
	static HLFileSystemObjectFactory*	GetDefaultFileFactory();
	static HLFileSystemObjectFactory*	GetDefaultDirectoryFactory();
	static HLFileSystemObjectFactory*	FindObjectFactory(UInt32 inFileType);
	static void							AddStandardObjectFactories();
	
private:
	typedef std::vector<HLFileSystemObjectFactory*>	ObjectFactoryList;
	
	static ObjectFactoryList*			sObjectFactoryList;
	static HLFileSystemObjectFactory*	sDefaultFileFactory;
	static HLFileSystemObjectFactory*	sDefaultDirectoryFactory;

};

#endif
