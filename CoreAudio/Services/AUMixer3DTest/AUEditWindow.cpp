/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

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
/*=============================================================================
	AUEditWindow.cpp
	
=============================================================================*/
#include <CoreServices/CoreServices.h>
#include "AUEditWindow.h"
#include "XDebugging.h"

AUEditWindowController::AUEditWindowController(XController *aSuper, IBNibRef nib, AudioUnit au, ComponentDescription inCompDesc) : 
	XController(aSuper) 
{
	mWindow = new AUEditWindow(this, nib, CFSTR("Window") /*CFSTR("EditWindow")*/, au, inCompDesc );
}

AUEditWindowController::~AUEditWindowController()
{
	delete mWindow;
}



AUEditWindow::AUEditWindow(XController *owner, IBNibRef nibRef, CFStringRef name, AudioUnit editUnit, ComponentDescription inCompDesc ) :
	XWindow(owner, nibRef, name),
	mEditUnit(editUnit)
{
	Component editComp = FindNextComponent(NULL, &inCompDesc);
	
	verify_noerr(OpenAComponent(editComp, &mEditView));
	
	ControlRef rootControl;
	verify_noerr(GetRootControl(mWindow, &rootControl));

	ControlRef customControl = 0;

	ControlID controlID;
	controlID.signature    	= 'cust';
    controlID.id        	= 1000;
    GetControlByID( mWindow, &controlID, &customControl );

	ControlRef ourControl = customControl ? customControl : rootControl;

	Rect r = {0,0,400,400};
	ControlRef viewPane;
	if(customControl) GetControlBounds(ourControl, &r);

	Float32Point location = { r.left, r.top };
	Float32Point size = { Float32(r.right - r.left ), Float32(r.bottom - r.top ) };
	
	
	verify_noerr(AudioUnitCarbonViewCreate(mEditView, mEditUnit, mWindow, ourControl, &location, &size, &viewPane));

	GetControlBounds(viewPane, &r);
	size.x = r.right-r.left; size.y = r.bottom-r.top;
	if(!customControl) SetSize(size);
	Show();	
}

void	AUEditWindow::TimerProc(EventLoopTimerRef inTimer, void *inUserData)
{
	AUEditWindow *This = (AUEditWindow *)inUserData;
	verify_noerr(CloseComponent(This->mEditView));
	This->mEditView = NULL;
}

AUEditWindow::~AUEditWindow()
{
	verify_noerr(CloseComponent(mEditView));
}
