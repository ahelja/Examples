/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
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

#include "TWebWindow.h"
#include "TPNGButton.h"

#define TRACE_RESOURCE_LOAD		0

enum
{
	kEventControlParentBoundsChanged	= 2009
};

static const EventTypeSpec kWindowEvents[] = {
	{ kEventClassMouse, kEventMouseUp },
	{ kEventClassMouse, kEventMouseMoved },
	{ kEventClassMouse, kEventMouseDragged },
	{ kEventClassMouse, kEventMouseWheelMoved },
	{ kEventClassCommand, kEventCommandProcess },
	{ kEventClassWindow, kEventWindowBoundsChanged },
	{ kEventClassWindow, kEventWindowDrawContent },
};

static const EventTypeSpec kControlEvents[] = {
	{ kEventClassControl, kEventControlParentBoundsChanged }
};

static const EventTypeSpec kTextEvents[] = {
	{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent }
};

static const EventTypeSpec kKeyboardEvents[] = {
        { kEventClassKeyboard, kEventRawKeyDown }
};

static const EventTypeSpec kMenuEvents[] = {
	{ kEventClassMenu, kEventMenuOpening }
};

static const EventTypeSpec kFrameLoadEvents[] = {
	{ kMyEventClassWebFrameLoad, kMyEventWebFrameLoadStarted },
	{ kMyEventClassWebFrameLoad, kMyEventWebReceivedPageTitle },
	{ kMyEventClassWebFrameLoad, kMyEventWebReceivedPageIcon },
	{ kMyEventClassWebFrameLoad, kMyEventWebFrameLoadDone },
        { kMyEventClassWebFrameLoad, kMyEventWebFrameLoadFailed }
};

static const EventTypeSpec kResourceLoadEvents[] = {
	{ kMyEventClassWebResourceLoad, kMyEventWebResourceGetIdentifier },
	{ kMyEventClassWebResourceLoad, kMyEventWebResourceDidReceiveResponse },
	{ kMyEventClassWebResourceLoad, kMyEventWebResourceDidReceiveContentLength },
	{ kMyEventClassWebResourceLoad, kMyEventWebResourceLoadFinished },
	{ kMyEventClassWebResourceLoad, kMyEventWebResourceLoadFailed }
};

static const EventTypeSpec kWindowOperationEvents[] = {
	{ kMyEventClassWebUI, kMyEventWebSetStatusText },
	{ kMyEventClassWebUI, kMyEventWebGetStatusText },
	{ kMyEventClassWebUI, kMyEventWebMouseMovedOverElement },
	{ kMyEventClassWebUI, kMyEventWebIsStatusBarVisible },
	{ kMyEventClassWebUI, kMyEventWebCreateWindow },
	{ kMyEventClassWebUI, kMyEventWebShowWindow },
	{ kMyEventClassWebUI, kMyEventWebShowWindowBehindFrontmost }
};

const ControlID		kTextFieldID = { 'WEBW', 1 };
const ControlID		kWebViewID = { 'WEBW', 2 };
const ControlID		kStatusFieldID = { 'WEBW', 3 };
const ControlID		kProgressBarID = { 'WEBW', 4 };
const ControlID		kBackButtonID = { 'WEBW', 5 };
const ControlID		kForwardButtonID = { 'WEBW', 6 };
const ControlID		kStopButtonID = { 'WEBW', 7 };
const ControlID		kReloadButtonID = { 'WEBW', 8 };

#define kControlStaticTextIsMultilineTag    'stim'

// This is a workaround that's only needed for Panther and earlier. It allows command-space, kana and eisu to toggle input methods.
static OSStatus RawKeyInputHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );

TWebWindow::TWebWindow()
	: TWindow( CFSTR( "main" ), CFSTR( "MainWindow" ) ),
        fPrintSession(NULL),
        fPageFormat(NULL),
        fPrintSettings(NULL)
{
	ControlRef				tempView, contentView;
	HIRect						contentRect, viewRect;
	HIRect						frame;
	float						statusRight;
	ControlFontStyleRec			fontStyle;
	Boolean						kYES = true;
	Boolean						kNO = false;
	HIRect						iconRect = { { 0, 0 }, { 16, 16 } };
	WindowAttributes 			attr;

	GetWindowAttributes( GetWindowRef(), &attr );
	fIsComposited = (attr & kWindowCompositingAttribute);

	// Take our user pane from the window and use it to create and position
	// our new web view

	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kWebViewID, &tempView );
	HIViewGetFrame( tempView, &frame );
	CFRelease( tempView );

	HIWebViewCreate( &fWebView );
	SetControlID( fWebView, &kWebViewID );
	HIViewSetFrame( fWebView, &frame );
	HIViewSetVisible( fWebView, true );
	GetRootControl( GetWindowRef(), &contentView );
	HIViewAddSubview( contentView, fWebView );
	
