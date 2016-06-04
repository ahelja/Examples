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
 *  AURenderQualityPopup.cpp
 *  CAServices
 *
 *  Created by Michael Hopkins on Fri Oct 25 2002.
 *  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 */

#include "AURenderQualityPopup.h"
#include "AUViewLocalizedStringKeys.h"

#define kChangeRenderQualityCmd	'cqty'

#define GV_GUTTER_MARGIN			10
#define GV_TEXT_SPACING				5
#define GV_HORIZONTAL_SPACING		10
#define GV_VERTICAL_SPACING			5
#define GV_PROPERTY_INDENTATION		64

#define kRenderTitleWidth 80
#define kQualityMenuID 21022

static CFStringRef kStringRenderQuality = kAUViewLocalizedStringKey_RenderQuality;
static CFStringRef kStringMaximum = kAUViewLocalizedStringKey_Maximum;
static CFStringRef kStringHigh = kAUViewLocalizedStringKey_High;
static CFStringRef kStringMedium = kAUViewLocalizedStringKey_Medium;
static CFStringRef kStringLow = kAUViewLocalizedStringKey_Low;
static CFStringRef kStringMinimum = kAUViewLocalizedStringKey_Minimum;
static bool sLocalized = false;

// Function prototypes
void RenderQualityListener(void * inRefCon, AudioUnit ci, AudioUnitPropertyID  inID, AudioUnitScope inScope, AudioUnitElement inElement);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AURenderQualityPopup::AURenderQualityPopup
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AURenderQualityPopup::AURenderQualityPopup (AUCarbonViewBase *inBase, 
				Point 					inLocation, 
				ControlFontStyleRec & 	inFontStyle)
	: AUPropertyControl (inBase)
{
    CFBundleRef mainBundle = CFBundleGetBundleWithIdentifier(kLocalizedStringBundle_AUView);
    if (mainBundle) {
        
        if (!sLocalized) {
            kStringRenderQuality = CFCopyLocalizedStringFromTableInBundle(
                kAUViewLocalizedStringKey_RenderQuality, kLocalizedStringTable_AUView,
                mainBundle, CFSTR("Render Quality Popup menu title"));
            
            kStringMaximum = CFCopyLocalizedStringFromTableInBundle(kAUViewLocalizedStringKey_Maximum,
                kLocalizedStringTable_AUView, mainBundle, CFSTR("Maximum"));
            kStringHigh = CFCopyLocalizedStringFromTableInBundle(kAUViewLocalizedStringKey_High,
                kLocalizedStringTable_AUView, mainBundle, CFSTR("High"));
            kStringMedium = CFCopyLocalizedStringFromTableInBundle(kAUViewLocalizedStringKey_Medium,
                kLocalizedStringTable_AUView, mainBundle, CFSTR("Medium"));
            kStringLow = CFCopyLocalizedStringFromTableInBundle(kAUViewLocalizedStringKey_Low,
                kLocalizedStringTable_AUView, mainBundle, CFSTR("Low"));
            kStringMinimum = CFCopyLocalizedStringFromTableInBundle(kAUViewLocalizedStringKey_Minimum,
                kLocalizedStringTable_AUView, mainBundle, CFSTR("Minimum"));
            
            sLocalized = true;
        }
        
        // create title string & coordinates    
        CFMutableStringRef renderTitle = CFStringCreateMutable(NULL, 0);
        CFStringAppend(renderTitle, kStringRenderQuality);
        CFStringAppend(renderTitle, kAUViewUnlocalizedString_TitleSeparator);
        Rect r;
        r.top = inLocation.v;
		r.bottom = r.top;
        r.left = inLocation.h;
		r.right = r.left;
        
        ControlRef ref;
        OSErr err = CreateStaticTextControl (GetCarbonWindow(), &r, renderTitle, &inFontStyle, &ref);
		verify_noerr(err);
		SInt16 labelWidth = 0;
		AUCarbonViewControl::SizeControlToFit(ref, &labelWidth);
		EmbedControl(ref);
		
        // create popup menu
        r.left += labelWidth + GV_HORIZONTAL_SPACING;
        r.right = r.left;
        
		short bundleRef = CFBundleOpenBundleResourceMap (mainBundle);

		static int topOffset = -100;
		if (topOffset == -100) {
			SInt32 sysVers;
			Gestalt(gestaltSystemVersion, &sysVers);
			// 1030 for Panther
			SInt32 minorVers = sysVers & 0xFF;
			SInt32 majorVers = (sysVers & 0xFF00) >> 8;
			topOffset = 2;
			if (majorVers == 0x10 && minorVers < 0x30) {
				topOffset = 4;
			}
		}
		
		r.top -= topOffset; // Jaguar 4 Panther 2
		
		verify_noerr(CreatePopupButtonControl (GetCarbonWindow(), &r, NULL,
			/*kQualityMenuID*/ -12345, false, -1, 0, 0, &mControl));
        
		MenuRef menuRef;
		verify_noerr(CreateNewMenu(kQualityMenuID, 0, &menuRef));
		
		verify_noerr(AppendMenuItemTextWithCFString (menuRef, kStringMaximum,	0, kChangeRenderQualityCmd, 0));
		verify_noerr(AppendMenuItemTextWithCFString (menuRef, kStringHigh,		0, kChangeRenderQualityCmd, 0));
		verify_noerr(AppendMenuItemTextWithCFString (menuRef, kStringMedium,	0, kChangeRenderQualityCmd, 0));
		verify_noerr(AppendMenuItemTextWithCFString (menuRef, kStringLow,		0, kChangeRenderQualityCmd, 0));
		verify_noerr(AppendMenuItemTextWithCFString (menuRef, kStringMinimum,	0, kChangeRenderQualityCmd, 0));
        
		verify_noerr(SetControlData(mControl, 0, kControlPopupButtonMenuRefTag, sizeof(menuRef), &menuRef));
		verify_noerr(SetControlFontStyle (mControl, &inFontStyle));
        
		AUCarbonViewControl::SizeControlToFit(mControl, &labelWidth, &mHeight);
		
		SetControl32BitMaximum(mControl, 5);
		
		UInt32 renderQuality = 0;
		UInt32 size = sizeof(UInt32);
		AudioUnitGetProperty (mView->GetEditAudioUnit(), 
							kAudioUnitProperty_RenderQuality,
							kAudioUnitScope_Global, 
							0, 
							&renderQuality, 
							&size);
        
		HandlePropertyChange(renderQuality);
        
		EmbedControl(mControl);

		CFBundleCloseBundleResourceMap (mainBundle, bundleRef);
		if (mHeight < 0) mHeight = 0;
	}
    
	RegisterEvents();
}

