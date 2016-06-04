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
	HLDeviceWindowThruControlsController.mm

==================================================================================================*/

//==================================================================================================
//	Includes
//==================================================================================================

//	Self Include
#include "HLDeviceWindowThruControlsController.h"

//	Local Includes
#include "HLDeviceWindowController.h"
#include "HLStripView.h"

//	PublicUtility Includes
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"

//==================================================================================================
//	HLDeviceWindowThruControlsController
//==================================================================================================

#define	kChannelText			10
#define	kVolumeSlider			11
#define	kVolumeText				12
#define	kOnOffCheckBox			13
#define	kDataDestinationPopUp	14
#define	kSoloCheckBox			15

static OSStatus	HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLDeviceWindowThruControlsController* inDeviceWindowThruControlsController);

@implementation HLDeviceWindowThruControlsController

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
	theDevice.AddPropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyDeviceHasChanged, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThruVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThruVolumeRangeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThru, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThruSolo, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThruDestination, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThruDestinations, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc, self);
	
	//	set up the scroll views
	[mScrollView setDrawsBackground: NO];
	[mScrollView setDocumentView: mStripView];
	[mScrollView setNeedsDisplay: YES];
	
	//	update the strip view
	[self SetupAllStrips];
	
	//	make sure the data destination pop-up matrix is hooked up
	[mStripView SetControl: kDataDestinationPopUp Target: self Action: @selector(DataDestinationMatrixAction:)];
	
	CACatch;
}

-(void)	dealloc
{
	if(!mDeviceIsDead)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	remove the listeners for the stuff we're tracking
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyDeviceHasChanged, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(0, kAudioDeviceSectionInput, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThruVolumeScalar, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThruVolumeRangeDecibels, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThru, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThruSolo, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThruDestination, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyPlayThruDestinations, (AudioDevicePropertyListenerProc)HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc);
	}
	
	[super dealloc];
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
			if(theDevice.HasPlayThruVolumeControl(inStrip))
			{
				[mStripView SetFloatValue: theDevice.GetPlayThruVolumeControlScalarValue(inStrip) ForControl: kVolumeSlider ForChannel: inStrip];
				[mStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: theDevice.PlayThruVolumeControlIsSettable(inStrip) ? YES : NO];
				
				[mStripView SetStringValue: [NSString stringWithFormat: @"%8.3f", theDevice.GetPlayThruVolumeControlDecibelValue(inStrip)] ForControl: kVolumeText ForChannel: inStrip];
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
			
			//	on/off
			CATry;
			if(theDevice.HasPlayThruOnOffControl(inStrip))
			{
				[mStripView SetBoolValue: theDevice.GetPlayThruOnOffControlValue(inStrip) ForControl: kOnOffCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kOnOffCheckBox ForChannel: inStrip Value: theDevice.PlayThruOnOffControlIsSettable(inStrip) ? YES : NO];
			}
			else
			{
				[mStripView SetBoolValue: false ForControl: kOnOffCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kOnOffCheckBox ForChannel: inStrip Value: NO];
			}
			CACatch;
			
			//	solo
			CATry;
			if(theDevice.HasPlayThruSoloControl(inStrip))
			{
				[mStripView SetBoolValue: theDevice.GetPlayThruSoloControlValue(inStrip) ForControl: kSoloCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kSoloCheckBox ForChannel: inStrip Value: theDevice.PlayThruSoloControlIsSettable(inStrip) ? YES : NO];
			}
			else
			{
				[mStripView SetBoolValue: false ForControl: kSoloCheckBox ForChannel: inStrip];
				[mStripView SetEnabled: kSoloCheckBox ForChannel: inStrip Value: NO];
			}
			CACatch;
			
			//	data destination
			CATry;
			if(theDevice.HasPlayThruDataDestinationControl(inStrip))
			{
				UInt32 theNumberDestinations = theDevice.GetNumberAvailablePlayThruDataDestinations(inStrip);
				
				//	there are data destinations, so remove all the itmes
				[mStripView RemoveAllMenuItems: kDataDestinationPopUp ForChannel: inStrip];
				
				//	get the IDs of all the destinations
				CAAutoArrayDelete<UInt32> theDestinationList(theNumberDestinations);
				theDevice.GetAvailablePlayThruDataDestinations(inStrip, theNumberDestinations, theDestinationList);
				
				//	add an item for each destination, using the tag to carry the destination ID
				for(UInt32 theDestinationIndex = 0; theDestinationIndex < theNumberDestinations; ++theDestinationIndex)
				{
					//	get the ID of the destination
					UInt32 theDestinationID = theDestinationList[theDestinationIndex];
					
					//	get the name of the destination
					NSString* theDestinationName = (NSString*)theDevice.CopyPlayThruDataDestinationNameForID(inStrip, theDestinationID);
					
					//	add the item to the menu
					[mStripView AppendMenuItem: theDestinationName Tag: theDestinationID ForControl: kDataDestinationPopUp ForChannel: inStrip];
					
					//	release the name
					[theDestinationName release];
				}
				
				//	set the selected item to the current data destination
				UInt32 theCurrentDestinationID = theDevice.GetCurrentPlayThruDataDestinationID(inStrip);
				[mStripView SetSelectedMenuItemByTag: theCurrentDestinationID ForControl: kDataDestinationPopUp ForChannel: inStrip];
				
				//	and enable the menu if the property is settable
				[mStripView SetEnabled: kDataDestinationPopUp ForChannel: inStrip Value: theDevice.PlayThruDataDestinationControlIsSettable(inStrip) ? YES : NO];
			}
			else
			{
				[mStripView RemoveAllMenuItems: kDataDestinationPopUp ForChannel: inStrip];
				[mStripView AppendMenuItem: @"N/A" Tag: 1 ForControl: kDataDestinationPopUp ForChannel: inStrip];
				[mStripView SetSelectedMenuItemByTag: 1 ForControl: kDataDestinationPopUp ForChannel: inStrip];
				[mStripView SetEnabled: kDataDestinationPopUp ForChannel: inStrip Value: NO];
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
			
			//	on/off
			[mStripView SetBoolValue: false ForControl: kOnOffCheckBox ForChannel: inStrip];
			[mStripView SetEnabled: kOnOffCheckBox ForChannel: inStrip Value: NO];
			
			//	Solo
			[mStripView SetBoolValue: false ForControl: kSoloCheckBox ForChannel: inStrip];
			[mStripView SetEnabled: kSoloCheckBox ForChannel: inStrip Value: NO];
			
			//	data destination
			[mStripView RemoveAllMenuItems: kDataDestinationPopUp ForChannel: inStrip];
			[mStripView AppendMenuItem: @"N/A" Tag: 1 ForControl: kDataDestinationPopUp ForChannel: inStrip];
			[mStripView SetSelectedMenuItemByTag: 1 ForControl: kDataDestinationPopUp ForChannel: inStrip];
			[mStripView SetEnabled: kDataDestinationPopUp ForChannel: inStrip Value: NO];
		}
	}
	else
	{
		//	volume
		[mStripView SetEnabled: kVolumeSlider ForChannel: inStrip Value: NO];
		
		[mStripView SetEnabled: kVolumeText ForChannel: inStrip Value: NO];
		
		//	on/off
		[mStripView SetEnabled: kOnOffCheckBox ForChannel: inStrip Value: NO];
			
		//	Solo
		[mStripView SetEnabled: kSoloCheckBox ForChannel: inStrip Value: NO];
			
		//	data destination
		[mStripView SetEnabled: kDataDestinationPopUp ForChannel: inStrip Value: NO];
	}
	
	CACatch;
}

