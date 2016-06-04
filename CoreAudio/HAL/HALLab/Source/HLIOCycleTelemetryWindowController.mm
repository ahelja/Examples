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
	HLIOCycleTelemetryWindowController.mm

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "HLIOCycleTelemetryWindowController.h"

//	Local Includes
#include "HLApplicationDelegate.h"
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareSystem.h"
#include "CAHALIOCycleTelemetryClient.h"

//	PublicUtility Includes
#include "CAAudioTimeStamp.h"
#include "CAAutoDisposer.h"
#include "CACFMessagePort.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAProcess.h"

//=============================================================================
//	HLIOCycleTelemetryWindowController
//=============================================================================

static OSStatus	HLIOCycleTelemetryWindowController_AudioHardwarePropertyListener(AudioHardwarePropertyID inPropertyID, HLIOCycleTelemetryWindowController* inIOCycleTelemetryWindowController);

@interface HLIOCycleTelemetryOutlineItem : NSObject
{
@public
	UInt32	mIOCycleIndex;
	UInt32	mRawEventNumber;
}

-(id)		initWithIOCycleIndex:	(UInt32)inIOCycleIndex;
-(id)		initWithIOCycleIndex:	(UInt32)inIOCycleIndex
			RawEventNumber:			(UInt32)inRawEventNumber;
-(UInt32)	GetIOCycleIndex;
-(UInt32)	GetRawEventNumber;
	
@end

@implementation HLIOCycleTelemetryOutlineItem

-(id)	initWithIOCycleIndex:	(UInt32)inIOCycleIndex
{
	mIOCycleIndex = inIOCycleIndex;
	mRawEventNumber = 0xFFFFFFFF;
	
	return [super init];
}
	
-(id)	initWithIOCycleIndex:	(UInt32)inIOCycleIndex
		RawEventNumber:			(UInt32)inRawEventNumber
{
	mIOCycleIndex = inIOCycleIndex;
	mRawEventNumber = inRawEventNumber;
	
	return [super init];
}
	
-(UInt32)	GetIOCycleIndex
{
	return mIOCycleIndex;
}

-(UInt32)	GetRawEventNumber
{
	return mRawEventNumber;
}

@end

struct	HLIOCycleTelemetryOutlineItemTracker
{
	UInt32							mIOCycleIndex;
	UInt32							mRawEventNumber;
	HLIOCycleTelemetryOutlineItem*	mItem;
	
	HLIOCycleTelemetryOutlineItemTracker(HLIOCycleTelemetryOutlineItem* inItem) : mIOCycleIndex(inItem->mIOCycleIndex), mRawEventNumber(inItem->mRawEventNumber), mItem(inItem)  {}
	HLIOCycleTelemetryOutlineItemTracker(UInt32 inIOCycleIndex, UInt32 inRawEventNumber) : mIOCycleIndex(inIOCycleIndex), mRawEventNumber(inRawEventNumber), mItem(NULL)  {}
	HLIOCycleTelemetryOutlineItemTracker(const HLIOCycleTelemetryOutlineItemTracker& inTracker) : mIOCycleIndex(inTracker.mIOCycleIndex), mRawEventNumber(inTracker.mRawEventNumber), mItem(inTracker.mItem)  { if(mItem != NULL) { [mItem retain]; } }
	HLIOCycleTelemetryOutlineItemTracker&	operator=(const HLIOCycleTelemetryOutlineItemTracker& inTracker) { if(mItem != NULL) { [mItem release]; } mIOCycleIndex = inTracker.mIOCycleIndex; mRawEventNumber = inTracker.mRawEventNumber; mItem = inTracker.mItem; if(mItem != NULL) { [mItem retain]; } return *this; }
	~HLIOCycleTelemetryOutlineItemTracker() { if(mItem != NULL) { [mItem release]; } }
};

