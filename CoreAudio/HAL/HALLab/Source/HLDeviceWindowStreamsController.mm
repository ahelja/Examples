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
	HLDeviceWindowStreamsController.mm

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "HLDeviceWindowStreamsController.h"

//	Local Includes
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareStream.h"
#include "HLDeviceWindowController.h"

//	PublicUtility Includes
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAStreamBasicDescription.h"

//=============================================================================
//	HLDeviceWindowStreamsController
//=============================================================================

@implementation HLDeviceWindowStreamsController

-(id)	init
{
	mDevice = 0;
	mDeviceIsDead = false;
	mCurrentInputStream = 0;
	mNumberInputIOProcFormats = 0;
	mInputIOProcFormats = NULL;
	mNumberInputPhysicalFormats = 0;
	mInputPhysicalFormats = NULL;
	mCurrentOutputStream = 0;
	mNumberOutputIOProcFormats = 0;
	mOutputIOProcFormats = NULL;
	mNumberOutputPhysicalFormats = 0;
	mOutputPhysicalFormats = NULL;
	return [super init];
}

-(void)		awakeFromNib
{
	//	figure out what device we are
	mDevice = [mWindowController GetAudioDeviceID];
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	add listeners to the device
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardSection, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)HLDeviceWindowStreamsControllerAudioDevicePropertyListenerProc, self);
	theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardSection, kAudioDevicePropertyStreams, (AudioDevicePropertyListenerProc)HLDeviceWindowStreamsControllerAudioDevicePropertyListenerProc, self);
	
	//	figure out what streams should be currently selected
	UInt32 theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionInput);
	if(theNumberStreams > 0)
	{
		[self SetCurrentStream: theDevice.GetStreamByIndex(kAudioDeviceSectionInput, 0) Section: kAudioDeviceSectionInput InstallListeners: true];
	}
	
	theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionOutput);
	if(theNumberStreams > 0)
	{
		[self SetCurrentStream: theDevice.GetStreamByIndex(kAudioDeviceSectionOutput, 0) Section: kAudioDeviceSectionOutput InstallListeners: true];
	}
	
	//	update the UI
	[self UpdateCurrentStreamItems: kAudioDeviceSectionInput];
	[self UpdateStreamInfoItems: kAudioDeviceSectionInput];
	[self UpdateDataSourceItems: kAudioDeviceSectionInput];
	[self UpdateIOProcFormatItems: kAudioDeviceSectionInput];
	[self UpdatePhysicalFormatItems: kAudioDeviceSectionInput];
	
	[self UpdateCurrentStreamItems: kAudioDeviceSectionOutput];
	[self UpdateStreamInfoItems: kAudioDeviceSectionOutput];
	[self UpdateDataSourceItems: kAudioDeviceSectionOutput];
	[self UpdateIOProcFormatItems: kAudioDeviceSectionOutput];
	[self UpdatePhysicalFormatItems: kAudioDeviceSectionOutput];
}

-(void)	dealloc
{
	if(!mDeviceIsDead)
	{
		//	teardown the streams
		[self SetCurrentStream: 0 Section: kAudioDeviceSectionInput InstallListeners: true];
		[self SetCurrentStream: 0 Section: kAudioDeviceSectionOutput InstallListeners: true];
		
		//	remove listeners from the device
		CAAudioHardwareDevice theDevice(mDevice);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardSection, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)HLDeviceWindowStreamsControllerAudioDevicePropertyListenerProc);
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardSection, kAudioDevicePropertyStreams, (AudioDevicePropertyListenerProc)HLDeviceWindowStreamsControllerAudioDevicePropertyListenerProc);
	}
	
	[super dealloc];
}

