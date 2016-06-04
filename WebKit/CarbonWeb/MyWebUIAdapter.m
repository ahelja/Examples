/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MyWebUIAdapter.h"

struct PanelInfo
{
	WindowRef		window;
	CFStringRef		string;
};

static OSStatus	InputPanelHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static void
OpenDialogEventProc( const NavEventCallbackMessage callbackSelector, 
									   NavCBRecPtr callbackParms, 
									   NavCallBackUserData callbackUD );

static Boolean 	PrepareEvent( UInt32 inKind, EventRef* outEvent );
static void 	FinishEvent( EventRef inEvent, HIObjectRef inObject, bool inSendToAll );

@implementation MyWebUIAdapter

- initWithHIObject: (HIObjectRef)inObject
{
    [super init];
    _object = inObject;	// non retained
    return self;
}

- (HIObjectRef)hiobject
{
	return _object;
}

- (WebView *)webView:(WebView *)sender createWebViewWithRequest:(NSURLRequest *)request
{
	WebView*	result = NULL;
	EventRef		event;

	if ( PrepareEvent( kMyEventWebCreateWindow, &event ) )
	{
		SetEventParameter( event, kParamNSURLRequest, kTypeNSURLRequest,
				sizeof( NSURLRequest *), &request );
		if ( SendEventToEventTargetWithOptions( event, HIObjectGetEventTarget( _object ),
					kEventTargetDontPropagate ) == noErr )
		{
			GetEventParameter( event, kParamWebView, kTypeWebView, NULL,
					sizeof( WebView * ), NULL, &result );
		}
		
		ReleaseEvent( event );
	}
	
	return result;
}

- (void)webViewShow:(WebView *)sender
{
	EventRef	event;
	
	if ( PrepareEvent( kMyEventWebShowWindow, &event ) )
		FinishEvent( event, _object, false );
}

- (void)showWindowBehindFrontmost
{
	EventRef	event;
	
	if ( PrepareEvent( kMyEventWebShowWindowBehindFrontmost, &event ) )
		FinishEvent( event, _object, false );
}

- (void)webView:(WebView *)sender setStatusText:(NSString *)text
{
	EventRef	event;
	
	if ( PrepareEvent( kMyEventWebSetStatusText, &event ) )
	{
		SetEventParameter( event, kParamText, typeCFStringRef, sizeof( CFStringRef ), &text );
		FinishEvent( event, _object, false );
	}
}

- (NSString *)webViewStatusText:(WebView *)sender
{
	NSString*		result = NULL;
	EventRef		event;

	if ( PrepareEvent( kMyEventWebGetStatusText, &event ) )
	{
		if ( SendEventToEventTargetWithOptions( event, HIObjectGetEventTarget( _object ),
					kEventTargetDontPropagate ) == noErr )
		{
			GetEventParameter( event, kParamText, typeCFStringRef, NULL,
					sizeof( CFStringRef ), NULL, &result );
		}
		
		ReleaseEvent( event );
	}
	
	return result;
}

- (void)webView:(WebView *)sender mouseDidMoveOverElement:(NSDictionary *)elementInformation modifierFlags:(unsigned int)modifierFlags
{
	EventRef	event;
	
	if ( PrepareEvent( kMyEventWebMouseMovedOverElement, &event ) )
	{
		SetEventParameter( event, kParamWebElementInfo, kTypeCFDictionaryRef, sizeof( CFDictionaryRef ), &elementInformation );
//		SetEventParameter( event, kEventParamKeyModifiers, typeUInt32, sizeof( UInt32 ), &modifierFlags );
// ••• FIX ME!!! Need to fix up modifier flags
//#warning Need to fix up modifier flags for mouse over event
		FinishEvent( event, _object, true );
	}
}

- (BOOL)webViewAreToolbarsVisible: (WebView *)sender
{
	BOOL		result = NO;
	EventRef	event;
	
	if ( PrepareEvent( kMyEventWebAreToolbarsVisible, &event ) )
	{
		Boolean		answer = false;
		
		if ( SendEventToEventTargetWithOptions( event, HIObjectGetEventTarget( _object ),
				kEventTargetDontPropagate ) == noErr )
		{
			GetEventParameter( event, kEventParamResult, typeBoolean, NULL,
					sizeof( Boolean ), NULL, &answer );
			
			result = (BOOL)answer;
		}
		
		ReleaseEvent( event );
	}
	
	return result;
}

- (void)webView:(WebView *)sender setToolbarsVisible:(BOOL)inVisible
{
	EventRef	event;
	
	if ( PrepareEvent( kMyEventWebSetToolbarsVisible, &event ) )
	{
		Boolean		visible = inVisible;

		SetEventParameter( event, kParamVisible, typeBoolean, sizeof( Boolean ), &visible );
		FinishEvent( event, _object, false );
	}
}

