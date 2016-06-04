/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
/*=============================================================================
	XApp.cpp
	
=============================================================================*/

#include "XApp.h"
#include "XDebugging.h"

XApp::XApp(CFStringRef nibName) :
	mMainNib(NULL)
{
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	RequireNoErrString(::CreateNibReference(nibName, &mMainNib), "can't create nib reference");

	EventTypeSpec events[] = {
		{ kEventClassCommand, kEventProcessCommand }
	};
	
	WantEventTypes(GetApplicationEventTarget(), GetEventTypeCount(events), events);
}

XApp::~XApp()
{
	// We don't need the nib reference anymore.
	if (mMainNib)
		::DisposeNibReference(mMainNib);
	::ExitToShell();
}

void	XApp::Initialize()
{
	CreateMenus();
	StartupAction();
}

#if 0
void	XApp::InstallEventHandlers()
{
	EventTypeSpec	eventSpec = { kEventClassCommand, kEventProcessCommand };	
  	
	// handler for the menu bar
	eventSpec.eventClass = kEventClassCommand;
	eventSpec.eventKind = kEventProcessCommand;
	InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP((EventHandlerProcPtr) MenuEventHandler), 
						1, &eventSpec, this, NULL);


	// handler for all the non menu bar application events
	eventSpec.eventClass = kEventClassApplication;
	eventSpec.eventKind = kEventAppActivated;
	InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP((EventHandlerProcPtr) AppEventHandler), 
						1, &eventSpec, this, NULL);
}

bool	XApp::HandleAppEvent(EventRef inEvent)
{
    UInt32					eventKind = GetEventKind(inEvent);

    switch (eventKind) {
	}
	return true;
}

bool	XApp::HandleMenuEvent(EventRef inEvent)
{
	return false;
}


#endif

void	XApp::CreateMenus()
{
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	RequireNoErrString(SetMenuBarFromNib(mMainNib, CFSTR("MenuBar")), "can't create menu bar");
}

int		XApp::Run()
{
	// Call the event loop
	::RunApplicationEventLoop();
	return noErr;
}

#if 0
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
pascal OSErr AppEventHandler(EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	XApp *app = (XApp *)inUserData;
	return app->HandleAppEvent(inEvent) ? noErr : eventNotHandledErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
pascal OSErr MenuEventHandler(EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	XApp *app = (XApp *)inUserData;
	return app->HandleMenuEvent(inEvent) ? noErr : eventNotHandledErr;

//	OSStatus				err = noErr;
//	HICommand				command;   
   
//	require_noerr(GetEventParameter(inEvent,kEventParamDirectObject,typeHICommand,NULL,sizeof(HICommand),NULL,&command), errexit);   
}
#endif
