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
	HLDeviceWindowInfoController.mm

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "HLDeviceWindowInfoController.h"

//	Local Includes
#include "HLDeviceWindowController.h"

//	PublicUtility Includes
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareStream.h"
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"

//	System Includes
#include <ApplicationServices/ApplicationServices.h>
#include <unistd.h>

//	Standard Library Includes
#include <algorithm>

//=============================================================================
//	HLDeviceWindowInfoController
//=============================================================================

static bool		IsRateCommon(Float64 inRate);
static Float64	GetCommonSampleRateInRangeByIndex(Float64 inMinimumRate, Float64 inMaximumRate, UInt32 inIndex);
static UInt32	GetNumberCommonRatesInRange(Float64 inMinimumRate, Float64 inMaximumRate);

@implementation HLDeviceWindowInfoController

-(id)	init
{
	mDevice = 0;
	mDeviceIsDead = false;
	mActualSampleRateUpdateTimer = NULL;
	return [super init];
}

-(void)	awakeFromNib
{
	CATry;
	
	mDevice = [mWindowController GetAudioDeviceID];
	
	[self InstallListeners];
	
	[self UpdateDeviceInfo: NULL];
	[self UpdateNominalSampleRateComboBox];
	[self UpdateActualSampleRateItems: NULL];
	[self UpdateIOBufferSizeComboBox];
	[self UpdateHogModeItems];
	[self UpdateMixabilityItems];
	[self UpdateClockSourceItems];
	
	CAAudioHardwareDevice theDevice(mDevice);
	if(theDevice.IsRunning())
	{
		[self StartUpdatingActualSampleRateItems: NULL];
	}
	else
	{
		[self StopUpdatingActualSampleRateItems: NULL];
	}
	
	CACatch;
}

-(void)	dealloc
{
	[self StopUpdatingActualSampleRateItems: NULL];
	[self RemoveListeners];
	[super dealloc];
}

