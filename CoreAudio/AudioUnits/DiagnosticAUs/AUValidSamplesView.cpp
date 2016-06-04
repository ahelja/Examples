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
	AUValidSamplesView.cpp
	
=============================================================================*/

#include "AUCarbonViewBase.h"
#include "AUControlGroup.h"
#include <map>
#include <vector>

#include "AUValidSamplesShared.h"


static const UInt32  kNumDisplayRows = 18;

class AUValidSamplesView : public AUCarbonViewBase {

public:
	AUValidSamplesView(AudioUnitCarbonView auv);
	virtual ~AUValidSamplesView();
	
	virtual OSStatus		CreateUI (Float32	inXOffset, Float32 	inYOffset);
	virtual void			RebuildUI();
    
protected:
	virtual OSStatus		CreateUIForParameters (Float32 inXOffset, Float32 inYOffset, Rect *outSize);
	OSStatus				CreateSpecificUI (Float32 inXOffset, Float32 inYOffset, Rect *outSize);
	
	void 					DrawParameter (CAAUParameter &auvp, const int x, int &y, ControlFontStyleRec &fontStyle, Rect *outSize);

	void 					UpdateDisplay();
    
private:
	AUEventListenerRef		mPropertyChangeListener;
	
	ControlRef				GetCarbonPane()	{	return mCarbonPane;	}    
	void					Cleanup();
	static void				AUPropertyChangedListener(   void *						inCallbackRefCon,
														void *						inObject,
														const AudioUnitEvent *		inEvent,
														UInt64						inEventHostTime,
														Float32						inParameterValue);

	ControlRef	mSampleDisplay[kNumDisplayRows], mChannelDisplay[kNumDisplayRows], mValueDisplay[kNumDisplayRows];
	UInt32		mLastDisplayIndex;
	VSInfoList*	mPropValue;
	UInt32		mListByteSize;
	ControlRef	mMarker;
};

static const int kHeadingBodyDist = 24;
static const int kParamNameWidth = 140;
static const int kSliderWidth = 240;
static const int kEditTextWidth = 40;
static const int kParamTagWidth = 30;
static const int kMinMaxWidth = 40;
static const int kPopupAdjustment = -13;
static const int kReadOnlyMeterWidth = 286;

static const int kClumpSeparator = 8;
static const int kRowHeight = 20;
static const int kTextHeight = 14;

static const int kInfoRowHeight = 16;

static const CFStringRef kParametersLabelString = CFSTR("Parameters");

COMPONENT_ENTRY(AUValidSamplesView)

AUValidSamplesView::AUValidSamplesView(AudioUnitCarbonView auv) 
		: AUCarbonViewBase(auv), 
		  mPropertyChangeListener (0),
		  mPropValue(0)
{
}

AUValidSamplesView::~AUValidSamplesView()
{
	Cleanup();
	if (mPropertyChangeListener)
		AUListenerDispose(mPropertyChangeListener);
	free(mPropValue);
}

static CFStringRef resetStr = CFSTR("-");

