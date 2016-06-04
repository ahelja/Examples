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

#include "MyFrameLoadAdapter.h"

static Boolean PrepareFrameLoadEvent( UInt32 inKind, WebDataSource* inSource, EventRef* outEvent );
static void FinishFrameLoadEvent( EventRef inEvent, HIObjectRef inObject );

@implementation MyFrameLoadAdapter

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

- (void)webView: (WebView *)wv didStartProvisionalLoadForFrame:(WebFrame *)frame
{
	EventRef	event;

	if ( PrepareFrameLoadEvent( kMyEventWebFrameLoadStarted, [frame provisionalDataSource], &event ) )
		FinishFrameLoadEvent( event, _object );
}

- (void)webView: (WebView *)wv didReceiveServerRedirectForProvisionalLoadForFrame:(WebFrame *)frame
{
	EventRef	event;

	if ( PrepareFrameLoadEvent( kMyEventWebServerRedirected, [frame provisionalDataSource], &event ) )
		FinishFrameLoadEvent( event, _object );
}

- (void)webView: (WebView *)wv didCommitLoadForFrame:(WebFrame *)frame
{
	EventRef	event;

	if ( PrepareFrameLoadEvent( kMyEventWebFrameLoadCommitted, [frame dataSource], &event ) )
		FinishFrameLoadEvent( event, _object );
}

- (void)webView: (WebView *)wv didReceiveTitle:(NSString *)title forFrame:(WebFrame *)frame
{
	EventRef	event;

	if ( PrepareFrameLoadEvent( kMyEventWebReceivedPageTitle, [frame dataSource], &event ) )
	{
		SetEventParameter( event, kParamTitle, typeCFStringRef,
				sizeof( CFStringRef ), &title );
		FinishFrameLoadEvent( event, _object );
	}
}

- (void)webView: (WebView *)wv didReceiveIcon:(NSImage *)image forFrame:(WebFrame *)frame
{
	EventRef	event;

	if ( image && PrepareFrameLoadEvent( kMyEventWebReceivedPageIcon, [frame dataSource], &event ) )
	{
		CGImageRef		icon = WebConvertNSImageToCGImageRef( image );

		if ( icon )
		{
			SetEventParameter( event, kParamIcon, kTypeCGImageRef,
					sizeof( CGImageRef ), &icon );
			FinishFrameLoadEvent( event, _object );
			CGImageRelease( icon );
		}
		else
		{
			ReleaseEvent( event );
		}
	}
}

- (void)webView: (WebView *)wv didFinishLoadForFrame:(WebFrame *)frame
{
	EventRef	event;

	if ( PrepareFrameLoadEvent( kMyEventWebFrameLoadDone, [frame dataSource], &event ) )
	{
		FinishFrameLoadEvent( event, _object );
	}
}


- (void)webView: (WebView *)wv didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
	EventRef	event;

	if ( PrepareFrameLoadEvent( kMyEventWebFrameLoadFailed, [frame dataSource], &event ) )
	{
		SetEventParameter( event, kParamWebError, kTypeWebError, sizeof(NSError * ), &error );
		FinishFrameLoadEvent( event, _object );
	}
}


- (void)webView: (WebView *)wv didFailProvisionalLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
	EventRef	event;

	if ( PrepareFrameLoadEvent( kMyEventWebFrameLoadFailed, [frame provisionalDataSource], &event ) )
	{
		SetEventParameter( event, kParamWebError, kTypeWebError, sizeof(NSError * ), &error );
		FinishFrameLoadEvent( event, _object );
	}
}


- (void)webView: (WebView *)wv didChangeLocationWithinPageForFrame:(WebFrame *)frame
{
	EventRef	event;

	if ( PrepareFrameLoadEvent( kMyEventWebLocationChangedWithinPage, [frame dataSource], &event ) )
		FinishFrameLoadEvent( event, _object );
}

- (void)webView: (WebView *)wv willPerformClientRedirectToURL:(NSURL *)URL delay:(NSTimeInterval)seconds fireDate:(NSDate *)date forFrame:(WebFrame *)frame
{
	EventRef	event;

	if ( PrepareFrameLoadEvent( kMyEventWebClientWillRedirect, NULL, &event ) )
	{
		SetEventParameter( event, kParamURL, kTypeCFURLRef, sizeof( CFURLRef ), &URL );
		SetEventParameter( event, kParamDelay, typeFloat, sizeof( float ), &seconds );
		SetEventParameter( event, kParamDate, kTypeCFDateRef, sizeof( CFDateRef ), &date );
		SetEventParameter( event, kParamWebFrame, kTypeWebFrame, sizeof( WebFrame* ), &frame );
		
		FinishFrameLoadEvent( event, _object );
	}
}

- (void)webView: (WebView *)webView didCancelClientRedirectForFrame:(WebFrame *)frame;
{
	EventRef	event;

	if ( PrepareFrameLoadEvent( kMyEventWebClientRedirectCancelled, NULL, &event ) )
	{
		SetEventParameter( event, kParamWebFrame, kTypeWebFrame, sizeof( WebFrame* ), &frame );
		FinishFrameLoadEvent( event, _object );
	}
}

@end

static Boolean
PrepareFrameLoadEvent(
	UInt32 			inKind,
	WebDataSource* 	inSource,
	EventRef* 		outEvent )
{
	EventRef 	event;
	OSStatus	err;
	Boolean		result = false;
	
	err = CreateEvent( NULL, kMyEventClassWebFrameLoad, inKind,
				GetCurrentEventTime(), 0, &event );
	if ( err == noErr )
	{
		if ( inSource )
		{
			SetEventParameter( event, kParamWebDataSource, kTypeWebDataSource,
					sizeof( WebDataSource * ), &inSource );
		}
		
		*outEvent = event;
		result = true;
	}

	return result;
}

static void
FinishFrameLoadEvent(
	EventRef 		inEvent,
	HIObjectRef 	inObject )
{
	SendEventToEventTargetWithOptions( inEvent, HIObjectGetEventTarget( inObject ),
			kEventTargetDontPropagate | kEventTargetSendToAllHandlers );
	ReleaseEvent( inEvent );
}

