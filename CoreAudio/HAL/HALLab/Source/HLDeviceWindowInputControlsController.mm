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
	HLDeviceWindowInputControlsController.mm

==================================================================================================*/

//==================================================================================================
//	Includes
//==================================================================================================

//	Self Include
#include "HLDeviceWindowInputControlsController.h"

//	Local Includes
#include "HLDeviceWindowController.h"
#include "HLStripView.h"

//	PublicUtility Includes
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"

//==================================================================================================
//	HLDeviceWindowInputControlsController
//==================================================================================================

#define	kChannelText		10
#define	kVolumeSlider		11
#define	kVolumeText			12
#define	kMuteCheckBox		13
#define	kDataSourcePopUp	14
#define	kSoloCheckBox		15

static OSStatus	HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLDeviceWindowInputControlsController* inDeviceWindowInputControlsController);

@implementation HLDeviceWindowInputControlsController

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
	theDevice.AddPropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyDeviceHasChanged, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyDataSource, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyDataSources, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyPreferredChannelsForStereo, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyJackIsConnected, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyVolumeRangeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyMute, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertySolo, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc, self);
	
	//	set up the is connected items
	[self UpdateIsConnectedItems];
	
	//	set up the stereo pair controls
	[self UpdateStereoPairItems];
	
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
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyDeviceHasChanged, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyDataSource, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyDataSources, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyPreferredChannelsForStereo, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyJackIsConnected, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyVolumeRangeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyMute, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertySolo, (AudioDevicePropertyListenerProc)HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc);
	}
	
	[super dealloc];
}

