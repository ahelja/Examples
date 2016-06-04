/*
 *  HITextView.cpp
 *  MLTEShowcase
 *
 *  Created on Mon Apr 26 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 *	This file contains code to manage the HITextView in MLTEShowcase
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
#include "TextView.h"
#include "HelpTextFrame.h"
#include "MLTEShowcaseUtils.h"
#include "NavOpenDialog.h"
#include "NavSaveDialog.h"


static
OSStatus
TextViewAddActionNameMapper( HIViewRef textView );

// add some default text to the HITextView
static
OSStatus
AddTextToTheTextView( HIViewRef textView );

// add an HIImageViewRef to the parentView
static
HIViewRef
AddImage( HIViewRef parentView );

// util function tells the HelpTextFrame to display a helpful message
static
OSStatus
SignalHelpMessage( WindowRef window, CFStringRef taskCFStr );

OSStatus
TextViewSetBGAlpha( HIViewRef textView, float alpha );

// pointer to a custom Navigation Services object for opening files
static
CNavOpenDialog* gNewFileOpener = NULL;

// pointer to a custom Navigation Services object for saving files
static
CNavSaveDialog* gNewFileSaver = NULL;

// MLTE callbacks
static
TXNContextualMenuSetupUPP gContextualMenuUPP = NULL;

static
TXNActionNameMapperUPP gActionNameMapperUPP = NULL;

static
OSStatus
TextViewUpdateMenuRef( HIViewRef textView, MenuRef  menuRef );

#pragma mark --- CUSTOM MLTE INSTANCE DATA FUNCTIONS ---

CMLTEViewData::CMLTEViewData()
{
	fFileURL = NULL;
}

CMLTEViewData::~CMLTEViewData()
{
	if( fFileURL != NULL )
		CFRelease( fFileURL );
}

void
CMLTEViewData::SetURL( CFURLRef url )
{
	if( fFileURL != NULL )
		CFRelease( fFileURL );
		
	fFileURL = url;
	
	if( fFileURL != NULL )
		CFRetain( url );
}

OSStatus
TextViewStoreMLTEInstanceData( HIViewRef textView, CMLTEViewData* mlteData )
{
	return SetControlProperty( textView, 'mDem', 'mDat', sizeof(CMLTEViewData*), mlteData );
}

CMLTEViewData*
TextViewRetrieveMLTEInstanceData( HIViewRef textView )
{
	CMLTEViewData* mlteData = NULL;
	if( GetControlProperty( textView, 'mDem', 'mDat', sizeof(CMLTEViewData*), NULL, (UInt32*)&mlteData ) == noErr )
		return mlteData;
	return NULL;
}

#pragma mark --- FILE I/O FUNCTIONS ---

// custom callbacks to work with this application's navigation services code
static OSStatus
OpenFileCallback( void* target, CFURLRef url, OSType type /*unused*/ )
{
	// The file type is not used because we don't have code to identify what type of file
	// was picked in the Navigation Services dialog.  If a client of MLTE has the filetype
	// info availalbe, they should definitely pass it on to the TXNReadFromCFURL API
	// as this will improve performance and reliable identification of the file.
	return TextViewReadCFURL( (HIViewRef)target, url );
}

static OSStatus
WriteFileCallback( void* target, CFURLRef url, OSType type, Boolean replacing )
{
	return TextViewWriteToCFURL( (HIViewRef)target, url, type, replacing );
}

