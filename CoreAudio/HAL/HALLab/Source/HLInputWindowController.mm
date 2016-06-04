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
	HLInputWindowController.mm

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "HLInputWindowController.h"

//	Local Includes
#include "HLApplicationDelegate.h"
#include "HLDeviceMenuController.h"
#include "HLFileSystem.h"

//	PublicUtility Includes
#include "CAAudioBufferList.h"
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareSystem.h"
#include "CAAudioTimeStamp.h"
#include "CAAutoDisposer.h"
#include "CACFString.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAHostTimeBase.h"
#include "CAStreamBasicDescription.h"

//	System Includes
#import <string.h>

//=============================================================================
//	HLInputWindowController
//=============================================================================

@implementation HLInputWindowController

-(id)	initWithApplicationDelegate:	(HLApplicationDelegate*)inApplicationDelegate
{
	CATry;
	
	//	initialize the super class
    [super initWithWindowNibName: @"InputWindow"];
	
	//	initialize the tinks
	mAudioDeviceIOProcTink = new CATink<AudioDeviceIOProc>((AudioDeviceIOProc)HLInputWindowControllerAudioDeviceIOProc);
	mAudioDevicePropertyListenerTink = new CATink<AudioDevicePropertyListenerProc>((AudioDevicePropertyListenerProc)HLInputWindowControllerAudioDevicePropertyListenerProc);
	mAudioStreamPropertyListenerTink = new CATink<AudioStreamPropertyListenerProc>((AudioStreamPropertyListenerProc)HLInputWindowControllerAudioStreamPropertyListenerProc);
	
	//	initialize the basic stuff
	mApplicationDelegate = inApplicationDelegate;
	
	//	initialize the device stuff
	mDeviceNumberInputStreams = 0;
	mDeviceInputStreamFormats = NULL;
	mDeviceNumberOutputStreams = 0;
	mDeviceOutputStreamFormats = NULL;
	mDeviceIsDoingIO = false;
	mDeviceIsDoingPlayThru = false;
	mDeviceIsDoingRecording = false;
	mCloseRecordingFile = false;
	[self SetupDevice: CAAudioHardwareSystem::GetDefaultDevice(kAudioDeviceSectionInput, false)];
	
	//	initialize the file stuff
	mRecordingFile = NULL;
	
	CACatch;
	
	return self;
}

-(void)	windowDidLoad
{
	CATry;
	
	//	get the device
	mDevice = [mDeviceMenuController GetSelectedAudioDevice];
	
	//	update the device UI
	[self UpdateDeviceInfo];
	mDeviceIsDoingPlayThru = [mPlayThruCheckBox intValue] != 0;
	mDeviceIsDoingRecording = [mRecordingCheckBox intValue] != 0;
	
	//	make a string to print the time for the 0 time for the time display in the telemetry
	Float64 theTimeInMilliseconds = [mApplicationDelegate GetNotificationStartTime] / 1000000.0;
	NSString* theStartTimeString = [[NSString alloc] initWithFormat: @"%f: Absolute Start Time (milliseconds)\n", theTimeInMilliseconds];
	
	//	add it to the telemetry
	int theLength = [[mTelemetryTextView textStorage] length];
	[[mTelemetryTextView textStorage] replaceCharactersInRange: NSMakeRange(theLength, 0) withString: theStartTimeString];
	[theStartTimeString release];
	
	[mStartDelayPopUp selectItemAtIndex: 0];

	CACatch;
}

-(void)	dealloc
{
	CATry;
	
	[self StopIO];
	[self TeardownDevice: mDevice];
	
	if(mRecordingFile != NULL)
	{
		ExtAudioFileDispose(mRecordingFile);
	}
	
	delete mAudioDeviceIOProcTink;
	delete mAudioDevicePropertyListenerTink;
	delete mAudioStreamPropertyListenerTink;
	
	CACatch;

	[super dealloc];
}

-(AudioDeviceID)	GetAudioDeviceID
{
	return [mDeviceMenuController GetSelectedAudioDevice];
}

-(bool)	IsDoingIO
{
	return mDeviceIsDoingIO;
}

