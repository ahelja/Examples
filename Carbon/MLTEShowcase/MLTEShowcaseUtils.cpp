/*
 *  MLTEShowcaseUtils.cpp
 *  MLTEShowcase
 *
 *  Created on Mon May 17 2004.
 *  Copyright 2004 Apple Computer, Inc. All rights reserved.

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

#include "MLTEShowcaseDefines.h"
#include "MLTEShowcaseUtils.h"

// Shows the spell check panel if it is hidden
void MyShowSpellCheckPanel(void)
{
	HICommand command;
	
	// Have to synthesize an event to show/hide the spelling panel
	command.commandID = kHICommandShowSpellingPanel;
	command.attributes = 0 | kHICommandFromMenu;
	command.menu.menuRef = GetMenuHandle(kHITextViewCommandMenuID);
	command.menu.menuItemIndex = kShowSpellMenuItemIndex;
	
	verify_noerr( ProcessHICommand ((HICommand*)(&command)) );
}

// a wrapper for console output.
void WarnString( char* msg )
{
	printf( msg );
	fflush( NULL );
}

// a wrapper for console output with an OSStatus
void WarnStatusString( char* formatStr, OSStatus status )
{
	static char buffer[255];
	sprintf( buffer, formatStr, status );
	WarnString( buffer );
}

// given a CFURL, make a file for it
OSStatus
CreateFileWithCFURL( CFURLRef fileUrl, FSRef& createdFSRef, OSType fileType, Boolean replacing )
{
	OSStatus  status = paramErr;
    CFURLRef pathUrl = NULL;
    FSRef pathRef;
	CFStringRef fileNameCFStr = NULL;
	UniChar* fileNameUStr = NULL;
	UInt32 fileNameLen;
	CFRange range;
	
    pathUrl = CFURLCreateCopyDeletingLastPathComponent( NULL, fileUrl );
	fileNameCFStr = CFURLCopyLastPathComponent( fileUrl );
	
	fileNameLen = CFStringGetLength( fileNameCFStr );
	
	if( fileNameLen > 0 )
	{
		fileNameUStr = (UniChar*)malloc( fileNameLen * 2 );
		
		range.location = 0;
		range.length = fileNameLen;
		
		// get file name as Unicode string
		CFStringGetCharacters( fileNameCFStr, range, fileNameUStr);
		
		// Convert the url to an FSRef.
		CFURLGetFSRef( pathUrl, &pathRef );
		
		if( replacing == true )
		{
			FSRef newFSRef;
			status = ::FSMakeFSRefUnicode( &pathRef,
											fileNameLen, 
											fileNameUStr,
											kTextEncodingUnicodeDefault,
											&newFSRef );
			status = FSDeleteObject( &newFSRef );
		}
		
		
		// Create an empty file on disk. This is a requirement when saving
		// via MLTE's CFURL API.
		status = ::FSCreateFileUnicode( &pathRef, 
										fileNameLen,
										fileNameUStr,
										kFSCatInfoNone, NULL,
										&createdFSRef, NULL /*fsspec*/ );
	}
	if( pathUrl != NULL )
		CFRelease( pathUrl );
	if( fileNameCFStr != NULL )
		CFRelease( fileNameCFStr );
	if( fileNameUStr != NULL )
		free( fileNameUStr );
	return status;
}

OSStatus
AddFontToATSUStyle( ATSUStyle& style, char* fontName, ByteCount nameLen  )
{
	ATSUFontID fontID;
	ATSUAttributeTag tag = kATSUFontTag;
	ByteCount size = sizeof(ATSUFontID);
	ATSUAttributeValuePtr value;
	
	require( fontName != NULL && nameLen > 0, FAIL );
	
	::ATSUFindFontFromName( fontName,
							nameLen,
							kFontFullName,
							kFontMacintoshPlatform,
							kCFStringEncodingMacRoman,
							(UInt32)kFontNoLanguage,
							&fontID );
	
	return ATSUSetAttributes( style, 1, &tag, &size, (void* const*)&fontID );
	FAIL:
	return paramErr;
}

CFNumberRef
UtilCreateEncodingCFNumberRef( UInt32 encoding )
{
	CFNumberRef encodingNumber = CFNumberCreate( NULL /*allocator*/,
												 kCFNumberLongType,
												 &encoding);
	return encodingNumber;
}

CFIndex MIN( CFIndex a, CFIndex b )
{
	return a < b ? a : b;
}