OSStatus
TextViewWriteToCFURL( HIViewRef textView, CFURLRef fileURL, OSType type, Boolean replacing )
{
	static const CFStringRef taskCFStr = CFSTR("TaskWriteCFURL");
	static const CFStringRef taskErrorCFStr = CFSTR("TaskWriteCFURLERROR");
	OSStatus status = paramErr;	
	FSRef createdFSRef;
	CFNumberRef encodingNumber = NULL;
	
	// create a file based on the URL provided
	status = CreateFileWithCFURL( fileURL, createdFSRef, type, replacing );

	// specify data type info
	CFStringRef dataKeys[2];   // this array of keys only contains CFStringRefs
	
	const void* dataValues[2]; // this array contains a CFStringRef and an CFNumber
							   // so we will use void* for the array type
	
	dataKeys[0] = kTXNDataOptionDocumentTypeKey;
	
	// set an appropriate document type and encoding
	// we are still using the old Textension file type constants for passing
	// the file type selection from Navigation Services, but please note that
	// these constants are being deprecated in Tiger.
	switch( type )
	{
		case 'RTF ':
			dataValues[0] = kTXNRTFDocumentType;
			encodingNumber = UtilCreateEncodingCFNumberRef( kCFStringEncodingUnicode );
			break;
		case kTXNUnicodeTextFile: // old MLTE Unicode file constant
			dataValues[0] = kTXNPlainTextDocumentType;
			encodingNumber = UtilCreateEncodingCFNumberRef( kCFStringEncodingUnicode );
			break;
		case 'text':
		case kTXNTextFile: // old MLTE text file constant
			dataValues[0] = kTXNPlainTextDocumentType;
			encodingNumber = UtilCreateEncodingCFNumberRef( kCFStringEncodingMacRoman );
			break;
		case kTXNTextensionFile: // old MLTE Textension file format file constant
		default:
			dataValues[0] = kTXNMLTEDocumentType;
			encodingNumber = UtilCreateEncodingCFNumberRef( kCFStringEncodingUnicode );
			break;
	}
	
	dataKeys[1] =	kTXNDataOptionCharacterEncodingKey;
	dataValues[1] = (void*)encodingNumber;
	
	CFDictionaryRef iDataOptionsDict = CFDictionaryCreate( NULL /*allocator*/,
														   (const void**)dataKeys,		
														   (const void**)dataValues, 
														   sizeof(dataKeys) / sizeof(CFStringRef), /*itemCount*/
														   &kCFCopyStringDictionaryKeyCallBacks, 
														   &(kCFTypeDictionaryValueCallBacks));
														   
	// Now make some meta data
	CFStringRef docKeys[6];
	CFStringRef docValues[6];
	
	docKeys[0] = kTXNDocumentAttributeTitleKey;
	docValues[0] = CFSTR("Example File");
	
	docKeys[1] = kTXNDocumentAttributeCompanyNameKey;
	docValues[1] = CFSTR("__MyCompanyName__");
	
	docKeys[2] = kTXNDocumentAttributeSubjectKey;
	docValues[2] = CFSTR("Example of saving a file");
	
	docKeys[3] = kTXNDocumentAttributeAuthorKey;
	docValues[3] = CFSTR("Me");
	
	docKeys[4] = kTXNDocumentAttributeKeywordsKey;
	docValues[4] = CFSTR("Textension, MLTE, File, Save, CFURL");
	
	docKeys[5] = kTXNDocumentAttributeCommentKey;
	docValues[5] = CFSTR("This is just a demonstration.");
	
	CFDictionaryRef iDocAttributesDict = CFDictionaryCreate( NULL							 /*allocator*/,
														    (const void**)docKeys,		
														    (const void**)docValues, 
										  					sizeof(docKeys) / sizeof(CFStringRef), /*itemCount*/
										  					&kCFCopyStringDictionaryKeyCallBacks, 
										  					&(kCFTypeDictionaryValueCallBacks));
	

	// console output
	WarnString( "Call TXNWriteRangeToCFURL with url: ");
	CFShow( fileURL );
	
	status = TXNWriteRangeToCFURL( HITextViewGetTXNObject(textView),
									kTXNStartOffset,
									kTXNEndOffset,
									iDataOptionsDict,
									iDocAttributesDict,
									fileURL );

	// clean up all the CFTypes we made
	if( iDataOptionsDict != NULL )
		CFRelease( iDataOptionsDict );
	if( iDocAttributesDict != NULL )
		CFRelease( iDocAttributesDict );
	
	if( status == noErr )
		SignalHelpMessage( HIViewGetWindow( textView ), taskCFStr );
	else
	{
		// If we fail to write a file, we'll leave the bad / broken file we created
		// to save the data into -- as evidence of the error.
		SignalHelpMessage( HIViewGetWindow( textView ), taskErrorCFStr );
	}
	
	return status;
}

OSStatus
TextViewReadCFURL( HIViewRef textView, CFURLRef cfURL )
{
	OSStatus status = paramErr;
	CFDictionaryRef dataOptions = NULL;
	static const CFStringRef taskCFStr = CFSTR("TaskReadCFURL");
	static const CFStringRef taskErrorCFStr = CFSTR("TaskReadCFURLError");

	status = TXNReadFromCFURL( HITextViewGetTXNObject(textView),
							kTXNStartOffset, kTXNEndOffset,
							dataOptions,
							cfURL,
							NULL /*docAttributes*/ );
	if( status != noErr )
	{
		WarnStatusString( "TXNReadFromCFURL returned: %lu ", status );
		SignalHelpMessage( HIViewGetWindow( textView ), taskErrorCFStr );
	}
	else
	{
		CMLTEViewData* mlteData = NULL;
		SignalHelpMessage( HIViewGetWindow( textView ), taskCFStr );
		HIViewSetNeedsDisplay( textView, true );
	
		// in some scenarios, it is useful to remember where we got the file
		// data from at a later time, so save the URL in our custom instance data object
		mlteData = TextViewRetrieveMLTEInstanceData( textView );
		if( mlteData != NULL )
			mlteData->SetURL( cfURL );
	}
	
	return status;
}

// Each time a new window is first opened, call this one time function to set up
// the text view
OSStatus
SetUpTheTextView( WindowRef window )
{
	OSStatus status = noErr;
	HIViewRef textView = NULL;
	HIViewRef scrollView = NULL;
	HIViewRef scrollParentView = NULL;
	
	CMLTEViewData* mlteData = NULL;
	
	// Get the HITextView from the window
	status = GetTextViewFromWindow( window, textView );
	
	require_action( textView != NULL, EXIT, status = paramErr );
	
	// make a new custom C++ object to hold MLTE related data
	mlteData = new CMLTEViewData();
	
	// DON'T FORGET TO DISPOSE THIS WHEN HITextView destructs!!!
	
	// put the custom object in the HITextView for this window
	// as a control property so that we can retrieve it later when we need it.
	status = TextViewStoreMLTEInstanceData( textView, mlteData );
	require_action( textView != NULL, EXIT, status = paramErr );
	
	// Now set the text view as we like it
	status = TextViewDefaultSettings( textView );
	check_noerr( status );
	status = TextViewAddActionNameMapper( textView );
	check_noerr( status );
	status = TextViewSetMargins( textView, 0 /*top*/, 0 /*left*/, 0 /*right*/ );
	check_noerr( status );
	
	// get fancy - try to add a picture behind the textView
	scrollView = HIViewGetSuperview( textView );
	scrollParentView = HIViewGetSuperview( scrollView );
	
	if( scrollParentView != NULL )
	{
		HIViewRef imageViewRef = AddImage( scrollParentView );
		if( imageViewRef != NULL )
		{
			HIRect scrollFrame;
			
			HIViewGetFrame( scrollView, &scrollFrame );
			HIViewSetFrame( imageViewRef, &scrollFrame );
			
			status = HIViewSetZOrder( scrollView, kHIViewZOrderAbove, imageViewRef );
			check_noerr( status );
			HIViewSetVisible(imageViewRef, true);
			
			status = TextViewSetBGAlpha( textView, 0.75 );
			check_noerr( status );
  
		}
	}
	
	// register for menu handing
	status = TextViewInstallMenuHandlers( textView );
  
	EXIT:
	return status;
}

