/*
 File:  drioctl.h
 
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
#ifndef _H_drioctl
#include <DiscRecording/DiscRecording.h>

/* Shell functions */
void ioctl_shell(DRDeviceRef device);
void ioctl_shell_interpret(DRDeviceRef device, char *line);
void ioctl_shell_dispatch(DRDeviceRef device,int argc,char **argv);

/* Helper functions */
CFTypeRef CopyDeviceMediaProperty(DRDeviceRef device, CFStringRef property);
int DeviceHasMediaOfClass(DRDeviceRef device,CFStringRef class);
int GetDiskNodeName(DRDeviceRef device, char *nameBuffer, size_t nameBufferSize);
int GetDiskNodePath(DRDeviceRef device, char *pathBuffer, size_t pathBufferSize);
int OpenDiskNode(DRDeviceRef device);
int ioctlWithDRDevice(DRDeviceRef device,unsigned long request,char *argp);
void hexdump(u_int8_t *buffer,u_int32_t bufferSize);

/* Commands */
void PrintHelp(DRDeviceRef device, int argc, char **argv);
void PrintDevice(DRDeviceRef device, int argc, char **argv);
void PrintVolumes(DRDeviceRef device, int argc, char **argv);
void GetSpeed(DRDeviceRef device, int argc, char **argv);
void SetSpeed(DRDeviceRef device, int argc, char **argv);
void ReadDiscInfo(DRDeviceRef device, int argc, char **argv);
void ReadTrackInfo(DRDeviceRef device, int argc, char **argv);
void ReadCD(DRDeviceRef device, int argc, char **argv);
void ReadTOC(DRDeviceRef device, int argc, char **argv);
void ReadMCN(DRDeviceRef device, int argc, char **argv);
void ReadISRC(DRDeviceRef device, int argc, char **argv);
void ExitShell(DRDeviceRef device,int argc, char **argv);

#endif /* _H_drioctl */
