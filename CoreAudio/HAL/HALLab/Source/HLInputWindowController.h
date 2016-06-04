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
	HLInputWindowController.h

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	PublicUtility Includes
#include "CATink.h"

//	System Includes
#include <AppKit/AppKit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>

//=============================================================================
//	Types
//=============================================================================

@class	HLApplicationDelegate;
@class	HLDeviceMenuController;

//=============================================================================
//	HLInputWindowController
//=============================================================================

@interface HLInputWindowController : NSWindowController
{

@public
	//	Device Info Items
	IBOutlet HLDeviceMenuController*	mDeviceMenuController;
	IBOutlet NSButton*					mDeviceInfoButton;
	IBOutlet NSTextField*				mDeviceSampleRateTextField;
	IBOutlet NSTextField*				mDeviceNumberChannelsTextField;
	IBOutlet NSTextField*				mDeviceBufferSizeTextField;
	
	//	Other Items
	IBOutlet NSTextView*				mTelemetryTextView;
	IBOutlet NSProgressIndicator*		mIsDoingIOIndicator;
	IBOutlet NSPopUpButton*				mStartDelayPopUp;
	IBOutlet NSButton*					mPlayThruCheckBox;
	IBOutlet NSButton*					mRecordingCheckBox;
	
	//	Device Stuff
	AudioDeviceID						mDevice;
	UInt32								mDeviceNumberInputStreams;
	AudioStreamBasicDescription*		mDeviceInputStreamFormats;
	UInt32								mDeviceNumberOutputStreams;
	AudioStreamBasicDescription*		mDeviceOutputStreamFormats;
	bool								mDeviceIsDoingIO;
	bool								mDeviceIsDoingPlayThru;
	bool								mDeviceIsDoingRecording;
	UInt32								mIOCounter;
	
	//	Recording Stuff
	ExtAudioFileRef						mRecordingFile;
	bool								mCloseRecordingFile;
	
	//	Basic Stuff
	HLApplicationDelegate*				mApplicationDelegate;
	
	//	Tinks
	CATink<AudioDeviceIOProc>*					mAudioDeviceIOProcTink;
	CATink<AudioDevicePropertyListenerProc>*	mAudioDevicePropertyListenerTink;
	CATink<AudioStreamPropertyListenerProc>*	mAudioStreamPropertyListenerTink;
	
}

//	Constrution/Destruction
-(id)				initWithApplicationDelegate:	(HLApplicationDelegate*)inApplicationDelegate;
-(void)				windowDidLoad;
-(void)				dealloc;

	//	Attributes
-(AudioDeviceID)	GetAudioDeviceID;
-(bool)				IsDoingIO;

	//	Window Management
-(void)				windowWillClose:				(NSNotification*)inNotification;

	//	View Management
-(IBAction)			DeviceInfoButtonAction:			(id)inSender;
-(IBAction)			StartHardwareButtonAction:		(id)inSender;
-(IBAction)			StopHardwareButtonAction:		(id)inSender;
-(IBAction)			StartIOButtonAction:			(id)inSender;
-(IBAction)			StopIOButtonAction:				(id)inSender;
-(IBAction)			PlayThruCheckBoxAction:			(id)inSender;
-(IBAction)			RecordingCheckBoxAction:		(id)inSender;

-(void)				UpdateDeviceInfo;
-(void)				AppendTelemetry:				(NSString*)inTelemetry;
-(Float64)			GetStartDelay;

	//	Device Handling
-(void)				SetupDevice:					(AudioDeviceID)inDevice;
-(void)				TeardownDevice:					(AudioDeviceID)inDevice;
-(void)				StartIO;
-(void)				StopIO;

	//	Device Menu support
-(AudioDeviceID)	GetInitialSelectedDevice:		(HLDeviceMenuController*)inDeviceMenuControl;
-(void)				SelectedDeviceChanged:			(HLDeviceMenuController*)inDeviceMenuControl
					OldDevice:						(AudioDeviceID)inOldDeviceID
					NewDevice:						(AudioDeviceID)inNewDeviceID;
-(BOOL)				ShouldDeviceBeInMenu:			(HLDeviceMenuController*)inDeviceMenuControl
					Device:							(AudioDeviceID)inDeviceID;

@end

//	HAL Callbacks
OSStatus	HLInputWindowControllerAudioHardwarePropertyListener(AudioHardwarePropertyID inPropertyID, HLInputWindowController* inInputWindowController);
OSStatus	HLInputWindowControllerAudioDeviceIOProc(AudioDeviceID inDevice, const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime, AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime, HLInputWindowController* inInputWindowController);
OSStatus	HLInputWindowControllerAudioDevicePropertyListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLInputWindowController* inInputWindowController);
OSStatus	HLInputWindowControllerAudioStreamPropertyListenerProc(AudioStreamID inStream, UInt32 inChannel, AudioDevicePropertyID inPropertyID, HLInputWindowController* inInputWindowController);
