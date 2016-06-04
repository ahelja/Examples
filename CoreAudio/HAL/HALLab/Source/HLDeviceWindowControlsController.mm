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
	HLDeviceWindowControlsController.mm

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "HLDeviceWindowControlsController.h"

//	Local Includes
#include "HLDeviceWindowController.h"
#include "HLStripView.h"

//	PublicUtility Includes
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"

//=============================================================================
//	HLDeviceWindowControlsController
//=============================================================================

@implementation HLDeviceWindowControlsController

-(id)	init
{
	mDevice = 0;
	mDeviceIsDead = false;
	return [super init];
}

-(void)	awakeFromNib
{
	CATry;
	
	//	get the device we're attached to
	mDevice = [mWindowController GetAudioDeviceID];
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	add listeners for the stuff we're tracking
	theDevice.AddPropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyDeviceHasChanged, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyDataSource, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyDataSources, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyPreferredChannelsForStereo, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyJackIsConnected, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyVolumeRangeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyMute, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyPlayThru, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyDriverShouldOwniSub, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertySubMute, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertySubVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertySubVolumeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc, self);
	
	//	set up the data source controls
	[self UpdateDataSourceItems: kAudioDeviceSectionInput];
	[self UpdateDataSourceItems: kAudioDeviceSectionOutput];
	
	//	set up the is connected items
	[self UpdateIsConnectedItems: kAudioDeviceSectionInput];
	[self UpdateIsConnectedItems: kAudioDeviceSectionOutput];
	
	//	set up the stereo pair controls
	[self UpdateStereoPairItems: kAudioDeviceSectionInput];
	[self UpdateStereoPairItems: kAudioDeviceSectionOutput];
	
	//	set up the iSub controls
	[self UpdateISubItems];
	
	//	set up the scroll views
	[mInputScrollView setDrawsBackground: NO];
	[mInputScrollView setDocumentView: mInputStripView];
	[mInputScrollView setNeedsDisplay: YES];
	
	[mOutputScrollView setDrawsBackground: NO];
	[mOutputScrollView setDocumentView: mOutputStripView];
	[mOutputScrollView setNeedsDisplay: YES];
	
	//	update the strip view
	[self SetupAllStrips: kAudioDeviceSectionInput];
	[self SetupAllStrips: kAudioDeviceSectionOutput];
	
	CACatch;
}

-(void)	dealloc
{
	if(!mDeviceIsDead)
	{
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	remove the listeners for the stuff we're tracking
	theDevice.RemovePropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	theDevice.RemovePropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyDeviceHasChanged, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	theDevice.RemovePropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	theDevice.RemovePropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyDataSource, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	theDevice.RemovePropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyDataSources, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	theDevice.RemovePropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyPreferredChannelsForStereo, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	theDevice.RemovePropertyListener(0, kAudioDeviceSectionWildcard, kAudioDevicePropertyJackIsConnected, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyVolumeRangeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyMute, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyPlayThru, (AudioDevicePropertyListenerProc)HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc);
	}
	
	[super dealloc];
}

-(void)		UpdateDataSourceItems:	(CAAudioHardwareDeviceSectionID)inSection
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	NSPopUpButton* theDataSourcePopUp = ((inSection == kAudioDeviceSectionInput) ? mInputDataSourcePopUp : mOutputDataSourcePopUp);
	
	if(!mDeviceIsDead)
	{
		if(theDevice.HasDataSourceControl(inSection))
		{
			UInt32 theNumberSources = theDevice.GetNumberAvailableDataSources(inSection);
			
			//	there are data sources, so remove all the itmes
			[theDataSourcePopUp removeAllItems];
			
			//	get the IDs of all the sources
			CAAutoArrayDelete<UInt32> theSourceList(theNumberSources);
			theDevice.GetAvailableDataSources(inSection, theNumberSources, theSourceList);
			
			//	add an item for each source, using the tag to carry the source ID
			for(UInt32 theSourceIndex = 0; theSourceIndex < theNumberSources; ++theSourceIndex)
			{
				//	get the ID of the source
				UInt32 theSourceID = theSourceList[theSourceIndex];
				
				//	get the name of the source
				NSString* theSourceName = (NSString*)theDevice.CopyDataSourceNameForID(inSection, theSourceID);
				[theSourceName autorelease];
				
				//	add it to the menu
				[theDataSourcePopUp addItemWithTitle: theSourceName];
				
				//	set the tag of the item to be the source ID
				[[theDataSourcePopUp lastItem] setTag: theSourceID];
			}
			
			//	set the selected item to the current data source
			UInt32 theCurrentSourceID = theDevice.GetCurrentDataSourceID(inSection);
			UInt32 theCurrentSourceIndex = [theDataSourcePopUp indexOfItemWithTag: theCurrentSourceID];
			[theDataSourcePopUp selectItemAtIndex: theCurrentSourceIndex];
			
			//	and enable the menu if the property is settable
			[theDataSourcePopUp setEnabled: theDevice.DataSourceControlIsSettable(inSection) ? YES : NO];
		}
		else
		{
			//	no data sources, remove all the times
			[theDataSourcePopUp removeAllItems];
			
			//	disable the whole thing
			[theDataSourcePopUp setEnabled: NO];
		}
	}
	else
	{
		[theDataSourcePopUp setEnabled: NO];
	}
	
	CACatch;
}