-(void)		UpdateCurrentStreamItems:	(CAAudioHardwareDeviceSectionID)inSection
{
	CATry;
	
	CAAudioHardwareDevice theDevice(mDevice);
	
	//	figure out which controls we need to touch
	NSPopUpButton* theCurrentStreamPopUp = ((inSection == kAudioDeviceSectionInput) ? mInputCurrentStreamPopUp : mOutputCurrentStreamPopUp);
	
	if(!mDeviceIsDead)
	{
		UInt32 theNumberStreams = theDevice.GetNumberStreams(inSection);
		if(theNumberStreams > 0)
		{
			//	there are streams, so remove all the itmes
			[theCurrentStreamPopUp removeAllItems];
			
			//	get the IDs of all the streams
			CAAutoArrayDelete<UInt32> theStreamList(theNumberStreams);
			theDevice.GetStreams(inSection, theNumberStreams, theStreamList);
			
			//	add an item for each stream, using the tag to carry the AudioStreamID
			for(UInt32 theStreamIndex = 0; theStreamIndex < theNumberStreams; ++theStreamIndex)
			{
				//	get the AudioStreamID of the stream
				AudioStreamID theAudioStreamID = theStreamList[theStreamIndex];
				CAAudioHardwareStream theStream(theAudioStreamID);
				
				//	get the name of the stream
				NSString* theStreamName = NULL;
				CATry;
				theStreamName = (NSString*)theStream.CopyName();
				CACatch;
				if(theStreamName == NULL)
				{
					//	the stream doesn't have a name, so make one up
					theStreamName = [[NSString alloc] initWithFormat: @"%@ Stream 0x%X", ((inSection == kAudioDeviceSectionInput) ? @"Input" : @"Output"), theAudioStreamID];
				}
				[theStreamName autorelease];
				
				//	add it to the menu
				[theCurrentStreamPopUp addItemWithTitle: theStreamName];
				
				//	set the tag of the item to be the source ID
				[[theCurrentStreamPopUp lastItem] setTag: theAudioStreamID];
			}
			
			//	set the selected item to the current data source
			AudioStreamID theCurrentStreamID = ((inSection == kAudioDeviceSectionInput) ? mCurrentInputStream : mCurrentOutputStream);
			UInt32 theCurrentSourceIndex = [theCurrentStreamPopUp indexOfItemWithTag: theCurrentStreamID];
			[theCurrentStreamPopUp selectItemAtIndex: theCurrentSourceIndex];
			
			//	and enable the menu
			[theCurrentStreamPopUp setEnabled: YES];
		}
		else
		{
			//	no streams, remove all the times
			[theCurrentStreamPopUp removeAllItems];
			
			//	disable the whole thing
			[theCurrentStreamPopUp setEnabled: NO];
		}
	}
	else
	{
		[theCurrentStreamPopUp setEnabled: NO];
	}
	
	CACatch;
}

-(IBAction)	CurrentStreamPopUpAction:	(id)inSender
{
	CAAudioHardwareDeviceSectionID theSection = kAudioDeviceSectionInput;
	NSPopUpButton* theCurrentStreamPopUp = NULL;

	CATry;
	
	if(inSender == mInputCurrentStreamPopUp)
	{
		theSection = kAudioDeviceSectionInput;
		theCurrentStreamPopUp = mInputCurrentStreamPopUp;
	}
	else
	{
		theSection = kAudioDeviceSectionOutput;
		theCurrentStreamPopUp = mOutputCurrentStreamPopUp;
	}
	
	//	get the selected item
	NSMenuItem* theSelectedItem = [theCurrentStreamPopUp selectedItem];
	if(theSelectedItem != NULL)
	{
		//	the selected item's tag is the AudioStreamID to switch to
		AudioStreamID theStreamID = [theSelectedItem tag];
		
		//	switch over to the new stream
		[self SetCurrentStream: theStreamID Section:theSection InstallListeners: true];
	}
	
	CACatch;
	
	//	upate the value
	[self UpdateCurrentStreamItems: theSection];
	[self UpdateStreamInfoItems: theSection];
	[self UpdateDataSourceItems: theSection];
	[self UpdateIOProcFormatItems: theSection];
	[self UpdatePhysicalFormatItems: theSection];
}