OSStatus
TextViewDefaultSettings( HIViewRef textView )
{
	OSStatus status = paramErr;
	static const CFStringRef taskCFStr = CFSTR("TaskDefault");
	WindowRef window;
	
	status = AddTextToTheTextView( textView );
	check_noerr( status );
	status = TextViewEditCommandSupport( textView, true );
	check_noerr( status );
	status = TextViewFontPanelSupport( textView, false );
	check_noerr( status );
	status = TextViewSpellingSupport( textView, false );
	check_noerr( status );
	status = TextViewScrollingOptions( textView, (UInt32)kTXNAutoScrollInsertionIntoView );
	check_noerr( status );
	status = TXNSetSelection(HITextViewGetTXNObject(textView), kTXNStartOffset, kTXNStartOffset);
	check_noerr( status );
	status = HIScrollViewNavigate(HIViewGetSuperview(textView),kHIScrollViewScrollToTop) ;
	check_noerr( status );

	status = SignalHelpMessage( HIViewGetWindow( textView ), taskCFStr );

	return status;
}

OSStatus
GetTextViewFromWindow( WindowRef window, HIViewRef& textView )
{
	OSStatus status = paramErr;
	if( window != NULL )
	{
		status = HIViewFindByID(HIViewGetRoot(window), kTextViewControlID, &textView);
		check_noerr( status );
	}
	return status;
}

OSStatus
TextViewFocusInWindow( WindowRef window )
{
	OSStatus status = paramErr;
	HIViewRef textView = NULL;
	HIViewRef scrollView = NULL;
	
	// Get the HITextView from the window
	status = GetTextViewFromWindow( window, textView );
	
	require_action( textView != NULL, EXIT, status = paramErr );
	scrollView = HIViewGetSuperview( textView );
	require_action( scrollView != NULL, EXIT, status = paramErr );
	
	HIViewSetFirstSubViewFocus( HIViewGetRoot(window), textView );
	status = HIViewSetNextFocus( scrollView, NULL );
	status = HIViewAdvanceFocus ( scrollView, 0 /*modifiers*/ );

	EXIT:
	return status;
}

#pragma mark --- EVENT HANDLER FUNCTIONS ---

OSStatus
TextViewInstallMenuHandlers( HIViewRef textView )
{
	EventTargetRef targetRef = GetControlEventTarget( textView );
    static const EventTypeSpec sAppEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess },
        { kEventClassCommand, kEventCommandUpdateStatus }	
    };

    OSStatus status = InstallEventHandler( targetRef, TextViewMenuEventHandler,
                                             GetEventTypeCount( sAppEvents ),
                                             sAppEvents, textView /*userData*/, NULL /*eventHandlerRef*/ );
    return status;
}

OSStatus
TextViewMenuEventHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;
    HICommand hiCommand;
	HIViewRef textView = (HIViewRef)userData;
	UInt32 eventKind;
    eventKind = GetEventKind( inEvent );

    switch ( GetEventClass( inEvent ) )
    {
        case kEventMenuOpening:
		{
			MenuRef menuRef;
			status = ::GetEventParameter( inEvent, kEventParamDirectObject, typeMenuRef,
										NULL, sizeof(typeMenuRef), NULL, &menuRef );
            status = TextViewUpdateMenuRef( textView, menuRef );
            break;
		}
        case kEventClassCommand:
        {
            status = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
                    NULL, sizeof( HICommand ), NULL, (void *) &hiCommand );
            require_action( status == noErr, HandleCommandEvent_err, status = eventNotHandledErr );

            if( eventKind == kEventProcessCommand 
                && ( hiCommand.attributes & kHICommandFromMenu ||  hiCommand.attributes & kHICommandFromControl ) == true )
			{
				status = TextViewProcessHICommand( textView, hiCommand );
			}
        }
    }
  
	HandleCommandEvent_err:
    return status;
}

OSStatus
TextViewProcessHICommand( HIViewRef textView, const HICommand& hiCommand )
{
	char buffer[255];
    OSStatus status = eventNotHandledErr;
	static const CFStringRef exampleCFStr = CFSTR("Example Action Group");
    switch( hiCommand.commandID )
    {
		case kToggleAutoSpellcheckCommand:
		{
			static const CFStringRef taskONCFStr = CFSTR("SpellcheckON");
			static const CFStringRef taskOFFCFStr = CFSTR("SpellcheckOFF");
			CFStringRef taskCFStr = NULL;
			
			Boolean spellState = TextViewToggleSpellCheckAsYouType( textView );
			
			taskCFStr = spellState?taskONCFStr:taskOFFCFStr;

			SignalHelpMessage( HIViewGetWindow( textView ), taskCFStr );
			status = noErr;
		}
			break;
			
		case kBeginUndoActionGroupCommand:
		{
			static const CFStringRef taskCFStr = CFSTR("ActionGroupBEGIN");
			SignalHelpMessage( HIViewGetWindow( textView ), taskCFStr );
			status = TextViewBeginActionGroup( textView, exampleCFStr );
		}
			break;
		case kEndUndoActionGroupCommand:
		{
			static const CFStringRef taskCFStr = CFSTR("ActionGroupEND");
			SignalHelpMessage( HIViewGetWindow( textView ), taskCFStr );
			status = TextViewEndActionGroup( textView );
		}
			break;
        default:
            ;
    }
    return status;
}

