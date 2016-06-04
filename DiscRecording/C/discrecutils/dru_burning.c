/*
	dru_burning.c
	
	Part of the Disc Recording Utility sources for command-line tools.  This
	code provides an example of handling progress during a burn.
*/
/*
 File:  dru_burning.c
 
 Copyright:  © Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
#include <DiscRecording/DiscRecording.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/ttycom.h>
#include "dru_burning.h"
#include "dru_progress.h"


typedef struct	druBurnStatus druBurnStatus;
struct druBurnStatus
{
	int				success;
	CFStringRef		lastState;
	CFNumberRef		lastTrack;
	CFDictionaryRef	completionStatus;
	char			stage[80];
	dru_progress_t	progressBar;
};

void druProgressCallback(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info);


#pragma mark -





/*
	druBurn
	
	Called to do a burn.  Burning is a long async process, so this function mostly
	handles providing appropriate progress and completion information to the user.
*/
int
druBurn(DRBurnRef burn, CFTypeRef layout)
{
	DRNotificationCenterRef		notificationCenter = NULL;
	CFRunLoopSourceRef			source = NULL;
	druBurnStatus				status = {0, NULL, NULL, NULL, {0}, 0};
	
	/* Create a progress bar. */
	status.progressBar = druProgressBarCreate();

	/* Sign up for notifications from the burn object. */
	notificationCenter = DRNotificationCenterCreate();
	source = DRNotificationCenterCreateRunLoopSource(notificationCenter);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	DRNotificationCenterAddObserver(notificationCenter,&status,druProgressCallback, kDRBurnStatusChangedNotification, burn);
	
	/* Okay, kick off the burn. */
	DRBurnWriteLayout(burn, layout);
	
	/* Enter a runloop until the burn finishes. */
	CFRunLoopRun();
	
	/* Clean up memory and exit. */
	CFRunLoopSourceInvalidate(source);
	if (notificationCenter != NULL)	CFRelease(notificationCenter);
	if (source != NULL)				CFRelease(source);
	if (status.progressBar != NULL)	druProgressBarDispose(status.progressBar,status.success);
	if (status.success)
		printf("Burn completed.\n");
	else
		druPrintFailureMessage("Burn", status.completionStatus);
	if (status.completionStatus != NULL)	CFRelease(status.completionStatus);
	
	return status.success;
}