-(void)		UpdateStreamInfoItems:		(CAAudioHardwareDeviceSectionID)inSection
{
	CATry;
	
	AudioStreamID theCurrentStreamID = ((inSection == kAudioDeviceSectionInput) ? mCurrentInputStream : mCurrentOutputStream);
	NSTextField* theAudioStreamIDTextField = ((inSection == kAudioDeviceSectionInput) ? mInputAudioStreamIDTextField : mOutputAudioStreamIDTextField);
	NSTextField* theStartingChannelTextField = ((inSection == kAudioDeviceSectionInput) ? mInputStartingChannelTextField : mOutputStartingChannelTextField);
	NSTextField* theTerminalTypeTextField = ((inSection == kAudioDeviceSectionInput) ? mInputTerminalTypeTextField : mOutputTerminalTypeTextField);
	NSTextField* theConnectedTextField = ((inSection == kAudioDeviceSectionInput) ? mInputConnectedTextField : mOutputConnectedTextField);
	
	if(!mDeviceIsDead)
	{
		if(theCurrentStreamID != 0)
		{
			CAAudioHardwareStream theStream(theCurrentStreamID);
			
			//	the AudioStreamID
			NSString* theNSString = [[NSString alloc] initWithFormat: @"0x%X", theStream.GetAudioStreamID()];
			[theNSString autorelease];
			[theAudioStreamIDTextField setStringValue: theNSString];
			
			//	the starting channel
			[theStartingChannelTextField setIntValue: theStream.GetStartingDeviceChannel()];
			
			//	the terminal type
			char theTerminalName[256];
			UInt32 theTerminalType = theStream.GetTerminalType();
			CAAudioHardwareStream::GetNameForTerminalType(theTerminalType, theTerminalName);
			theNSString = [[NSString alloc] initWithCString: theTerminalName];
			[theNSString autorelease];
			[theTerminalTypeTextField setStringValue: theNSString];
			
			//	whether or not the stream is connected
			if(theStream.HasIsConnectedStatus())
			{
				[theConnectedTextField setStringValue: (theStream.GetIsConnectedStatus() ? @"Yes" : @"No")];
			}
			else
			{
				[theConnectedTextField setStringValue: @"N/A"];
			}
		}
		else
		{
			[theAudioStreamIDTextField setStringValue: @""];
			[theStartingChannelTextField setStringValue: @""];
			[theTerminalTypeTextField setStringValue: @""];
			[theConnectedTextField setStringValue: @""];
		}
	}
	
	CACatch;
}