inline bool	operator<(const HLIOCycleTelemetryOutlineItemTracker& x, const HLIOCycleTelemetryOutlineItemTracker& y) { return  (x.mIOCycleIndex < y.mIOCycleIndex) || ((x.mIOCycleIndex == y.mIOCycleIndex) && (x.mRawEventNumber < y.mRawEventNumber)); }
inline bool	operator==(const HLIOCycleTelemetryOutlineItemTracker& x, const HLIOCycleTelemetryOutlineItemTracker& y) { return  (x.mIOCycleIndex == y.mIOCycleIndex) && (x.mRawEventNumber < y.mRawEventNumber); }
inline bool	operator!=(const HLIOCycleTelemetryOutlineItemTracker& x, const HLIOCycleTelemetryOutlineItemTracker& y) { return !(x == y); }
inline bool	operator<=(const HLIOCycleTelemetryOutlineItemTracker& x, const HLIOCycleTelemetryOutlineItemTracker& y) { return (x < y) || (x == y); }
inline bool	operator>=(const HLIOCycleTelemetryOutlineItemTracker& x, const HLIOCycleTelemetryOutlineItemTracker& y) { return !(x < y); }
inline bool	operator>(const HLIOCycleTelemetryOutlineItemTracker& x, const HLIOCycleTelemetryOutlineItemTracker& y) { return !((x < y) || (x == y)); }

typedef std::set<HLIOCycleTelemetryOutlineItemTracker>	HLIOCycleTelemetryOutlineItemList;

@implementation HLIOCycleTelemetryWindowController

-(id)	initWithApplicationDelegate:	(HLApplicationDelegate*)inApplicationDelegate
{
	CATry;
	
	//	initialize the super class
    [super initWithWindowNibName: @"IOCycleTelemetryWindow"];
	
	//	initialize the basic stuff
	mApplicationDelegate = inApplicationDelegate;
	mProcessID = -1;
	mDevice = CAAudioHardwareSystem::GetDefaultDevice(kAudioDeviceSectionOutput, false);
	mTelemetryUpdateTimer = NULL;
	mTelemetry = new CAHALIOCycleTelemetryClient("/tmp/HALLab Latency Trace", ".txt");
	mShowDataInCycles = true;
	mTelemetryOutlineItems = new HLIOCycleTelemetryOutlineItemList();
	
	//	sign up for device list notifications
	CAAudioHardwareSystem::AddPropertyListener(kAudioHardwarePropertyDevices, (AudioHardwarePropertyListenerProc)HLIOCycleTelemetryWindowController_AudioHardwarePropertyListener, self);
	
	CACatch;
	
	return self;
}

-(void)	windowDidLoad
{
	CATry;
	
	//	update the process name
	[mProcessNameTextField setStringValue: @""];
	[mProcessIDTextField setIntValue: mProcessID];
	
	//	update the capture rate popup
	[self UpdateCaptureRatePopUp];
	
	//	update the device popup
	[self UpdateDevicePopUp];

	//	set the column title of the telemetry outline
	NSTableColumn* theColumn = [[mTelemetryOutline tableColumns] objectAtIndex: 0];
	if(theColumn != NULL)
	{
		char theHeaderCString[1024];
		if(mShowDataInCycles)
		{
			mTelemetry->CreateSummaryHeaderForIOCycle(theHeaderCString, false);
		}
		else
		{
			mTelemetry->CreateSummaryHeaderForRawEvent(theHeaderCString);
		}
		[[theColumn headerCell] setStringValue: [NSString stringWithCString: theHeaderCString]];
	}
	
	//	update the latency tracing items
	[self UpdateLatencyTracingItems];
	
	CACatch;
}

-(void)	windowWillClose:	(NSNotification*)inNotification
{
	//	the window is closing, so arrange to get cleaned up
	[mApplicationDelegate DestroyIOCycleTelemetryWindow: self];
}

-(void)	dealloc
{
	CATry;
	
	[self StopUpdatingTelemetry: self];
	CAAudioHardwareSystem::RemovePropertyListener(kAudioHardwarePropertyDevices, (AudioHardwarePropertyListenerProc)HLIOCycleTelemetryWindowController_AudioHardwarePropertyListener);
	delete mTelemetry;
	
	delete mTelemetryOutlineItems;
	
	CACatch;

	[super dealloc];
}

