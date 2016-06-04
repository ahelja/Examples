/*
 *  NavOpenDialog.cpp
 *  MLTEShowcase
 *
 *  Created on Wed Jun 09 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.

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

#include "NavOpenDialog.h"
#include "NavUtils.h"
#include "MLTEShowcaseDefines.h"

CNavOpenDialog::CNavOpenDialog()
{
}

CNavOpenDialog::~CNavOpenDialog()
{
    this->ReleaseNavDialogRef();
    ::DisposeNavEventUPP( fEventHandler );
	
	// we're not using the popup menu at this time so it should always be NULL
	if( fPopuMenuItemNamesCFArray != NULL )
		::CFRelease(fPopuMenuItemNamesCFArray);
}

OSStatus
CNavOpenDialog::Init()
{
    OSStatus status = paramErr;
    fNavDialogRef = NULL;
	fEventHandler = NULL;
	fPreviewer = NULL;
	fObjectFilter = NULL;

	// we're not using the popup menu at this time so it should always be NULL
	fPopuMenuItemNamesCFArray = NULL;
	
    status = ::NavGetDefaultDialogCreationOptions( &fNavDialogOptions );
	
	require_noerr( status, FAIL );
    fNavDialogOptions.modality = kWindowModalityAppModal;
    fNavDialogOptions.optionFlags = kNavAllFilesInPopup
                                    | kNavAllowInvisibleFiles
                                    | kNavSupportPackages;
	
	status = CreateGetFileDialog( CNavOpenDialog::NavModernEventProc );
	
	FAIL:
    return status;
}

OSStatus
CNavOpenDialog::ReInit()
{
	if( fNavDialogRef != NULL )
	{
		NavDialogDispose( fNavDialogRef );
		fNavDialogRef = NULL;
	}
	return CreateGetFileDialog( CNavOpenDialog::NavModernEventProc );
}

OSStatus
CNavOpenDialog::CreateGetFileDialog( NavEventProcPtr eventHandler )
{
    OSStatus status = paramErr;
	
	require( fNavDialogRef == NULL, FAIL );
	
    if( fEventHandler != NULL )
	{
		DisposeNavEventUPP( fEventHandler );
		fEventHandler = NULL;
	}
	if( eventHandler != NULL )
		fEventHandler = ::NewNavEventUPP( eventHandler );
	
    if( fEventHandler != NULL )
    {
		
		// assumption: fPreviewer, and fObjectFilter are either NULL or correctly setup
		// before this function was called.
		
        status = ::NavCreateGetFileDialog( &fNavDialogOptions,
                                            NULL /*typeList*/, // the old way to filter files
                                            fEventHandler /*event callback*/,
                                            fPreviewer/*preview callback*/,
                                            fObjectFilter/*filter callback*/,  // the OSX way to filter files
                                            this/*userData*/,
                                            &fNavDialogRef/*dialog ref*/ );
    }
	FAIL:
	return status;
}

OSStatus
CNavOpenDialog::Run()
{
	OSStatus status = paramErr;
    if( fNavDialogRef != NULL )
        status = NavDialogRun( fNavDialogRef );
    return status;
}
	
pascal
void
CNavOpenDialog::NavModernEventProc( NavEventCallbackMessage msg, NavCBRecPtr params, void* userData )
{
    OSStatus status = noErr;
	CNavOpenDialog* openDialog = (CNavOpenDialog*)userData;
    switch( msg )
    {
        case kNavCBStart:
			// any initial set up like custom control state is set here
            break;
        case kNavCBCustomize:
			// add custom controls to the Nav dialog here
            break;
        case kNavCBUserAction:
            {
                NavReplyRecord reply;
                NavUserAction userAction = 0;
                status = NavDialogGetReply( params->context, &reply );
                if( status == noErr )
                {
                    userAction = NavDialogGetUserAction( params->context );
                    switch( userAction )
                    {
                        case kNavUserActionOpen:
						{
                            if( userData != NULL )
								openDialog->OpenFile( reply );
						}
						break;
    
                    }
                    status = NavDisposeReply( &reply );
                }
                break;
            }
        case kNavCBPopupMenuSelect:
            {
                NavMenuItemSpecPtr pNavMenuItem = NULL;
                pNavMenuItem = (NavMenuItemSpec *)params->eventData.eventDataParms.param;
                if (pNavMenuItem!= NULL)
                    openDialog->SetCurrentFormatMenuSelection( pNavMenuItem->menuType );
            }
            break;
        case kNavCBTerminate:
            {
				// We must make a new NavDialogRef on every run of the NavDialog
				// so dispose of the current one on terminate
				openDialog->ReleaseNavDialogRef();
            }
            break;
    }
}