-(void)	UpdateDataSourceItems:	(CAAudioHardwareDeviceSectionID)inSection
{
	CATry;
	
	AudioStreamID theCurrentStreamID = ((inSection == kAudioDeviceSectionInput) ? mCurrentInputStream : mCurrentOutputStream);
	NSPopUpButton* theDataSourcePopUp = ((inSection == kAudioDeviceSectionInput) ? mInputDataSourcePopUp : mOutputDataSourcePopUp);

	if(!mDeviceIsDead)
	{
		if(theCurrentStreamID != 0)
		{
			CAAudioHardwareStream theStream(theCurrentStreamID);
			
			if(theStream.HasDataSourceControl())
			{
				UInt32 theNumberSources = theStream.GetNumberAvailableDataSources();
				
				//	there are data sources, so remove all the itmes
				[theDataSourcePopUp removeAllItems];
				
				//	get the IDs of all the sources
				CAAutoArrayDelete<UInt32> theSourceList(theNumberSources);
				theStream.GetAvailableDataSources(theNumberSources, theSourceList);
				
				//	add an item for each source, using the tag to carry the source ID
				for(UInt32 theSourceIndex = 0; theSourceIndex < theNumberSources; ++theSourceIndex)
				{
					//	get the ID of the source
					UInt32 theSourceID = theSourceList[theSourceIndex];
					
					//	get the name of the source
					NSString* theSourceName = (NSString*)theStream.CopyDataSourceNameForID(theSourceID);
					[theSourceName autorelease];
					
					//	add it to the menu
					[theDataSourcePopUp addItemWithTitle: theSourceName];
					
					//	set the tag of the item to be the source ID
					[[theDataSourcePopUp lastItem] setTag: theSourceID];
				}
				
				//	set the selected item to the current data source
				UInt32 theCurrentSourceID = theStream.GetCurrentDataSourceID();
				UInt32 theCurrentSourceIndex = [theDataSourcePopUp indexOfItemWithTag: theCurrentSourceID];
				[theDataSourcePopUp selectItemAtIndex: theCurrentSourceIndex];
				
				//	and enable the menu
				[theDataSourcePopUp setEnabled: theStream.DataSourceControlIsSettable() ? YES : NO];
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
	AudioStreamID theCurrentStreamID = 0;
	CAAudioHardwareDeviceSectionID theSection = kAudioDeviceSectionInput;
	NSPopUpButton* theDataSourcePopUp = NULL;

	CATry;
	
	if(inSender == mInputDataSourcePopUp)
	{
		theCurrentStreamID = mCurrentInputStream;
		theSection = kAudioDeviceSectionInput;
		theDataSourcePopUp = mInputDataSourcePopUp;
	}
	else
	{
		theCurrentStreamID = mCurrentOutputStream;
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
		CAAudioHardwareStream theStream(theCurrentStreamID);
		theStream.SetCurrentDataSourceByID(theSourceID);
	}
	
	CACatch;
	
	//	upate the value
	[self UpdateDataSourceItems: theSection];
}

-(void)	UpdateIOProcFormatItems:	(CAAudioHardwareDeviceSectionID)inSection
{
	char theFormatNameCString[256];
	NSString* theFormatName;
	
	CATry;
	
	AudioStreamID theCurrentStreamID = ((inSection == kAudioDeviceSectionInput) ? mCurrentInputStream : mCurrentOutputStream);
	NSPopUpButton* theFormatPopUp = ((inSection == kAudioDeviceSectionInput) ? mInputIOProcFormatPopUp : mOutputIOProcFormatPopUp);
	UInt32 theNumberFormats = ((inSection == kAudioDeviceSectionInput) ? mNumberInputIOProcFormats : mNumberOutputIOProcFormats);
	AudioStreamBasicDescription* theFormats = ((inSection == kAudioDeviceSectionInput) ? mInputIOProcFormats : mOutputIOProcFormats);
	
	if(!mDeviceIsDead)
	{
		if(theCurrentStreamID != 0)
		{
			CAAudioHardwareStream theStream(theCurrentStreamID);
			
			//	remove all the items
			[theFormatPopUp removeAllItems];
			
			//	add an item for each format
			for(UInt32 theFormatIndex = 0; theFormatIndex < theNumberFormats; ++theFormatIndex)
			{
				//	get the name for the format
				CAStreamBasicDescription::GetSimpleName(theFormats[theFormatIndex], theFormatNameCString, false);
				theFormatName = [[NSString alloc] initWithCString: theFormatNameCString];
				[theFormatName autorelease];
				
				//	add it to the menu
				[theFormatPopUp addItemWithTitle: theFormatName];
			}
			
			//	set the selected item to the current format
			AudioStreamBasicDescription theCurrentFormat;
			theStream.GetCurrentIOProcFormat(theCurrentFormat);
			CAStreamBasicDescription::GetSimpleName(theCurrentFormat, theFormatNameCString, false);
			theFormatName = [[NSString alloc] initWithCString: theFormatNameCString];
			[theFormatName autorelease];
			int theCurrentFormatIndex = [theFormatPopUp indexOfItemWithTitle: theFormatName];
			[theFormatPopUp selectItemAtIndex: theCurrentFormatIndex];
			
			//	and enable the menu
			[theFormatPopUp setEnabled: YES];
		}
		else
		{
			//	no data sources, remove all the times
			[theFormatPopUp removeAllItems];
			
			//	disable the whole thing
			[theFormatPopUp setEnabled: NO];
		}
	}
	else
	{
		[theFormatPopUp setEnabled: NO];
	}
	
	CACatch;
}

-(IBAction)	IOProcFormatPopUpAction:	(id)inSender
{
	AudioStreamID theCurrentStreamID = 0;
	CAAudioHardwareDeviceSectionID theSection = kAudioDeviceSectionInput;
	NSPopUpButton* theFormatPopUp = NULL;
	UInt32 theNumberFormats = 0;
	AudioStreamBasicDescription* theFormats = NULL;

	try
	{	
		if(inSender == mInputIOProcFormatPopUp)
		{
			theCurrentStreamID = mCurrentInputStream;
			theSection = kAudioDeviceSectionInput;
			theFormatPopUp = mInputIOProcFormatPopUp;
			theNumberFormats = mNumberInputIOProcFormats;
			theFormats = mInputIOProcFormats;
		}
		else
		{
			theCurrentStreamID = mCurrentOutputStream;
			theSection = kAudioDeviceSectionOutput;
			theFormatPopUp = mOutputIOProcFormatPopUp;
			theNumberFormats = mNumberOutputIOProcFormats;
			theFormats = mOutputIOProcFormats;
		}
		
		//	get the index of the selected item
		int theSelectedItemIndex = [theFormatPopUp indexOfSelectedItem];
		if((theSelectedItemIndex != -1) && (theSelectedItemIndex < (int)theNumberFormats))
		{
			//	tell the stream about the new format
			CAAudioHardwareStream theStream(theCurrentStreamID);
			theStream.SetCurrentIOProcFormat(theFormats[theSelectedItemIndex]);
		}
	}
	catch(...)
	{
		//	upate the value
		[self UpdateIOProcFormatItems: theSection];
	}
}

-(void)	UpdatePhysicalFormatItems:	(CAAudioHardwareDeviceSectionID)inSection
{
	char theFormatNameCString[256];
	NSString* theFormatName;
	
	CATry;
	
	AudioStreamID theCurrentStreamID = ((inSection == kAudioDeviceSectionInput) ? mCurrentInputStream : mCurrentOutputStream);
	NSPopUpButton* theFormatPopUp = ((inSection == kAudioDeviceSectionInput) ? mInputPhysicalFormatPopUp : mOutputPhysicalFormatPopUp);
	UInt32 theNumberFormats = ((inSection == kAudioDeviceSectionInput) ? mNumberInputPhysicalFormats : mNumberOutputPhysicalFormats);
	AudioStreamBasicDescription* theFormats = ((inSection == kAudioDeviceSectionInput) ? mInputPhysicalFormats : mOutputPhysicalFormats);
	
	if(!mDeviceIsDead)
	{
		if(theCurrentStreamID != 0)
		{
			CAAudioHardwareStream theStream(theCurrentStreamID);
			
			//	remove all the items
			[theFormatPopUp removeAllItems];
			
			//	add an item for each format
			for(UInt32 theFormatIndex = 0; theFormatIndex < theNumberFormats; ++theFormatIndex)
			{
				//	get the name for the format
				CAStreamBasicDescription::GetSimpleName(theFormats[theFormatIndex], theFormatNameCString, false);
				theFormatName = [[NSString alloc] initWithCString: theFormatNameCString];
				[theFormatName autorelease];
				
				//	add it to the menu
				[theFormatPopUp addItemWithTitle: theFormatName];
			}
			
			//	set the selected item to the current format
			AudioStreamBasicDescription theCurrentFormat;
			theStream.GetCurrentPhysicalFormat(theCurrentFormat);
			CAStreamBasicDescription::GetSimpleName(theCurrentFormat, theFormatNameCString, false);
			theFormatName = [[NSString alloc] initWithCString: theFormatNameCString];
			[theFormatName autorelease];
			int theCurrentFormatIndex = [theFormatPopUp indexOfItemWithTitle: theFormatName];
			[theFormatPopUp selectItemAtIndex: theCurrentFormatIndex];
			
			//	and enable the menu
			[theFormatPopUp setEnabled: YES];
		}
		else
		{
			//	no data sources, remove all the times
			[theFormatPopUp removeAllItems];
			
			//	disable the whole thing
			[theFormatPopUp setEnabled: NO];
		}
	}
	else
	{
		[theFormatPopUp setEnabled: NO];
	}
	
	CACatch;
}

-(IBAction)	PhysicalFormatPopUpAction:	(id)inSender
{
	AudioStreamID theCurrentStreamID = 0;
	CAAudioHardwareDeviceSectionID theSection = kAudioDeviceSectionInput;
	NSPopUpButton* theFormatPopUp = NULL;
	UInt32 theNumberFormats = 0;
	AudioStreamBasicDescription* theFormats = NULL;

	try
	{	
		if(inSender == mInputPhysicalFormatPopUp)
		{
			theCurrentStreamID = mCurrentInputStream;
			theSection = kAudioDeviceSectionInput;
			theFormatPopUp = mInputPhysicalFormatPopUp;
			theNumberFormats = mNumberInputPhysicalFormats;
			theFormats = mInputPhysicalFormats;
		}
		else
		{
			theCurrentStreamID = mCurrentOutputStream;
			theSection = kAudioDeviceSectionOutput;
			theFormatPopUp = mOutputPhysicalFormatPopUp;
			theNumberFormats = mNumberOutputPhysicalFormats;
			theFormats = mOutputPhysicalFormats;
		}
		
		//	get the index of the selected item
		int theSelectedItemIndex = [theFormatPopUp indexOfSelectedItem];
		if((theSelectedItemIndex != -1) && (theSelectedItemIndex < (int)theNumberFormats))
		{
			//	tell the stream about the new format
			CAAudioHardwareStream theStream(theCurrentStreamID);
			theStream.SetCurrentPhysicalFormat(theFormats[theSelectedItemIndex]);
		}
	}
	catch(...)
	{
		//	upate the value
		[self UpdatePhysicalFormatItems: theSection];
	}
}

-(void)		SetCurrentStream:			(AudioStreamID)inStream
			Section:					(CAAudioHardwareDeviceSectionID)inSection
			InstallListeners:			(bool)inInstallListeners;
{
	CATry;
	
	AudioStreamID& theCurrentStreamID = ((inSection == kAudioDeviceSectionInput) ? mCurrentInputStream : mCurrentOutputStream);
	
	//	teardown the current stream
	if(theCurrentStreamID != 0)
	{
		CAAudioHardwareStream theStream(theCurrentStreamID);
		
		if(inInstallListeners)
		{
			CATry;
			theStream.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDevicePropertyStreamFormat, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc);
			theStream.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDevicePropertyStreamFormats, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc);
			theStream.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioStreamPropertyPhysicalFormat, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc);
			theStream.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioStreamPropertyPhysicalFormats, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc);
			theStream.RemovePropertyListener(0, kAudioDevicePropertyJackIsConnected, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc);
			theStream.RemovePropertyListener(0, kAudioDevicePropertyDataSource, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc);
			theStream.RemovePropertyListener(0, kAudioDevicePropertyDataSources, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc);
			CACatch;
		}
		
		theCurrentStreamID = 0;
		
		[self UpdateIOProcFormatList: inSection];
		[self UpdatePhysicalFormatList: inSection];
	}
	
	//	set up the new one
	if(inStream != 0)
	{
		//	add listeners for the important stuff
		CAAudioHardwareStream theStream(inStream);
		
		if(inInstallListeners)
		{
			CATry;
			theStream.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDevicePropertyStreamFormat, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc, self);
			theStream.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDevicePropertyStreamFormats, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc, self);
			theStream.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioStreamPropertyPhysicalFormat, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc, self);
			theStream.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioStreamPropertyPhysicalFormats, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc, self);
			theStream.AddPropertyListener(0, kAudioDevicePropertyJackIsConnected, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc, self);
			theStream.AddPropertyListener(0, kAudioDevicePropertyDataSource, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc, self);
			theStream.AddPropertyListener(0, kAudioDevicePropertyDataSources, (AudioStreamPropertyListenerProc)HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc, self);
			CACatch;
		}
		
		theCurrentStreamID = inStream;
		
		//	update the format list
		[self UpdateIOProcFormatList: inSection];
		[self UpdatePhysicalFormatList: inSection];
	}
	
	CACatch;
}

