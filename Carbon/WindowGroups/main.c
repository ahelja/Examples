/*
	File:		main.c (WindowGroups)

	Contains:	Sample application which demonstrates the WindowGroup API.

	Version:	Mac OS X

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

	Copyright © 2002-2003 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

/*******************************************************************************
**	constants
*******************************************************************************/
static const ControlID	kButtonID = { 'WGRP', 1 };

#define kNewGroupieCommand		'ngrp'
#define	kGroupieCommand			'grup'
#define	kIsAGroupie				1
#define	kIsNotAGroupie			0

/*******************************************************************************
**	prototypes
*******************************************************************************/
static void CreatePopupWindow(
	WindowRef				owner );
static void DisposePopupWindow(
	WindowRef				popup,
	WindowRef				owner );
static OSStatus WindowHandler(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	void*					inUserData );
OSStatus MakeNewGroupieWindow(
	WindowRef*				outWindow );
OSStatus ApplicationEventHandler(
	EventHandlerCallRef		inHandlerCallRef,
	EventRef				inEvent,
	void					*inUserData );
OSStatus GroupieWindowEventHandler(
	EventHandlerCallRef		inHandlerCallRef,
	EventRef				inEvent,
	void					*inUserData );
OSStatus SetupWindowGroup();
	
/*******************************************************************************
**	globals
*******************************************************************************/
WindowGroupRef				gGroupieWindowGroup = NULL;

/*******************************************************************************
**	main
*******************************************************************************/
int main(int argc, char* argv[])
{
    IBNibRef 			nibRef;
    WindowRef 			window;
    const EventTypeSpec	kWindowEvents[] = { { kEventClassCommand, kEventCommandProcess },
										  { kEventClassCommand, kEventCommandUpdateStatus },
										  { kEventClassWindow, kEventWindowHandleContentClick } };
    const EventTypeSpec	kAppEvents[] = { { kEventClassCommand, kEventCommandProcess } };
    OSStatus			err;

	// Do the window group setup
	err = SetupWindowGroup();
    require_noerr( err, CantSetUpWindowGroup );

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

	InstallWindowEventHandler( window, WindowHandler, GetEventTypeCount( kWindowEvents ),
				kWindowEvents, window, NULL );
				
    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // The window was created hidden so show it.
    ShowWindow( window );
    
    // Install app event handler
	err = InstallApplicationEventHandler( ApplicationEventHandler,
		GetEventTypeCount( kAppEvents ), kAppEvents, NULL, NULL );

    // Call the event loop
    RunApplicationEventLoop();

	// Clean up a bit.
	ReleaseWindowGroup( gGroupieWindowGroup );

CantSetUpWindowGroup:
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

/*******************************************************************************
**	WindowHandler
*******************************************************************************/
static OSStatus
WindowHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	WindowRef		window = (WindowRef)inUserData;
	OSStatus		result = eventNotHandledErr;
	HICommand		cmd;

	if ( GetEventClass( inEvent ) == kEventClassCommand )
	{
		GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
			sizeof( HICommand ), NULL, &cmd );
	
		switch ( GetEventKind( inEvent ) )
		{
			case kEventCommandProcess:
				if ( cmd.commandID == 'POPU' )
				{
					ControlRef		control;
					
					CreatePopupWindow( window );
					GetControlByID( window, &kButtonID, &control );
					DisableControl( control );
					result = noErr;
				}
				break;
		}
	}
	else if ( GetEventClass( inEvent ) == kEventClassWindow )
	{
		if ( GetEventKind( inEvent ) == kEventWindowHandleContentClick )
		{
			WindowRef		popup;
			
			if ( GetWindowProperty( window, 'WGRP', 'POPU', sizeof( WindowRef ), NULL, &popup ) == noErr )
			{
				ControlRef		control;
				
				DisposePopupWindow( popup, window );
				GetControlByID( window, &kButtonID, &control );
				EnableControl( control );
				result = noErr;
			}
		}
	}
	
	return result;
}

/*******************************************************************************
**	CreatePopupWindow
*******************************************************************************/
static void
CreatePopupWindow( WindowRef owner )
{
	IBNibRef		nibRef;
	Rect			rect;
	WindowGroupRef	group;
	WindowRef		popup;

	CreateNibReference( CFSTR("main"), &nibRef );
	CreateWindowFromNib( nibRef, CFSTR("Popup"), &popup );

	GetWindowBounds( owner, kWindowContentRgn, &rect );
	MoveWindow( popup, rect.left + 40, rect.top + 40, false );
	SetWindowGroup( popup, GetWindowGroup( owner ) );
	CreateWindowGroup( 0, &group );
	SetWindowGroupParent( group, GetWindowGroup( owner ) );
	SetWindowGroup( owner, group );
	SetWindowGroup( popup, group );
	BringToFront( popup );
	ChangeWindowGroupAttributes( group, kWindowGroupAttrLayerTogether | kWindowGroupAttrMoveTogether
				| kWindowGroupAttrSharedActivation | kWindowGroupAttrHideOnCollapse, 0 );
	ChangeWindowAttributes( popup, 0, kWindowNoShadowAttribute );
	ShowWindow( popup );

	DisposeNibReference( nibRef );
	
	SetWindowProperty( owner, 'WGRP', 'POPU', sizeof( WindowRef ), &popup );
}