-(void)	UpdateDeviceInfo:	(id)inSender
{
	if(!mDeviceIsDead)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	the device's ID
		NSString* theNSString = [[NSString alloc] initWithFormat: @"0x%X", theDevice.GetAudioDeviceID()];
		[theNSString autorelease];
		[mAudioDeviceIDTextField setStringValue: theNSString];
		
		//	is the device alive or dead
		[mDeviceIsAliveTextField setStringValue: (theDevice.IsAlive() ? @"Yes" : @"No")];
		
		//	the device's plug-in status
		if(theDevice.HasDevicePlugInStatus())
		{
			OSStatus thePlugInStatus = theDevice.GetDevicePlugInStatus();
			
			if(thePlugInStatus == 0)
			{
				[mDevicePlugInTextField setStringValue: @"Present"];
			}
			else
			{
				[mDevicePlugInTextField setIntValue: thePlugInStatus];
			}
		}
		else
		{
			[mDevicePlugInTextField setStringValue: @"None"];
		}
		
		//  the devices that are related
		UInt32 theNumberRelatedDevices = theDevice.GetNumberRelatedDevices();
		CAAutoArrayDelete<AudioDeviceID> theRelatedDevices(theNumberRelatedDevices);
		if(theNumberRelatedDevices > 0)
		{
			theDevice.GetRelatedDevices(theNumberRelatedDevices, theRelatedDevices);
			NSMutableString* theRelatedDevicesString = [[NSMutableString alloc] init];
			[theRelatedDevicesString autorelease];
			for(UInt32 theIndex = 0; theIndex < theNumberRelatedDevices; ++theIndex)
			{
				[theRelatedDevicesString appendFormat: @"0x%X", theRelatedDevices[theIndex]];
				if(theIndex < (theNumberRelatedDevices - 1))
				{
					[theRelatedDevicesString appendString: @", "];
				}
			}
			[mRelatedDevicesTextField setStringValue: theRelatedDevicesString];
		}
		else
		{
			[mRelatedDevicesTextField setStringValue: @""];
		}
		
		//	the device's transport Type
		char theTransportName[256];
		UInt32 theTransportType = theDevice.GetTransportType();
		CAAudioHardwareDevice::GetNameForTransportType(theTransportType, theTransportName);
		theNSString = [[NSString alloc] initWithCString: theTransportName];
		[theNSString autorelease];
		[mTransportTypeTextField setStringValue: theNSString];
		
		//	the device's running status
		theNSString = theDevice.IsRunning() ? @"Yes" : @"No";
		[mIsRunningTextField setStringValue: theNSString];
		
		//	the device's running somewhere status
		theNSString = theDevice.IsRunningSomewhere() ? @"Yes" : @"No";
		[mIsRunningSomewhereTextField setStringValue: theNSString];
		
		//	whether or not the device uses variable sized IO buffers
		if(theDevice.UsesVariableIOBufferSizes())
		{
			[mUsesVariableBufferSizesTextField setIntValue: theDevice.GetMaximumVariableIOBufferSize()];
		}
		else
		{
			[mUsesVariableBufferSizesTextField setStringValue: @"N/A"];
		}
	
		//	the device's IO Buffer Size Range
		UInt32 theMinimumBufferSize = 0;
		UInt32 theMaximumBufferSize = 0;
		theDevice.GetIOBufferSizeRange(theMinimumBufferSize, theMaximumBufferSize);
		theNSString = [[NSString alloc] initWithFormat: @"%d - %d", theMinimumBufferSize, theMaximumBufferSize];
		[theNSString autorelease];
		[mIOBufferSizeRangeTextField setStringValue: theNSString];
		
		//	the device's clock domain
		if(theDevice.HasClockDomain())
		{
			theNSString = [[NSString alloc] initWithFormat: @"0x%X", theDevice.GetClockDomain()];
			[theNSString autorelease];
			[mClockDomainTextField setStringValue: theNSString];
		}
		else
		{
			[mClockDomainTextField setStringValue: @"N/A"];
		}
		
		//	the device's name
		theNSString = (NSString*)theDevice.CopyName();
		[theNSString autorelease];
		[mDeviceNameTextField setStringValue: theNSString];
		
		//	the device's manufacturer
		theNSString = (NSString*)theDevice.CopyManufacturer();
		[theNSString autorelease];
		[mManufacturerTextField setStringValue: theNSString];
		
		//	the device's owning plug-in bundle ID
		theNSString = (NSString*)theDevice.CopyOwningPlugInBundleID();
		[theNSString autorelease];
		[mOwningPlugInTextField setStringValue: theNSString];
		
		//	the device's UID
		theNSString = (NSString*)theDevice.CopyUID();
		[theNSString autorelease];
		[mUIDTextField setStringValue: theNSString];
		
		//	the device's model UID
		if(theDevice.HasModelUID())
		{
			theNSString = (NSString*)theDevice.CopyModelUID();
			[theNSString autorelease];
			[mModelUIDTextField setStringValue: theNSString];
		}
		else
		{
			[mModelUIDTextField setStringValue: @"N/A"];
		}
		
		//	the device's config app
		try
		{
			//	Make sure the property exists
			if(theDevice.HasProperty(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyConfigurationApplication))
			{
				//	get the bundle ID of the config app
				theNSString = (NSString*)theDevice.CopyConfigurationApplicationBundleID();
				[theNSString autorelease];
				
				//	get the FSRef of the config app
				FSRef theAppFSRef;
				OSStatus theError = LSFindApplicationForInfo(kLSUnknownCreator, (CFStringRef)theNSString, NULL, &theAppFSRef, NULL);
				if(theError == 0)
				{
					//	get the display name for the app
					theError = LSCopyDisplayNameForRef(&theAppFSRef, (CFStringRef*)&theNSString);
					ThrowIfError(theError, CAException(theError), "HLDeviceWindowInfoController::-(void)UpdateDeviceInfo: couldn't get the display name for the FSRef of the config app");
					
					//	set the string in the UI and enable the button
					[mConfigAppTextField setStringValue: theNSString];
					[mLaunchConfigAppButton setEnabled: YES];
					
					//	release the display name of the app
					[theNSString release];
				}
				else
				{
					[mConfigAppTextField setStringValue: theNSString];
					[mLaunchConfigAppButton setEnabled: NO];
				}
			}
			else
			{
				[mConfigAppTextField setStringValue: @"N/A"];
				[mLaunchConfigAppButton setEnabled: NO];
			}
		}
		catch(...)
		{
			[mConfigAppTextField setStringValue: @"N/A"];
			[mLaunchConfigAppButton setEnabled: NO];
		}
		
		if(theDevice.HasSection(kAudioDeviceSectionInput))
		{
			//	the device's input latency
			[mInputLatencyTextField setIntValue: theDevice.GetLatency(kAudioDeviceSectionInput)];
			
			//	the device's input safety offset
			[mInputSafetyOffsetTextField setIntValue: theDevice.GetSafetyOffset(kAudioDeviceSectionInput)];
		}
		else
		{
			[mInputLatencyTextField setStringValue: @"N/A"];
			[mInputSafetyOffsetTextField setStringValue: @"N/A"];
		}
		
		if(theDevice.HasSection(kAudioDeviceSectionOutput))
		{
			//	the device's output latency
			[mOutputLatencyTextField setIntValue: theDevice.GetLatency(kAudioDeviceSectionOutput)];
			
			//	the device's output safety offset
			[mOutputSafetyOffsetTextField setIntValue: theDevice.GetSafetyOffset(kAudioDeviceSectionOutput)];
		}
		else
		{
			[mOutputLatencyTextField setStringValue: @"N/A"];
			[mOutputSafetyOffsetTextField setStringValue: @"N/A"];
		}
	}
	else
	{
		[mDeviceIsAliveTextField setStringValue: @"No"];
		[mLaunchConfigAppButton setEnabled: NO];
	}
}

