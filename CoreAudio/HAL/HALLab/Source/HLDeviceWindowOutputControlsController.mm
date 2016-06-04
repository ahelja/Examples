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
/*==================================================================================================
	HLDeviceWindowOutputControlsController.mm

==================================================================================================*/

//==================================================================================================
//	Includes
//==================================================================================================

//	Self Include
#include "HLDeviceWindowOutputControlsController.h"

//	Local Includes
#include "HLDeviceWindowController.h"
#include "HLStripView.h"

//	PublicUtility Includes
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"

//==================================================================================================
//	HLDeviceWindowOutputControlsController
//==================================================================================================

#define	kChannelText		10
#define	kVolumeSlider		11
#define	kVolumeText			12
#define	kMuteCheckBox		13
#define	kDataSourcePopUp	14
#define	kSoloCheckBox		15

static OSStatus	HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLDeviceWindowOutputControlsController* inDeviceWindowOutputControlsController);

@implementation HLDeviceWindowOutputControlsController

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
	theDevice.AddPropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDeviceHasChanged, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyDataSource, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyDataSources, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyPreferredChannelsForStereo, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyJackIsConnected, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyVolumeRangeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyMute, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertySolo, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDriverShouldOwniSub, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertySubMute, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertySubVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertySubVolumeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc, self);
	
	//	set up the is connected items
	[self UpdateIsConnectedItems];
	
	//	set up the stereo pair controls
	[self UpdateStereoPairItems];
	
	//	set up the iSub controls
	[self UpdateISubItems];
	
	//	set up the scroll views
	[mScrollView setDrawsBackground: NO];
	[mScrollView setDocumentView: mStripView];
	[mScrollView setNeedsDisplay: YES];
	
	//	update the strip view
	[self SetupAllStrips];
	
	//	make sure the data source pop-up matrix is hooked up
	[mStripView SetControl: kDataSourcePopUp Target: self Action: @selector(DataSourceMatrixAction:)];
	
	CACatch;
}

-(void)	dealloc
{
	if(!mDeviceIsDead)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	remove the listeners for the stuff we're tracking
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDeviceHasChanged, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDataSource, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyDataSources, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyPreferredChannelsForStereo, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyJackIsConnected, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyVolumeRangeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyMute, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertySolo, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDriverShouldOwniSub, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertySubMute, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertySubVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertySubVolumeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc);
	}
	
	[super dealloc];
}

-(void)		UpdateIsConnectedItems
{
	CATry
	
	if(!mDeviceIsDead)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	the device's output is connected
		if(theDevice.HasIsConnectedStatus(kAudioDeviceSectionOutput))
		{
			[mIsConnectedTextField setStringValue: (theDevice.GetIsConnectedStatus(kAudioDeviceSectionOutput) ? @"Yes" : @"No")];
		}
		else
		{
			[mIsConnectedTextField setStringValue: @"N/A"];
		}
	}
	
	CACatch;
}

-(void)		UpdateStereoPairItems
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	if(!mDeviceIsDead)
	{
		if(theDevice.HasSection(kAudioDeviceSectionOutput) && theDevice.HasPreferredStereoChannels(kAudioDeviceSectionOutput))
		{
			//	get the default stereo pair
			UInt32 theLeft = 0;
			UInt32 theRight = 0;
			theDevice.GetPreferredStereoChannels(kAudioDeviceSectionOutput, theLeft, theRight);
			
			//	they have to be greater than 0 to be valid
			if((theLeft > 0) && (theRight > 0))
			{
				//	enable the pop-ups
				[mLeftChannelPopUp setEnabled: YES];
				[mRightChannelPopUp setEnabled: YES];
			
				//	remove all the items from the pop-ups
				[mLeftChannelPopUp removeAllItems];
				[mRightChannelPopUp removeAllItems];
				
				//	get the total number of channels
				UInt32 theNumberChannels = theDevice.GetTotalNumberChannels(kAudioDeviceSectionOutput);
				
				//	add a menu item for each channel
				for(UInt32 theChannel = 1; theChannel <= theNumberChannels; ++theChannel)
				{
					NSString* theItemString = [NSString stringWithFormat: @"%d", theChannel];
					[mLeftChannelPopUp addItemWithTitle: theItemString];
					[mRightChannelPopUp addItemWithTitle: theItemString];
				}
				
				//	select the appropriate items
				[mLeftChannelPopUp selectItemAtIndex: theLeft - 1];
				[mRightChannelPopUp selectItemAtIndex: theRight - 1];
			}
			else
			{
				//	remove all the items from the pop-ups
				[mLeftChannelPopUp removeAllItems];
				[mRightChannelPopUp removeAllItems];
				
				//	disable them
				[mLeftChannelPopUp setEnabled: NO];
				[mRightChannelPopUp setEnabled: NO];
			}
		}
		else
		{
			//	this device doesn't have output or doesn't have the preference
			
			//	remove all the items from the pop-ups
			[mLeftChannelPopUp removeAllItems];
			[mRightChannelPopUp removeAllItems];
			
			//	disable them
			[mLeftChannelPopUp setEnabled: NO];
			[mRightChannelPopUp setEnabled: NO];
		}
	}
	else
	{
		[mLeftChannelPopUp setEnabled: NO];
		[mRightChannelPopUp setEnabled: NO];
	}
	
	CACatch;
}