//	fController = [[WebView alloc] initWithView: HIWebFrameViewGetNSView( fWebView ) frameName: nil groupName: nil];
	fController = HIWebViewGetWebView( fWebView );

	fFrameLoadelegate = [[MyFrameLoadAdapter alloc] initWithHIObject: (HIObjectRef)GetWindowRef()];
	[fController setFrameLoadDelegate:fFrameLoadelegate];
	RegisterForEvents( GetEventTypeCount( kFrameLoadEvents ), kFrameLoadEvents );
	
	fResourceLoadDelegate = [[MyWebResourceLoadAdapter alloc] initWithHIObject: (HIObjectRef)GetWindowRef()];
	[fController setResourceLoadDelegate:fResourceLoadDelegate];
	RegisterForEvents( GetEventTypeCount( kResourceLoadEvents ), kResourceLoadEvents );
	
	fWindowOperationsDelegate = [[MyWebUIAdapter alloc] initWithHIObject: (HIObjectRef)GetWindowRef()];
	[fController setUIDelegate:fWindowOperationsDelegate];
	RegisterForEvents( GetEventTypeCount( kWindowOperationEvents ), kWindowOperationEvents );

	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kStatusFieldID, &fStatusText );
	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kProgressBarID, &fProgressBar );
	
	// Set up the back button image and attach a back menu
	
	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kBackButtonID, &fBackButton );
	TPNGButton::GetFromHIViewRef( fBackButton )->SetImageName( CFSTR( "Back" ) );
	
	fBackMenu = new TBackForwardMenu( fController, TBackForwardMenu::kBack );
	TPNGButton::GetFromHIViewRef( fBackButton )->SetMenu( fBackMenu->GetMenuRef() );

	// Set up the forward button image and attach a forward menu
	
	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kForwardButtonID, &fForwardButton );
	TPNGButton::GetFromHIViewRef( fForwardButton )->SetImageName( CFSTR( "Forward" ) );

	fForwardMenu = new TBackForwardMenu( fController, TBackForwardMenu::kForward );
	TPNGButton::GetFromHIViewRef( fForwardButton )->SetMenu( fForwardMenu->GetMenuRef() );

	// Now set the images for the reload and stop button

	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kReloadButtonID, &fReloadButton );
	TPNGButton::GetFromHIViewRef( fReloadButton )->SetImageName( CFSTR( "Reload" ) );

	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kStopButtonID, &fStopButton );
	TPNGButton::GetFromHIViewRef( fStopButton )->SetImageName( CFSTR( "Stop" ) );

	// Make the status text smaller (IB doesn't let me do it there)
	fontStyle.flags = kControlUseThemeFontIDMask;
	fontStyle.font = kThemeLabelFont;
	SetControlFontStyle( fStatusText, &fontStyle );

	// And don't let it wrap
	SetControlData( fStatusText, 0, kControlStaticTextIsMultilineTag, sizeof( Boolean ), &kNO );

	// Get some metrics to be used for resizing.

	// Get the real content view for this purpose if we're not in composited mode, since
	// the traditional root control spans all of QuickDraw space, which isn't very useful.

	if ( !fIsComposited )
		HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kHIViewWindowContentID, &contentView );

	HIViewGetBounds( contentView, &contentRect );
	HIViewGetFrame( fWebView, &viewRect );

	fBrowserBottomGap = CGRectGetMaxY( contentRect ) - CGRectGetMaxY( viewRect );
	fBrowserRightGap = CGRectGetMaxX( contentRect ) - CGRectGetMaxX( viewRect );
	
	HIViewGetFrame( fStatusText, &viewRect );
	fStatusBottomGap = CGRectGetMaxY( contentRect ) - CGRectGetMaxY( viewRect );
	statusRight = CGRectGetMaxX( viewRect );

	HIViewGetFrame( fProgressBar, &viewRect );
	fProgressBottomGap = CGRectGetMaxY( contentRect ) - CGRectGetMaxY( viewRect );
	fProgressRightGap = CGRectGetMaxX( contentRect ) - CGRectGetMaxX( viewRect );

	fStatusProgressGap = viewRect.origin.x - statusRight;
	
	// Install our text input handler on the text field to listen for
	// return or enter. This will trigger us to load the URL typed.

	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kTextFieldID, &fTextField );
	InstallEventHandler( GetControlEventTarget( fTextField ), TextInputHandler,
		GetEventTypeCount( kTextEvents ), kTextEvents, this, NULL );
	
	SetControlData( fTextField, 0, kControlEditTextSingleLineTag, sizeof( Boolean ), &kYES );
        
	// Install a parent bounds changed handler on the content view. This is the only
	// way to get proper updating to happen during live resize. If you install a
	// bounds changed handler on the content, or a window bounds changed handler, you
	// will be in for a world of yuck. Doing it this way ensures a very fragile
	// order of operations that needs to happen just so.

	InstallEventHandler( GetControlEventTarget( contentView ), ContentBoundsChanged,
		GetEventTypeCount( kControlEvents ), kControlEvents, this, NULL );

	RegisterForEvents( GetEventTypeCount( kWindowEvents ), kWindowEvents );
	
	// Create an image view by hand to show the page icon
	// Unfortunately, ImageView does not work in non-compositing mode

	if ( fIsComposited )
	{
		HIImageViewCreate( NULL, &fPageIcon );
		HIImageViewSetScaleToFit( fPageIcon, true );
		HIViewAddSubview( contentView, fPageIcon );
		HIViewGetFrame( fTextField, &frame );
		iconRect.origin.x = CGRectGetMaxX( frame ) + 10;
		iconRect.origin.y = CGRectGetMinY( frame );
		HIViewSetFrame( fPageIcon, &iconRect );
		HIViewSetVisible( fPageIcon, true );
	}
	else
	{
		fPageIcon = NULL;
	}

	fResourceCount = 0;
	fResourceCompletedCount = 0;
	fResourceFailedCount = 0;
	
	// Need a Panther check here
	//ChangeWindowAttributes( GetWindowRef(), kWindowAsyncDragAttribute , 0 );
	
	TWebWindow* temp = this;
	SetWindowProperty( GetWindowRef(), 'WEBW', 1, sizeof( TWebWindow * ), (const void *)&temp );
	
        long version;
        Gestalt(gestaltSystemVersion, &version);
        if (version >= 0x01030 && version < 0x01040) {
            // This is a workaround that's only needed for Panther. It allows command-space, kana and eisu to toggle input methods.
            InstallEventHandler( GetControlEventTarget( fWebView ), RawKeyInputHandler,
                                 GetEventTypeCount( kKeyboardEvents ), kKeyboardEvents, NULL, NULL );
            InstallWindowEventHandler( GetWindowRef(), RawKeyInputHandler, 
                                       GetEventTypeCount( kKeyboardEvents ), kKeyboardEvents, NULL, NULL );
        }
}

TWebWindow::~TWebWindow()
{
	if ( fController )
		CFRelease( fController );

	if ( fFrameLoadelegate )
		[fFrameLoadelegate release];

	delete fBackMenu;
	delete fForwardMenu;
}

//-------------------------------------------------------------------------------------
//	TWebWindow::GetFromWindowRef
//-------------------------------------------------------------------------------------
//	Given a window ref, get the corresponding TWebWindow object, if any.
//
TWebWindow*
TWebWindow::GetFromWindowRef( WindowRef inWindow )
{
	TWebWindow*	object = NULL;
	
	GetWindowProperty( inWindow, 'WEBW', 1, sizeof( TWebWindow * ), NULL, &object );
	
	return object;
}


//-------------------------------------------------------------------------------------
//	TWebWindow::GoToItem
//-------------------------------------------------------------------------------------
//	Either uses the topmost window, or creates a new window if none are open and tells
//	it to load the item given.
//
void
TWebWindow::GoToItem( WebHistoryItem* inItem )
{
	WindowRef	window = GetFrontWindowOfClass( kDocumentWindowClass, true );
	bool		opened = false;
	CFStringRef	urlString = (CFStringRef)[inItem URLString];
	CFURLRef	url = CFURLCreateWithString( NULL, urlString, NULL );
	
	while ( window )
	{
		TWebWindow* object;
		
		if ( ( object = GetFromWindowRef( window ) ) != NULL )
		{
			SelectWindow( window );
			object->LoadURL( url );
			opened = true;
			break;
		}
		window = GetNextWindowOfClass( window, kDocumentWindowClass, true );
	}
	
	if ( !opened )
	{
		TWebWindow*	windObj = new TWebWindow();
		windObj->Show();
		windObj->LoadURL( url );
	}
	
	CFRelease( url );
}

//-------------------------------------------------------------------------------------
//	TWebWindow::LoadURL
//-------------------------------------------------------------------------------------
//	Load the specified URL into the main frame.
//
void
TWebWindow::LoadURL( CFURLRef inURL )
{
    NSURLRequest*	request = [NSURLRequest requestWithURL:(NSURL*)inURL];

	LoadRequest( request );
}

//-------------------------------------------------------------------------------------
//	TWebWindow::LoadRequest
//-------------------------------------------------------------------------------------
//	Load the specified request into the main frame.
//
void
TWebWindow::LoadRequest( NSURLRequest* inRequest )
{
    WebFrame* 		mainFrame;

	mainFrame = [fController mainFrame];
	[mainFrame loadRequest:inRequest];
}

