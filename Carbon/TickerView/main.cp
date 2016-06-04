#include <Carbon/Carbon.h>

#include "TTickerView.h"


// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//
OSStatus WindowEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData );

// -----------------------------------------------------------------------------
//	main
// -----------------------------------------------------------------------------
//
int main(int argc, char* argv[])
{
#pragma unused( argc, argv )
	IBNibRef 		nibRef;
	WindowRef 		window;
	HIViewRef		view;
	Rect			state;
	EventTypeSpec	events[] = {	{ kEventClassWindow, kEventWindowBoundsChanging },
									{ kEventClassWindow, kEventWindowBoundsChanged } };
	
	OSStatus		err;
	
	// Workaround: Unfortunately, at the time we call RegisterClass below,
	// the HIView base class isn't registered. It's supposed to be automatically
	// registered, but something is going wrong in HIToolbox. We can force
	// it to register by creating any arbitrary view. Here, we simply create
	// and release a scroll view. That's enough to make sure the HIView base
	// class is registered. Sorry folks.
	HIScrollViewCreate( kHIScrollViewOptionsVertScroll, &view );
	CFRelease( view );
	
	// Register our ticker view subclass 
	TTickerView::RegisterClass();
	
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference( CFSTR( "main" ), &nibRef );
	require_noerr( err, CantGetNibRef );
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	err = SetMenuBarFromNib( nibRef, CFSTR( "MenuBar" ) );
	require_noerr( err, CantSetMenuBar );
	
	// Then create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	err = CreateWindowFromNib( nibRef, CFSTR( "MainWindow" ), &window );
	require_noerr( err, CantCreateWindow );
	
	{
		Rect		bounds;
		GetWindowBounds( window, kWindowContentRgn, &bounds );
		GetWindowStandardState( window, &state );
//		GetWindowIdealUserState( window, &state );
		state.bottom = state.top + (bounds.bottom - bounds.top);
		SetWindowStandardState( window, &state );
	//	SetWindowIdealUserState( window, &state );
	}
	
	// Install a custom handler on the window so we can do some resize-related activities.
	InstallWindowEventHandler( window, NewEventHandlerUPP( WindowEventHandler ),
		GetEventTypeCount( events ), events, window, NULL );
	
	// We don't need the nib reference anymore.
	DisposeNibReference( nibRef );
	
	// The window was created hidden so show it.
	ShowWindow( window );
	
	// Call the event loop
	RunApplicationEventLoop();
	
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	
	return err;
}

// -----------------------------------------------------------------------------
//	WindowEventHandler
// -----------------------------------------------------------------------------
//
OSStatus WindowEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	OSStatus			err;
	WindowRef			window = (WindowRef)inUserData;
	Rect				originalBounds;
	Rect				currentBounds;
	ControlID			id = { 'TikV', 0 };
	HIViewRef			view;
	HIRect				frame;
	
	check( GetEventClass( inEvent ) == kEventClassWindow );
	
	err = GetEventParameter( inEvent, kEventParamOriginalBounds, typeQDRectangle, NULL,
		sizeof( originalBounds ), NULL, &originalBounds );
	require_noerr( err, CantGetOriginalBounds );
	err = GetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle, NULL,
		sizeof( currentBounds ), NULL, &currentBounds );
	require_noerr( err, CantGetCurrentBounds );
		
	if ( GetEventKind( inEvent ) == kEventWindowBoundsChanging )
	{
		// during the resize (present tense), we want to constrain the height
		// to match our starting height. We don't want the window to get any bigger
		// or smaller than that.
		
		currentBounds.bottom = currentBounds.top + (originalBounds.bottom - originalBounds.top);
		
		err = SetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle,
			sizeof( currentBounds ), &currentBounds );
	}
	else // this will be a kEventWindowBoundsChanged event
	{
		// after we've been resized, give our ticker view the proper new size.
		// keep its bounds offset the same from the window's new bounding rectangle
		// as it was from the original bounding rectangle.
		
		err = GetControlByID( window, &id, &view );
		require_noerr( err, CantGetTickerViewByID );
		
		// this avoids remnants when downsizing. this shouldn't really be
		// necessary, since the HIView mechanism should take care of redrawing
		// that area for us, but it isn't working properly.
		HIViewSetVisible( view, false );
		
		HIViewGetFrame( view, &frame );
		frame.size.height += (currentBounds.bottom - currentBounds.top) - (originalBounds.bottom - originalBounds.top);
		frame.size.width += (currentBounds.right - currentBounds.left) - (originalBounds.right - originalBounds.left);
		HIViewSetFrame( view, &frame );
		
		HIViewSetVisible( view, true );
	}
	
CantGetTickerViewByID:
CantGetCurrentBounds:
CantGetOriginalBounds:

	return err;
}