-(void)		UpdateIsConnectedItems
{
	CATry
	
	if(!mDeviceIsDead)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	the device's input is connected
		if(theDevice.HasIsConnectedStatus(kAudioDeviceSectionInput))
		{
			[mIsConnectedTextField setStringValue: (theDevice.GetIsConnectedStatus(kAudioDeviceSectionInput) ? @"Yes" : @"No")];
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
		if(theDevice.HasSection(kAudioDeviceSectionInput) && theDevice.HasPreferredStereoChannels(kAudioDeviceSectionInput))
		{
			//	get the default stereo pair
			UInt32 theLeft = 0;
			UInt32 theRight = 0;
			theDevice.GetPreferredStereoChannels(kAudioDeviceSectionInput, theLeft, theRight);
			
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
				UInt32 theNumberChannels = theDevice.GetTotalNumberChannels(kAudioDeviceSectionInput);
				
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
			//	this device doesn't have input or doesn't have the preference
			
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
	theDevice.SetPreferredStereoChannels(kAudioDeviceSectionInput, theLeft, theRight);
	
	CACatch;
	
	//	upate the value
	[self UpdateStereoPairItems];
}

-(void)		SetupAllStrips
{
	CATry;
	
	if(mDevice != 0)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	get the number channels
		UInt32 theNumberChannels = theDevice.GetTotalNumberChannels(kAudioDeviceSectionInput);
		
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
			if(theDevice.HasVolumeControl(inStrip, kAudioDeviceSectionInput))
			{
				[mStripView SetFloatValue: theDevice.GetVolumeControlScalarValue(inStrip, kAudioDeviceSectionInput) ForControl: kVolumeSlider ForChannel: inStrip];
				[mStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: theDevice.VolumeControlIsSettable(inStrip, kAudioDeviceSectionInput) ? YES : NO];
				
				[mStripView SetStringValue: [NSString stringWithFormat: @"%8.3f", theDevice.GetVolumeControlDecibelValue(inStrip, kAudioDeviceSectionInput)] ForControl: kVolumeText ForChannel: inStrip];
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
			if(theDevice.HasMuteControl(inStrip, kAudioDeviceSectionInput))
			{
				[mStripView SetBoolValue: theDevice.GetMuteControlValue(inStrip, kAudioDeviceSectionInput) ForControl: kMuteCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kMuteCheckBox ForChannel: inStrip Value: theDevice.MuteControlIsSettable(inStrip, kAudioDeviceSectionInput) ? YES : NO];
			}
			else
			{
				[mStripView SetBoolValue: false ForControl: kMuteCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kMuteCheckBox ForChannel: inStrip Value: NO];
			}
			CACatch;
			
			//	solo
			CATry;
			if(theDevice.HasSoloControl(inStrip, kAudioDeviceSectionInput))
			{
				[mStripView SetBoolValue: theDevice.GetSoloControlValue(inStrip, kAudioDeviceSectionInput) ForControl: kSoloCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kSoloCheckBox ForChannel: inStrip Value: theDevice.SoloControlIsSettable(inStrip, kAudioDeviceSectionInput) ? YES : NO];
			}
			else
			{
				[mStripView SetBoolValue: false ForControl: kSoloCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kSoloCheckBox ForChannel: inStrip Value: NO];
			}
			CACatch;
			
			//	data source
			CATry;
			if(theDevice.HasDataSourceControl(inStrip, kAudioDeviceSectionInput))
			{
				UInt32 theNumberSources = theDevice.GetNumberAvailableDataSources(inStrip, kAudioDeviceSectionInput);
				
				//	there are data sources, so remove all the itmes
				[mStripView RemoveAllMenuItems: kDataSourcePopUp ForChannel: inStrip];
				
				//	get the IDs of all the sources
				CAAutoArrayDelete<UInt32> theSourceList(theNumberSources);
				theDevice.GetAvailableDataSources(inStrip, kAudioDeviceSectionInput, theNumberSources, theSourceList);
				
				//	add an item for each source, using the tag to carry the source ID
				for(UInt32 theSourceIndex = 0; theSourceIndex < theNumberSources; ++theSourceIndex)
				{
					//	get the ID of the source
					UInt32 theSourceID = theSourceList[theSourceIndex];
					
					//	get the name of the source
					NSString* theSourceName = (NSString*)theDevice.CopyDataSourceNameForID(inStrip, kAudioDeviceSectionInput, theSourceID);
					
					//	add the item to the menu
					[mStripView AppendMenuItem: theSourceName Tag: theSourceID ForControl: kDataSourcePopUp ForChannel: inStrip];
					
					//	release the name
					[theSourceName release];
				}
				
				//	set the selected item to the current data source
				UInt32 theCurrentSourceID = theDevice.GetCurrentDataSourceID(inStrip, kAudioDeviceSectionInput);
				[mStripView SetSelectedMenuItemByTag: theCurrentSourceID ForControl: kDataSourcePopUp ForChannel: inStrip];
				
				//	and enable the menu if the property is settable
				[mStripView SetEnabled: kDataSourcePopUp ForChannel: inStrip Value: theDevice.DataSourceControlIsSettable(inStrip, kAudioDeviceSectionInput) ? YES : NO];
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
	theDevice.SetMuteControlValue(theRow, kAudioDeviceSectionInput, theCellValue);
	
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
	theDevice.SetSoloControlValue(theRow, kAudioDeviceSectionInput, theCellValue);
	
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
		theDevice.SetCurrentDataSourceByID(theRow, kAudioDeviceSectionInput, theTag);
	}
	
	CACatch;
}

@end

static OSStatus	HLDeviceWindowInputControlsControllerAudioDevicePropertyListenerProc(AudioDeviceID /*inDevice*/, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLDeviceWindowInputControlsController* inDeviceWindowInputControlsController)
{
	NS_DURING
	CATry;
	
	switch(inPropertyID)
	{
		case kAudioDevicePropertyDeviceIsAlive:
			inDeviceWindowInputControlsController->mDeviceIsDead = true;
			
			//	set up the is connected items
			[inDeviceWindowInputControlsController UpdateIsConnectedItems];
			
			//	set up the stereo pair controls
			[inDeviceWindowInputControlsController UpdateStereoPairItems];
			
			//	update the strip view
			[inDeviceWindowInputControlsController SetupAllStrips];
			break;
		
		case kAudioDevicePropertyDeviceHasChanged:
			//	set up the is connected items
			[inDeviceWindowInputControlsController UpdateIsConnectedItems];
			
			//	set up the stereo pair controls
			[inDeviceWindowInputControlsController UpdateStereoPairItems];
			
			//	update the strip view
			[inDeviceWindowInputControlsController SetupAllStrips];
			break;
			
		case kAudioDevicePropertyStreamConfiguration:
			[inDeviceWindowInputControlsController SetupAllStrips];
			[inDeviceWindowInputControlsController UpdateStereoPairItems];
			break;
			
		case kAudioDevicePropertyPreferredChannelsForStereo:
			[inDeviceWindowInputControlsController UpdateStereoPairItems];
			break;
			
		case kAudioDevicePropertyJackIsConnected:
			[inDeviceWindowInputControlsController UpdateIsConnectedItems];
			break;
			
		case kAudioDevicePropertyVolumeScalar:
		case kAudioDevicePropertyVolumeRangeDecibels:
		case kAudioDevicePropertyMute:
		case kAudioDevicePropertySolo:
		case kAudioDevicePropertyDataSource:
		case kAudioDevicePropertyDataSources:
			[inDeviceWindowInputControlsController SetupStripControls: inChannel];
			break;
	};
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	return 0;
}
