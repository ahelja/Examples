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

#ifndef __TABHANDLING__
#include "TabHandling.h"
#endif

#define		TAB_ID 	 	128
#define		TAB_SIGNATURE	'tabs'

int		tabList[] = {3, 129, 130, 131};   // Tab UserPane IDs
int		lastTabIndex = 1;

// ----------------------------------------------------------------------
// Show the selected pane, hide the others.

void SelectItemOfTabControl(ControlRef tabControl)
{
    ControlRef userPane;
    ControlRef selectedPane = NULL;
    ControlID controlID;
    UInt16 i;

    SInt16 index = GetControlValue(tabControl);
    
    lastTabIndex = index;
    controlID.signature = TAB_SIGNATURE;

    for (i = 1; i < tabList[0] + 1; i++)
    {
        controlID.id = tabList[i];
        GetControlByID(GetControlOwner(tabControl), &controlID, &userPane);
       
        if (i == index) {
            selectedPane = userPane;
        } else {
            SetControlVisibility(userPane,false,false);
            DisableControl(userPane);
        }
    }
    
    if (selectedPane != NULL) {
        EnableControl(selectedPane);
        SetControlVisibility(selectedPane, true, true);
    }
    
    Draw1Control(tabControl);
}

// ----------------------------------------------------------------------
// Listen to events. Only switch if the tab value has changed.

pascal OSStatus TabEventHandler(EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
    OSStatus result = eventNotHandledErr;
    
    ControlRef theControl;
    ControlID controlID;
    
    GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof( ControlRef ), NULL, &theControl );
    
    GetControlID(theControl, &controlID);
    
    // If this event didn't trigger a tab change, give somebody else a chance to handle it.
    if (controlID.id == TAB_ID && GetControlValue(theControl) != lastTabIndex) {
        result = noErr;
        SelectItemOfTabControl(theControl);
    }    
    
    return result;
}

// ----------------------------------------------------------------------

void InstallTabHandler(WindowRef window)
{
    EventTypeSpec	controlSpec = { kEventClassControl, kEventControlHit }; // event class, event kind
    ControlRef 		tabControl;
    ControlID 		controlID;

    // Find the tab control and install an event handler on it.
    controlID.signature = TAB_SIGNATURE;
    controlID.id = TAB_ID;
    GetControlByID(window, &controlID, &tabControl);

    InstallEventHandler(GetControlEventTarget(tabControl),
                        NewEventHandlerUPP( TabEventHandler ),
                        1,
                        &controlSpec,
                        0,
                        NULL);

    //Select the active tab to start with.
    lastTabIndex = GetControlValue(tabControl);
    SelectItemOfTabControl(tabControl); 
}
    