-(void)	UpdateNominalSampleRateComboBox
{
	if(!mDeviceIsDead)
	{
		if(mDevice != 0)
		{
			CAAudioHardwareDevice theDevice(mDevice);
			Float64 theRate = 0.0;
			try
			{
				theRate = theDevice.GetNominalSampleRate();
			}
			catch(...)
			{
				theRate = 0.0;
			}
			[mNominalSampleRateComboBox setDoubleValue: theRate];
		}
		else
		{
			[mNominalSampleRateComboBox setDoubleValue: 0.0];
		}
		
		//	trim the number of display items to the size of the list
		//	or 12, whichever is smaller
		int theNumberItems = [self numberOfItemsInNominalSampleRateComboBox: mNominalSampleRateComboBox];
		[mNominalSampleRateComboBox setNumberOfVisibleItems: std::min(12, theNumberItems)];
	}
	else
	{
		[mNominalSampleRateComboBox setEnabled: NO];
	}
	[mNominalSampleRateComboBox reloadData];
}

-(IBAction)	NominalSampleRateComboBoxAction:	(id)inSender
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	get the value of the combo box
	Float64 theValue = [mNominalSampleRateComboBox doubleValue];
	
	//	set the sample rate
	theDevice.SetNominalSampleRate(theValue);
	
	CACatch;
	
	//	upate the value
	[self UpdateNominalSampleRateComboBox];
}

-(int)	numberOfItemsInNominalSampleRateComboBox:	(NSComboBox*)inComboBox
{
	int theAnswer = 0;
	
	CATry;
	
	if(mDevice != 0)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	get the number of rate ranges
		UInt32 theNumberRateRanges = theDevice.GetNumberNominalSampleRateRanges();
		
		//	allocate space to hold them
		CAAutoArrayDelete<AudioValueRange> theRateRanges(theNumberRateRanges);
		
		//	get the ranges from the device
		theDevice.GetNominalSampleRateRanges(theNumberRateRanges, theRateRanges);
		
		//	iterate through the ranges and add the minimum, maximum, and common rates in between
		for(UInt32 theRangeIndex = 0; theRangeIndex < theNumberRateRanges; ++theRangeIndex)
		{
			//	get the number of common rates in the rage
			UInt32 theNumberCommonRates = GetNumberCommonRatesInRange(theRateRanges[theRangeIndex].mMinimum, theRateRanges[theRangeIndex].mMaximum);
			
			//	count all the common rates in the range
			theAnswer += theNumberCommonRates;
			
			//	count the minimum if it isn't the first common rate
			if(!IsRateCommon(theRateRanges[theRangeIndex].mMinimum))
			{
				++theAnswer;
			}
			
			//	count the maximum if it isn't the last common rate
			if(!IsRateCommon(theRateRanges[theRangeIndex].mMaximum))
			{
				++theAnswer;
			}
		}
	}
	else
	{
		theAnswer = 1;
	}
	
	CACatch;
	
	return theAnswer;
}

-(id)	NominalSampleRateComboBox:	(NSComboBox*)inComboBox
		objectValueForItemAtIndex:	(int)inItemIndex
{
	Float64 theAnswer = 0;
	
	CATry;
	
	if(mDevice != 0)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	get the number of rate ranges
		UInt32 theNumberRateRanges = theDevice.GetNumberNominalSampleRateRanges();
		
		//	allocate space to hold them
		CAAutoArrayDelete<AudioValueRange> theRateRanges(theNumberRateRanges);
		
		//	get the ranges from the device
		theDevice.GetNominalSampleRateRanges(theNumberRateRanges, theRateRanges);
		
		//	start counting at zero
		int theIndex = 0;
		
		//	iterate through the ranges and add the minimum, maximum, and common rates in between
		for(UInt32 theRangeIndex = 0; (theAnswer == 0) && (theRangeIndex < theNumberRateRanges); ++theRangeIndex)
		{
			//	get the number of common rates in the rage
			int theNumberCommonRates = GetNumberCommonRatesInRange(theRateRanges[theRangeIndex].mMinimum, theRateRanges[theRangeIndex].mMaximum);
			
			//	get the first and last common rates in the range
			Float64 theFirstCommonRate = GetCommonSampleRateInRangeByIndex(theRateRanges[theRangeIndex].mMinimum, theRateRanges[theRangeIndex].mMaximum, 0);
			Float64 theLastCommonRate = GetCommonSampleRateInRangeByIndex(theRateRanges[theRangeIndex].mMinimum, theRateRanges[theRangeIndex].mMaximum, theNumberCommonRates - 1);
			
			//	it's the minimum, if the minimum isn't a common rate
			if(theRateRanges[theRangeIndex].mMinimum != theFirstCommonRate)
			{
				if(theIndex == inItemIndex)
				{
					theAnswer = theRateRanges[theRangeIndex].mMinimum;
				}
				else
				{
					++theIndex;
				}
			}
			
			//	check the common rates in the range
			if(theAnswer == 0)
			{
				if(inItemIndex < (theIndex + theNumberCommonRates))
				{
					//	inItemIndex is in the common rates between the range
					theAnswer = GetCommonSampleRateInRangeByIndex(theRateRanges[theRangeIndex].mMinimum, theRateRanges[theRangeIndex].mMaximum, inItemIndex - theIndex);
				}
				else if((inItemIndex == (theIndex + theNumberCommonRates)) && (theRateRanges[theRangeIndex].mMaximum != theLastCommonRate))
				{
					//	it's the maximum, since the maximum isn't a common rate
					theAnswer = theRateRanges[theRangeIndex].mMaximum;
				}
				
				//	increment by the number of common rates
				theIndex += theNumberCommonRates;
				
				//	also increment if the maximum isn't a common rate
				if(theRateRanges[theRangeIndex].mMaximum != theLastCommonRate)
				{
					++theIndex;
				}
			}
		}
	}
	else
	{
		theAnswer = 0;
	}
		
	CACatch;
	
	return [NSNumber numberWithDouble: theAnswer];
}