OSStatus
TextViewUpdateMenuRef( HIViewRef textView, MenuRef  menuRef )
{
	return noErr;
}

#pragma mark --- MLTE EDIT COMMAND CHECK SUPPORT ---

CFStringRef
MyActionNameMapperCallback(CFStringRef actionName, UInt32 commandID, void* userData )
{
	static const CFStringRef errorStr = CFSTR("Error!");
	static const CFStringRef kMyCustomActionString = CFSTR("MyActionGroup");

	CFOptionFlags compareOptions = 0;
	CFStringRef actionCFStr = NULL;

	// allocate buffer for string we will return
	CFMutableStringRef builtCFStr = CFStringCreateMutable( NULL /*allocator*/, 50 /*maxLen*/ );

	// is the actionName key one of my custom strings?
	if( CFStringCompare( actionName, kMyCustomActionString, compareOptions ) == kCFCompareEqualTo )
	{
		actionCFStr = CFSTR("Custom Action Group");
	}
	else
	{
		actionCFStr = CFCopyLocalizedString ( actionName, "Actions strings displayed in the edit menu" );
	}
	
	require_action( actionCFStr != NULL, FAIL, CFStringAppend( builtCFStr, errorStr) );
	
	if( commandID == kHICommandUndo )
	{
		CFStringAppend( builtCFStr, CFSTR("Undo ") );
		CFStringAppend( builtCFStr, actionCFStr );
	}
	else if( commandID = kHICommandRedo )
	{
		CFStringAppend( builtCFStr, CFSTR("Redo ") );
		CFStringAppend( builtCFStr, actionCFStr );
	}
	else
		CFStringAppend( builtCFStr, errorStr);
		
	FAIL:
	return builtCFStr;
};

OSStatus
TextViewAddActionNameMapper( HIViewRef textView )
{
	if( gActionNameMapperUPP == NULL )
		gActionNameMapperUPP = NewTXNActionNameMapperUPP( MyActionNameMapperCallback );
		
	return TXNSetActionNameMapper( HITextViewGetTXNObject(textView), gActionNameMapperUPP, NULL /*userData*/ );
}

OSStatus
TextViewEditCommandSupport( HIViewRef textView, Boolean on )
{
	OSStatus status = noErr;
	TXNCommandEventSupportOptions options = 0;
	TXNObject txnObj =  HITextViewGetTXNObject(textView);
	
	// got TXNObject?
	require( txnObj != NULL, EXIT );

	// get existing option settings...
	status = TXNGetCommandEventSupport( txnObj, &options );
	require_noerr( status, EXIT );
	
	// add or subtract edit command support as requested
	if( on )	
	{
		options |= kTXNSupportEditCommandProcessing;
		options |= kTXNSupportEditCommandUpdating;
	}
	else
	{
		if( options & kTXNSupportEditCommandProcessing )
			options ^= kTXNSupportEditCommandProcessing;
		if( options & kTXNSupportEditCommandUpdating )
			options ^= kTXNSupportEditCommandUpdating;
	}
	// reset modified options
	status = TXNSetCommandEventSupport(txnObj, options );
	verify_noerr( status );
	
	EXIT:
	;
	return status;
}

#pragma mark --- MLTE SPELL CHECK SUPPORT ---

OSStatus
TextViewSpellingSupport( HIViewRef textView, Boolean on )
{
	OSStatus status = noErr;
	TXNCommandEventSupportOptions options = 0;
	TXNObject txnObj =  HITextViewGetTXNObject(textView);
	
	// Got TXNObject?
	require( txnObj != NULL, EXIT );

	// Get existing option settings...
	status = TXNGetCommandEventSupport( txnObj, &options );
	require_noerr( status, EXIT );
	
	// add or subtract spelling support as requested
	// (and enable/disable menu items appropriately)
	if( on )	
	{
		options |= kTXNSupportSpellCheckCommandProcessing;
		options |= kTXNSupportSpellCheckCommandUpdating;
		EnableMenuCommand(NULL, kHICommandShowSpellingPanel);
		EnableMenuCommand(NULL, kToggleAutoSpellcheckCommand);
		verify_noerr( SignalHelpMessage( HIViewGetWindow( textView ), CFSTR("SpellSupportEnable") ));
	}
	else
	{
		if( options & kTXNSupportSpellCheckCommandProcessing )
			options ^= kTXNSupportSpellCheckCommandProcessing;
		if( options & kTXNSupportSpellCheckCommandUpdating )
			options ^= kTXNSupportSpellCheckCommandUpdating;
		DisableMenuCommand(NULL, kHICommandShowSpellingPanel);
		DisableMenuCommand(NULL, kToggleAutoSpellcheckCommand);
		verify_noerr( SignalHelpMessage( HIViewGetWindow( textView ), CFSTR("SpellSupportDisable") ));
	}

	// Set auto spell check state accordingly
	status = TextViewSpellCheckAsYouType(textView, on);
	verify_noerr( status );

	// reset modified options
	status = TXNSetCommandEventSupport(txnObj, options);
	verify_noerr( status );
	
	HIViewSetNeedsDisplay( textView, true );

	EXIT:
	;
	return status;
}

