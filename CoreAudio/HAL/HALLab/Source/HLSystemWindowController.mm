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
	HLSystemWindowController.mm

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "HLSystemWindowController.h"

//	Internal Includes
#include "HLApplicationDelegate.h"
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareSystem.h"

//	PublicUtility Includes
#include "CAException.h"

//=============================================================================
//	HLSystemWindowController
//=============================================================================

@implementation HLSystemWindowController

-(id)	initWithApplicationDelegate: (HLApplicationDelegate*)inApplicationDelegate
{
	//	initialize the super class
    [super initWithWindowNibName: @"SystemWindow"];
	
	mApplicationDelegate = inApplicationDelegate;
	
	//	sign up for system notifications
	CAAudioHardwareSystem::AddPropertyListener(kAudioPropertyWildcardPropertyID, (AudioHardwarePropertyListenerProc)HLSystemWindowControllerAudioHardwarePropertyListener, self);
	
	return self;
}

-(void)	windowDidLoad
{
	CATry;
	
	//	update the window
	[[self window] setExcludedFromWindowsMenu: YES];

	//	update the boot chime UI
	[self UpdateBootChimeSliderItems];
	
	//	update other UI items
	[self UpdateAllowIdleSleepCheckBox];
	
	//	update the default device UI
	[self UpdateDefaultDevicePopUp: true IsSystem: false];
	[self UpdateDefaultDevicePopUp: false IsSystem: false];
	[self UpdateDefaultDevicePopUp: false IsSystem: true];

	//	tell the table what selector to call when a double click happens
	[mDeviceListTableView setDoubleAction: @selector(SystemTableViewDoubleClickAction:)];
	
	CACatch;
}

-(void)	dealloc
{
	CAAudioHardwareSystem::RemovePropertyListener(kAudioPropertyWildcardPropertyID, (AudioHardwarePropertyListenerProc)HLSystemWindowControllerAudioHardwarePropertyListener);

	[super dealloc];
}

-(void)	UpdateBootChimeSliderItems
{
	[self UpdateBootChimeSliderKnob];
	[self UpdateBootChimeSliderCurrentTextField];
	[self UpdateBootChimeMinMaxTextFields];
}

-(void)	UpdateBootChimeSliderKnob
{
	if(CAAudioHardwareSystem::HasBootChimeVolumeControl())
	{
		//	Get the current value from the HAL
		Float32 theCurrentDBValue = CAAudioHardwareSystem::GetBootChimeVolumeControlValueAsDB();
		
		//	Get the dB value of the slider
		Float32 theCurrentSliderValue = [mBootChimeVolumeSlider floatValue];
		Float32 theCurrentSliderDBValue = CAAudioHardwareSystem::ConvertBootChimeVolumeControlScalarValueToDB(theCurrentSliderValue);
		
		if(theCurrentDBValue != theCurrentSliderDBValue)
		{
			//	Get the current scalar value for the control from the HAL
			Float32 theCurrentScalarValue = CAAudioHardwareSystem::GetBootChimeVolumeControlValueAsScalar();
			
			//	Set the knob to point at this value
			[mBootChimeVolumeSlider setFloatValue: theCurrentScalarValue];
		}
	}
	else
	{
		//	Set the knob to 0
		[mBootChimeVolumeSlider setFloatValue: 0.0];
		
		//	disable the control
		[mBootChimeVolumeSlider setEnabled: NO];
	}
}

-(void)	UpdateBootChimeSliderCurrentTextField
{
	NSString* theValueString = NULL;
	
	//	make a string out of the dB value
	if(CAAudioHardwareSystem::HasBootChimeVolumeControl())
	{
		//	Get the current value from the HAL
		Float32 theCurrentDBValue = CAAudioHardwareSystem::GetBootChimeVolumeControlValueAsDB();

		//	make a string out of the dB value
		theValueString = [[NSString alloc] initWithFormat: @"%7.3f", theCurrentDBValue];
	}
	else
	{
		theValueString = [[NSString alloc] initWithFormat: @"%7.3f", 0.0];
	}
	
	//	release the string
	[theValueString autorelease];
	
	//	set the value of the text
	[mBootChimeVolumeCurrentText setStringValue: theValueString];
}

