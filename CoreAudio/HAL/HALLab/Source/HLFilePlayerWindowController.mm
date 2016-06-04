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
	HLFilePlayerWindowController.mm

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "HLFilePlayerWindowController.h"

//	Local Includes
#include "HLApplicationDelegate.h"
#include "HLAudioConverter.h"
#include "HLAudioFile.h"
#include "HLDeviceMenuController.h"
#include "HLFileSystem.h"

//	PublicUtility Includes
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareSystem.h"
#include "CAAudioTimeStamp.h"
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAStreamBasicDescription.h"

//	System Includes
#import <string.h>

//=============================================================================
//	HLFilePlayerWindowController
//=============================================================================

@implementation HLFilePlayerWindowController

-(id)	initWithApplicationDelegate:	(HLApplicationDelegate*)inApplicationDelegate
{
	CATry;
	
	//	initialize the super class
    [super initWithWindowNibName: @"FilePlayerWindow"];
	
	//	initialize the tinks
	mAudioDeviceIOProcTink = new CATink<AudioDeviceIOProc>((AudioDeviceIOProc)HLFilePlayerWindowControllerAudioDeviceIOProc);
	mAudioDevicePropertyListenerTink = new CATink<AudioDevicePropertyListenerProc>((AudioDevicePropertyListenerProc)HLFilePlayerWindowControllerAudioDevicePropertyListenerProc);
	mAudioStreamPropertyListenerTink = new CATink<AudioStreamPropertyListenerProc>((AudioStreamPropertyListenerProc)HLFilePlayerWindowControllerAudioStreamPropertyListenerProc);
	
	//	initialize the basic stuff
	mApplicationDelegate = inApplicationDelegate;
	
	//	initialize the file stuff
	mFileIsLoaded = false;
	mFileName = NULL;
	memset(&mFileFormat, 0, sizeof(AudioStreamBasicDescription));
	mFileData = NULL;
	mFileDataSize = 0;
	mFilePlaybackPosition = 0;
	
	//	initialize the device stuff
	mDeviceNumberStreams = 0;
	mDeviceStreamFormats = NULL;
	mDeviceIsPlaying = false;
	[self SetupDevice: CAAudioHardwareSystem::GetDefaultDevice(kAudioDeviceSectionOutput, false)];
	
	CACatch;
	
	return self;
}