Boolean
TextViewIsSpellingSupportEnabled( HIViewRef textView )
{
	Boolean state = false;
	TXNCommandEventSupportOptions options = 0;
	TXNObject txnObj =  HITextViewGetTXNObject(textView);	

	verify_noerr( TXNGetCommandEventSupport( txnObj, &options ) );
	
	if ( (options & kTXNSupportSpellCheckCommandProcessing) &&
		 (options & kTXNSupportSpellCheckCommandUpdating) )
		state = true;
	
	return state;
}

void
MyContextualMenuCallback( MenuRef iContextualMenu, TXNObject object, void *inUserData)
{
	OSStatus status = noErr;
	MenuItemAttributes inAttributes = 0;
	MenuCommand commandID = kHICommandAbout; // we'll insert a menu that shows the "About box"
	MenuItemIndex appendIndex = 0;
	
	status = AppendMenuItemTextWithCFString( iContextualMenu, CFSTR("About MLTEShowcase"),
												inAttributes, /*MenuItemAttributes*/
												commandID, /*MenuCommand*/
												&appendIndex );
}

OSStatus
TextViewSetupSpellingContextualMenu( HIViewRef textView )
{
	if( gContextualMenuUPP == NULL )
		gContextualMenuUPP = NewTXNContextualMenuSetupUPP( MyContextualMenuCallback );

	return TXNSetContextualMenuSetup( HITextViewGetTXNObject(textView),
										gContextualMenuUPP,
										NULL /*userData*/ );
}

OSStatus
TextViewSpellCheckAsYouType( HIViewRef textView, Boolean enable )
{
	UniChar mark = ( enable ) ? kMenuCheckmarkGlyph : kMenuNullGlyph;
	verify_noerr( SetMenuCommandMark(NULL, kToggleAutoSpellcheckCommand, mark) );
	return TXNSetSpellCheckAsYouType( HITextViewGetTXNObject(textView), enable );
}

Boolean
TextViewToggleSpellCheckAsYouType( HIViewRef textView )
{
	Boolean spellCheckOn = TXNGetSpellCheckAsYouType(HITextViewGetTXNObject(textView));
	OSStatus status = TextViewSpellCheckAsYouType( textView, !spellCheckOn );
	
	if( status != noErr )
	{
		// console output
		char uMsg[255];
		sprintf( uMsg, "TextViewSpellCheckAsYouType returned %lu", status );
		WarnString( uMsg );
	}
	
	return !spellCheckOn;
}

#pragma mark --- MLTE FONT PANEL SUPPORT ---

OSStatus
TextViewFontPanelSupport( HIViewRef textView, Boolean on )
{
	OSStatus status = noErr;
	TXNCommandEventSupportOptions options = 0;
	TXNObject txnObj =  HITextViewGetTXNObject(textView);
	
	// Got TXNObject?
	require( txnObj != NULL, EXIT );

	// Get existing option settings...
	status = TXNGetCommandEventSupport( txnObj, &options );
	require_noerr( status, EXIT );
	
	// Add or subract font command support as requested,
	// (and enable/disable menu items appropriately)
	if( on )	
	{
		options |= kTXNSupportFontCommandProcessing;
		options |= kTXNSupportFontCommandUpdating;
		EnableMenuCommand(NULL, kHICommandShowHideFontPanel);
		if ( ! FPIsFontPanelVisible() )
			verify_noerr( FPShowHideFontPanel() );
		verify_noerr( SignalHelpMessage( HIViewGetWindow( textView ), CFSTR("FontPanelEnable") ));
	}
	else
	{
		if( options & kTXNSupportFontCommandProcessing )
			options ^= kTXNSupportFontCommandProcessing;
		if( options & kTXNSupportFontCommandUpdating )
			options ^= kTXNSupportFontCommandUpdating;
		DisableMenuCommand(NULL, kHICommandShowHideFontPanel);
		if ( FPIsFontPanelVisible() )
			verify_noerr( FPShowHideFontPanel() );
		verify_noerr( SignalHelpMessage( HIViewGetWindow( textView ), CFSTR("FontPanelDisable") ));
	}
	// reset modified options
	status = TXNSetCommandEventSupport(txnObj, options );
	verify_noerr( status );
	
	EXIT:
	;
	return status;
}

Boolean
TextViewIsFontPanelSupportEnabled( HIViewRef textView )
{
	Boolean state = false;
	TXNCommandEventSupportOptions options = 0;
	TXNObject txnObj =  HITextViewGetTXNObject(textView);	

	verify_noerr( TXNGetCommandEventSupport( txnObj, &options ) );
	
	if ( (options & kTXNSupportFontCommandProcessing) &&
		 (options & kTXNSupportFontCommandUpdating) )
		state = true;
	
	return state;
}

#pragma mark --- MLTE AUTO SCROLL BEHAVIOR ---