-(IBAction)	DataSourcePopUpAction:	(id)inSender
{
	CAAudioHardwareDeviceSectionID theSection = kAudioDeviceSectionInput;
	NSPopUpButton* theDataSourcePopUp = NULL;

	CATry;
	
	if(inSender == mInputDataSourcePopUp)
	{
		theSection = kAudioDeviceSectionInput;
		theDataSourcePopUp = mInputDataSourcePopUp;
	}
	else
	{
		theSection = kAudioDeviceSectionOutput;
		theDataSourcePopUp = mOutputDataSourcePopUp;
	}
	
	//	get the selected item
	NSMenuItem* theSelectedItem = [theDataSourcePopUp selectedItem];
	if(theSelectedItem != NULL)
	{
		//	the selected item's tag is the source ID to switch to
		UInt32 theSourceID = [theSelectedItem tag];
		
		//	tell the device about the new selection
		CAAudioHardwareDevice theDevice(mDevice);
		theDevice.SetCurrentDataSourceByID(theSection, theSourceID);
	}
	
	CACatch;
	
	//	upate the value
	[self UpdateDataSourceItems: theSection];
}

-(void)		UpdateIsConnectedItems:	(CAAudioHardwareDeviceSectionID)inSection
{
	CATry
	
	if(!mDeviceIsDead)
	{
		NSTextField* theIsConnectedTextField = ((inSection == kAudioDeviceSectionInput) ? mInputIsConnectedTextField : mOutputIsConnectedTextField);
		
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	the device's input is connected
		if(theDevice.HasIsConnectedStatus(inSection))
		{
			[theIsConnectedTextField setStringValue: (theDevice.GetIsConnectedStatus(inSection) ? @"Yes" : @"No")];
		}
		else
		{
			[theIsConnectedTextField setStringValue: @"N/A"];
		}
	}
	
	CACatch;
}

-(void)		UpdateStereoPairItems: (CAAudioHardwareDeviceSectionID)inSection
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	NSPopUpButton* theLeftPopUp = ((inSection == kAudioDeviceSectionInput) ? mInputLeftChannelPopUp : mOutputLeftChannelPopUp);
	NSPopUpButton* theRightPopUp = ((inSection == kAudioDeviceSectionInput) ? mInputRightChannelPopUp : mOutputRightChannelPopUp);
	
	if(!mDeviceIsDead)
	{
		if(theDevice.HasSection(inSection) && theDevice.HasPreferredStereoChannels(inSection))
		{
			//	get the default stereo pair
			UInt32 theLeft = 0;
			UInt32 theRight = 0;
			theDevice.GetPreferredStereoChannels(inSection, theLeft, theRight);
			
			//	they have to be greater than 0 to be valid
			if((theLeft > 0) && (theRight > 0))
			{
				//	enable the pop-ups
				[theLeftPopUp setEnabled: YES];
				[theRightPopUp setEnabled: YES];
			
				//	remove all the items from the pop-ups
				[theLeftPopUp removeAllItems];
				[theRightPopUp removeAllItems];
				
				//	get the total number of channels
				UInt32 theNumberChannels = theDevice.GetTotalNumberChannels(inSection);
				
				//	add a menu item for each channel
				for(UInt32 theChannel = 1; theChannel <= theNumberChannels; ++theChannel)
				{
					NSString* theItemString = [NSString stringWithFormat: @"%d", theChannel];
					[theLeftPopUp addItemWithTitle: theItemString];
					[theRightPopUp addItemWithTitle: theItemString];
				}
				
				//	select the appropriate items
				[theLeftPopUp selectItemAtIndex: theLeft - 1];
				[theRightPopUp selectItemAtIndex: theRight - 1];
			}
			else
			{
				//	remove all the items from the pop-ups
				[theLeftPopUp removeAllItems];
				[theRightPopUp removeAllItems];
				
				//	disable them
				[theLeftPopUp setEnabled: NO];
				[theRightPopUp setEnabled: NO];
			}
		}
		else
		{
			//	this device doesn't have input or doesn't have the preference
			
			//	remove all the items from the pop-ups
			[theLeftPopUp removeAllItems];
			[theRightPopUp removeAllItems];
			
			//	disable them
			[theLeftPopUp setEnabled: NO];
			[theRightPopUp setEnabled: NO];
		}
	}
	else
	{
		[theLeftPopUp setEnabled: NO];
		[theRightPopUp setEnabled: NO];
	}
	
	CACatch;
}