//-------------------------------------------------------------------------------------
//	TWebWindow::SetStatus
//-------------------------------------------------------------------------------------
//	Set the status text.
//
void
TWebWindow::SetStatus( CFStringRef inString )
{
	SetControlData( fStatusText, 0, kControlStaticTextCFStringTag, sizeof( CFStringRef ), &inString );
	HIViewSetNeedsDisplay( fStatusText, true );
}

//-------------------------------------------------------------------------------------
//	TWebWindow::CopyStatusText
//-------------------------------------------------------------------------------------
//	Return a copy of the status text when requested.
//
CFStringRef
TWebWindow::CopyStatusText()
{
	CFStringRef		string = NULL;
	
	GetControlData( fStatusText, 0, kControlStaticTextCFStringTag, sizeof( CFStringRef ), &string, NULL );
	
	return string;
}

//-------------------------------------------------------------------------------------
//	TWebWindow::SetTextField
//-------------------------------------------------------------------------------------
//	Sets the link in the edit field as the user clicks links.
//
void
TWebWindow::SetTextField( CFStringRef inString )
{
	ControlEditTextSelectionRec	selection = { 0, 0 };
	
	SetControlData( fTextField, 0, kControlEditTextCFStringTag, sizeof( CFStringRef ), &inString );
 	SetControlData( fTextField, 0, kControlEditTextSelectionTag, sizeof( ControlEditTextSelectionRec ), &selection );
	HIViewSetNeedsDisplay( fTextField, true );
}

//-------------------------------------------------------------------------------------
//	TWebWindow::SetPageIcon
//-------------------------------------------------------------------------------------
//	Set the page image.
//
void
TWebWindow::SetPageIcon( CGImageRef inImage )
{
	if ( fPageIcon )
		HIImageViewSetImage( fPageIcon, inImage );
}

//-------------------------------------------------------------------------------------
//	TWebWindow::UpdateLoadStatus
//-------------------------------------------------------------------------------------
//	As the page loads, we need to update our information. When we are finally done, we
//	clear our total resource counts.
//
void
TWebWindow::UpdateLoadStatus()
{
	SetLoadProgress( fResourceCompletedCount + fResourceFailedCount, fResourceCount );
	
	if ( fResourceCompletedCount + fResourceFailedCount == fResourceCount )
	{
		fResourceCount = 0;
		fResourceCompletedCount = 0;
		fResourceFailedCount = 0;
	}
}

//-------------------------------------------------------------------------------------
//	TWebWindow::SetLoadProgress
//-------------------------------------------------------------------------------------
//	Adjust the progress bar and status text during page load.
//
void
TWebWindow::SetLoadProgress( SInt32 inCurrResources, SInt32 inTotalResources )
{
	SetControlMaximum( fProgressBar, inTotalResources );
	SetControlValue( fProgressBar, inCurrResources );

	if ( HIViewIsVisible( fProgressBar ) )
	{
		CFStringRef		string = CFStringCreateWithFormat( NULL, NULL, CFSTR( "Loaded %d of %d" ),
										inCurrResources, inTotalResources );
		
		SetStatus( string );
		CFRelease( string );
	}
}

//-------------------------------------------------------------------------------------
//	TWebWindow::MouseMovedOverElement
//-------------------------------------------------------------------------------------
//	When the mouse rolls over a new element (link, etc.) we are told here. We can extract
//	information from the dictionary handed to us to decide what to show.
//
void
TWebWindow::MouseMovedOverElement( CFDictionaryRef inDict )
{
    CFURLRef URL = (CFURLRef)CFDictionaryGetValue( inDict, (CFStringRef)WebElementLinkURLKey );
	
	if ( URL != NULL )
		SetStatus( (CFStringRef)CFURLGetString( URL ) );
	else
		SetStatus( CFSTR( "" ) );
}

//-------------------------------------------------------------------------------------
//	TWebWindow::CanViewSource
//-------------------------------------------------------------------------------------
//	Tells us when we can and can't view the source of the document displayed in our main
//	frame.
//
Boolean
TWebWindow::CanViewSource()
{
	WebDataSource* dataSource = [[fController mainFrame] dataSource];
	
	return ( [[dataSource representation] canProvideDocumentSource] );
}

//-------------------------------------------------------------------------------------
//	TWebWindow::ViewSource
//-------------------------------------------------------------------------------------
//	Dump source to stdout for now. I really wish we had a text view for Jaguar.
//
void
TWebWindow::ViewSource()
{
	WebDataSource* dataSource = [[fController mainFrame] dataSource];
	
	if ( CanViewSource() )
	{
		CFStringRef	string = (CFStringRef)[[dataSource representation] documentSource];
		CFShow( string );
//		CFRelease( string );
	}
}

//-------------------------------------------------------------------------------------
//	TWebWindow::HandleCommand
//-------------------------------------------------------------------------------------
//	Handle a command.
//
Boolean
TWebWindow::HandleCommand( const HICommand& command )
{
	Boolean		handled = false;
	
	switch( command.commandID )
	{
		case 'LINK':
			{
				CFStringRef		string;
				CFURLRef		url;
				
				GetControlData( fTextField, 0, kControlEditTextCFStringTag, sizeof( CFStringRef ), &string, NULL );			
				url = CFURLCreateWithString( NULL, string, NULL );
				
				LoadURL( url );
				
				CFRelease( url );
				
				handled = true;
			}
			break;
			
		case 'BACK':
			[fController goBack];
			handled = true;
			break;

		case 'FWD ':
			[fController goForward];
			handled = true;
			break;
		
		case 'STOP':
			[[fController mainFrame] stopLoading];
			handled = true;
			break;
		
		case 'RELO':
			[[fController mainFrame] reload];
			handled = true;
			break;
		
		case 'VIEW':
			ViewSource();
			handled = true;
			break;
		
		case 'CHIS':
			[[WebHistory optionalSharedHistory] removeAllItems];
			handled = true;
			break;
		
                case kHICommandPageSetup :
                        {
                            HandlePageSetup();
                            handled = true;
                        }
                        break;
                        
                case kHICommandPrint :
                        {
                            HandlePrint();
                            handled = true;
                        }
                        break;
                
		default:
			handled = TWindow::HandleCommand( command );
			break;
	}
	
	return handled;
}

//-------------------------------------------------------------------------------------
//	TWebWindow::UpdateCommandStatus
//-------------------------------------------------------------------------------------
//	Update a command's status (for menu items).
//
Boolean
TWebWindow::UpdateCommandStatus( const HICommand& command )
{
	Boolean		handled = false;
	
	switch( command.commandID )
	{
		case 'VIEW':
			if ( CanViewSource() )
				EnableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			else
				DisableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			
			handled = true;
			break;
		
		case 'BACK':
			{
				WebBackForwardList* list = [fController backForwardList];
				if ( [list backItem] == NULL )
					DisableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
				else
					EnableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
				
				handled = true;
			}
			break;
			
		case 'FWD ':
			{
				WebBackForwardList* list = [fController backForwardList];
				if ( [list forwardItem] == NULL )
					DisableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
				else
					EnableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
				
				handled = true;
			}
			break;
			
		default:
			handled = TWindow::UpdateCommandStatus( command );
			break;
	}
	
	return handled;
}