OSStatus
TextViewScrollingOptions( HIViewRef textView, UInt32 opts )
{
	// kAutoScrollInsertionIntoView  = 0,
	// kAutoScrollNever              = 1,
	// kAutoScrollWhenInsertionVisible = 2
	
	// One of these keys will be used to signal the HelpTextframe with a
	// useful message
	static const CFStringRef autoScrollCFStr = CFSTR("TaskAutoscroll");
	static const CFStringRef scrollNeverCFStr = CFSTR("TaskAutoscrollNever");
	static const CFStringRef scrollIfVisibileCFStr = CFSTR("TaskAutoscrollIfVisible");
	static const CFStringRef scrollUnknownCFStr = CFSTR("TaskAutoscrollUnknown");
	
	CFStringRef taskCFStr = NULL;
	
	switch( opts )
	{
		case kTXNAutoScrollInsertionIntoView:
			taskCFStr = autoScrollCFStr;
			break;
		case kTXNAutoScrollNever:
			taskCFStr = scrollNeverCFStr;
			break;
		case kTXNAutoScrollWhenInsertionVisible:
			taskCFStr = scrollIfVisibileCFStr;
			break;
		default:
			taskCFStr = scrollUnknownCFStr;
	}
	
	if( taskCFStr != NULL )
		SignalHelpMessage( HIViewGetWindow( textView ), taskCFStr );
			
	return TextViewSetObjectControlData( textView, kTXNAutoScrollBehaviorTag, kUnsigned, 0, opts );
}

#pragma mark --- MLTE ACTION GROUP SUPPORT ---

OSStatus
TextViewBeginActionGroup( HIViewRef textView, CFStringRef iActionGroupName )
{
	return TXNBeginActionGroup( HITextViewGetTXNObject(textView), iActionGroupName);
}

OSStatus
TextViewEndActionGroup( HIViewRef textView )
{
	return TXNEndActionGroup( HITextViewGetTXNObject(textView));
}				

#pragma mark --- MLTE API USAGE ---

OSStatus 				
TextViewGetObjectControlData( HIViewRef textView, TXNControlTag controlTag, Boolean isSigned, 
                                SInt32& oSignedData, UInt32& oUnsignedData )
{
    OSStatus	status = paramErr;
    TXNControlTag	oControlTags[1] = {controlTag};
    TXNControlData	oControlData[1];

    status = ::TXNGetTXNObjectControls(	HITextViewGetTXNObject(textView), 1, oControlTags, oControlData );
    if (status == noErr)
    {
        if( isSigned )
            oSignedData = oControlData[0].sValue;
        else
            oUnsignedData = oControlData[0].uValue;
    }                                 
    fail:
    return status;
}

OSStatus
TextViewSetObjectControlData( HIViewRef textView, TXNControlTag controlTag, Boolean isSigned, 
                                 SInt32 iSignedData, UInt32 iUnsignedData )
{
    OSStatus status = noErr;
    
	TXNControlTag iControlTags[1] = {controlTag};
	TXNControlData iControlData[1];
	if( isSigned )
		iControlData[0].sValue = (SInt32)iSignedData;
	else
		iControlData[0].uValue = (UInt32)iUnsignedData;

	return TXNSetTXNObjectControls( HITextViewGetTXNObject(textView), false, 1, iControlTags, iControlData );
}

// IncrementFontSizeOfCurrentSelection will call through to TXNSetTypeAttributes
OSStatus
TextViewIncrementFontSizeOfCurrentSelection( HIViewRef textView, Boolean incrementUp )
{
    return TextViewIncrementFontSizeOfThisRange( textView,
												 kTXNUseCurrentSelection, kTXNUseCurrentSelection,
												 incrementUp, 1 );
}

OSStatus
TextViewIncrementFontSizeOfThisRange( HIViewRef textView,
									  TXNOffset startRng, TXNOffset endRng,
									  Boolean incrementUp, UInt32 count = 1 /*unused*/ )
{
    SInt32 direction =  incrementUp?kTXNIncrementTypeSize:kTXNDecrementTypeSize;
    
	TXNTypeAttributes	typeAttr[] = {{kTXNQDFontSizeAttribute, kTXNQDFontSizeAttributeSize,
										{0}}};
	typeAttr[0].data.dataValue = direction;
	return TXNSetTypeAttributes( HITextViewGetTXNObject(textView), 1, typeAttr, startRng, endRng );
}

// SetFontSizeOfCurrentSelection will call through to TXNSetTypeAttributes
OSStatus
TextViewSetFontSizeOfCurrentSelection( HIViewRef textView, UInt32 setSize )
{
    return TextViewSetFontSizeOfThisRange( textView, setSize, kTXNUseCurrentSelection, kTXNUseCurrentSelection );
}

OSStatus
TextViewSetFontSizeOfThisRange( HIViewRef textView, UInt32 setSize, TXNOffset startRng, TXNOffset endRng )
{
    TXNTypeAttributes	typeAttr[] = {{kTXNQDFontSizeAttribute, kTXNQDFontSizeAttributeSize,
												{0}}};
	typeAttr[0].data.dataValue = (setSize << 16);
	return TXNSetTypeAttributes( HITextViewGetTXNObject(textView), 1, typeAttr, startRng, endRng );
}

// SetFontStyleOfCurrentSelection will call through to TXNSetTypeAttributes
OSStatus
TextViewSetFontStyleOfCurrentSelection( HIViewRef textView, UInt32 setStyle) // style really SInt16?
{
    return TextViewSetFontStyleOfThisRange( textView, setStyle, kTXNUseCurrentSelection, kTXNUseCurrentSelection );
}