-(IBAction)	StereoPairPopUpAction:	(id)inSender
{
	CAAudioHardwareDeviceSectionID theSection = kAudioDeviceSectionInput;
	NSPopUpButton* theLeftPopUp = NULL;
	NSPopUpButton* theRightPopUp = NULL;
	
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	if((inSender == mInputLeftChannelPopUp) || (inSender == mInputRightChannelPopUp))
	{
		theSection = kAudioDeviceSectionInput;
		theLeftPopUp = mInputLeftChannelPopUp;
		theRightPopUp = mInputRightChannelPopUp;
	}
	else
	{
		theSection = kAudioDeviceSectionOutput;
		theLeftPopUp = mOutputLeftChannelPopUp;
		theRightPopUp = mOutputRightChannelPopUp;
	}

	//	get the index of the item selected in each menu
	UInt32 theLeft = [theLeftPopUp indexOfSelectedItem];
	UInt32 theRight = [theRightPopUp indexOfSelectedItem];
	
	//	add one since device channels are 1-based
	++theLeft;
	++theRight;
	
	//	set the preferred stereo pair
	theDevice.SetPreferredStereoChannels(theSection, theLeft, theRight);
	
	CACatch;
	
	//	upate the value
	[self UpdateStereoPairItems: theSection];
}

-(void)		UpdateISubItems
{
	CATry;
	
	if(!mDeviceIsDead)
	{
		if(mDevice != 0)
		{
			CAAudioHardwareDevice theDevice(mDevice);
			
			//	set the value or disable the control
			
			//	iSub Ownership
			if(theDevice.HasISubOwnershipControl())
			{
				[mOwnISubCheckBox setIntValue: theDevice.GetISubOwnershipControlValue() ? 1 : 0];
				[mOwnISubCheckBox setEnabled: theDevice.ISubOwnershipControlIsSettable() ? YES : NO];
			}
			else
			{
				[mOwnISubCheckBox setIntValue: 0];
				[mOwnISubCheckBox setEnabled: NO];
			}
			
			//	mute
			if(theDevice.HasSubMuteControl(0, kAudioDeviceSectionOutput))
			{
				[mISubMuteCheckBox setIntValue: theDevice.GetSubMuteControlValue(0, kAudioDeviceSectionOutput) ? 1 : 0];
				[mISubMuteCheckBox setEnabled: theDevice.SubMuteControlIsSettable(0, kAudioDeviceSectionOutput) ? YES : NO];
			}
			else
			{
				[mISubMuteCheckBox setIntValue: 0];
				[mISubMuteCheckBox setEnabled: NO];
			}
			
			//	volume
			if(theDevice.HasSubVolumeControl(0, kAudioDeviceSectionOutput))
			{
				[mISubVolumeSlider setFloatValue: theDevice.GetSubVolumeControlScalarValue(0, kAudioDeviceSectionOutput)];
				[mISubVolumeSlider setEnabled: theDevice.SubVolumeControlIsSettable(0, kAudioDeviceSectionOutput) ? YES : NO];
				
				[mISubVolumeTextField setStringValue: [NSString stringWithFormat: @"%8.3f", theDevice.GetSubVolumeControlDecibelValue(0, kAudioDeviceSectionOutput)]];
				[mISubVolumeTextField setEnabled: YES];
			}
			else
			{
				[mISubVolumeSlider setFloatValue: 1.0];
				[mISubVolumeSlider setEnabled: NO];
				
				[mISubVolumeTextField setStringValue: [NSString stringWithFormat: @"%8.3f", 1.0]];
				[mISubVolumeTextField setEnabled: NO];
			}
		}
		else
		{
			//	no device, disable all the controls
			
			//	iSub Ownership
			[mOwnISubCheckBox setIntValue: 0];
			[mOwnISubCheckBox setEnabled: NO];

			//	mute
			[mISubMuteCheckBox setIntValue: 0];
			[mISubMuteCheckBox setEnabled: NO];

			//	volume
			[mISubVolumeSlider setFloatValue: 1.0];
			[mISubVolumeSlider setEnabled: NO];
			
			[mISubVolumeTextField setStringValue: [NSString stringWithFormat: @"%8.3f", 1.0]];
			[mISubVolumeTextField setEnabled: NO];
		}
	}
	else
	{
		//	iSub Ownership
		[mOwnISubCheckBox setEnabled: NO];

		//	mute
		[mISubMuteCheckBox setEnabled: NO];

		//	volume
		[mISubVolumeSlider setEnabled: NO];
		[mISubVolumeTextField setEnabled: NO];
	}
	
	CACatch;
}

