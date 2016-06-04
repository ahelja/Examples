/*
 *  main.cpp
 *  MLTEShowcase
 *
 *  Created on Mon Apr 26 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 *	main
 
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
 */

#include <Carbon/Carbon.h>

#include "MLTEShowcaseDefines.h"
#include "MLTEShowcaseUtils.h"
#include "TextView.h"
#include "HelpTextFrame.h"


// Prototypes
pascal OSStatus CommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
static OSStatus NewWindow();
static OSStatus InstallApplicationEventHandlers( void );
pascal OSStatus WindowFocusAcquired(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
static OSStatus UpdateControlsFromTextViewWindow( WindowRef window );

// Main entry point
int
main(int argc, char* argv[])
{
    IBNibRef nibRef;
    WindowRef window;
    OSStatus status;
		
   // Can we run this particular demo application?
	long response;
	status = Gestalt(gestaltSystemVersion, &response);
	Boolean ok = ((status == noErr) && (response >= 0x00001030));
	if (!ok)
	{
		StandardAlert(kAlertStopAlert, (const StringPtr)"\pMac OS X 10.3 (minimum) is required for this application", (const StringPtr)"\p", NULL, NULL);
		ExitToShell();
	}
	
	// Install application-wide event handlers
	status = InstallApplicationEventHandlers();
    require_noerr( status, CantInstallEventHandlers );

    // Get a NIB reference and install the main menu bar
    status = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( status, CantGetNibRef );

    status = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( status, CantSetMenuBar );

    DisposeNibReference(nibRef);

	// Create the first demo window
	status = NewWindow();
	
    // Call the event loop
    RunApplicationEventLoop();

CantInstallEventHandlers:
CantSetMenuBar:
CantGetNibRef:
	return status;
}

// Set up application-wide event handlers
OSStatus
InstallApplicationEventHandlers( void )
{
	EventTypeSpec commandEventType = {kEventClassCommand, kEventCommandProcess};
	OSStatus status;
	
	status = InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(CommandProcess), 1, &commandEventType, NULL, NULL);
    require_noerr( status, CantInstallHICommandHandler );
	
CantInstallHICommandHandler:
	return status;
}

