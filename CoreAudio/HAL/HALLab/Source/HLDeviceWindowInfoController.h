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
	HLDeviceWindowInfoController.h

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	System Includes
#include <Cocoa/Cocoa.h>
#include <CoreAudio/CoreAudio.h>

//=============================================================================
//	Types
//=============================================================================

@class	HLDeviceWindowController;

//=============================================================================
//	HLDeviceWindowInfoController
//=============================================================================

@interface HLDeviceWindowInfoController : NSObject
{

@public
	IBOutlet NSTextField*				mAudioDeviceIDTextField;
	IBOutlet NSTextField*				mDeviceIsAliveTextField;
	IBOutlet NSTextField*				mDevicePlugInTextField;
	IBOutlet NSTextField*				mTransportTypeTextField;
	IBOutlet NSTextField*				mIsRunningTextField;
	IBOutlet NSTextField*				mIsRunningSomewhereTextField;
	IBOutlet NSComboBox*				mNominalSampleRateComboBox;
	IBOutlet NSTextField*				mActualSampleRateTextField;
	IBOutlet NSComboBox*				mIOBufferSizeComboBox;
	IBOutlet NSTextField*				mUsesVariableBufferSizesTextField;
	IBOutlet NSTextField*				mIOBufferSizeRangeTextField;
	IBOutlet NSTextField*				mClockDomainTextField;

	IBOutlet NSTextField*				mDeviceNameTextField;
	IBOutlet NSTextField*				mManufacturerTextField;
	IBOutlet NSTextField*				mOwningPlugInTextField;
	IBOutlet NSTextField*				mUIDTextField;
	IBOutlet NSTextField*				mModelUIDTextField;
	IBOutlet NSTextField*				mRelatedDevicesTextField;
	IBOutlet NSTextField*				mConfigAppTextField;
	IBOutlet NSButton*					mLaunchConfigAppButton;
	IBOutlet NSTextField*				mHogModeOwnerTextField;
	IBOutlet NSButton*					mTakeHogModeButton;
	IBOutlet NSMatrix*					mMixingRadioButtonMatrix;
	IBOutlet NSPopUpButton*				mClockSourcePopUp;
	IBOutlet NSTextField*				mInputLatencyTextField;
	IBOutlet NSTextField*				mInputSafetyOffsetTextField;
	IBOutlet NSTextField*				mOutputLatencyTextField;
	IBOutlet NSTextField*				mOutputSafetyOffsetTextField;
	
	IBOutlet HLDeviceWindowController*	mWindowController;
	
	AudioDeviceID						mDevice;
	bool								mDeviceIsDead;
	NSTimer*							mActualSampleRateUpdateTimer;

}

//	Construction/Destruction
-(id)		init;
-(void)		awakeFromNib;
-(void)		dealloc;

//	View Management
-(void)		UpdateDeviceInfo:							(id)inSender;

-(void)		UpdateNominalSampleRateComboBox;
-(IBAction)	NominalSampleRateComboBoxAction:			(id)inSender;
-(int)		numberOfItemsInNominalSampleRateComboBox:	(NSComboBox*)inComboBox;
-(id)		NominalSampleRateComboBox:					(NSComboBox*)inComboBox
			objectValueForItemAtIndex:					(int)inItemIndex;

-(void)		UpdateActualSampleRateItems:				(NSTimer*)inTimer;
-(void)		StartUpdatingActualSampleRateItems:			(id)inSender;
-(void)		StopUpdatingActualSampleRateItems:			(id)inSender;

-(void)		UpdateIOBufferSizeComboBox;
-(IBAction)	IOBufferSizeComboBoxAction:					(id)inSender;
-(int)		numberOfItemsInIOBufferSizeComboBox:		(NSComboBox*)inComboBox;
-(id)		IOBufferSizeComboBox:						(NSComboBox*)inComboBox
			objectValueForItemAtIndex:					(int)inItemIndex;

-(IBAction)	LaunchConfigAppButtonAction:				(id)inSender;

-(void)		UpdateHogModeItems;
-(IBAction)	TakeHogModeButtonAction:					(id)inSender;

-(void)		UpdateMixabilityItems;
-(IBAction)	MixingRadioButtonMatrixAction:				(id)inSender;

-(void)		UpdateClockSourceItems;
-(IBAction)	ClockSourcePopUpAction:						(id)inSender;

-(void)		WindowWillClose;

//	NSComboBox DataSource dispatching
-(int)		numberOfItemsInComboBox: 					(NSComboBox*)inComboBox;
-(id)		comboBox: 									(NSComboBox*)inComboBox
			objectValueForItemAtIndex:					(int)inItemIndex;

//	Device Management
-(void)		InstallListeners;
-(void)		RemoveListeners;

@end

OSStatus	HLDeviceWindowInfoControllerAudioDevicePropertyListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLDeviceWindowInfoController* inDeviceWindowInfoController);
OSStatus	HLDeviceWindowInfoControllerAudioStreamPropertyListenerProc(AudioStreamID inStream, UInt32 inChannel, AudioDevicePropertyID inPropertyID, HLDeviceWindowInfoController* inDeviceWindowInfoController);