void 		AUValidSamplesView::UpdateDisplay()
{
	UInt32 size = mListByteSize;
	OSStatus result = AudioUnitGetProperty (GetEditAudioUnit(), 
								kAUValidSamples_InvalidSamplesPropertyID, 
								kAudioUnitScope_Global, 
								0, 
								mPropValue, 
								&size);

	if (result)
		return;

	unsigned int numRetrieved = mPropValue->numEntries;
	
	if (!numRetrieved)
		return;
		
	unsigned int start = 0;
	if (numRetrieved >= kNumDisplayRows)
		start = numRetrieved - kNumDisplayRows;
	
	char cstr[64];

	for (unsigned int i = start; i < numRetrieved; ++i) 
	{
		VSInfo &item = mPropValue->data[i];
		
		sprintf (cstr, "%ld", item.sample);	
		CFStringRef str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
		SetControlData (mSampleDisplay[mLastDisplayIndex], 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
		CFRelease (str);

		sprintf (cstr, "%ld", item.channel);	
		str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
		SetControlData (mChannelDisplay[mLastDisplayIndex], 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
		CFRelease (str);
	
		float absx = fabs(item.value);
				// a bad number!
		if (absx < 1e-15)
			strcpy (cstr, "DENORMAL");
		else
			sprintf (cstr, "%.6f", item.value);
			
		str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
		SetControlData (mValueDisplay[mLastDisplayIndex], 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
		CFRelease (str);
		
		mLastDisplayIndex = ++mLastDisplayIndex % kNumDisplayRows;
	}
	
	int lastItem = mLastDisplayIndex == 0 ? (kNumDisplayRows - 1) : (mLastDisplayIndex - 1);
	MoveControl (mMarker, 96, 84 + lastItem * kInfoRowHeight);
	
	if (!IsCompositWindow())
		DrawOneControl (mCarbonPane);		
}

OSStatus	AUValidSamplesView::CreateSpecificUI (Float32 inXOffset, Float32 inYOffset, Rect *outSize)
{
	static const int kRowHeight = 24;
	static const int kHeadingBodyDist = 24;
    static const int kNameWidth = 100;
	
	int y = short(inYOffset) + kHeadingBodyDist;
	const int x = short(inXOffset) + 40;
	
	outSize->left= x;
	outSize->right= x;
	outSize->top = y;
	outSize->bottom = y;
	
	ControlFontStyleRec fontStyle;
	fontStyle.flags = kControlUseFontMask | kControlUseJustMask;
	fontStyle.font = kControlFontSmallBoldSystemFont;
	fontStyle.just = teFlushLeft;

	Rect r = *outSize;
	r.bottom += kRowHeight + 8;
	
	ControlRef ref;
	OSStatus result;

// Info Header
	r.left = 120;
	r.right += 300;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Detected Invalid Sample Information"), &fontStyle, &ref), home);
	EmbedControl(ref);
	
	r.top += kRowHeight;
	r.bottom = r.top + kRowHeight;
	r.left = x + 64;
	r.right = r.left;

// Column Headers
	r.right += kNameWidth;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Sample Offset"), &fontStyle, &ref), home);
	EmbedControl(ref);
	
	r.left = r.right;
	r.right = r.left + kNameWidth;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Channel"), &fontStyle, &ref), home);
	EmbedControl(ref);

	r.left = r.right;
	r.right = r.left + kNameWidth;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Value"), &fontStyle, &ref), home);
	EmbedControl(ref);

	r.top += kRowHeight;
	r.bottom = r.top + kRowHeight;
	r.left = x + 64;
	r.right = r.left;
	fontStyle.font = kControlFontSmallSystemFont;

// 	mSampleDisplay[kNumDisplayRows], mChannelDisplay[kNumDisplayRows], mValueDisplay[kNumDisplayRows];
	
	for (mLastDisplayIndex = 0; mLastDisplayIndex < kNumDisplayRows; ++mLastDisplayIndex) 
	{
		r.right += kNameWidth;
		require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, resetStr, &fontStyle, &mSampleDisplay[mLastDisplayIndex]), home);
		EmbedControl(mSampleDisplay[mLastDisplayIndex]);
		
		r.left = r.right;
		r.right = r.left + kNameWidth;
		require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, resetStr, &fontStyle, &mChannelDisplay[mLastDisplayIndex]), home);
		EmbedControl(mChannelDisplay[mLastDisplayIndex]);
	
		r.left = r.right;
		r.right = r.left + kNameWidth;
		require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, resetStr, &fontStyle, &mValueDisplay[mLastDisplayIndex]), home);
		EmbedControl(mValueDisplay[mLastDisplayIndex]);
	
		r.top += kInfoRowHeight;
		r.bottom = r.top + kInfoRowHeight;
		r.left = x + 64;
		r.right = r.left;
	}
	mLastDisplayIndex = 0;
	
	Rect mr;
	mr.top = 84 + mLastDisplayIndex * kInfoRowHeight;
	mr.bottom = mr.top + kInfoRowHeight;
	mr.left = 96;
	mr.right = 106;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &mr, CFSTR("*"), &fontStyle, &mMarker), home);
	EmbedControl(mMarker);

	outSize->bottom = r.bottom;
	outSize->right = y + kNameWidth + kNameWidth + kNameWidth;

	UpdateDisplay();
	
home:
	return result;
}

void 		AUValidSamplesView::Cleanup()
{
	ClearControls();
    
	UInt16 		numSubControls;
	ControlRef 	subControl;
	if (CountSubControls (mCarbonPane, &numSubControls) == noErr){
		for (UInt16 i = numSubControls; i > 0; --i) {
			GetIndexedSubControl(mCarbonPane, i, &subControl);
			DisposeControl(subControl);
		}
	}
}

