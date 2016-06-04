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
	HLDeviceMenuController.mm

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "HLDeviceMenuController.h"

//	PublicUtility Includes
#include "CAAudioHardwareSystem.h"
#include "CAAudioHardwareDevice.h"
#include "CADebugMacros.h"
#include "CAException.h"

//=============================================================================
//	HLDeviceMenuController
//=============================================================================

@implementation HLDeviceMenuController

-(id)	init
{
	CATry;
	
	mDevicePopUp = NULL;
	mDelegate = NULL;
	mDeviceID = 0;
	mAudioHardwarePropertyListenerTink = new CATink<AudioHardwarePropertyListenerProc>((AudioHardwarePropertyListenerProc)HLDeviceMenuController_AudioHardwarePropertyListener);
	
	CACatch;
	
	return [super init];
}

-(void)	awakeFromNib
{
	CATry;
	
	//	get the initial selection from the delegate
	if(mDelegate != NULL)
	{
		mDeviceID = [mDelegate GetInitialSelectedDevice: self];
	}
	
	//	update the pop-up
	[self UpdateDevicePopUp];
	
	//	sign up for device list notifications
	CAAudioHardwareSystem::AddPropertyListener(kAudioHardwarePropertyDevices, (AudioHardwarePropertyListenerProc)mAudioHardwarePropertyListenerTink, self);
	
	CACatch;
}

-(void)	dealloc
{
	CATry;
	
	CAAudioHardwareSystem::RemovePropertyListener(kAudioHardwarePropertyDevices, (AudioHardwarePropertyListenerProc)mAudioHardwarePropertyListenerTink);
	delete mAudioHardwarePropertyListenerTink;
	[super dealloc];
	
	CACatch;
}

-(AudioDeviceID)	GetSelectedAudioDevice
{
	return mDeviceID;
}

-(void)	SetSelectedAudioDevice:	(AudioDeviceID)inDeviceID
{
	if(mDeviceID != inDeviceID)
	{
		//	only accept the new device if it is in the menu
		int theDeviceIndex = [mDevicePopUp indexOfItemWithTag: inDeviceID];
		if(theDeviceIndex != -1)
		{
			AudioDeviceID theOldDeviceID = mDeviceID;
			mDeviceID = inDeviceID;
			[mDevicePopUp selectItemAtIndex: theDeviceIndex];
			
			//	tell the delegate that things have changed
			if(mDelegate != NULL)
			{
				[mDelegate SelectedDeviceChanged: self OldDevice: theOldDeviceID NewDevice: mDeviceID];
			}
		}
	}
}

-(id)	GetDelegate
{
	return mDelegate;
}

-(void)	SetDelegate:	(id)inDelegate
{
	mDelegate = inDelegate;
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
		
		if(mDeviceID != theDeviceID)
		{
			AudioDeviceID theOldDeviceID = mDeviceID;
			
			//	cache the state
			mDeviceID = theDeviceID;
			
			//	tell the delegate
			if(mDelegate != NULL)
			{
				[mDelegate SelectedDeviceChanged: self OldDevice: theOldDeviceID NewDevice: mDeviceID];
			}
		}
	}
	
	CACatch;
}

-(void)	UpdateDevicePopUp
{
	CATry;
	
	//	remove all the current items
	[mDevicePopUp removeAllItems];
	
	//	iterate across the devices and add the names of the devices to the menu
	UInt32 theNumberDevices = CAAudioHardwareSystem::GetNumberDevices();
	if(theNumberDevices > 0)
	{
		UInt32 theDeviceIndex = 0;
		while(theDeviceIndex < theNumberDevices)
		{
			//	get the device
			CAAudioHardwareDevice theDevice(CAAudioHardwareSystem::GetDeviceAtIndex(theDeviceIndex));
			
			//	only add it if it is acceptable to the delegate
			bool shouldAddToMenu = true;
			if(mDelegate != NULL)
			{
				shouldAddToMenu = [mDelegate ShouldDeviceBeInMenu: self Device: theDevice.GetAudioDeviceID()] == YES;
			}
			
			if(shouldAddToMenu)
			{
				//  append a menu item with fake name
				[mDevicePopUp addItemWithTitle: @"____FOO____BAR____"];
				
				//  grab it
				NSMenuItem* theMenuItem = [mDevicePopUp lastItem];
			
				//	get the device's name
				NSString* theDeviceName = (NSString*)theDevice.CopyName();
				
				//  set the menu item's title to the device's name
				[theMenuItem setTitle: theDeviceName];
				
				//	also set the tag for the menu item to be the AudioDeviceID
				[[mDevicePopUp lastItem] setTag: theDevice.GetAudioDeviceID()];
				
				//  release the device name
				[theDeviceName release];
			}
			
			//	go to the next one
			++theDeviceIndex;
		}
	}
	
	//	set up the value of the pop-up
	if([mDevicePopUp numberOfItems] != 0)
	{
		//	set the value of the menu to the current device
		int theDeviceIndex = [mDevicePopUp indexOfItemWithTag: mDeviceID];
		if(theDeviceIndex != -1)
		{
			[mDevicePopUp selectItemAtIndex: theDeviceIndex];
		}
		else
		{
			//	the cached device is no longer in the menu, so pick a new one
			AudioDeviceID theOldDeviceID = mDeviceID;
			mDeviceID = [[mDevicePopUp itemAtIndex: 0] tag];
			
			//	tell the delegate that things have changed
			if(mDelegate != NULL)
			{
				[mDelegate SelectedDeviceChanged: self OldDevice: theOldDeviceID NewDevice: mDeviceID];
			}
		}
	}
	else
	{
		//	no devices, so put "None" in the menu
		[mDevicePopUp addItemWithTitle: @"None"];
		[[mDevicePopUp lastItem] setTag: 0];
		[mDevicePopUp selectItemAtIndex: 0];
		mDeviceID = 0;
	}
	
	CACatch;
}

@end

OSStatus	HLDeviceMenuController_AudioHardwarePropertyListener(AudioHardwarePropertyID inPropertyID, HLDeviceMenuController* inDeviceMenuController)
{
	NS_DURING
	CATry;
	
	switch(inPropertyID)
	{
		case kAudioHardwarePropertyDevices:
			[inDeviceMenuController UpdateDevicePopUp];
			break;
			
		default:
			break;
	};
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER

	return 0;
}
