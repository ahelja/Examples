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
#include "TWebView.h"

#include "CarbonWindowAdapter.h"

//#include <AppKit/NSCarbonWindow.h>
#include <AppKit/NSGraphicsContextPrivate.h>
#include <CoreGraphics/CGSEvent.h>
#include <WebKit/WebKit.h>
#include <HIToolbox/CarbonEventsPriv.h>
#include "HIViewAdapter.h"



WebController*
CreateWebControllerWithHIView( HIViewRef inView, WebDataSource* dataSource, CFStringRef inName )
{
	TWebView* 	view = (TWebView*)HIObjectDynamicCast( (HIObjectRef)inView, kHIWebViewClassID );
	WebController*	result = NULL;
	
	if ( view )
		result = [[WebController alloc] initWithView: view->GetNSView() controllerSetName:(NSString*)inName];

	return result;
}

OSStatus
HIWebViewCreate( HIViewRef* outControl )
{
	return TWebView::Create( outControl );
}

WebController*
HIWebViewGetController( HIViewRef inControl )
{
	TWebView* 	view = (TWebView*)HIObjectDynamicCast( (HIObjectRef)inControl, kHIWebViewClassID );
	WebController*	result = NULL;
	
	if ( view )
		result = view->GetController();
	
	return result;
}

@interface NSWindow(HIWebView)
- (NSGraphicsContext *)_threadContext;
@end

@interface LocationChangeDelegate : WebLocationChangeDelegate
{
    TWebView* view;
}
- initWithView: (TWebView* )inView;
@end

@interface ResourceLoadDelegate : WebResourceLoadDelegate
{
    TWebView* view;
    int resourceCompletedCount;
    int resourceFailedCount;
    int resourceCount;
}
- initWithView: (TWebView* )inView;
@end

@interface NSEvent( Secret )
- (NSEvent *)_initWithCGSEvent:(CGSEventRecord)cgsEvent eventRef:(void *)eventRef;
@end

@interface NSView( Secret )
- (void) _setHIView:(HIViewRef)view;
- (void) _clearDirtyRectsForTree;
@end

@interface MenuItemProxy : NSObject <NSValidatedUserInterfaceItem> {
	int	_tag;
	SEL _action;
}

- (id)initWithAction:(SEL)action;
- (SEL)action;
- (int)tag;

@end

@implementation MenuItemProxy

- (id)initWithAction:(SEL)action {
	[super init];
    if (self == nil) return nil;
	
	_action = action;
	
	return self;
}

- (SEL)action {
	return _action;
}

- (int)tag {
	return 0;
}
@end

@interface NSWindowGraphicsContext (NSHLTBAdditions)
- (void)setCGContext:(CGContextRef)cgContext;
@end

@implementation NSWindowGraphicsContext (NSHLTBAdditions)
- (void)setCGContext:(CGContextRef)cgContext {
    CGContextRetain(cgContext);
    if (_cgsContext) {
        CGContextRelease(_cgsContext);
    }
    _cgsContext = cgContext;
}
@end

const OSType NSAppKitPropertyCreator = 'akit';
const OSType NSViewCarbonControlViewPropertyTag = 'view';
const OSType NSViewCarbonControlAutodisplayPropertyTag = 'autd';
const OSType NSViewCarbonControlFirstResponderViewPropertyTag = 'frvw';
const OSType NSCarbonWindowPropertyTag = 'win ';

static SEL _NSSelectorForHICommand( const HICommand& hiCommand );

TWebView::TWebView( ControlRef inControl ) : TView( inControl )
{
	NSRect		frame = { { 0, 0 }, { 400, 400  } };

	fView = [[WebView alloc] initWithFrame: frame];
	[HIViewAdapter bindHIViewToNSView:inControl nsView:fView];
	
	fFirstResponder = NULL;
	fKitWindow = NULL;

	ActivateInterface( kKeyboardFocus );

	fEventHandler = NULL;
	fWindowHandler = NULL;
}

TWebView::~TWebView()
{
	[HIViewAdapter unbindNSView:fView];
	[fView release];
	
	if ( fEventHandler )
		RemoveEventHandler( fEventHandler );
	
	if ( fWindowHandler )
		RemoveEventHandler( fWindowHandler );
}

void
TWebView::RegisterClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		TView::RegisterSubclass( kHIWebViewClassID, Construct );
		sRegistered = true;
	}
}