-(void)	windowWillClose:	(NSNotification*)inNotification
{
	//	the window is closing, so arrange to get cleaned up
	[mApplicationDelegate DestroyInputWindow: self];
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

-(IBAction)	StartHardwareButtonAction:	(id)inSender
{
	CATry;
	
	if(mDevice != 0)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		theDevice.StartIOProc(NULL);
		[self AppendTelemetry: [[NSString alloc] initWithString: @"starting up the device"]];
	}
	
	CACatch;
}

-(IBAction)	StopHardwareButtonAction:	(id)inSender
{
	CATry;
	
	if(mDevice != 0)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		theDevice.StopIOProc(NULL);
		[self AppendTelemetry: [[NSString alloc] initWithString: @"stopping the device"]];
	}
	
	CACatch;
}

-(IBAction)	StartIOButtonAction:	(id)inSender
{
	CATry;
	
	if((mDevice != 0) && !mDeviceIsDoingIO)
	{
		[self StartIO];
	}
	
	CACatch;
}

-(IBAction)	StopIOButtonAction:	(id)inSender
{
	CATry;
	
	if((mDevice != 0) && mDeviceIsDoingIO)
	{
		[self StopIO];
	}
	
	CACatch;
}

-(IBAction) PlayThruCheckBoxAction: (id)inSender
{
	CATry;
	
	mDeviceIsDoingPlayThru = [mPlayThruCheckBox intValue] != 0;
	
	CACatch;
}

-(IBAction) RecordingCheckBoxAction: (id)inSender
{
	CATry;
	
	mDeviceIsDoingRecording = [mRecordingCheckBox intValue] != 0;
	
	CACatch;
}

-(void)	UpdateDeviceInfo
{
	CATry;
	
	if(mDevice != 0)
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	update the text fields
		[mDeviceSampleRateTextField setDoubleValue: theDevice.GetNominalSampleRate()];
		[mDeviceNumberChannelsTextField setIntValue: theDevice.GetTotalNumberChannels(kAudioDeviceSectionInput)];
		[mDeviceBufferSizeTextField setIntValue: theDevice.GetIOBufferSize()];
	}
	else
	{
		//	no device, so clear out the fields
		[mDeviceSampleRateTextField setStringValue: @""];
		[mDeviceNumberChannelsTextField setStringValue: @""];
		[mDeviceBufferSizeTextField setStringValue: @""];
	}
	
	CACatch;
}

-(void)	AppendTelemetry:	(NSString*)inTelemetry
{
	//	write out the time stamp
	UInt64 theTime = AudioConvertHostTimeToNanos(AudioGetCurrentHostTime());
	theTime -= [mApplicationDelegate GetNotificationStartTime];
	Float64 theTimeInMilliseconds = theTime / 1000000.0;
	NSString* theTimeString = [[NSString alloc] initWithFormat: @"%f: ", theTimeInMilliseconds];
	
	int theLength = [[mTelemetryTextView textStorage] length];
	[[mTelemetryTextView textStorage] replaceCharactersInRange: NSMakeRange(theLength, 0) withString: theTimeString];
	
	[theTimeString release];

	//	write out the telemetry
	theLength = [[mTelemetryTextView textStorage] length];
	[[mTelemetryTextView textStorage] replaceCharactersInRange: NSMakeRange(theLength, 0) withString: inTelemetry];
	
	//	write out the newline
	theLength = [[mTelemetryTextView textStorage] length];
	[[mTelemetryTextView textStorage] replaceCharactersInRange: NSMakeRange(theLength, 0) withString: @"\n"];
	
	//	release the string
	[inTelemetry release];
}

-(Float64)	GetStartDelay
{
	Float64 theAnswer = 0;
	
	//	retrieve the selected item
	NSMenuItem* theSelectedItem = [mStartDelayPopUp selectedItem];
	if(theSelectedItem != NULL)
	{
		//	the tag of the selected items is the number of milliseconds
		theAnswer = [theSelectedItem tag] / 1000.0;
	}
	
	return theAnswer;
}