-(void)	windowDidLoad
{
	CATry;
	
	//	get the device
	mDevice = [mDeviceMenuController GetSelectedAudioDevice];
	
	//	update the file UI
	[self UpdateFileInfo];
	
	//	update the device UI
	[self UpdateDeviceInfo];
	
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
	
	[self StopPlayback];
	[self TeardownDevice: mDevice];
	[self UnloadFile];
	[self TeardownConverters];
	
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

-(bool)	IsPlaying
{
	return mDeviceIsPlaying;
}

-(void)	windowWillClose:	(NSNotification*)inNotification
{
	//	the window is closing, so arrange to get cleaned up
	[mApplicationDelegate DestroyFilePlayerWindow: self];
}

-(IBAction)	LoadFileButtonAction:	(id)inSender
{
	CATry;
	
	//	save the playback state
	bool wasPlaying = mDeviceIsPlaying;
	
	//	stop playback
	[self StopPlayback];
			
	NSArray*		theFileTypes = [NSArray arrayWithObjects: @"aif", @"aiff", NSFileTypeForHFSTypeCode('AIFF'), NSFileTypeForHFSTypeCode('AIFC'), @"sd2", NSFileTypeForHFSTypeCode('Sd2f'), @"wav", NSFileTypeForHFSTypeCode('WAVE'), nil];
	NSOpenPanel*	theOpenPanel = [NSOpenPanel openPanel];

	int theResult = [theOpenPanel runModalForDirectory: NULL file: NULL types: theFileTypes];
	
    if(theResult == NSOKButton)
	{
        NSArray* theFilesToOpen = [theOpenPanel filenames];
		if([theFilesToOpen count] > 0)
		{
			NSString* theFileName = [theFilesToOpen objectAtIndex: 0];
			
			[self UnloadFile];
			[self LoadFile: theFileName];
			
			[self TeardownConverters];
			[self SetupConverters];
			
			[self UpdateFileInfo];
		}
    }
	
	//	restart playback
	if(wasPlaying)
	{
		[self StartPlayback];
	}
	
	CACatch;
}

-(IBAction)	SendFileToAllChannelsCheckBoxAction:	(id)inSender
{
	CATry;
	
	//	the channel map for all the converters will change
	bool isPlaying = [self IsPlaying];
	if(isPlaying)
	{
		[self StopPlayback];
	}
	
	//	rebuild the converters
	[self TeardownConverters];
	[self SetupConverters];

	//	restart playback, if necessary
	if(isPlaying)
	{
		[self StartPlayback];
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
	
	if((mDevice != 0) && !mDeviceIsPlaying)
	{
		//	initialize the counter
		mFilePlaybackPosition = 0;
		
		[self StartPlayback];
	}
	
	CACatch;
}

-(IBAction)	StopIOButtonAction:	(id)inSender
{
	CATry;
	
	if((mDevice != 0) && mDeviceIsPlaying)
	{
		[self StopPlayback];
	}
	
	CACatch;
}

-(void)	UpdateFileInfo
{
	CATry;
	
	if(mFileIsLoaded)
	{
		[mFileNameTextField setStringValue: mFileName];
		[mFileSampleRateTextField setDoubleValue: mFileFormat.mSampleRate];
		char theFormatName[256];
		CAStreamBasicDescription::GetSimpleName(mFileFormat, theFormatName, false);
		NSString* theString = [NSString stringWithCString: theFormatName];
		[mFileFormatTextField setStringValue: theString];
		[mFileSizeTextField setIntValue: mFileDataSize];
	}
	else
	{
		[mFileNameTextField setStringValue: @""];
		[mFileSampleRateTextField setStringValue: @""];
		[mFileFormatTextField setStringValue: @""];
		[mFileSizeTextField setStringValue: @""];
	}

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
		[mDeviceNumberChannelsTextField setIntValue: theDevice.GetTotalNumberChannels(kAudioDeviceSectionOutput)];
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

-(void)	LoadFile:	(NSString*)inFileName
{
	if(!mFileIsLoaded)
	{
		HLAudioFile* theFile = NULL;
		//	make a file object from the path
		try
		{
			//	get the size of the name
			theFile = dynamic_cast<HLAudioFile*>(HLFileSystem::AllocateObject((CFStringRef)inFileName));
		}
		catch(...)
		{
			theFile = NULL;
		}
		if(theFile != NULL)
		{
			//	open the file
			theFile->Open(true, false);
			
			//	get the file's name
			mFileName = [inFileName retain];
		
			//	get the file's format
			theFile->GetFormat(mFileFormat);
			
			//	get the number of bytes of audio data in the file
			mFileDataSize = theFile->GetAudioByteSize();
			
			//	allocate a buffer to hold the whole thing
			mFileData = (Byte*)malloc(mFileDataSize);
			
			//	read in all the data
			theFile->ReadAudioBytes(0, mFileDataSize, mFileData, false);
			
			//	update the size
			mFileDataSize /= mFileFormat.mBytesPerFrame;
			
			mFileIsLoaded = true;
			
			//	close the file
			theFile->Close();
		
			//	toss the file
			delete theFile;
		}
	}
}

-(void)	UnloadFile
{
	if(mFileIsLoaded)
	{
		[mFileName release];
		mFileName = NULL;
		memset(&mFileFormat, 0, sizeof(AudioStreamBasicDescription));
		free(mFileData);
		mFileData = NULL;
		mFileDataSize = 0;
		mFilePlaybackPosition = 0;
		mFileIsLoaded = false;
	}
}

-(void)	SetupDevice:	(AudioDeviceID)inDevice
{
	//	This routine is for configuring the playback device and installing
	//	IOProcs and listeners. The strategy for this window is to only
	//	respond to changes in the device and to reconfigure the converter
	//	to match things up. If the user wants to change things about the
	//	device that affect playback, it will be done in the device info
	//	window.
	CATry;
	
	if(inDevice != 0)
	{
		[self AppendTelemetry: [[NSString alloc] initWithString: @"setting up device"]];
		
		//	make a device object
		CAAudioHardwareDevice theDevice(inDevice);
		
		//	get the format information
		mDeviceNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionOutput);
		mDeviceStreamFormats = (AudioStreamBasicDescription*)calloc(mDeviceNumberStreams, sizeof(AudioStreamBasicDescription));
		theDevice.GetCurrentIOProcFormats(kAudioDeviceSectionOutput, mDeviceNumberStreams, mDeviceStreamFormats);
		
		//	install the IO proc
		theDevice.AddIOProc((AudioDeviceIOProc)mAudioDeviceIOProcTink, self);
		
		//	turn off all the input streams
		UInt32 theNumberInputStreams = theDevice.GetNumberStreams(kAudioDeviceSectionInput);
		if(theNumberInputStreams > 0)
		{
			CAAutoFree<bool> theInputStreamUsage(theNumberInputStreams);
			for(UInt32 theInputStreamIndex = 0; theInputStreamIndex < theNumberInputStreams; ++theInputStreamIndex)
			{
				theInputStreamUsage[theInputStreamIndex] = false;
			}
			theDevice.SetIOProcStreamUsage((AudioDeviceIOProc)mAudioDeviceIOProcTink, kAudioDeviceSectionInput, theInputStreamUsage);
		}
		
		//	turn on all the output streams
		UInt32 theNumberOutputStreams = theDevice.GetNumberStreams(kAudioDeviceSectionOutput);
		if(theNumberOutputStreams > 0)
		{
			CAAutoFree<bool> theOutputStreamUsage(theNumberOutputStreams);
			for(UInt32 theOutputStreamIndex = 0; theOutputStreamIndex < theNumberOutputStreams; ++theOutputStreamIndex)
			{
				theOutputStreamUsage[theOutputStreamIndex] = true;
			}
			theDevice.SetIOProcStreamUsage((AudioDeviceIOProc)mAudioDeviceIOProcTink, kAudioDeviceSectionOutput, theOutputStreamUsage);
		}
		
		//	install a listener for the device dying
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
		
		//	install a listener for IO overloads
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDeviceProcessorOverload, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
		
		//	install a listener for buffer size changes
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyBufferFrameSize, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
		
		//	install a listener for sample rate changes
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyNominalSampleRate, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
		
		//	install a listener for stream layout changes
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink, self);
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
		mDeviceNumberStreams = 0;
		free(mDeviceStreamFormats);
		
		//	remove the IO proc
		theDevice.RemoveIOProc((AudioDeviceIOProc)mAudioDeviceIOProcTink);
		
		//	remove the listener for the device dying
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyDeviceIsAlive, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink);
		
		//	remove the listener for IO overloads
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDeviceProcessorOverload, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink);
		
		//	remove the listener for buffer size changes
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyBufferFrameSize, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink);
		
		//	remove the listener for sample rate changes
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyNominalSampleRate, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink);
		
		//	remove the listener for stream layout changes
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput, kAudioDevicePropertyStreamConfiguration, (AudioDevicePropertyListenerProc)mAudioDevicePropertyListenerTink);
	}

	CACatch;
}