-(void)	UpdateIOProcFormatList:	(CAAudioHardwareDeviceSectionID)inSection
{
	CATry;
	
	AudioStreamID& theCurrentStreamID = ((inSection == kAudioDeviceSectionInput) ? mCurrentInputStream : mCurrentOutputStream);
	UInt32& theNumberFormats = ((inSection == kAudioDeviceSectionInput) ? mNumberInputIOProcFormats : mNumberOutputIOProcFormats);
	AudioStreamBasicDescription*& theFormats = ((inSection == kAudioDeviceSectionInput) ? mInputIOProcFormats : mOutputIOProcFormats);
	
	//	dispose of the old stuff
	if(theFormats != NULL)
	{
		delete[] theFormats;
		theFormats = NULL;
	}
	theNumberFormats = 0;
		
	//	set up format list
	if(theCurrentStreamID != 0)
	{
		CAAudioHardwareStream theStream(theCurrentStreamID);
		
		//	get the number of available formats
		UInt32 theNumberAvailableFormats = theStream.GetNumberAvailableIOProcFormats();
		
		//	allocate enough space to hold all of them if necessary
		theFormats = new AudioStreamBasicDescription[theNumberAvailableFormats];
		
		//	iterate through the available formats, and add them to the list if they are unique
		UInt32 theAvailableFormatIndex = 0;
		while(theAvailableFormatIndex < theNumberAvailableFormats)
		{
			//	get the format
			AudioStreamBasicDescription theFormat;
			theStream.GetAvailableIOProcFormatByIndex(theAvailableFormatIndex, theFormat);
			
			//	remove the fields that have their own controls
			theFormat.mSampleRate = 0;
			
			//	look for the format in the list
			bool wasFound = false;
			for(UInt32 theFormatIndex = 0; !wasFound && (theFormatIndex < theNumberFormats); ++theFormatIndex)
			{
				wasFound = theFormat == theFormats[theFormatIndex];
			}
			
			//	if it wasn't there, add it
			if(!wasFound)
			{
				theFormats[theNumberFormats] = theFormat;
				++theNumberFormats;
			}
			
			//	go to the next one
			++theAvailableFormatIndex;
		}
	}
	
	CACatch;
}