OSStatus
TWebView::Create( ControlRef* outControl )
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	
	RegisterClass();

	err = HIObjectCreate( kHIWebViewClassID, event, (HIObjectRef*)outControl );
	
	ReleaseEvent( event );

	return err;
}

UInt32
TWebView::GetBehaviors()
{
	return TView::GetBehaviors() | kControlGetsFocusOnClick;
}

OSStatus
TWebView::Initialize( TCarbonEvent& inEvent )
{
	static const EventTypeSpec kEvents[] = { 
		{ kEventClassMouse, kEventMouseUp },
		{ kEventClassMouse, kEventMouseMoved },
		{ kEventClassMouse, kEventMouseDragged },
		{ kEventClassMouse, kEventMouseWheelMoved },
		{ kEventClassKeyboard, kEventRawKeyDown },
		{ kEventClassKeyboard, kEventRawKeyRepeat }
	};

	OSStatus	err = TView::Initialize( inEvent );
	
	if ( err == noErr )
	{
		InstallEventHandler( GetEventTarget(), EventHandler,
			GetEventTypeCount( kEvents ), kEvents, this, &fEventHandler );
	}

	return err;
}

WebController*
TWebView::GetController()
{
	return [(WebView*)fView controller];
}


void
TWebView::Draw( RgnHandle limitRgn, CGContextRef inContext )
{
	HIRect		bounds;
	CGContextRef		temp;
	Rect				drawRect;
	HIRect				hiRect;
	
	bounds = Bounds();
//	CGContextStrokeRect( inContext, bounds );

	temp = (CGContextRef)[[fKitWindow _threadContext] graphicsPort];
	CGContextRetain( temp );
	[[fKitWindow _threadContext] setCGContext: inContext];
	[NSGraphicsContext setCurrentContext:[fKitWindow _threadContext] ];

	GetRegionBounds( limitRgn, &drawRect );
	hiRect.origin.x = drawRect.left;
	hiRect.origin.y = bounds.size.height - drawRect.bottom; // flip y
	hiRect.size.width = drawRect.right - drawRect.left;
	hiRect.size.height = drawRect.bottom - drawRect.top;

/*
{
NSRect temp = [fView frame];
printf( "view frame is (%g %g) (%g %g)\n", temp.origin.y, temp.origin.x,
	temp.size.height, temp.size.width );

bounds = Frame();
printf( "our frame is (%g %g) (%g %g)\n", bounds.origin.y, bounds.origin.x,
	bounds.size.height, bounds.size.width );
}
*/
	[fView displayRect: *(NSRect*)&hiRect];

	[[fKitWindow _threadContext] setCGContext: temp];
	CGContextRelease( temp );
}

ControlPartCode
TWebView::HitTest( const HIPoint& where )
{
	if ( CGRectContainsPoint( Bounds(), where ) )
		return 1;
	else
		return kControlNoPart;
}

OSStatus
TWebView::GetRegion(
	ControlPartCode		inPart,
	RgnHandle			outRgn )
{
	OSStatus	 err;
	
	switch ( inPart )
	{
		case -3:
			{
				if ( [fView isOpaque] )
				{
					HIRect	bounds = Bounds();
					Rect	temp;
					
					temp.top = (SInt16)bounds.origin.y;
					temp.left = (SInt16)bounds.origin.x;
					temp.bottom = (SInt16)CGRectGetMaxY( bounds );
					temp.right = (SInt16)CGRectGetMaxX( bounds );
	
					RectRgn( outRgn, &temp );
					err = noErr;
				}
				else
				{
					err = TView::GetRegion( inPart, outRgn );
				}
			}
			break;
		
		default:
			err = TView::GetRegion( inPart, outRgn );
			break;
	}
	
	return err;
}

