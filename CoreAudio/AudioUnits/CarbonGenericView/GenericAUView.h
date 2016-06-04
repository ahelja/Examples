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
	GenericAUView.h
	
	
	Revision 1.10  2005/02/26 00:27:02  bills
	move scroll functionality to base class
	
	Revision 1.9  2005/02/25 23:07:24  bills
	accessor moved to base class
	
	Revision 1.8  2005/02/22 00:58:24  luke
	[4016789] enable scrolling in CarbonGenericView for those views in composited windows
	
	Revision 1.7  2005/02/22 00:19:11  bills
	fix compiler (gcc_4) warning
	
	Revision 1.6  2005/02/05 01:14:39  bills
	update property notification handling
	
	Revision 1.5  2005/01/25 19:28:32  luke
	add newline at EOF
	
	Revision 1.4  2004/11/13 22:52:22  bills
	fix sizing, bool param display and refactor for subclassing
	
	Revision 1.3  2004/11/09 18:08:24  luke
	remove vestigal BypassEffect code
	
	Revision 1.2  2004/07/10 01:42:51  bills
	use AUParamInfo to parse through param IDs
	
	Revision 1.1  2004/06/15 20:24:57  luke
	coalesced CoreAudioUI pieces into new coherent directory
	
	Revision 1.15  2004/04/14 00:50:06  luke
	size static text appropriately
	
	Revision 1.14  2004/02/13 22:33:19  dwyatt
	[3558808] use an AU event listener instead of a direct AU property change notification, to defer rebuilding the UI when parameter list changes
	
	Revision 1.13  2003/12/01 18:40:29  luke
	AUVParameter -> CAAUParameter
	
	Revision 1.12  2003/11/18 18:37:06  luke
	fix [3418362].  even though that bug was originally marked as behaves correctly, dev emails prompted us to fix it anyways.
	
	Revision 1.11  2003/09/19 06:42:53  bills
	fix the logic to rebuild the UI when parameters change
	
	Revision 1.10  2003/09/10 18:17:23  bills
	fix resizing when parameter list changes
	
	Revision 1.9  2003/09/03 22:28:26  bills
	cleanup code - Cleanup method!
	
	Revision 1.8  2003/08/27 01:05:06  mhopkins
	Added support for Disk Streaming Property
	
	Revision 1.7  2003/07/31 01:59:04  bills
	tweaks to view layouts (small sliders) and clumps!
	
	Revision 1.6  2003/05/16 19:48:58  mhopkins
	Changed RespondToEventTimer() to match cahnges in AUCarbonViewBase.cpp
	
	Revision 1.5  2003/05/10 00:11:37  bills
	make smaller and support read only params
	
	Revision 1.4  2003/03/05 17:51:35  mhopkins
	Changed member variable name to mParentViewContainer
	
	Revision 1.3  2002/10/30 18:56:29  mhopkins
	Reduced vertical spacing, changed parameter labels so that they better follow Apple UI guidelines
	
	Revision 1.2  2002/10/21 23:35:42  mhopkins
	Slight refactoring of UI code
	
	Revision 1.1  2002/07/30 19:56:02  bills
	break out class decl into a header file
	
	
	created 30 Jul 2002,Bill Stewart
	Copyright (c) 2002 Apple Computer, Inc.  All Rights Reserved
	
	$NoKeywords: $
=============================================================================*/
#ifndef __GenericAUView_h__
#define __GenericAUView_h__

#include "AUCarbonViewBase.h"
#include "AUControlGroup.h"
#include "AULoadCPU.h"
#include "AURenderQualityPopup.h"
#include "AUDiskStreamingCheckbox.h"

#include <vector>

class MeterView;

class GenericAUView : public AUCarbonViewBase {

friend class MeterView;

public:
							GenericAUView(AudioUnitCarbonView auv);
							virtual ~GenericAUView();
	
	virtual OSStatus		CreateUI (Float32	inXOffset, Float32 	inYOffset);
	virtual void			RebuildUI();
    	
protected:
	virtual OSStatus		CreateUIForStandardProperties (Float32 inXOffset, Float32 inYOffset, Rect *outSize);
	virtual OSStatus		CreateUIForParameters (Float32 inXOffset, Float32 inYOffset, Rect *outSize);
	
	virtual void 			RespondToEventTimer (EventLoopTimerRef inTimer);
	void					DrawParameterTitleInRect(const CFStringRef inString, ControlFontStyleRec &fontStyle, Rect &inRect);
	void 					DrawParameter (const CAAUParameter &auvp, const int x, int &y, ControlFontStyleRec &fontStyle, Rect *outSize);

	virtual void			HandleParameterChange (const AudioUnitParameter & inParam,
												UInt64						inEventHostTime,
												Float32						inParameterValue) {}

	virtual void			HandlePropertyChange (const AudioUnitProperty  & inProp);

	virtual void			AddListenerEvents();

	virtual OSStatus		CreateSpecificUI (Float32 inXOffset, Float32 inYOffset, Rect *outSize);

	AUEventListenerRef		mEventListener;

	static void				AUEventChange(	void *						inCallbackRefCon,
											void *						inObject,
											const AudioUnitEvent *		inEvent,
											UInt64						inEventHostTime,
											Float32						inParameterValue);

private:
	AUVPresets *					mFactoryPresets;
	AULoadCPU *						mLoadCPU;
	AURenderQualityPopup *			mQualityPopup;
	AUDiskStreamingCheckbox *		mStreamingCheckbox;
		
	typedef std::vector<MeterView*> MeterList;
	MeterList						mMeters;


	void					Cleanup();

	ControlRef				GetSizedStaticTextControl (CFStringRef inString, Rect *inRect, Boolean inIsBold);
};


#endif //__GenericAUView_h__