-(void)	UpdatePhysicalFormatList:	(CAAudioHardwareDeviceSectionID)inSection
{
	CATry;
	
	AudioStreamID& theCurrentStreamID = ((inSection == kAudioDeviceSectionInput) ? mCurrentInputStream : mCurrentOutputStream);
	UInt32& theNumberFormats = ((inSection == kAudioDeviceSectionInput) ? mNumberInputPhysicalFormats : mNumberOutputPhysicalFormats);
	AudioStreamBasicDescription*& theFormats = ((inSection == kAudioDeviceSectionInput) ? mInputPhysicalFormats : mOutputPhysicalFormats);
	
	//	dispose of the old stuff
	if(theFormats != NULL)
	{
		delete[] theFormats;
		theFormats = NULL;
	}
	theNumberFormats = 0;
		
	//	set up format list
	if(theCurrentStreamID != 0)
	{
		CAAudioHardwareStream theStream(theCurrentStreamID);
		
		//	get the number of available formats
		UInt32 theNumberAvailableFormats = theStream.GetNumberAvailablePhysicalFormats();
		
		//	allocate enough space to hold all of them if necessary
		theFormats = new AudioStreamBasicDescription[theNumberAvailableFormats];
		
		//	iterate through the available formats, and add them to the list if they are unique
		UInt32 theAvailableFormatIndex = 0;
		while(theAvailableFormatIndex < theNumberAvailableFormats)
		{
			//	get the format
			AudioStreamBasicDescription theFormat;
			theStream.GetAvailablePhysicalFormatByIndex(theAvailableFormatIndex, theFormat);
			
			//	remove the fields that have their own controls
			theFormat.mSampleRate = 0;
			
			//	look for the format in the list
			bool wasFound = false;
			for(UInt32 theFormatIndex = 0; !wasFound && (theFormatIndex < theNumberFormats); ++theFormatIndex)
			{
				wasFound = theFormat == theFormats[theFormatIndex];
			}
			
			//	if it wasn't there, add it
			if(!wasFound)
			{
				theFormats[theNumberFormats] = theFormat;
				++theNumberFormats;
			}
			
			//	go to the next one
			++theAvailableFormatIndex;
		}
	}
	
	CACatch;
}

