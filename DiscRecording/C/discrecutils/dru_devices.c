/*
	dru_devices.c
	
	Part of the Disc Recording Utility sources for command-line tools.  This
	code provides an example of prompting the user to select a device and/or
	insert media, and how to create a textual description of a device.
*/
/*
 File:  dru_devices.c
 
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
#include "dru_devices.h"

/* DRNotificationCallback to wait for media. */
void druWaitForMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info);

/* DRNotificationCallback to wait for blank media. */
void druWaitForBlankMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info);

/* DRNotificationCallback to wait for erasable media. */
void druWaitForErasableMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info);

/* DRNotificationCallback to wait for overwritable media. */
void druWaitForOverwritableMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info);

/* DRNotificationCallback to wait for appendable media. */
void druWaitForAppendableMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info);

/* Standard druDeviceFilterProcs. */
int druFilter_AnyBurner(DRDeviceRef device);
int druFilter_AnyEraser(DRDeviceRef device);
int druFilter_CDBurners(DRDeviceRef device);
int druFilter_DVDBurners(DRDeviceRef device);



#pragma mark -




/*
	druPromptForDevice
	
	Interactively asks the user to select a device from the devices which are
	currently attached.  If only one device is connected, the device is
	automatically chosen and nothing is printed.
	
	The optional filter function is called to filter devices.  If you wish to
	suppress a device, the filter function should return 0.
	
	The returned device is retained by this routine.
*/
DRDeviceRef
druPromptForDevice(char *promptString, druDeviceFilterProc filter)
{
	CFArrayRef	deviceList = DRCopyDeviceArray();
	CFIndex		deviceCount = CFArrayGetCount(deviceList);
	DRDeviceRef	device;
	CFIndex		selection;
	char		userInput[10];
	
	/* Can't proceed without at least one drive. */
	if (deviceCount == 0)
	{
		printf("Sorry, no CD/DVD drives were found.\n");
		exit(1);
	}
	
	/* Filter the list. */
	if (filter != NULL)
	{
		CFMutableArrayRef	filteredList = CFArrayCreateMutableCopy(NULL,0,deviceList);
		
		for (selection=deviceCount-1; selection>=0; --selection)
			if ((*filter)((DRDeviceRef)CFArrayGetValueAtIndex(filteredList,selection)) == 0)
				CFArrayRemoveValueAtIndex(filteredList,selection);
		
		CFRelease(deviceList);
		deviceList = filteredList;
		deviceCount = CFArrayGetCount(deviceList);
	}
	
	/* Can't proceed without at least one drive. */
	if (deviceCount == 0)
	{
		printf("Sorry, no eligible drives were found.\n");
		exit(1);
	}
	
	/* If there's only one device, which is actually true for many machines (those with
		an internal CD burner and no external burners attached) then the choice
		is obvious, and we don't need to display a menu. */
	if (deviceCount == 1)
	{
		device = (DRDeviceRef)CFArrayGetValueAtIndex(deviceList,0);
		CFRetain(device);
		CFRelease(deviceList);
		return device;
	}
	
	/* Display a menu of devices. */
	printf("Available devices:\n");
	druDisplayDeviceList(deviceList);
	
	/* Display the prompt. */
	if (promptString == NULL)
		promptString = "Please select a device:";
	printf("%s ", promptString);
	fflush(stdout);
	
	/* Get user input. */
	userInput[0] = 0;
	selection = atoi(fgets(userInput,sizeof(userInput),stdin)) - 1;
	if (selection < 0 || selection >= deviceCount)
	{
		printf("Aborted.\n");
		exit(1);
	}
	
	/* Return the selected device. */
	device = (DRDeviceRef)CFArrayGetValueAtIndex(deviceList,selection);
	CFRetain(device);
	CFRelease(deviceList);
	return device;
}



/*
	druPromptForMediaInDevice
	
	Interactively prompts for media in a device.  The type of
	media is not considered, so this is probably most useful for pure data discs.  In other
	situations, the type of media may be important - for example, DVD media is not valid if
	you're writing an audio CD.	
 */
