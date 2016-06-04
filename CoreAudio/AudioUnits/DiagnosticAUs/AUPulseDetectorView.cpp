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
	AUPulseDetectorView.cpp
	
=============================================================================*/

#include "AUCarbonViewBase.h"
#include "AUControlGroup.h"
#include <map>
#include <vector>

#include "AUPulseShared.h"
#include "GenericAUView.h"

class AUPulseDetectorView : public GenericAUView {

public:
							AUPulseDetectorView(AudioUnitCarbonView auv);
							virtual ~AUPulseDetectorView();

	virtual void			RebuildUI();
	    
protected:
	virtual OSStatus		CreateSpecificUI (Float32 inXOffset, Float32 inYOffset, Rect *outSize);
	
	void 					UpdatePulseDisplay();

	virtual void			HandlePropertyChange (const AudioUnitProperty  & inProp);
	
	virtual void			AddListenerEvents();

private:
	ControlRef				mLastFrameDisplay, 
							mLastPulseDisplay, 
							mNumMeasDisplay, 	
							mMeanDisplay, 
							mStdDevDisplay, 
							mMinDisplay, 
							mMaxDisplay;
};

COMPONENT_ENTRY(AUPulseDetectorView)

AUPulseDetectorView::AUPulseDetectorView(AudioUnitCarbonView auv) 
		: GenericAUView(auv)
{
}

AUPulseDetectorView::~AUPulseDetectorView()
{
}

static CFStringRef resetStr = CFSTR("-");

void 		AUPulseDetectorView::UpdatePulseDisplay()
{
	AUPulseMetrics metrics;
	UInt32 size = sizeof(metrics);
	OSStatus result = AudioUnitGetProperty (GetEditAudioUnit(), 
								kAUPulseMetricsPropertyID, 
								kAudioUnitScope_Global, 
								0, 
								&metrics, 
								&size);
	if (result) return;

	if (metrics.isValid) 
	{
		Float64 sampleRate;
		size = sizeof(sampleRate);
		result = AudioUnitGetProperty (GetEditAudioUnit(), 
									kAudioUnitProperty_SampleRate, 
									kAudioUnitScope_Global, 
									0, 
									&sampleRate, 
									&size);	
		if (result) return;

		char cstr[64];
		
		sprintf (cstr, "%ld Samples, %.2f msecs", metrics.lastPulseTime, (metrics.lastPulseTime / sampleRate * 1000.));	
		CFStringRef str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
		SetControlData (mLastPulseDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
		CFRelease (str);

		sprintf (cstr, "%ld Samples, %.2f msecs", metrics.numFrames, (metrics.numFrames / sampleRate * 1000.));	
		str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
		SetControlData (mLastFrameDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
		CFRelease (str);

		sprintf (cstr, "%ld", metrics.numMeasurements);	
		str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
		SetControlData (mNumMeasDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
		CFRelease (str);

		sprintf (cstr, "%.2f", metrics.mean);	
		str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
		SetControlData (mMeanDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
		CFRelease (str);
		
		if (metrics.stdDev)
		{
			sprintf (cstr, "%.2f", metrics.stdDev);	
			CFStringRef str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
			SetControlData (mStdDevDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
			CFRelease (str);

			sprintf (cstr, "%ld", metrics.min);	
			str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
			SetControlData (mMinDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
			CFRelease (str);

			sprintf (cstr, "%ld", metrics.max);	
			str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
			SetControlData (mMaxDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
			CFRelease (str);

			sprintf (cstr, "%.2f", metrics.mean);	
			str = CFStringCreateWithCString (0, cstr, kCFStringEncodingASCII);
			SetControlData (mMeanDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);
			CFRelease (str);
		} 
		else 
		{
			SetControlData (mStdDevDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &resetStr);
			SetControlData (mMinDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &resetStr);
			SetControlData (mMaxDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &resetStr);
		}
		
		if (!IsCompositWindow())
			DrawOneControl (mCarbonPane);		

	} else {
		CFStringRef str = CFSTR("<<Unable To Detect Pulse>>");
		SetControlData (mLastPulseDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &str);

		SetControlData (mLastFrameDisplay, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &resetStr);

		if (!IsCompositWindow()) {
			DrawOneControl (mLastPulseDisplay);		
			DrawOneControl (mLastFrameDisplay);
		}
	}
}

OSStatus	AUPulseDetectorView::CreateSpecificUI (Float32 inXOffset, Float32 inYOffset, Rect *outSize)
{
	static const int kRowHeight = 24;
	static const int kHeadingBodyDist = 24;
    static const int kNameWidth = 200;
	
	int y = short(inYOffset) + kHeadingBodyDist;
	const int x = short(inXOffset);
	
	outSize->left= x;
	outSize->right= x;
	outSize->top = y;
	outSize->bottom = y;
	
	ControlFontStyleRec fontStyle;
	fontStyle.flags = kControlUseFontMask | kControlUseJustMask;
	fontStyle.font = kControlFontSmallSystemFont;

	Rect r = *outSize;
	r.bottom += kRowHeight + 8;
	
	ControlRef ref;
	OSStatus result;

// PULSE TIME
	r.right += kNameWidth;
	fontStyle.just = teFlushRight;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Last Pulse Time: "), &fontStyle, &ref), home);
	EmbedControl(ref);
	
	r.left += r.right;
	r.right += kNameWidth;
	fontStyle.just = teFlushLeft;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, resetStr, &fontStyle, &mLastPulseDisplay), home);
	EmbedControl(mLastPulseDisplay);
	r.top += kRowHeight + 20;
	r.bottom = r.top + kRowHeight;
	r.left = x;
	r.right = x;

// LAST FRAME SIZE
	r.right += kNameWidth;
	fontStyle.just = teFlushRight;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Last Frame Size: "), &fontStyle, &ref), home);
	EmbedControl(ref);
	
	r.left += r.right;
	r.right += kNameWidth;
	fontStyle.just = teFlushLeft;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, resetStr, &fontStyle, &mLastFrameDisplay), home);
	EmbedControl(mLastFrameDisplay);
	r.top += kRowHeight;
	r.bottom = r.top + kRowHeight;
	r.left = x;
	r.right = x;

// NUM MEASUREMENTS
	r.right += kNameWidth;
	fontStyle.just = teFlushRight;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Number of Measurements: "), &fontStyle, &ref), home);
	EmbedControl(ref);
	
	r.left += r.right;
	r.right += kNameWidth;
	fontStyle.just = teFlushLeft;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, resetStr, &fontStyle, &mNumMeasDisplay), home);
	EmbedControl(mNumMeasDisplay);
	r.top += kRowHeight;
	r.bottom = r.top + kRowHeight;
	r.left = x;
	r.right = x;

// MEAN
	r.right += kNameWidth;
	fontStyle.just = teFlushRight;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Mean: "), &fontStyle, &ref), home);
	EmbedControl(ref);
	
	r.left += r.right;
	r.right += kNameWidth;
	fontStyle.just = teFlushLeft;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, resetStr, &fontStyle, &mMeanDisplay), home);
	EmbedControl(mMeanDisplay);
	r.top += kRowHeight;
	r.bottom = r.top + kRowHeight;
	r.left = x;
	r.right = x;