OSStatus
TWebView::Click( TCarbonEvent& inEvent )
{
	CGSEventRecord			eventRec;
	NSEvent*				kitEvent;
	ControlRef				focus;
	NSView*					targ;
	EventRef				newEvent;
	Point					where;
	OSStatus				err;
	UInt32					modifiers;
	Rect					windRect;
	
//	inEvent.GetParameter<CGSEventRecord>( 'cgs ', 'cgs ', &eventRec );
	GetEventPlatformEventRecord( inEvent.GetEventRef(), &eventRec );

	// We need to make the event be a kEventMouseDown event, or the webkit might trip up when
	// we click on a Netscape plugin. It calls ConvertEventRefToEventRecord, assuming
	// that mouseDown was passed an event with a real mouse down eventRef. We just need a
	// minimal one here.

	err = CreateEvent( NULL, kEventClassMouse, kEventMouseDown, GetEventTime( inEvent ), 0, &newEvent );
	require_noerr( err, CantAllocNewEvent );

	inEvent.GetParameter( kEventParamWindowMouseLocation, &where );
	GetWindowBounds( GetWindowRef(), kWindowStructureRgn, &windRect );
	where.h += windRect.left;
	where.v += windRect.top;
	
	inEvent.GetParameter( kEventParamKeyModifiers, &modifiers );
	SetEventParameter( newEvent, kEventParamMouseLocation, typeQDPoint, sizeof( Point ), &where );
	SetEventParameter( newEvent, kEventParamKeyModifiers, typeUInt32, sizeof( UInt32 ), &modifiers );
	
	kitEvent = [[NSEvent alloc] _initWithCGSEvent:(CGSEventRecord)eventRec eventRef:(void *)newEvent];

	// Grab the keyboard focus
	// ••• FIX: Need to switch to a real part code, not focusnextpart. Have to handle
	//			subviews properly as well.

	GetKeyboardFocus( GetWindowRef(), &focus );
	if ( focus != GetViewRef() )
		::SetKeyboardFocus( GetWindowRef(), GetViewRef(), kControlFocusNextPart );

	targ = [[fKitWindow _borderView] hitTest:[kitEvent locationInWindow]];

	[targ mouseDown:kitEvent];

	[kitEvent release];

CantAllocNewEvent:	
	return noErr;
}

OSStatus
TWebView::MouseUp( TCarbonEvent& inEvent )
{
	CGSEventRecord			eventRec;
	NSEvent*				kitEvent;
//	ControlRef				focus;
	NSView*					targ;
	
//	inEvent.GetParameter<CGSEventRecord>( 'cgs ', 'cgs ', &eventRec );
	GetEventPlatformEventRecord( inEvent.GetEventRef(), &eventRec );
	RetainEvent( inEvent.GetEventRef() );
	kitEvent = [[NSEvent alloc] _initWithCGSEvent:(CGSEventRecord)eventRec eventRef:(void *)inEvent.GetEventRef()];

	targ = [[fKitWindow _borderView] hitTest:[kitEvent locationInWindow]];

	[targ mouseUp:kitEvent];

	[kitEvent release];
	
	return noErr;
}

OSStatus
TWebView::MouseMoved( TCarbonEvent& inEvent )
{
	CGSEventRecord			eventRec;
	NSEvent*				kitEvent;
//	ControlRef				focus;
	NSView*					targ;
	
//	inEvent.GetParameter<CGSEventRecord>( 'cgs ', 'cgs ', &eventRec );
	GetEventPlatformEventRecord( inEvent.GetEventRef(), &eventRec );
	RetainEvent( inEvent.GetEventRef() );
	kitEvent = [[NSEvent alloc] _initWithCGSEvent:(CGSEventRecord)eventRec eventRef:(void *)inEvent.GetEventRef()];

	targ = [[fKitWindow _borderView] hitTest:[kitEvent locationInWindow]];

	[targ mouseMoved:kitEvent];

	[kitEvent release];
	
	return noErr;
}

OSStatus
TWebView::MouseDragged( TCarbonEvent& inEvent )
{
	CGSEventRecord			eventRec;
	NSEvent*				kitEvent;
//	ControlRef				focus;
	NSView*					targ;
	
//	inEvent.GetParameter<CGSEventRecord>( 'cgs ', 'cgs ', &eventRec );
	GetEventPlatformEventRecord( inEvent.GetEventRef(), &eventRec );
	RetainEvent( inEvent.GetEventRef() );
	kitEvent = [[NSEvent alloc] _initWithCGSEvent:(CGSEventRecord)eventRec eventRef:(void *)inEvent.GetEventRef()];

	targ = [[fKitWindow _borderView] hitTest:[kitEvent locationInWindow]];

	[targ mouseDragged:kitEvent];

	[kitEvent release];
	
	return noErr;
}

