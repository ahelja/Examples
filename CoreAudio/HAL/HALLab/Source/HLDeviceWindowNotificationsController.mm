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
	HLDeviceWindowNotificationsController.mm

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "HLDeviceWindowNotificationsController.h"

//	Local Includes
#include "HLApplicationDelegate.h"
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareStream.h"
#include "HLDeviceWindowController.h"

//	PublicUtility Includes
#include "CAException.h"

//=============================================================================
//	HLDeviceWindowNotificationsController
//=============================================================================

@implementation HLDeviceWindowNotificationsController

-(id)	init
{
	mDevice = 0;
	return [super init];
}

-(void)	awakeFromNib
{
	CATry;
	
	mDevice = [mWindowController GetAudioDeviceID];
	
	//	make a string to print the time for the 0 time for the time display in the notifications
	Float64 theNotificationTimeInMilliseconds = [[mWindowController GetApplicationDelegate] GetNotificationStartTime] / 1000000.0;
	NSString* theStartTimeString = [[NSString alloc] initWithFormat: @"%f: Absolute Start Time (milliseconds)\n", theNotificationTimeInMilliseconds];
	[theStartTimeString autorelease];
	
	//	append that to the text views
	int theLength = [[mDeviceNotificationsTextView textStorage] length];
	[[mDeviceNotificationsTextView textStorage] replaceCharactersInRange: NSMakeRange(theLength, 0) withString: theStartTimeString];
	theLength = [[mStreamNotificationsTextView textStorage] length];
	[[mStreamNotificationsTextView textStorage] replaceCharactersInRange: NSMakeRange(theLength, 0) withString: theStartTimeString];
	
	[self InstallListeners];
	
	CACatch;
}

-(void)	dealloc
{
	[self RemoveListeners];
	
	[super dealloc];
}

-(void)	InstallListeners
{
	CATry;
	
	if(!mDeviceIsDead && (mDevice != 0))
	{
		CAAudioHardwareDevice theDevice(mDevice);
		
		//	add a device listener for all notifications
		theDevice.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardSection, kAudioPropertyWildcardPropertyID, (AudioDevicePropertyListenerProc)HLDeviceWindowNotificationsControllerAudioDevicePropertyListenerProc, self);
		
		//	add a Listener to all the input streams
		UInt32 theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionInput);
		UInt32 theIndex = 0;
		for(; theIndex < theNumberStreams; ++theIndex)
		{
			CAAudioHardwareStream theStream(theDevice.GetStreamByIndex(kAudioDeviceSectionInput, theIndex));
			theStream.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardPropertyID, (AudioStreamPropertyListenerProc)HLDeviceWindowNotificationsControllerAudioStreamPropertyListenerProc, self);
		}
		theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionOutput);
		for(theIndex = 0; theIndex < theNumberStreams; ++theIndex)
		{
			CAAudioHardwareStream theStream(theDevice.GetStreamByIndex(kAudioDeviceSectionOutput, theIndex));
			theStream.AddPropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardPropertyID, (AudioStreamPropertyListenerProc)HLDeviceWindowNotificationsControllerAudioStreamPropertyListenerProc, self);
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
		theDevice.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardSection, kAudioPropertyWildcardPropertyID, (AudioDevicePropertyListenerProc)HLDeviceWindowNotificationsControllerAudioDevicePropertyListenerProc);
		
		//	remove the Listener from all the input streams
		UInt32 theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionInput);
		UInt32 theIndex = 0;
		for(; theIndex < theNumberStreams; ++theIndex)
		{
			CAAudioHardwareStream theStream(theDevice.GetStreamByIndex(kAudioDeviceSectionInput, theIndex));
			theStream.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardPropertyID, (AudioStreamPropertyListenerProc)HLDeviceWindowNotificationsControllerAudioStreamPropertyListenerProc);
		}
		theNumberStreams = theDevice.GetNumberStreams(kAudioDeviceSectionOutput);
		for(theIndex = 0; theIndex < theNumberStreams; ++theIndex)
		{
			CAAudioHardwareStream theStream(theDevice.GetStreamByIndex(kAudioDeviceSectionOutput, theIndex));
			theStream.RemovePropertyListener(kAudioPropertyWildcardChannel, kAudioPropertyWildcardPropertyID, (AudioStreamPropertyListenerProc)HLDeviceWindowNotificationsControllerAudioStreamPropertyListenerProc);
		}
	}
	
	CACatch;
}

@end

