/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
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

// Structure used for communication between our prompt function
// and the alert panel event handler
typedef struct
{
	WindowRef		window;
	CFStringRef		string;
} PanelInfo;

static OSStatus	InputPanelHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );

static const ControlID	kPromptLabelID = { 'INPU', 1 };
static const ControlID	kTextFieldID = { 'INPU', 2 };

//-------------------------------------------------------------------------------------
//	Prompt
//-------------------------------------------------------------------------------------
//	Put up a modal panel and request some text.
//
CFStringRef
Prompt( CFStringRef inPrompt, CFStringRef inDefaultText )
{
    IBNibRef 			nibRef;
	OSStatus			err;
	WindowRef			window;
	EventTypeSpec		kEvents[] = { { kEventClassCommand, kEventCommandProcess } };
	PanelInfo			info;
	HIViewRef			view;
	
	info.window = window;
	info.string = NULL;

    err = CreateNibReference( CFSTR( "main" ), &nibRef );
    require_noerr( err, CantGetNibRef );
	
	err = CreateWindowFromNib( nibRef, CFSTR( "Prompt" ), &window );
	require_noerr( err, CantCreateWindow );
	
	DisposeNibReference( nibRef );

	if ( inPrompt )
	{
		HIViewFindByID( HIViewGetRoot( window ), kPromptLabelID, &view );
		SetControlData( view, 0, kControlStaticTextCFStringTag, sizeof( CFStringRef ), &inPrompt );
	}
	
	HIViewFindByID( HIViewGetRoot( window ), kTextFieldID, &view );

	if ( inDefaultText )
		SetControlData( view, 0, kControlEditTextCFStringTag, sizeof( CFStringRef ), &inDefaultText );	

	SetKeyboardFocus( window, view, kControlFocusNextPart );

	InstallWindowEventHandler( window, InputPanelHandler, GetEventTypeCount( kEvents ),
				kEvents, &info, NULL );	
	
	ShowWindow( window );
	
	info.window = window;

	RunAppModalLoopForWindow( window );

	DisposeWindow( window );

CantCreateWindow:
CantGetNibRef:
	return info.string;
}

//-------------------------------------------------------------------------------------
//	InputPanelHandler
//-------------------------------------------------------------------------------------
//	Deal with events in our prompt panel. We merely respond to the cancel and OK commands
// 	that are sent from the push buttons and terminate our modal loop. If OK is pressed,
//	we get the string and store it in our PanelInfo structure.
//
static OSStatus
InputPanelHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	HICommand			command;
	OSStatus			result = eventNotHandledErr;
	PanelInfo*			info = (PanelInfo*)inUserData;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
			sizeof( HICommand ), NULL, &command );

	if ( command.commandID == kHICommandCancel )
	{
		QuitAppModalLoopForWindow( info->window );
		result = noErr;
	}
	else if ( command.commandID == kHICommandOK )
	{
		HIViewRef		textField;
		
		HIViewFindByID( HIViewGetRoot( info->window ), kTextFieldID, &textField );
		GetControlData( textField, 0, kControlEditTextCFStringTag, sizeof( CFStringRef ), &info->string, NULL );
		QuitAppModalLoopForWindow( info->window );
	}
	
	return result;
}