OSStatus	AUValidSamplesView::CreateUI(Float32	inXOffset, Float32 	inYOffset)
{
	if (mPropertyChangeListener == NULL) {
		AUEventListenerCreate(AUPropertyChangedListener, this, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0., 0., &mPropertyChangeListener);
		AudioUnitEvent e;
		e.mEventType = kAudioUnitEvent_PropertyChange;
		e.mArgument.mProperty.mAudioUnit = mEditAudioUnit;
		e.mArgument.mProperty.mPropertyID = kAudioUnitProperty_ParameterList;
		e.mArgument.mProperty.mScope = kAudioUnitScope_Global;
		e.mArgument.mProperty.mElement = 0;
		AUEventListenerAddEventType(mPropertyChangeListener, this, &e);
		
		e.mArgument.mProperty.mPropertyID = kAudioUnitProperty_ParameterInfo;
		AUEventListenerAddEventType(mPropertyChangeListener, this, &e);

		e.mArgument.mProperty.mPropertyID = kAUValidSamples_InvalidSamplesPropertyID;
		AUEventListenerAddEventType(mPropertyChangeListener, this, &e);

		AudioUnitGetPropertyInfo (GetEditAudioUnit(), 
									kAUValidSamples_InvalidSamplesPropertyID, 
									kAudioUnitScope_Global, 
									0, 
									&mListByteSize, 
									NULL);
		mPropValue = (VSInfoList*)malloc(mListByteSize);
	}
	
	Point location;
	location.h = 6 + short(inXOffset);
	location.v = 10 + short(inYOffset);
	
	static const int kParamNameWidth = 140;
	static const int kSliderWidth = 240;
	static const int kEditTextWidth = 40;
	static const int kParamTagWidth = 30;
	static const SInt16 kRightStartOffset = kParamNameWidth + kEditTextWidth + kParamTagWidth;
	static const SInt16 kTotalWidth = kRightStartOffset + kSliderWidth;
		
	// Display the Name of the unit
	AUControlGroup::AddAUInfo (this, location, kRightStartOffset, kTotalWidth);
	
	Rect theRect;
	ComponentResult result = CreateSpecificUI ((Float32)location.h, (Float32) location.v, &theRect);
	if (result != noErr)
		return result;
		
	return CreateUIForParameters ((Float32)location.h, (Float32)theRect.bottom, &theRect);
}

void AUValidSamplesView::RebuildUI() 
{
	Cleanup();
		
	mBottomRight.h = mBottomRight.v = 0;
	SizeControl(mCarbonPane, 0, 0);
		
		// now that all controls have been removed, we need to re-add controls
	CreateUI(mXOffset, mYOffset);

		// we should only resize the control if a subclass has embedded
		// controls in this AND this is done with the EmbedControl call below
		// if mBottomRight is STILL equal to zero, then that wasn't done
		// so don't size the control
	Rect paneBounds;
	GetControlBounds(mCarbonPane, &paneBounds);
	if (mBottomRight.h != 0 && mBottomRight.v != 0)
		SizeControl(mCarbonPane, (short) (mBottomRight.h - mXOffset), (short) (mBottomRight.v - mYOffset));
}

