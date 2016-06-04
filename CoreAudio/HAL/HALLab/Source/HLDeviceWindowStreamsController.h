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
	HLDeviceWindowStreamsController.h

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Local Includes
#include "CAAudioHardwareDevice.h"

//	System Includes
#include <Cocoa/Cocoa.h>

//=============================================================================
//	Types
//=============================================================================

@class	HLDeviceWindowController;

//=============================================================================
//	HLDeviceWindowStreamsController
//=============================================================================

@interface HLDeviceWindowStreamsController : NSObject
{

@public
	IBOutlet NSPopUpButton*				mInputCurrentStreamPopUp;
	IBOutlet NSTextField*				mInputAudioStreamIDTextField;
	IBOutlet NSTextField*				mInputStartingChannelTextField;
	IBOutlet NSTextField*				mInputTerminalTypeTextField;
	IBOutlet NSPopUpButton*				mInputDataSourcePopUp;
	IBOutlet NSTextField*				mInputConnectedTextField;
	IBOutlet NSPopUpButton*				mInputIOProcFormatPopUp;
	IBOutlet NSPopUpButton*				mInputPhysicalFormatPopUp;

	IBOutlet NSPopUpButton*				mOutputCurrentStreamPopUp;
	IBOutlet NSTextField*				mOutputAudioStreamIDTextField;
	IBOutlet NSTextField*				mOutputStartingChannelTextField;
	IBOutlet NSTextField*				mOutputTerminalTypeTextField;
	IBOutlet NSPopUpButton*				mOutputDataSourcePopUp;
	IBOutlet NSTextField*				mOutputConnectedTextField;
	IBOutlet NSPopUpButton*				mOutputIOProcFormatPopUp;
	IBOutlet NSPopUpButton*				mOutputPhysicalFormatPopUp;

	IBOutlet HLDeviceWindowController*	mWindowController;
	
	AudioDeviceID						mDevice;
	bool								mDeviceIsDead;
	
	AudioStreamID						mCurrentInputStream;
	UInt32								mNumberInputIOProcFormats;
	AudioStreamBasicDescription*		mInputIOProcFormats;
	UInt32								mNumberInputPhysicalFormats;
	AudioStreamBasicDescription*		mInputPhysicalFormats;
	
	AudioStreamID						mCurrentOutputStream;
	UInt32								mNumberOutputIOProcFormats;
	AudioStreamBasicDescription*		mOutputIOProcFormats;
	UInt32								mNumberOutputPhysicalFormats;
	AudioStreamBasicDescription*		mOutputPhysicalFormats;

}

//	Construction/Destruction
-(id)		init;
-(void)		awakeFromNib;
-(void)		dealloc;

//	View Management
-(void)		UpdateCurrentStreamItems:	(CAAudioHardwareDeviceSectionID)inSection;
-(IBAction)	CurrentStreamPopUpAction:	(id)inSender;

-(void)		UpdateStreamInfoItems:		(CAAudioHardwareDeviceSectionID)inSection;

-(void)		UpdateDataSourceItems:		(CAAudioHardwareDeviceSectionID)inSection;
-(IBAction)	DataSourcePopUpAction:		(id)inSender;

-(void)		UpdateIOProcFormatItems:	(CAAudioHardwareDeviceSectionID)inSection;
-(IBAction)	IOProcFormatPopUpAction:	(id)inSender;

-(void)		UpdatePhysicalFormatItems:	(CAAudioHardwareDeviceSectionID)inSection;
-(IBAction)	PhysicalFormatPopUpAction:	(id)inSender;

//	Stream Management
-(void)		SetCurrentStream:			(AudioStreamID)inStream
			Section:					(CAAudioHardwareDeviceSectionID)inSection
			InstallListeners:			(bool)inInstallListeners;
-(void)		UpdateIOProcFormatList:		(CAAudioHardwareDeviceSectionID)inSection;
-(void)		UpdatePhysicalFormatList:	(CAAudioHardwareDeviceSectionID)inSection;

@end

OSStatus	HLDeviceWindowStreamsControllerAudioDevicePropertyListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLDeviceWindowStreamsController* inDeviceWindowStreamsController);
OSStatus	HLDeviceWindowStreamsControllerAudioStreamPropertyListenerProc(AudioStreamID inStream, UInt32 inChannel, AudioDevicePropertyID inPropertyID, HLDeviceWindowStreamsController* inDeviceWindowStreamsController);