-(IBAction)	OwnISubCheckBoxAction:	(id)inSender
{
	CATry;
	
	//	get the value of the control
	bool theValue = [mOwnISubCheckBox intValue] != 0;
	
	//	set the hardware value
	CAAudioHardwareDevice theDevice(mDevice);
	theDevice.SetISubOwnershipControlValue(theValue);
	
	CACatch;
}

-(IBAction)	ISubMuteCheckBoxAction:	(id)inSender
{
	CATry;
	
	//	get the value of the control
	bool theValue = [mISubMuteCheckBox intValue] != 0;
	
	//	set the hardware value
	CAAudioHardwareDevice theDevice(mDevice);
	theDevice.SetSubMuteControlValue(0, kAudioDeviceSectionOutput, theValue);
	
	CACatch;
}

-(IBAction)	ISubVolumeSliderAction:	(id)inSender
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	get the value of the control
	Float32 theValue = [mISubVolumeSlider floatValue];
	
	//	convert that to dB
	Float32 theDBValue = theDevice.GetSubVolumeControlDecibelForScalarValue(0, kAudioDeviceSectionOutput, theValue);
	
	//	get the current dB value
	Float32 theCurrentDBValue = theDevice.GetSubVolumeControlDecibelValue(0, kAudioDeviceSectionOutput);
	
	//	only set the hardware value if it's different
	if(theDBValue != theCurrentDBValue)
	{
		theDevice.SetSubVolumeControlDecibelValue(0, kAudioDeviceSectionOutput, theDBValue);
	}
	
	CACatch;
}

-(void)		SetupAllStrips:	(CAAudioHardwareDeviceSectionID)inSection
{
	CATry;
	
	HLStripView* theStripView = ((inSection == kAudioDeviceSectionInput) ? mInputStripView : mOutputStripView);
	
	if(mDevice != 0)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	get the number channels
		UInt32 theNumberChannels = theDevice.GetTotalNumberChannels(inSection);
		
		if(theNumberChannels > 0)
		{
			//	set the size of the strip view
			[theStripView SetNumberStrips: theNumberChannels + 1];
			
			//	iterate through the channels and set up the controls
			for(UInt32 theChannelIndex = 0; theChannelIndex <= theNumberChannels; ++theChannelIndex)
			{
				CATry;
				[self SetupStripControls: theChannelIndex Section: inSection];
				CACatch;
			}
		}
		else
		{
			//	no channels, no controls
			[theStripView SetNumberStrips: 1];
			[self SetupStripControls: 0 Section: inSection];
		}
	}
	else
	{
		//	no channels, no controls
		[theStripView SetNumberStrips: 1];
		[self SetupStripControls: 0 Section: inSection];
	}
	
	CACatch;
}

#define	kChannelText		10
#define	kVolumeSlider		11
#define	kVolumeText			12
#define	kMuteCheckBox		13
#define	kPlayThruCheckBox	14

