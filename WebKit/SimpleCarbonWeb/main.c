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

#include "Prompt.h"
#include "WebWindow.h"

const EventTypeSpec kCommandEvents[] = {
	{ kEventClassCommand, kEventCommandProcess },
	{ kEventClassCommand, kEventCommandUpdateStatus }
};

static OSStatus		CommandHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );

int main(int argc, char* argv[])
{
    IBNibRef 			nibRef;
    OSStatus			err;
	
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
  
	// Fire up the Web Kit
	WebInitForCarbon();
 
	// Install an event handler to handle the open command
	InstallEventHandler( GetApplicationEventTarget(), CommandHandler,
		GetEventTypeCount( kCommandEvents ), kCommandEvents, 0, NULL );

	// Run the application
    RunApplicationEventLoop();

CantSetMenuBar:
CantGetNibRef:
	return err;
}

//-------------------------------------------------------------------------------------
//	CommandHandler
//-------------------------------------------------------------------------------------
//	Handle an application-level command.
//
static OSStatus
CommandHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	HICommand		command;
	OSStatus		result = eventNotHandledErr;
	bool			process = false;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
			sizeof( HICommand ), NULL, &command );
	
	process = (GetEventKind( inEvent ) == kEventCommandProcess);

	switch ( command.commandID )
	{
		case kHICommandOpen:
			if ( process )
			{
				CFStringRef		url = Prompt( CFSTR( "Enter a fully qualified URL:" ),
											  CFSTR( "http://" ) );
				if ( url )
				{
					OpenWebWindow( url );
					CFRelease( url );
				}
			}
			result = noErr;
			break;

		case kHICommandClose:
			if ( !process )
			{
				// By default, the close command is disabled. Each window controls
				// whether or not it can be closed via the command. In Panther, we
				// will have a facility to declare menu items as auto-disabled.
				if ( command.attributes & kHICommandFromMenu )
					DisableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
				result = noErr;
			}
			break;
	}
	
	return result;
}