-(void)	SetupDevice:	(AudioDeviceID)inDevice
{
	//	This routine is for configuring the IO device and installing
	//	IOProcs and listeners. The strategy for this window is to only
	//	respond to changes in the device. If the user wants to change
	//	things about the device that affect IO, it will be done in the
	//  device info window.
	CATry;
	
	if(inDevice != 0)
	{
		[self AppendTelemetry: [[NSString alloc] initWithString: @"setting up device"]];
		
		//	make a device object
		CAAudioHardwareDevice theDevice(inDevice);
		
		//	get the format information
		mDeviceNumberInputStreams = theDevice.GetNumberStreams(kAudioDeviceSectionInput);
		mDeviceInputStreamFormats = (AudioStreamBasicDescription*)calloc(mDeviceNumberInputStreams, sizeof(AudioStreamBasicDescription));
		theDevice.GetCurrentIOProcFormats(kAudioDeviceSectionInput, mDeviceNumberInputStreams, mDeviceInputStreamFormats);
		
		mDeviceNumberOutputStreams = theDevice.GetNumberStreams(kAudioDeviceSectionOutput);
		mDeviceOutputStreamFormats = (AudioStreamBasicDescription*)calloc(mDeviceNumberOutputStreams, sizeof(AudioStreamBasicDescription));
		theDevice.GetCurrentIOProcFormats(kAudioDeviceSectionOutput, mDeviceNumberOutputStreams, mDeviceOutputStreamFormats);
		
		//	install the IO proc
		theDevice.AddIOProc((AudioDeviceIOProc)mAudioDeviceIOProcTink, self);
		
		//	install a listener for the device running
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyDeviceIsRunning, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
		
		//	install a listener for the device dying
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
		
		//	install a listener for IO overloads
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDeviceProcessorOverload, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
		
		//	install a listener for buffer size changes
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyBufferFrameSize, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
		
		//	install a listener for sample rate changes
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyNominalSampleRate, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
		
		//	install a listener for stream layout changes
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionWildcard, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
	}
	
	CACatch;
}

-(void)	TeardownDevice:	(AudioDeviceID)inDevice
{
	CATry;
	
	if(inDevice != 0)
	{
		[self AppendTelemetry: [[NSString alloc] initWithString: @"tearing down device"]];
		
		//	make a device object
		CAAudioHardwareDevice theDevice(inDevice);
		
		//	get rid of the format info
		mDeviceNumberInputStreams = 0;
		free(mDeviceInputStreamFormats);
		mDeviceNumberOutputStreams = 0;
		free(mDeviceOutputStreamFormats);
		
		//	remove the IO proc
		theDevice.RemoveIOProc((AudioDeviceIOProc)mAudioDeviceIOProcTink);
		
		//	remove the listener for the device dying
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink);
		
		//	remove the listener for IO overloads
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDeviceProcessorOverload, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink);
		
		//	remove the listener for buffer size changes
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyBufferFrameSize, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink);
		
		//	remove the listener for sample rate changes
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyNominalSampleRate, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink);
		
		//	remove the listener for stream layout changes
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionInput, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink);
	}

	CACatch;
}

