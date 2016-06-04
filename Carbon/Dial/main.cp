// =============================================================================
//	main.cp
// =============================================================================
//

#include <Carbon/Carbon.h>

#include "DialView.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
const ControlID	kDialControl = { 'dial', 0 };
const UInt32	kShowTickMarksCommand = 'show';
const UInt16	kMaxTickCount = 20;

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//
OSStatus AppEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData);
OSStatus WindowEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData);
OSStatus ValueChangedHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData);
OSStatus CreateWindow();

DEFINE_ONE_SHOT_HANDLER_GETTER( AppEventHandler );
DEFINE_ONE_SHOT_HANDLER_GETTER( WindowEventHandler );
DEFINE_ONE_SHOT_HANDLER_GETTER( ValueChangedHandler );

OSStatus ShowTickMarks(
	WindowRef			inWindow,
	Boolean				inShow );

// -----------------------------------------------------------------------------
//	main
// -----------------------------------------------------------------------------
//
int main(
	int				argc,
	char*			argv[] )
{
#pragma unused( argc, argv )
    IBNibRef 		nibRef;
    EventTypeSpec	appEventList[] = { { kEventClassCommand, kEventProcessCommand } };
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference( CFSTR( "main" ), &nibRef );
	require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib( nibRef, CFSTR( "MenuBar" ) );
    require_noerr( err, CantSetMenuBar );
    
    // We don't need the nib reference anymore.
    DisposeNibReference( nibRef );
    
    // Install the applcation event handler
	err = InstallApplicationEventHandler( GetAppEventHandlerUPP(),
			GetEventTypeCount( appEventList ), appEventList, NULL, NULL );
	require_noerr( err, CantInstallAppEventHandler );
	
	err = CreateWindow();
	require_noerr( err, CantCreateWindow );
	
	// Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantInstallAppEventHandler:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

// -----------------------------------------------------------------------------
//	CreateWindow
// -----------------------------------------------------------------------------
//
OSStatus CreateWindow()
{
    IBNibRef 		nibRef;
    WindowRef 		window;
    EventTypeSpec	windowEventList[] = {
							{ kEventClassCommand, kEventProcessCommand } };
	EventTypeSpec	valueChangedEventList[] = {
							{ kEventClassControl, kEventControlValueFieldChanged } };
    OSStatus		err;
	HIViewRef		view;

	DialViewRegister();

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference( CFSTR( "main" ), &nibRef );
    require_noerr( err, CantGetNibRef );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib( nibRef, CFSTR( "MainWindow" ), &window );
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference( nibRef );
    
    // Install the window event handler
	err = InstallWindowEventHandler( window, GetWindowEventHandlerUPP(),
			GetEventTypeCount( windowEventList ), windowEventList, window, NULL );
	require_noerr( err, CantInstallWindowEventHandler );
	
	err = GetControlByID( window, &kDialControl, &view );
	require_noerr( err, CantGetControlByID );
	
    // Install the control event handler that watches for value changes
	err = InstallControlEventHandler( view, GetValueChangedHandlerUPP(),
			GetEventTypeCount( valueChangedEventList ), valueChangedEventList, view, NULL );
	require_noerr( err, CantInstallControlEventHandler );
	
	// The window was created hidden so show it.
    ShowWindow( window );
    
CantInstallControlEventHandler:
CantGetControlByID:
CantInstallWindowEventHandler:
CantCreateWindow:
CantGetNibRef:
	return err;
}	

// -----------------------------------------------------------------------------
//	AppEventHandler
// -----------------------------------------------------------------------------
//
OSStatus AppEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
#pragma unused( inHandlerCallRef, inUserData )
	HICommand			hiCommand;
	OSStatus			err = eventNotHandledErr;
	
	// UInt32			eventClass = GetEventClass( inEvent );
	// UInt32			eventKind = GetEventKind( inEvent );

	// We are only registered for the kEventClassCommand/kEventProcessCommand pair

	// if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	//{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );
		
		switch ( hiCommand.commandID )
		{
			case kHICommandNew:
				err = CreateWindow();
				break;
	
			default:
				err = eventNotHandledErr;
				break;
		}
	//}
	
CantGetEventParameter:
	return err;
}

// -----------------------------------------------------------------------------
//	ValueChangedHandler
// -----------------------------------------------------------------------------
//
OSStatus ValueChangedHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	OSStatus			err = noErr;
	HIViewRef			view = (HIViewRef) inUserData;
	CFStringRef			title;
	const ControlID		kValueText = { 'valF', 0 };
	
	title = CFStringCreateWithFormat( NULL, NULL, CFSTR( "%ld" ), GetControlValue( view ) );
	require_action( title != NULL, CantCreateString, err = memFullErr );
	
	err = GetControlByID( GetControlOwner( view ), &kValueText, &view );
	require_noerr( err, CantGetControlByID );

	err = SetControlData( view, 0, kControlStaticTextCFStringTag, sizeof( CFStringRef ), &title );
	HIViewSetNeedsDisplay( view, true );
	
CantGetControlByID:
	CFRelease( title );
	
CantCreateString:
	return err;
}

// -----------------------------------------------------------------------------
//	ShowTickMarks
// -----------------------------------------------------------------------------
//
OSStatus ShowTickMarks(
	WindowRef			inWindow,
	Boolean				inShow )
{
	OSStatus			err;
	HIViewRef			view;
	UInt16				ticks;

	err = GetControlByID( inWindow, &kDialControl, &view );
	require_noerr( err, CantGetControlByID );

	if ( inShow == true )
	{
		ticks = GetControl32BitMaximum( view ) - GetControl32BitMinimum( view ) + 1;
		
		// Too many ticks is a big blobby circle. Limit to kMaxTickCount.
		if ( ticks > kMaxTickCount )
			ticks = kMaxTickCount;
	}
	else
	{
		ticks = 0;
	}

	err = SetDialTickMarks( view, ticks );
	require_noerr( err, CantSetTickMarks );
	
	HIViewSetNeedsDisplay( view, true );

CantSetTickMarks:
CantGetControlByID:
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
#pragma unused( inHandlerCallRef )
	HICommandExtended	hiCommand;
	OSStatus			err;
	// UInt32			eventClass = GetEventClass( inEvent );
	// UInt32			eventKind = GetEventKind( inEvent );
	WindowRef			window = (WindowRef) inUserData;
	
	// We are only registered for the kEventClassCommand/kEventProcessCommand pair

	// if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	//{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommandExtended ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );
		
		switch ( hiCommand.commandID )
		{
			case kShowTickMarksCommand:
				err = ShowTickMarks( window, GetControlValue( hiCommand.source.control ) == 1 );
			
			default:
				err = eventNotHandledErr;
				break;
		}
	//}
	
CantGetEventParameter:
	return err;
}
