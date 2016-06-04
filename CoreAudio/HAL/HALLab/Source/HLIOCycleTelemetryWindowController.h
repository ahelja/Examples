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
	HLIOCycleTelemetryWindowController.h

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	System Includes
#include <AppKit/AppKit.h>
#include <CoreAudio/CoreAudio.h>

//	Standard Library Includes
#include <set>

//=============================================================================
//	Types
//=============================================================================

@class	HLApplicationDelegate;
class	CAHALIOCycleTelemetryClient;
struct	HLIOCycleTelemetryOutlineItemTracker;
typedef std::set<HLIOCycleTelemetryOutlineItemTracker>	HLIOCycleTelemetryOutlineItemList;

//=============================================================================
//	HLIOCycleTelemetryWindowController
//=============================================================================

@interface HLIOCycleTelemetryWindowController : NSWindowController
{

@public
	//	Interface Items
	IBOutlet NSTextField*				mProcessNameTextField;
	IBOutlet NSTextField*				mProcessIDTextField;
	IBOutlet NSPopUpButton*				mDevicePopUp;
	IBOutlet NSPopUpButton*				mCaptureRatePopUp;
	IBOutlet NSMatrix*					mShowRadioButtonMatrix;
	IBOutlet NSOutlineView*				mTelemetryOutline;
	IBOutlet NSButton*					mEnableLatencyTracingCheckBox;
	IBOutlet NSButton*					mTraceOnOverloadsCheckBox;
	IBOutlet NSButton*					mTraceOnWakingLateCheckBox;
	IBOutlet NSTextField*				mWakingLateThresholdTextField;
	IBOutlet NSButton*					mTraceOnLongCyclesCheckBox;
	IBOutlet NSTextField*				mLongCycleThresholdTextField;
	
	//	Implementation
	HLApplicationDelegate*				mApplicationDelegate;
	pid_t								mProcessID;
	AudioDeviceID						mDevice;
	NSTimer*							mTelemetryUpdateTimer;
	CAHALIOCycleTelemetryClient*		mTelemetry;
	bool								mShowDataInCycles;
	HLIOCycleTelemetryOutlineItemList*	mTelemetryOutlineItems;
	
}

//	Construction/Destruction
-(id)		initWithApplicationDelegate:	(HLApplicationDelegate*)inApplicationDelegate;
-(void)		windowDidLoad;
-(void)		windowWillClose:				(NSNotification*)inNotification;
-(void)		dealloc;

//	View Management
-(IBAction)	SelectProcessButtonAction:			(id)inSender;
-(IBAction)	ProcessIDTextFieldAction:			(id)inSender;
-(IBAction)	CaptureRatePopUpAction:				(id)inSender;
-(IBAction)	ShowRadioMatrixAction:				(id)inSender;
-(IBAction)	PreviousButtonAction:				(id)inSender;
-(IBAction)	NextButtonAction:					(id)inSender;
-(IBAction)	SaveDataButtonAction:				(id)inSender;
-(IBAction)	SaveZerosButtonAction:				(id)inSender;
-(IBAction)	ClearDataButtonAction:				(id)inSender;
-(IBAction)	TelemetryOutlineAction:				(id)inSender;
-(IBAction)	DevicePopUpAction:					(id)inSender;
-(IBAction)	DeviceInfoButtonAction:				(id)inSender;
-(IBAction)	EnableLatencyTracingCheckBoxAction:	(id)inSender;
-(IBAction)	TraceOnOverloadsCheckBoxAction:		(id)inSender;
-(IBAction)	TraceOnWakingLateCheckBoxAction:	(id)inSender;
-(IBAction)	WakingLateTextFieldAction:			(id)inSender;
-(IBAction)	TraceOnLongCyclesCheckBoxAction:	(id)inSender;
-(IBAction)	LongCycleTextFieldAction:			(id)inSender;

-(void)		UpdateCaptureRatePopUp;
-(void)		UpdateDevicePopUp;
-(void)		UpdateLatencyTracingItems;

//	Update Timer Management
-(void)		UpdateTelemetry:				(NSTimer*)inTimer;
-(void)		StartUpdatingTelemetry:			(id)inSender;
-(void)		StopUpdatingTelemetry:			(id)inSender;
-(Float64)	GetUpdateInterval;

//	Outline Data Source
-(int)		outlineView:					(NSOutlineView*)inOutlineView
			numberOfChildrenOfItem:			(id)inItem;
-(BOOL)		outlineView:					(NSOutlineView*)inOutlineView
			isItemExpandable:				(id)inItem;
-(id)		outlineView:					(NSOutlineView*)inOutlineView
			child:							(int)inIndex
			ofItem:							(id)inItem;
-(id)		outlineView:					(NSOutlineView*)inOutlineView
			objectValueForTableColumn:		(NSTableColumn*)inTableColumn
			byItem:							(id)inItem;

@end