-(void)		SetupStripControls:	(UInt32)inStrip
			Section:			(CAAudioHardwareDeviceSectionID)inSection
{
	CATry;
	
	HLStripView* theStripView = ((inSection == kAudioDeviceSectionInput) ? mInputStripView : mOutputStripView);
	
	//	set the channel name
	if(inStrip == 0)
	{
		[theStripView SetStringValue: @"M" ForControl: kChannelText ForChannel: inStrip];
	}
	else
	{
		[theStripView SetIntValue: inStrip ForControl: kChannelText ForChannel: inStrip];
	}
	
	if(!mDeviceIsDead)
	{
		if(mDevice != 0)
		{
			CAAudioHardwareDevice theDevice(mDevice);
			
			//	set the value or disable the control
			
			//	volume
			CATry;
			if(theDevice.HasVolumeControl(inStrip, inSection))
			{
				[theStripView SetFloatValue: theDevice.GetVolumeControlScalarValue(inStrip, inSection) ForControl: kVolumeSlider ForChannel: inStrip];
				[theStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: theDevice.VolumeControlIsSettable(inStrip, inSection) ? YES : NO];
				
				[theStripView SetStringValue: [NSString stringWithFormat: @"%8.3f", theDevice.GetVolumeControlDecibelValue(inStrip, inSection)] ForControl: kVolumeText ForChannel: inStrip];
				[theStripView SetEnabled: kVolumeText ForChannel: inStrip Value: YES];
			}
			else
			{
				[theStripView SetFloatValue: 1.0 ForControl: kVolumeSlider ForChannel: inStrip];
				[theStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: NO];
				
				[theStripView SetStringValue: [NSString stringWithFormat: @"%8.3f", 1.0] ForControl: kVolumeText ForChannel: inStrip];
				[theStripView SetEnabled: kVolumeText ForChannel: inStrip Value: NO];
			}
			CACatch;
			
			//	mute
			CATry;
			if(theDevice.HasMuteControl(inStrip, inSection))
			{
				[theStripView SetBoolValue: theDevice.GetMuteControlValue(inStrip, inSection) ForControl: kMuteCheckBox ForChannel: inStrip];
				[theStripView SetEnabled: kMuteCheckBox ForChannel: inStrip Value: theDevice.MuteControlIsSettable(inStrip, inSection) ? YES : NO];
			}
			else
			{
				[theStripView SetBoolValue: false ForControl: kMuteCheckBox ForChannel: inStrip];
				[theStripView SetEnabled: kMuteCheckBox ForChannel: inStrip Value: NO];
			}
			CACatch;
			
			//	play thru
			CATry;
			if(theDevice.HasPlayThruControl(inStrip, inSection))
			{
				[theStripView SetBoolValue: theDevice.GetPlayThruControlValue(inStrip, inSection) ForControl: kPlayThruCheckBox ForChannel: inStrip];
				[theStripView SetEnabled: kPlayThruCheckBox ForChannel: inStrip Value: theDevice.PlayThruControlIsSettable(inStrip, inSection) ? YES : NO];
			}
			else
			{
				[theStripView SetBoolValue: false ForControl: kPlayThruCheckBox ForChannel: inStrip];
				[theStripView SetEnabled: kPlayThruCheckBox ForChannel: inStrip Value: NO];
			}
			CACatch;
		}
		else
		{
			//	no device, disable all the controls
			
			//	volume
			[theStripView SetFloatValue: 1.0 ForControl: kVolumeSlider ForChannel: inStrip];
			[theStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: NO];
			
			[theStripView SetStringValue: [NSString stringWithFormat: @"%8.3f", 1.0] ForControl: kVolumeText ForChannel: inStrip];
			[theStripView SetEnabled: kVolumeText ForChannel: inStrip Value: NO];
			
			//	mute
			[theStripView SetBoolValue: false ForControl: kMuteCheckBox ForChannel: inStrip];
			[theStripView SetEnabled: kMuteCheckBox ForChannel: inStrip Value: NO];
			
			//	play thru
			[theStripView SetBoolValue: false ForControl: kPlayThruCheckBox ForChannel: inStrip];
			[theStripView SetEnabled: kPlayThruCheckBox ForChannel: inStrip Value: NO];
		}
	}
	else
	{
		//	volume
		[theStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: NO];
		
		[theStripView SetEnabled: kVolumeText ForChannel: inStrip Value: NO];
		
		//	mute
		[theStripView SetEnabled: kMuteCheckBox ForChannel: inStrip Value: NO];
		
		//	play thru
		[theStripView SetEnabled: kPlayThruCheckBox ForChannel: inStrip Value: NO];
	}
	
	CACatch;
}