void
druPromptForMediaInDevice(DRDeviceRef device)
{
	DRNotificationCenterRef	notificationCenter = NULL;
	CFRunLoopSourceRef		source = NULL;
	
	/* Indicate our interest in acquiring media reservations.  This is reference counted
		and sticky, reservations will be acquired on our behalf (when possible) until a
		corresponding call to DRDeviceReleaseMediaReservation is made.  We do not make
		that call, its up to our caller to decide if/when its appropriate. */
	DRDeviceAcquireMediaReservation(device);
	
	/* If the device contains blank media right now, then we're done. */
	if (druDeviceContainsMedia(device))
		return;
	
	/* Display a prompt. */
    CFDictionaryRef deviceInfo = DRDeviceCopyInfo((DRDeviceRef)device);
    printf("Please insert media in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
	
	/* Open the tray (and eject existing media, if any). */
	/* This call may or may not work - the media in the device may be busy
		and can't be unmounted, or the device may not even have a tray (slot-load
																		drives are an example of this).  However, we don't really care; this is just
		a convenience to the user and will do the right thing if the right thing
		can be done. */
	/* We also don't want to eject the media if it's still spinning up -
		the user may have just inserted it, and it takes a good 5-10 seconds
		on some drives for discs to be recognized. */
	if (!druDeviceIsBecomingReady(device))
		DRDeviceEjectMedia(device);
	
	/* Sign up for device status notifications, and enter a tiny runloop so that we can avoid polling. */
	notificationCenter = DRNotificationCenterCreate();
	source = DRNotificationCenterCreateRunLoopSource(notificationCenter);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	DRNotificationCenterAddObserver(notificationCenter, NULL, druWaitForMedia, NULL, device);
	CFRunLoopRun();
	CFRunLoopSourceInvalidate(source);
	
	/* Clean up memory and we're done. */
	if (notificationCenter != NULL)	CFRelease(notificationCenter);
	if (source != NULL)				CFRelease(source);
}



/*
	druPromptForBlankMediaInDevice
	
	Interactively prompts for blank, writable media in a particular device.  The type of
	media is not considered, so this is probably most useful for pure data discs.  In other
	situations, the type of media may be important - for example, DVD media is not valid if
	you're writing an audio CD.
	
	When the call completes, there is blank media in the drive and we will, if possible,
	have a reservation on the media (so nobody else can burn to it or grab it out from
	underneath us).
*/
void
druPromptForBlankMediaInDevice(DRDeviceRef device)
{
	DRNotificationCenterRef	notificationCenter = NULL;
	CFRunLoopSourceRef		source = NULL;
	
	/* Indicate our interest in acquiring media reservations.  This is reference counted
		and sticky, reservations will be acquired on our behalf (when possible) until a
		corresponding call to DRDeviceReleaseMediaReservation is made.  We do not make
		that call, its up to our caller to decide if/when its appropriate. */
	DRDeviceAcquireMediaReservation(device);
	
	/* If the device contains blank media right now, then we're done. */
	if (druDeviceContainsBlankMedia(device))
		return;
	
	/* Display a prompt. */
    CFDictionaryRef deviceInfo = DRDeviceCopyInfo((DRDeviceRef)device);
    printf("Please insert blank media in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
	
	/* Open the tray (and eject existing media, if any). */
	/* This call may or may not work - the media in the device may be busy
		and can't be unmounted, or the device may not even have a tray (slot-load
		drives are an example of this).  However, we don't really care; this is just
		a convenience to the user and will do the right thing if the right thing
		can be done. */
	/* We also don't want to eject the media if it's still spinning up -
		the user may have just inserted it, and it takes a good 5-10 seconds
		on some drives for discs to be recognized. */
	if (!druDeviceIsBecomingReady(device))
		DRDeviceEjectMedia(device);
	
	/* Sign up for device status notifications, and enter a tiny runloop so that we can avoid polling. */
	notificationCenter = DRNotificationCenterCreate();
	source = DRNotificationCenterCreateRunLoopSource(notificationCenter);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	DRNotificationCenterAddObserver(notificationCenter, NULL, druWaitForBlankMedia, NULL, device);
	CFRunLoopRun();
	CFRunLoopSourceInvalidate(source);
	
	/* Clean up memory and we're done. */
	if (notificationCenter != NULL)	CFRelease(notificationCenter);
	if (source != NULL)				CFRelease(source);
}



/*
	druPromptForErasableMediaInDevice
	
	Interactively prompts for erasable media in a particular device.  The type of media 
	is not considered.
	
	When the call completes, there is erasable media in the drive and we have a reservation
	on the media (so nobody else can burn to it or grab it out from underneath us).
*/
void
druPromptForErasableMediaInDevice(DRDeviceRef device)
{
	DRNotificationCenterRef	notificationCenter = NULL;
	CFRunLoopSourceRef		source = NULL;
	
	/* Indicate our interest in acquiring media reservations.  This is reference counted
		and sticky, reservations will be acquired on our behalf (when possible) until a
		corresponding call to DRDeviceReleaseMediaReservation is made.  We do not make
		that call, its up to our caller to decide if/when its appropriate. */
	DRDeviceAcquireMediaReservation(device);
	
	/* If the device contains erasable media right now, then we're done. */
	if (druDeviceContainsErasableMedia(device))
		return;
	
	/* Display a prompt. */
    CFDictionaryRef deviceInfo = DRDeviceCopyInfo((DRDeviceRef)device);    
	printf("Please insert erasable media in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
	
	/* Open the tray (and eject existing media, if any). */
	/* This call may or may not work - the media in the device may be busy
		and can't be unmounted, or the device may not even have a tray (slot-load
		drives are an example of this).  However, we don't really care; this is just
		a convenience to the user and will do the right thing if the right thing
		can be done. */
	/* We also don't want to eject the media if it's still spinning up -
		the user may have just inserted it, and it takes a good 5-10 seconds
		on some drives for discs to be recognized. */
	if (!druDeviceIsBecomingReady(device))
		DRDeviceEjectMedia(device);
	
	/* Sign up for device status notifications, and enter a tiny runloop so that we can avoid polling. */
	notificationCenter = DRNotificationCenterCreate();
	source = DRNotificationCenterCreateRunLoopSource(notificationCenter);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	DRNotificationCenterAddObserver(notificationCenter, NULL, druWaitForErasableMedia, NULL, device);
	CFRunLoopRun();
	CFRunLoopSourceInvalidate(source);
	
	/* Clean up memory and we're done. */
	if (notificationCenter != NULL)	CFRelease(notificationCenter);
	if (source != NULL)				CFRelease(source);
}



/*
	druPromptForOverwritableMediaInDevice
	
	Interactively prompts for overwritable media in a particular device.  The type of media 
	is not considered.
	
	When the call completes, there is overwritable media in the drive and we have a reservation
	on the media (so nobody else can burn to it or grab it out from underneath us).
*/
void
druPromptForOverwritableMediaInDevice(DRDeviceRef device)
{
	DRNotificationCenterRef	notificationCenter = NULL;
	CFRunLoopSourceRef		source = NULL;
	
	/* Indicate our interest in acquiring media reservations.  This is reference counted
		and sticky, reservations will be acquired on our behalf (when possible) until a
		corresponding call to DRDeviceReleaseMediaReservation is made.  We do not make
		that call, its up to our caller to decide if/when its appropriate. */
	DRDeviceAcquireMediaReservation(device);
	
	/* If the device contains overwritable media right now, then we're done. */
	if (druDeviceContainsOverwritableMedia(device))
		return;
	
	/* Display a prompt. */
    CFDictionaryRef deviceInfo = DRDeviceCopyInfo((DRDeviceRef)device);    
	printf("Please insert blank or erasable media in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
	
	/* Open the tray (and eject existing media, if any). */
	/* This call may or may not work - the media in the device may be busy
		and can't be unmounted, or the device may not even have a tray (slot-load
		drives are an example of this).  However, we don't really care; this is just
		a convenience to the user and will do the right thing if the right thing
		can be done. */
	/* We also don't want to eject the media if it's still spinning up -
		the user may have just inserted it, and it takes a good 5-10 seconds
		on some drives for discs to be recognized. */
	if (!druDeviceIsBecomingReady(device))
		DRDeviceEjectMedia(device);
	
	/* Sign up for device status notifications, and enter a tiny runloop so that we can avoid polling. */
	notificationCenter = DRNotificationCenterCreate();
	source = DRNotificationCenterCreateRunLoopSource(notificationCenter);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	DRNotificationCenterAddObserver(notificationCenter, NULL, druWaitForOverwritableMedia, NULL, device);
	CFRunLoopRun();
	CFRunLoopSourceInvalidate(source);
	
	/* Clean up memory and we're done. */
	if (notificationCenter != NULL)	CFRelease(notificationCenter);
	if (source != NULL)				CFRelease(source);
}




/*
	druPromptForAppendableMediaInDevice
	
	Interactively prompts for appendable media in a particular device.  The type of media 
	is not considered.
	
	When the call completes, there is overwritable media in the drive and we have a reservation
	on the media (so nobody else can burn to it or grab it out from underneath us).
 */
void
druPromptForAppendableMediaInDevice(DRDeviceRef device)
{
	DRNotificationCenterRef	notificationCenter = NULL;
	CFRunLoopSourceRef		source = NULL;
	
	/* Indicate our interest in acquiring media reservations.  This is reference counted
		and sticky, reservations will be acquired on our behalf (when possible) until a
		corresponding call to DRDeviceReleaseMediaReservation is made.  We do not make
		that call, its up to our caller to decide if/when its appropriate. */
	DRDeviceAcquireMediaReservation(device);
	
	/* If the device contains overwritable media right now, then we're done. */
	if (druDeviceContainsAppendableMedia(device))
		return;
	
	/* Display a prompt. */
    CFDictionaryRef deviceInfo = DRDeviceCopyInfo((DRDeviceRef)device);    
	printf("Please insert blank or appendable media in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
	
	/* Open the tray (and eject existing media, if any). */
	/* This call may or may not work - the media in the device may be busy
		and can't be unmounted, or the device may not even have a tray (slot-load
																		drives are an example of this).  However, we don't really care; this is just
		a convenience to the user and will do the right thing if the right thing
		can be done. */
	/* We also don't want to eject the media if it's still spinning up -
		the user may have just inserted it, and it takes a good 5-10 seconds
		on some drives for discs to be recognized. */
	if (!druDeviceIsBecomingReady(device))
		DRDeviceEjectMedia(device);
	
	/* Sign up for device status notifications, and enter a tiny runloop so that we can avoid polling. */
	notificationCenter = DRNotificationCenterCreate();
	source = DRNotificationCenterCreateRunLoopSource(notificationCenter);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	DRNotificationCenterAddObserver(notificationCenter, NULL, druWaitForAppendableMedia, NULL, device);
	CFRunLoopRun();
	CFRunLoopSourceInvalidate(source);
	
	/* Clean up memory and we're done. */
	if (notificationCenter != NULL)	CFRelease(notificationCenter);
	if (source != NULL)				CFRelease(source);
}



/*
	druDeviceContainsMedia
	
	Returns TRUE if the device contains media.
 */
int
druDeviceContainsMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
		result = 1;
	
	CFRelease(deviceStatus);

	return result;
}



/*
	druDeviceContainsBlankMedia
	
	Returns TRUE if the device contains blank media.
*/
int
druDeviceContainsBlankMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	blank = (CFBooleanRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsBlankKey);
		CFBooleanRef	appendable = (CFBooleanRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsAppendableKey);
		
		/* There's media, but is it blank and writable? */
		if (blank != NULL && CFBooleanGetValue(blank) && appendable != NULL && CFBooleanGetValue(appendable))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDeviceContainsErasableMedia
	
	Returns TRUE if the device contains erasable media.
*/
int
druDeviceContainsErasableMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	erasable = (CFBooleanRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsErasableKey);
		
		/* There's media, but is it erasable? */
		if (erasable != NULL && CFBooleanGetValue(erasable))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDeviceContainsOverwritableMedia
	
	Returns TRUE if the device contains overwritable media.
*/
int
druDeviceContainsOverwritableMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	overwritable = (CFBooleanRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsOverwritableKey);
		
		/* There's media, but is it overwritable? */
		if (overwritable != NULL && CFBooleanGetValue(overwritable))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDeviceContainsAppendableMedia
	
	Returns TRUE if the device contains appendable media.
 */
int
druDeviceContainsAppendableMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	overwritable = (CFBooleanRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsAppendableKey);
		
		/* There's media, but is it overwritable? */
		if (overwritable != NULL && CFBooleanGetValue(overwritable))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDeviceContainsNonBlankMedia
	
	Returns TRUE if the device contains non-blank media.
*/
int
druDeviceContainsNonBlankMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	blank = (CFBooleanRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsBlankKey);
		
		/* There's media, but is it non-blank? */
		if (blank != NULL && !CFBooleanGetValue(blank))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDeviceContainsNonErasableMedia
	
	Returns TRUE if the device contains non-erasable media.
*/
int
druDeviceContainsNonErasableMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	erasable = (CFBooleanRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsErasableKey);
		
		/* There's media, but is it non-erasable? */
		if (erasable != NULL && !CFBooleanGetValue(erasable))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDeviceContainsNonOverwritableMedia
	
	Returns TRUE if the device contains non-overwritable media.
*/
int
druDeviceContainsNonOverwritableMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	overwritable = (CFBooleanRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsOverwritableKey);
		
		/* There's media, but is it non-erasable? */
		if (overwritable != NULL && !CFBooleanGetValue(overwritable))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}
/*
	druDeviceContainsNonAppendableMedia
	
	Returns TRUE if the device contains non-appendable media.
 */
int
druDeviceContainsNonAppendableMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	overwritable = (CFBooleanRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsAppendableKey);
		
		/* There's media, but is it non-erasable? */
		if (overwritable != NULL && !CFBooleanGetValue(overwritable))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDeviceIsBecomingReady
	
	Returns TRUE if the device is becoming ready (eg, spinning up).
*/
int
druDeviceIsBecomingReady(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result;
	
	result = (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateInTransition)) ? 1:0;
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDisplayDeviceList
	
	Displays a list of devices, prefixed by their numeric index in the array.
*/
void
druDisplayDeviceList(CFArrayRef deviceArray)
{
	CFIndex		i, deviceCount = CFArrayGetCount(deviceArray);
	for (i=0;i<deviceCount;++i)
	{
		DRDeviceRef		thisDevice = (DRDeviceRef)CFArrayGetValueAtIndex(deviceArray,i);
		char			description[100];
		printf("%2d) %s\n", (int)(i+1), druGetDeviceDescription(thisDevice,description,sizeof(description)));
	}
}




/*
	druGetDeviceDescription
	
	Fills a character buffer with a device's normal description: VENDOR PRODUCT via BUS.
	The incoming buffer is returned as a convenience.
*/
char *
druGetDeviceDescription(DRDeviceRef device, char *buffer, size_t bufSize)
{
	CFDictionaryRef	deviceInfo = DRDeviceCopyInfo(device);
	CFStringRef		bus = (CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDevicePhysicalInterconnectKey);
	CFStringRef		desc;
	CFIndex			len = 0;
	
	#if 1	/* for now, until the bus starts getting returned in ASCII */
	if (CFEqual(bus,kDRDevicePhysicalInterconnectFireWire))		bus = CFSTR("FireWire");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectUSB))		bus = CFSTR("USB");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectATAPI))	bus = CFSTR("ATAPI");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectSCSI))	bus = CFSTR("SCSI");
	else														bus = CFSTR("unknown interface");
	#endif
	
	desc = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@ %@ via %@"),
			CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),
			CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),
			bus);
	CFStringGetBytes(desc, CFRangeMake(0,CFStringGetLength(desc)), kCFStringEncodingASCII,
					'.', false, (UInt8*)buffer, bufSize-1, &len);
	buffer[len] = 0;
	
	CFRelease(deviceInfo);
	CFRelease(desc);
	return buffer;
}