//-------------------------------------------------------------------------------------
//	TWebWindow::HandleFrameLoad
//-------------------------------------------------------------------------------------
//	This is called while we are moving from one page to another. You can use a location
//	change delegate to stay in the loop as things are happening. For example, you can
//	watch for the page title and the page icon.
//
OSStatus
TWebWindow::HandleFrameLoad( TCarbonEvent& inEvent )
{
	OSStatus		result = eventNotHandledErr;
	
	switch ( inEvent.GetKind() )
	{
		WebDataSource*		dataSource;
		NSError*			error;
		CFStringRef			title;
		CGImageRef			icon;
		
		case kMyEventWebFrameLoadStarted:
			inEvent.GetParameter<WebDataSource*>( kParamWebDataSource, kTypeWebDataSource, &dataSource );
			FrameLoadStarted( dataSource );
			result = noErr;
			break;

		case kMyEventWebReceivedPageTitle:
			inEvent.GetParameter<CFStringRef>( kParamTitle, typeCFStringRef, &title );
			SetTitle( title );
			result = noErr;
			break;
		
		case kMyEventWebReceivedPageIcon:
			inEvent.GetParameter<CGImageRef>( kParamIcon, kTypeCGImageRef, &icon );
			SetPageIcon( icon );
			result = noErr;
			break;

		case kMyEventWebFrameLoadDone:
			inEvent.GetParameter<WebDataSource*>( kParamWebDataSource, kTypeWebDataSource, &dataSource );
			FrameLoadDone( NULL, dataSource );
			result = noErr;
			break;
                        
		case kMyEventWebFrameLoadFailed:
			inEvent.GetParameter<NSError*>( kParamWebError, kTypeWebError, &error );
			inEvent.GetParameter<WebDataSource*>( kParamWebDataSource, kTypeWebDataSource, &dataSource );
			FrameLoadDone( error, dataSource );
			result = noErr;
			break;
	}
	
	return result;
}

//-------------------------------------------------------------------------------------
//	TWebWindow::HandleResourceLoad
//-------------------------------------------------------------------------------------
//	You can install a resource load delegate to watch things as they load. You can use
//	this to drive a progress bar, or show some other type of status information to
//	entertain the user while the data is coming in, for example.
//
//	In our example here, we take a simplistic approach Ñ we merely maintain a count of
//	all the resources coming in. As we get new resources, we are asked to provide some
//	sort of identifier. We just give it a number, which we keep incrementing as we get
//	new pieces of data. I have pretty much all the hooks filled in below just to show
// 	them all, but we only use a few for this application. When the number finished plus
//	the number failed equals our total resource count, we consider ourselves done.
//
OSStatus
TWebWindow::HandleResourceLoad( TCarbonEvent& inEvent )
{
	OSStatus			result = eventNotHandledErr;
	CFTypeRef			identifier = NULL;
	UInt32				number = 0;
	NSURLResponse*		resp;
	UInt32				length;

	switch( inEvent.GetKind() )
	{
		case kMyEventWebResourceGetIdentifier:
			fResourceCount++;
			
			identifier = CFNumberCreate( NULL, kCFNumberSInt32Type, &fResourceCount );
			
			SetEventParameter( inEvent, kParamIdentifier, typeCFTypeRef,
				sizeof( CFTypeRef ), &identifier );
			
			#if TRACE_RESOURCE_LOAD	
			fprintf( stderr, "Started load of resource %lu\n", fResourceCount );
			#endif

			result = noErr;
			break;
		
		case kMyEventWebResourceDidReceiveResponse:
			GetEventParameter( inEvent, kParamIdentifier, typeCFTypeRef, NULL,
				sizeof( CFTypeRef ), NULL, &identifier );
			if ( identifier )
				CFNumberGetValue( (CFNumberRef)identifier, kCFNumberSInt32Type, &number );

			GetEventParameter( inEvent, kParamNSURLResponse, kTypeNSURLResponse, NULL,
				sizeof( NSURLResponse * ), NULL, &resp );
			
			#if TRACE_RESOURCE_LOAD	
			fprintf( stderr, "Received response for %lu: %p\n", number, resp );
			#endif
			
			result = noErr;
			break;
		
		case kMyEventWebResourceDidReceiveContentLength:
			GetEventParameter( inEvent, kParamIdentifier, typeCFTypeRef, NULL,
				sizeof( CFTypeRef ), NULL, &identifier );
			if ( identifier )
				CFNumberGetValue( (CFNumberRef)identifier, kCFNumberSInt32Type, &number );
			GetEventParameter( inEvent, kParamLength, typeUInt32, NULL,
				sizeof( UInt32 ), NULL, &length );
			
			#if TRACE_RESOURCE_LOAD	
			fprintf( stderr, "Received content length of resource %lu: %lu\n",
				number, length );
			#endif
			
			result = noErr;
			break;

		case kMyEventWebResourceLoadFinished:
			GetEventParameter( inEvent, kParamIdentifier, typeCFTypeRef, NULL,
				sizeof( CFTypeRef ), NULL, &identifier );
			CFNumberGetValue( (CFNumberRef)identifier, kCFNumberSInt32Type, &number );

			#if TRACE_RESOURCE_LOAD	
			fprintf( stderr, "Load finished for resource %lu\n", fResourceCount );
			#endif

			CFRelease( identifier );

			fResourceCompletedCount++;
			UpdateLoadStatus();
			result = noErr;
			break;
		
		case kMyEventWebResourceLoadFailed:
			GetEventParameter( inEvent, kParamIdentifier, typeCFTypeRef, NULL,
				sizeof( CFTypeRef ), NULL, &identifier );
			CFNumberGetValue( (CFNumberRef)identifier, kCFNumberSInt32Type, &number );

			#if TRACE_RESOURCE_LOAD	
			fprintf( stderr, "Load FAILED for resource %lu\n", fResourceCount );
			#endif

			CFRelease( identifier );

			fResourceFailedCount++;
			UpdateLoadStatus();
			result = noErr;
			break;
	}
	
	return result;
}