-(IBAction)	InputMuteMatrixAction:		(id)inSender
{
	CATry;
		
	//	get the newly selected cell
	NSCell* theCell = [inSender selectedCell];
	
	//	get the row and column that the cell is in
	int theRow = 0;
	int theColumn = 0;
	[inSender getRow: &theRow column: &theColumn ofCell: theCell];
	
	//	get the value of the cell
	bool theCellValue = [theCell intValue] != 0;
	
	//	set the hardware value
	CAAudioHardwareDevice theDevice(mDevice);
	theDevice.SetMuteControlValue(theRow, kAudioDeviceSectionInput, theCellValue);
	
	CACatch;
}

-(IBAction)	InputPlayThruMatrixAction:	(id)inSender
{
	CATry;
	
	//	get the newly selected cell
	NSCell* theCell = [inSender selectedCell];
	
	//	get the row and column that the cell is in
	int theRow = 0;
	int theColumn = 0;
	[inSender getRow: &theRow column: &theColumn ofCell: theCell];
	
	//	get the value of the cell
	bool theCellValue = [theCell intValue] != 0;
	
	//	set the hardware value
	CAAudioHardwareDevice theDevice(mDevice);
	theDevice.SetPlayThruControlValue(theRow, kAudioDeviceSectionInput, theCellValue);
	
	CACatch;
}

-(IBAction)	InputVolumeMatrixAction:	(id)inSender
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	get the newly selected cell
	NSCell* theCell = [inSender selectedCell];
	
	//	get the row and column that the cell is in
	int theRow = 0;
	int theColumn = 0;
	[inSender getRow: &theRow column: &theColumn ofCell: theCell];
	
	//	get the value of the cell
	Float32 theCellValue = [theCell floatValue];
	
	//	convert that to dB
	Float32 theCellDBValue = theDevice.GetVolumeControlDecibelForScalarValue(theRow, kAudioDeviceSectionInput, theCellValue);
	
	//	get the current dB value
	Float32 theCurrentDBValue = theDevice.GetVolumeControlDecibelValue(theRow, kAudioDeviceSectionInput);
	
	//	only set the hardware value if it's different
	if(theCellDBValue != theCurrentDBValue)
	{
		theDevice.SetVolumeControlDecibelValue(theRow, kAudioDeviceSectionInput, theCellDBValue);
	}
	
	CACatch;
}

-(IBAction)	OutputMuteMatrixAction:		(id)inSender
{
	CATry;
	
	//	get the newly selected cell
	NSCell* theCell = [inSender selectedCell];
	
	//	get the row and column that the cell is in
	int theRow = 0;
	int theColumn = 0;
	[inSender getRow: &theRow column: &theColumn ofCell: theCell];
	
	//	get the value of the cell
	bool theCellValue = [theCell intValue] != 0;
	
	//	set the hardware value
	CAAudioHardwareDevice theDevice(mDevice);
	theDevice.SetMuteControlValue(theRow, kAudioDeviceSectionOutput, theCellValue);
	
	CACatch;
}

-(IBAction)	OutputVolumeMatrixAction:	(id)inSender
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	get the newly selected cell
	NSCell* theCell = [inSender selectedCell];
	
	//	get the row and column that the cell is in
	int theRow = 0;
	int theColumn = 0;
	[inSender getRow: &theRow column: &theColumn ofCell: theCell];
	
	//	get the value of the cell
	Float32 theCellValue = [theCell floatValue];
	
	//	convert that to dB
	Float32 theCellDBValue = theDevice.GetVolumeControlDecibelForScalarValue(theRow, kAudioDeviceSectionOutput, theCellValue);
	
	//	get the current dB value
	Float32 theCurrentDBValue = theDevice.GetVolumeControlDecibelValue(theRow, kAudioDeviceSectionOutput);
	
	//	only set the hardware value if it's different
	if(theCellDBValue != theCurrentDBValue)
	{
		theDevice.SetVolumeControlDecibelValue(theRow, kAudioDeviceSectionOutput, theCellDBValue);
	}
	
	CACatch;
}

@end