-(void)	UpdateActualSampleRateItems:	(NSTimer*)inTimer
{
	CATry;
	
	if(!mDeviceIsDead)
	{
		if(mDevice != 0)
		{
			CAAudioHardwareDevice theDevice(mDevice);
			[mActualSampleRateTextField setDoubleValue: theDevice.GetActualSampleRate()];
		}
		else
		{
			[mActualSampleRateTextField setStringValue: @""];
		}
	}
	
	CACatch;
}

-(void)		StartUpdatingActualSampleRateItems:	(id)inSender
{
	if(mActualSampleRateUpdateTimer == NULL)
	{
		mActualSampleRateUpdateTimer = [[NSTimer scheduledTimerWithTimeInterval: 0.5 target: self selector: @selector(UpdateActualSampleRateItems:) userInfo: NULL repeats: YES] retain];
	}
}

-(void)		StopUpdatingActualSampleRateItems:	(id)inSender
{
	if(mActualSampleRateUpdateTimer != NULL)
	{
		[mActualSampleRateUpdateTimer invalidate];
		[mActualSampleRateUpdateTimer release];
		mActualSampleRateUpdateTimer = NULL;
		[self UpdateActualSampleRateItems: NULL];
	}
}

-(void)	UpdateIOBufferSizeComboBox
{
	if(!mDeviceIsDead)
	{
		if(mDevice != 0)
		{
			CAAudioHardwareDevice theDevice(mDevice);
			UInt32 theIOBufferSize = theDevice.GetIOBufferSize();
			[mIOBufferSizeComboBox setIntValue: theIOBufferSize];
		}
		else
		{
			[mIOBufferSizeComboBox setIntValue: 0];
		}
		
		//	trim the number of display items to the size of the list
		//	or 12, whichever is smaller
		int theNumberItems = [self numberOfItemsInIOBufferSizeComboBox: mIOBufferSizeComboBox];
		[mIOBufferSizeComboBox setNumberOfVisibleItems: std::min(12, theNumberItems)];
	}
	else
	{
		[mIOBufferSizeComboBox setEnabled: NO];
	}
	[mIOBufferSizeComboBox reloadData];
}

-(IBAction)	IOBufferSizeComboBoxAction:	(id)inSender
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	get the value of the combo box
	int theValue = [mIOBufferSizeComboBox intValue];
	
	//	set the buffer size
	theDevice.SetIOBufferSize(theValue);
	
	CACatch;
	
	//	upate the value
	[self UpdateIOBufferSizeComboBox];
}

-(int)	numberOfItemsInIOBufferSizeComboBox:	(NSComboBox*)inComboBox
{
	int theAnswer = 0;
	
	CATry;
	
	if(mDevice != 0)
	{
		//	populate the combo box with the minimum, the maximum, and all
		//	the powers of two in between.
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	get the range
		UInt32 theMinimumBufferSize = 0;
		UInt32 theMaximumBufferSize = 0;
		theDevice.GetIOBufferSizeRange(theMinimumBufferSize, theMaximumBufferSize);
		
		//	get the first power of two greater than or equal to the minimum
		Float64 theCurrentPowerOf2 = log2(theMinimumBufferSize);
		theCurrentPowerOf2 = ceil(theCurrentPowerOf2);
		theCurrentPowerOf2 = pow(2.0, theCurrentPowerOf2);
		
		//	count the minimum if it isn't a power of two
		if(theMinimumBufferSize != theCurrentPowerOf2)
		{
			++theAnswer;
		}
		
		//	count the powers of two up to the maximum
		while(theCurrentPowerOf2 < theMaximumBufferSize)
		{
			++theAnswer;
			theCurrentPowerOf2 *= 2.0;
		}
		
		//	count the maximum
		++theAnswer;
	}
	else
	{
		theAnswer = 1;
	}
	
	CACatch;
	
	return theAnswer;
}