-(void)	StartPlayback
{
	CATry;
	
	if((mDevice != 0) && !mDeviceIsPlaying)
	{
		[self AppendTelemetry: [[NSString alloc] initWithString: @"starting playback"]];
		mIOCounter = 0;
		
		//	make a device object
		CAAudioHardwareDevice theDevice(mDevice);
		
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
			theDevice.StartIOProcAtTime((AudioDeviceIOProc)mAudioDeviceIOProcTink, theStartTime, false, false);
			
			//	append the info about when the HAL says we will start
			theString = [[NSString alloc] initWithFormat: @"      HAL start time: (%9.0f, %12qd)", theStartTime.mSampleTime, theStartTime.mHostTime];
			[self AppendTelemetry: theString];
		}
		else
		{
			//	no start delay, so just start the IO proc
			theDevice.StartIOProc((AudioDeviceIOProc)mAudioDeviceIOProcTink);
		}
		
		//	the device is playing, so start the spinner
		[mIsPlayingIndicator startAnimation: self];

		mDeviceIsPlaying = true;
	}
	
	CACatch;
}

-(void)	StopPlayback
{
	CATry;
	
	if((mDevice != 0) && mDeviceIsPlaying)
	{
		[self AppendTelemetry: [[NSString alloc] initWithString: @"stopping playback"]];

		//	make a device object
		CAAudioHardwareDevice theDevice(mDevice);
		
		mDeviceIsPlaying = false;
		
		//	the device is playing, so stop the spinner
		[mIsPlayingIndicator stopAnimation: self];
	
		//	stop the IO proc
		theDevice.StopIOProc((AudioDeviceIOProc)mAudioDeviceIOProcTink);
	}
	
	CACatch;
}