void
CNavOpenDialog::SetOpenCallback( OpenCallbackPtr callback, void* userData )
{
	fCallbackUserData = userData;
	fOpenCallback = callback;
}

OSStatus
CNavOpenDialog::OpenFile( const NavReplyRecord& reply )
{
    OSStatus status = paramErr;
    CFURLRef gotURL = NULL;
	
    status = GetCFURLOutOfNavReply( reply, &gotURL );
	
    if( status == noErr && gotURL != NULL 
		&& fOpenCallback != NULL && fCallbackUserData != NULL )
    {
		fOpenCallback( fCallbackUserData, gotURL, 0 /*type*/ );
		CFRelease( gotURL );
    }
    return status;
}

void
CNavOpenDialog::SetCurrentFormatMenuSelection( UInt32 index )
{
	fMenuIndexOfFormatMenu = index;
}

void
CNavOpenDialog::InitPopUpMenuExtensionCFArray()
{
	static const CFStringRef menuItemNames[] = { CFSTR("MLTE Format"),
									CFSTR( "Rich Text Format" ),
									CFSTR( "Plain Unicode Text"),
									CFSTR( "Plain Text Format") };
	if( fPopuMenuItemNamesCFArray == NULL )
	{
		fPopuMenuItemNamesCFArray = CFArrayCreateMutable( NULL, sizeof( menuItemNames ), NULL );
		
		for( int i = 0; i < sizeof( menuItemNames ) / sizeof( CFStringRef ); i++ )
		{
			CFArrayAppendValue( fPopuMenuItemNamesCFArray, menuItemNames[i] );
		}
	}
}

void
CNavOpenDialog::ReleaseNavDialogRef()
{
    if( fNavDialogRef != NULL )
        NavDialogDispose( fNavDialogRef );
    fNavDialogRef = NULL;
}

// This function is not used at this time, but it gives an example of how we might
// filter files for display in the open dialog, depending on the file name extension.
// We might use this function in the filter callback to Navigation Services.
Boolean CNavOpenDialog::SupportFileIdentifiedByDotExtension( CFStringRef fileName )
{		
	static const CFStringRef kFilenameExtensions[] =
		{ CFSTR(".rtf"), CFSTR(".htm"), CFSTR(".html"), CFSTR(".txt"), CFSTR(".text"),
		  CFSTR(".sh"), CFSTR(".conf"), CFSTR(".ucs"), CFSTR(".utxt"), CFSTR(".utext"),
		  CFSTR(".uni"), CFSTR(".unicode"), CFSTR(".plist"), CFSTR(".php"), CFSTR(".c"),
		  CFSTR(".config"), CFSTR(".h"), CFSTR(".cp"), CFSTR(".cpp"), CFSTR(".perl"),
		  CFSTR(".py"), CFSTR(".hpp"), CFSTR(".tpp"), CFSTR(".i"), CFSTR(".rc"), CFSTR(".make"),
		  CFSTR(".apaci"), CFSTR(".r"),CFSTR(".rsrc"), CFSTR(".pp"), CFSTR(".p"), CFSTR(".script"),
		  CFSTR(".as"), CFSTR(".xml"), CFSTR(".xsl")//may want to weed out extensions that are a superset for example ".htm" is in ".html"
		};
    static const UInt16 kFilenameExtensionCount = sizeof( kFilenameExtensions ) / sizeof( CFStringRef );
    
    Boolean returnSupport = false;
						
    // check all supported filetypes
    for ( int i = 0; i < kFilenameExtensionCount; i++ )
    {
        if ( CFStringHasSuffix(fileName, kFilenameExtensions[i]) )
        {
            returnSupport = true;
            break;	// we support it so don't bother comparing against the remaining extensions
        }
    }
    return returnSupport;
}