//-------------------------------------------------------------------------------------
//	TWebWindow::HandleWebUI
//-------------------------------------------------------------------------------------
//	This is a somewhat miscellaneous bunch of stuff. Basically, this handler deals with
//	window operation delegate events. These can be sent from javascript operations such
//	as setting the status text or querying the visibility of the status bar, as well as
//	other actions like bringing up a context menu to open a new browser window. You can
//	also listen here to find out when the mouse has rolled over a new element such as a
//	link so you can display some information about it.
//
OSStatus
TWebWindow::HandleWebUI( TCarbonEvent& inEvent )
{
	OSStatus		result = eventNotHandledErr;
	
	switch( inEvent.GetKind() )
	{
		case kMyEventWebSetStatusText:
			{
				CFStringRef	text;
				
				inEvent.GetParameter<CFStringRef>( kParamText, typeCFStringRef, &text );
				SetStatus( text );

				//fprintf( stderr, "Status text set\n" );
				result = noErr;
			}
			break;
		
		case kMyEventWebGetStatusText:
			{
				CFStringRef	text = CopyStatusText();
				
				if ( text )
					inEvent.SetParameter<CFStringRef>( kParamText, typeCFStringRef, text );

				//fprintf( stderr, "Status text retrieved\n" );
				result = noErr;
			}
			break;
		
		case kMyEventWebMouseMovedOverElement:
			{
				CFDictionaryRef		dict;
				
				inEvent.GetParameter<CFDictionaryRef>( kParamWebElementInfo, kTypeCFDictionaryRef, &dict );
				MouseMovedOverElement( dict );
				result = noErr;
			}
			break;
		
		case kMyEventWebIsStatusBarVisible:
			{
				Boolean 		kTrue = true;
				
				inEvent.SetParameter( kEventParamResult, kTrue );
				//fprintf( stderr, "IsStatusBarVisible handled\n" );
				result = noErr;
			}
			break;
		
		case kMyEventWebCreateWindow:
			{
				NSURLRequest*			request;
				WebView*			controller;
				
				inEvent.GetParameter<NSURLRequest*>(kParamNSURLRequest,
						kTypeNSURLRequest, &request );
				
				{
					TWebWindow*	window = new TWebWindow();
					window->LoadRequest( request );
					controller = window->fController;
				}
				
				inEvent.SetParameter<WebView*>(kParamWebView,
						kTypeWebView, controller );
				
				result = noErr;
			}
			break;
		
		case kMyEventWebShowWindow:
		case kMyEventWebShowWindowBehindFrontmost:
			// This example doesn't handle the 'behind frontmost' case differently because I'm lazy.
			if ( !IsVisible() )
				Show();
			
			result = noErr;
			break;
	}
	
	return result;
}


//-------------------------------------------------------------------------------------
//	TWebWindow::HandleEvent
//-------------------------------------------------------------------------------------
//	General handler. Branches out to other sub-handlers for clarity.
//
OSStatus
TWebWindow::HandleEvent( EventHandlerCallRef inCallRef, TCarbonEvent& inEvent )
{
	OSStatus 					result = eventNotHandledErr;

        // This is a workaround that's only needed for Panther and earlier.  It makes inline input in text fields in HIWebView work correctly.
        [NSApp setWindowsNeedUpdate:YES];
    
	switch ( inEvent.GetClass() )
	{
		case kMyEventClassWebFrameLoad:
			result = HandleFrameLoad( inEvent );
			break;
		
		case kMyEventClassWebResourceLoad:
			result = HandleResourceLoad( inEvent );
			break;
		
		case kMyEventClassWebUI:
			result = HandleWebUI( inEvent );
			break;
		
		case kEventClassWindow:
			switch( inEvent.GetKind() )
			{
				case kEventWindowDrawContent:
					{
						Rect		portRect;
						
						SetPortWindowPort( GetWindowRef() );
						GetPortBounds( GetWindowPort( GetWindowRef() ), &portRect );
						SetThemeBackground( kThemeBrushDialogBackgroundActive, 32, true );
						EraseRect( &portRect );
						
					}
					break;
			}
			break;
			
		case kEventClassMouse:
			switch( inEvent.GetKind() )
			{
				case kEventMouseMoved:
					{
						WindowRef		temp;
						Point			where;
						WindowPartCode	part;
						HIViewRef		view;
					
						GetEventParameter( inEvent, kEventParamMouseLocation, typeQDPoint, NULL,
								sizeof( Point ), NULL, &where );
								
						part = FindWindow( where, &temp );
						if ( temp == GetWindowRef() )
						{
							Rect		bounds;
							WindowRef	window = GetWindowRef();
							
							GetWindowBounds( window, kWindowStructureRgn, &bounds );
							where.h -= bounds.left;
							where.v -= bounds.top;
							SetEventParameter( inEvent, kEventParamWindowRef, typeWindowRef, sizeof( WindowRef ), &window );
		//					SetEventParameter( inEvent, kEventParamWindowPartCode, typeWindowPartCode, sizeof( WindowPartCode ), &part );
							SetEventParameter( inEvent, kEventParamWindowMouseLocation, typeQDPoint, sizeof( Point ), &where );
							
							HIViewGetViewForMouseEvent( HIViewGetRoot( window ), inEvent, &view );
						
                            if ( view == NULL)
                                result = noErr;
                            else
                                result = SendEventToEventTargetWithOptions( inEvent, HIObjectGetEventTarget( (HIObjectRef)view ),
										kEventTargetDontPropagate );
						}
					}
					break;
				
				case kEventMouseUp:
				case kEventMouseDragged:
				case kEventMouseWheelMoved:
					{
						HIViewRef view;

						HIViewGetViewForMouseEvent( HIViewGetRoot( GetWindowRef() ), inEvent, &view );
		
                        if ( view == NULL)
                            result = noErr;
                        else
                            result = SendEventToEventTargetWithOptions( inEvent, HIObjectGetEventTarget( (HIObjectRef)view ),
									kEventTargetDontPropagate );
					}
					break;
				
				default:
					result = TWindow::HandleEvent( inCallRef, inEvent );
					break;
			}
			break;

		default:
			result = TWindow::HandleEvent( inCallRef, inEvent );
			break;
	}

	return result;
}

//-------------------------------------------------------------------------------------
//	TWebWindow::ContentBoundsChanged
//-------------------------------------------------------------------------------------
//	A handler on the content view that looks for bounds changed. When so, we resize our
//	web view and others accordingly. Just busy work.
//
OSStatus
TWebWindow::ContentBoundsChanged( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	TCarbonEvent	event( inEvent );
	TWebWindow*		window = (TWebWindow*)inUserData;
	ControlRef		contentView;
	HIRect			frame, bounds, oldFrame;
	float			progressLeft;
		
	HIViewFindByID( HIViewGetRoot( window->GetWindowRef() ), kHIViewWindowContentID, &contentView );
	
	HIViewGetBounds( contentView, &bounds );
	HIViewGetFrame( window->fWebView, &frame );
	
	oldFrame = frame;
	
	frame.size.width = (CGRectGetMaxX( bounds ) - window->fBrowserRightGap) - frame.origin.x;
	frame.size.height = (CGRectGetMaxY( bounds ) - window->fBrowserBottomGap) - frame.origin.y;

//	printf( "TWebWindow: moving web view to (%g %g) (%g %g)\n", frame.origin.x, frame.origin.y,
//		frame.size.width, frame.size.height );
	if ( !window->fIsComposited )
	{
		if ( oldFrame.size.width > frame.size.width
			|| oldFrame.size.height > frame.size.height )
		{
			RgnHandle		region = NewRgn();
			HIShapeRef		oldShape, newShape, diff;
			
			oldShape = HIShapeCreateWithRect( &oldFrame );
			newShape = HIShapeCreateWithRect( &frame );
			diff = HIShapeCreateDifference( oldShape, newShape );
			HIShapeGetAsQDRgn( diff, region );
			InvalWindowRgn( window->GetWindowRef(), region );
			DisposeRgn( region );
			CFRelease( diff );
			CFRelease( newShape );
			CFRelease( oldShape );
		}
	}
	
	HIViewSetFrame( window->fWebView, &frame );
	
	HIViewGetFrame( window->fProgressBar, &frame );
	frame.origin.x = (CGRectGetMaxX( bounds ) - window->fProgressRightGap) - frame.size.width;
	frame.origin.y = (CGRectGetMaxY( bounds ) - window->fProgressBottomGap) - frame.size.height;
	HIViewSetFrame( window->fProgressBar, &frame );

	progressLeft = frame.origin.x;

	HIViewGetFrame( window->fStatusText, &frame );
	frame.origin.y = (CGRectGetMaxY( bounds ) - window->fStatusBottomGap) - frame.size.height;
	frame.size.width = (progressLeft - window->fStatusProgressGap) - frame.origin.x;
	HIViewSetFrame( window->fStatusText, &frame );

	return noErr;
}