OSStatus
TWebView::MouseWheelMoved( TCarbonEvent& inEvent )
{
	CGSEventRecord			eventRec;
	NSEvent*				kitEvent;
//	ControlRef				focus;
	NSView*					targ;
	
//	inEvent.GetParameter<CGSEventRecord>( 'cgs ', 'cgs ', &eventRec );
	GetEventPlatformEventRecord( inEvent.GetEventRef(), &eventRec );
	RetainEvent( inEvent.GetEventRef() );
	kitEvent = [[NSEvent alloc] _initWithCGSEvent:(CGSEventRecord)eventRec eventRef:(void *)inEvent.GetEventRef()];

	targ = [[fKitWindow _borderView] hitTest:[kitEvent locationInWindow]];

	[targ scrollWheel:kitEvent];

	[kitEvent release];
	
	return noErr;
}

OSStatus
TWebView::ContextMenuClick( TCarbonEvent& inEvent )
{
	CGSEventRecord			eventRec;
	NSEvent*				kitEvent;
	OSStatus				result = eventNotHandledErr;
	NSView*					targ;
	
//	inEvent.GetParameter<CGSEventRecord>( 'cgs ', 'cgs ', &eventRec );
	GetEventPlatformEventRecord( inEvent.GetEventRef(), &eventRec );
	RetainEvent( inEvent.GetEventRef() );
	kitEvent = [[NSEvent alloc] _initWithCGSEvent:(CGSEventRecord)eventRec eventRef:(void *)inEvent.GetEventRef()];

	targ = [[fKitWindow _borderView] hitTest:[kitEvent locationInWindow]];

    if ( [targ _allowsContextMenus] )
	{
        NSMenu * contextMenu = [targ menuForEvent:kitEvent];

        if ( contextMenu )
		{
            [contextMenu _popUpMenuWithEvent:kitEvent forView:targ];
			result = noErr;
        }
    }

	[kitEvent release];
	
	return result;
}

OSStatus
TWebView::Construct( ControlRef inControl, TView** outView )
{
	*outView = new TWebView( inControl );
	
	return noErr;
}

ControlKind
TWebView::GetKind()
{
	const ControlKind kMyKind = { 'macs', 'coco' };
	
	return kMyKind;
}

OSStatus
TWebView::BoundsChanged(
	UInt32				inOptions,
	const HIRect&		inOriginalBounds,
	const HIRect&		inCurrentBounds,
	RgnHandle			inInvalRgn )
{
	if ( fView )
	{
		SyncFrame();
	}
	
	return noErr;
}

void
TWebView::OwningWindowChanged(
	WindowRef			oldWindow,
	WindowRef			newWindow )
{
	if ( newWindow )
	{
    	OSStatus err = GetWindowProperty(newWindow, NSAppKitPropertyCreator, NSCarbonWindowPropertyTag, sizeof(NSWindow *), NULL, &fKitWindow);
		if ( err != noErr )
		{
			fKitWindow = [[CarbonWindowAdapter alloc] initWithCarbonWindowRef: newWindow takingOwnership: NO disableOrdering:NO carbon:YES];
    		SetWindowProperty(newWindow, NSAppKitPropertyCreator, NSCarbonWindowPropertyTag, sizeof(NSWindow *), &fKitWindow);
		}
		
		[[fKitWindow contentView] addSubview:fView];
		SyncFrame();
	}
}

void
TWebView::SyncFrame()
{
	if ( GetSuperview() )
	{
		HIRect		frame;
		HIRect		parentBounds;
		NSPoint		origin;

		frame = Frame();
	
		HIViewGetBounds( GetSuperview(), &parentBounds );
		
		origin.x = frame.origin.x;
		origin.y = parentBounds.size.height - CGRectGetMaxY( frame );

		[fView setFrameOrigin: origin];
		[fView setFrameSize: *(NSSize*)&frame.size];
	}
}