- (BOOL)webViewIsStatusBarVisible:(WebView *)sender
{
	BOOL		result = NO;
	EventRef	event;
	
	if ( PrepareEvent( kMyEventWebIsStatusBarVisible, &event ) )
	{
		Boolean		answer = false;
		
		if ( SendEventToEventTargetWithOptions( event, HIObjectGetEventTarget( _object ),
				kEventTargetDontPropagate ) == noErr )
		{
			GetEventParameter( event, kEventParamResult, typeBoolean, NULL,
					sizeof( Boolean ), NULL, &answer );
			
			result = (BOOL)answer;
		}
		
		ReleaseEvent( event );
	}
	
	return result;
}

- (void)webView:(WebView *)sender setStatusBarVisible:(BOOL)inVisible
{
	EventRef	event;
	
	if ( PrepareEvent( kMyEventWebSetStatusBarVisible, &event ) )
	{
		Boolean		visible = inVisible;

		SetEventParameter( event, kParamVisible, typeBoolean, sizeof( Boolean ), &visible );
		FinishEvent( event, _object, false );
	}
}

- (void)webView:(WebView *)sender setFrame:(NSRect)frame
{
	EventRef	event;
	
	if ( PrepareEvent( kMyEventWebSetWindowBounds, &event ) )
	{
		// NSRect and HI/CGRect have the same layout
		SetEventParameter( event, kParamBounds, typeHIRect, sizeof( HIRect ), &frame );
		FinishEvent( event, _object, false );
	}
}

- (NSRect)webViewFrame:(WebView *)sender
{
	NSRect rect = { { 0, 0 }, { 0, 0 } };
	
	fprintf( stderr, "frame: IMPLEMENT ME!\n" );

	return rect;
}

- (void)webView:(WebView *)sender runJavaScriptAlertPanelWithMessage:(NSString *)message
{
	AlertStdCFStringAlertParamRec		param;
	DialogRef							alert;
	DialogItemIndex						itemHit;

	param.version 		= kStdCFStringAlertVersionOne;
	param.movable 		= true;
	param.helpButton 	= false;
	param.defaultText 	= (CFStringRef)kAlertDefaultOKText;
	param.cancelText 	= NULL;
	param.otherText 	= NULL;
	param.defaultButton = kAlertStdAlertOKButton;
	param.cancelButton 	= 0;
	param.position 		= kWindowDefaultPosition;
	param.flags 		= 0;
	
	CreateStandardAlert( 0, (CFStringRef)message, NULL, NULL, &alert );
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	RunStandardAlert( alert, NULL, &itemHit );
        [pool release];
}

- (BOOL)webView:(WebView *)sender runJavaScriptConfirmPanelWithMessage:(NSString *)message
{
	AlertStdCFStringAlertParamRec		param;
	DialogRef							alert;
	DialogItemIndex						itemHit;

	param.version 		= kStdCFStringAlertVersionOne;
	param.movable 		= true;
	param.helpButton 	= false;
	param.defaultText 	= (CFStringRef)kAlertDefaultOKText;
	param.cancelText 	= (CFStringRef)kAlertDefaultCancelText;
	param.otherText 	= NULL;
	param.defaultButton = kAlertStdAlertOKButton;
	param.cancelButton 	= kAlertStdAlertCancelButton;
	param.position 		= kWindowDefaultPosition;
	param.flags 		= 0;
	
	CreateStandardAlert( 0, (CFStringRef)message, NULL, &param, &alert );
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	RunStandardAlert( alert, NULL, &itemHit );
        [pool release];
	
	return (itemHit == kAlertStdAlertOKButton );
}

- (NSString *)webView:(WebView *)sender runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt defaultText:(NSString *)defaultText
{
    IBNibRef 			nibRef;
	OSStatus			err;
	WindowRef			window;
	EventTypeSpec		kEvents[] = { { kEventClassCommand, kEventCommandProcess } };
	PanelInfo			info;
	ControlID			promptID = { 'INPU', 1 };
	ControlID			textFieldID = { 'INPU', 2 };
	HIViewRef			view;

        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
    err = CreateNibReference( CFSTR( "main" ), &nibRef );
    require_noerr( err, CantGetNibRef );
	
	err = CreateWindowFromNib( nibRef, CFSTR( "InputPanel" ), &window );
	require_noerr( err, CantCreateWindow );
	
	DisposeNibReference( nibRef );

	if ( prompt )
	{
		HIViewFindByID( HIViewGetRoot( window ), promptID, &view );
		SetControlData( view, 0, kControlStaticTextCFStringTag, sizeof( CFStringRef ), &prompt );
	}
	
	if ( defaultText )
	{
		HIViewFindByID( HIViewGetRoot( window ), textFieldID, &view );
		SetControlData( view, 0, kControlEditTextCFStringTag, sizeof( CFStringRef ), &defaultText );	
	}

	info.window = window;
	info.string = NULL;

	InstallWindowEventHandler( window, InputPanelHandler, GetEventTypeCount( kEvents ),
				kEvents, &info, NULL );
	ShowWindow( window );
	
	RunAppModalLoopForWindow( window );

	DisposeWindow( window );

CantCreateWindow:
CantGetNibRef:

        [pool release];

	return (NSString*)info.string;
}