-(IBAction)	SelectProcessButtonAction:	(id)inSender
{
}

-(IBAction)	ProcessIDTextFieldAction:	(id)inSender
{
	//	get the value
	pid_t theNewProcessID = [mProcessIDTextField intValue];
	
	//	sanity check it
	if((theNewProcessID != -1) && !CAProcess::ProcessExists(theNewProcessID))
	{
		theNewProcessID = -1;
		[mProcessIDTextField setIntValue: theNewProcessID];
	}
	
	if(theNewProcessID != mProcessID)
	{
		[self StopUpdatingTelemetry: self];
		mProcessID = theNewProcessID;
		[self UpdateCaptureRatePopUp];
		if(mProcessID != -1)
		{
			ProcessSerialNumber thePSN = { 1, mProcessID };
			NSString* theProcessName = NULL;
			CopyProcessName(&thePSN, (CFStringRef*)&theProcessName);
			if(theProcessName != NULL)
			{
				[mProcessNameTextField setStringValue: theProcessName];
				[theProcessName autorelease];
			}
			else
			{
				[mProcessNameTextField setStringValue: @""];
			}
		}
		else
		{
			[mProcessNameTextField setStringValue: @""];
		}
	}
}

-(IBAction)	CaptureRatePopUpAction:	(id)inSender
{
	[self StartUpdatingTelemetry: self];
}

-(IBAction)	ShowRadioMatrixAction:	(id)inSender
{
	//	get the newly selected cell
	NSCell* theCell = [mShowRadioButtonMatrix selectedCell];
	
	//	get the row/column the cell is in
	int theRow = 0;
	int theColumn = 0;
	[mShowRadioButtonMatrix getRow: &theRow column: &theColumn ofCell: theCell];
	
	//	this radio set is vertically oriented
	switch(theRow)
	{
		case 0:
			//	showing things in cycles
			if(!mShowDataInCycles)
			{
				mShowDataInCycles = true;
			}
			break;
		
		case 1:
			//	showing things in the raw
			if(mShowDataInCycles)
			{
				mShowDataInCycles = false;
			}
			break;
		
		default:
			break;
	};
	NSTableColumn* theColumnObject = [[mTelemetryOutline tableColumns] objectAtIndex: 0];
	if(theColumnObject != NULL)
	{
		char theHeaderCString[1024];
		if(mShowDataInCycles)
		{
			mTelemetry->CreateSummaryHeaderForIOCycle(theHeaderCString, false);
		}
		else
		{
			mTelemetry->CreateSummaryHeaderForRawEvent(theHeaderCString);
		}
		[[theColumnObject headerCell] setStringValue: [NSString stringWithCString: theHeaderCString]];
	}
	[mTelemetryOutline reloadData];
}

-(IBAction)	PreviousButtonAction:	(id)inSender
{
	//	get the first selected row
	UInt32 theEventIndex = [mTelemetryOutline numberOfRows] - 1;
	NSEnumerator* theEnumerator = [mTelemetryOutline selectedRowEnumerator];
	NSNumber* theSelectedIndex = [theEnumerator nextObject];
	if(theSelectedIndex != NULL)
	{
		theEventIndex = [theSelectedIndex longValue];
	}
	//	find the index before this one that has an error
	if(mShowDataInCycles)
	{
		theEventIndex = mTelemetry->GetPreviousErrorIOCycleIndex(theEventIndex);
	}
	else
	{
		theEventIndex = mTelemetry->GetPreviousErrorRawEventIndex(theEventIndex);
	}
	if(theEventIndex != 0xFFFFFFFF)
	{
		[mTelemetryOutline selectRow: theEventIndex byExtendingSelection: NO];
		[mTelemetryOutline scrollRowToVisible: theEventIndex];
	}
}

