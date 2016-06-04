/*
 *  NavSaveDialog.cpp
 *  MLTEShowcase
 *
 *  Created on 6/9/04.
 *  Copyright 2004 Apple Computer, Inc. All rights reserved.

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

#include "NavSaveDialog.h"
#include "NavUtils.h"
#include "MLTEShowcaseDefines.h"

// no initialization in constructor
// init in separate init function instead
CNavSaveDialog::CNavSaveDialog()
{
}

// destructor - clean up allocated memory
CNavSaveDialog::~CNavSaveDialog()
{
    if ( fNavDialogOptions.saveFileName != NULL )
    {
        ::CFRelease( fNavDialogOptions.saveFileName );
        fNavDialogOptions.saveFileName = NULL;
    }
    if ( fNavDialogOptions.clientName != NULL )
    {
        ::CFRelease( fNavDialogOptions.clientName );
        fNavDialogOptions.clientName = NULL;
    }
	
    this->ReleaseNavDialogRef();
	
	if( fEventHandler != NULL )
	{
		DisposeNavEventUPP( fEventHandler );
		fEventHandler = NULL;
	}
	if( fPopUpMenuItemNamesCFArray != NULL )
	{
		CFRelease( fPopUpMenuItemNamesCFArray );
		fPopUpMenuItemNamesCFArray = NULL;
	}
	if( fFileExtensionNamesCFArray != NULL )
	{
		CFRelease( fFileExtensionNamesCFArray );
		fFileExtensionNamesCFArray = NULL;
	}
}

// Init routine will prepare our Nav Dialog Ref for running
OSStatus
CNavSaveDialog::Init( WindowRef parentWindow )
{
    OSStatus status = paramErr;
	fDocumentWindowRef = parentWindow; // parent window is where the sheet will be attached
    fNavDialogRef = NULL;
	fEventHandler = NULL;
	fPopUpMenuItemNamesCFArray = NULL;
	fFileExtensionNamesCFArray = NULL;
	
	fMenuIndexOfFormatMenu = (UInt32)-1;
	
	require( parentWindow != NULL, FAIL );
	
	// get the default options, and then modify them below to suit our needs
    status = ::NavGetDefaultDialogCreationOptions( &fNavDialogOptions );
	
	require_noerr( status, FAIL );
	
	fNavDialogOptions.saveFileName = NULL;
	fNavDialogOptions.clientName = kAppNameCFStr;
	
    fNavDialogOptions.parentWindow = fDocumentWindowRef;
    fNavDialogOptions.modality = kWindowModalityWindowModal; // a sheet on OSX
	
	fNavDialogOptions.optionFlags |= kNavSupportPackages;
	
	this->InitPopUpMenuExtensionCFArray();
	
	status = this->CreatePutFileDialog( CNavSaveDialog::NavModernEventProc );
	
	FAIL:
    return status;
}

// after every run of a Navigation Services dialog,
// we must dipose of the old NavDialogRef and make a new one
// We also need a new window to display the sheet on
// (because it might be a different window from the last time)
OSStatus
CNavSaveDialog::ReInit( WindowRef parentWindow )
{
	if( fNavDialogRef != NULL )
	{
		NavDialogDispose( fNavDialogRef );
		fNavDialogRef = NULL;
	}
	fDocumentWindowRef = parentWindow;
	// on ReInit, we will reuse the old event handler proc
	return this->CreatePutFileDialog( NULL /*eventHandlerProc*/ );
}

OSStatus
CNavSaveDialog::CreatePutFileDialog( NavEventProcPtr eventHandler )
{
    OSStatus status = paramErr;
	static const CFStringRef extensionCFStr = CFSTR( ".mlte" );
	CFStringRef windowTitleCFStr;
	CFMutableStringRef defaultFileNameCFStr = CFStringCreateMutable( NULL /*alloc*/, 255 /*maxLen*/ );
	
	require( fNavDialogRef == NULL && fDocumentWindowRef != NULL, FAIL );
	
    if( fEventHandler == NULL && eventHandler != NULL )
		fEventHandler = ::NewNavEventUPP( eventHandler );
	
    if( fEventHandler != NULL )
    {
		// the popupExtension is the menu used to specify the file save format
		// it should have been created at initialization time for this object
		fNavDialogOptions.popupExtension = fPopUpMenuItemNamesCFArray;
		
		// we reuse the fNavDialogOptions data, so we need to update
		// the saveFileName based on the current parent window's name
		// every time we pass through here
		if( fNavDialogOptions.saveFileName != NULL )
		{
			CFRelease( fNavDialogOptions.saveFileName );
			fNavDialogOptions.saveFileName = NULL;
		}

		status = CopyWindowTitleAsCFString( fDocumentWindowRef, &windowTitleCFStr );
		if( windowTitleCFStr != NULL )
		{
			CFStringAppend( defaultFileNameCFStr, windowTitleCFStr );
			CFRelease( windowTitleCFStr );
			CFStringAppend( defaultFileNameCFStr, extensionCFStr );
		}
		
		status = ::NavCreatePutFileDialog( &fNavDialogOptions,
											kFileType,
											// don't use kFileCreator for the signature below
											// or you'll get an extra menu item in popup extension
											// with the application name
											kNavGenericSignature,
											fEventHandler,
											(void*)this,
											&fNavDialogRef );
		status = NavDialogSetSaveFileName ( fNavDialogRef, defaultFileNameCFStr );
		// see 
		// http://lists.apple.com/archives/nav-serv-developers/2003/Apr/08/navcreateputfiledialogan.txt
		// for more info
    }
	FAIL:
	return status;
}

