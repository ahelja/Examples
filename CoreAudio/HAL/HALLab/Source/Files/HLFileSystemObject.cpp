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
	HLFileSystemObject.cpp

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

#include "HLFileSystemObject.h"
#include "CACFString.h"
#include "HLFileSystemCFStringConstants.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include <ApplicationServices/ApplicationServices.h>
#include <string.h>

//=============================================================================
//	HLFileSystemObject
//=============================================================================

HLFileSystemObject::HLFileSystemObject(const FSRef& inFSRef)
{
	//	copy the FSRef
	memcpy(&mFSRef, &inFSRef, sizeof(FSRef));
}

HLFileSystemObject::~HLFileSystemObject()
{
}

bool	HLFileSystemObject::IsFile() const
{
	FSCatalogInfo theCatalogInfo;
	OSStatus theError = FSGetCatalogInfo(&mFSRef, kFSCatInfoNodeFlags, &theCatalogInfo, NULL, NULL, NULL);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::IsFile: couldn't get the catalog info");
	return (theCatalogInfo.nodeFlags & kFSNodeIsDirectoryMask) == 0;
}

bool	HLFileSystemObject::IsDirectory() const
{
	FSCatalogInfo theCatalogInfo;
	OSStatus theError = FSGetCatalogInfo(&mFSRef, kFSCatInfoNodeFlags, &theCatalogInfo, NULL, NULL, NULL);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::IsDirectory: couldn't get the catalog info");
	return (theCatalogInfo.nodeFlags & kFSNodeIsDirectoryMask) != 0;
}

bool	HLFileSystemObject::IsVolume() const
{
	FSCatalogInfo theCatalogInfo;
	OSStatus theError = FSGetCatalogInfo(&mFSRef, kFSCatInfoNodeFlags | kFSCatInfoNodeID, &theCatalogInfo, NULL, NULL, NULL);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::IsVolume: couldn't get the catalog info");
	return ((theCatalogInfo.nodeFlags & kFSNodeIsDirectoryMask) != 0) && (theCatalogInfo.nodeID == fsRtDirID);
}

void	HLFileSystemObject::GetFSRef(FSRef& outFSRef) const
{
	memcpy(&outFSRef, &mFSRef, sizeof(FSRef));
}

void	HLFileSystemObject::GetParentFSRef(FSRef& outFSRef) const
{
	OSStatus theError = FSGetCatalogInfo(&mFSRef, kFSCatInfoNone, NULL, NULL, NULL, &outFSRef);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::GetParentFSRef: couldn't get the catalog info");
}

CFStringRef	HLFileSystemObject::CopyName() const
{
	//	get the whole unicode name from the catalog info
	HFSUniStr255 theUniStr255;
	OSStatus theError = FSGetCatalogInfo(&mFSRef, kFSCatInfoNone, NULL, &theUniStr255, NULL, NULL);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::CopyName: couldn't get the catalog information");
	
	//	make a CFString with it
	return CFStringCreateWithCharacters(NULL, theUniStr255.unicode, theUniStr255.length);
}

CFStringRef	HLFileSystemObject::CopyDisplayName() const
{
	//	get the display name from Launch Services
	CFStringRef theDisplayName = NULL;
	OSStatus theError = LSCopyDisplayNameForRef(&mFSRef, &theDisplayName);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::CopyDisplayName: couldn't get the display name");
	
	return theDisplayName;
}