-(AudioDeviceID)	GetInitialSelectedDevice:	(HLDeviceMenuController*)inDeviceMenuControl
{
	//	the initial selection of the device menu is the default output device
	return CAAudioHardwareSystem::GetDefaultDevice(false, false);
}

-(void)	SelectedDeviceChanged:	(HLDeviceMenuController*)inDeviceMenuControl
		OldDevice:				(AudioDeviceID)inOldDeviceID
		NewDevice:				(AudioDeviceID)inNewDeviceID
{
	if((mDevice == inOldDeviceID) && (inNewDeviceID != inOldDeviceID))
	{
		//	save the playback state
		bool wasPlaying = mDeviceIsPlaying;
		
		//	stop playback
		[self StopPlayback];
		
		//	teardown the current device
		[self TeardownDevice: inOldDeviceID];
		
		mDevice = [mDeviceMenuController GetSelectedAudioDevice];
	
		//	setup the new device
		[self SetupDevice: inNewDeviceID];
		
		//	rebuild the converters
		[self TeardownConverters];
		[self SetupConverters];
		
		//	update the device info
		[self UpdateDeviceInfo];
		
		//	restart playback
		if(wasPlaying)
		{
			[self StartPlayback];
		}
	}
}

-(BOOL)	ShouldDeviceBeInMenu:	(HLDeviceMenuController*)inDeviceMenuControl
		Device:					(AudioDeviceID)inDeviceID
{
	CAAudioHardwareDevice theDevice(inDeviceID);
	
	BOOL theAnswer = NO;
	
	if(theDevice.HasSection(kAudioDeviceSectionOutput))
	{
		theAnswer = YES;
	}
	
	return theAnswer;
}

-(void)	SetupConverters
{
	if(mNumberAudioConverters == 0)
	{
		//	we need to have both the file loaded and the device initialized
		//	in order to have the right info to setup the converters
		if(mFileIsLoaded && (mDevice != 0))
		{
			//	we need one AudioConverter per playback stream
			mNumberAudioConverters = mDeviceNumberStreams;
			mAudioConverters = (HLAudioConverter**)calloc(mNumberAudioConverters, sizeof(HLAudioConverter*));
			bool sendFileToAllChannelsCheckBox = [mSendFileToAllChannelsCheckBox intValue] != 0;
			SInt32 theStartingChannel = 0;
			for(UInt32 theStreamIndex = 0; theStreamIndex < mNumberAudioConverters; ++theStreamIndex)
			{
				//	allocate a converter that goes from the file to the stream
				mAudioConverters[theStreamIndex] = new HLAudioConverter(mFileFormat, mDeviceStreamFormats[theStreamIndex], (AudioConverterComplexInputDataProc)HLFilePlayerWindowControllerAudioConverterComplexInputDataProc, self);
				
				//	setup the channel map for the converter
				CAAutoArrayDelete<SInt32> theChannelMap(mDeviceStreamFormats[theStreamIndex].mChannelsPerFrame);
				for(UInt32 theChannelIndex = 0; theChannelIndex < mDeviceStreamFormats[theStreamIndex].mChannelsPerFrame; ++theChannelIndex)
				{
					if(sendFileToAllChannelsCheckBox)
					{
						theChannelMap[theChannelIndex] = (theStartingChannel + theChannelIndex) % mFileFormat.mChannelsPerFrame;
					}
					else
					{
						if((theStartingChannel + theChannelIndex) < mFileFormat.mChannelsPerFrame)
						{
							theChannelMap[theChannelIndex] = theStartingChannel + theChannelIndex;
						}
						else
						{
							theChannelMap[theChannelIndex] = -1 ;
						}
					}
				}
				mAudioConverters[theStreamIndex]->SetChannelMap(mDeviceStreamFormats[theStreamIndex].mChannelsPerFrame, theChannelMap);
				theStartingChannel += mDeviceStreamFormats[theStreamIndex].mChannelsPerFrame;
			}
		}
	}
}

-(void)	TeardownConverters
{
	if(mNumberAudioConverters > 0)
	{
		for(UInt32 theStreamIndex = 0; theStreamIndex < mNumberAudioConverters; ++theStreamIndex)
		{
			delete mAudioConverters[theStreamIndex];
		}
		free(mAudioConverters);
		mAudioConverters = NULL;
		mNumberAudioConverters = 0;
	}
}