OSStatus
TWebView::SetFocusPart(
	ControlPartCode 		desiredFocus,
	RgnHandle 				invalidRgn,
	Boolean 				focusEverything,
	ControlPartCode* 		actualFocus )
{
    NSView *	freshlyMadeFirstResponderView;
    OSStatus 	osStatus;
    SInt32 		partCodeToReturn;

    // Do what Carbon is telling us to do.
    if ( desiredFocus == kControlFocusNoPart )
	{
        // Relinquish the keyboard focus.
        RelinquishFocus( true ); //(autodisplay ? YES : NO));
        freshlyMadeFirstResponderView = nil;
        partCodeToReturn = kControlFocusNoPart;
		//NSLog(@"Relinquished the key focus because we have no choice.");
    }
	else if ( desiredFocus == kControlFocusNextPart || desiredFocus == kControlFocusPrevPart )
	{
        BOOL goForward = (desiredFocus == kControlFocusNextPart );

        // Advance the keyboard focus, maybe right off of this view.  Maybe a subview of this one already has the keyboard focus, maybe not.
        freshlyMadeFirstResponderView = AdvanceFocus( goForward );
        partCodeToReturn = freshlyMadeFirstResponderView ? desiredFocus : kControlFocusNoPart;
        //NSLog(freshlyMadeFirstResponderView ? @"Advanced the key focus." : @"Relinquished the key focus.");
    }
	else
	{
		// What's this?
		check(false);
		freshlyMadeFirstResponderView = nil;
		partCodeToReturn = kControlFocusNoPart;
    }

	fFirstResponder = freshlyMadeFirstResponderView;

	*actualFocus = partCodeToReturn;

	// Done.
	return noErr;
}

NSView*
TWebView::AdvanceFocus( bool forward )
{
    NSResponder*		oldFirstResponder;
    NSView*				currentKeyView;
    NSView*				viewWeMadeFirstResponder;
    
    //	Focus on some part (subview) of this control (view).  Maybe
	//	a subview of this one already has the keyboard focus, maybe not.
	
	oldFirstResponder = [fKitWindow firstResponder];

	// If we tab out of our NSView, it will no longer be the responder
	// when we get here. We'll try this trick for now. We might need to
	// tag the view appropriately.

	if ( fFirstResponder && fFirstResponder != oldFirstResponder )
	{
		return NULL;
	}
	
	if ( [oldFirstResponder isKindOfClass:[NSView class]] )
	{
		NSView*		tentativeNewKeyView;

        // Some view in this window already has the keyboard focus.  It better at least be a subview of this one.
        NSView*	oldFirstResponderView = (NSView *)oldFirstResponder;
        check( [oldFirstResponderView isDescendantOf:fView] );

		if ( oldFirstResponderView != fFirstResponder
			&& ![oldFirstResponderView isDescendantOf:fFirstResponder] )
		{
            // Despite our efforts to record what view we made the first responder
			// (for use in the next paragraph) we couldn't keep up because the user
			// has clicked in a text field to make it the key focus, instead of using
			// the tab key.  Find a control on which it's reasonable to invoke
			// -[NSView nextValidKeyView], taking into account the fact that
			// NSTextFields always pass on first-respondership to a temporarily-
			// contained NSTextView.

			NSView *viewBeingTested;
			currentKeyView = oldFirstResponderView;
			viewBeingTested = currentKeyView;
			while ( viewBeingTested != fView )
			{
				if ( [viewBeingTested isKindOfClass:[NSTextField class]] )
				{
					currentKeyView = viewBeingTested;
					break;
				}
				else
				{
					viewBeingTested = [viewBeingTested superview];
				}
			}
		}
		else 
		{
			// We recorded which view we made into the first responder the
			// last time the user hit the tab key, and nothing has invalidated
			// our recorded value since.
			
			currentKeyView = fFirstResponder;
		}

        // Try to move on to the next or previous key view.  We use the laboriously
		// recorded/figured currentKeyView instead of just oldFirstResponder as the
		// jumping-off-point when searching for the next valid key view.  This is so
		// we don't get fooled if we recently made some view the first responder, but
		// it passed on first-responder-ness to some temporary subview.
		
        // You can't put normal views in a window with Carbon-control-wrapped views.
		// Stuff like this would break.  M.P. Notice - 12/2/00

        tentativeNewKeyView = forward ? [currentKeyView nextValidKeyView] : [currentKeyView previousValidKeyView];
        if ( tentativeNewKeyView && [tentativeNewKeyView isDescendantOf:fView] )
		{
            // The user has tabbed to another subview of this control view.  Change the keyboard focus.
            //NSLog(@"Tabbed to the next or previous key view.");

            [fKitWindow makeFirstResponder:tentativeNewKeyView];
            viewWeMadeFirstResponder = tentativeNewKeyView;
        }
		else
		{
            // The user has tabbed past the subviews of this control view.  The window is the first responder now.
            //NSLog(@"Tabbed past the first or last key view.");
            [fKitWindow makeFirstResponder:fKitWindow];
            viewWeMadeFirstResponder = nil;
        }
    }
	else
	{
        // No view in this window has the keyboard focus.  This view should
		// try to select one of its key subviews.  We're not interested in
		// the subviews of sibling views here.

		//NSLog(@"No keyboard focus in window. Attempting to set...");

		NSView *tentativeNewKeyView;
		check(oldFirstResponder==fKitWindow);
		if ( [fView acceptsFirstResponder] )
			tentativeNewKeyView = fView;
		else
			tentativeNewKeyView = [fView nextValidKeyView];
        if ( tentativeNewKeyView && [tentativeNewKeyView isDescendantOf:fView] )
		{
            // This control view has at least one subview that can take the keyboard focus.
            if ( !forward )
			{
                // The user has tabbed into this control view backwards.  Find
				// and select the last subview of this one that can take the
				// keyboard focus.  Watch out for loops of valid key views.

                NSView *firstTentativeNewKeyView = tentativeNewKeyView;
                NSView *nextTentativeNewKeyView = [tentativeNewKeyView nextValidKeyView];
                while ( nextTentativeNewKeyView 
						&& [nextTentativeNewKeyView isDescendantOf:fView] 
						&& nextTentativeNewKeyView!=firstTentativeNewKeyView)
				{
                    tentativeNewKeyView = nextTentativeNewKeyView;
                    nextTentativeNewKeyView = [tentativeNewKeyView nextValidKeyView];
                }

            }

            // Set the keyboard focus.
            //NSLog(@"Tabbed into the first or last key view.");
            [fKitWindow makeFirstResponder:tentativeNewKeyView];
            viewWeMadeFirstResponder = tentativeNewKeyView;
        }
		else
		{
            // This control view has no subviews that can take the keyboard focus.
            //NSLog(@"Can't tab into this view.");
            viewWeMadeFirstResponder = nil;
        }
    }

    // Done.
    return viewWeMadeFirstResponder;
}