/*******************************************************************************
**	DisposePopupWindow
*******************************************************************************/
static void
DisposePopupWindow( WindowRef popup, WindowRef owner )
{
	WindowGroupRef	currGroup = GetWindowGroup( owner );
	
	// Put us back into our original group
	SetWindowGroup( owner, GetWindowGroupParent( currGroup ) );
	DisposeWindow( popup );
	ReleaseWindowGroup( currGroup );
	
	RemoveWindowProperty( owner, 'WGRP', 'POPU' );				
}

/*******************************************************************************
**	ApplicationEventHandler
*******************************************************************************/
OSStatus ApplicationEventHandler(
	EventHandlerCallRef		inHandlerCallRef,
	EventRef				inEvent,
	void					*inUserData )
{
	OSStatus 			theErr = eventNotHandledErr;
	HICommand			theHICommand;
	WindowRef			theNewWindow;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
		NULL, sizeof( HICommand ), NULL, &theHICommand );
	
	switch ( theHICommand.commandID )
	{
		case kNewGroupieCommand:
			MakeNewGroupieWindow( &theNewWindow );
			theErr = noErr;
			break;
	}

	return theErr;
}

/*******************************************************************************
**	MakeNewGroupieWindow
*******************************************************************************/
OSStatus MakeNewGroupieWindow(
	WindowRef*				outWindow )
{
    IBNibRef				theNibRef;
	OSStatus				theErr;
    const EventTypeSpec		kEvents[] = { { kEventClassCommand, kEventCommandProcess } };
	static int				theCreatedCount = 0;
	Str255					theTitle;
	
	// Create a nib reference
	theErr = CreateNibReference( CFSTR("main"), &theNibRef );
	require_noerr( theErr, CantGetNibRef );
	
	// Create a window.
    theErr = CreateWindowFromNib( theNibRef, CFSTR( "GroupieWindow" ), outWindow );
	
	// We don't need the nib reference anymore.
	DisposeNibReference( theNibRef );

	// Did we create a window?
	require_noerr( theErr, CantCreateWindow );

	// Title the window
	theCreatedCount++;
	sprintf( ( char* ) theTitle+1, "Groupie %d", theCreatedCount );
	theTitle[0] = strlen( ( char* ) theTitle+1 );
	SetWTitle( *outWindow, theTitle );

	// Cascade
	RepositionWindow( *outWindow, NULL, kWindowCascadeOnMainScreen );
	
	// The window was created hidden so show it.
    ShowWindow( *outWindow );
    
    // Initially, NOT in the group
	SetWRefCon( *outWindow, kIsNotAGroupie );
	
	// Install event handlers
	theErr = InstallWindowEventHandler( *outWindow, GroupieWindowEventHandler,
		GetEventTypeCount( kEvents ), kEvents, *outWindow, NULL );

CantGetNibRef:
CantCreateWindow:
	return theErr;
}

/*******************************************************************************
**	GroupieWindowEventHandler
*******************************************************************************/
OSStatus GroupieWindowEventHandler(
	EventHandlerCallRef		inHandlerCallRef,
	EventRef				inEvent,
	void					*inUserData )
{
	OSStatus 			theErr = eventNotHandledErr;
	HICommand			theHICommand;
	WindowRef			theWindow = ( WindowRef ) inUserData;

	GetEventParameter(
		inEvent, kEventParamDirectObject, typeHICommand, NULL,
		sizeof( HICommand ), NULL, &theHICommand );
	
	switch ( theHICommand.commandID )
	{
		case kGroupieCommand:
			if ( GetWRefCon( theWindow ) == kIsNotAGroupie )
			{
				// Not in the group, add it
				SetWindowGroup( theWindow, gGroupieWindowGroup );
				SetWRefCon( theWindow, kIsAGroupie );
			}
			else
			{
				// In the window group, remove it
				SetWindowGroup( theWindow, GetWindowGroupOfClass( kDocumentWindowClass ) );
				SetWRefCon( theWindow, kIsNotAGroupie );
			}
			theErr = noErr;
			break;
	}

	return theErr;
}

/*******************************************************************************
**	SetupWindowGroup
*******************************************************************************/
OSStatus SetupWindowGroup()
{
	OSStatus		theErr;
	WindowGroupRef	theWindowGroup;

	check( gGroupieWindowGroup == NULL );
	
	theErr = CreateWindowGroup( kWindowGroupAttrSelectAsLayer | kWindowGroupAttrMoveTogether |
			kWindowGroupAttrSharedActivation | kWindowGroupAttrHideOnCollapse, &theWindowGroup );
	require_noerr( theErr, CantCreateWindowGroup );

	gGroupieWindowGroup = theWindowGroup;

CantCreateWindowGroup:
	return theErr;
}