- (void)webView:(WebView *)sender runOpenPanelForFileButtonWithResultListener:(id<WebOpenPanelResultListener>)resultListener
{
	NavDialogCreationOptions	dialogOptions;
	NavDialogRef				dialog;
	OSStatus 					theErr = noErr;

	NavGetDefaultDialogCreationOptions( &dialogOptions );

	dialogOptions.modality = kWindowModalityWindowModal;
	dialogOptions.parentWindow = (WindowRef)_object; // hack for now
	dialogOptions.clientName = CFStringCreateWithPascalString( NULL, LMGetCurApName(), GetApplicationTextEncoding());
	dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;

	theErr = NavCreateChooseFileDialog( &dialogOptions, NULL, OpenDialogEventProc, NULL, NULL, resultListener, &dialog );
	if ( theErr == noErr )
	{
		theErr = NavDialogRun( dialog );
	}
	
	if ( theErr != noErr )
	{
		NavDialogDispose( dialog );
		[resultListener cancel];
	}
}

@end

static void
OpenDialogEventProc( const NavEventCallbackMessage callbackSelector, 
									   NavCBRecPtr callbackParms, 
									   NavCallBackUserData callbackUD )
{
	id<WebOpenPanelResultListener> 	resultListener = (id<WebOpenPanelResultListener>)callbackUD;

	switch ( callbackSelector )
	{	
		case kNavCBUserAction:
			if ( callbackParms->userAction == kNavUserActionChoose )
			{
				NavReplyRecord	reply;
				OSStatus		status;
				
				status = NavDialogGetReply( callbackParms->context, &reply );
				if ( status == noErr )
				{
					OSStatus		anErr;
					AEKeyword		keywd;
					DescType		returnedType;
					Size			actualSize;
					FSRef 			fileRef;
					FSCatalogInfo	theCatInfo;
					UInt8			path[1024];
					CFStringRef		filename;
					
					anErr = AEGetNthPtr( &reply.selection, 1, typeFSRef, &keywd, &returnedType,
									(Ptr)(&fileRef), sizeof( fileRef ), &actualSize );
					require_noerr(anErr, AEGetNthPtr);
			
					anErr = FSGetCatalogInfo( &fileRef, kFSCatInfoFinderInfo, &theCatInfo, NULL, NULL, NULL );
					require_noerr(anErr, FSGetCatalogInfo);
					
					FSRefMakePath( &fileRef, path, sizeof( path ) );
					filename = CFStringCreateWithCString( NULL, (char *)path, kCFStringEncodingUTF8 );
					[resultListener chooseFilename:(NSString*)filename];
					CFRelease( filename );

				AEGetNthPtr:
				FSGetCatalogInfo:

					NavDisposeReply( &reply );
				}
			}
			else if ( callbackParms->userAction == kNavUserActionCancel )
			{
				[resultListener cancel];
			}
			break;

		case kNavCBTerminate:
			NavDialogDispose( callbackParms->context );
			break;
	}
}

static OSStatus
InputPanelHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	HICommand			command;
	OSStatus			result = eventNotHandledErr;
	PanelInfo*			info = (PanelInfo*)inUserData;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
			sizeof( HICommand ), NULL, &command );
	
	if ( command.commandID == kHICommandCancel )
	{
		QuitAppModalLoopForWindow( info->window );
		result = noErr;
	}
	else if ( command.commandID == kHICommandOK )
	{
		ControlID		theID = { 'INPU', 2 };
		HIViewRef		textField;
		
		HIViewFindByID( HIViewGetRoot( info->window ), theID, &textField );
		GetControlData( textField, 0, kControlEditTextCFStringTag, sizeof( CFStringRef ), &info->string, NULL );
		QuitAppModalLoopForWindow( info->window );
	}
	
	return result;
}


static Boolean
PrepareEvent(
	UInt32 			inKind,
	EventRef* 		outEvent )
{
	EventRef 	event;
	OSStatus	err;
	Boolean		result = false;
	
	err = CreateEvent( NULL, kMyEventClassWebUI, inKind,
				GetCurrentEventTime(), 0, &event );
	if ( err == noErr )
	{
		*outEvent = event;
		result = true;
	}

	return result;
}

static void
FinishEvent(
	EventRef 		inEvent,
	HIObjectRef 	inObject,
	bool			inSendToAll )
{
	OptionBits	options = kEventTargetDontPropagate;
	
	if ( inSendToAll )
		options |= kEventTargetSendToAllHandlers;

	SendEventToEventTargetWithOptions( inEvent, HIObjectGetEventTarget( inObject ),
			options );
	ReleaseEvent( inEvent );
}
