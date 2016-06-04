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
/*
 *  AULoadCPU.cpp
 *  CAServices
 *
 *  Created by Michael Hopkins on Thu Oct 24 2002.
 *  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 */

#include "AULoadCPU.h"
#include "AUViewLocalizedStringKeys.h"

#define kToggleRestrictCPULoadCmd 	'rcpu'
#define kChangeCPULoadCmd		 	'ccpu'
#define kIncrementCPULoadCmd		'+cpu'
#define kDecrementCPULoadCmd		'-cpu'

#define GV_GUTTER_MARGIN			10
#define GV_TEXT_SPACING				5
#define GV_HORIZONTAL_SPACING		10
#define GV_VERTICAL_SPACING			5
#define GV_PROPERTY_INDENTATION		64

#define kLabelAndSliderSpacing		4
#define kSliderThinDimension 		16

static CFStringRef kStringRestrictCPULoad = kAUViewLocalizedStringKey_RestrictCPULoad;
static CFStringRef kStringPercentSymbol = kAUViewLocalizedStringKey_PercentSymbol;
static bool sLocalized = false;

/*** FUNCTION PROTOTYPES ***/
void SetControlStringAndValue(ControlRef control, int value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AULoadCPU::AULoadCPU
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AULoadCPU::AULoadCPU (AUCarbonViewBase *inBase, 
				Point 					inLocation, 
				ControlFontStyleRec & 	inFontStyle)
	: AUPropertyControl (inBase)
{
    if (!sLocalized) {
        CFBundleRef bundle = CFBundleGetBundleWithIdentifier(kLocalizedStringBundle_AUView);
        if (bundle) {
            kStringRestrictCPULoad = CFCopyLocalizedStringFromTableInBundle(
                kAUViewLocalizedStringKey_RestrictCPULoad, kLocalizedStringTable_AUView, bundle,
                CFSTR("Restrict CPU Load button title"));
            kStringPercentSymbol = CFCopyLocalizedStringFromTableInBundle(
                kAUViewLocalizedStringKey_PercentSymbol, kLocalizedStringTable_AUView, bundle,
                CFSTR("% symbol"));
            sLocalized = true;
        }
	}
    
	CFMutableStringRef cpuLoadTitle = CFStringCreateMutable(NULL, 0);
	CFStringAppend(cpuLoadTitle, kStringRestrictCPULoad);
	CFStringAppend(cpuLoadTitle, kAUViewUnlocalizedString_TitleSeparator);
    
	// [1] create checkbox
	Rect r;
	r.top = inLocation.v;
	r.bottom = r.top;
	r.left = inLocation.h;
	r.right = r.left;
	
	OSErr err = CreateCheckBoxControl(	GetCarbonWindow(), 
                                        &r, 
                                        cpuLoadTitle, 
                                        0, 
                                        true, 
                                        &mNoCPURestrictionsBtn);
	CFRelease(cpuLoadTitle);
	if (err != noErr)
		return;
    
	ControlSize smallSize = kControlSizeSmall;
	verify_noerr (SetControlData (mNoCPURestrictionsBtn, kControlEntireControl, kControlSizeTag, sizeof (ControlSize), &smallSize));
	verify_noerr (SetControlFontStyle (mNoCPURestrictionsBtn, &inFontStyle));
	
	SInt16 width, height;
	AUCarbonViewControl::SizeControlToFit(mNoCPURestrictionsBtn, &width, &mHeight);
	
	SetControlCommandID (mNoCPURestrictionsBtn, kToggleRestrictCPULoadCmd);
	SetControl32BitValue (mNoCPURestrictionsBtn, 1);

	EmbedControl(mNoCPURestrictionsBtn);
    
	// [2] create spinner control
	// get the current value of the property
	Float32 theCPULoad = 0;
	UInt32 size = sizeof(Float32);
	AudioUnitGetProperty(mView->GetEditAudioUnit(), kAudioUnitProperty_CPULoad, kAudioUnitScope_Global, 0, &theCPULoad, &size);
	char text[64];
	
	sprintf(text, "%.0f", theCPULoad);
	
	// move rectangle's x-position to the right of the titled checkbox & add spacing
	r.left = r.right + width;
	r.right = r.left;
	r.top -= 1;
	
	CreateLittleArrowsControl (GetCarbonWindow(), &r, (long)theCPULoad * 100, 0, 100, 5, &mLittleArrowsBtn);
	
	AUCarbonViewControl::SizeControlToFit(mLittleArrowsBtn, &width, &height);
	if (height > mHeight) mHeight = height;
	EmbedControl(mLittleArrowsBtn);
	
	// [3] create text field
	
	// move rectangle's x-position to the right of the spinner & add spacing
	r.left += width + GV_HORIZONTAL_SPACING;
	r.right = r.left + 36; // 36 is the width we're using for the text field -- // [FIX] should make a constant
	
	// create text field
	CFStringRef	cfstr = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
	CreateEditUnicodeTextControl (GetCarbonWindow(), &r, cfstr, false, &inFontStyle, &mControl);
	SInt16 oldHeight = (r.bottom - r.top) + 1;
	SInt16 newHeight;
	
	AUCarbonViewControl::SizeControlToFit(mControl, NULL, &newHeight);
	if (newHeight > mHeight) mHeight = newHeight;
	int topDelta = int(roundf(float(oldHeight) - float(newHeight)) * 0.5f);
	if (topDelta < 0) topDelta = 0;
	r.top += topDelta + 3;
	r.bottom = r.top + newHeight;
	SetControlBounds (mControl, &r);
	SetControl32BitMaximum(mControl, 100);
	SetControl32BitMinimum(mControl, 0);
		
	mUpdating = false;
	mLoadValue = 60;
	HandlePropertyChange(theCPULoad * 100);
	ControlKeyFilterUPP proc = AULoadCPU::NumericKeyFilterCallback;
	verify_noerr(SetControlData(mControl, 0, kControlEditTextKeyFilterTag, sizeof(proc), &proc));		
	EmbedControl(mControl);

	CFRelease (cfstr);
	
	ControlRef ref;
	r.left = r.right + (GV_TEXT_SPACING * 2);
	r.right = r.left + 14; 
	r.top = inLocation.v + 2;
	r.bottom = r.top + 16;
	
	OSErr theErr =  CreateStaticTextControl (GetCarbonWindow(), &r, kStringPercentSymbol, &inFontStyle, &ref);
	if (theErr == noErr)
		EmbedControl(ref);
	if (mHeight < 0) mHeight = 0;
	RegisterEvents();
}

void	AULoadCPU::AddInterest (AUEventListenerRef		inListener,
								void *					inObject)
{
	AudioUnitEvent e;
	e.mEventType = kAudioUnitEvent_PropertyChange;
	e.mArgument.mProperty.mAudioUnit = mView->GetEditAudioUnit();
	e.mArgument.mProperty.mPropertyID = kAudioUnitProperty_CPULoad;
	e.mArgument.mProperty.mScope = kAudioUnitScope_Global;
	e.mArgument.mProperty.mElement = 0;
	
	AUEventListenerAddEventType(inListener, inObject, &e);
}

void	AULoadCPU::RemoveInterest (AUEventListenerRef	inListener,
											void *					inObject)
{
	AudioUnitEvent e;
	e.mEventType = kAudioUnitEvent_PropertyChange;
	e.mArgument.mProperty.mAudioUnit = mView->GetEditAudioUnit();
	e.mArgument.mProperty.mPropertyID = kAudioUnitProperty_CPULoad;
	e.mArgument.mProperty.mScope = kAudioUnitScope_Global;
	e.mArgument.mProperty.mElement = 0;

	AUEventListenerRemoveEventType(inListener, inObject, &e);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AULoadCPU::NumericKeyFilterCallback
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ControlKeyFilterResult	AULoadCPU::NumericKeyFilterCallback(ControlRef theControl, 
												SInt16 *keyCode, SInt16 *charCode, 
												EventModifiers *modifiers)
{
	SInt16 c = *charCode;
	OSErr  err;
	if (isdigit(c) || c == '+' || c == '-' || c == '.' || c == '\b' || c == 0x7F || (c >= 0x1c && c <= 0x1f)
	|| c == '\t')
		return kControlKeyFilterPassKey;
	if (c == '\r' || c == 3) {	// return or Enter
		CFStringRef cfstr;
		err = GetControlData(theControl, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr, NULL);
		if (err != noErr) {
			CFRelease (cfstr);
			return kControlKeyFilterBlockKey;
		}
		int paramValue = 0;
		char valstr[32];
		CFStringGetCString(cfstr, valstr, sizeof(valstr), kCFStringEncodingASCII);
		sscanf(valstr, "%d", &paramValue);	
		SetControl32BitValue(theControl, paramValue);
		CFRelease (cfstr);
	}
	return kControlKeyFilterBlockKey;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AULoadCPU::HandleEvent
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool	AULoadCPU::HandleEvent(EventRef event)
{	
	UInt32 		eventClass = GetEventClass(event);		// the class of the event
	bool 		result = false;
		
	switch(eventClass) {
		case kEventClassMouse:
			result = HandleMouseEvent(event);
			break;
		case kEventClassCommand:
			result = HandleCommandEvent(event);
			break;
		case kEventClassControl:
			result = HandleControlEvent(event);
			break;
	}
	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AULoadCPU::RegisterEvents
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AULoadCPU::RegisterEvents ()
{
	EventTypeSpec events[] = {
		{ kEventClassMouse, kEventMouseDown},	
		{ kEventClassMouse, kEventMouseUp},	
		{ kEventClassCommand, kEventCommandProcess}
	};
	
	WantEventTypes(GetWindowEventTarget(GetCarbonWindow()), GetEventTypeCount(events), events);

	EventTypeSpec controlEvents[] = {
		{ kEventClassControl, kEventControlValueFieldChanged }	// N.B. OS X only
	};
	
	WantEventTypes(GetControlEventTarget(mControl), GetEventTypeCount(controlEvents), controlEvents);	
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AULoadCPU::HandlePropertyChange
//
//	Responds to a property changed message from the registered listener and updates the UI
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AULoadCPU::HandlePropertyChange(Float32 load) {
	if (load == 0 || load == 100) {
		if (!mUpdating)
			SetControlStringAndValue((int)load);				
		DeactivateControl (mControl);
		DeactivateControl (mLittleArrowsBtn);
		SetControl32BitValue (mNoCPURestrictionsBtn, 0);
		
	} else {
		if (!mUpdating)
			SetControlStringAndValue ((int)load);
		ActivateControl (mControl);
		ActivateControl (mLittleArrowsBtn);
		SetControl32BitValue (mNoCPURestrictionsBtn, 1);
	}

	mUpdating = false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AULoadCPU::HandleControlChange
//
//	Sets the property with the appropriate value based on the UI
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AULoadCPU::HandleControlChange ()
{
	Float32 val 	= GetControl32BitValue(mControl) / 100.0;
	
	AudioUnitSetProperty (mView->GetEditAudioUnit(), 
							kAudioUnitProperty_CPULoad,
							kAudioUnitScope_Global, 
							0, 
							&val, 
							sizeof(val));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AULoadCPU::HandleMouseEvent
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool AULoadCPU::HandleMouseEvent(EventRef inEvent) {
	UInt32 		eventKind  = GetEventKind(inEvent);			// the kind of the event
	CGrafPtr 	gp = GetWindowPort(GetCarbonWindow()), save;
	Point 		pt;
	HIPoint		mousePos;
	bool 		status = false;
	
	GetPort(&save);
	SetPort (gp);	

	GetEventParameter (	inEvent, kEventParamMouseLocation, 
						typeHIPoint, NULL, sizeof (mousePos), NULL, &mousePos);
	
	pt.h = short(mousePos.x); pt.v = short(mousePos.y);
	GlobalToLocal(&pt);
	
	if (eventKind == kEventMouseDown) {
		ControlPartCode part;
		ControlRef theControl = FindControlUnderMouse (pt, GetCarbonWindow(), &part);
		
		if (theControl == mLittleArrowsBtn) {
			if (part == kControlUpButtonPart)
				SetControlCommandID (mLittleArrowsBtn, kIncrementCPULoadCmd);
			else
				SetControlCommandID (mLittleArrowsBtn, kDecrementCPULoadCmd);			
		}
	}
	SetPort (save); 
	return status;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AULoadCPU::HandleControlEvent
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool AULoadCPU::HandleControlEvent(EventRef inEvent) {
	UInt32 		eventKind  = GetEventKind(inEvent);			// the kind of the event
	ControlRef 	ref;
	
	GetEventParameter (inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &ref);

	switch (eventKind) {
		case kEventControlValueFieldChanged:
			if (ref == mControl) {
				mUpdating = true;
				HandleControlChange();		// !!MSH- make sure this is called even when the arrows are pressed
				return true;	// handled
			} else
				return false;
	}
	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AULoadCPU::HandleCommandEvent
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool AULoadCPU::HandleCommandEvent(EventRef inEvent) {
	HICommand	command;
	GetEventParameter (inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);
	bool 		status = false;
	SInt32		value;
	ControlRef	ref;
	
	GetEventParameter (inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &ref);
	value = GetControl32BitValue (mControl);
			
	switch (command.commandID) {
		case kIncrementCPULoadCmd:
			value += 5;
			if (value > 100)
				value = 100;
			
			SetControlStringAndValue(value);
			status =  true;
			break;
		case kDecrementCPULoadCmd:
			value -= 5;
			if (value < 0)
				value = 0;
			SetControlStringAndValue(value);			
			status =  true;
			break;
		case kToggleRestrictCPULoadCmd:
			value = GetControl32BitValue (mNoCPURestrictionsBtn);
			if (value == 1) {
				// return the cpu load to previous value
				SetControlStringAndValue(mLoadValue);				
				ActivateControl(mControl);
				ActivateControl(mLittleArrowsBtn);
			}
			else {
				// set the cpu load to 0
				mLoadValue = GetControl32BitValue(mControl);
				SetControlStringAndValue(0);				
				DeactivateControl (mControl);
				DeactivateControl (mLittleArrowsBtn);
			}
			status =  true;
			break;
	}
	return status;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AULoadCPU::SetControlStringAndValue
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AULoadCPU::SetControlStringAndValue(int value) {
	::SetControlStringAndValue(mControl, value);
}

void SetControlStringAndValue(ControlRef control, int value) {
	char	str [5];
	CFStringRef cfstr;

	sprintf(str, "%d", value);
	cfstr = CFStringCreateWithCString(NULL, str, kCFStringEncodingUTF8);
	SetControl32BitValue (control, value);
	verify_noerr(SetControlData(control, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr));
	CFRelease(cfstr);
}

bool	AULoadCPU::HandlePropertyChange (const AudioUnitProperty &inProp)
{
	if (inProp.mPropertyID == kAudioUnitProperty_CPULoad) 
	{
		Float32 propVal;
		UInt32 theSize = sizeof (Float32);
		
		ComponentResult result = AudioUnitGetProperty(inProp.mAudioUnit, 
												inProp.mPropertyID, 
												inProp.mScope, 
												inProp.mElement, &propVal, &theSize);
		
		if (result == noErr) {
			HandlePropertyChange(propVal*100);
			return true;
		}
	}
	return false;
}	
