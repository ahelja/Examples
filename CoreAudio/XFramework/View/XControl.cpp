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
	XControl.cpp
	
=============================================================================*/

#include "XControl.h"
#include "XWindow.h"
#include "XDebugging.h"

#include <Carbon/Carbon.h>

XControl::XControl(XWindow *window, OSType signature, UInt32 id)
	:mListener(0)
{
	ControlID cid = { signature, id };
	RequireNoErrString(GetControlByID(window->MacWindow(), &cid, &mControl), "GetControlByID failed");
	SetControlReference(mControl, SInt32(this));
}

void	XControl::SetText(ConstStr255Param text)
{
	SetControlData(mControl, 0, kControlEditTextTextTag, text[0], &text[1]);
}

void	XControl::GetText(Str255 text)
{
	Size length = sizeof(text) - 1;
	GetControlData(mControl, 0, kControlEditTextTextTag, length, &text[1], &length);
	text[0] = length;
}

// crude but serviceable
int		XControl::GetTextAsInt()
{
	char str[64];
	Size length = sizeof(str) - 1;
	GetControlData(mControl, 0, kControlEditTextTextTag, length, str, &length);
	int value = -1;
	sscanf(str, "%d", &value);
	return value;
}

void	XControl::SetMenuHandle(MenuHandle menu)
{
	SetControlData(mControl, 0, kControlPopupButtonMenuHandleTag, sizeof(MenuHandle), &menu);
	SetControl32BitMaximum(mControl, CountMenuItems(menu));
}

void	XControl::SetNullControlAction ()
{
	::SetControlAction (mControl, SliderTrackProc);
}

void	XControl::SliderTrackProc(ControlRef theControl, ControlPartCode partCode)
{
	XControl* THIS = (XControl*)::GetControlReference(theControl);
	if (THIS->mListener)
		THIS->mListener->ControlValueChanged (THIS);
}

bool	XControl::HandleEvent(EventRef event)
{	
	if (!mListener) return false;
	
	UInt32 eclass = GetEventClass(event);
	UInt32 ekind = GetEventKind(event);
	ControlRef control;
	
	switch (eclass) {
	case kEventClassControl:
		switch (ekind) {
			case kEventControlValueFieldChanged:
			{
				GetEventParameter(event, 
									kEventParamDirectObject, 
									kEventParamControlRef, 
									NULL, 								
									sizeof(ControlRef), 
									NULL, 
									&control);
				mListener->ControlValueChanged (this);
				return true;
			}
		}
	}

	return false;
}