OSStatus	HLDeviceWindowControlsControllerAudioDevicePropertyListenerProc(AudioDeviceID /*inDevice*/, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLDeviceWindowControlsController* inDeviceWindowControlsController)
{
	NS_DURING
	CATry;
	
	switch(inPropertyID)
	{
		case kAudioDevicePropertyDeviceIsAlive:
			inDeviceWindowControlsController->mDeviceIsDead = true;
			
			//	set up the data source controls
			[inDeviceWindowControlsController UpdateDataSourceItems: kAudioDeviceSectionInput];
			[inDeviceWindowControlsController UpdateDataSourceItems: kAudioDeviceSectionOutput];
			
			//	set up the is connected items
			[inDeviceWindowControlsController UpdateIsConnectedItems: kAudioDeviceSectionInput];
			[inDeviceWindowControlsController UpdateIsConnectedItems: kAudioDeviceSectionOutput];
			
			//	set up the stereo pair controls
			[inDeviceWindowControlsController UpdateStereoPairItems: kAudioDeviceSectionInput];
			[inDeviceWindowControlsController UpdateStereoPairItems: kAudioDeviceSectionOutput];
			
			//	update the strip view
			[inDeviceWindowControlsController SetupAllStrips: kAudioDeviceSectionInput];
			[inDeviceWindowControlsController SetupAllStrips: kAudioDeviceSectionOutput];
			
			//	updatae the iSub controls
			[inDeviceWindowControlsController UpdateISubItems];
			break;
		
		case kAudioDevicePropertyDeviceHasChanged:
			//	set up the data source controls
			[inDeviceWindowControlsController UpdateDataSourceItems: kAudioDeviceSectionInput];
			[inDeviceWindowControlsController UpdateDataSourceItems: kAudioDeviceSectionOutput];
			
			//	set up the is connected items
			[inDeviceWindowControlsController UpdateIsConnectedItems: kAudioDeviceSectionInput];
			[inDeviceWindowControlsController UpdateIsConnectedItems: kAudioDeviceSectionOutput];
			
			//	set up the stereo pair controls
			[inDeviceWindowControlsController UpdateStereoPairItems: kAudioDeviceSectionInput];
			[inDeviceWindowControlsController UpdateStereoPairItems: kAudioDeviceSectionOutput];
			
			//	update the strip view
			[inDeviceWindowControlsController SetupAllStrips: kAudioDeviceSectionInput];
			[inDeviceWindowControlsController SetupAllStrips: kAudioDeviceSectionOutput];
			
			//	updatae the iSub controls
			[inDeviceWindowControlsController UpdateISubItems];
			break;
			
		case kAudioDevicePropertyStreamConfiguration:
			[inDeviceWindowControlsController SetupAllStrips: (inIsInput ? kAudioDeviceSectionInput : kAudioDeviceSectionOutput)];
			[inDeviceWindowControlsController UpdateStereoPairItems: (inIsInput ? kAudioDeviceSectionInput : kAudioDeviceSectionOutput)];
			break;
			
		case kAudioDevicePropertyPreferredChannelsForStereo:
			[inDeviceWindowControlsController UpdateStereoPairItems: (inIsInput ? kAudioDeviceSectionInput : kAudioDeviceSectionOutput)];
			break;
			
		case kAudioDevicePropertyJackIsConnected:
			[inDeviceWindowControlsController UpdateIsConnectedItems: (inIsInput ? kAudioDeviceSectionInput : kAudioDeviceSectionOutput)];
			break;
			
		case kAudioDevicePropertyDataSource:
		case kAudioDevicePropertyDataSources:
			[inDeviceWindowControlsController UpdateDataSourceItems: (inIsInput ? kAudioDeviceSectionInput : kAudioDeviceSectionOutput)];
			break;
			
		case kAudioDevicePropertyVolumeScalar:
		case kAudioDevicePropertyVolumeRangeDecibels:
		case kAudioDevicePropertyMute:
		case kAudioDevicePropertyPlayThru:
			[inDeviceWindowControlsController SetupStripControls: inChannel Section: (inIsInput ? kAudioDeviceSectionInput : kAudioDeviceSectionOutput)];
			break;
		
		case kAudioDevicePropertyDriverShouldOwniSub:
		case kAudioDevicePropertySubMute:
		case kAudioDevicePropertySubVolumeScalar:
		case kAudioDevicePropertySubVolumeRangeDecibels:
			[inDeviceWindowControlsController UpdateISubItems];
			break;
	};
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	return 0;
}