// STANDARD DEVIATION
	r.right += kNameWidth;
	fontStyle.just = teFlushRight;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Standard Deviation: "), &fontStyle, &ref), home);
	EmbedControl(ref);
	
	r.left += r.right;
	r.right += kNameWidth;
	fontStyle.just = teFlushLeft;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, resetStr, &fontStyle, &mStdDevDisplay), home);
	EmbedControl(mStdDevDisplay);
	r.top += kRowHeight;
	r.bottom = r.top + kRowHeight;
	r.left = x;
	r.right = x;

// MIN
	r.right += kNameWidth;
	fontStyle.just = teFlushRight;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Minimum: "), &fontStyle, &ref), home);
	EmbedControl(ref);
	
	r.left += r.right;
	r.right += kNameWidth;
	fontStyle.just = teFlushLeft;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, resetStr, &fontStyle, &mMinDisplay), home);
	EmbedControl(mMinDisplay);
	r.top += kRowHeight;
	r.bottom = r.top + kRowHeight;
	r.left = x;
	r.right = x;

// MAX
	r.right += kNameWidth;
	fontStyle.just = teFlushRight;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, CFSTR("Maximum: "), &fontStyle, &ref), home);
	EmbedControl(ref);
	
	r.left += r.right;
	r.right += kNameWidth;
	fontStyle.just = teFlushLeft;
	require_noerr (result = CreateStaticTextControl (GetCarbonWindow(), &r, resetStr, &fontStyle, &mMaxDisplay), home);
	EmbedControl(mMaxDisplay);

	outSize->bottom = r.bottom;
	outSize->right = y + kNameWidth + kNameWidth;
	
home:
	return result;
}

void 		AUPulseDetectorView::RebuildUI()
{
	GenericAUView::RebuildUI();
	
	mLastFrameDisplay = 0;
	mLastPulseDisplay = 0;
	mNumMeasDisplay = 0;
	mMeanDisplay = 0;
	mStdDevDisplay = 0;
	mMinDisplay = 0;
	mMaxDisplay = 0;
}

void		AUPulseDetectorView::AddListenerEvents()
{
	AudioUnitEvent e;
	e.mEventType = kAudioUnitEvent_PropertyChange;
	e.mArgument.mProperty.mAudioUnit = mEditAudioUnit;
	e.mArgument.mProperty.mScope = kAudioUnitScope_Global;
	e.mArgument.mProperty.mElement = 0;
	e.mArgument.mProperty.mPropertyID = kAUPulseMetricsPropertyID;
	AUEventListenerAddEventType(mEventListener, this, &e);
}

void	AUPulseDetectorView::HandlePropertyChange (const AudioUnitProperty  & inProp)
{
	if (inProp.mPropertyID == kAUPulseMetricsPropertyID) 
		UpdatePulseDisplay();
}