-(IBAction)	NextButtonAction:	(id)inSender
{
	//	get the last selected row
	UInt32 theEventIndex = ([mTelemetryOutline selectedRow] != -1) ? [mTelemetryOutline selectedRow] : 0;
	//	find the index before this one that has an error
	if(mShowDataInCycles)
	{
		theEventIndex = mTelemetry->GetNextErrorIOCycleIndex(theEventIndex);
	}
	else
	{
		theEventIndex = mTelemetry->GetNextErrorRawEventIndex(theEventIndex);
	}
	if(theEventIndex != 0xFFFFFFFF)
	{
		[mTelemetryOutline selectRow: theEventIndex byExtendingSelection: NO];
		[mTelemetryOutline scrollRowToVisible: theEventIndex];
	}
}

-(IBAction)	SaveDataButtonAction:	(id)inSender
{
	if(mTelemetry->GetNumberIOCycles() > 0)
	{
		//	create a save dialog
		NSSavePanel* theSavePanel = [NSSavePanel savePanel];
		
		//	set it up 
		[theSavePanel setCanSelectHiddenExtension: NO];
		[theSavePanel setExtensionHidden: NO];
		[theSavePanel setRequiredFileType: @"txt"];
		
		//	run it
		int theResult = [theSavePanel runModal];
		
		//	do the work if the user hit OK
		if(theResult == NSOKButton)
		{
			//	get the path for saving the file
			NSString* thePathString = [theSavePanel filename];
			const char* thePath = [thePathString cString];
			
			//	open a new file
			FILE* theFile = fopen(thePath, "w+");
			if(theFile != NULL)
			{
				char theString[2048];
				
				//	get the header
				mTelemetry->CreateSummaryHeaderForIOCycle(theString, true);
				fprintf(theFile, "%s", theString);
				
				//	go through the telemetry and put the spreadsheet summary into the file
				UInt32 theNumberCycles = mTelemetry->GetNumberIOCycles();
				for(UInt32 theCycleIndex = 0; theCycleIndex < theNumberCycles; ++theCycleIndex)
				{
					mTelemetry->CreateSummaryForIOCycle(theCycleIndex, theString, true);
					fprintf(theFile, "%s", theString);
				}
				
				//	close the file
				fclose(theFile);
			}
		}
	}
}

-(IBAction)	SaveZerosButtonAction:	(id)inSender
{
}

-(IBAction)	ClearDataButtonAction:	(id)inSender
{
	if(mTelemetry != NULL)
	{
		mTelemetry->Clear();
	}
	if(mTelemetryOutlineItems != NULL)
	{
		mTelemetryOutlineItems->clear();
	}
	[mTelemetryOutline reloadData];
}

-(IBAction)	TelemetryOutlineAction:	(id)inSender
{
}

-(IBAction)	DevicePopUpAction:	(id)inSender
{
	//	means the device has changed
	CATry;
		
	//	retrieve the selected item
	NSMenuItem* theSelectedItem = [mDevicePopUp selectedItem];
	if(theSelectedItem != NULL)
	{
		//	get the corresponding device
		AudioDeviceID theDeviceID = [theSelectedItem tag];
		
		if(mDevice != theDeviceID)
		{
			//	stop updating the telemetry
			[self StopUpdatingTelemetry: self];
			
			mDevice = theDeviceID;
		}
	}
	
	CACatch;
}

-(IBAction)	DeviceInfoButtonAction:	(id)inSender
{
	CATry;
	
	if(mDevice != 0)
	{
		[mApplicationDelegate ShowDeviceWindow: mDevice];
	}
	
	CACatch;
}

-(IBAction)	EnableLatencyTracingCheckBoxAction:	(id)inSender
{
	CATry;
	
	if(mTelemetry->CanDoLatencyTracing())
	{
		mTelemetry->SetIsLatencyTracingEnabled([mEnableLatencyTracingCheckBox intValue] != 0);
	}
	
	CACatch;
}

-(IBAction)	TraceOnOverloadsCheckBoxAction:	(id)inSender
{
	CATry;
	
	if(mTelemetry->CanDoLatencyTracing())
	{
		mTelemetry->SetOverloadTrigger([mTraceOnOverloadsCheckBox intValue] != 0);
	}
	
	CACatch;
}