-(void)	UpdateBootChimeMinMaxTextFields
{
	if(CAAudioHardwareSystem::HasBootChimeVolumeControl())
	{
		//	get the range of the slider in dB from the HAL
		Float32 theDBMinimum = 0;
		Float32 theDBMaximum = 0;
		CAAudioHardwareSystem::GetBootChimeVolumeControlDBRange(theDBMinimum, theDBMaximum);
		
		//	make the display strings
		NSString* theMinimum = [[NSString alloc] initWithFormat: @"%7.3f", theDBMinimum];
		NSString* theMaximum = [[NSString alloc] initWithFormat: @"%7.3f", theDBMaximum];
		[theMinimum autorelease];
		[theMaximum autorelease];
		
		//	shove the strings into the text fields
		[mBootChimeVolumeMinimumText setStringValue: theMinimum];
		[mBootChimeVolumeMaximumText setStringValue: theMaximum];
	}
	else
	{
		//	no control, no values
		NSString* theMinimum = [[NSString alloc] initWithFormat: @"%7.3f", 0.0];
		NSString* theMaximum = [[NSString alloc] initWithFormat: @"%7.3f", 0.0];
		[theMinimum autorelease];
		[theMaximum autorelease];
		
		//	shove the strings into the text fields
		[mBootChimeVolumeMinimumText setStringValue: theMinimum];
		[mBootChimeVolumeMaximumText setStringValue: theMaximum];
	}
}

-(IBAction)	BootChimeVolumeSliderAction:	(id)inSender
{
	CATry;
	
	//	Get the current value from the HAL
	Float32 theCurrentDBValue = CAAudioHardwareSystem::GetBootChimeVolumeControlValueAsDB();
	
	//	Get the dB value of the slider
	Float32 theCurrentSliderValue = [mBootChimeVolumeSlider floatValue];
	Float32 theCurrentSliderDBValue = CAAudioHardwareSystem::ConvertBootChimeVolumeControlScalarValueToDB(theCurrentSliderValue);

	if(theCurrentDBValue != theCurrentSliderDBValue)
	{
		//	tell the HAL to change the value
		CAAudioHardwareSystem::SetBootChimeVolumeControlValueAsScalar(theCurrentSliderValue);
		
		//	update the current text field
		[self UpdateBootChimeSliderCurrentTextField];
	}

	CACatch;
}

-(void) UpdateAllowIdleSleepCheckBox
{
	CATry;
	
	//	get the value from the HAL
	bool allowsSleep = CAAudioHardwareSystem::AllowsIdleSleepDuringIO();
	
	//	figure out what state that is
	int theState = allowsSleep ? NSOnState : NSOffState;
	
	//	set the check box accordingly
	if([mAllowIdleSleepCheckBox state] != theState)
	{
		[mAllowIdleSleepCheckBox setState: theState];
	}
	
	CACatch;
}

-(IBAction) AllowIdleSleepCheckBoxAction:	(id)inSender
{
	CATry;
	
	//	get the state of the check box
	int theState = [mAllowIdleSleepCheckBox state];
	
	//	figure out what it means
	bool allowsSleep = false;
	if(theState == NSOnState)
	{
		allowsSleep = true;
	}
	else if(theState == NSOffState)
	{
		allowsSleep = false;
	}
	
	//	tell the HAL
	CAAudioHardwareSystem::SetAllowsIdleSleepDuringIO(allowsSleep);
	
	CACatch;
}

-(IBAction) PlayAlertSoundButtonAction:	(id)inSender
{
	NSBeep();
}