OSStatus	HLDeviceWindowNotificationsControllerAudioDevicePropertyListenerProc(AudioDeviceID /*inDevice*/, UInt32 inChannel, Boolean inIsInput, AudioDevicePropertyID inPropertyID, HLDeviceWindowNotificationsController* inDeviceWindowNotificationsController)
{
	NSAutoreleasePool* thePool = [[NSAutoreleasePool alloc] init];
	
	NS_DURING
	CATry;

	//	get the time of the notification
	UInt64 theNotificationTime = AudioConvertHostTimeToNanos(AudioGetCurrentHostTime());
	Float64 theNotificationTimeInMilliseconds = (theNotificationTime - [[inDeviceWindowNotificationsController->mWindowController GetApplicationDelegate] GetNotificationStartTime]) / 1000000.0;
	
	//	figure out what the name of the notification is and do some processing
	char* theNotificationNameString = NULL;
	switch(inPropertyID)
	{
		case kAudioDevicePropertyDeviceIsAlive:
			theNotificationNameString = "Is Alive";
			inDeviceWindowNotificationsController->mDeviceIsDead = true;
			break;
			
		case kAudioDevicePropertyStreams:
			theNotificationNameString = "Stream List";
			[inDeviceWindowNotificationsController RemoveListeners];
			[inDeviceWindowNotificationsController InstallListeners];
			break;
	};
	
	//	make a string out of the 4CC property ID
	char* the4CC = (char*)&inPropertyID;
	char thePropertyIDString[5] = { the4CC[0], the4CC[1], the4CC[2], the4CC[3], 0 };
	
	//	make a string out of the section
	const char* theSectionString = inIsInput ? "Input" : "Output";
	
	//	make a string for the notification
	NSString* theNotificationString = NULL;
	if(theNotificationNameString != NULL)
	{
		theNotificationString = [[NSString alloc] initWithFormat: @"%f: %s ('%s') on %s channel %d\n", theNotificationTimeInMilliseconds, theNotificationNameString, thePropertyIDString, theSectionString, inChannel];
	}
	else
	{
		theNotificationString = [[NSString alloc] initWithFormat: @"%f: '%s' on %s channel %d\n", theNotificationTimeInMilliseconds, thePropertyIDString, theSectionString, inChannel];
	}
	[theNotificationString autorelease];
	
	//	append that to the text view
	int theLength = [[inDeviceWindowNotificationsController->mDeviceNotificationsTextView textStorage] length];
	[[inDeviceWindowNotificationsController->mDeviceNotificationsTextView textStorage] replaceCharactersInRange: NSMakeRange(theLength, 0) withString: theNotificationString];
	
	CACatch;
	NS_HANDLER
	NS_ENDHANDLER
	
	[thePool release];
	
	return 0;
}

OSStatus	HLDeviceWindowNotificationsControllerAudioStreamPropertyListenerProc(AudioStreamID inStream, UInt32 inChannel, AudioDevicePropertyID inPropertyID, HLDeviceWindowNotificationsController* inDeviceWindowNotificationsController)
{
	NSAutoreleasePool* thePool = [[NSAutoreleasePool alloc] init];
	
	CATry;
	NS_DURING;

	//	get the time of the notification
	UInt64 theNotificationTime = AudioConvertHostTimeToNanos(AudioGetCurrentHostTime());
	Float64 theNotificationTimeInMilliseconds = (theNotificationTime - [[inDeviceWindowNotificationsController->mWindowController GetApplicationDelegate] GetNotificationStartTime]) / 1000000.0;
	
	char* theNotificationNameString = NULL;
	switch(inPropertyID)
	{
	};
	
	//	make a string out of the 4CC property ID
	char* the4CC = (char*)&inPropertyID;
	char thePropertyIDString[5] = { the4CC[0], the4CC[1], the4CC[2], the4CC[3], 0 };
	
	//	make a string for the notification
	NSString* theNotificationString = NULL;
	if(theNotificationNameString != NULL)
	{
		theNotificationString = [[NSString alloc] initWithFormat: @"%f: 0x%X: %s ('%s') on channel %d\n", theNotificationTimeInMilliseconds, inStream, theNotificationNameString, thePropertyIDString, inChannel];
	}
	else
	{
		theNotificationString = [[NSString alloc] initWithFormat: @"%f: 0x%X: '%s' on channel %d\n", theNotificationTimeInMilliseconds, inStream, thePropertyIDString, inChannel];
	}
	[theNotificationString autorelease];
	
	//	append that to the text view
	int theLength = [[inDeviceWindowNotificationsController->mStreamNotificationsTextView textStorage] length];
	[[inDeviceWindowNotificationsController->mStreamNotificationsTextView textStorage] replaceCharactersInRange: NSMakeRange(theLength, 0) withString: theNotificationString];
	
	NS_HANDLER
	NS_ENDHANDLER
	CACatch;
	
	[thePool release];
	
	return 0;
}