// Create a new demo window
OSStatus
NewWindow()
{
    IBNibRef nibRef;
    WindowRef window;
	EventTypeSpec commandEventType = {kEventClassWindow, kEventWindowFocusAcquired};
    OSStatus status = noErr;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
	status = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( status, CantGetNibRef );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    status = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( status, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
	
	// Let's customize the HITextView in that window
	status = SetUpTheTextView( window );	
	check_noerr( status );

    // The window was created hidden so show it.
    ShowWindow( window );

	// by default, focus the HITextView at start
	status = TextViewFocusInWindow( window );
    require_noerr( status, CantFocusTextView );

	// set up the control state from the state of the text view
	status = UpdateControlsFromTextViewWindow( window );
    require_noerr( status, CantSetControlState );

	// Install a handler to update the UI when this window comes to the front
	status = InstallEventHandler(GetWindowEventTarget(window), NewEventHandlerUPP(WindowFocusAcquired), 1, &commandEventType, NULL, NULL);
    require_noerr( status, CantInstallWindowEventHandler );
	
CantFocusTextView:
CantSetControlState:
CantInstallWindowEventHandler:
CantCreateWindow:
CantGetNibRef:
	return status;
}

// Event handling for window focus acquired events
pascal OSStatus
WindowFocusAcquired(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
#pragma unused (nextHandler, userData)
	WindowRef window;
	OSStatus status = noErr;

	GetEventParameter(theEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window);
	require_noerr( status, CantGetEventParameter);

	if ( window != NULL )
	{
		HIViewRef textView;
		status = GetTextViewFromWindow(window, textView);
		
		if ( status == noErr )
		{
			UniChar mark; // For checking menu items

			// Handle font panel menu item and palette
			if ( TextViewIsFontPanelSupportEnabled(textView) ) {
				EnableMenuCommand(NULL, kHICommandShowHideFontPanel);
				if ( ! FPIsFontPanelVisible() )
					verify_noerr( FPShowHideFontPanel() );
			} else {
				DisableMenuCommand(NULL, kHICommandShowHideFontPanel);
				if ( FPIsFontPanelVisible() )
					verify_noerr( FPShowHideFontPanel() );
			}
			
			// Handle spelling menu items
			if ( TextViewIsSpellingSupportEnabled(textView) ) {
				EnableMenuCommand(NULL, kHICommandShowSpellingPanel);
				EnableMenuCommand(NULL, kToggleAutoSpellcheckCommand);
				MyShowSpellCheckPanel();
			} else {
				DisableMenuCommand(NULL, kHICommandShowSpellingPanel);
				DisableMenuCommand(NULL, kToggleAutoSpellcheckCommand);
			}
				
			// Auto-spellcheck menu item
			mark = ( TXNGetSpellCheckAsYouType(HITextViewGetTXNObject(textView)) ) ? kMenuCheckmarkGlyph : kMenuNullGlyph;
			verify_noerr( SetMenuCommandMark(NULL, kToggleAutoSpellcheckCommand, mark) );
		}
	}

CantGetEventParameter:
	return status;
}

// Resets the state of the controls in a window based on the state of the TextView
OSStatus UpdateControlsFromTextViewWindow( WindowRef window )
{
	OSStatus status;
	HIViewRef textView;
	ControlRef control;
	SInt32 newValue;
	UInt32 unsignedValue;

	status = GetTextViewFromWindow(window, textView);
	require_noerr( status, CantGetTextView );

	// Font panel checkbox
	status = GetControlByID(window, &kFontPanelCheckboxControl, &control);
	require_noerr( status, CantGetControl );
	newValue = TextViewIsFontPanelSupportEnabled(textView) ? kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue;
	SetControl32BitValue(control, newValue);
	
	// Spelling checkbox
	status = GetControlByID(window, &kSpellCheckboxControl, &control);
	require_noerr( status, CantGetControl );
	newValue = TextViewIsSpellingSupportEnabled(textView) ? kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue;
	SetControl32BitValue(control, newValue);
	
	// Autoscroll radio button
	status = GetControlByID(window, &kAutoScrollRadioButtonControl, &control);
	require_noerr( status, CantGetControl );
	status = TextViewGetObjectControlData( textView, kTXNAutoScrollBehaviorTag, kUnsigned, newValue, unsignedValue );
	require_noerr( status, CantGetValue );
	newValue += 1; // Autoscroll constants are zero-based, radio button state is one-based
	SetControl32BitValue(control, newValue);

CantGetValue:
CantGetControl:
CantGetTextView:
	return status;
}

// Event handling for HICommand events
pascal OSStatus
CommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
#pragma unused (nextHandler, userData)
	WindowRef window;
	HIViewRef textView;
	HICommandExtended inCommand;
	OSStatus status = noErr;

	status = GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommandExtended), NULL, &inCommand);
	require_noerr( status, CantGetEventParameter);

	// Check to see if this came from one of the buttons
	if ( inCommand.attributes & kHICommandFromControl ) {
		ControlRef hitControl;

		// Get the control and the window & text view associated with it
		hitControl = inCommand.source.control;
		window = HIViewGetWindow(hitControl);
		status = GetTextViewFromWindow(window, textView);
		if ( status != noErr ) return status;

		// The button controls need the text view to be the active focus
		// in order for the HICommands to be correctly dispatched.
		// So we will just force the TextView to be the focus before handling.
		// the HICommand.
		status = TextViewFocusInWindow( HIViewGetWindow(hitControl) );
		if ( status != noErr ) return status;
		
		switch (inCommand.commandID)
		{
			case kDefaultHITextViewCommand:		// **** Reset button				
				status = TextViewDefaultSettings( textView );
				if (status == noErr)
					status = UpdateControlsFromTextViewWindow(window);
				if (status == noErr)
					status = TextViewFocusInWindow( window );
				break;

			case kFontPanelSupportCommand:		// **** Checkboxes
				if ( GetControl32BitValue(hitControl) == kControlCheckBoxCheckedValue )
					status = TextViewDemoFontPanelSupport( textView );
				else
					status = TextViewFontPanelSupport(textView, false);
				break;
			case kDemoSpellingSupportCommand:
				if ( GetControl32BitValue(hitControl) == kControlCheckBoxCheckedValue )
					status = TextViewDemoSpellingSupport( textView );
				else
					status = TextViewSpellingSupport(textView, false);
				break;
				
			case kDemoAutoScrollCommand:		// **** Radio button
				// The radio button values correspond to the MLTE auto scroll constants
				// plus one (the constants start from zero, the radio button values
				// start from one):
				//
				// (radio button 1)  kTXNAutoScrollInsertionIntoView     == 0
				// (radio button 2)  kTXNAutoScrollNever                 == 1
				// (radio button 3)  kTXNAutoScrollWhenInsertionVisible  == 2
				//
				// See the definition of the type TXNAutoScrollBehavior for
				// more information on each constant and its associated behavior.
				status =  TextViewScrollingOptions( textView, (UInt32)(GetControl32BitValue(hitControl) - 1) );
				break;

			case kDemoActionGrouping:			// **** Demo actions
				status = TextViewDemoActionGroup( textView );
				break;
			case kHICommandSave:
			case kDemoCFURLWriteCommand:
				status = TextViewDemoWriteToCFURL( textView );
				break;
			case kDemoCFURLReadCommand:
				status = TextViewDemoReadFromCFURL( textView );
				break;
			default:
				status = eventNotHandledErr;
				break;
		}
	}
	else {
		switch (inCommand.commandID)
		{
			case kHICommandNew:
				status = NewWindow();
				break;
			case kHICommandOpen:
				window = GetFrontWindowOfClass(kDocumentWindowClass, true);
				if ( window != NULL )
				{
					status = GetTextViewFromWindow(window, textView);
					if ( status == noErr )
						status = TextViewDemoReadFromCFURL( textView );
				}
				break;
			default:
				status = eventNotHandledErr;
				break;
		}
	}

CantGetEventParameter:
	return status;
}
