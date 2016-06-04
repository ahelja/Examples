/*
	dru_devices.h
	
	Part of the Disc Recording Utility sources for command-line tools.  This
	code provides an example of prompting the user to select a device and/or
	insert media, and how to create a textual description of a device.
*/
/*
 File:  dru_devices.h
 
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

#ifndef _H_dru_devices
#define _H_dru_devices

#include <DiscRecording/DiscRecording.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Filter proc returns true to allow, false to suppress */
typedef int (*druDeviceFilterProc)(DRDeviceRef device);
int druFilter_AnyBurner(DRDeviceRef device);
int druFilter_AnyEraser(DRDeviceRef device);
int druFilter_CDBurners(DRDeviceRef device);
int druFilter_DVDBurners(DRDeviceRef device);

/* Displays a prompt asking for a device. Optional filter to constrain the choices. */
DRDeviceRef	druPromptForDevice(char *promptString, druDeviceFilterProc filter);

/* Displays a prompt asking for media in a device. */
void		druPromptForMediaInDevice(DRDeviceRef device);

/* Displays a prompt asking for blank media in a device. */
void		druPromptForBlankMediaInDevice(DRDeviceRef device);

/* Displays a prompt asking for erasable media in a device. */
void		druPromptForErasableMediaInDevice(DRDeviceRef device);

/* Displays a prompt asking for overwritable media in a device. */
void		druPromptForOverwritableMediaInDevice(DRDeviceRef device);

/* Displays a prompt asking for appendable media in a device. */
void		druPromptForAppendableMediaInDevice(DRDeviceRef device);

/* Displays a list of devices. */
void		druDisplayDeviceList(CFArrayRef deviceArray);

/* Returns true if the device contains media. */
int			druDeviceContainsMedia(DRDeviceRef device);

/* Returns true if the device contains blank media. */
int			druDeviceContainsBlankMedia(DRDeviceRef device);

/* Returns true if the device contains erasable media. */
int			druDeviceContainsErasableMedia(DRDeviceRef device);

/* Returns true if the device contains overwritable media. */
int			druDeviceContainsOverwritableMedia(DRDeviceRef device);

/* Returns true if the device contains appendable media. */
int			druDeviceContainsAppendableMedia(DRDeviceRef device);

/* Returns true if the device contains non-blank media. */
int			druDeviceContainsNonBlankMedia(DRDeviceRef device);

/* Returns true if the device contains non-erasable media. */
int			druDeviceContainsNonErasableMedia(DRDeviceRef device);

/* Returns true if the device contains non-overwritable media. */
int			druDeviceContainsNonOverwritableMedia(DRDeviceRef device);

/* Returns true if the device contains non-appendable media. */
int			druDeviceContainsNonAppendableMedia(DRDeviceRef device);

/* Returns true if the device is spinning up (ie, the tray was just closed) */
int			druDeviceIsBecomingReady(DRDeviceRef device);

/* Returns true if the media in the device is reserved for this processes use */
int			druMediaIsReserved(DRDeviceRef device);

/* Returns a standard device description - VENDOR PRODUCT via BUS */
char *		druGetDeviceDescription(DRDeviceRef device, char *buffer, size_t bufSize);

/* Returns a short device description - VENDOR PRODUCT */
char *		druGetDeviceShortDescription(DRDeviceRef device, char *buffer, size_t bufSize);

/* Returns a long device description - VENDOR PRODUCT (FIRMWARE) via BUS */
char *		druGetDeviceLongDescription(DRDeviceRef device, char *buffer, size_t bufSize);


#ifdef __cplusplus
}
#endif

#endif /* _H_dru_devices */