-(void)	UpdateDefaultDevicePopUp:	(bool)inIsInput
		IsSystem:					(bool)inIsSystem
{
	//	figure out which pop-up we're talking about
	CAAudioHardwareDeviceSectionID theSection = inIsInput ? kAudioDeviceSectionInput : kAudioDeviceSectionOutput;
	NSPopUpButton* thePopUp = NULL; 
	if(inIsInput)
	{
		thePopUp = mDefaultInputDevicePopUp;
	}
	else if(inIsSystem)
	{
		thePopUp = mDefaultSystemOutputDevicePopUp;
	}
	else
	{
		thePopUp = mDefaultOutputDevicePopUp;
	}

	//	remove all the current items
	[thePopUp removeAllItems];
	
	//	iterate across the devices and add their names to the menu
	UInt32 theNumberDevices = CAAudioHardwareSystem::GetNumberDevices();
	if(theNumberDevices > 0)
	{
		UInt32 theDeviceIndex = 0;
		while(theDeviceIndex < theNumberDevices)
		{
			//	get the device
			CAAudioHardwareDevice theDevice(CAAudioHardwareSystem::GetDeviceAtIndex(theDeviceIndex));
			
			//	only add it if it has input and wants to be the default device
			if(theDevice.HasSection(theSection) && theDevice.CanBeDefaultDevice(theSection, inIsSystem))
			{
				//	get it's name
				NSString* theDeviceName = (NSString*)theDevice.CopyName();
				[theDeviceName autorelease];
				
				//	append the name to the menu
				[thePopUp addItemWithTitle: theDeviceName];
				
				//	also set the tag for the menu item to be the AudioDeviceID
				[[thePopUp lastItem] setTag: theDevice.GetAudioDeviceID()];
			}
			
			//	go to the next one
			++theDeviceIndex;
		}
	}
	
	//	set up the value of the pop-up
	if([thePopUp numberOfItems] != 0)
	{
		//	set the value of the menu to the current default device
		AudioDeviceID theDefaultDeviceID = CAAudioHardwareSystem::GetDefaultDevice(theSection, inIsSystem);
		UInt32 theDeviceIndex = [thePopUp indexOfItemWithTag: theDefaultDeviceID];
		[thePopUp selectItemAtIndex: theDeviceIndex];
	}
	else
	{
		//	no devices, so put "None" in the menu
		[thePopUp addItemWithTitle: @"None"];
		[[thePopUp lastItem] setTag: 0];
		[thePopUp selectItemAtIndex: 0];
	}
}

-(IBAction)	DefaultDevicePopUpAction:	(id)inSender
{
	CATry;

	NSMenuItem* theSelectedItem = [inSender selectedItem];
	if(theSelectedItem != NULL)
	{
		//	figure out which default device we're talking about
		CAAudioHardwareDeviceSectionID theSection = kAudioDeviceSectionInput;
		bool isSystem = false;
		if([inSender isEqual: mDefaultInputDevicePopUp])
		{
			theSection = kAudioDeviceSectionInput;
			isSystem = false;
		}
		else if([inSender isEqual: mDefaultOutputDevicePopUp])
		{
			theSection = kAudioDeviceSectionOutput;
			isSystem = false;
		}
		else
		{
			theSection = kAudioDeviceSectionOutput;
			isSystem = true;
		}
		
		//	get the AudioDeviceID from the tag
		AudioDeviceID theDeviceID = [theSelectedItem tag];
		CAAudioHardwareSystem::SetDefaultDevice(theSection, isSystem, theDeviceID);
	}
	
	CACatch;
}

-(int)	numberOfRowsInTableView:	(NSTableView*)inTableView
{
	UInt32 theNumberDevices = 0;
	CATry;
	theNumberDevices = CAAudioHardwareSystem::GetNumberDevices();
	CACatch;
	return theNumberDevices;
}

