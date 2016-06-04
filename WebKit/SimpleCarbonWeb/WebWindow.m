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

#include "WebWindow.h"

static const EventTypeSpec kWindowEvents[] = {
	{ kEventClassMouse, kEventMouseMoved },
	{ kEventClassMouse, kEventMouseUp },
	{ kEventClassMouse, kEventMouseDragged },
	{ kEventClassMouse, kEventMouseWheelMoved }
};

static const EventTypeSpec kBoundsChangedEvent = {
	kEventClassControl, kEventControlBoundsChanged
};

static void		LoadURL( HIViewRef inView, CFURLRef inURL );
static OSStatus	ContentBoundsChanged( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );

//-------------------------------------------------------------------------------------
//	OpenWebWindow
//-------------------------------------------------------------------------------------
//	Open a new window and display a web page.
//
void
OpenWebWindow( CFStringRef inString )
{
	CFURLRef		url;
	WindowRef		window;
	OSStatus		err;
	IBNibRef		nibRef;
	HIViewRef		webView, contentView, root;
	HIRect			bounds;
	
	url = CFURLCreateWithString( NULL, inString, NULL );
	require( url != NULL, CantCreateURL );

	err = CreateNibReference( CFSTR( "main" ), &nibRef );
	require_noerr( err, CantOpenNIB );

	err = CreateWindowFromNib( nibRef, CFSTR( "MainWindow" ), &window );
	require_noerr_action( err, CantCreateWindow, DisposeNibReference( nibRef ) );

	DisposeNibReference( nibRef );

	HIWebViewCreate( &webView );
	
	HIViewFindByID( HIViewGetRoot( window ), kHIViewWindowContentID, &contentView );
	HIViewGetBounds( contentView, &bounds );
	HIViewSetFrame( webView, &bounds );
	
	// If you are using a non-composited window, you embed in the traditional
	// root control, gotten via GetRootControl. Else you would just embed in the
	// content view we already fetched above. Currently, this example is not using
	// a composited window.
	
	GetRootControl( window, &root );
	HIViewAddSubview( root, webView );
	HIViewSetVisible( webView, true );
	
	LoadURL( webView, url );

	// To deal with window resize, we install a handler on the content view. This is
	// the absolute best way to do resize. If you are using a composited window, it
	// becomes a requirement due to the way the HIView/WebKit glue works. In general,
	// you want to do this in composited mode anyway because you avoid a double-draw,
	// so it's a good practice to get into. This current example does not use a
	// composited window, fyi.

	InstallControlEventHandler( contentView, ContentBoundsChanged, 1,
			&kBoundsChangedEvent, webView, NULL );

	ShowWindow( window );

CantCreateWindow:
CantOpenNIB:
	CFRelease( url );

CantCreateURL:
	return;
}

//-------------------------------------------------------------------------------------
//	LoadURL
//-------------------------------------------------------------------------------------
//	Tell the web view to load a request for a URL. This is the only piece of Obj-C in
//	this example.
//
static void
LoadURL( HIViewRef inView, CFURLRef inURL )
{
	WebView*		nativeView;
    NSURLRequest*	request;
    WebFrame* 		mainFrame;

	nativeView = HIWebViewGetWebView( inView );
	request = [NSURLRequest requestWithURL:(NSURL*)inURL];
	mainFrame = [nativeView mainFrame];
	[mainFrame loadRequest:request];
}

//-------------------------------------------------------------------------------------
//	ContentBoundsChanged
//-------------------------------------------------------------------------------------
//	A handler on the content view that looks for bounds changed. When so, we resize our
//	web view accordingly. Just busy work. Panther introduces the concept of HILayout,
//	which would eliminate the need for this function.
//
static OSStatus
ContentBoundsChanged( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	HIViewRef		webView = (HIViewRef)inUserData;
	ControlRef		contentView;
	HIRect			bounds;
	WindowRef		window = GetControlOwner( webView );
	
	HIViewFindByID( HIViewGetRoot( window ), kHIViewWindowContentID, &contentView );
	
	HIViewGetBounds( contentView, &bounds );
	HIViewSetFrame( webView, &bounds );

	return noErr;
}