-(void)	StartIO
{
	CATry;
	
	if((mDevice != 0) && !mDeviceIsDoingIO)
	{
		[self AppendTelemetry: [[NSString alloc] initWithString: @"starting IO"]];
		mIOCounter = 0;
		
		//	make a device object
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	initialize the file object, if we're recording
		if(mDeviceIsDoingRecording)
		{
			//	we record everything into /tmp using a unique name.
			char thePath[64];
			UInt32 theFileNameIndex = 0;
			do
			{
				++theFileNameIndex;
				sprintf(thePath, "/tmp/HALLabRecording-%04lu.aiff", theFileNameIndex);
			}
			while(HLFileSystem::ObjectExists(thePath));
			
			//	make an FSRef for the /tmp directory
			FSRef theParentDirectory;
			HLFileSystem::MakeFSRefFromPath("/tmp", theParentDirectory);
			
			//	construct a CFString to hold the name
			CACFString theFileName(CFStringCreateWithFormat(NULL, NULL, CFSTR("HALLabRecording-%04lu.aiff"), theFileNameIndex));
			
			//	construct the ASBD for the file, which is going to be 24 bit signed integer
			AudioStreamBasicDescription theFormat;
			theFormat.mSampleRate = mDeviceInputStreamFormats[0].mSampleRate;
			theFormat.mFormatID = kAudioFormatLinearPCM;
			theFormat.mFormatFlags = kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
			theFormat.mBitsPerChannel = 24;
			theFormat.mChannelsPerFrame = mDeviceInputStreamFormats[0].mChannelsPerFrame;
			theFormat.mBytesPerFrame = theFormat.mChannelsPerFrame * 3;
			theFormat.mFramesPerPacket = 1;
			theFormat.mBytesPerPacket = theFormat.mBytesPerFrame;
			
			//	create the file
			OSStatus theError = ExtAudioFileCreateNew(&theParentDirectory, theFileName.GetCFString(), kAudioFileAIFCType, &theFormat, NULL, &mRecordingFile);
			if(theError == 0)
			{
				//	tell the file what format the data is we're going to give it
				theError = ExtAudioFileSetProperty(mRecordingFile, kExtAudioFileProperty_ClientDataFormat, sizeof(AudioStreamBasicDescription), mDeviceInputStreamFormats);
				if(theError == 0)
				{
					//	prep it for IO
					theError = ExtAudioFileWriteAsync(mRecordingFile, 0, NULL);
					if(theError != 0)
					{
						//	failed at prepping the file
						DebugMessage("HLInputWindowController::StartIO: Couldn't prep the recording file for IO");
						ExtAudioFileDispose(mRecordingFile);
						mRecordingFile = NULL;
					}
				}
				else
				{
					//	failed at setting the format on the file, so don't do any recording
					DebugMessage("HLInputWindowController::StartIO: Couldn't set the format of the recording file");
					ExtAudioFileDispose(mRecordingFile);
					mRecordingFile = NULL;
				}
			}
			else
			{
				//	couldn't open the file, so don't do any recording
				DebugMessage("HLInputWindowController::StartIO: Couldn't open the recording file");
				mRecordingFile = NULL;
			}
		}
			
		//	get the start delay (in seconds)
		Float64 theStartDelaySeconds = [self GetStartDelay];
		
		if(theStartDelaySeconds > 0.0)
		{
			//	there is a start delay, calculate when to start
			
			//	get the current time (in samples)
			AudioTimeStamp theCurrentTime = CAAudioTimeStamp::kZero;
			theCurrentTime.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;
			theDevice.GetCurrentTime(theCurrentTime);
			
			//	append the current time
			NSString* theString = [[NSString alloc] initWithFormat: @"        current time: (%9.0f, %12qd)", theCurrentTime.mSampleTime, theCurrentTime.mHostTime];
			[self AppendTelemetry: theString];
			
			//	add the start delay
			AudioTimeStamp theStartHostTime = CAAudioTimeStamp::kZero;
			theStartHostTime.mHostTime = theCurrentTime.mHostTime + AudioConvertNanosToHostTime(static_cast<UInt64>(theStartDelaySeconds * 1000000000.0));
			theStartHostTime.mFlags = kAudioTimeStampHostTimeValid;
			
			//	convert that to a sample time too
			AudioTimeStamp theStartTime = CAAudioTimeStamp::kZero;
			theStartTime.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;
			theDevice.TranslateTime(theStartHostTime, theStartTime);
			
			//	append the info about when we want to start
			theString = [[NSString alloc] initWithFormat: @"requested start time: (%9.0f, %12qd)", theStartTime.mSampleTime, theStartTime.mHostTime];
			[self AppendTelemetry: theString];
			
			//	start the IO proc
			theDevice.StartIOProcAtTime((AudioDeviceIOProc)mAudioDeviceIOProcTink, theStartTime, true, false);
			
			//	append the info about when the HAL says we will start
			theString = [[NSString alloc] initWithFormat: @"      HAL start time: (%9.0f, %12qd)", theStartTime.mSampleTime, theStartTime.mHostTime];
			[self AppendTelemetry: theString];
		}
		else
		{
			//	no start delay, so just start the IO proc
			theDevice.StartIOProc((AudioDeviceIOProc)mAudioDeviceIOProcTink);
		}
		
		//	the device is doing IO, so start the spinner
		[mIsDoingIOIndicator startAnimation: self];

		mDeviceIsDoingIO = true;
	}
	
	CACatch;
}