OSStatus	AUValidSamplesView::CreateUIForParameters (Float32 inXOffset, Float32 inYOffset, Rect *outSize) 
{
	// for each parameter in the global scope, create:
	//		label	horiozontal slider	text entry	label
	// descending vertically
	// inside mCarbonWindow, embedded in mCarbonPane
	
	OSStatus err;
	UInt32 propertySize;
	
	outSize->top = (short) inYOffset;
	outSize->left= (short) inXOffset;
	outSize->bottom = outSize->top;
	outSize->right  = outSize->left;
	
	err = AudioUnitGetPropertyInfo(mEditAudioUnit, kAudioUnitProperty_ParameterList,
		kAudioUnitScope_Global, 0, &propertySize, NULL);
	if (err) return err;
	if (propertySize == 0)
		return noErr;
	
	int nparams = propertySize / sizeof(AudioUnitPropertyID);
	AudioUnitParameterID *paramIDs = new AudioUnitParameterID[nparams];

	err = AudioUnitGetProperty(mEditAudioUnit, kAudioUnitProperty_ParameterList,
		kAudioUnitScope_Global, 0, paramIDs, &propertySize);
	
	typedef std::vector <CAAUParameter> SortedParamList;
	typedef std::map <UInt32, SortedParamList, std::less<UInt32> > ParameterMap;
	
	ParameterMap params;
	if (!err) {
		for (int i = 0; i < nparams; ++i) {
			CAAUParameter auvp(mEditAudioUnit, paramIDs[i], kAudioUnitScope_Global, 0);
			const AudioUnitParameterInfo &paramInfo = auvp.ParamInfo();
			
			if (paramInfo.flags & kAudioUnitParameterFlag_ExpertMode)
				continue;
			
			if (!(paramInfo.flags & kAudioUnitParameterFlag_IsWritable)
					&& !(paramInfo.flags & kAudioUnitParameterFlag_IsReadable))
				continue;
			
			// ok - if we're here, then we have a parameter we are going to display.
			UInt32 clump = 0;
			auvp.GetClumpID (clump);
			params[clump].push_back (auvp);
		}
	}
	
	delete[] paramIDs;

	if (!err) {
		
		int y = short(inYOffset);
		const int x = short(inXOffset);
		
		Point location;
		location.h = x;
		location.v = y;
                
		ControlFontStyleRec fontStyle;
		fontStyle.flags = kControlUseFontMask | kControlUseJustMask;
		fontStyle.font = kControlFontSmallSystemFont;
						
		if (nparams > 0) {
			// Create the header for the parameters
			Rect r = {location.v, location.h, location.v + kRowHeight, location.h + 66};
			
			ControlRef ref;
			fontStyle.font = kControlFontSmallBoldSystemFont;
	
			OSErr theErr =  CreateStaticTextControl (GetCarbonWindow(), &r, kParametersLabelString, &fontStyle, &ref);
			if (theErr == noErr) {
				EmbedControl(ref);
				r.left = r.right + 2;
				r.right = location.h + 440;
				r.bottom -= 5;

				if (noErr == CreateSeparatorControl (GetCarbonWindow(), &r, &ref))
				EmbedControl(ref);
				
				y += kRowHeight;
				location.v += kRowHeight;
				outSize->bottom += kRowHeight;
			}
			
			fontStyle.font = kControlFontSmallSystemFont;
		}
		
		ParameterMap::iterator i = params.begin();
		while (1) {
			if (i != params.end()) {
				for (SortedParamList::iterator piter = (*i).second.begin(); piter != (*i).second.end(); ++piter)
				{
					DrawParameter (*piter, x, y, fontStyle, outSize);
					y += kRowHeight;
					outSize->bottom += kRowHeight;
				}
			}
			if (i != params.end() && ++i != params.end()) {
				y += kClumpSeparator;
				outSize->bottom += kClumpSeparator;
			} else {
				// now we want to add 16 pixels to the bottom and the right
				mBottomRight.h += 16;
				mBottomRight.v += 16;
				break;
			}
		}
	}
	return noErr;
}