-(id)	IOBufferSizeComboBox:		(NSComboBox*)inComboBox
		objectValueForItemAtIndex:	(int)inItemIndex
{
	int theAnswer = 0;
	
	CATry;
	
	if(mDevice != 0)
	{
		//	populate the combo box with the minimum, the maximum, and all
		//	the powers of two in between.
		int theCurrentIndex = 0;
		
		CAAudioHardwareDevice theDevice(mDevice);
		UInt32 theMinimumBufferSize = 0;
		UInt32 theMaximumBufferSize = 0;
		theDevice.GetIOBufferSizeRange(theMinimumBufferSize, theMaximumBufferSize);
		
		//	get the first power of two greater than or equal to the minimum
		Float64 theCurrentPowerOf2 = log2(theMinimumBufferSize);
		theCurrentPowerOf2 = ceil(theCurrentPowerOf2);
		theCurrentPowerOf2 = pow(2.0, theCurrentPowerOf2);
		
		//	the minimum is first if it isn't a power of 2
		if((inItemIndex == 0) && (theMinimumBufferSize != theCurrentPowerOf2))
		{
			theAnswer = theMinimumBufferSize;
		}
		
		if(theAnswer == 0)
		{
			//	have to skip over the first one, if it wasn't a power of 2
			if(theMinimumBufferSize != theCurrentPowerOf2)
			{
				++theCurrentIndex;
			}
			
			//	count the powers of two up to the maximum
			while((theCurrentPowerOf2 < theMaximumBufferSize) && (theAnswer == 0))
			{
				if(theCurrentIndex == inItemIndex)
				{
					theAnswer = (int)theCurrentPowerOf2;
				}
				++theCurrentIndex;
				theCurrentPowerOf2 *= 2.0;
			}
		}
		
		if((theAnswer == 0) && (theCurrentIndex == inItemIndex))
		{
			theAnswer = theMaximumBufferSize;
		}
	}
	else
	{
		theAnswer = 0;
	}
		
	CACatch;
	
	return [NSNumber numberWithUnsignedLong: theAnswer];
}

-(IBAction)	LaunchConfigAppButtonAction:	(id)inSender
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	get the bundle ID of the config app
	NSString* theNSString = (NSString*)theDevice.CopyConfigurationApplicationBundleID();
	[theNSString autorelease];
	
	//	get the FSRef of the config app
	FSRef theAppFSRef;
	OSStatus theError = LSFindApplicationForInfo(kLSUnknownCreator, (CFStringRef)theNSString, NULL, &theAppFSRef, NULL);
	ThrowIfError(theError, CAException(theError), "HLDeviceWindowInfoController::-(void)LaunchConfigAppButtonAction: couldn't get the FSRef for the bundle ID");
	
	//	open it
	theError = LSOpenFSRef(&theAppFSRef, NULL);
	ThrowIfError(theError, CAException(theError), "HLDeviceWindowInfoController::-(void)LaunchConfigAppButtonAction: couldn't open the FSRef for the bundle ID");
	
	CACatch;
}

-(void)	UpdateHogModeItems
{
	CATry;
	
	if(!mDeviceIsDead)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		pid_t theHogModeOwner = theDevice.GetHogModeOwner();
		
		//	update the text field
		NSString* theNSString = [[NSString alloc] initWithFormat: @"%d", theHogModeOwner];
		[theNSString autorelease];
		[mHogModeOwnerTextField setStringValue: theNSString];
		
		//	update the text on the "take" button
		if(getpid() == theHogModeOwner)
		{
			[mTakeHogModeButton	setTitle: @"Release"];
		}
		else
		{
			[mTakeHogModeButton	setTitle: @"Take"];
		}
		
		if(theDevice.PropertyIsSettable(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyHogMode))
		{
			[mTakeHogModeButton setEnabled: YES];
		}
		else
		{
			[mTakeHogModeButton setEnabled: NO];
		}
	}
	else
	{
		[mTakeHogModeButton setEnabled: NO];
	}

	CACatch;
}

-(IBAction)	TakeHogModeButtonAction:	(id)inSender
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	get the current hog mode owner
	pid_t theHogModeOwner = theDevice.GetHogModeOwner();
	
	//	depending on who owns hog mode, take or release it
	if(theHogModeOwner != getpid())
	{
		theDevice.TakeHogMode();
	}
	else
	{
		theDevice.ReleaseHogMode();
	}

	CACatch;
}

-(void)	UpdateMixabilityItems
{
	CATry;
	
	if(!mDeviceIsDead)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		[mMixingRadioButtonMatrix setEnabled: theDevice.SupportsChangingMixability()];
		
		//	figure out if mixing is on or off
		bool isMixing = theDevice.IsMixable();
		
		//	turn on the appropriate radio button
		if(isMixing)
		{
			[mMixingRadioButtonMatrix setState: 1 atRow: 0 column: 0];
		}
		else
		{
			[mMixingRadioButtonMatrix setState: 1 atRow: 0 column: 1];
		}
	}
	else
	{
		[mMixingRadioButtonMatrix setEnabled: NO];
	}
	
	CACatch;
}