void	AURenderQualityPopup::AddInterest (AUEventListenerRef		inListener,
											void *					inObject)
{
	AudioUnitEvent e;
	e.mEventType = kAudioUnitEvent_PropertyChange;
	e.mArgument.mProperty.mAudioUnit = mView->GetEditAudioUnit();
	e.mArgument.mProperty.mPropertyID = kAudioUnitProperty_RenderQuality;
	e.mArgument.mProperty.mScope = kAudioUnitScope_Global;
	e.mArgument.mProperty.mElement = 0;
	
	AUEventListenerAddEventType(inListener, inObject, &e);
}

void	AURenderQualityPopup::RemoveInterest (AUEventListenerRef	inListener,
											void *					inObject)
{
	AudioUnitEvent e;
	e.mEventType = kAudioUnitEvent_PropertyChange;
	e.mArgument.mProperty.mAudioUnit = mView->GetEditAudioUnit();
	e.mArgument.mProperty.mPropertyID = kAudioUnitProperty_RenderQuality;
	e.mArgument.mProperty.mScope = kAudioUnitScope_Global;
	e.mArgument.mProperty.mElement = 0;

	AUEventListenerRemoveEventType(inListener, inObject, &e);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AURenderQualityPopup::HandleEvent
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool	AURenderQualityPopup::HandleEvent(EventRef event)
{	
	UInt32 		eventClass = GetEventClass(event);		// the class of the event
	HICommand	command;
	bool 		result = false;
	
	GetEventParameter (event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);
		
	switch(eventClass) {
		case kEventClassCommand:
			if (command.commandID == kChangeRenderQualityCmd) {
				SetControl32BitValue(mControl, command.menu.menuItemIndex);
				HandleControlChange();
				result = true;
			}
			break;
	}
	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AURenderQualityPopup::HandlePropertyChange
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AURenderQualityPopup::HandlePropertyChange(UInt32 quality) 
{
	int item = kRenderQuality_Medium;
	switch (quality) {
		case kRenderQuality_Max:
			item = 1;
			break;
		case kRenderQuality_High:
			item = 2;
			break;
		case kRenderQuality_Medium:
			item = 3;
			break;
		case kRenderQuality_Low:
			item = 4;
			break;
		case kRenderQuality_Min:
			item = 5;
			break;
	}
	// only set the control value if it is different from the current value
	UInt32 value = GetControl32BitValue(mControl);
	if (item != (int)value)
		SetControl32BitValue(mControl, item);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AURenderQualityPopup::HandleControlChange
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AURenderQualityPopup::HandleControlChange ()
{
	UInt32	renderQuality = kRenderQuality_Medium;
	int 	val = GetControl32BitValue(mControl);
	
	switch (val) {
		case 1:
			renderQuality = kRenderQuality_Max;
			break;
		case 2:
			renderQuality = kRenderQuality_High;
			break;
		case 3:
			renderQuality = kRenderQuality_Medium;
			break;
		case 4:
			renderQuality = kRenderQuality_Low;
			break;
		case 5:
			renderQuality = kRenderQuality_Min;
			break;
	}
	
	AudioUnitSetProperty (mView->GetEditAudioUnit(), 
							kAudioUnitProperty_RenderQuality,
							kAudioUnitScope_Global, 
							0, 
							&renderQuality, 
							sizeof(renderQuality));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AURenderQualityPopup::RegisterEvents
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AURenderQualityPopup::RegisterEvents ()
{
	EventTypeSpec events[] = {
		{ kEventClassCommand, kEventCommandProcess}
	};
	
	WantEventTypes(GetWindowEventTarget(GetCarbonWindow()), GetEventTypeCount(events), events);
}

bool	AURenderQualityPopup::HandlePropertyChange (const AudioUnitProperty &inProp)
{
	if (inProp.mPropertyID == kAudioUnitProperty_RenderQuality) 
	{
		UInt32 propVal;
		UInt32 theSize = sizeof (UInt32);
		
		ComponentResult result = AudioUnitGetProperty(inProp.mAudioUnit, 
												inProp.mPropertyID, 
												inProp.mScope, 
												inProp.mElement, &propVal, &theSize);
		
		if (result == noErr) {
			HandlePropertyChange(propVal);
			return true;
		}
	}
	return false;
}