-(IBAction)	OnOffMatrixAction:		(id)inSender
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
	theDevice.SetPlayThruOnOffControlValue(theRow, theCellValue);
	
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
	theDevice.SetPlayThruSoloControlValue(theRow, theCellValue);
	
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
	Float32 theCellDBValue = theDevice.GetPlayThruVolumeControlDecibelForScalarValue(theRow, theCellValue);
	
	//	get the current dB value
	Float32 theCurrentDBValue = theDevice.GetPlayThruVolumeControlDecibelValue(theRow);
	
	//	only set the hardware value if it's different
	if(theCellDBValue != theCurrentDBValue)
	{
		theDevice.SetPlayThruVolumeControlDecibelValue(theRow, theCellDBValue);
	}
	
	CACatch;
}

-(IBAction)	DataDestinationMatrixAction:		(id)inSender
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
		theDevice.SetCurrentPlayThruDataDestinationByID(theRow, theTag);
	}
	
	CACatch;
}

@end

static OSStatus	HLDeviceWindowThruControlsControllerAudioDevicePropertyListenerProc(AudioDeviceID /*inDevice*/, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLDeviceWindowThruControlsController* inDeviceWindowThruControlsController)
{
	NS_DURING
	CATry;
	
	switch(inPropertyID)
	{
		case kAudioDevicePropertyDeviceIsAlive:
			inDeviceWindowThruControlsController->mDeviceIsDead = true;
			[inDeviceWindowThruControlsController SetupAllStrips];
			break;
		
		case kAudioDevicePropertyDeviceHasChanged:
			[inDeviceWindowThruControlsController SetupAllStrips];
			break;
			
		case kAudioDevicePropertyStreamConfiguration:
			[inDeviceWindowThruControlsController SetupAllStrips];
			break;
			
		case kAudioDevicePropertyPlayThruVolumeScalar:
		case kAudioDevicePropertyPlayThruVolumeRangeDecibels:
		case kAudioDevicePropertyPlayThru:
			[inDeviceWindowThruControlsController SetupStripControls: inChannel];
			break;
	};
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	return 0;
}