-(IBAction)	MixingRadioButtonMatrixAction:	(id)inSender
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	figure out if mixing is on or off
	bool isMixing = theDevice.IsMixable();
	
	//	get the newly selected cell
	NSCell* theCell = [mMixingRadioButtonMatrix selectedCell];
	
	//	get the column that the cell is in
	int theRow = 0;
	int theColumn = 0;
	[mMixingRadioButtonMatrix getRow: &theRow column: &theColumn ofCell: theCell];
	
	switch(theColumn)
	{
		case 0:
			//	mixing is now on
			if(!isMixing)
			{
				theDevice.SetIsMixable(true);
			}
			break;
		
		case 1:
			//	mixing is now off
			if(isMixing)
			{
				theDevice.SetIsMixable(false);
			}
			break;
		
		default:
			//	it's in an indeterminate state, so fix it
			[self UpdateMixabilityItems];
			break;
	};
	
	CACatch;
	
	[self UpdateMixabilityItems];
}

-(void)		UpdateClockSourceItems
{
	CATry;
	
	if(!mDeviceIsDead)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		UInt32 theNumberSources = theDevice.GetNumberAvailableClockSources();
		if(theNumberSources > 0)
		{
			//	there are clock sources, so remove all the itmes
			[mClockSourcePopUp removeAllItems];
			
			//	get the IDs of all the sources
			CAAutoArrayDelete<UInt32> theSourceList(theNumberSources);
			theDevice.GetAvailableClockSources(theNumberSources, theSourceList);
			
			//	add an item for each source, using the tag to carry the source ID
			for(UInt32 theSourceIndex = 0; theSourceIndex < theNumberSources; ++theSourceIndex)
			{
				//	get the ID of the source
				UInt32 theSourceID = theSourceList[theSourceIndex];
				
				//	get the name of the source
				NSString* theSourceName = (NSString*)theDevice.CopyClockSourceNameForID(theSourceID);
				[theSourceName autorelease];
				
				//	add it to the menu
				[mClockSourcePopUp addItemWithTitle: theSourceName];
				
				//	set the tag of the item to be the source ID
				[[mClockSourcePopUp lastItem] setTag: theSourceID];
			}
			
			//	set the selected item to the current clock source
			UInt32 theCurrentSourceID = theDevice.GetCurrentClockSourceID();
			UInt32 theCurrentSourceIndex = [mClockSourcePopUp indexOfItemWithTag: theCurrentSourceID];
			[mClockSourcePopUp selectItemAtIndex: theCurrentSourceIndex];
			
			//	and enable the menu
			[mClockSourcePopUp setEnabled: YES];
		}
		else
		{
			//	no clock sources, remove all the times
			[mClockSourcePopUp removeAllItems];
			
			//	disable the whole thing
			[mClockSourcePopUp setEnabled: NO];
		}
	}
	else
	{
		[mClockSourcePopUp setEnabled: NO];
	}
	
	CACatch;
}

-(IBAction)	ClockSourcePopUpAction:	(id)inSender
{
	CATry;
	
	//	get the selected item
	NSMenuItem* theSelectedItem = [mClockSourcePopUp selectedItem];
	if(theSelectedItem != NULL)
	{
		//	the selected item's tag is the source ID to switch to
		UInt32 theSourceID = [theSelectedItem tag];
		
		//	tell the device about the new selection
		CAAudioHardwareDevice theDevice(mDevice);
		theDevice.SetCurrentClockSourceByID(theSourceID);
	}
	
	CACatch;
	
	//	upate the value
	[self UpdateClockSourceItems];
}

-(void)	WindowWillClose
{
	[self StopUpdatingActualSampleRateItems: NULL];
}

-(int)	numberOfItemsInComboBox:	(NSComboBox*)inComboBox
{
	int theAnswer = 0;
	
	CATry;

	if(inComboBox == mNominalSampleRateComboBox)
	{
		theAnswer = [self numberOfItemsInNominalSampleRateComboBox: inComboBox];
	}
	else if(inComboBox == mIOBufferSizeComboBox)
	{
		theAnswer = [self numberOfItemsInIOBufferSizeComboBox: inComboBox];
	}
	
	CACatch;
	
	return theAnswer;
}

-(id)	comboBox: 					(NSComboBox*)inComboBox
		objectValueForItemAtIndex:	(int)inItemIndex
{
	id theAnswer = NULL;
	
	CATry;

	if(inComboBox == mIOBufferSizeComboBox)
	{
		theAnswer = [self IOBufferSizeComboBox: inComboBox objectValueForItemAtIndex: inItemIndex];
	}
	else if(inComboBox == mNominalSampleRateComboBox)
	{
		theAnswer = [self NominalSampleRateComboBox: inComboBox objectValueForItemAtIndex: inItemIndex];
	}
	
	CACatch;
	
	return theAnswer;
}