//-------------------------------------------------------------------------------------
//	IsCommandSpace
//-------------------------------------------------------------------------------------
//	This is a workaround that's only needed for Panther and earlier. It allows command-space to toggle input methods.
//
static bool IsCommandSpace( EventRef inEvent ) {
    UInt32		modifiers;
    bool		result  = false;
    
    GetEventParameter( inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof( UInt32 ), NULL, &modifiers );
    if( modifiers & cmdKey )
    {
        char	ch;
        GetEventParameter( inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof( char ), NULL, &ch );
        if( ch == ' ' ) result  = true;
    }
    return result;
}

//-------------------------------------------------------------------------------------
//	IsKanaOrEisu
//-------------------------------------------------------------------------------------
//	This is a workaround that's only needed for Panther. It allows command-space, kana and eisu to toggle input methods.
//
static bool IsKanaOrEisu(EventRef inEvent)
{
    UInt32 keyCode;
    GetEventParameter(inEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyCode), NULL, &keyCode);
    return (keyCode == 0x66 || keyCode == 0x68);
}

//-------------------------------------------------------------------------------------
//	RawKeyInputHandler
//-------------------------------------------------------------------------------------
//	This is a workaround that's only needed for Panther. It allows command-space, kana and eisu to toggle input methods.
//
static OSStatus RawKeyInputHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData)
{
    OSStatus result = eventNotHandledErr;
    if (IsCommandSpace(inEvent) || IsKanaOrEisu(inEvent)) {
        result  = eventPassToNextTargetErr;
    }
    return result;
}


//-------------------------------------------------------------------------------------
//	TWebWindow::TextInputHandler
//-------------------------------------------------------------------------------------
//	A bunch of code on our edit field to get the key chars so we can ultimately look for
//	return and enter.
//
OSStatus
TWebWindow::TextInputHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	bool		handled = false;
	EventRef	rawKey;
	UInt32		modifiers;
	ByteCount	bufferSize;
	UniChar		singleCharacter;
	UniChar*	charBuffer = &singleCharacter;
	UInt32		charCount;
	UInt32		charIndex;
	TWebWindow*	window = (TWebWindow*)inUserData;
	ControlRef	control = window->fTextField;
	ControlRef	focus;
	OSStatus	result = eventNotHandledErr;

        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        
	GetKeyboardFocus( window->GetWindowRef(), &focus );
	
	if( focus == control	// don't propagate text input events to superview
		&& IsControlActive( control )
		&& IsControlEnabled( control )
		&& IsControlVisible( control ) )
	{
		// modifiers are extracted from the raw key event inside this event
		modifiers = 0;
		if ( GetEventParameter( inEvent, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof( rawKey ), NULL, &rawKey ) == noErr )
			GetEventParameter( rawKey, kEventParamKeyModifiers, typeUInt32, NULL, sizeof( UInt32 ), NULL, &modifiers );
		
		// see how many characters there are in this event
		bufferSize = 0;
		GetEventParameter( inEvent, kEventParamTextInputSendText, typeUnicodeText, NULL, 0, &bufferSize, NULL );
		charCount = bufferSize / sizeof( singleCharacter );
		require_quiet( charCount > 0, TextInputNoCharacters );
		
		if ( charCount > 1 )
			charBuffer = (UniChar*)malloc( bufferSize );
		require_quiet( charBuffer != NULL, TextInputBufferAllocationFailed );
		
		result = GetEventParameter( inEvent, kEventParamTextInputSendText, typeUnicodeText, NULL, bufferSize, NULL, charBuffer );
		require_noerr( result, TextInputCantGetCharacters );
		
		// we have officially handled the event if any one character is handled by the view
		for ( charIndex = 0; charIndex < charCount; charIndex++ )
			handled = handled || window->AcceptTextInput( control, charBuffer[charIndex], modifiers, inEvent );
		
	TextInputCantGetCharacters:
	
		if ( charBuffer != &singleCharacter )
			free( (void*)charBuffer );
		
	TextInputBufferAllocationFailed:
	TextInputNoCharacters:
		
		if ( handled )
			result = noErr;
		else
			result = eventNotHandledErr;
	}
	
        [pool release];
        
	return result;
}

//-------------------------------------------------------------------------------------
//	TWebWindow::AcceptTextInput
//-------------------------------------------------------------------------------------
//	OK. Some text input has occurred in our edit field. If it's return or enter. Use
//	what's typed in as a URL. We just send a LINK command from here to kick off the
//	process.
//
Boolean
TWebWindow::AcceptTextInput( ControlRef inControl, UniChar inChar, UInt32 modifiers, EventRef inEvent )
{
	Boolean		handled = false;

	if ( inChar == 0x0d || inChar == 0x0E )
	{
		HICommand		command;
		
		ClearKeyboardFocus( GetControlOwner( inControl ) );

		command.attributes = 0;
		command.commandID = 'LINK';
		ProcessHICommand( &command );
		handled = true;
	}
	
	return handled;
}

//-------------------------------------------------------------------------------------
//	TWebWindow::FrameLoadStarted
//-------------------------------------------------------------------------------------
//	A page load has started. Display something useful in the status text, and start
//	showing the progress bar. Switch the stop/reload buttons to stop.
//
void
TWebWindow::FrameLoadStarted( WebDataSource* dataSource )
{
	NSURLRequest*		request = [dataSource request];

        if ([[[dataSource webFrame] webView] mainFrame] == [dataSource webFrame]){
            SetTitle( (CFStringRef)[[request URL] absoluteString] );
            SetTextField( (CFStringRef)[[request URL] absoluteString] );
            SetStatus( CFSTR( "Loading..." ) );
            HIViewSetVisible( fProgressBar, true );
            
            EnableControl( fStopButton ); // we start out disabled, so enable it
            HIViewSetVisible( fStopButton, true );
            HIViewSetVisible( fReloadButton, false );
        }
}

//-------------------------------------------------------------------------------------
//	TWebWindow::ReceivedPageTitle
//-------------------------------------------------------------------------------------
//	I'm not quite sure why I didn't call SetTitle directly from our event handler, but
//	here we've received the page title during a page load.
//
void
TWebWindow::ReceivedPageTitle( CFStringRef title, WebDataSource* dataSource )
{
	SetTitle( title );
}