-(id)	tableView:					(NSTableView*)inTableView
		objectValueForTableColumn:	(NSTableColumn*)inTableColumn
		row:						(int)inRowIndex
{
	id theAnswer = NULL;
	
	CATry;

	//	get the identifier for the column
	NSString* theIdentifier = [inTableColumn identifier];
	
	//	get the device for the row
	CAAudioHardwareDevice theDevice(CAAudioHardwareSystem::GetDeviceAtIndex(inRowIndex));

	//	figure out what kind of data is being requested
	if([theIdentifier isEqual: @"Name"])
	{
		//	have to get the device's name
		theAnswer = (NSString*)theDevice.CopyName();
	}
	else if([theIdentifier isEqual: @"In"])
	{
		//	need to calculate the number of input channels
		theAnswer = [[NSString alloc] initWithFormat: @"%d", theDevice.GetTotalNumberChannels(kAudioDeviceSectionInput)];
	}
	else if([theIdentifier isEqual: @"Out"])
	{
		//	need to calculate the number of output channels
		theAnswer = [[NSString alloc] initWithFormat: @"%d", theDevice.GetTotalNumberChannels(kAudioDeviceSectionOutput)];
	}
	else if([theIdentifier isEqual: @"ID"])
	{
		//	need to get the device's AudioDeviceID
		theAnswer = [[NSString alloc] initWithFormat: @"0x%X", theDevice.GetAudioDeviceID()];
	}

	//	make sure this string goes away
	[theAnswer autorelease];

	CACatch;

	return theAnswer;
}

-(IBAction)	SystemTableViewAction:	(id)inSender
{
	CATry;
	CACatch;
}

-(IBAction)	SystemTableViewDoubleClickAction:	(id)inSender
{
	CATry;

	//	get the selected row from the table
	int theSelectedRow = [mDeviceListTableView selectedRow];
	if(theSelectedRow != -1)
	{
		//	get the device for the selected row
		AudioDeviceID theDeviceID = CAAudioHardwareSystem::GetDeviceAtIndex(theSelectedRow);
		
		[mApplicationDelegate ShowDeviceWindow: theDeviceID];
	}

	CACatch;
}

-(IBAction)	ShowDeviceInfoButtonAction:	(id)inSender
{
	CATry;

	//	get the selected row from the table
	int theSelectedRow = [mDeviceListTableView selectedRow];
	if(theSelectedRow != -1)
	{
		//	get the device for the selected row
		AudioDeviceID theDeviceID = CAAudioHardwareSystem::GetDeviceAtIndex(theSelectedRow);
		
		[mApplicationDelegate ShowDeviceWindow: theDeviceID];
	}

	CACatch;
}

-(void)	DeviceListChanged
{
	[mDeviceListTableView reloadData];
	[self UpdateDefaultDevicePopUp: true IsSystem: false];
	[self UpdateDefaultDevicePopUp: false IsSystem: false];
	[self UpdateDefaultDevicePopUp: false IsSystem: true];
}