/*
	druGetDeviceShortDescription
	
	Fills a character buffer with a device's short description: VENDOR PRODUCT.
	The incoming buffer is returned as a convenience.
*/
char *
druGetDeviceShortDescription(DRDeviceRef device, char *buffer, size_t bufSize)
{
	CFDictionaryRef	deviceInfo = DRDeviceCopyInfo(device);
	CFStringRef		desc = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@ %@"),
							CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),
							CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey));
	CFIndex			len = 0;
	
	CFStringGetBytes(desc, CFRangeMake(0,CFStringGetLength(desc)), kCFStringEncodingASCII,
					'.', false, (UInt8*)buffer, bufSize-1, &len);
	buffer[len] = 0;
	
	CFRelease(deviceInfo);
	CFRelease(desc);
	return buffer;
}


/*
	dru_getlongdevicedescription
	
	Fills a character buffer with a device's long description: VENDOR PRODUCT (FIRMWARE) via BUS.
	The incoming buffer is returned as a convenience.
*/
char *
druGetDeviceLongDescription(DRDeviceRef device, char *buffer, size_t bufSize)
{
	CFDictionaryRef	deviceInfo = DRDeviceCopyInfo(device);
	CFStringRef		bus = (CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDevicePhysicalInterconnectKey);
	CFStringRef		desc;
	CFIndex			len = 0;
	
	CFDictionaryRef deviceStatus = DRDeviceCopyStatus(device);
	CFDictionaryRef mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
	CFStringRef		media = CFSTR("No Media");
	if (mediaInfo != NULL)
	{
		media = (CFStringRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaTypeKey);
		if (CFEqual(media,kDRDeviceMediaTypeCDROM))						media = CFSTR("CD-ROM");
		else if (CFEqual(media,kDRDeviceMediaTypeCDR))					media = CFSTR("CD-R");
		else if (CFEqual(media,kDRDeviceMediaTypeCDRW))					media = CFSTR("CD-RW");
		else if (CFEqual(media,kDRDeviceMediaTypeDVDROM))				media = CFSTR("DVD-ROM");
		else if (CFEqual(media,kDRDeviceMediaTypeDVDRAM))				media = CFSTR("DVD-RAM");
		else if (CFEqual(media,kDRDeviceMediaTypeDVDR))					media = CFSTR("DVD-R");
		else if (CFEqual(media,kDRDeviceMediaTypeDVDRW))				media = CFSTR("DVD-RW");
		else if (CFEqual(media,kDRDeviceMediaTypeDVDPlusR))				media = CFSTR("DVD+R");
		else if (CFEqual(media,kDRDeviceMediaTypeDVDPlusRDoubleLayer))	media = CFSTR("DVD+R DL");
		else if (CFEqual(media,kDRDeviceMediaTypeDVDPlusRW))			media = CFSTR("DVD+RW");
		else															media = CFSTR("unknown media");
	}
	CFRelease(deviceStatus);

	#if 1	/* for now, until the bus starts getting returned in ASCII */
	if (CFEqual(bus,kDRDevicePhysicalInterconnectFireWire))		bus = CFSTR("FireWire");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectUSB))		bus = CFSTR("USB");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectATAPI))	bus = CFSTR("ATAPI");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectSCSI))	bus = CFSTR("SCSI");
	else														bus = CFSTR("unknown interface");
	#endif
	
	desc = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@ %@ (%@) via %@, %@"),
			CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),
			CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),
			CFDictionaryGetValue(deviceInfo,kDRDeviceFirmwareRevisionKey),
			bus, media);
	CFStringGetBytes(desc, CFRangeMake(0,CFStringGetLength(desc)), kCFStringEncodingASCII,
					'.', false, (UInt8*)buffer, bufSize-1, &len);
	buffer[len] = 0;
	
	CFRelease(deviceInfo);
	CFRelease(desc);
	return buffer;
}