//-------------------------------------------------------------------------------------
//	TWebWindow::FrameLoadDone
//-------------------------------------------------------------------------------------
// 	We're done loading a page. Hide the progress bar, clear our status, switch from the
//	stop to the reload button, and update our back/forward buttons.
//
void
TWebWindow::FrameLoadDone( NSError* error, WebDataSource* dataSource )
{
	WebBackForwardList*		list;
		
	HIViewSetVisible( fProgressBar, false );
	HIViewSetVisible( fStopButton, false );
	HIViewSetVisible( fReloadButton, true );
	EnableControl( fReloadButton );
	
	list = [fController backForwardList];
	if ( [list backItem] == NULL )
		DisableControl( fBackButton );
	else
		EnableControl( fBackButton );

	if ( [list forwardItem] == NULL )
		DisableControl( fForwardButton );
	else
		EnableControl( fForwardButton );
	
        if ( error == NULL ) {
            SetStatus( CFSTR( "" ) );
        } else {
            SetTitle( CFSTR( "" ) );
            SetStatus( (CFStringRef)[(NSError*)error localizedDescription] );
                
            DialogRef alert = 0;
            CreateStandardSheet(kAlertStopAlert,
                                CFSTR("Failed to load page"),
                                (CFStringRef)[(NSError*)error localizedDescription],
                                NULL,
                                GetWindowEventTarget(GetWindowRef()),
                                &alert);
            ShowSheetWindow(GetDialogWindow(alert), GetWindowRef());
        }
}


void TWebWindow::HandlePageSetup()
{
    static PMSheetDoneUPP gPageSetupDone = NewPMSheetDoneUPP(PageSetupSheetDone);
    
    /* if there's already a print session in progress, then exit */
    if(NULL != fPrintSession) {
        return;
    }

    require_noerr(::PMCreateSession(&fPrintSession), couldntCreateSession);
    ::PMSessionUseSheets(fPrintSession, GetWindowRef(), gPageSetupDone);
    
    if(NULL == fPageFormat) {
        require_noerr(::PMCreatePageFormat(&fPageFormat), couldntCreatePageFormat);
        require_noerr(::PMSessionDefaultPageFormat(fPrintSession, fPageFormat), couldntCreatePageFormat);
    } else {
        Boolean wasResetToDefaults;
        require_noerr(::PMSessionValidatePageFormat(fPrintSession, fPageFormat, &wasResetToDefaults), couldntCreatePageFormat);
    }
    
    Boolean ignoredReturn;	// accepted is ignored when using sheets.
    require_noerr(::PMSessionPageSetupDialog(fPrintSession, fPageFormat, &ignoredReturn), runDialogFailed);

    return;
    
runDialogFailed:
couldntCreatePageFormat:
couldntCreateSession:
    
    DialogRef errorSheet = 0;
    CreateStandardSheet(
        kAlertStopAlert,
        CFSTR("Page Setup Failed"),
        CFSTR("An error occured when trying to bring up the page setup dialog"),
        NULL,
        GetWindowEventTarget(GetWindowRef()),
        &errorSheet);

    ShowSheetWindow(GetDialogWindow(errorSheet), GetWindowRef());
}

void TWebWindow::HandlePrint()
{
    static PMSheetDoneUPP gPrintDialogDone = NewPMSheetDoneUPP(PrintingSheetDone);

    /* if there's already a print session in progress, then exit */
    if(NULL != fPrintSession) {
        return;
    }
    
    require_noerr(::PMCreateSession(&fPrintSession), couldntCreateSession);
    ::PMSessionUseSheets(fPrintSession, GetWindowRef(), gPrintDialogDone);

    // Validate the page format
    if(NULL == fPageFormat) {
        require_noerr(::PMCreatePageFormat(&fPageFormat), couldntCreatePageFormat);
        require_noerr(::PMSessionDefaultPageFormat(fPrintSession, fPageFormat), couldntCreatePageFormat);
    } else {
        Boolean wasResetToDefaults;
        require_noerr(::PMSessionValidatePageFormat(fPrintSession, fPageFormat, &wasResetToDefaults), couldntCreatePageFormat);
    }
    
    if(NULL == fPrintSettings) {
        require_noerr(::PMCreatePrintSettings(&fPrintSettings), couldntCreatePrintSettings);
        require_noerr(::PMSessionDefaultPrintSettings(fPrintSession, fPrintSettings), couldntCreatePrintSettings);
    } else {
        Boolean wasResetToDefaults;
        require_noerr(::PMSessionValidatePrintSettings(fPrintSession, fPrintSettings, &wasResetToDefaults), couldntCreatePrintSettings);
    }

    Boolean ignoredValue;
    require_noerr(::PMSessionPrintDialog(fPrintSession, fPrintSettings, fPageFormat, &ignoredValue), runDialogFailed);

    return;
    
runDialogFailed :
couldntCreatePrintSettings :
couldntCreatePageFormat : 
couldntCreateSession:

    DialogRef errorSheet = 0;
    CreateStandardSheet(
        kAlertStopAlert,
        CFSTR("Print Failed"),
        CFSTR("An error occured when trying to print the document"),
        NULL,
        GetWindowEventTarget(GetWindowRef()),
        &errorSheet);

    ShowSheetWindow(GetDialogWindow(errorSheet), GetWindowRef());
}


void TWebWindow::PageSetupDone(Boolean accepted)
{
    PMRelease(fPrintSession);
    fPrintSession = NULL;
}

// Work-around to address problems with printing a Cocoa WebView from carbon.
@interface NSView (PrivateStuff)
- (void)setPageWidthForPrinting:(float)pageWidth;
- (void)_setPrinting:(BOOL)printing minimumPageWidth:(float)minPageWidth maximumPageWidth:(float)maxPageWidth adjustViewSize:(BOOL)adjustViewSize;
- (void)_setPrinting:(BOOL)printing pageWidth:(float)pageWidth;
@end

static void preparePageForPrinting (NSView <WebDocumentView> *view, float pageWidth)
{
    if ([view respondsToSelector:@selector(setPageWidthForPrinting:)]) {
        NSLog (@"%s:  pageWidth = %f", __FUNCTION__, pageWidth);
        [view setPageWidthForPrinting:pageWidth];
    }
    else if ([view respondsToSelector:@selector(_setPrinting:minimumPageWidth:maximumPageWidth:adjustViewSize:)]) {
        [view _setPrinting:NO minimumPageWidth:0. maximumPageWidth:0. adjustViewSize:NO];
        [view _setPrinting:YES minimumPageWidth:pageWidth maximumPageWidth:pageWidth adjustViewSize:YES];
    }
    else if ([view respondsToSelector:@selector(_setPrinting:pageWidth:)]) {
        [view _setPrinting:NO pageWidth:0.];
        [view _setPrinting:YES pageWidth:pageWidth];
    }
}

