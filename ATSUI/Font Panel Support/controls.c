/*

File: controls.c

Abstract: Contains event handling code for ATSUIFontPanelDemo. See the
function DoFontPanelSelectionEvent() for the main part of the font panel
event handling code. See also atsui.c in this sample for helper functions
such as ATSUIStuffSetFont(), ATSUIStuffSetSize(), and
ATSUIStuffSetDictionary().

Version: <1.0>

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2004 Apple Computer, Inc., All Rights Reserved

*/


#include "atsui.h"
#include "globals.h"
#include "controls.h"

// Sets up controls and their event handlers
//
OSStatus SetUpControls(void)
{
    ControlID				stringInputControlID = { 'TEXT', 0 };
    ControlID				updateButtonControlID = { 'UPDT', 0 };
    EventTypeSpec			eventType;
    OSStatus				err;
    int						i;

    // **** Text Control window
    err = GetControlByID(gTextPalette, &stringInputControlID, &gStringInputControl);
    require_noerr( err, CantGetControl );
    err = GetControlByID(gTextPalette, &updateButtonControlID, &gUpdateButtonControl);
    require_noerr( err, CantGetControl );
 
    // Install an event handler for the controls
    eventType.eventClass = kEventClassControl;
    eventType.eventKind  = kEventControlHit;
    err = InstallApplicationEventHandler(NewEventHandlerUPP(DoControlHitEvent), 1, &eventType, NULL, NULL);
    require_noerr( err, ContInstallEventHandler );

    // Install an event handler for the font panel selection
    eventType.eventClass = kEventClassFont;
    eventType.eventKind  = kEventFontSelection;
    err = InstallApplicationEventHandler(NewEventHandlerUPP(DoFontPanelSelectionEvent), 1, &eventType, NULL, NULL);
    require_noerr( err, ContInstallEventHandler );

    // Install an event handler for the font panel close events
    eventType.eventClass = kEventClassFont;
    eventType.eventKind  = kEventFontPanelClosed;
    err = InstallApplicationEventHandler(NewEventHandlerUPP(DoFontPanelCloseEvent), 1, &eventType, NULL, NULL);
    require_noerr( err, ContInstallEventHandler );

ContInstallEventHandler:
CantGetControl:
    return err;
}


// Checks for updates to control
//
pascal OSStatus DoControlHitEvent(EventHandlerCallRef nextHandler, EventRef theEvent, void *userData)
{
    Boolean needsRedrawing;
    OSStatus status;
    ControlRef thisControl;
    CFStringRef	editString;
    int i;
    
    // Figure out which control this came from
    status = GetEventParameter(theEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &thisControl);
    require_noerr( status, CantGetEventParameter );

    status = eventNotHandledErr;
    
    // **** Text Control window

    // "Update" button
    if ( thisControl == gUpdateButtonControl ) {
        // Get the string from the user
        verify_noerr( GetControlData(gStringInputControl, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), (void *)&editString, NULL) );
        ATSUIStuffSetText(editString);
        needsRedrawing = true;
        status = noErr;
		// According to the comment under "Core Event Types" in the header "CarbonEventsCore.h", the
		// CFString will be released when the EventRef is destroyed, so there is no need release it here.
    }

    // Update the display if need be
    if ( needsRedrawing )
        ATSUIStuffDraw(GetWindowPort(gMainWindow));

CantGetEventParameter:
    return status;
}


// Handles changes to the font panel
//
pascal OSStatus DoFontPanelSelectionEvent(EventHandlerCallRef nextHandler, EventRef theEvent, void *userData)
{
    Boolean needsRedrawing = false;
    ATSUFontID font = kATSUInvalidFontID;
    CFDictionaryRef dict;
    Fixed size;
    OSStatus err;
    
    // Check to see if the font has changed
    err = GetEventParameter(theEvent, kEventParamATSUFontID, typeATSUFontID, NULL, sizeof(ATSUFontID), NULL, &font);
    require( ((err == noErr) || ( err == eventParameterNotFoundErr )), CantGetEventParameter );
    if ( err == noErr ) {
        ATSUIStuffSetFont(font);
        needsRedrawing = true;
    }

    // Check to see if the size has changed
    err = GetEventParameter(theEvent, kEventParamATSUFontSize, typeATSUSize, NULL, sizeof(Fixed), NULL, &size);
    require( ((err == noErr) || ( err == eventParameterNotFoundErr )), CantGetEventParameter );
    if ( err == noErr ) {
        ATSUIStuffSetSize(size);
        needsRedrawing = true;
    }

    // Check to see if the CFDictionary has been returned
	// This dictionary contains font variations, features, color, and drop shadow effects.
    err = GetEventParameter(theEvent, kEventParamDictionary, typeCFDictionaryRef, NULL, sizeof(CFDictionaryRef), NULL, &dict);
    require( ((err == noErr) || ( err == eventParameterNotFoundErr )), CantGetEventParameter );
    if ( err == noErr ) {
        ATSUIStuffSetDictionary(dict);
        needsRedrawing = true;
		// According to the comment under "Core Event Types" in the header "CarbonEventsCore.h", the
		// dictionary will be released when the EventRef is destroyed, so there is no need release it here.
    }

    // Update the display
    if ( needsRedrawing )
        ATSUIStuffDraw(GetWindowPort(gMainWindow));

CantGetEventParameter:
    return err;
}


// Handles close events for the font panel
//
pascal OSStatus DoFontPanelCloseEvent(EventHandlerCallRef nextHandler, EventRef theEvent, void *userData)
{
    // Show the font panel again.
    // Normally an app would have a "show font panel" menu item to toggle its visibility.
	// We want to force it to be visible at all times for the purposes of this demo.
    if ( ! FPIsFontPanelVisible() ) FPShowHideFontPanel();
    
    return noErr;
}