-(void)	StopIO
{
	CATry;
	
	if((mDevice != 0) && mDeviceIsDoingIO)
	{
		[self AppendTelemetry: [[NSString alloc] initWithString: @"stopping IO"]];

		//	make a device object
		CAAudioHardwareDevice theDevice(mDevice);
		
		mDeviceIsDoingIO = false;
		mCloseRecordingFile = true;
		
		//	the device is doing IO, so stop the spinner
		[mIsDoingIOIndicator stopAnimation: self];
	
		//	stop the IO proc
		theDevice.StopIOProc((AudioDeviceIOProc)mAudioDeviceIOProcTink);
	}
	
	CACatch;
}

-(AudioDeviceID)	GetInitialSelectedDevice:	(HLDeviceMenuController*)inDeviceMenuControl
{
	//	the initial selection of the device menu is the default output device
	return CAAudioHardwareSystem::GetDefaultDevice(true, false);
}

-(void)	SelectedDeviceChanged:	(HLDeviceMenuController*)inDeviceMenuControl
		OldDevice:				(AudioDeviceID)inOldDeviceID
		NewDevice:				(AudioDeviceID)inNewDeviceID
{
	//	save the IO state
	bool wasDoingIO = mDeviceIsDoingIO;
	
	//	stop IO
	[self StopIO];
	
	//	teardown the current device
	[self TeardownDevice: inOldDeviceID];
	
	mDevice = inNewDeviceID;

	//	setup the new device
	[self SetupDevice: inNewDeviceID];
	
	//	update the device info
	[self UpdateDeviceInfo];
	
	//	restart IO
	if(wasDoingIO)
	{
		[self StartIO];
	}
}

-(BOOL)	ShouldDeviceBeInMenu:	(HLDeviceMenuController*)inDeviceMenuControl
		Device:					(AudioDeviceID)inDeviceID
{
	CAAudioHardwareDevice theDevice(inDeviceID);
	
	BOOL theAnswer = NO;
	
	if(theDevice.HasSection(kAudioDeviceSectionInput))
	{
		theAnswer = YES;
	}
	
	return theAnswer;
}

@end

OSStatus	HLInputWindowControllerAudioDeviceIOProc(AudioDeviceID inDevice, const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime, AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime, HLInputWindowController* inInputWindowController)
{
	CATry;
	
	CAAudioHardwareDevice theDevice(inDevice);
	
	if((inInputTime != NULL) && (inInputWindowController->mIOCounter == 0))
	{
		//	append the info about when really started
		NSString* theString = [[NSString alloc] initWithFormat: @"     real start time: (%9.0f, %12qd)", inInputTime->mSampleTime, inInputTime->mHostTime];
		[inInputWindowController performSelectorOnMainThread: @selector(AppendTelemetry:) withObject: theString waitUntilDone: NO];
	}
	
	if(inInputWindowController->mDeviceIsDoingPlayThru && (inInputData != NULL) && (outOutputData != NULL))
	{
		CAAudioBufferList::Copy(*inInputData, 0, *outOutputData, 0);
	}
	
	if(inInputWindowController->mDeviceIsDoingRecording && (inInputWindowController->mRecordingFile != NULL) && (inInputData != NULL))
	{
		//	recording is enabled, so write the first input stream's data
		UInt32 theNumberFramesToWrite = inInputData->mBuffers[0].mDataByteSize / inInputWindowController->mDeviceInputStreamFormats[0].mBytesPerFrame;
		AudioBufferList theWriteABL = { 1, { inInputData->mBuffers[0].mNumberChannels, inInputData->mBuffers[0].mDataByteSize, inInputData->mBuffers[0].mData } };
		OSStatus theError = ExtAudioFileWriteAsync(inInputWindowController->mRecordingFile, theNumberFramesToWrite, &theWriteABL);
		AssertNoError(theError, "HLInputWindowControllerAudioDeviceIOProc: couldn't write the data");
	}
	
	CACatch;
	
	++inInputWindowController->mIOCounter;
	
	return 0;
}