OSStatus
TextViewSetFontStyleOfThisRange( HIViewRef textView, UInt32 setStyle, TXNOffset startRng, TXNOffset endRng )
{
	TXNTypeAttributes typeAttr[] = {{kTXNQDFontStyleAttribute, kTXNQDFontStyleAttributeSize,
									{0}}};
	typeAttr[0].data.dataValue = setStyle;
	return TXNSetTypeAttributes( HITextViewGetTXNObject(textView), 1, typeAttr, startRng, endRng );
}

OSStatus
TextViewSetMargins( HIViewRef textView, SInt16 top, SInt16 left, SInt16 right )
{
	TXNMargins margins;
	
	margins.topMargin = top;
	margins.leftMargin = left;
	margins.bottomMargin = 0; // cannot change bottomMargin in MLTE at this time
	margins.rightMargin = right;

	//	Set the margins in the object.
	TXNControlTag	iControlTags[] = {kTXNMarginsTag};
	TXNControlData	iControlData[1];
	iControlData[0].marginsPtr = &margins;

	return TXNSetTXNObjectControls( HITextViewGetTXNObject(textView), false, 1, iControlTags, iControlData );
}

#pragma mark --- HITextView set alpha background for transparency ---

OSStatus
TextViewSetBGAlpha( HIViewRef textView, float alpha )
{
	OSStatus status = paramErr;
	
	SInt32 unused;
	CGColorRef prevColor = NULL;
	CGColorRef newColor = NULL;
	
	status = HITextViewCopyBackgroundColor( textView, &prevColor );
  
	require_noerr( status, FAIL );
	
	// make a new copy with alpha
	newColor = CGColorCreateCopyWithAlpha( prevColor, alpha );
	
	require_action( newColor != NULL, FAIL, status = paramErr );

	// reset the new color with alpha
	status = HITextViewSetBackgroundColor( textView, newColor );

	if( newColor != NULL )
		CGColorRelease( newColor );
		
	FAIL:
	return status;
}

#pragma mark --- FEATURE DEMO FUNCTIONS ---

OSStatus
TextViewDemoSpellingSupport( HIViewRef textView )
{
	OSStatus status = paramErr;
	static const CFStringRef taskCFStr = CFSTR("TaskSpellcheck");

	status = TextViewSpellingSupport( textView, true );

	MyShowSpellCheckPanel();

	verify_noerr( SignalHelpMessage( HIViewGetWindow( textView ), taskCFStr ) );
	return status; 
}

OSStatus
TextViewDemoFontPanelSupport( HIViewRef textView )
{
	OSStatus status = paramErr;
	static const CFStringRef taskCFStr = CFSTR("TaskFontPanel");

	status = TextViewFontPanelSupport( textView, true );

	if ( ! FPIsFontPanelVisible() )
		verify_noerr( FPShowHideFontPanel() );

	verify_noerr( SignalHelpMessage( HIViewGetWindow( textView ), taskCFStr ) );
	return status; 
}

OSStatus
TextViewDemoActionGroup( HIViewRef textView )
{
	const char* sampleText = "bigger...\n";
	static const CFStringRef taskCFStr = CFSTR("TaskActionGroup");

	static const CFStringRef actionLabel = CFSTR("Example Action Group");
	
	TXNObject txnObj = HITextViewGetTXNObject(textView);
	
    TXNTypeAttributes	typeAttr[] = {{kTXNQDFontSizeAttribute,
									   kTXNQDFontSizeAttributeSize,
										{0}}};
	typeAttr[0].data.dataValue = (24 << 16);
	
	TXNBeginActionGroup( txnObj, actionLabel);
	
	TXNSetData( txnObj, kTXNTextData, sampleText, strlen( sampleText ), kTXNStartOffset, kTXNEndOffset );

	TXNSetTypeAttributes( txnObj, 1, typeAttr, kTXNStartOffset, kTXNEndOffset );
					  
	typeAttr[0].data.dataValue = (36 << 16);
	TXNSetTypeAttributes( txnObj, 1, typeAttr, kTXNEndOffset, kTXNEndOffset );
	
	TXNSetData( txnObj, kTXNTextData, sampleText, strlen( sampleText ), kTXNEndOffset, kTXNEndOffset );
				  
	typeAttr[0].data.dataValue = (48 << 16);
	TXNSetTypeAttributes( txnObj, 1, typeAttr, kTXNEndOffset, kTXNEndOffset );
	
	TXNSetData( txnObj, kTXNTextData, sampleText, strlen( sampleText ), kTXNEndOffset, kTXNEndOffset );
	
	typeAttr[0].data.dataValue = (72 << 16);
	TXNSetTypeAttributes( txnObj, 1, typeAttr, kTXNEndOffset, kTXNEndOffset );
	
	TXNSetData( txnObj, kTXNTextData, sampleText, strlen( sampleText ), kTXNEndOffset, kTXNEndOffset );
	
	typeAttr[0].data.dataValue = (96 << 16);
	TXNSetTypeAttributes( txnObj, 1, typeAttr, kTXNEndOffset, kTXNEndOffset );
	
	TXNSetData( txnObj, kTXNTextData, sampleText, strlen( sampleText ) - 1 /*don't set last line return*/, 
				kTXNEndOffset, kTXNEndOffset );
	
	TXNEndActionGroup( txnObj );  
	SignalHelpMessage( HIViewGetWindow( textView ), taskCFStr );
	return noErr;
}