@end

OSStatus	HLDeviceWindowStreamsControllerAudioDevicePropertyListenerProc(AudioDeviceID inDevice, UInt32 /*inChannel*/, Boolean /*inIsInput*/, AudioDevicePropertyID inPropertyID, HLDeviceWindowStreamsController* inDeviceWindowStreamsController)
{
	NS_DURING
	CATry;
	
	CAAudioHardwareDevice theDevice(inDevice);
//	CAAudioHardwareDeviceSectionID theSection = (inIsInput ? kAudioDeviceSectionInput : kAudioDeviceSectionOutput);
	
	switch(inPropertyID)
	{
		case kAudioDevicePropertyDeviceIsAlive:
			inDeviceWindowStreamsController->mDeviceIsDead = true;
			[inDeviceWindowStreamsController UpdateCurrentStreamItems: kAudioDeviceSectionInput];
			[inDeviceWindowStreamsController UpdateStreamInfoItems: kAudioDeviceSectionInput];
			[inDeviceWindowStreamsController UpdateDataSourceItems: kAudioDeviceSectionInput];
			[inDeviceWindowStreamsController UpdatePhysicalFormatItems: kAudioDeviceSectionInput];
			
			[inDeviceWindowStreamsController UpdateCurrentStreamItems: kAudioDeviceSectionOutput];
			[inDeviceWindowStreamsController UpdateStreamInfoItems: kAudioDeviceSectionOutput];
			[inDeviceWindowStreamsController UpdateDataSourceItems: kAudioDeviceSectionOutput];
			[inDeviceWindowStreamsController UpdatePhysicalFormatItems: kAudioDeviceSectionOutput];
			break;
		
		case kAudioDevicePropertyDeviceHasChanged:
		case kAudioDevicePropertyStreams:
			{
				//	the device's stream list has changed
				
				//	all cached AudioStreamIDs are now invalid, so we have to pick a new stream to be the current one
				UInt32 theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionInput);
				AudioStreamID theNewStream = 0;
				if(theNumberStreams > 0)
				{
					theNewStream = theDevice.GetStreamByIndex(kAudioDeviceSectionInput, 0);
				}
				[inDeviceWindowStreamsController SetCurrentStream: theNewStream Section: kAudioDeviceSectionInput InstallListeners: false];
				[inDeviceWindowStreamsController UpdateCurrentStreamItems: kAudioDeviceSectionInput];
				[inDeviceWindowStreamsController UpdateStreamInfoItems: kAudioDeviceSectionInput];
				[inDeviceWindowStreamsController UpdateDataSourceItems: kAudioDeviceSectionInput];
				[inDeviceWindowStreamsController UpdatePhysicalFormatItems: kAudioDeviceSectionInput];
				
				theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionInput);
				theNewStream = 0;
				if(theNumberStreams > 0)
				{
					theNewStream = theDevice.GetStreamByIndex(kAudioDeviceSectionOutput, 0);
				}
				[inDeviceWindowStreamsController SetCurrentStream: theNewStream Section: kAudioDeviceSectionOutput InstallListeners: false];
				[inDeviceWindowStreamsController UpdateCurrentStreamItems: kAudioDeviceSectionOutput];
				[inDeviceWindowStreamsController UpdateStreamInfoItems: kAudioDeviceSectionOutput];
				[inDeviceWindowStreamsController UpdateDataSourceItems: kAudioDeviceSectionOutput];
				[inDeviceWindowStreamsController UpdatePhysicalFormatItems: kAudioDeviceSectionOutput];
			}
			break;
	}
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	return 0; 
}