/*
	druErase
	
	Called to do a erase. Pass in the device and a flag to indicate a full or quick erase.
*/
int
druErase(DRDeviceRef device, int fullErase)
{
	DREraseRef					erase = NULL;
	DRNotificationCenterRef		notificationCenter = NULL;
	CFRunLoopSourceRef			source = NULL;
	druBurnStatus				status = {0, NULL, NULL, NULL, {0}, 0};
	CFMutableDictionaryRef		properties;

	erase = DREraseCreate(device);

	if (erase != NULL)
	{
		/* Create a progress bar. */
		status.progressBar = druProgressBarCreate();
		
		/* Sign up for notifications from the erase object. */
		notificationCenter = DRNotificationCenterCreate();
		source = DRNotificationCenterCreateRunLoopSource(notificationCenter);
		CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
		DRNotificationCenterAddObserver(notificationCenter,&status,druProgressCallback,kDREraseStatusChangedNotification, erase);
		
		/* setup erase properties for type of erase to be performed */
		properties = CFDictionaryCreateMutable(NULL,0,&kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
		if (fullErase)
			CFDictionaryAddValue(properties,kDREraseTypeKey,kDREraseTypeComplete);
		else
			CFDictionaryAddValue(properties,kDREraseTypeKey,kDREraseTypeQuick);
		DREraseSetProperties(erase, properties);
		
		/* Okay, start the erase. */
		DREraseStart(erase);
		
		/* Enter a runloop until the burn finishes. */
		CFRunLoopRun();
		
		/* Clean up memory and exit. */
		CFRunLoopSourceInvalidate(source);
		if (notificationCenter != NULL)	CFRelease(notificationCenter);
		if (source != NULL)				CFRelease(source);
		if (status.progressBar != NULL)	druProgressBarDispose(status.progressBar,status.success);
		CFRelease(erase);
	}
	if (status.success)
		printf("Erase completed.\n");
	else
		druPrintFailureMessage("Erase", status.completionStatus);
	if (status.completionStatus != NULL)	CFRelease(status.completionStatus);
	
	return status.success;
}


#pragma mark -


/*
	druProgressCallback
	
	DRNotificationCallback to handle burn or erase progress.
*/
void
druProgressCallback(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef taskStatus)
{
#pragma unused(center, name, object)
	druBurnStatus	*status = (druBurnStatus*)observer;
	CFStringRef		currentState = NULL;
	CFNumberRef		progressRef = NULL;
	CFNumberRef		currentTrackRef = NULL;
	float			progress = 0.0;
	int				currentTrack = 0;
	char			buffer[sizeof(status->stage)];
	
	/* Get information from the status dictionary. */
	currentState = (CFStringRef)CFDictionaryGetValue(taskStatus,kDRStatusStateKey);
	progressRef = (CFNumberRef)CFDictionaryGetValue(taskStatus,kDRStatusPercentCompleteKey);
	currentTrackRef = (CFNumberRef)CFDictionaryGetValue(taskStatus,kDRStatusCurrentTrackKey);

	/* Fetch values from CFNumbers. */
	if (progressRef != NULL)
		CFNumberGetValue(progressRef,kCFNumberFloatType,&progress);
	if (currentTrackRef != NULL)
		CFNumberGetValue(currentTrackRef,kCFNumberIntType,&currentTrack);
	
	/* Check to see if primary burn state has changed. (Preparing, Writing, Verifying, etc) */
	if (status->lastState == NULL ||
		!CFEqual(status->lastState,currentState) ||
		((currentTrackRef != NULL) && !CFEqual(status->lastTrack,currentTrackRef)))
	{
		/* Yes - did we have a previous state? */
		if (status->lastState != NULL)
		{
			/* Forget about the old state. */
			if (status->lastState) CFRelease(status->lastState);
			if (status->lastTrack) CFRelease(status->lastTrack);
		}
		
		/* If the burn was successful, stop the runloop. */
		if (CFEqual(currentState,kDRStatusStateDone))
		{
			status->completionStatus = (CFDictionaryRef)CFRetain(taskStatus);
			status->success = 1;
			CFRunLoopStop(CFRunLoopGetCurrent());
			return;
		}
		
		/* If the burn was unsuccessful, print a failure message (localized)
			and stop the runloop. */
		if (CFEqual(currentState,kDRStatusStateFailed))
		{
			status->completionStatus = (CFDictionaryRef)CFRetain(taskStatus);
			status->success = 0;
			CFRunLoopStop(CFRunLoopGetCurrent());
			return;
		}
		
		/* Remember the new state. */
		status->lastState = CFStringCreateCopy(NULL,currentState);
		if (currentTrackRef) status->lastTrack = (CFNumberRef)CFRetain(currentTrackRef);
		
		/* Translate the stage into a user-visible string.
			
			We only display a few of the possible states - the others are
			brief and the user doesn't really care about them.
		*/
		buffer[0] = 0;
		if (CFEqual(currentState,kDRStatusStatePreparing))
		{
			if (currentTrack == 0)
				snprintf(buffer,sizeof(buffer),"Preparing...");
			else
				snprintf(buffer,sizeof(buffer),"Preparing track %d ...", currentTrack);
		}
		else if (CFEqual(currentState,kDRStatusStateTrackWrite))
		{
			if (currentTrack != 0)
				snprintf(buffer,sizeof(buffer),"Writing track %d ...", currentTrack);
		}
		else if (CFEqual(currentState,kDRStatusStateSessionClose) ||
				 CFEqual(currentState,kDRStatusStateTrackClose))
		{
			snprintf(buffer,sizeof(buffer),"Closing... (this might take a while)");
		}
		else if (CFEqual(currentState,kDRStatusStateVerifying))
		{
			if (currentTrack != 0)
				snprintf(buffer,sizeof(buffer),"Verifying...");
		}
		else if (CFEqual(currentState,kDRStatusStateErasing))
		{
			snprintf(buffer,sizeof(buffer),"Erasing...");
		}
		
		/* Change the stage string - the progress bar will catch this. */
		if (buffer[0] != 0 && strcmp(status->stage,buffer))
			strncpy(status->stage, buffer, sizeof(status->stage));
	}
	
	/* Update the progress bar. */
	druProgressBarUpdate(status->progressBar,status->stage,progress);
}





/*
	druPrintFailureMessage
	
	Prints out a localized burn failure message from the burn engine.
*/
void
druPrintFailureMessage(const char *task, CFDictionaryRef status)
{
	CFDictionaryRef		errorStatus;
	UInt8				message[256];
	UInt8				info[256];
	CFIndex				len = 0;
	
	strncpy((char*)message,"no error message available.",sizeof(message));
	info[0] = 0;
	
	if (status != NULL && CFDictionaryGetValueIfPresent(status,kDRErrorStatusKey,(const void **)&errorStatus))
	{
		CFStringRef		errorString;
		if (CFDictionaryGetValueIfPresent(errorStatus,kDRErrorStatusErrorStringKey,(const void **)&errorString))
		{
			CFStringGetBytes(errorString, CFRangeMake(0,CFStringGetLength(errorString)), kCFStringEncodingASCII,
						'.', false, (UInt8*)message, sizeof(message)-1, &len);
			message[len] = 0;
		}

		if (CFDictionaryGetValueIfPresent(errorStatus,kDRErrorStatusErrorInfoStringKey,(const void **)&errorString))
		{
			CFStringGetBytes(errorString, CFRangeMake(0,CFStringGetLength(errorString)), kCFStringEncodingASCII,
						'.', false, (UInt8*)info, sizeof(info)-1, &len);
			info[len] = 0;
		}
	}
	
	printf("%s failed: %s\n", task, message);

	if (info[0])
		printf("   %s\n",info);
}