-(void)	InstallListeners
{
	CATry;
	
	if(!mDeviceIsDead && (mDevice != 0))
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	add a device listener for all notifications
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardSection, kAudioPropertyWildcardPropertyID, (AudioDevicePropertyListenerProc)HLDeviceWindowInfoControllerAudioDevicePropertyListenerProc, self);
		
		//	add a Listener to all the input streams
		UInt32 theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionInput);
		UInt32 theIndex = 0;
		for(; theIndex < theNumberStreams; ++theIndex)
		{
			CAAudioHardwareStream theStream(theDevice.GetStreamByIndex(kAudioDeviceSectionInput, theIndex));
			theStream.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardPropertyID, (AudioStreamPropertyListenerProc)HLDeviceWindowInfoControllerAudioStreamPropertyListenerProc, self);
		}
		theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionOutput);
		for(theIndex = 0; theIndex < theNumberStreams; ++theIndex)
		{
			CAAudioHardwareStream theStream(theDevice.GetStreamByIndex(kAudioDeviceSectionOutput, theIndex));
			theStream.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardPropertyID, (AudioStreamPropertyListenerProc)HLDeviceWindowInfoControllerAudioStreamPropertyListenerProc, self);
		}
	}
	
	CACatch;
}

-(void)	RemoveListeners
{
	CATry;
	
	if(!mDeviceIsDead && (mDevice != 0))
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	remove the device listener for all notifications
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardSection, kAudioPropertyWildcardPropertyID, (AudioDevicePropertyListenerProc)HLDeviceWindowInfoControllerAudioDevicePropertyListenerProc);
		
		//	remove the Listener from all the input streams
		UInt32 theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionInput);
		UInt32 theIndex = 0;
		for(; theIndex < theNumberStreams; ++theIndex)
		{
			CAAudioHardwareStream theStream(theDevice.GetStreamByIndex(kAudioDeviceSectionInput, theIndex));
			theStream.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardPropertyID, (AudioStreamPropertyListenerProc)HLDeviceWindowInfoControllerAudioStreamPropertyListenerProc);
		}
		theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionOutput);
		for(theIndex = 0; theIndex < theNumberStreams; ++theIndex)
		{
			CAAudioHardwareStream theStream(theDevice.GetStreamByIndex(kAudioDeviceSectionOutput, theIndex));
			theStream.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardPropertyID, (AudioStreamPropertyListenerProc)HLDeviceWindowInfoControllerAudioStreamPropertyListenerProc);
		}
	}
	
	CACatch;
}

@end

OSStatus	HLDeviceWindowInfoControllerAudioDevicePropertyListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean /*inIsInput*/, AudioDevicePropertyID inPropertyID, HLDeviceWindowInfoController* inDeviceWindowInfoController)
{
	NS_DURING
	CATry;

	CAAudioHardwareDevice theDevice(inDevice);
	switch(inPropertyID)
	{
		case kAudioDevicePropertyDeviceIsAlive:
			inDeviceWindowInfoController->mDeviceIsDead = true;
			[inDeviceWindowInfoController UpdateDeviceInfo: NULL];
			[inDeviceWindowInfoController UpdateNominalSampleRateComboBox];
			[inDeviceWindowInfoController UpdateActualSampleRateItems: NULL];
			[inDeviceWindowInfoController UpdateIOBufferSizeComboBox];
			[inDeviceWindowInfoController UpdateHogModeItems];
			[inDeviceWindowInfoController UpdateMixabilityItems];
			[inDeviceWindowInfoController UpdateClockSourceItems];
			break;
			
		case kAudioDevicePropertyHogMode:
			[inDeviceWindowInfoController UpdateHogModeItems];
			[inDeviceWindowInfoController UpdateMixabilityItems];
			break;
		
		case kAudioDevicePropertySupportsMixing:
			[inDeviceWindowInfoController UpdateMixabilityItems];
			break;
			
		case kAudioDevicePropertyDeviceNameCFString:
		case kAudioDevicePropertyDeviceManufacturerCFString:
		case kAudioDevicePropertyLatency:
		case kAudioDevicePropertyUsesVariableBufferFrameSizes:
		case kAudioDevicePropertyStreams:
		case kAudioDevicePropertySafetyOffset:
		case kAudioDevicePropertyStreamConfiguration:
			[inDeviceWindowInfoController UpdateDeviceInfo: NULL];
			break;
		
		case kAudioDevicePropertyNominalSampleRate:
			if(inChannel == 0)
			{
				[inDeviceWindowInfoController UpdateNominalSampleRateComboBox];
				[inDeviceWindowInfoController UpdateActualSampleRateItems: NULL];
			}
			break;
		
		case kAudioDevicePropertyAvailableNominalSampleRates:
			if(inChannel == 0)
			{
				[inDeviceWindowInfoController->mNominalSampleRateComboBox reloadData];
				[inDeviceWindowInfoController UpdateNominalSampleRateComboBox];
				[inDeviceWindowInfoController UpdateActualSampleRateItems: NULL];
			}
			break;
		
		case kAudioDevicePropertyBufferFrameSize:
			[inDeviceWindowInfoController UpdateIOBufferSizeComboBox];
			break;
			
		case kAudioDevicePropertyBufferFrameSizeRange:
			[inDeviceWindowInfoController UpdateDeviceInfo: NULL];
			break;
		
		case kAudioDevicePropertyClockSource:
		case kAudioDevicePropertyClockSources:
			[inDeviceWindowInfoController UpdateClockSourceItems];
			break;
		
		case kAudioDevicePropertyDeviceIsRunningSomewhere:
			[inDeviceWindowInfoController UpdateDeviceInfo: NULL];
			break;
		
		case kAudioDevicePropertyDeviceHasChanged:
			[inDeviceWindowInfoController UpdateDeviceInfo: NULL];
			[inDeviceWindowInfoController UpdateNominalSampleRateComboBox];
			[inDeviceWindowInfoController UpdateActualSampleRateItems: NULL];
			[inDeviceWindowInfoController UpdateIOBufferSizeComboBox];
			[inDeviceWindowInfoController UpdateHogModeItems];
			[inDeviceWindowInfoController UpdateMixabilityItems];
			[inDeviceWindowInfoController UpdateClockSourceItems];
			break;
			
		//	the following selectors are sent from the IO thread
		case kAudioDevicePropertyDeviceIsRunning:
			[inDeviceWindowInfoController performSelectorOnMainThread: @selector(UpdateDeviceInfo:) withObject: NULL waitUntilDone: NO];
			if(theDevice.IsRunning())
			{
				[inDeviceWindowInfoController performSelectorOnMainThread: @selector(StartUpdatingActualSampleRateItems:) withObject: NULL waitUntilDone: NO];
			}
			else
			{
				[inDeviceWindowInfoController performSelectorOnMainThread: @selector(StopUpdatingActualSampleRateItems:) withObject: NULL waitUntilDone: NO];
			}
			break;
		
	};
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	return 0;
}

