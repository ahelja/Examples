/*
        IMPORTANT: This Apple software is supplied to you by Apple Computer,
        Inc. ("Apple") in consideration of your agreement to the following terms,
        and your use, installation, modification or redistribution of this Apple
        software constitutes acceptance of these terms.  If you do not agree with
        these terms, please do not use, install, modify or redistribute this Apple
        software.
        
        In consideration of your agreement to abide by the following terms, and
        subject to these terms, Apple grants you a personal, non-exclusive
        license, under Apple’s copyrights in this original Apple software (the
        "Apple Software"), to use, reproduce, modify and redistribute the Apple
        Software, with or without modifications, in source and/or binary forms;
        provided that if you redistribute the Apple Software in its entirety and
        without modifications, you must retain this notice and the following text
        and disclaimers in all such redistributions of the Apple Software.
        Neither the name, trademarks, service marks or logos of Apple Computer,
        Inc. may be used to endorse or promote products derived from the Apple
        Software without specific prior written permission from Apple. Except as
        expressly stated in this notice, no other rights or licenses, express or
        implied, are granted by Apple herein, including but not limited to any
        patent rights that may be infringed by your derivative works or by other
        works in which the Apple Software may be incorporated.
        
        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
        NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
        IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
        PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
        ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
        
        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
        SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
        MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
        WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
        LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
        OF SUCH DAMAGE.  
*/

#include <Carbon/Carbon.h>
#include "mainWindowHandler.h"
#include "modalDialogHandler.h"

// Reference to our main window
WindowRef 		gMainWindow;

OSStatus CreateMainWindow()
{
    IBNibRef 		nibRef;
    EventTypeSpec 	mainSpec = {kEventClassCommand, kEventCommandProcess };

    OSStatus		err;

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
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &gMainWindow);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // Attach our handler to the window.
    err =  InstallWindowEventHandler (gMainWindow, NewEventHandlerUPP (MainWindowEventHandler), 
                                    1, &mainSpec, (void *) gMainWindow, NULL);
    
    // The window was created hidden so show it.
    ShowWindow( gMainWindow );

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
    
    return err;
}

// Window event handler
pascal OSStatus MainWindowEventHandler (EventHandlerCallRef myHandler, EventRef event, void *userData)
{
    OSStatus 		result = eventNotHandledErr;
    HICommand		command;

    // Get the HI Command
    GetEventParameter (event, kEventParamDirectObject, typeHICommand, NULL, 
                            sizeof (HICommand), NULL, &command);
    // Look for our SHOW command
    switch (command.commandID)
    {
        case 'SHOW':
                CreateDialogWindow();  // Show the dialog.
                result = noErr;
                break;
    }
    
    //Return how we handled the event.
    return result;
}

// The dialog handler will call this function when the user clicks
// a button.
void HandleResponse(bool response)
{
    ControlRef 		staticTextControl;
    ControlID 		staticTextControlID;
    OSStatus		err;
    CFStringRef		text;
    
    // Find our text control
    staticTextControlID.signature = 'DEMO';
    staticTextControlID.id = 131;
    
    err = GetControlByID(gMainWindow, &staticTextControlID, &staticTextControl);
    require_noerr( err, CantFindTextControl );

    // Generate a response
    if (response == TRUE)
        text = CFSTR("Just make sure it's not annoying!");
    else
        text = CFSTR("That's good.");
    
    // Change the display string
    SetControlData(staticTextControl, 0,kControlStaticTextCFStringTag, sizeof(CFStringRef), &text);
    
CantFindTextControl:
    return;
}