-(IBAction)	StereoPairPopUpAction:	(id)inSender
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	get the index of the item selected in each menu
	UInt32 theLeft = [mLeftChannelPopUp indexOfSelectedItem];
	UInt32 theRight = [mRightChannelPopUp indexOfSelectedItem];
	
	//	add one since device channels are 1-based
	++theLeft;
	++theRight;
	
	//	set the preferred stereo pair
	theDevice.SetPreferredStereoChannels(kAudioDeviceSectionOutput, theLeft, theRight);
	
	CACatch;
	
	//	upate the value
	[self UpdateStereoPairItems];
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

-(void)		SetupAllStrips
{
	CATry;
	
	if(mDevice != 0)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	get the number channels
		UInt32 theNumberChannels = theDevice.GetTotalNumberChannels(kAudioDeviceSectionOutput);
		
		if(theNumberChannels > 0)
		{
			//	set the size of the strip view
			[mStripView SetNumberStrips: theNumberChannels + 1];
			
			//	iterate through the channels and set up the controls
			for(UInt32 theChannelIndex = 0; theChannelIndex <= theNumberChannels; ++theChannelIndex)
			{
				CATry;
				[self SetupStripControls: theChannelIndex];
				CACatch;
			}
		}
		else
		{
			//	no channels, no controls
			[mStripView SetNumberStrips: 1];
			[self SetupStripControls: 0];
		}
	}
	else
	{
		//	no channels, no controls
		[mStripView SetNumberStrips: 1];
		[self SetupStripControls: 0];
	}
	
	CACatch;
}

