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

#include "AUEditWindow.h"
#include "XDebugging.h"
#include <AudioUnit/AudioUnitProperties.h>

AUEditWindowController::AUEditWindowController(XController *aSuper, IBNibRef nib, AudioUnit au, bool forceGeneric) : 
	XController(aSuper) 
{
	mWindow = new AUEditWindow(this, nib, CFSTR("EditWindow"), au, forceGeneric);
}

AUEditWindowController::~AUEditWindowController()
{
	delete mWindow;
}

AUEditWindow::AUEditWindow(XController *owner, IBNibRef nibRef, CFStringRef name, AudioUnit editUnit, bool forceGeneric) :
	XWindow(owner, nibRef, name),
	mEditUnit(editUnit)
{
	OSStatus err;
	ComponentDescription editorComponentDesc;
	
	// set up to use generic UI component
	editorComponentDesc.componentType = kAudioUnitCarbonViewComponentType;
	editorComponentDesc.componentSubType = 'gnrc';
	editorComponentDesc.componentManufacturer = 'appl';
	editorComponentDesc.componentFlags = 0;
	editorComponentDesc.componentFlagsMask = 0;
	
	if (!forceGeneric) {
		// ask the AU for its first editor component
		UInt32 propertySize;
		err = AudioUnitGetPropertyInfo(editUnit, kAudioUnitProperty_GetUIComponentList,
			kAudioUnitScope_Global, 0, &propertySize, NULL);
		if (!err) {
			int nEditors = propertySize / sizeof(ComponentDescription);
			ComponentDescription *editors = new ComponentDescription[nEditors];
			err = AudioUnitGetProperty(editUnit, kAudioUnitProperty_GetUIComponentList,
				kAudioUnitScope_Global, 0, editors, &propertySize);
			if (!err)
				// just pick the first one for now
				editorComponentDesc = editors[0];
			delete[] editors;
		}
	}
	Component editComp = FindNextComponent(NULL, &editorComponentDesc);
	
	verify_noerr(OpenAComponent(editComp, &mEditView));
	
	ControlRef rootControl;
	verify_noerr(GetRootControl(mWindow, &rootControl));

	Rect r;
	ControlRef viewPane;
	GetControlBounds(rootControl, &r);
	Float32Point location = { 0., 0. };
	Float32Point size = { Float32(r.right), Float32(r.bottom) };
	verify_noerr(AudioUnitCarbonViewCreate(mEditView, mEditUnit, mWindow, rootControl, &location, &size, &viewPane));
	
	AudioUnitCarbonViewSetEventListener(mEditView, EventListener, this);

	GetControlBounds(viewPane, &r);
	size.x = r.right-r.left; size.y = r.bottom-r.top;
	SetSize(size);
	Show();

/*	EventLoopTimerRef timer;
	RequireNoErr(
		InstallEventLoopTimer(
			GetMainEventLoop(), 5., 0., TimerProc, this, &timer));*/
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

void AUEditWindow::EventListener(void *inUserData, AudioUnitCarbonView inView, 
	const AudioUnitParameter *inParameter, AudioUnitCarbonViewEventID inEvent, 
	const void *inEventParam)
{
	//printf("message %ld\n", inEvent);
}