OSStatus	HLDeviceWindowInfoControllerAudioStreamPropertyListenerProc(AudioStreamID /*inStream*/, UInt32 /*inChannel*/, AudioDevicePropertyID /*inPropertyID*/, HLDeviceWindowInfoController* /*inDeviceWindowInfoController*/)
{
	NS_DURING
	CATry;
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	return 0;
}

static UInt32	sNumberCommonSampleRates = 15;
static Float64	sCommonSampleRates[] = {	  8000.0,  11025.0,  12000.0,
											 16000.0,  22050.0,  24000.0,
											 32000.0,  44100.0,  48000.0,
											 64000.0,  88200.0,  96000.0,
											128000.0, 176400.0, 192000.0 };

static bool		IsRateCommon(Float64 inRate)
{
	bool theAnswer = false;
	for(UInt32 theIndex = 0; !theAnswer && (theIndex < sNumberCommonSampleRates); ++theIndex)
	{
		theAnswer = inRate == sCommonSampleRates[theIndex];
	}
	return theAnswer;
}

static UInt32	GetNumberCommonRatesInRange(Float64 inMinimumRate, Float64 inMaximumRate)
{
	//	find the index of the first common rate greater than or equal to the minimum
	UInt32 theFirstIndex = 0;
	while((theFirstIndex < sNumberCommonSampleRates) && (sCommonSampleRates[theFirstIndex] < inMinimumRate))
	{
		++theFirstIndex;
	}
	
	//	find the index of the first common rate greater than or equal to the maximum
	UInt32 theLastIndex = theFirstIndex;
	while((theFirstIndex < sNumberCommonSampleRates) && (sCommonSampleRates[theLastIndex] < inMaximumRate))
	{
		++theLastIndex;
	}
	
	//	the number in the range is the difference
	UInt32 theAnswer = theLastIndex - theFirstIndex;
	if(IsRateCommon(inMinimumRate) || IsRateCommon(inMaximumRate))
	{
		++theAnswer;
	}
	return theAnswer;
}

static Float64	GetCommonSampleRateInRangeByIndex(Float64 inMinimumRate, Float64 inMaximumRate, UInt32 inIndex)
{
	Float64 theAnswer = 0.0;
	
	//	find the index of the first common rate greater than or equal to the minimum
	UInt32 theFirstIndex = 0;
	while((theFirstIndex < sNumberCommonSampleRates) && (sCommonSampleRates[theFirstIndex] < inMinimumRate))
	{
		++theFirstIndex;
	}
	
	//	find the index of the first common rate greater than or equal to the maximum
	UInt32 theLastIndex = theFirstIndex;
	while((theFirstIndex < sNumberCommonSampleRates) && (sCommonSampleRates[theLastIndex] < inMaximumRate))
	{
		++theLastIndex;
	}
	
	//	the number in the range is the difference
	UInt32 theNumberInRange = theLastIndex - theFirstIndex;
	if(IsRateCommon(inMinimumRate) || IsRateCommon(inMaximumRate))
	{
		++theNumberInRange;
	}
	
	//	get the value from the array if it's in range
	if(inIndex < theNumberInRange)
	{
		theAnswer = sCommonSampleRates[inIndex + theFirstIndex];
	}
	
	return theAnswer;
}