-(IBAction)	TraceOnWakingLateCheckBoxAction:	(id)inSender
{
	CATry;
	
	if(mTelemetry->CanDoLatencyTracing())
	{
		if([mTraceOnWakingLateCheckBox intValue] != 0)
		{
			Float64 theTrigger = [mWakingLateThresholdTextField doubleValue];
			if(theTrigger == 0)
			{
				theTrigger = 3;
				[mWakingLateThresholdTextField setDoubleValue: theTrigger];
			}
			mTelemetry->SetIOThreadSchedulingLatencyTrigger(theTrigger);
		}
		else
		{
			mTelemetry->SetIOThreadSchedulingLatencyTrigger(0);
		}
	}
	
	CACatch;
}

-(IBAction)	WakingLateTextFieldAction:	(id)inSender
{
	CATry;
	
	if(mTelemetry->CanDoLatencyTracing())
	{
		if([mTraceOnWakingLateCheckBox intValue] != 0)
		{
			mTelemetry->SetIOThreadSchedulingLatencyTrigger([mWakingLateThresholdTextField doubleValue]);
		}
		else
		{
			mTelemetry->SetIOThreadSchedulingLatencyTrigger(0);
		}
	}
	
	CACatch;
}

-(IBAction)	TraceOnLongCyclesCheckBoxAction:	(id)inSender
{
	CATry;
	
	if(mTelemetry->CanDoLatencyTracing())
	{
		if([mTraceOnLongCyclesCheckBox intValue] != 0)
		{
			Float64 theTrigger = [mLongCycleThresholdTextField doubleValue];
			if(theTrigger == 0)
			{
				theTrigger = 1000;
				[mLongCycleThresholdTextField setDoubleValue: theTrigger];
			}
			mTelemetry->SetIOCycleDurationTrigger(theTrigger);
		}
		else
		{
			mTelemetry->SetIOCycleDurationTrigger(0);
		}
	}
	
	CACatch;
}

-(IBAction)	LongCycleTextFieldAction:	(id)inSender
{
	CATry;
	
	if(mTelemetry->CanDoLatencyTracing())
	{
		if([mTraceOnLongCyclesCheckBox intValue] != 0)
		{
			mTelemetry->SetIOCycleDurationTrigger([mLongCycleThresholdTextField doubleValue]);
		}
		else
		{
			mTelemetry->SetIOCycleDurationTrigger(0);
		}
	}
	
	CACatch;
}

-(void)	UpdateCaptureRatePopUp
{
	if(mProcessID == -1)
	{
		UInt32 theItemIndex = [mCaptureRatePopUp indexOfItemWithTag: 0];
		[mCaptureRatePopUp selectItemAtIndex: theItemIndex];
		[mCaptureRatePopUp setEnabled: NO];
	}
	else
	{
		[mCaptureRatePopUp setEnabled: YES];
	}
}

-(void)	UpdateDevicePopUp
{
	CATry;
	
	//	remove all the current items
	[mDevicePopUp removeAllItems];
	
	//	iterate across the devices and add the names of the output devices to the menu
	UInt32 theNumberDevices = CAAudioHardwareSystem::GetNumberDevices();
	if(theNumberDevices > 0)
	{
		UInt32 theDeviceIndex = 0;
		while(theDeviceIndex < theNumberDevices)
		{
			//	get the device
			CAAudioHardwareDevice theDevice(CAAudioHardwareSystem::GetDeviceAtIndex(theDeviceIndex));
			
			//	get it's name
			NSString* theDeviceName = (NSString*)theDevice.CopyName();
			
			//	append the name to the menu
			[mDevicePopUp addItemWithTitle: @"foo"];
			[[mDevicePopUp lastItem] setTitle: theDeviceName];

			[theDeviceName release];
			
			//	also set the tag for the menu item to be the AudioDeviceID
			[[mDevicePopUp lastItem] setTag: theDevice.GetAudioDeviceID()];
			
			//	go to the next one
			++theDeviceIndex;
		}
	}
	
	//	set up the value of the pop-up
	if([mDevicePopUp numberOfItems] != 0)
	{
		//	set the value of the menu to the current device
		UInt32 theDeviceIndex = [mDevicePopUp indexOfItemWithTag: mDevice];
		[mDevicePopUp selectItemAtIndex: theDeviceIndex];
	}
	else
	{
		//	no devices, so put "None" in the menu
		[mDevicePopUp addItemWithTitle: @"None"];
		[[mDevicePopUp lastItem] setTag: 0];
		[mDevicePopUp selectItemAtIndex: 0];
	}
	
	CACatch;
}

