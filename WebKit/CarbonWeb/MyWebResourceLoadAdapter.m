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

#include "MyWebResourceLoadAdapter.h"

static Boolean 	PrepareResourceLoadEvent( UInt32 inKind, WebDataSource* inSource, EventRef* outEvent );
static void 	FinishResourceLoadEvent( EventRef inEvent, HIObjectRef inObject );

@implementation MyWebResourceLoadAdapter

- initWithHIObject: (HIObjectRef)inObject
{
    self = [super init];
	if ( self )
	{
		_object = inObject;	// non retained
	}
    return self;
}

- (HIObjectRef)hiobject
{
	return _object;
}

- webView:(WebView*)wv identifierForInitialRequest:(NSURLRequest *)request fromDataSource: (WebDataSource *)dataSource
{
	EventRef	event;
	id			result = NULL;
	
	if ( PrepareResourceLoadEvent( kMyEventWebResourceGetIdentifier, dataSource, &event ) )
	{
		SetEventParameter( event, kParamNSURLRequest, kTypeNSURLRequest, sizeof( NSURLRequest * ), &request );
		SendEventToEventTargetWithOptions( event, HIObjectGetEventTarget( _object ), kEventTargetDontPropagate );
		GetEventParameter( event, kParamIdentifier, typeCFTypeRef, NULL, sizeof( CFTypeRef ), NULL, &result );

		ReleaseEvent( event );
	}
	
	return result;
}

-(void)webView: (WebView *)wv resource:identifier didReceiveResponse: (NSURLResponse *)response fromDataSource:(WebDataSource *)dataSource
{
	EventRef	event;

	if ( PrepareResourceLoadEvent( kMyEventWebResourceDidReceiveResponse, dataSource, &event ) )
	{
		SetEventParameter( event, kParamIdentifier, typeCFTypeRef, sizeof( CFTypeRef ), &identifier );
		SetEventParameter( event, kParamNSURLResponse, kTypeNSURLResponse, sizeof( NSURLResponse * ), &response );
		FinishResourceLoadEvent( event, _object );
	}
}

-(void)webView: (WebView *)wv resource:identifier didReceiveContentLength: (unsigned)length fromDataSource:(WebDataSource *)dataSource
{
	EventRef	event;
	
	if ( PrepareResourceLoadEvent( kMyEventWebResourceDidReceiveContentLength, dataSource, &event ) )
	{
		SetEventParameter( event, kParamIdentifier, typeCFTypeRef, sizeof( CFTypeRef ), &identifier );
		SetEventParameter( event, kParamLength, typeUInt32, sizeof( UInt32 ), &length );
		FinishResourceLoadEvent( event, _object );
	}
}

-(void)webView: (WebView *)wv resource:identifier didFinishLoadingFromDataSource:(WebDataSource *)dataSource
{
	EventRef	event;
	
	if ( PrepareResourceLoadEvent( kMyEventWebResourceLoadFinished, dataSource, &event ) )
	{
		SetEventParameter( event, kParamIdentifier, typeCFTypeRef, sizeof( CFTypeRef ), &identifier );
		FinishResourceLoadEvent( event, _object );
	}
}

-(void)webView: (WebView *)wv resource:identifier didFailLoadingWithError:(NSError *)error fromDataSource:(WebDataSource *)dataSource
{
	EventRef	event;
	
	if ( PrepareResourceLoadEvent( kMyEventWebResourceLoadFailed, dataSource, &event ) )
	{
		SetEventParameter( event, kParamIdentifier, typeCFTypeRef, sizeof( CFTypeRef ), &identifier );
		SetEventParameter( event, kParamWebError, kTypeWebError, sizeof( NSError * ), &error );
		FinishResourceLoadEvent( event, _object );
	}
}

- (void)webView: (WebView *)wv pluginFailedWithError:(NSError *)error dataSource:(WebDataSource *)dataSource
{
	EventRef	event;
	
	if ( PrepareResourceLoadEvent( kMyEventWebPluginFailed, dataSource, &event ) )
	{
		SetEventParameter( event, kParamWebPluginError, kTypeWebPluginError, sizeof( NSError * ), &error );
		FinishResourceLoadEvent( event, _object );
	}
}

@end

static Boolean
PrepareResourceLoadEvent(
	UInt32 			inKind,
	WebDataSource* 	inSource,
	EventRef* 		outEvent )
{
	EventRef 	event;
	OSStatus	err;
	Boolean		result = false;
	
	err = CreateEvent( NULL, kMyEventClassWebResourceLoad, inKind,
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
FinishResourceLoadEvent(
	EventRef 		inEvent,
	HIObjectRef 	inObject )
{
	SendEventToEventTargetWithOptions( inEvent, HIObjectGetEventTarget( inObject ),
			kEventTargetDontPropagate | kEventTargetSendToAllHandlers );
	ReleaseEvent( inEvent );
}