OSStatus	HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc(AudioStreamID inStream, UInt32 /*inChannel*/, AudioDevicePropertyID inPropertyID, HLDeviceWindowStreamsController* inDeviceWindowStreamsController)
{
	NS_DURING
	CATry;
	
	CAAudioHardwareStream theStream(inStream);
	CAAudioHardwareDeviceSectionID theSection = theStream.GetSection();
	
	switch(inPropertyID)
	{
		case kAudioDevicePropertyStreamFormat:
		case kAudioDevicePropertyStreamFormats:
			[inDeviceWindowStreamsController UpdateIOProcFormatList: theSection];
			[inDeviceWindowStreamsController UpdateIOProcFormatItems: theSection];
			break;
			
		case kAudioStreamPropertyPhysicalFormat:
		case kAudioStreamPropertyPhysicalFormats:
			[inDeviceWindowStreamsController UpdatePhysicalFormatList: theSection];
			[inDeviceWindowStreamsController UpdatePhysicalFormatItems: theSection];
			break;
			
		case kAudioDevicePropertyJackIsConnected:
			[inDeviceWindowStreamsController UpdateStreamInfoItems: theSection];
			break;
		
		case kAudioDevicePropertyDataSource:
		case kAudioDevicePropertyDataSources:
			[inDeviceWindowStreamsController UpdateDataSourceItems: theSection];
			break;
			
	};
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	return 0;
}
