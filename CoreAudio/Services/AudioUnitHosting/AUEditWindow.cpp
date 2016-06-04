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
	AUEditWindow.cpp
	
=============================================================================*/

#include "AUEditWindow.h"
#include "XDebugging.h"

static const float kOffsetForAUView_X = 220;
static const float kOffsetForAUView_Y = 90;

AUEditWindowController::AUEditWindowController(XController *aSuper, IBNibRef nib) 
	: XController(aSuper) 
{
	mWindow = new AUEditWindow(this, nib, CFSTR("EditWindow"));
}
	
AUEditWindowController::~AUEditWindowController()
{
	delete mWindow;
}

void AUEditWindowController::SetUnitToDisplay (AudioUnit editUnit, bool forceGeneric)
{
	mWindow->SetUnitToDisplay (editUnit, forceGeneric);
}

void AUEditWindow::SetUnitToDisplay (AudioUnit editUnit, bool forceGeneric)
{
	CloseView ();

	ComponentDescription editorComponentDesc;
	
	// set up to use generic UI component
	editorComponentDesc.componentType = kAudioUnitCarbonViewComponentType;
	editorComponentDesc.componentSubType = 'gnrc';
	editorComponentDesc.componentManufacturer = 'appl';
	editorComponentDesc.componentFlags = 0;
	editorComponentDesc.componentFlagsMask = 0;
	
	OSStatus err;

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
	
	SetUnitToDisplay (editUnit, editorComponentDesc);
}

void AUEditWindow::CloseView ()
{
	if (mViewPaneResizer) {
		delete mViewPaneResizer;
		mViewPaneResizer = 0;
	}
	if (mEditView) {
		verify_noerr(CloseComponent(mEditView));
		mEditView = 0;
		mAUViewPane = 0;
	}
}

void AUEditWindow::SetUnitToDisplay (AudioUnit editUnit, ComponentDescription& inDesc)
{
	CloseView();
	
	mEditUnit = editUnit;

	Component editComp = FindNextComponent(NULL, &inDesc);
	
	verify_noerr(OpenAComponent(editComp, &mEditView));
	
	ControlRef rootControl;
	verify_noerr(GetRootControl(mWindow, &rootControl));

	Rect r;
	GetControlBounds(rootControl, &r);
	Float32Point location = { kOffsetForAUView_X, kOffsetForAUView_Y };
	Float32Point size = { Float32(r.right), Float32(r.bottom) };
	verify_noerr(AudioUnitCarbonViewCreate(mEditView, mEditUnit, mWindow, rootControl, &location, &size, &mAUViewPane));

	mViewPaneResizer = new ResizeControlHandler(this);
	EventTypeSpec resizeEvent[] = {
		{ kEventClassControl, kEventControlBoundsChanged },
	};
	
	mViewPaneResizer->WantEventTypes (GetControlEventTarget(mAUViewPane), GetEventTypeCount(resizeEvent), resizeEvent);

	AudioUnitCarbonViewSetEventListener(mEditView, EventListener, this);
	
	ResizeUI();
}

void	AUEditWindow::ResizeUI()
{
	Rect r;
	GetControlBounds(mAUViewPane, &r);

	Float32Point size;
	size.x = r.right-r.left; size.y = r.bottom-r.top;
	
	if (size.x == 0 || size.y == 0)
		return;
	
	size.x += kOffsetForAUView_X;
	size.y += kOffsetForAUView_Y;
	
	Rect r2;
	GetControlBounds (mResizeableControl->MacControl(), &r2);
	
	if ((r.bottom - r.top) < (r2.bottom - r2.top + 20))
		size.y = r2.bottom + 20;

	SetSize(size);	
}

AUEditWindow::AUEditWindow(XController *owner, IBNibRef nibRef, CFStringRef name) 
	: XWindow(owner, nibRef, name),
	  mEditView (0),
	  mAUViewPane (0),
	  mViewPaneResizer(0)
{
}

void 	AUEditWindow::SetResizeableControl (XControl* inControl) 
{ 
	mResizeableControl = inControl; 
}

AUEditWindow::~AUEditWindow()
{
	CloseView();
}

void AUEditWindow::EventListener(void *inUserData, AudioUnitCarbonView inView, 
	const AudioUnitParameter *inParameter, AudioUnitCarbonViewEventID inEvent, 
	const void *inEventParam)
{
	//printf("message %ld\n", inEvent);
}