OSStatus	HLInputWindowControllerAudioDevicePropertyListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLInputWindowController* inInputWindowController)
{
	NS_DURING
	CATry;
	
	//	only react to master channel, output section notifications here
	if((inDevice == [inInputWindowController GetAudioDeviceID]) && (inChannel == 0) && (inIsInput == 0))
	{
		bool isDoingIO;
		CAAudioHardwareDevice theDevice(inDevice);
		NSString* theTelemetry = NULL;
		bool deferTelemetry = false;
	
		switch(inPropertyID)
		{
			case kAudioDevicePropertyDeviceIsRunning:
				theTelemetry = [[NSString alloc] initWithString: @"device transport changed state"];
				deferTelemetry = true;
	
				if(inInputWindowController->mCloseRecordingFile && (inInputWindowController->mRecordingFile != NULL))
				{
					inInputWindowController->mCloseRecordingFile = false;
					ExtAudioFileDispose(inInputWindowController->mRecordingFile);
					inInputWindowController->mRecordingFile = NULL;
				}
				break;
				
			case kAudioDevicePropertyDeviceIsAlive:
				{
					theTelemetry = [[NSString alloc] initWithString: @"device has died"];
	
					//	the device is dead, so stop IO if necessary
					isDoingIO = [inInputWindowController IsDoingIO];
					if(isDoingIO)
					{
						[inInputWindowController StopIO];
					}
					
					//	teardown the device
					[inInputWindowController TeardownDevice: inDevice];
					
					//	change the IO device to the default device
					[inInputWindowController SetupDevice: CAAudioHardwareSystem::GetDefaultDevice(kAudioDeviceSectionOutput, false)];
					
					//	restart IO, if necessary
					if(isDoingIO)
					{
						[inInputWindowController StartIO];
					}
					
					//	update the info
					[inInputWindowController UpdateDeviceInfo];
				}
				break;
				
			case kAudioDeviceProcessorOverload:
				{
					theTelemetry = [[NSString alloc] initWithString: @"overload"];
					deferTelemetry = true;
				}
				break;
				
			case kAudioDevicePropertyBufferFrameSize:
				theTelemetry = [[NSString alloc] initWithString: @"buffer size changed"];
				[inInputWindowController UpdateDeviceInfo];
				break;
				
			case kAudioDevicePropertyNominalSampleRate:
				theTelemetry = [[NSString alloc] initWithString: @"sample rate changed"];
				
				//	the device's format has changed, so stop IO if necessary
				isDoingIO = [inInputWindowController IsDoingIO];
				if(isDoingIO)
				{
					[inInputWindowController StopIO];
				}
				
				//	teardown the device
				[inInputWindowController TeardownDevice: inDevice];
				
				//	rebuild the device
				[inInputWindowController SetupDevice: inDevice];
				
				//	restart IO, if necessary
				if(isDoingIO)
				{
					[inInputWindowController StartIO];
				}
				
				//	update the info
				[inInputWindowController UpdateDeviceInfo];
				break;
				
			case kAudioDevicePropertyStreamConfiguration:
				theTelemetry = [[NSString alloc] initWithString: @"stream configuration changed"];
				
				//	the device's format has changed, so stop IO if necessary
				isDoingIO = [inInputWindowController IsDoingIO];
				if(isDoingIO)
				{
					[inInputWindowController StopIO];
				}
				
				//	teardown the device
				[inInputWindowController TeardownDevice: inDevice];
				
				//	rebuild the device
				[inInputWindowController SetupDevice: inDevice];
				
				//	restart IO, if necessary
				if(isDoingIO)
				{
					[inInputWindowController StartIO];
				}
				
				//	update the info
				[inInputWindowController UpdateDeviceInfo];
				break;
		};
	
		if(theTelemetry != NULL)
		{
			if(deferTelemetry)
			{
				[inInputWindowController performSelectorOnMainThread: @selector(AppendTelemetry:) withObject: theTelemetry waitUntilDone: NO];
			}
			else
			{
				[inInputWindowController AppendTelemetry: theTelemetry];
			}
		}
	}
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER

	return 0;
}

OSStatus	HLInputWindowControllerAudioStreamPropertyListenerProc(AudioStreamID /*inStream*/, UInt32 /*inChannel*/, AudioDevicePropertyID /*inPropertyID*/, HLInputWindowController* /*inInputWindowController*/)
{
	NS_DURING
	CATry;
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	return 0;
}