-(void)	AppendNotification:	(AudioHardwarePropertyID)inPropertyID
{
	//	get the time of the notification
	UInt64 theNotificationTime = AudioConvertHostTimeToNanos(AudioGetCurrentHostTime());
	Float64 theNotificationTimeInMilliseconds = (theNotificationTime - [mApplicationDelegate GetNotificationStartTime]) / 1000000.0;
	
	//	get the name of the notification
	char* theNotificationNameString = NULL;
	switch(inPropertyID)
	{
		case kAudioHardwarePropertyDevices:
			theNotificationNameString = "Device List";
			break;
			
		case kAudioHardwarePropertyDefaultInputDevice:
				theNotificationNameString = "Default Input Device";
				break;
				
		case kAudioHardwarePropertyDefaultOutputDevice:
				theNotificationNameString = "Default Output Device";
				break;
				
		case kAudioHardwarePropertyDefaultSystemOutputDevice:
				theNotificationNameString = "Default System Output Device";
				break;
				
		case kAudioHardwarePropertyDeviceForUID:
				theNotificationNameString = "Device For UID (shouldn't see this)";
				break;
				
		case kAudioHardwarePropertySleepingIsAllowed:
				theNotificationNameString = "Idle Sleeping";
				break;
				
		case kAudioHardwarePropertyUnloadingIsAllowed:
				theNotificationNameString = "Unloading";
				break;
				
		case kAudioHardwarePropertyRunLoop:
				theNotificationNameString = "Run Loop";
				break;
				
		case kAudioHardwarePropertyBootChimeVolumeScalar:
				theNotificationNameString = "Boot Chime (Scalar)";
				break;
				
		case kAudioHardwarePropertyBootChimeVolumeDecibels:
				theNotificationNameString = "Boot Chime (dB)";
				break;
				
		case kAudioHardwarePropertyBootChimeVolumeRangeDecibels:
				theNotificationNameString = "Boot Chime dB Range";
				break;
				
		case kAudioHardwarePropertyBootChimeVolumeScalarToDecibels:
				theNotificationNameString = "Convert Boot Chime Volume Scalar->dB (shouldn't see this)";
				break;
				
		case kAudioHardwarePropertyBootChimeVolumeDecibelsToScalar:
				theNotificationNameString = "Convert Boot Chime Volume dB->Scalar (shouldn't see this)";
				break;
			
	};
	
	//	make a string out of the 4CC property ID
	char* the4CC = (char*)&inPropertyID;
	char thePropertyIDString[5] = { the4CC[0], the4CC[1], the4CC[2], the4CC[3], 0 };
	
	//	make a string for the notification
	NSString* theNotificationString = NULL;
	if(theNotificationNameString != NULL)
	{
		theNotificationString = [[NSString alloc] initWithFormat: @"%12.4f: %s ('%s')\n", theNotificationTimeInMilliseconds, theNotificationNameString, thePropertyIDString];
	}
	else
	{
		theNotificationString = [[NSString alloc] initWithFormat: @"%12.4f: '%s'\n", theNotificationTimeInMilliseconds, thePropertyIDString];
	}
	[theNotificationString autorelease];
	
	//	append that to the text view
	int theLength = [[mNotificationTextView textStorage] length];
	[[mNotificationTextView textStorage] replaceCharactersInRange: NSMakeRange(theLength, 0) withString: theNotificationString];
}

@end

OSStatus	HLSystemWindowControllerAudioHardwarePropertyListener(AudioHardwarePropertyID inPropertyID, HLSystemWindowController* inSystemWindowController)
{
	NS_DURING
	CATry;

	//	append the notification to the text view
	[inSystemWindowController AppendNotification: inPropertyID];
	
	//	do the appropriate work
	switch(inPropertyID)
	{
		case kAudioHardwarePropertyBootChimeVolumeDecibels:
			[inSystemWindowController UpdateBootChimeSliderKnob];
			[inSystemWindowController UpdateBootChimeSliderCurrentTextField];
			break;
			
		case kAudioHardwarePropertyBootChimeVolumeRangeDecibels:
			[inSystemWindowController UpdateBootChimeSliderKnob];
			[inSystemWindowController UpdateBootChimeSliderCurrentTextField];
			[inSystemWindowController UpdateBootChimeMinMaxTextFields];
			break;
			
		case kAudioHardwarePropertySleepingIsAllowed:
			[inSystemWindowController UpdateAllowIdleSleepCheckBox];
			break;
			
		case kAudioHardwarePropertyDefaultInputDevice:
			[inSystemWindowController UpdateDefaultDevicePopUp: kAudioDeviceSectionInput IsSystem: false];
			break;
			
		case kAudioHardwarePropertyDefaultOutputDevice:
			[inSystemWindowController UpdateDefaultDevicePopUp: kAudioDeviceSectionOutput IsSystem: false];
			break;
			
		case kAudioHardwarePropertyDefaultSystemOutputDevice:
			[inSystemWindowController UpdateDefaultDevicePopUp: kAudioDeviceSectionOutput IsSystem: true];
			break;
			
		case kAudioHardwarePropertyDevices:
			[inSystemWindowController DeviceListChanged];
			break;
			
		default:
			break;
	};
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER

	return 0;
}