-(void)		SetupStripControls:	(UInt32)inStrip
{
	CATry;
	
	//	set the channel name
	if(inStrip == 0)
	{
		[mStripView SetStringValue: @"M" ForControl: kChannelText ForChannel: inStrip];
	}
	else
	{
		[mStripView SetIntValue: inStrip ForControl: kChannelText ForChannel: inStrip];
	}
	
	if(!mDeviceIsDead)
	{
		if(mDevice != 0)
		{
			CAAudioHardwareDevice theDevice(mDevice);
			
			//	set the value or disable the control
			
			//	volume
			CATry;
			if(theDevice.HasVolumeControl(inStrip, kAudioDeviceSectionOutput))
			{
				[mStripView SetFloatValue: theDevice.GetVolumeControlScalarValue(inStrip, kAudioDeviceSectionOutput) ForControl: kVolumeSlider ForChannel: inStrip];
				[mStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: theDevice.VolumeControlIsSettable(inStrip, kAudioDeviceSectionOutput) ? YES : NO];
				
				[mStripView SetStringValue: [NSString stringWithFormat: @"%8.3f", theDevice.GetVolumeControlDecibelValue(inStrip, kAudioDeviceSectionOutput)] ForControl: kVolumeText ForChannel: inStrip];
				[mStripView SetEnabled: kVolumeText ForChannel: inStrip Value: YES];
			}
			else
			{
				[mStripView SetFloatValue: 1.0 ForControl: kVolumeSlider ForChannel: inStrip];
				[mStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: NO];
				
				[mStripView SetStringValue: [NSString stringWithFormat: @"%8.3f", 1.0] ForControl: kVolumeText ForChannel: inStrip];
				[mStripView SetEnabled: kVolumeText ForChannel: inStrip Value: NO];
			}
			CACatch;
			
			//	mute
			CATry;
			if(theDevice.HasMuteControl(inStrip, kAudioDeviceSectionOutput))
			{
				[mStripView SetBoolValue: theDevice.GetMuteControlValue(inStrip, kAudioDeviceSectionOutput) ForControl: kMuteCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kMuteCheckBox ForChannel: inStrip Value: theDevice.MuteControlIsSettable(inStrip, kAudioDeviceSectionOutput) ? YES : NO];
			}
			else
			{
				[mStripView SetBoolValue: false ForControl: kMuteCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kMuteCheckBox ForChannel: inStrip Value: NO];
			}
			CACatch;
			
			//	solo
			CATry;
			if(theDevice.HasSoloControl(inStrip, kAudioDeviceSectionOutput))
			{
				[mStripView SetBoolValue: theDevice.GetSoloControlValue(inStrip, kAudioDeviceSectionOutput) ForControl: kSoloCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kSoloCheckBox ForChannel: inStrip Value: theDevice.SoloControlIsSettable(inStrip, kAudioDeviceSectionOutput) ? YES : NO];
			}
			else
			{
				[mStripView SetBoolValue: false ForControl: kSoloCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kSoloCheckBox ForChannel: inStrip Value: NO];
			}
			CACatch;
			
			//	data source
			CATry;
			if(theDevice.HasDataSourceControl(inStrip, kAudioDeviceSectionOutput))
			{
				UInt32 theNumberSources = theDevice.GetNumberAvailableDataSources(inStrip, kAudioDeviceSectionOutput);
				
				//	there are data sources, so remove all the itmes
				[mStripView RemoveAllMenuItems: kDataSourcePopUp ForChannel: inStrip];
				
				//	get the IDs of all the sources
				CAAutoArrayDelete<UInt32> theSourceList(theNumberSources);
				theDevice.GetAvailableDataSources(inStrip, kAudioDeviceSectionOutput, theNumberSources, theSourceList);
				
				//	add an item for each source, using the tag to carry the source ID
				for(UInt32 theSourceIndex = 0; theSourceIndex < theNumberSources; ++theSourceIndex)
				{
					//	get the ID of the source
					UInt32 theSourceID = theSourceList[theSourceIndex];
					
					//	get the name of the source
					NSString* theSourceName = (NSString*)theDevice.CopyDataSourceNameForID(inStrip, kAudioDeviceSectionOutput, theSourceID);
					
					//	add the item to the menu
					[mStripView AppendMenuItem: theSourceName Tag: theSourceID ForControl: kDataSourcePopUp ForChannel: inStrip];
					
					//	release the name
					[theSourceName release];
				}
				
				//	set the selected item to the current data source
				UInt32 theCurrentSourceID = theDevice.GetCurrentDataSourceID(inStrip, kAudioDeviceSectionOutput);
				[mStripView SetSelectedMenuItemByTag: theCurrentSourceID ForControl: kDataSourcePopUp ForChannel: inStrip];
				
				//	and enable the menu if the property is settable
				[mStripView SetEnabled: kDataSourcePopUp ForChannel: inStrip Value: theDevice.DataSourceControlIsSettable(inStrip, kAudioDeviceSectionOutput) ? YES : NO];
			}
			else
			{
				[mStripView RemoveAllMenuItems: kDataSourcePopUp ForChannel: inStrip];
				[mStripView AppendMenuItem: @"N/A" Tag: 1 ForControl: kDataSourcePopUp ForChannel: inStrip];
				[mStripView SetSelectedMenuItemByTag: 1 ForControl: kDataSourcePopUp ForChannel: inStrip];
				[mStripView SetEnabled: kDataSourcePopUp ForChannel: inStrip Value: NO];
			}
			CACatch;
		}
		else
		{
			//	no device, disable all the controls
			
			//	volume
			[mStripView SetFloatValue: 1.0 ForControl: kVolumeSlider ForChannel: inStrip];
			[mStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: NO];
			
			[mStripView SetStringValue: [NSString stringWithFormat: @"%8.3f", 1.0] ForControl: kVolumeText ForChannel: inStrip];
			[mStripView SetEnabled: kVolumeText ForChannel: inStrip Value: NO];
			
			//	mute
			[mStripView SetBoolValue: false ForControl: kMuteCheckBox ForChannel: inStrip];
			[mStripView SetEnabled: kMuteCheckBox ForChannel: inStrip Value: NO];
			
			//	solo
			[mStripView SetBoolValue: false ForControl: kSoloCheckBox ForChannel: inStrip];
			[mStripView SetEnabled: kSoloCheckBox ForChannel: inStrip Value: NO];
			
			//	data source
			[mStripView RemoveAllMenuItems: kDataSourcePopUp ForChannel: inStrip];
			[mStripView AppendMenuItem: @"N/A" Tag: 1 ForControl: kDataSourcePopUp ForChannel: inStrip];
			[mStripView SetSelectedMenuItemByTag: 1 ForControl: kDataSourcePopUp ForChannel: inStrip];
			[mStripView SetEnabled: kDataSourcePopUp ForChannel: inStrip Value: NO];
		}
	}
	else
	{
		//	volume
		[mStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: NO];
		
		[mStripView SetEnabled: kVolumeText ForChannel: inStrip Value: NO];
		
		//	mute
		[mStripView SetEnabled: kMuteCheckBox ForChannel: inStrip Value: NO];
			
		//	solo
		[mStripView SetEnabled: kSoloCheckBox ForChannel: inStrip Value: NO];
			
		//	data source
		[mStripView SetEnabled: kDataSourcePopUp ForChannel: inStrip Value: NO];
	}
	
	CACatch;
}