void TWebWindow::PrintDialogDone(Boolean accepted)
{
    if(accepted) {
        NSRange pageRange;
        UInt32 firstPage, lastPage;
        OSStatus	printErr;
		
        check(NULL != fController);
        NSView <WebDocumentView> *webDocumentView = [[[fController mainFrame] frameView] documentView];
        
        if(NULL != webDocumentView) {
            PMRect pageBounds, paperBounds;
             			
            PMGetAdjustedPageRect (fPageFormat, &pageBounds);
            float carbonPageWidth = pageBounds.right - pageBounds.left;
            float carbonPageHeight = pageBounds.bottom - pageBounds.top;

            PMGetAdjustedPaperRect (fPageFormat, &paperBounds);
            float carbonPaperWidth = paperBounds.right - paperBounds.left;
            float carbonPaperHeight = paperBounds.bottom - paperBounds.top;

            CGRect destinationRect;
            destinationRect.size.height = carbonPageHeight;
            destinationRect.size.width = carbonPageWidth;
            destinationRect.origin.x = pageBounds.left;
            destinationRect.origin.y = pageBounds.top;

            double paperToPageYOffset = carbonPaperHeight + paperBounds.top - carbonPageHeight;

            // Compute the page range and rectangles using a Cocoa NSPrintOperation.  This
            // operation is just used temporarily to compute the needed values.  Later,
            // we print to PDF using dataWithPDFInsideRect:, which sets up it's own
            // print operation.
            NSMutableData *data = [NSMutableData dataWithCapacity:4096];   // Not actually used.
            NSPrintOperation *operation = [NSPrintOperation PDFOperationWithView:webDocumentView insideRect:[webDocumentView bounds] toData:data];
            [[operation printInfo] setPaperSize:NSMakeSize(carbonPaperWidth, carbonPaperHeight)];
            [NSPrintOperation setCurrentOperation:operation];
            
            // knowsPageRange: requires the print operation set above.
            [webDocumentView knowsPageRange: &pageRange];
            NSLog (@"%s:  pageRange.location = %d, pageRange.length = %d", __FUNCTION__, (int)pageRange.location, (int)pageRange.length);
            
            // Now get the page rectangle for each page.
            NSMutableArray *pageRects = [[NSMutableArray arrayWithCapacity:pageRange.length] retain];
            int i;
            float maxSourceHeight = 0;
            for (i = 1; i <= (int)pageRange.length; i++) {
                NSRect aPageRect = [webDocumentView rectForPage:i];
                [pageRects addObject:[NSValue valueWithRect:aPageRect]];
                if (aPageRect.size.height > maxSourceHeight)
                    maxSourceHeight = aPageRect.size.height;
            }

            // Reset the print operation so dataWithPDFInsideRect: will succeed.
            [NSPrintOperation setCurrentOperation:nil];
            
            PMGetPageRange(fPrintSettings, &firstPage, &lastPage);
            
            if(firstPage < 1)
                firstPage = 1;
                
            if(lastPage > pageRange.length)
                lastPage = pageRange.length;

            const CFStringRef contextType = kPMGraphicsContextCoreGraphics;
            CFArrayRef contextTypesArray = CFArrayCreate(NULL, (const void **) &contextType, 1, &kCFTypeArrayCallBacks);
            if( contextTypesArray != NULL )
			{
				verify_noerr(PMSessionSetDocumentFormatGeneration(fPrintSession, kPMDocumentFormatPDF, contextTypesArray, NULL));
				CFRelease(contextTypesArray);
			}
            
            // Begin the print loop
            printErr   = PMSessionBeginDocument(fPrintSession, fPrintSettings, fPageFormat);

            NSRect lastPageRect = [[pageRects objectAtIndex: (lastPage-1)] rectValue];

            float scale = (pageBounds.bottom - pageBounds.top) / maxSourceHeight;
            
            for(UInt32 pageCtr = firstPage; ((pageCtr <= lastPage) && (PMSessionError(fPrintSession) == noErr)); pageCtr++)
			{
                NSRect sourceRect;
                
                sourceRect = [[pageRects objectAtIndex: (pageCtr-1)] rectValue];
                
                preparePageForPrinting (webDocumentView, lastPageRect.size.width);
                    
            NSLog (@"%s:  sourceRect.origin.x = %f, sourceRect.origin.x = %f, sourceRect.size.width = %f, sourceRect.size.height = %f", 
                        __FUNCTION__, 
                        sourceRect.origin.x, sourceRect.origin.y,
                        sourceRect.size.width, sourceRect.size.height);
                CFDataRef pageData = (CFDataRef) [webDocumentView dataWithPDFInsideRect: sourceRect];
                CFRetain(pageData);
                
                // Create a CGPDFDocument from the page data.
                CGDataProviderRef dataProvider = CGDataProviderCreateWithData(NULL, CFDataGetBytePtr(pageData), CFDataGetLength(pageData), NULL);
                CGPDFDocumentRef myDocument = CGPDFDocumentCreateWithProvider(dataProvider);
                
                if(NULL != myDocument) {
                    CGContextRef pageContext;

                    // If we're printing a partial page scale and adjust y accordingly.
                    if (sourceRect.size.height < maxSourceHeight) {
                        destinationRect.size.height = sourceRect.size.height * scale;
                        destinationRect.origin.y = carbonPageHeight - destinationRect.size.height;
                    }
                    else {
                        destinationRect.size.height = carbonPageHeight;
                        destinationRect.origin.y = pageBounds.top;
                    }

                    printErr = PMSessionBeginPage(fPrintSession, fPageFormat, NULL );
					
                    if( printErr == noErr )
					{
						PMSessionGetGraphicsContext(fPrintSession, kPMGraphicsContextCoreGraphics, (void **) &pageContext);

						CGContextSaveGState( pageContext );

						CGContextTranslateCTM(pageContext, -paperBounds.left, paperToPageYOffset );

						CGContextDrawPDFDocument(pageContext, destinationRect, myDocument, 1);

						CGContextRestoreGState( pageContext );

						PMSessionEndPage(fPrintSession);

						CGPDFDocumentRelease(myDocument);
					}
                }

                CGDataProviderRelease(dataProvider);                
                CFRelease(pageData);
            } // end-for pageCtr
            
            [pageRects release];
            
            PMSessionEndDocument(fPrintSession);
        }
	}
    
    PMRelease(fPrintSession);
    fPrintSession = NULL;
}

void TWebWindow::PageSetupSheetDone(PMPrintSession printSession, WindowRef documentWindow, Boolean accepted)
{
    TWebWindow *webWindowDone = TWebWindow::GetFromWindowRef(documentWindow);
    if(NULL != webWindowDone) {
        check(printSession == webWindowDone->fPrintSession);
        webWindowDone->PageSetupDone(accepted);
    }
}

void TWebWindow::PrintingSheetDone(PMPrintSession printSession, WindowRef documentWindow, Boolean accepted)
{
    TWebWindow *webWindowDone = TWebWindow::GetFromWindowRef(documentWindow);
    if(NULL != webWindowDone) {
        check(printSession == webWindowDone->fPrintSession);
        webWindowDone->PrintDialogDone(accepted);
    }
}