// invoke navigation services code to pick a file to open
// When the user confirms the Navigation Services dialog, the
// OpenFileCallback will be called with the info obtained from Nav Services.
OSStatus
TextViewDemoReadFromCFURL( HIViewRef textView )
{
	if( gNewFileOpener == NULL )
	{
		gNewFileOpener = new CNavOpenDialog();
		gNewFileOpener->Init();
		
		
	}
	else
	{
		gNewFileOpener->ReInit();
	}
	//*** Need to call SetOpenCallback everytime otherwise the first textView sticks
	//and documents opened after the first go to the first.
	// Look for a return to the TextView in the OpenFileCallback() function
	// function when the Navigation Services dialog is confirmed
	gNewFileOpener->SetOpenCallback( OpenFileCallback, textView );
	gNewFileOpener->Run();
	
	return noErr;
}					

// invoke navigation services code to pick where to save a file
// When the user confirms the Navigation Services dialog, the
// WriteFileCallback will be called with the info obtained from Nav Services.
OSStatus
TextViewDemoWriteToCFURL( HIViewRef textView )
{
	WindowRef window = HIViewGetWindow( textView );
	
	if( window != NULL )
	{
		if( gNewFileSaver == NULL )
		{
			gNewFileSaver = new CNavSaveDialog();
			gNewFileSaver->Init( window );
			
			// Look for a return to the TextView in the WriteFileCallback() function
			// function when the Navigation Services dialog is confirmed
			gNewFileSaver->SetSaveCallback( WriteFileCallback, textView );
		}
		else
			gNewFileSaver->ReInit( window );
		gNewFileSaver->Run();
	}
	return noErr;
}

#pragma mark --- UTILITY FUNCTIONS ---

// add some default text to the HITextView
OSStatus
AddTextToTheTextView( HIViewRef textView )
{
	OSStatus status = paramErr;
	CFURLRef fileURLRef = NULL;
	fileURLRef = CFBundleCopyResourceURL( CFBundleGetMainBundle(), CFSTR("MLTEShowcaseSample.mlte"), NULL, NULL );
	if( fileURLRef != NULL )
	{
		CMLTEViewData* mlteData = NULL;
		
		mlteData = TextViewRetrieveMLTEInstanceData( textView );
		if( mlteData != NULL )
			mlteData->SetURL( fileURLRef );
			
		if( mlteData != NULL )
			mlteData->SetURL( fileURLRef );
			
		TextViewReadCFURL( textView, fileURLRef );
		
		CFRelease( fileURLRef );
	}
	return status;
}

// add an HIImageViewRef to the parentView
HIViewRef
AddImage( HIViewRef parentView )
{
	OSStatus status = paramErr;
	HIViewRef cgImageView = NULL;
	CGImageRef cgImage = NULL;
	CGDataProviderRef cgImageProvider = NULL;
	HILayoutInfo layoutInfo; 
	CFBundleRef theAppBundle = NULL;
	CFStringRef imageFileName = NULL;
	CFURLRef imageURL = NULL;
	
	HIRect parentFrame;
	
	theAppBundle = CFBundleGetMainBundle(); 
	imageFileName = CFStringCreateWithCString( NULL /*allocator*/, "rainbowpark.jpg", kCFStringEncodingASCII ); 
	imageURL = CFBundleCopyResourceURL( theAppBundle, imageFileName, NULL, NULL );

	cgImageProvider = CGDataProviderCreateWithURL(imageURL);
	
	require( cgImageProvider != NULL, FAIL );
	cgImage = CGImageCreateWithJPEGDataProvider( cgImageProvider, NULL, false, kCGRenderingIntentDefault );
	
	require( cgImage != NULL, FAIL );
	
	CGDataProviderRelease( cgImageProvider );
	CFRelease( imageFileName );
	
	HIImageViewCreate( cgImage, &cgImageView );
	CGImageRelease( cgImage);
	
	require( cgImageView != NULL, FAIL );
	
	layoutInfo.binding.left.kind = kHILayoutBindLeft;
	layoutInfo.binding.left.toView = NULL;
	layoutInfo.binding.left.offset = 0.0;
	layoutInfo.binding.top.kind = kHILayoutBindTop;
	layoutInfo.binding.top.toView = NULL;
	layoutInfo.binding.top.offset = 0.0;
	layoutInfo.binding.right.kind = kHILayoutBindRight;
	layoutInfo.binding.right.toView = NULL;
	layoutInfo.binding.right.offset = 0.0;
	layoutInfo.binding.bottom.kind = kHILayoutBindBottom;
	layoutInfo.binding.bottom.toView = NULL;
	layoutInfo.binding.bottom.offset = 0.0;
	verify_noerr( HIViewSetLayoutInfo(cgImageView, &layoutInfo) );
	verify_noerr( HIImageViewSetScaleToFit(cgImageView, true) );
	status = HIViewAddSubview(parentView, cgImageView);
	
	if( status != noErr )
	{
		CFRelease( cgImageView );
		cgImageView = NULL;
	}
	
	FAIL:
	return cgImageView;

}

// util function tells the HelpTextFrame to display a helpful message
OSStatus
SignalHelpMessage( WindowRef window, CFStringRef taskCFStr )
{
	OSStatus status = paramErr;
	if( window != NULL && taskCFStr != NULL )
	{
		HIViewRef helpFrame = NULL;
		status = GetHelpTextFrame( window, helpFrame );
		if( helpFrame != NULL )
			status = HelpTextFrameSetLocalizedHelpMessageByKey( helpFrame, taskCFStr );
		else
			status = paramErr;
	}
	return status;
}
