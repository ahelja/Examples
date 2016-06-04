/*
 File:  main.c
 
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
#include <glob.h>
#include "dru_devices.h"
#include "dru_burning.h"


int main(int argc, char *argv[]);



int main(int argc, char *argv[])
{
#pragma unused(argc, argv)
	DRDeviceRef		device = NULL;
	char			buffer[80];
	int				fullErase = 0;
	
	/* Hello world! */
	printf("Bulk Erase\n");
	printf("\n");
	printf("  This tool will erase discs in bulk.  When an erasable disc is\n");
	printf("  inserted into the selected drive, it is automatically erased\n");
	printf("  and then ejected.  Loops forever until killed - hit ^C to exit.\n");
	printf("\n");
	
	/* First, use DRU to prompt the user to pick a device.
		If there's only one device to choose, the selection is automatic. */
	device = druPromptForDevice(NULL,druFilter_AnyEraser);
	
	/* Print out a description of the device. */
	printf("Selected device: %s\n", druGetDeviceDescription(device,buffer,sizeof(buffer)));
	printf("\n");
	
	/* Next, ask the user if they want to do quick or full erases. */
	printf("Do you want to perform quick or full erases? \n");
	printf("\n");
	printf("  A quick erase finishes quickly, but does not erase the entire\n");
	printf("  disc.  Sometimes one drive may be able to read a disc that\n");
	printf("  another has quick-erased.  Quick erases are useful if you are\n");
	printf("  going to re-burn the disc immediately, or when data security is\n");
	printf("  not a concern.\n");
	printf("\n");
	printf("  A full erase takes a lot longer to complete, sometimes as much as\n");
	printf("  20-40 minutes or more, but the entire disc is erased.  Fully-erased\n");
	printf("  discs are effectively like new and can be stored for later use or used\n");
	printf("  in a different drive.\n");
	printf("\n");
	printf("Please select (Q)uick or (F)ull [default is Quick]: ");
	fflush(stdout);
	fgets(buffer,sizeof(buffer),stdin);
	if (buffer[0] == 'f' || buffer[0] == 'F')
		fullErase = 1;
	printf("%s erase selected.\n", fullErase ? "Full":"Quick");
	
	if (DRDeviceAcquireExclusiveAccess(device) == noErr)
	{
		/* Loop forever, until the user kills our process. */
		while (1)
		{
			/* Use DRU to prompt the user to insert erasable media. */
			printf("\n");
			druPromptForErasableMediaInDevice(device);
			
			/* Time to erase.  DRU automatically handles the erase and progress. */
			druErase(device,fullErase);
			
			/* Eject the media. */
			DRDeviceEjectMedia(device);
		}
	}
	else
	{
		printf("The device is in use by another application.\n");
	}
	
	/* Clean up after ourselves. This code never runs because of
		the infinite loop above, but is displayed here for completeness. */
	if (device != NULL)	
	{
		DRDeviceReleaseExclusiveAccess(device);
		CFRelease(device);
	}
	
	return 0;
}