-(void)	UpdateLatencyTracingItems
{
	CATry;
	
	if(mTelemetry->CanDoLatencyTracing())
	{
		//	can do the tracing so set up the current values
		[mEnableLatencyTracingCheckBox setIntValue: mTelemetry->IsLatencyTracingEnabled() ? 1 : 0];
		[mEnableLatencyTracingCheckBox setEnabled: YES];
		
		[mTraceOnOverloadsCheckBox setIntValue: mTelemetry->GetOverloadTrigger() ? 1 : 0];
		[mTraceOnOverloadsCheckBox setEnabled: YES];
		
		Float64 theTrigger = mTelemetry->GetIOThreadSchedulingLatencyTrigger();
		[mTraceOnWakingLateCheckBox setIntValue: theTrigger > 0 ? 1 : 0];
		[mTraceOnWakingLateCheckBox setEnabled: YES];
		[mWakingLateThresholdTextField setDoubleValue: theTrigger];
		[mWakingLateThresholdTextField setEnabled: YES];
		
		theTrigger = mTelemetry->GetIOCycleDurationTrigger();
		[mTraceOnLongCyclesCheckBox setIntValue: theTrigger > 0 ? 1 : 0];
		[mTraceOnLongCyclesCheckBox setEnabled: YES];
		[mLongCycleThresholdTextField setDoubleValue: theTrigger];
		[mLongCycleThresholdTextField setEnabled: YES];
	}
	else
	{
		//	can't do latency tracing so disable everything
		[mEnableLatencyTracingCheckBox setIntValue: 0];
		[mEnableLatencyTracingCheckBox setEnabled: NO];
		
		[mTraceOnOverloadsCheckBox setIntValue: 0];
		[mTraceOnOverloadsCheckBox setEnabled: NO];
		
		[mTraceOnWakingLateCheckBox setIntValue: 0];
		[mTraceOnWakingLateCheckBox setEnabled: NO];
		[mWakingLateThresholdTextField setIntValue: 0];
		[mWakingLateThresholdTextField setEnabled: NO];
		
		[mTraceOnLongCyclesCheckBox setIntValue: 0];
		[mTraceOnLongCyclesCheckBox setEnabled: NO];
		[mLongCycleThresholdTextField setIntValue: 0];
		[mLongCycleThresholdTextField setEnabled: NO];
	}
	
	CACatch;
}

-(void)	UpdateTelemetry:	(NSTimer*)inTimer
{
	if(mTelemetry != NULL)
	{
		if(mTelemetry->Update())
		{
			[mTelemetryOutline reloadData];
		}
	}
}

-(void)	StartUpdatingTelemetry:	(id)inSender
{
	CATry;
	
	//	get the update inteval
	Float64 theUpdateInterval = [self GetUpdateInterval];
	
	if((mDevice != 0) && (mProcessID != -1) && (theUpdateInterval > 0.0) && (mTelemetry != NULL))
	{
		//	stop capturing at the current rate
		if(mTelemetryUpdateTimer != NULL)
		{
			[mTelemetryUpdateTimer invalidate];
			[mTelemetryUpdateTimer release];
			mTelemetryUpdateTimer = NULL;
		}
		if(mTelemetry != NULL)
		{
			mTelemetry->Teardown();
		}
		if(mTelemetryOutlineItems != NULL)
		{
			mTelemetryOutlineItems->clear();
		}
		
		[mTelemetryOutline reloadData];
		
		//	initialize the telemetry client
		mTelemetry->Initialize(mProcessID, mDevice);
		
		//	start the timer
		mTelemetryUpdateTimer = [[NSTimer scheduledTimerWithTimeInterval: theUpdateInterval target: self selector: @selector(UpdateTelemetry:) userInfo: NULL repeats: YES] retain];
	}
	else
	{
		//	the rate is set to 0 so stop
		[self StopUpdatingTelemetry: self];
	}
	
	CACatch;
}