// The Navigation Services dialog only appears when Run is called
OSStatus
CNavSaveDialog::Run()
{
	OSStatus status = paramErr;
    if( fNavDialogRef != NULL )
        status = NavDialogRun( fNavDialogRef );
    return status;
}

// The evemt handler callback for the Navigation Services dialog when it is active
pascal
void
CNavSaveDialog::NavModernEventProc( NavEventCallbackMessage callBackSelector,
                                     NavCBRecPtr callBackParms, void* userData )
{
    CNavSaveDialog* saveDialog = (CNavSaveDialog*)userData; // the CNavSaveDialog object is passed as userData
    OSStatus status = noErr;
    NavReplyRecord reply;
    
    require( saveDialog != NULL, FAIL );
    
    switch( callBackSelector )
    {
        case kNavCBPopupMenuSelect: // value of Show/Format popup menu changed
        {
            NavMenuItemSpecPtr pNavMenuItem = NULL;
            pNavMenuItem = (NavMenuItemSpec *)callBackParms->eventData.eventDataParms.param;
            if (pNavMenuItem!= NULL)
			{
                saveDialog->SetCurrentFormatMenuSelection( pNavMenuItem->menuType );
			}
        }
            break;
        case kNavCBCancel: // user cancelled the dialog
            break;
        case kNavCBUserAction:
        // User has taken one of the actions described in
        // NavUserAction definition. May or may not dismiss the dialog
        {
            status = ::NavDialogGetReply( callBackParms->context, &reply );
            require_noerr( status, FAIL );
            switch( ::NavDialogGetUserAction( callBackParms->context ) /*userAction*/ )
            {
                case kNavUserActionCancel: // user cancelled dialog
                    break;
                case kNavUserActionSaveChanges: // user clicked Save button
                    break;
                case kNavUserActionDiscardChanges: // user clicked the Discard button in AskDiscardChanges
                    break;
                case kNavUserActionDontSaveChanges: // user clicked Don't Save button in AskSaveChanges
                    break;
                case kNavUserActionDiscardDocuments: // user clicked Discard Changes button in AskReviewDocuments
                    break;
                case kNavUserActionSaveAs: // user clicked the Save button
                {
                    require( saveDialog != NULL, FAIL );
                    status = saveDialog->SaveAs( reply );
				}
                    break;
            }
            status = ::NavDisposeReply( &reply );
        }
            break;
        case kNavCBTerminate: // last message sent, after dlog invisible
        {
            saveDialog->ReleaseNavDialogRef();
        }
            break;
    }
    FAIL:
    return;
}

void
CNavSaveDialog::SetSaveCallback( NewSaveCallbackPtr callback, void* userData )
{
	fCallbackUserData = userData;
	fSaveCallback = callback;
}

OSType
CNavSaveDialog::GetSelectedFileType()
{
	// "MLTE Format"
	// "Rich Text Format (RTF)"
	// "Plain Unicode Format"
	// "Plain Text Format"
	
	OSType selectedType = 0;
	
    switch( fMenuIndexOfFormatMenu )
	{
		case 0:
			selectedType = kTXNTextensionFile;
			break;
		case 1:
			selectedType = 'RTF ';
			break;
		case 2:
			selectedType = kTXNUnicodeTextFile;
			break;
		case 3:
			selectedType = kTXNTextFile;
			break;
		default:
			selectedType = 0;
			break;
	}
	return selectedType;
}