-(IBAction)	MuteMatrixAction:		(id)inSender
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

-(IBAction)	SoloMatrixAction:		(id)inSender
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
	theDevice.SetSoloControlValue(theRow, kAudioDeviceSectionOutput, theCellValue);
	
	CACatch;
}

-(IBAction)	VolumeMatrixAction:	(id)inSender
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

-(IBAction)	DataSourceMatrixAction:		(id)inSender
{
	CATry;
		
	//	get the newly selected cell
	NSPopUpButtonCell* theCell = (NSPopUpButtonCell*)[inSender selectedCell];
	
	//	get the row and column that the cell is in
	int theRow = 0;
	int theColumn = 0;
	[inSender getRow: &theRow column: &theColumn ofCell: theCell];
	
	//	get the value of the cell
	NSMenuItem* theSelectedItem = [theCell selectedItem];
	if(theSelectedItem != NULL)
	{
		//	return the selected item's tag
		UInt32 theTag = [theSelectedItem tag];
	
		//	set the hardware value
		CAAudioHardwareDevice theDevice(mDevice);
		theDevice.SetCurrentDataSourceByID(theRow, kAudioDeviceSectionOutput, theTag);
	}
	
	CACatch;
}

@end

static OSStatus	HLDeviceWindowOutputControlsControllerAudioDevicePropertyListenerProc(AudioDeviceID /*inDevice*/, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLDeviceWindowOutputControlsController* inDeviceWindowOutputControlsController)
{
	NS_DURING
	CATry;
	
	switch(inPropertyID)
	{
		case kAudioDevicePropertyDeviceIsAlive:
			inDeviceWindowOutputControlsController->mDeviceIsDead = true;
			
			//	set up the is connected items
			[inDeviceWindowOutputControlsController UpdateIsConnectedItems];
			
			//	set up the stereo pair controls
			[inDeviceWindowOutputControlsController UpdateStereoPairItems];
			
			//	update the strip view
			[inDeviceWindowOutputControlsController SetupAllStrips];
			
			//	updatae the iSub controls
			[inDeviceWindowOutputControlsController UpdateISubItems];
			break;
		
		case kAudioDevicePropertyDeviceHasChanged:
			//	set up the is connected items
			[inDeviceWindowOutputControlsController UpdateIsConnectedItems];
			
			//	set up the stereo pair controls
			[inDeviceWindowOutputControlsController UpdateStereoPairItems];
			
			//	update the strip view
			[inDeviceWindowOutputControlsController SetupAllStrips];
			
			//	updatae the iSub controls
			[inDeviceWindowOutputControlsController UpdateISubItems];
			break;
			
		case kAudioDevicePropertyStreamConfiguration:
			[inDeviceWindowOutputControlsController SetupAllStrips];
			[inDeviceWindowOutputControlsController UpdateStereoPairItems];
			break;
			
		case kAudioDevicePropertyPreferredChannelsForStereo:
			[inDeviceWindowOutputControlsController UpdateStereoPairItems];
			break;
			
		case kAudioDevicePropertyJackIsConnected:
			[inDeviceWindowOutputControlsController UpdateIsConnectedItems];
			break;
			
		case kAudioDevicePropertyVolumeScalar:
		case kAudioDevicePropertyVolumeRangeDecibels:
		case kAudioDevicePropertyMute:
		case kAudioDevicePropertySolo:
		case kAudioDevicePropertyDataSource:
		case kAudioDevicePropertyDataSources:
			[inDeviceWindowOutputControlsController SetupStripControls: inChannel];
			break;
		
		case kAudioDevicePropertyDriverShouldOwniSub:
		case kAudioDevicePropertySubMute:
		case kAudioDevicePropertySubVolumeScalar:
		case kAudioDevicePropertySubVolumeRangeDecibels:
			[inDeviceWindowOutputControlsController UpdateISubItems];
			break;
	};
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	return 0;
}