-(void)	StopUpdatingTelemetry:	(id)inSender
{
	if(mTelemetryUpdateTimer != NULL)
	{
		[mTelemetryUpdateTimer invalidate];
		[mTelemetryUpdateTimer release];
		mTelemetryUpdateTimer = NULL;
	}
	if(mTelemetry != NULL)
	{
		mTelemetry->SetIsEnabledOnServer(false);
	}
	if(mCaptureRatePopUp != NULL)
	{
		UInt32 theItemIndex = [mCaptureRatePopUp indexOfItemWithTag: 0];
		[mCaptureRatePopUp selectItemAtIndex: theItemIndex];
	}
}

-(Float64)	GetUpdateInterval
{
	Float64 theAnswer = 0;
	
	//	retrieve the selected item
	NSMenuItem* theSelectedItem = [mCaptureRatePopUp selectedItem];
	if(theSelectedItem != NULL)
	{
		//	the tag of the selected items is the number of updates in two seconds
		theAnswer = [theSelectedItem tag] / 2.0;
	}
	
	return theAnswer;
}

-(int)	outlineView:			(NSOutlineView*)inOutlineView
		numberOfChildrenOfItem:	(id)inItem
{
	int theAnswer = 0;
	
	if(mTelemetry != NULL)
	{
		if(mShowDataInCycles)
		{
			if(inItem == NULL)
			{
				theAnswer = mTelemetry->GetNumberIOCycles();
			}
			else
			{
				UInt32 theIOCycleIndex = [inItem GetIOCycleIndex];
				UInt32 theRawEventNumber = [inItem GetRawEventNumber];
				
				if(theRawEventNumber == 0xFFFFFFFF)
				{
					theAnswer = mTelemetry->GetNumberEventsInIOCycle(theIOCycleIndex);
				}
			}
		}
		else
		{
			if(inItem == NULL)
			{
				theAnswer = mTelemetry->GetNumberRawEvents();
			}
		}
	}
	
	return theAnswer;
}

-(BOOL)	outlineView:		(NSOutlineView*)inOutlineView
		isItemExpandable:	(id)inItem
{
	BOOL theAnswer = NO;
	
	if(mTelemetry != NULL)
	{
		if(mShowDataInCycles)
		{
			if(inItem == NULL)
			{
				theAnswer = YES;
			}
			else
			{
				UInt32 theRawEventNumber = [inItem GetRawEventNumber];
				
				if(theRawEventNumber == 0xFFFFFFFF)
				{
					theAnswer = YES;
				}
			}
		}
		else
		{
			if(inItem == NULL)
			{
				theAnswer = YES;
			}
		}
	}
	
	return theAnswer;
}

-(id)	outlineView:	(NSOutlineView*)inOutlineView
		child:			(int)inIndex
		ofItem:			(id)inItem
{
	HLIOCycleTelemetryOutlineItem* theAnswer = NULL;
	
	if((mTelemetry != NULL) && (mTelemetryOutlineItems != NULL))
	{
		bool doAllocation = false;
		UInt32 theIOCycleIndex = 0;
		UInt32 theRawEventNumber = 0;
		if(mShowDataInCycles)
		{
			if(inItem == NULL)
			{
				theIOCycleIndex = inIndex;
				theRawEventNumber = 0xFFFFFFFF;
				doAllocation = true;
			}
			else
			{
				theIOCycleIndex = [inItem GetIOCycleIndex];
				theRawEventNumber = inIndex;
				doAllocation = true;
			}
		}
		else
		{
			if(inItem == NULL)
			{
				theIOCycleIndex = 0;
				theRawEventNumber = inIndex;
				doAllocation = true;
			}
		}
		
		if(doAllocation)
		{
			//	first look in the list to see if we already have one
			HLIOCycleTelemetryOutlineItemList::iterator theIterator = mTelemetryOutlineItems->find(HLIOCycleTelemetryOutlineItemTracker(theIOCycleIndex, theRawEventNumber));
			if(theIterator != mTelemetryOutlineItems->end())
			{
				theAnswer = theIterator->mItem;
			}
			else
			{
				theAnswer = [[HLIOCycleTelemetryOutlineItem alloc] initWithIOCycleIndex: theIOCycleIndex RawEventNumber: theRawEventNumber];
				mTelemetryOutlineItems->insert(HLIOCycleTelemetryOutlineItemTracker(theAnswer));
			}
		}
	}
	
	return theAnswer;
}