OSStatus
CNavSaveDialog::SaveAs( const NavReplyRecord& navReply )
{
    OSStatus status = paramErr;
	CFURLRef gotURLRef = NULL;
	CFURLRef fullURLRef = NULL;
    HFSUniStr255 gotHFSSaveFileName;
	CFStringRef fileNameCFStr = NULL;
	
    require( navReply.validRecord == true, FAIL );
	require( fSaveCallback != NULL && fCallbackUserData != NULL, FAIL );

    /*
    struct NavReplyRecord {
        UInt16 version; 
        Boolean validRecord; 
        Boolean replacing; 
        Boolean isStationery; 
        Boolean translationNeeded; 
        AEDescList selection; 
        ScriptCode keyScript; 
        FileTranslationSpecArrayHandle fileTranslation; 
        UInt32 reserved1; 
        char reserved[231]; 
        char reserved[227]; 
        UInt8 reserved2; 
        char reserved[225];
    };
    */

	// get a URL to the parent directory to save the file in
    status = GetCFURLOutOfNavReply( navReply, &gotURLRef );
	
	// get name of target file to save
	gotHFSSaveFileName.length = 0;
	GetHFSUniStrSaveFileName( fNavDialogRef, gotHFSSaveFileName );
	
	// create a full URL including parent directory and target file name
	fileNameCFStr = CFStringCreateWithCharacters( NULL /*alloc*/, gotHFSSaveFileName.unicode, gotHFSSaveFileName.length );
	fullURLRef = CFURLCreateCopyAppendingPathComponent(NULL /*allocator*/, gotURLRef, fileNameCFStr, true);
	
	// we pass our Navigation Services gathered info back to the application via a custom callback
    if( status == noErr && fullURLRef != NULL )
    {
		fSaveCallback( fCallbackUserData, fullURLRef, this->GetSelectedFileType(), navReply.replacing );
    }
	
	// clean up
	if( fileNameCFStr  != NULL )
		CFRelease( fileNameCFStr );
	if( fullURLRef  != NULL )
		CFRelease( fullURLRef );
	if( gotURLRef  != NULL )
		CFRelease( gotURLRef );
    
    FAIL:
    return status;
}

// This function is called when the user makes a menu selection in the format menu of the
// Navigation Services dialog.  We will update some data based on the user selection.
void
CNavSaveDialog::SetCurrentFormatMenuSelection( UInt32 index )
{
	CFIndex fileExtensionCount = CFArrayGetCount( fFileExtensionNamesCFArray );
	CFIndex oldExtLen = 0;
	
	// update the save filename displayed in the Navigation Services dialog...
	
	// get a reference to the current name
	CFStringRef saveName = NavDialogGetSaveFileName( fNavDialogRef );
	CFIndex len = CFStringGetLength( saveName );
	
	// make a copy that we can change
	CFMutableStringRef newSaveName = CFStringCreateMutableCopy( NULL /*alloc*/, 255 /*maxLen*/, saveName );
	
	// get an appropriate filename extension based on the user's menu choice
	const CFStringRef newExtName = (const CFStringRef)CFArrayGetValueAtIndex( fFileExtensionNamesCFArray, index );
	
	// find what if any was the old filename extension, and get the len of it so it can be deleted
	// we only check for and replace known filename extensions that we support
	for( CFIndex i = 0; i < fileExtensionCount; i++ )
	{
		const CFStringRef oldExt = (const CFStringRef)CFArrayGetValueAtIndex( fFileExtensionNamesCFArray, i );
		if( CFStringHasSuffix( saveName, oldExt ) )
		{
			 oldExtLen = CFStringGetLength( oldExt );
			 break;
		}
	}
	// replace the old filename extension with the new one.  If no suported old filename extension was
	// found, we just append the new one.
	CFStringReplace( newSaveName, CFRangeMake( len - oldExtLen, oldExtLen ), newExtName );
	NavDialogSetSaveFileName ( fNavDialogRef, newSaveName );
	// update index into format menu
	fMenuIndexOfFormatMenu = index;
}

void
CNavSaveDialog::InitPopUpMenuExtensionCFArray()
{
	static const CFStringRef menuItemNames[] = { CFSTR("MLTE Format"),
									CFSTR( "Rich Text Format (RTF)" ),
									CFSTR( "Plain Unicode Format"),
									CFSTR( "Plain Text Format") };

	static const CFStringRef fileExtensionNames[] = { CFSTR(".mlte"),
									CFSTR( ".rtf" ),
									CFSTR( ".utxt"),
									CFSTR( ".txt") };
									
	if( fPopUpMenuItemNamesCFArray == NULL ) // lazy init
	{
		fPopUpMenuItemNamesCFArray = ::CFArrayCreateMutable( NULL, sizeof( menuItemNames ), NULL );	
		for( int i = 0; i < sizeof( menuItemNames ) / sizeof( CFStringRef ); i++ )
		{
			CFArrayAppendValue( fPopUpMenuItemNamesCFArray, menuItemNames[i] );
		}
	}
	if( fFileExtensionNamesCFArray == NULL ) // lazy init
	{
		fFileExtensionNamesCFArray = ::CFArrayCreateMutable( NULL, sizeof( fileExtensionNames ), NULL );	
		for( int i = 0; i < sizeof( fileExtensionNames ) / sizeof( CFStringRef ); i++ )
		{
			CFArrayAppendValue( fFileExtensionNamesCFArray, fileExtensionNames[i] );
		}
	}
}

void
CNavSaveDialog::ReleaseNavDialogRef()
{
    if( fNavDialogRef != NULL )
        NavDialogDispose( fNavDialogRef );
    fNavDialogRef = NULL;
}