void 	AUValidSamplesView::DrawParameter (CAAUParameter &auvp, const int x, int &y, ControlFontStyleRec &fontStyle, Rect *outSize)
{
	ControlRef newControl;			
	Rect r;
    			
	if (auvp.HasNamedParams()) 									// NAMED PARAMETER (POPUP)
	{
		fontStyle.font = kControlFontSmallSystemFont;
		
		r.top = y + 2;		r.bottom = r.top + kTextHeight;
		r.left = x;		r.right = r.left + kParamNameWidth;
		fontStyle.just = teFlushRight;
		
		CFMutableStringRef str = CFStringCreateMutableCopy(NULL, CFStringGetLength(auvp.GetName()) + 1, auvp.GetName());
		CFStringAppend (str, CFSTR(":"));
		
		if (noErr == CreateStaticTextControl(mCarbonWindow, 
											&r,
											str, 
											&fontStyle, 
											&newControl)) 
			verify_noerr(EmbedControl(newControl));
		
		CFRelease (str);
		
		static int topOffset = -100;
		static int rowOffset = -100;
		if (topOffset == -100) {
			SInt32 sysVers;
			Gestalt(gestaltSystemVersion, &sysVers);
			// 1030 for Panther
			SInt32 minorVers = sysVers & 0xFF;
			SInt32 majorVers = (sysVers & 0xFF00) >> 8;
			topOffset = 2;
			rowOffset = 5;
			if (majorVers == 0x10 && minorVers < 0x30) {
				topOffset = 4;
				rowOffset = 7;
			}
		}
		
		r.top -= topOffset; // Jaguar 4 Panther 2
		r.left = r.right + 8;	r.right = r.left + kSliderWidth + kPopupAdjustment;
		r.bottom += 5;
		fontStyle.font = kControlFontSmallSystemFont;
		
		AUControlGroup::CreatePopupMenu (this, auvp, r, fontStyle);
		if (outSize->right - outSize->left < r.right - r.left)
			outSize->right = outSize->left + (r.right - r.left);
		
		y += rowOffset; // Jaguar 7 - Panther 5
		
	} 
	else if (auvp.ParamInfo().unit == kAudioUnitParameterUnit_Boolean) 	// BOOLEAN (Checkbox)
	{
		fontStyle.font = kControlFontSmallSystemFont;
					
		Rect r;
		r.top = y;								r.bottom = r.top + kTextHeight; // tail allowance
		r.left = x + kParamNameWidth + kMinMaxWidth + 10;		r.right = r.left + 100;
		
		r.top -= 1;
		r.bottom -= 2;
		
		CFMutableStringRef str = CFStringCreateMutableCopy(NULL, CFStringGetLength(auvp.GetName()), auvp.GetName());

		if (noErr == CreateCheckBoxControl(GetCarbonWindow(), 
											&r, 
											str, 
											0, 
											true, 
											&newControl)) 
		{
			verify_noerr (SetControlFontStyle (newControl, &fontStyle));
			SInt16 baseLineOffset;
			verify_noerr (GetBestControlRect (newControl, &r, &baseLineOffset));
			r.bottom += baseLineOffset;
			SetControlBounds (newControl, &r);
			EmbedControl (newControl);
		
			AddCarbonControl(AUCarbonViewControl::kTypeDiscrete, auvp, newControl);
		}
		CFRelease (str);
		
		if (outSize->right - outSize->left < r.right - r.left)
			outSize->right = outSize->left + (r.right - r.left);
		
		y -= 4;
	}
	else 																// ALL OTHERS (Slider control)
	{
		fontStyle.font = kControlFontSmallSystemFont;
		
		r.top = y;		r.bottom = y + kTextHeight; // for tails of letters
		r.left = x;		r.right = r.left + kParamNameWidth;
		fontStyle.just = teFlushRight;
		
		int theWidth = r.right - r.left;
		
		r.top += 1;
		r.bottom += 1;
		CFMutableStringRef str = CFStringCreateMutableCopy(NULL, CFStringGetLength(auvp.GetName())+1, auvp.GetName());
		CFStringAppend (str, CFSTR(":"));
		
		if (noErr == CreateStaticTextControl(mCarbonWindow, &r, str, &fontStyle, &newControl))
			verify_noerr(EmbedControl(newControl));
			
		CFRelease (str);
		
		r.left = r.right + 8;	r.right = r.left + kSliderWidth;
		theWidth += 8 + kSliderWidth;
		r.bottom -= 2; // take the tail adjustement out
		
		Point labelSize, textSize;
		labelSize.v = textSize.v = kTextHeight - 3;
		labelSize.h = kMinMaxWidth;
		textSize.h = kEditTextWidth;
		
		AUControlGroup::CreateLabelledSliderAndEditText(this, auvp, r, labelSize, textSize, fontStyle);					
		if (auvp.GetParamTag()) {
			r.left = r.right + 8; 	r.right = r.left + kParamTagWidth;
			fontStyle.just = 0;
            
            // try to localize param tag
            CFStringRef tagString = auvp.GetParamTag();
            
			if (noErr == CreateStaticTextControl(mCarbonWindow, &r, tagString, &fontStyle, &newControl))
				verify_noerr(EmbedControl(newControl));
			theWidth += 8 + kParamTagWidth;
		}
		if (outSize->right - outSize->left < theWidth) {
			outSize->right = outSize->left + theWidth;
        }
	}
}

void	AUValidSamplesView::AUPropertyChangedListener(	void *					inCallbackRefCon,
													void *						inObject,
													const AudioUnitEvent *		inEvent,
													UInt64						inEventHostTime,
													Float32						inParameterValue)
{
	if (inEvent->mEventType == kAudioUnitEvent_PropertyChange)
	{
		if (inEvent->mArgument.mProperty.mPropertyID == kAudioUnitProperty_ParameterList 
			|| inEvent->mArgument.mProperty.mPropertyID == kAudioUnitProperty_ParameterInfo) 
		{
			AUValidSamplesView *This = static_cast<AUValidSamplesView *>(inCallbackRefCon);
			This->RebuildUI();
		}
		else if (inEvent->mArgument.mProperty.mPropertyID == kAUValidSamples_InvalidSamplesPropertyID) 
		{
			AUValidSamplesView *This = static_cast<AUValidSamplesView *>(inCallbackRefCon);
			This->UpdateDisplay();
		}
	}
}