-(id)	outlineView:				(NSOutlineView*)inOutlineView
		objectValueForTableColumn:	(NSTableColumn*)inTableColumn
		byItem:						(id)inItem
{
	static NSDictionary *sNormalAttributes			= [[NSDictionary dictionaryWithObjectsAndKeys: [NSFont userFixedPitchFontOfSize:9], NSFontAttributeName, [NSColor darkGrayColor], NSForegroundColorAttributeName, nil] retain];
	static NSDictionary *sErrorAttrributes			= [[NSDictionary dictionaryWithObjectsAndKeys: [NSFont userFixedPitchFontOfSize:9], NSFontAttributeName, [NSColor redColor], NSForegroundColorAttributeName, nil] retain];
	static NSDictionary *sSignalPresetAttrributes	= [[NSDictionary dictionaryWithObjectsAndKeys: [NSFont userFixedPitchFontOfSize:9], NSFontAttributeName, [NSColor blackColor], NSForegroundColorAttributeName, nil] retain];
	id theAnswer = NULL;
	
	if(mTelemetry != NULL)
	{
		if(inItem == NULL)
		{
			theAnswer = @"IO Cycle Telemetry";
		}
		else
		{
			UInt32 theRawEventNumber = [inItem GetRawEventNumber];
			char	theCString[2048];
			bool theEventHasError = false;
			bool theEventHasSignal = false;
			if(mShowDataInCycles)
			{
				UInt32 theIOCycleIndex = [inItem GetIOCycleIndex];
				if(theRawEventNumber == 0xFFFFFFFF)
				{
					mTelemetry->CreateSummaryForIOCycle(theIOCycleIndex, theCString, false);
					theEventHasError = mTelemetry->IOCycleHasError(theIOCycleIndex);
					theEventHasSignal = mTelemetry->IOCycleHasSignal(theIOCycleIndex);
				}
				else
				{
					mTelemetry->CreateSummaryForEventInIOCycle(theIOCycleIndex, theRawEventNumber, theCString);
					theEventHasError = mTelemetry->EventInIOCycleHasError(theIOCycleIndex, theRawEventNumber);
					theEventHasSignal = true;
				}
			}
			else
			{
				mTelemetry->CreateSummaryForRawEvent(theRawEventNumber, theCString);
				theEventHasError = mTelemetry->IsRawEventError(theRawEventNumber);
				theEventHasSignal = true;
			}
			
			NSDictionary* theAttrs;
			if(theEventHasError)
			{
				theAttrs = sErrorAttrributes;
			}
			else if(theEventHasSignal)
			{
				theAttrs = sSignalPresetAttrributes;
			}
			else
			{
				theAttrs = sNormalAttributes;
			}
			theAnswer = [[[NSMutableAttributedString alloc] initWithString: [NSString stringWithCString: theCString] attributes: theAttrs] autorelease];
		}
	}
	
	return theAnswer;
}

@end

static OSStatus	HLIOCycleTelemetryWindowController_AudioHardwarePropertyListener(AudioHardwarePropertyID inPropertyID, HLIOCycleTelemetryWindowController* inIOCycleTelemetryWindowController)
{
	NS_DURING
	CATry;

	switch(inPropertyID)
	{
		case kAudioHardwarePropertyDevices:
			[inIOCycleTelemetryWindowController UpdateDevicePopUp];
			break;
			
		default:
			break;
	};
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER

	return 0;
}