@end

OSStatus	HLFilePlayerWindowControllerAudioDeviceIOProc(AudioDeviceID inDevice, const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime, AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime, HLFilePlayerWindowController* inFilePlayerWindowController)
{
	CATry;
	
	CAAudioHardwareDevice theDevice(inDevice);
	
	if((inOutputTime != NULL) && (inFilePlayerWindowController->mIOCounter == 0))
	{
		//	append the info about when really started
		NSString* theString = [[NSString alloc] initWithFormat: @"     real start time: (%9.0f, %12qd)", inOutputTime->mSampleTime, inOutputTime->mHostTime];
		[inFilePlayerWindowController performSelectorOnMainThread: @selector(AppendTelemetry:) withObject: theString waitUntilDone: NO];
	}
	
	++inFilePlayerWindowController->mIOCounter;
		
	if((outOutputData != NULL) && (inFilePlayerWindowController->mNumberAudioConverters > 0))
	{
		//	figure out how many frames of data we need to supply
		UInt32 theNumberFrames = outOutputData->mBuffers[0].mDataByteSize / inFilePlayerWindowController->mDeviceStreamFormats[0].mBytesPerFrame;
			
		//	iterate through all the streams represented in the output AudioBufferList
		for(UInt32 theBufferIndex = 0; theBufferIndex < outOutputData->mNumberBuffers; ++theBufferIndex)
		{
			//	get the AudioBuffer to work on
			AudioBuffer* theBuffer = &outOutputData->mBuffers[theBufferIndex];
			
			//	get the format for the steam
			AudioStreamBasicDescription theFormat = inFilePlayerWindowController->mDeviceStreamFormats[theBufferIndex];
			
			//	set up the AudioBufferList to be pulled into based on the current AudioBuffer
			AudioBufferList theBufferList;
			theBufferList.mNumberBuffers = 1;
			theBufferList.mBuffers[0].mNumberChannels = theBuffer->mNumberChannels;
			theBufferList.mBuffers[0].mDataByteSize = theBuffer->mDataByteSize;
			theBufferList.mBuffers[0].mData = theBuffer->mData;
		
			//	get the converter for the stream
			HLAudioConverter* theConverter = inFilePlayerWindowController->mAudioConverters[theBufferIndex];
			
			//	pull on the converter to get the output data
			if(theConverter != NULL)
			{
				theConverter->FillBuffer(theNumberFrames, &theBufferList, NULL);
			}
			
			//  Skank alert! This is ugly and should be done a different way, but I'm tired and need a break
			if(theNumberFrames <= inFilePlayerWindowController->mFilePlaybackPosition)
			{
				inFilePlayerWindowController->mFilePlaybackPosition -= theNumberFrames;
			}
			else
			{
				inFilePlayerWindowController->mFilePlaybackPosition = (inFilePlayerWindowController->mFilePlaybackPosition + (inFilePlayerWindowController->mFileDataSize - 1)) - theNumberFrames;
			}
		}
		
		//  update the playback position for real now
		inFilePlayerWindowController->mFilePlaybackPosition = (inFilePlayerWindowController->mFilePlaybackPosition + theNumberFrames) % inFilePlayerWindowController->mFileDataSize;
	}
	
	CACatch;
	
	return 0;
}