void	HLFileSystemObject::SetName(CFStringRef inName)
{
	//	the FSRef for a file can change after renaming,
	//	so don't allow it to be done while the file is open
	ThrowIf(IsOpenForReading() || IsOpenForWriting(), CAException(fBsyErr), "HLFileSystemObject::SetName: can't change the name of an open file");
	
	//	make a raw unicode string out of the CFString
	CACFString theFileName(inName, false);
	UInt32 theFileNameLength = 255;
	UniChar	theFileNameString[255];
	theFileName.GetUnicodeString(theFileNameString, theFileNameLength);
	
	//	save off the current FSRef, since it may change
	FSRef theOldFSRef;
	memcpy(&theOldFSRef, &mFSRef, sizeof(FSRef));
	
	//	rename the file, getting us the new FSRef
	OSStatus theError = FSRenameUnicode(&theOldFSRef, theFileNameLength, theFileNameString, kTextEncodingUnknown, &mFSRef);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::SetName: couldn't rename the file");
}

CFStringRef	HLFileSystemObject::CopyNameExtension() const
{
	//	get the file name from the catalog info
	HFSUniStr255 theUniStr255;
	OSStatus theError = FSGetCatalogInfo(&mFSRef, kFSCatInfoNone, NULL, &theUniStr255, NULL, NULL);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::CopyNameExtension: couldn't get the catalog information");
	
	//	use Launch Services to find the start of the extension
	UniCharCount theExtensionStart = 0;
	LSGetExtensionInfo(theUniStr255.length, theUniStr255.unicode, &theExtensionStart);
	
	//	assume that there wasn't an extension
	CFStringRef theExtension = NULL;
	if(theExtensionStart != kLSInvalidExtensionIndex)
	{
		//	but there was, so make a CFString out of it
		theExtension = CFStringCreateWithCharacters(NULL, &theUniStr255.unicode[theExtensionStart], theUniStr255.length - theExtensionStart);
	}
	
	return theExtension;
}

void	HLFileSystemObject::SetNameExtension()
{
}

void	HLFileSystemObject::SetNameExtensionOnFile(CFStringRef inNameExtension)
{
	//	the FSRef for a file can change after renaming,
	//	so don't allow it to be done while the file is open
	ThrowIf(IsOpenForReading() || IsOpenForWriting(), CAException(fBsyErr), "HLFileSystemObject::SetNameExtension: can't change the file name extension of an open file");
	
	//	make a raw unicode string out of the CFString containing the new extension
	CACFString theExtension(inNameExtension, false);
	UInt32 theExtensionLength = 255;
	UniChar	theExtensionString[255];
	theExtension.GetUnicodeString(theExtensionString, theExtensionLength);

	//	get the file name from the catalog info
	HFSUniStr255 theNameString;
	OSStatus theError = FSGetCatalogInfo(&mFSRef, kFSCatInfoNone, NULL, &theNameString, NULL, NULL);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::SetNameExtension: couldn't get the catalog information");
	
	//	use Launch Services to find the start of the extension
	UniCharCount theExtensionStart = 0;
	LSGetExtensionInfo(theNameString.length, theNameString.unicode, &theExtensionStart);
	
	UInt32 theCharsToCopy;
	if(theExtensionStart != kLSInvalidExtensionIndex)
	{
		//	there already was an extension, so replace it
		
		//	figure out how many characters worth of extension can fit
		theCharsToCopy = 255UL - theExtensionStart;
		
		//	range it against the actual length of the extension
		theCharsToCopy = theExtensionLength < theCharsToCopy ? theExtensionLength : theCharsToCopy;
		
		//	copy the extension
		memcpy(&theNameString.unicode[theExtensionStart], theExtensionString, theCharsToCopy * sizeof(UniChar));
		
		//	update the length of the name string
		theNameString.length = theExtensionStart + theCharsToCopy;

		//	save off the current FSRef, since it may change
		FSRef theOldFSRef;
		memcpy(&theOldFSRef, &mFSRef, sizeof(FSRef));

		//	rename the file, getting us the new FSRef
		theError = FSRenameUnicode(&theOldFSRef, theNameString.length, theNameString.unicode, kTextEncodingUnknown, &mFSRef);
		ThrowIfError(theError, CAException(theError), "HLFileSystemObject::SetName: couldn't rename the file");
	}
	else if(theNameString.length < 254)
	{
		//	there isn't an extension yet, and there's room for at least two characters
		
		//	make a raw unicode string with the period
		CFRange thePeriodRange = { 0, 1 };
		UniChar	thePeriodString[255];
		CFStringGetCharacters(HLFileSystemCFStringConstants::sNameExtensionDelimiter, thePeriodRange, thePeriodString);
		
		//	copy it at the end of the string
		theNameString.unicode[theNameString.length] = thePeriodString[0];
		
		//	update the length
		theNameString.length += 1;
		
		//	figure out how many characters worth of extension can fit
		theCharsToCopy = 255UL - theNameString.length;

		//	range it against the actual length of the extension
		theCharsToCopy = theExtensionLength < theCharsToCopy ? theExtensionLength : theCharsToCopy;

		//	copy the extension
		memcpy(&theNameString.unicode[theNameString.length], theExtensionString, theCharsToCopy * sizeof(UniChar));

		//	update the length of the name string
		theNameString.length += theCharsToCopy;

		//	save off the current FSRef, since it may change
		FSRef theOldFSRef;
		memcpy(&theOldFSRef, &mFSRef, sizeof(FSRef));

		//	rename the file, getting us the new FSRef
		theError = FSRenameUnicode(&theOldFSRef, theNameString.length, theNameString.unicode, kTextEncodingUnknown, &mFSRef);
		ThrowIfError(theError, CAException(theError), "HLFileSystemObject::SetName: couldn't rename the file");
	}
}