void
TWebView::RelinquishFocus( bool inAutodisplay )
{
    NSResponder*  firstResponder;

    // Apparently Carbon thinks that some subview of this control view has the keyboard focus,
	// or we wouldn't be being asked to relinquish focus.

	firstResponder = [fKitWindow firstResponder];
	if ( [firstResponder isKindOfClass:[NSView class]] )
	{
		// Some subview of this control view really is the first responder right now.
		check( [(NSView *)firstResponder isDescendantOf:fView] );

		// Make the window the first responder, so that no view is the key view.
        [fKitWindow makeFirstResponder:fKitWindow];

		// 	If this control is not allowed to do autodisplay, don't let
		//	it autodisplay any just-changed focus rings or text on the
		//	next go around the event loop. I'm probably clearing more
		//	dirty rects than I have to, but it doesn't seem to hurt in
		//	the print panel accessory view case, and I don't have time
		//	to figure out exactly what -[NSCell _setKeyboardFocusRingNeedsDisplay]
		//	is doing when invoked indirectly from -makeFirstResponder up above.  M.P. Notice - 12/4/00

		if ( !inAutodisplay )
			[[fView opaqueAncestor] _clearDirtyRectsForTree];
    }
	else
	{
		//  The Cocoa first responder does not correspond to the Carbon
		//	control that has the keyboard focus.  This can happen when
		//	you've closed a dialog by hitting return in an NSTextView
		//	that's a subview of this one; Cocoa closed the window, and
		//	now Carbon is telling this control to relinquish the focus
		//	as it's being disposed.  There's nothing to do.

		check(firstResponder==window);
	}
}

void
TWebView::ActiveStateChanged()
{
	if ( [fView respondsToSelector:@selector(setEnabled)] )
	{
		[(NSControl*)fView setEnabled: IsEnabled()];
		Invalidate();
	}
}

OSStatus
TWebView::EventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	OSStatus			result;
	TWebView*			view = (TWebView*) inUserData;
	TCarbonEvent		event( inEvent );
	
	result = view->HandleEvent( inCallRef, event );

	return result;
}