OSStatus	HLFilePlayerWindowControllerAudioDevicePropertyListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLFilePlayerWindowController* inFilePlayerWindowController)
{
	NS_DURING
	CATry;
	
	//	only react to master channel, output section notifications here
	if((inDevice == [inFilePlayerWindowController GetAudioDeviceID]) && (inChannel == 0) && (inIsInput == 0))
	{
		bool isPlaying;
		CAAudioHardwareDevice theDevice(inDevice);
		NSString* theTelemetry = NULL;
		bool deferTelemetry = false;
	
		switch(inPropertyID)
		{
			case kAudioDevicePropertyDeviceIsAlive:
				{
					theTelemetry = [[NSString alloc] initWithString: @"device has died"];
	
					//	update the device menu with the new device, which has the side effect of changing the device for playback
					[inFilePlayerWindowController->mDeviceMenuController SetSelectedAudioDevice: CAAudioHardwareSystem::GetDefaultDevice(kAudioDeviceSectionOutput, false)];
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
				[inFilePlayerWindowController UpdateDeviceInfo];
				break;
				
			case kAudioDevicePropertyNominalSampleRate:
				theTelemetry = [[NSString alloc] initWithString: @"sample rate changed"];
				
				//	the device's format has changed, so stop playback if necessary
				isPlaying = [inFilePlayerWindowController IsPlaying];
				if(isPlaying)
				{
					[inFilePlayerWindowController StopPlayback];
				}
				
				//	teardown the device
				[inFilePlayerWindowController TeardownDevice: inDevice];
				
				//	rebuild the device
				[inFilePlayerWindowController SetupDevice: inDevice];
				
				//	rebuild the converters
				[inFilePlayerWindowController TeardownConverters];
				[inFilePlayerWindowController SetupConverters];
			
				//	restart playback, if necessary
				if(isPlaying)
				{
					[inFilePlayerWindowController StartPlayback];
				}
				
				//	update the info
				[inFilePlayerWindowController UpdateDeviceInfo];
				break;
				
			case kAudioDevicePropertyStreamConfiguration:
				theTelemetry = [[NSString alloc] initWithString: @"stream configuration changed"];
				
				//	the device's format has changed, so stop playback if necessary
				isPlaying = [inFilePlayerWindowController IsPlaying];
				if(isPlaying)
				{
					[inFilePlayerWindowController StopPlayback];
				}
				
				//	teardown the device
				[inFilePlayerWindowController TeardownDevice: inDevice];
				
				//	rebuild the device
				[inFilePlayerWindowController SetupDevice: inDevice];
				
				//	rebuild the converters
				[inFilePlayerWindowController TeardownConverters];
				[inFilePlayerWindowController SetupConverters];
			
				//	restart playback, if necessary
				if(isPlaying)
				{
					[inFilePlayerWindowController StartPlayback];
				}
				
				//	update the info
				[inFilePlayerWindowController UpdateDeviceInfo];
				break;
		};
	
		if(theTelemetry != NULL)
		{
			if(deferTelemetry)
			{
				[inFilePlayerWindowController performSelectorOnMainThread: @selector(AppendTelemetry:) withObject: theTelemetry waitUntilDone: NO];
			}
			else
			{
				[inFilePlayerWindowController AppendTelemetry: theTelemetry];
			}
		}
	}
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER

	return 0;
}

OSStatus	HLFilePlayerWindowControllerAudioStreamPropertyListenerProc(AudioStreamID /*inStream*/, UInt32 /*inChannel*/, AudioDevicePropertyID /*inPropertyID*/, HLFilePlayerWindowController* /*inFilePlayerWindowController*/)
{
	NS_DURING
	CATry;
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	return 0;
}

OSStatus	HLFilePlayerWindowControllerAudioConverterComplexInputDataProc(AudioConverterRef /*inAudioConverter*/, UInt32* ioNumberDataPackets, AudioBufferList* ioData, AudioStreamPacketDescription** /*outDataPacketDescription*/, HLFilePlayerWindowController* inFilePlayerWindowController)
{
	CATry;
	
	//	figure out how many frames we can return
	UInt32 theFramesToCopy = std::min(*ioNumberDataPackets, inFilePlayerWindowController->mFileDataSize - inFilePlayerWindowController->mFilePlaybackPosition);
	
	//	return that much
	*ioNumberDataPackets = theFramesToCopy;
	
	//	fill out the buffer list
	ioData->mNumberBuffers = 1;
	ioData->mBuffers[0].mNumberChannels = inFilePlayerWindowController->mFileFormat.mChannelsPerFrame;
	ioData->mBuffers[0].mDataByteSize = theFramesToCopy * inFilePlayerWindowController->mFileFormat.mBytesPerFrame;
	ioData->mBuffers[0].mData = inFilePlayerWindowController->mFileData + (inFilePlayerWindowController->mFilePlaybackPosition * inFilePlayerWindowController->mFileFormat.mBytesPerFrame);
	
	//	increment the playback position
	inFilePlayerWindowController->mFilePlaybackPosition += theFramesToCopy;
	if(inFilePlayerWindowController->mFilePlaybackPosition >= inFilePlayerWindowController->mFileDataSize)
	{
		inFilePlayerWindowController->mFilePlaybackPosition = 0;
	}
	
	CACatch;
	
	return 0;
}