#pragma mark -



/*
	druMediaIsReserved
	
	Returns TRUE if the device contains blank media.
*/
int
druMediaIsReserved(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = (CFStringRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	reserved = (CFBooleanRef)CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsReservedKey);
		
		/* There's media, but do we have the reservation? */
		if (reserved != NULL && CFBooleanGetValue(reserved))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druWaitForMedia
	
	DRNotificationCallback to wait for blank media.
 */
void
druWaitForMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info)
{
#pragma unused(center, info, observer)
	DRDeviceRef	device = (DRDeviceRef)object;
	
	/* The device may have been unplugged - check for that. */
	if (CFEqual(name,kDRDeviceDisappearedNotification) || !DRDeviceIsValid(device))
	{
		printf("Aborted. (device disconnected)\n");
		exit(1);
	}
	
	/* If the device status changed... */
	if (CFEqual(name,kDRDeviceStatusChangedNotification))
	{
		/* And if there's media now.... */
		if (druDeviceContainsMedia(device))
		{
			/* Then stop the runloop. */
			CFRunLoopStop(CFRunLoopGetCurrent());
			return;
		}
	}
}



/*
	druWaitForBlankMedia
	
	DRNotificationCallback to wait for blank media.
*/
void
druWaitForBlankMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info)
{
#pragma unused(center, info, observer)
	DRDeviceRef	device = (DRDeviceRef)object;
	
	/* The device may have been unplugged - check for that. */
	if (CFEqual(name,kDRDeviceDisappearedNotification) || !DRDeviceIsValid(device))
	{
		printf("Aborted. (device disconnected)\n");
		exit(1);
	}
	
	/* If the device status changed... */
	if (CFEqual(name,kDRDeviceStatusChangedNotification))
	{
		/* And if there's blank media now.... */
		if (druDeviceContainsBlankMedia(device))
		{
			/* Then stop the runloop. */
			CFRunLoopStop(CFRunLoopGetCurrent());
			return;
		}
		
		/* Otherwise if there's non-blank media... */
		if (druDeviceContainsNonBlankMedia(device))
		{
			/* Warn of unsuitable media, then open the tray and eject it */
			CFDictionaryRef deviceInfo = DRDeviceCopyInfo(device);
			printf("Media is not blank in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
			
			/* This call may or may not work - the media in the device may be busy
				and can't be unmounted, or the device may not even have a tray (slot-load
				drives are an example of this).  However, we don't really care; this is just
				a convenience to the user and will do the right thing if the right thing
				can be done. */
			DRDeviceEjectMedia(device);
			
			/* Redisplay the prompt. */
			printf("Please insert blank media in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
		}
	}
}



/*
	druWaitForErasableMedia
	
	DRNotificationCallback to wait for erasable media.
*/
void
druWaitForErasableMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info)
{
#pragma unused(center, info, observer)
	DRDeviceRef	device = (DRDeviceRef)object;
	
	/* The device may have been unplugged - check for that. */
	if (CFEqual(name,kDRDeviceDisappearedNotification) || !DRDeviceIsValid(device))
	{
		printf("Aborted. (device disconnected)\n");
		exit(1);
	}
	
	/* If the device status changed... */
	if (CFEqual(name,kDRDeviceStatusChangedNotification))
	{
		/* And if there's erasable media now.... */
		if (druDeviceContainsErasableMedia(device))
		{
			/* Then stop the runloop. */
			CFRunLoopStop(CFRunLoopGetCurrent());
			return;
		}
		
		/* Otherwise if there's non-erasable media... */
		if (druDeviceContainsNonErasableMedia(device))
		{
			/* Warn of unsuitable media, then open the tray and eject it */
			CFDictionaryRef deviceInfo = DRDeviceCopyInfo(device);
			printf("Media is not erasable in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
			
			/* This call may or may not work - the media in the device may be busy
				and can't be unmounted, or the device may not even have a tray (slot-load
				drives are an example of this).  However, we don't really care; this is just
				a convenience to the user and will do the right thing if the right thing
				can be done. */
			DRDeviceEjectMedia(device);
			
			/* Redisplay the prompt. */
			printf("Please insert erasable media in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
		}
	}
}



/*
	druWaitForOverwritableMedia
	
	DRNotificationCallback to wait for overwritable media.
*/
void
druWaitForOverwritableMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info)
{
#pragma unused(center, info, observer)
	DRDeviceRef	device = (DRDeviceRef)object;
	
	/* The device may have been unplugged - check for that. */
	if (CFEqual(name,kDRDeviceDisappearedNotification) || !DRDeviceIsValid(device))
	{
		printf("Aborted. (device disconnected)\n");
		exit(1);
	}
	
	/* If the device status changed... */
	if (CFEqual(name,kDRDeviceStatusChangedNotification))
	{
		/* And if there's overwritable media now.... */
		if (druDeviceContainsOverwritableMedia(device))
		{
			/* Then stop the runloop. */
			CFRunLoopStop(CFRunLoopGetCurrent());
			return;
		}
		
		/* Otherwise if there's non-overwritable media... */
		if (druDeviceContainsNonOverwritableMedia(device))
		{
			/* Warn of unsuitable media, then open the tray and eject it */
			CFDictionaryRef deviceInfo = DRDeviceCopyInfo(device);
			printf("Media is not blank or erasable in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
			
			/* This call may or may not work - the media in the device may be busy
				and can't be unmounted, or the device may not even have a tray (slot-load
				drives are an example of this).  However, we don't really care; this is just
				a convenience to the user and will do the right thing if the right thing
				can be done. */
			DRDeviceEjectMedia(device);
			
			/* Redisplay the prompt. */
			printf("Please insert blank or erasable media in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
		}
	}
}
/*
	druWaitForAppendableMedia
	
	DRNotificationCallback to wait for appendable media.
 */
void
druWaitForAppendableMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info)
{
#pragma unused(center, info, observer)
	DRDeviceRef	device = (DRDeviceRef)object;
	
	/* The device may have been unplugged - check for that. */
	if (CFEqual(name,kDRDeviceDisappearedNotification) || !DRDeviceIsValid(device))
	{
		printf("Aborted. (device disconnected)\n");
		exit(1);
	}
	
	/* If the device status changed... */
	if (CFEqual(name,kDRDeviceStatusChangedNotification))
	{
		/* And if there's overwritable media now.... */
		if (druDeviceContainsAppendableMedia(device))
		{
			/* Then stop the runloop. */
			CFRunLoopStop(CFRunLoopGetCurrent());
			return;
		}
		
		/* Otherwise if there's non-overwritable media... */
		if (druDeviceContainsNonAppendableMedia(device))
		{
			/* Warn of unsuitable media, then open the tray and eject it */
			CFDictionaryRef deviceInfo = DRDeviceCopyInfo(device);
			printf("Media is not blank or appendable in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
			
			/* This call may or may not work - the media in the device may be busy
				and can't be unmounted, or the device may not even have a tray (slot-load
																				drives are an example of this).  However, we don't really care; this is just
				a convenience to the user and will do the right thing if the right thing
				can be done. */
			DRDeviceEjectMedia(device);
			
			/* Redisplay the prompt. */
			printf("Please insert blank or appendable media in %s %s.\n",CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),CFStringGetSystemEncoding()), CFStringGetCStringPtr((CFStringRef)CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),CFStringGetSystemEncoding()));
		}
	}
}



int
druFilter_AnyBurner(DRDeviceRef device)
{
	CFDictionaryRef		info = DRDeviceCopyInfo(device);
	CFDictionaryRef		capabilities = (CFDictionaryRef)CFDictionaryGetValue(info,kDRDeviceWriteCapabilitiesKey);
	
	int	result = (CFDictionaryGetValue(capabilities,kDRDeviceCanWriteKey) == kCFBooleanTrue);
	
	CFRelease(info);
	return result;
}


int
druFilter_AnyEraser(DRDeviceRef device)
{
	CFDictionaryRef		info = DRDeviceCopyInfo(device);
	CFDictionaryRef		capabilities = (CFDictionaryRef)CFDictionaryGetValue(info,kDRDeviceWriteCapabilitiesKey);
	
	int	result = (CFDictionaryGetValue(capabilities,kDRDeviceCanWriteKey) == kCFBooleanTrue);
	
	CFRelease(info);
	return result;
}



int
druFilter_CDBurners(DRDeviceRef device)
{
	CFDictionaryRef		info = DRDeviceCopyInfo(device);
	CFDictionaryRef		capabilities = (CFDictionaryRef)CFDictionaryGetValue(info,kDRDeviceWriteCapabilitiesKey);
	
	int	result = (CFDictionaryGetValue(capabilities,kDRDeviceCanWriteCDKey) == kCFBooleanTrue);
	
	CFRelease(info);
	return result;
}



int
druFilter_DVDBurners(DRDeviceRef device)
{
	CFDictionaryRef		info = DRDeviceCopyInfo(device);
	CFDictionaryRef		capabilities = (CFDictionaryRef)CFDictionaryGetValue(info,kDRDeviceWriteCapabilitiesKey);
	
	int	result = (CFDictionaryGetValue(capabilities,kDRDeviceCanWriteDVDKey) == kCFBooleanTrue);
	
	CFRelease(info);
	return result;
}