OSStatus
TWebView::HandleEvent( EventHandlerCallRef inCallRef, TCarbonEvent& inEvent )
{
	CGSEventRecord			eventRec;
	NSEvent*				kitEvent;
	ControlRef				focus;
	OSStatus				result = eventNotHandledErr;

	switch ( inEvent.GetClass() )
	{
		case kEventClassKeyboard:
//			inEvent.GetParameter<CGSEventRecord>( 'cgs ', 'cgs ', &eventRec );
			GetEventPlatformEventRecord( inEvent.GetEventRef(), &eventRec );
			RetainEvent( inEvent.GetEventRef() );
			kitEvent = [[NSEvent alloc] _initWithCGSEvent:(CGSEventRecord)eventRec eventRef:(void *)inEvent.GetEventRef()];

			[fKitWindow sendSuperEvent:kitEvent];
			
			result = noErr;
			break;

		case kEventClassMouse:
			switch ( inEvent.GetKind() )
			{
				case kEventMouseUp:
					result = MouseUp( inEvent );
					break;
				
				case kEventMouseWheelMoved:
					result = MouseWheelMoved( inEvent );
					break;

				case kEventMouseMoved:
					result = MouseMoved( inEvent );
					break;

				case kEventMouseDragged:
					result = MouseDragged( inEvent );
					break;
			}
			break;
	}
	
	return result;
}

OSStatus
TWebView::ProcessCommand( const HICommand& inCommand )
{
	OSStatus		result = eventNotHandledErr;
	NSResponder*	resp;
	
	resp = [fKitWindow firstResponder];

	if ( resp == fView || ( [resp respondsToSelector:@selector(isDescendantOf:)] && [resp isDescendantOf: fView] ) )
	{
		switch ( inCommand.commandID )
		{
			case kHICommandCut:
				if ( [resp respondsToSelector:@selector(cut:)] )
				{
					[resp cut:nil];
					result = noErr;
				}
				break;
	
			case kHICommandCopy:
				if ( [resp respondsToSelector:@selector(copy:)] )
				{
					[resp copy:nil];
					result = noErr;
				}
				break;
	
			case kHICommandPaste:
				if ( [resp respondsToSelector:@selector(paste:)] )
				{
					[resp paste:nil];
					result = noErr;
				}
				break;
	
			case kHICommandClear:
				if ( [resp respondsToSelector:@selector(delete:)] )
				{
					[resp delete:nil];
					result = noErr;
				}
				break;
	
			case kHICommandSelectAll:
				if ( [resp respondsToSelector:@selector(selectAll:)] )
				{
					[resp selectAll:nil];
					result = noErr;
				}
				break;
		}
	}
	
	return result;
}

OSStatus
TWebView::UpdateCommandStatus( const HICommand& inCommand )
{
	OSStatus		result = eventNotHandledErr;
	MenuItemProxy* 	proxy = NULL;
	NSResponder*	resp;
	
	resp = [fKitWindow firstResponder];
	
	if ( resp == fView || ( [resp respondsToSelector:@selector(isDescendantOf:)] && [resp isDescendantOf: fView] ) )
	{
		if ( inCommand.attributes & kHICommandFromMenu )
		{
			SEL selector = _NSSelectorForHICommand( inCommand );

			if ( selector )
			{
				if ( [resp respondsToSelector: selector] )
				{
					proxy = [[MenuItemProxy alloc] initWithAction: selector];
					
					if ( [resp validateUserInterfaceItem: proxy] )
						EnableMenuItem( inCommand.menu.menuRef, inCommand.menu.menuItemIndex );
					else
						DisableMenuItem( inCommand.menu.menuRef, inCommand.menu.menuItemIndex );
					
					result = noErr;
				}
			}
		}
	}
	
	if ( proxy )
		[proxy release];

	return result;
}

// Blatantly stolen from AppKit and cropped a bit

static SEL
_NSSelectorForHICommand( const HICommand& inCommand )
{
    switch ( inCommand.commandID )
	{
        case kHICommandUndo: return @selector(undo:);
        case kHICommandRedo: return @selector(redo:);
        case kHICommandCut  : return @selector(cut:);
        case kHICommandCopy : return @selector(copy:);
        case kHICommandPaste: return @selector(paste:);
        case kHICommandClear: return @selector(delete:);
        case kHICommandSelectAll: return @selector(selectAll:);
        default: return NULL;
    }

    return NULL;
}