UInt32	HLFileSystemObject::GetType() const
{
	//	get the file type from Launch Services
	LSItemInfoRecord theInfo;
	OSStatus theError = LSCopyItemInfoForRef(&mFSRef, kLSRequestTypeCreator, &theInfo);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::GetType: couldn't get the file type");
	
	return theInfo.filetype;
}

void	HLFileSystemObject::SetType()
{
}

void	HLFileSystemObject::SetTypeOnFile(UInt32 inType)
{
	//	get the current Finder info from the catalog
	FSCatalogInfo theInfo;
	OSStatus theError = FSGetCatalogInfo(&mFSRef, kFSCatInfoFinderInfo, &theInfo, NULL, NULL, NULL);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::SetType: couldn't get the catalog information");
	
	//	update just the type
	FInfo* theFinderInfo = reinterpret_cast<FInfo*>(&theInfo.finderInfo);
	theFinderInfo->fdType = inType;

	//	write it back out
	theError = FSSetCatalogInfo(&mFSRef, kFSCatInfoFinderInfo, &theInfo);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::SetType: couldn't set the catalog information");
}

UInt32	HLFileSystemObject::GetCreator() const
{
	//	get the file creator from Launch Services
	LSItemInfoRecord theInfo;
	OSStatus theError = LSCopyItemInfoForRef(&mFSRef, kLSRequestTypeCreator, &theInfo);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::SetName: couldn't rename the file");
	
	return theInfo.creator;
}

void	HLFileSystemObject::SetCreator(UInt32 inCreator)
{
	//	get the current Finder info from the catalog
	FSCatalogInfo theInfo;
	OSStatus theError = FSGetCatalogInfo(&mFSRef, kFSCatInfoFinderInfo, &theInfo, NULL, NULL, NULL);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::SetType: couldn't get the catalog information");
	
	//	update just the creator
	FInfo* theFinderInfo = reinterpret_cast<FInfo*>(&theInfo.finderInfo);
	theFinderInfo->fdCreator = inCreator;

	//	write it back out
	theError = FSSetCatalogInfo(&mFSRef, kFSCatInfoFinderInfo, &theInfo);
	ThrowIfError(theError, CAException(theError), "HLFileSystemObject::SetType: couldn't set the catalog information");
}

bool	HLFileSystemObject::IsOpenForReading() const
{
	return false;
}

bool	HLFileSystemObject::IsOpenForWriting() const
{
	return false;
}

