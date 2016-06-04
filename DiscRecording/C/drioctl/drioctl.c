/*
 File:  drioctl.c
 
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
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOCDMediaBSDClient.h>
#include <IOKit/storage/IODVDMediaBSDClient.h>
#include <IOKit/storage/IOCDTypes.h>
#include <IOKit/storage/IODVDTypes.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "dru_devices.h"
#include "drioctl.h"


/* Defined limits for parsing. */
#define INPUT_LINE_MAX_CHARS		1024
#define INPUT_LINE_MAX_ARGS			10

/* Command table */
typedef void (*ioctl_command_entry)(DRDeviceRef device,int argc,char **argv);
struct ioctl_command_table_entry
{
	const char *		command;
	ioctl_command_entry	proc;
	const char *		description;
};

const struct ioctl_command_table_entry	kCommandTable[] = {
	{ "help",			PrintHelp,			"Displays this help." },
	{ "device",			PrintDevice,		"Prints a description of the targeted device." },
	{ "volumes",		PrintVolumes,		"Shows what volumes, if any, are mounted from the device." },
	{ "getSpeed",		GetSpeed,			"Reads current device speed." },
	{ "setSpeed",		SetSpeed,			"Changes current device speed." },
	{ "discInfo",		ReadDiscInfo,		"Reads disc information." },
	{ "trackInfo",		ReadTrackInfo,		"Reads track/rzone information." },
	{ "readCD",			ReadCD,				"(CD media) Reads a block and prints the result to stdout." },
	{ "toc",			ReadTOC,			"(CD media) Prints the CD table of contents." },
	{ "mcn",			ReadMCN,			"(CD media) Prints the disc MCN." },
	{ "isrc",			ReadISRC,			"(CD media) Prints a single track's ISRC." },
	{ "exit",			ExitShell,			"Terminates the shell." },
	
	/* Synonyms */
	{ "quit",			ExitShell,			NULL },
	{ "readDiscInfo",	ReadDiscInfo,		NULL },
	{ "readTrackinfo",	ReadTrackInfo,		NULL },
	{ "readRZoneInfo",	ReadTrackInfo,		NULL },
	{ "rzoneInfo",		ReadTrackInfo,		NULL },
	{ "readTOC",		ReadTOC,			NULL }
};

const int kNumberOfCommands = (int)(sizeof(kCommandTable) / sizeof(struct ioctl_command_table_entry));


#if 0
#pragma mark -
#endif



/* --------------------------------------------------------------
	main
   --------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
	DRDeviceRef		device = NULL;
	
	/* Hello world! */
	printf("DiscRecording ioctl Explorer\n\n");
	
	/* Prompt the user to pick a device.
		If there's only one device to choose, the selection is automatic.
		We allow any CD device, not just writers.
	*/
	device = druPromptForDevice(NULL,NULL);
	
	/* Enter interactive mode. */
	ioctl_shell(device);
	return 0;
}




/* --------------------------------------------------------------
	ioctl_shell
   --------------------------------------------------------------
   Interactive interface for running ioctls on a device.
*/
void
ioctl_shell(
	DRDeviceRef device)
{
	char	buf[INPUT_LINE_MAX_CHARS];
	
	ioctl_shell_interpret(device,"device");
	printf("Enter 'help' to see a list of commands.\n");
	while (1)
	{
		/* Display a prompt. */
		printf("> ");
		fflush(stdout);
		
		/* Read a line of input. */
		buf[0] = 0;
		fgets(buf,sizeof(buf),stdin);
		
		/* Interpret the line. */
		ioctl_shell_interpret(device,buf);
	}
}



/* --------------------------------------------------------------
	ioctl_shell_interpret
   --------------------------------------------------------------
   Interprets a single ioctl_shell command.
*/
void
ioctl_shell_interpret(
	DRDeviceRef device,
	char *buf)
{
	char **ap, *argv[INPUT_LINE_MAX_ARGS];
	int	len, argc;
	
	/* Trim trailing newlines, if any. */
	len = strlen(buf);
	while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
		buf[--len] = 0;
	
	/* Tokenize the string. */
	for (ap = argv; (*ap = strsep(&buf," \t")) != NULL;)
		if (**ap != '\0')
			if (++ap >= &argv[INPUT_LINE_MAX_ARGS])
				break;
	argc = (ap - &argv[0]);
	
	/* Dispatch the call. */
	ioctl_shell_dispatch(device,argc,argv);
}


/* --------------------------------------------------------------
	ioctl_shell_dispatch
   --------------------------------------------------------------
   Dispatches a parsed ioctl_shell command.
*/
void
ioctl_shell_dispatch(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	int i;
	
	/* If there were no tokens, exit. */
	if (argc == 0)
		return;
	
	/* Find the command in the table. */
	for (i=0; i<kNumberOfCommands; ++i)
	{
		if (!strcasecmp(argv[0],kCommandTable[i].command))
		{
			(*kCommandTable[i].proc)(device,argc,argv);
			return;
		}
	}
	
	/* Print out an error message for an unrecognized command. */
	fprintf(stderr,"Unrecognized command '%s'.  Enter 'help' to get a list of commands.\n", (char*)argv[0]);
}



#if 0
#pragma mark -
#endif




/* --------------------------------------------------------------
	CopyDeviceMediaProperty
   --------------------------------------------------------------
   Copies a property from the device's media info dictionary.  If
   no media is present or the property is not found, returns NULL.
*/
CFTypeRef
CopyDeviceMediaProperty(
	DRDeviceRef device,
	CFStringRef property)
{
	CFDictionaryRef	status, mediaInfo;
	CFTypeRef value = NULL;
	
	/* Get the device's current status.  This should never return NULL,
		but we check anyway. */
	status = DRDeviceCopyStatus(device);
	if (status != NULL)
	{
		/* Get specific information about the inserted media.  This may
			return NULL if no media is present. */
		mediaInfo = (CFDictionaryRef)CFDictionaryGetValue(status,kDRDeviceMediaInfoKey);
		if (mediaInfo != NULL)
		{
			/* Get the property from the media info dictionary.  This may
				return NULL if the property is not present. */
			value = CFDictionaryGetValue(mediaInfo,property);
			if (value != NULL)
				CFRetain(value);
		}
		
		CFRelease(status);
	}
	
	/* Return the value to the caller. */
	return value;
}



/* --------------------------------------------------------------
	DeviceHasMediaOfClass
   --------------------------------------------------------------
   Returns 1 if the device contains media of the specified class,
   0 otherwise.  Class may be kDRDeviceMediaClassCD or
   kDRDeviceMediaClassDVD.
*/
int
DeviceHasMediaOfClass(
	DRDeviceRef device,
	CFStringRef desiredClass)
{
	CFStringRef	mediaClass;
	int	result = 0;
	
	/* Get the media class.  May return NULL if no media. */
	mediaClass = (CFStringRef)CopyDeviceMediaProperty(device,kDRDeviceMediaClassKey);
	if (mediaClass != NULL)
	{
		/* Compare the result with the desired class. */
		result = CFEqual(mediaClass,desiredClass);
		
		/* Clean up allocated memory. */
		CFRelease(mediaClass);
	}
	
	return result;
}



/* --------------------------------------------------------------
	GetDiskNodeName
   --------------------------------------------------------------
   Gets the BSD disk name for the media in the device. For example: "disk1".
   Return value is a boolean: 1 for success, 0 for failure.
*/
int
GetDiskNodeName(
	DRDeviceRef device,
	char *nameBuffer,
	size_t nameBufferSize)
{
	int result = 0;
	CFStringRef bsdName;
	
	/* Copy BSD name property from the media info dictionary. */
	bsdName = (CFStringRef)CopyDeviceMediaProperty(device,kDRDeviceMediaBSDNameKey);
	if (bsdName != NULL)
	{
		/* Convert the CFString into a filesystem name.  MacOS X
			uses UTF-8 for pathnames. */
		result = (int)CFStringGetCString(bsdName,nameBuffer,
						nameBufferSize,kCFStringEncodingUTF8);
		
		/* Clean up allocated memory. */
		CFRelease(bsdName);
	}

	return result;
}



/* --------------------------------------------------------------
	GetDiskNodePath
   --------------------------------------------------------------
   Gets the full path to the BSD disk node for the media in the
   device. For example: "/dev/disk1".
   
   Return value is a boolean: 1 for success, 0 for failure.
*/
int
GetDiskNodePath(
	DRDeviceRef device,
	char *pathBuffer,
	size_t pathBufferSize)
{
	/* Initialize the buffer with the partial path "/dev/" */
	if (pathBufferSize < 6)
		return 0;
	strcpy(pathBuffer,"/dev/");
	
	/* Append the node name to the end. */
	return GetDiskNodeName(device,pathBuffer+5,pathBufferSize-5);
}



/* --------------------------------------------------------------
	OpenDiskNode
   --------------------------------------------------------------
   Opens a read-only path to the BSD disk node for the media in the
   device.  Return value is the file handle, -1 if the node
   could not be opened, or -2 if no media was present.
*/
int
OpenDiskNode(
	DRDeviceRef device)
{
	char	deviceNode[40];
	
	/* Get the path to the device node. */
	if (GetDiskNodePath(device,deviceNode,sizeof(deviceNode)) == 0)
	{
		errno = ENOENT;
		return -1;
	}
	
	/* Open the node and return the handle. */
	return open(deviceNode,O_RDONLY,0);
}



/* --------------------------------------------------------------
	ioctlWithDRDevice
   --------------------------------------------------------------
   Issues an ioctl to the specified DRDeviceRef.  Calling conventions
   are the same as ioctl(2) except that instead of a file handle
   this routine takes a DRDeviceRef.
   
   ioctls require BSD disk nodes, and BSD disk nodes are lazily
   created on MacOS X, and only exist when media is present.  This
   command will fail with EBADF if no media is present in the device.
*/
int
ioctlWithDRDevice(
	DRDeviceRef device,
	unsigned long request,
	char *argp)
{
	int file, result;
	errno = 0;
	
	/* Open a read-only path to the BSD disk node. */
	file = OpenDiskNode(device);
	if (file < 0) return EBADF;
	
	/* Issue the ioctl. */
	result = ioctl(file,request,argp);
	
	/* Close the BSD disk node. */
	close(file);
	
	/* Return the result of the ioctl. */
	return result;
}



/* --------------------------------------------------------------
	hexdump
   --------------------------------------------------------------
   Dumps a buffer as hex.  Uses the same format as hexdump -C.
*/
void
hexdump(
	u_int8_t	*buffer,
	u_int32_t	bufferSize)
{
	u_int32_t	i,j;
	
	/* Loop through each byte in the buffer. */
	for (i=0; i<bufferSize; ++i)
	{
		/* Line header at the start of every 16 bytes */
		if ((i & 0xF) == 0)
			printf("%08x ",(int)i);
		
		/* Extra space before every set of 8 bytes */
		if ((i & 0x7) == 0)
			printf(" ");
		
		/* Print the byte */
		printf("%02x ", (int)buffer[i]);
		
		/* ASCII summary at the end of every 16 bytes,
			and at the end of all data. */
		if (((i & 0xF) == 0xF) || (i == bufferSize-1))
		{
			/* Pad if we're at the end of all data. */
			for (j=i&0xF; j<0xF; ++j)
				printf((j&0x7) ? "   ":"    ");
			
			/* Print out a separator. */
			printf(" |");
			
			/* Loop through characters on this line */
			for (j=(i&~0xF); j<=i; ++j)
				printf("%c",isprint(buffer[j]) ? (char)buffer[j]:(char)'.');
			
			/* Print out a separator and end-of-line. */
			printf("|\n");
		}
	}
	
	/* Print out the data length. */
	printf("%08x\n", (int)bufferSize);
}


#if 0
#pragma mark -
#endif


/* --------------------------------------------------------------
	PrintHelp
   --------------------------------------------------------------
   Prints a list of commands and what they do.
*/
void
PrintHelp(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	int i;
	
	/* If the user entered "help command [args...]", redispatch it as
		"command help [args...]".  This allows each function to handle
		its own command-specific help. */
	if (argc > 1)
	{
		char *temp = argv[0];
		argv[0] = argv[1];
		argv[1] = temp;
		
		/* Don't re-dispatch to ourselves if the user enters "help help"! */
		if (strcasecmp(argv[0],"help"))
		{
			ioctl_shell_dispatch(device,argc,argv);
			return;
		}
	}
	
	/* Print a table of help. */
	printf("\nAvailable commands:\n");
	for (i=0; i<kNumberOfCommands; ++i)
		if (kCommandTable[i].description != NULL)
			printf("    %-20s%s\n", kCommandTable[i].command, kCommandTable[i].description);
	printf("\n");
	printf("Type 'help command' for more information on a specific command.\n");
}


/* --------------------------------------------------------------
	PrintDevice
   --------------------------------------------------------------
   Prints out a description of the device.
*/
void
PrintDevice(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	char	displayName[256];
	
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
		printf("device\n");
		printf("   Displays a description of the current device.  Warns if the device\n");
		printf("   has been disconnected.\n");
		return;
	}
	
	/* Otherwise, just print the device name. */
	printf("%s%s\n", druGetDeviceDescription(device,displayName,sizeof(displayName)),
		DRDeviceIsValid(device) ? "":" (no longer attached)");
}



/* --------------------------------------------------------------
	PrintVolumes
   --------------------------------------------------------------
*/
void
PrintVolumes(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	char deviceNode[45];
	struct statfs *mntbuf = NULL;
	int i, deviceNodeLen, num = 0, found = 0;
	
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
		printf("volumes\n");
		printf("   Displays a list of the volumes mounted from the media in\n");
		printf("   the device, if any.\n");
		return;
	}
	
	/* Get the BSD name for the media. */
	if (!GetDiskNodePath(device,deviceNode,sizeof(deviceNode)))
	{
		fprintf(stderr,"No media present.\n");
		return;
	}
	deviceNodeLen = strlen(deviceNode);
	
	/* Call getfsstat to get information about all mounted filesystems. */
	num = getfsstat(NULL,0,MNT_NOWAIT);
	mntbuf = (struct statfs *)calloc(num,sizeof(struct statfs));
	num = getfsstat(mntbuf,num * sizeof(struct statfs),MNT_NOWAIT);
	
	/* Loop through the filesystems to determine if any of them match
		the BSD name. */
	for (i=0; i<num; ++i)
	{
		/* This is the dev node that was mounted */
		char *mountedNode = mntbuf[i].f_mntfromname;
		
		/* Check to see if the mounted node has our node path as a prefix. */
		if (!strncmp(mountedNode,deviceNode,deviceNodeLen))
		{
			printf("%s\n", mntbuf[i].f_mntonname);
			found += 1;
		}
	}
	
	/* If no volumes were found, say so. */
	if (found == 0)
		fprintf(stderr,"No volumes mounted.\n");
	
	/* Clean up allocated memory. */
	free(mntbuf);
}



/* --------------------------------------------------------------
	GetSpeed
   --------------------------------------------------------------
*/
void
GetSpeed(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	u_int16_t	speed;
	int ch, showKPS = 1, showXFactor = 1;
	float divisor = 176.4;
	const char *units = NULL;
	unsigned long request;
	
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
	usage:
		printf("getSpeed [-k] [-x]\n");
		printf("   Returns the current read speed of the device.\n");
		printf("     -k   Print speed in KB / s (K=1000)\n");
		printf("     -x   Print speed as an approximate x-factor\n");
		return;
	}
	
	/* Parse options. */
	optreset = 1;
	optind = 1;
	while ((ch = getopt(argc,argv,"kx")) != -1)
	{
		switch (ch)
		{
			case 'k':
				showKPS = 1;
				showXFactor = 0;
				break;
			
			case 'x':
				showKPS = 0;
				showXFactor = 1;
				break;
			
			default:
				goto usage;
		}
	}
	
	/* If CD media is inserted, it's one ioctl.  If DVD media is
		inserted, it's a different ioctl.  We also use a different
		divisor for CD vs DVD media xfactors.  The two ioctls are
		otherwise equivalent. */
	if (DeviceHasMediaOfClass(device,kDRDeviceMediaClassCD))
	{
		request = DKIOCCDGETSPEED;
		divisor = kDRDeviceBurnSpeedCD1x;	/* value is the same for read/write speeds */
		units = "CD";
	}
	else if (DeviceHasMediaOfClass(device,kDRDeviceMediaClassDVD))
	{
		request = DKIOCDVDGETSPEED;
		divisor = kDRDeviceBurnSpeedDVD1x;	/* value is the same for read/write speeds */
		units = "DVD";
	}
	else
	{
		fprintf(stderr,"Unrecognized media class in device!\n");
		return;
	}
	
	/* Issue the ioctl. */
	if (ioctlWithDRDevice(device,request,(char*)&speed) || errno)
	{
		perror("ioctl failed");
		return;
	}
	
	/* Print the speed. */
	if (showKPS)
		printf("%d KB/s (K=1000)\n", (int)speed);
	if (showXFactor)
		printf("%.1fx %s\n", (float)(speed / divisor), units);
}



/* --------------------------------------------------------------
	SetSpeed
   --------------------------------------------------------------
*/
void
SetSpeed(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	u_int16_t speed;
	float kps = 0.0;
	unsigned long request;
	int ch, xfactor = 0;
	
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
	usage:
		printf("setSpeed [-m] [-k kps] [-x xfactor]\n");
		printf("   Changes the current read speed of the device.\n");
		printf("     -m   Use the maximum available speed.\n");
		printf("     -k   Use the specified speed in KB / s (K=1000)\n");
		printf("     -x   Use the specified x-factor\n");
		return;
	}
	
	/* Parse options. */
	optreset = 1;
	optind = 1;
	while ((ch = getopt(argc,argv,"mk:x:")) != -1)
	{
		switch (ch)
		{
			case 'm':
				xfactor = 0;
				kps = 65536.0;	/* 0xFFFF */
				break;
			
			case 'k':
				xfactor = 0;
				if (sscanf(optarg,"%f",&kps) != 1)
					goto usage;
				break;
			
			case 'x':
				xfactor = 1;
				if (sscanf(optarg,"%f",&kps) != 1)
					goto usage;
				break;
			
			default:
				goto usage;
		}
	}
	
	/* If CD media is inserted, it's one ioctl.  If DVD media is
		inserted, it's a different ioctl.  We also use a different
		multiplier for CD vs DVD media xfactors.  The two ioctls are
		otherwise equivalent. */
	if (DeviceHasMediaOfClass(device,kDRDeviceMediaClassCD))
	{
		request = DKIOCCDSETSPEED;
		if (xfactor)
			kps *= kDRDeviceBurnSpeedCD1x;	/* value is the same for read/write speeds */
	}
	else if (DeviceHasMediaOfClass(device,kDRDeviceMediaClassDVD))
	{
		request = DKIOCDVDSETSPEED;
		if (xfactor)
			kps *= kDRDeviceBurnSpeedDVD1x;	/* value is the same for read/write speeds */
	}
	else
	{
		fprintf(stderr,"Unrecognized media class in device!\n");
		return;
	}
	
	/* Issue the ioctl. */
	speed = (u_int16_t)kps;
	if (ioctlWithDRDevice(device,request,(char*)&speed) || errno)
	{
		perror("ioctl failed");
		return;
	}
}



/* --------------------------------------------------------------
	ReadDiscInfo
   --------------------------------------------------------------
*/
void
ReadDiscInfo(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	int ch, raw = 0;
	
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
	usage:
		printf("discInfo\n");
		printf("   Retrieves and parses a disc information structure, of the type\n");
		printf("   returned by the MMC command READ DISC INFORMATION.\n");
		printf("     -f       Formatted output.  This is the default.\n");
		printf("     -r       Raw output (hex dump)\n");
		return;
	}
	
	/* Parse options. */
	optreset = 1;
	optind = 1;
	while ((ch = getopt(argc,argv,"rf")) != -1)
	{
		switch (ch)
		{
			case 'r':
				raw = 1;
				break;
			
			case 'f':
				raw = 0;
				break;
			
			default:
				goto usage;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 0)
		goto usage;
	
	/* If CD media is inserted, it's one ioctl.  If DVD media is
		inserted, it's a different ioctl.  We also use a different
		data structure for the command and output. */
	if (DeviceHasMediaOfClass(device,kDRDeviceMediaClassCD))
	{
		dk_cd_read_disc_info_t	discInfoParam;
		CDDiscInfo				discInfo;
		
		memset(&discInfoParam,0,sizeof(discInfoParam));
		memset(&discInfo,0,sizeof(discInfo));
		discInfoParam.bufferLength = sizeof(discInfo);
		discInfoParam.buffer = &discInfo;
		
		/* Issue the ioctl */
		if (ioctlWithDRDevice(device,DKIOCCDREADDISCINFO,(char*)&discInfoParam) || errno)
		{
			perror("ioctl failed");
			return;
		}
		
		/* Print information from the struct. */
		if (raw)
		{
			/* Just print a hex dump. */
			hexdump((u_int8_t*)&discInfo,discInfoParam.bufferLength);
		}
		else
		{
			/* Print formatted output. */
			printf("erasable:                        %s\n",
				(discInfo.erasable == 0) ? "NO":"YES");
			printf("discStatus:                      %d (%s)\n", (int)discInfo.discStatus,
				(discInfo.discStatus == 0) ? "blank":
				(discInfo.discStatus == 1) ? "incomplete/appendable":
				(discInfo.discStatus == 2) ? "complete":"random-writable");
			printf("stateOfLastSession:              %d (%s)\n", (int)discInfo.stateOfLastSession,
				(discInfo.stateOfLastSession == 0) ? "empty":
				(discInfo.stateOfLastSession == 1) ? "incomplete":
				(discInfo.stateOfLastSession == 2) ? "reserved/damaged":"complete");
			printf("numberOfFirstTrack:              %d\n", (int)discInfo.numberOfFirstTrack);
			printf("numberOfSessions:                %d\n", (int)discInfo.numberOfSessionsLSB + 256*(int)discInfo.numberOfSessionsMSB);
			printf("firstTrackInLastSession:         %d\n", (int)discInfo.firstTrackNumberInLastSessionLSB + 256*(int)discInfo.firstTrackNumberInLastSessionMSB);
			printf("lastTrackInLastSession:          %d\n", (int)discInfo.lastTrackNumberInLastSessionLSB + 256*(int)discInfo.lastTrackNumberInLastSessionMSB);
			printf("discIdentificationValid:         %s\n",
				(discInfo.discIdentificationValid == 0) ? "NO":"YES");
			printf("discBarCodeValid:                %s\n",
				(discInfo.discBarCodeValid == 0) ? "NO":"YES");
			printf("unrestrictedUse:                 %s\n",
				(discInfo.unrestrictedUse == 0) ? "NO":"YES");
			printf("discType:                        $%02X (%s)\n", (int)discInfo.discType,
				(discInfo.discType == 0x00) ? "CD-DA or CD-ROM":
				(discInfo.discType == 0x10) ? "CD-I":
				(discInfo.discType == 0x20) ? "CD-ROM XA":"unknown");
			if (discInfo.discIdentificationValid)
			{
				printf("discIdentification:              $%08X\n", (int)discInfo.discIdentification);
			}
			printf("lastSessionLeadInStart:          %d:%02d.%02d\n",
				(int)discInfo.lastSessionLeadInStartTime.minute,
				(int)discInfo.lastSessionLeadInStartTime.second,
				(int)discInfo.lastSessionLeadInStartTime.frame);
			printf("lastPossibleStartTimeOfLeadOut:  %d:%02d.%02d\n",
				(int)discInfo.lastPossibleStartTimeOfLeadOut.minute,
				(int)discInfo.lastPossibleStartTimeOfLeadOut.second,
				(int)discInfo.lastPossibleStartTimeOfLeadOut.frame);
			if (discInfo.discBarCodeValid)
			{
				printf("discBarCode:                     %d %d %d %d %d %d %d %d\n",
					(int)discInfo.discBarCode[0], (int)discInfo.discBarCode[1],
					(int)discInfo.discBarCode[2], (int)discInfo.discBarCode[3],
					(int)discInfo.discBarCode[4], (int)discInfo.discBarCode[5],
					(int)discInfo.discBarCode[6], (int)discInfo.discBarCode[7]);
			}
		}
	}
	else if (DeviceHasMediaOfClass(device,kDRDeviceMediaClassDVD))
	{
		dk_dvd_read_disc_info_t	discInfoParam;
		DVDDiscInfo				discInfo;
		
		memset(&discInfoParam,0,sizeof(discInfoParam));
		memset(&discInfo,0,sizeof(discInfo));
		discInfoParam.bufferLength = sizeof(discInfo);
		discInfoParam.buffer = &discInfo;
		
		/* Issue the ioctl */
		if (ioctlWithDRDevice(device,DKIOCDVDREADDISCINFO,(char*)&discInfoParam) || errno)
		{
			perror("ioctl failed");
			return;
		}
		
		/* Print information from the struct. */
		if (raw)
		{
			/* Just print a hex dump. */
			hexdump((u_int8_t*)&discInfo,discInfoParam.bufferLength);
		}
		else
		{
			/* Print formatted output. */
			printf("erasable:                        %s\n",
				(discInfo.erasable == 0) ? "NO":"YES");
			printf("discStatus:                      %d (%s)\n", (int)discInfo.discStatus,
				(discInfo.discStatus == 0) ? "blank":
				(discInfo.discStatus == 1) ? "incomplete/appendable":
				(discInfo.discStatus == 2) ? "complete":"random-writable");
			printf("stateOfLastBorder:               %d (%s)\n", (int)discInfo.stateOfLastBorder,
				(discInfo.stateOfLastBorder == 0) ? "empty":
				(discInfo.stateOfLastBorder == 1) ? "incomplete":
				(discInfo.stateOfLastBorder == 2) ? "reserved/damaged":"complete");
			printf("numberOfBorders:                 %d\n", (int)discInfo.numberOfBordersLSB + 256*(int)discInfo.numberOfBordersMSB);
			printf("firstRZoneNumberInLastBorder:    %d\n", (int)discInfo.firstRZoneNumberInLastBorderLSB + 256*(int)discInfo.firstRZoneNumberInLastBorderMSB);
			printf("lastRZoneNumberInLastBorder:     %d\n", (int)discInfo.lastRZoneNumberInLastBorderLSB + 256*(int)discInfo.lastRZoneNumberInLastBorderMSB);
			printf("discBarCodeValid:                %s\n",
				(discInfo.discBarCodeValid == 0) ? "NO":"YES");
			printf("unrestrictedUse:                 %s\n",
				(discInfo.unrestrictedUse == 0) ? "NO":"YES");
			if (discInfo.discBarCodeValid)
			{
				printf("discBarCode:                     %d %d %d %d %d %d %d %d\n",
					(int)discInfo.discBarCode[0], (int)discInfo.discBarCode[1],
					(int)discInfo.discBarCode[2], (int)discInfo.discBarCode[3],
					(int)discInfo.discBarCode[4], (int)discInfo.discBarCode[5],
					(int)discInfo.discBarCode[6], (int)discInfo.discBarCode[7]);
			}
		}
	}
}



/* --------------------------------------------------------------
	ReadTrackInfo
   --------------------------------------------------------------
*/
void
ReadTrackInfo(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	int ch, raw = 0;
	int	which = 0;
	
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
	usage:
		printf("trackInfo [-r | -f] [track]\n");
		printf("   Retrieves and parses a track/rzone information structure, of the type\n");
		printf("   returned by the MMC command READ TRACK/RZONE INFORMATION.\n");
		printf("     -f       Formatted output.  This is the default.\n");
		printf("     -r       Raw output (hex dump)\n");
		printf("     track    Print track information for only the specified track/rzone.\n");
		printf("              Default is to print information for all tracks/rzones.\n");
		return;
	}
	
	/* Parse options. */
	optreset = 1;
	optind = 1;
	while ((ch = getopt(argc,argv,"rf")) != -1)
	{
		switch (ch)
		{
			case 'r':
				raw = 1;
				break;
			
			case 'f':
				raw = 0;
				break;
			
			default:
				goto usage;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc > 0)
	{
		if (sscanf(argv[0],"%i",&which) != 1)
			goto usage;
		argc -= 1;
		argv += 1;
	}
	if (argc != 0)
		goto usage;
	
	/* If CD media is inserted, it's one ioctl.  If DVD media is
		inserted, it's a different ioctl.  We also use a different
		data structure for the command and output. */
	if (DeviceHasMediaOfClass(device,kDRDeviceMediaClassCD))
	{
		dk_cd_read_disc_info_t	discInfoParam;
		CDDiscInfo				discInfo;
		int						start = which;
		int						end = which;
		
		/* If no parameters were specified, find out the number of tracks/rzones
			and loop through them all. */
		if (start == 0)
		{
			/* Issue an ioctl to read the disc info. */
			memset(&discInfoParam,0,sizeof(discInfoParam));
			memset(&discInfo,0,sizeof(discInfo));
			discInfoParam.bufferLength = sizeof(discInfo);
			discInfoParam.buffer = &discInfo;
			if (ioctlWithDRDevice(device,DKIOCCDREADDISCINFO,(char*)&discInfoParam) || errno)
			{
				perror("ioctl failed");
				return;
			}
			
			/* Set start and end to loop through all tracks. */
			start = (int)discInfo.numberOfFirstTrack;
			end = (int)discInfo.lastTrackNumberInLastSessionLSB +
					256*(int)discInfo.lastTrackNumberInLastSessionMSB;
		}
		
		/* Loop through selected tracks and print track info. */
		for (which=start; which<=end; ++which)
		{
			dk_cd_read_track_info_t trackInfoParam;
			CDTrackInfo				trackInfo;
			
			memset(&trackInfoParam,0,sizeof(trackInfoParam));
			memset(&trackInfo,0,sizeof(trackInfo));
			trackInfoParam.bufferLength = sizeof(trackInfo);
			trackInfoParam.buffer = &trackInfo;
			trackInfoParam.address = (u_int32_t)which;
			trackInfoParam.addressType = kCDTrackInfoAddressTypeTrackNumber;
			
			/* Issue the ioctl */
			if (ioctlWithDRDevice(device,DKIOCCDREADTRACKINFO,(char*)&trackInfoParam) || errno)
			{
				perror("ioctl failed");
				return;
			}
			
			/* Print information from the struct. */
			if (raw)
			{
				/* Just print a hex dump. */
				hexdump((u_int8_t*)&trackInfo,trackInfoParam.bufferLength);
			}
			else
			{
				/* Print formatted output. */
				printf("trackNumber:              %d\n", (int)trackInfo.trackNumberLSB + 256*(int)trackInfo.trackNumberMSB);
				printf("sessionNumber:            %d\n", (int)trackInfo.sessionNumberLSB + 256*(int)trackInfo.sessionNumberMSB);
				printf("damage:                   %s\n",
					(trackInfo.damage == 0) ? "NO":"YES");
				printf("copy:                     %s\n",
					(trackInfo.copy == 0) ? "NO":"YES");
				if (!trackInfo.blank)
				{
					printf("trackMode:                $%02X (%s)\n", (int)trackInfo.trackMode,
						(trackInfo.trackMode == 0x00) ? "2ch audio, no pre-emphasis, digital copy prohibited":
						(trackInfo.trackMode == 0x01) ? "2ch audio, with pre-emphasis, digital copy prohibited":
						(trackInfo.trackMode == 0x02) ? "2ch audio, no pre-emphasis, digital copy permitted":
						(trackInfo.trackMode == 0x03) ? "2ch audio, with pre-emphasis, digital copy permitted":
						(trackInfo.trackMode == 0x04) ? "data track, digital copy prohibited":
						(trackInfo.trackMode == 0x06) ? "data track, digital copy permitted":
						(trackInfo.trackMode == 0x08) ? "4ch audio, no pre-emphasis, digital copy prohibited":
						(trackInfo.trackMode == 0x09) ? "4ch audio, with pre-emphasis, digital copy prohibited":
						(trackInfo.trackMode == 0x0A) ? "4ch audio, no pre-emphasis, digital copy permitted":
						(trackInfo.trackMode == 0x0B) ? "4ch audio, with pre-emphasis, digital copy permitted":
						"unknown");
				}
				printf("reserved:                 %s\n",
					(trackInfo.reservedTrack == 0) ? "NO":"YES");
				printf("blank:                    %s\n",
					(trackInfo.blank == 0) ? "NO":"YES");
				printf("packet:                   %s\n",
					(trackInfo.packet == 0) ? "NO":"YES");
				printf("fixedPacket:              %s\n",
					(trackInfo.fixedPacket == 0) ? "NO":"YES");
				printf("dataMode:                 $%02X (%s)\n", (int)trackInfo.dataMode,
					(trackInfo.dataMode == 0x01) ? "CD-ROM Mode 1":
					(trackInfo.dataMode == 0x02) ? "CD-ROM Mode 2":"unknown");
				printf("lastRecordedAddressValid: %s\n",
					(trackInfo.lastRecordedAddressValid == 0) ? "NO":"YES");
				printf("nextWritableAddressValid: %s\n",
					(trackInfo.nextWritableAddressValid == 0) ? "NO":"YES");
				printf("trackStartAddress:        $%08X\n", (int)trackInfo.trackStartAddress);
				if (trackInfo.nextWritableAddressValid)
				{
					printf("nextWritableAddress:      $%08X\n", (int)trackInfo.nextWritableAddress);
				}
				printf("freeBlocks:               %d ($%08X)\n", (int)trackInfo.freeBlocks, (int)trackInfo.freeBlocks);
				if (trackInfo.fixedPacket)
				{
					printf("fixedPacketSize:          %d\n", (int)trackInfo.fixedPacketSize);
				}
				printf("trackSize:                %d ($%08X)\n", (int)trackInfo.trackSize, (int)trackInfo.trackSize);
				if (trackInfo.lastRecordedAddressValid)
				{
					printf("lastRecordedAddress:      $%08X\n", (int)trackInfo.trackSize);
				}
			}
			if (which < end)
				printf("----------------------------------------\n");
		}
	}
	else if (DeviceHasMediaOfClass(device,kDRDeviceMediaClassDVD))
	{
		dk_dvd_read_disc_info_t	discInfoParam;
		DVDDiscInfo				discInfo;
		int						start = which;
		int						end = which;
		
		/* If no parameters were specified, find out the number of tracks/rzones
			and loop through them all. */
		if (start == 0)
		{
			/* Issue an ioctl to read the disc info. */
			memset(&discInfoParam,0,sizeof(discInfoParam));
			memset(&discInfo,0,sizeof(discInfo));
			discInfoParam.bufferLength = sizeof(discInfo);
			discInfoParam.buffer = &discInfo;
			if (ioctlWithDRDevice(device,DKIOCDVDREADDISCINFO,(char*)&discInfoParam) || errno)
			{
				perror("ioctl failed");
				return;
			}
			
			/* Set start and end to loop through all rzones. */
			start = (int)1;
			end = (int)discInfo.lastRZoneNumberInLastBorderLSB +
					256*(int)discInfo.lastRZoneNumberInLastBorderMSB;
		}
		
		/* Loop through selected rzones and print rzone info. */
		for (which=start; which<=end; ++which)
		{
			dk_dvd_read_rzone_info_t	rzoneInfoParam;
			DVDRZoneInfo				rzoneInfo;
			
			memset(&rzoneInfoParam,0,sizeof(rzoneInfoParam));
			memset(&rzoneInfo,0,sizeof(rzoneInfo));
			rzoneInfoParam.bufferLength = sizeof(rzoneInfo);
			rzoneInfoParam.buffer = &rzoneInfo;
			rzoneInfoParam.address = (u_int32_t)which;
			rzoneInfoParam.addressType = kDVDRZoneInfoAddressTypeRZoneNumber;
			
			/* Issue the ioctl */
			if (ioctlWithDRDevice(device,DKIOCDVDREADRZONEINFO,(char*)&rzoneInfoParam) || errno)
			{
				perror("ioctl failed");
				return;
			}
			
			/* Print information from the struct. */
			if (raw)
			{
				/* Just print a hex dump. */
				hexdump((u_int8_t*)&rzoneInfo,rzoneInfoParam.bufferLength);
			}
			else
			{
				/* Print formatted output. */
				printf("rzoneNumber:              %d\n", (int)rzoneInfo.rzoneNumberLSB + 256*(int)rzoneInfo.rzoneNumberMSB);
				printf("borderNumber:             %d\n", (int)rzoneInfo.borderNumberLSB + 256*(int)rzoneInfo.borderNumberMSB);
				printf("damage:                   %s\n",
					(rzoneInfo.damage == 0) ? "NO":"YES");
				printf("copy:                     %s\n",
					(rzoneInfo.copy == 0) ? "NO":"YES");
				printf("reserved:                 %s\n",
					(rzoneInfo.reservedRZone == 0) ? "NO":"YES");
				printf("blank:                    %s\n",
					(rzoneInfo.blank == 0) ? "NO":"YES");
				printf("incremental:              %s\n",
					(rzoneInfo.incremental == 0) ? "NO":"YES");
				printf("restrictedOverwrite:      %s\n",
					(rzoneInfo.restrictedOverwrite == 0) ? "NO":"YES");
				printf("lastRecordedAddressValid: %s\n",
					(rzoneInfo.lastRecordedAddressValid == 0) ? "NO":"YES");
				printf("nextWritableAddressValid: %s\n",
					(rzoneInfo.nextWritableAddressValid == 0) ? "NO":"YES");
				printf("rzoneStartAddress:        $%08X\n", (int)rzoneInfo.rzoneStartAddress);
				if (rzoneInfo.nextWritableAddressValid)
				{
					printf("nextWritableAddress:      $%08X\n", (int)rzoneInfo.nextWritableAddress);
				}
				printf("freeBlocks:               %d ($%08X)\n", (int)rzoneInfo.freeBlocks, (int)rzoneInfo.freeBlocks);
				if (rzoneInfo.incremental)
				{
					printf("blockingFactor:           %d\n", (int)rzoneInfo.blockingFactor);
				}
				printf("rzoneSize:                %d ($%08X)\n", (int)rzoneInfo.rzoneSize, (int)rzoneInfo.rzoneSize);
				if (rzoneInfo.lastRecordedAddressValid)
				{
					printf("lastRecordedAddress:      $%08X\n", (int)rzoneInfo.rzoneSize);
				}
			}
			if (which < end)
				printf("----------------------------------------\n");
		}
	}
}



/* --------------------------------------------------------------
	ReadCD
   --------------------------------------------------------------
*/
void
ReadCD(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	u_int64_t		lba = 0;
	int				numBlocks = 1;
	int				area = kCDSectorAreaUser;
	int				type = kCDSectorTypeUnknown;
	int				blockSize = kCDSectorSizeWhole;
	int				ch;
	dk_cd_read_t	readCDParam;
	u_int8_t		*buffer;
	
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
	usage:
		printf("readCD [-n num] [-a area] [-t type] [-s bsize] lba\n");
		printf("   Reads one or more CD blocks and dumps them to output as hex.\n");
		printf("     -n num    Number of blocks to read.  Default is 1.\n");
		printf("     -a area   Specifies the area of the block to read.  Default is user data.\n");
		printf("               This value is obtained by a bitwise OR of the following masks:\n");
		printf("                 sync        0x80\n");
		printf("                 header      0x20\n");
		printf("                 subheader   0x40\n");
		printf("                 user data   0x10\n");
		printf("                 aux         0x08\n");
		printf("                 error flags 0x02\n");
		printf("                 subchannel  0x01\n");
		printf("     -t type   Specifies the expected block type.  Default is unknown.\n");
		printf("               This value may be one of the following:\n");
		printf("                 unknown          0\n");
		printf("                 CD-DA            1\n");
		printf("                 Mode 1           2\n");
		printf("                 Mode 2           3\n");
		printf("                 Mode 2, Form 1   4\n");
		printf("                 Mode 2, Form 2   5\n");
		printf("     -s bsize  Specifies the expected block size.  Default is 2352.\n");
		printf("     lba       Logical block address to read.\n");
		return;
	}
	
	/* Parse options. */
	optreset = 1;
	optind = 1;
	while ((ch = getopt(argc,argv,"n:a:t:s:")) != -1)
	{
		switch (ch)
		{
			case 'n':
				if (sscanf(optarg,"%i",&numBlocks) != 1)
					goto usage;
				break;
			
			case 'a':
				if (sscanf(optarg,"%i",&area) != 1)
					goto usage;
				break;
			
			case 't':
				if (sscanf(optarg,"%i",&type) != 1)
					goto usage;
				break;
			
			case 's':
				if (sscanf(optarg,"%i",&blockSize) != 1)
					goto usage;
				break;
			
			default:
				goto usage;
		}
	}
	argc -= optind;
	argv += optind;
	
	if (argc != 1)
		goto usage;
	if (sscanf(argv[0],"%llu",&lba) != 1)
		goto usage;
	
	/* Check params. */
	if (numBlocks * blockSize <= 0)
	{
		fprintf(stderr,"Invalid block count (%d) or size (%d).\n", (int)numBlocks, (int)blockSize);
		return;
	}
	
	/* Check to make sure the device contains CD media. */
	if (!DeviceHasMediaOfClass(device,kDRDeviceMediaClassCD))
	{
		fprintf(stderr,"CD media required.\n");
		return;
	}
	
	/* Allocate memory for the block buffer. */
	buffer = (u_int8_t*)calloc(numBlocks,blockSize);
	
	/* Prepare the ioctl. */
	memset(&readCDParam,0,sizeof(readCDParam));
	readCDParam.offset = (u_int64_t)(lba * blockSize);
	readCDParam.sectorArea = (u_int8_t)area;
	readCDParam.sectorType = (u_int8_t)type;
	readCDParam.bufferLength = numBlocks * blockSize;
	readCDParam.buffer = buffer;
	
	/* Issue the ioctl. */
	if (ioctlWithDRDevice(device,DKIOCCDREAD,(char*)&readCDParam) || errno)
	{
		perror("ioctl failed");
		free(buffer);
		return;
	}
	
	/* Print the contents of the buffer as a hex dump. */
	hexdump(buffer,readCDParam.bufferLength);
	
	/* Clean up allocated memory. */
	free(buffer);
}



/* --------------------------------------------------------------
	ReadTOC
   --------------------------------------------------------------
*/
void
ReadTOC(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	dk_cd_read_toc_t	readTOCParam;
	u_int8_t			buffer[2048];
	int ch, raw = 0, format = 2;
	
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
	usage:
		printf("toc [-r | -f] [-m | -l] [-t type]\n");
		printf("   Displays the table of contents or other structures on the disc.\n");
		printf("     -f       Formatted output.  This is the default.\n");
		printf("     -r       Raw output (hex dump)\n");
		printf("     -t type  Specifies the data requested.  This may be one of the following:\n");
		printf("                  2     Table of contents.  This is the default.\n");
		printf("                  3     Program memory area.\n");
		printf("                  4     Absolute Time in Pre-Groove.\n");
		printf("                  5     CD-Text packets.\n");
		return;
	}
	
	/* Parse options. */
	optreset = 1;
	optind = 1;
	while ((ch = getopt(argc,argv,"rfmlt:")) != -1)
	{
		switch (ch)
		{
			case 'r':
				raw = 1;
				break;
			
			case 'f':
				raw = 0;
				break;
			
			case 't':
				if (sscanf(optarg,"%i",&format) != 1)
					goto usage;
				break;
			
			default:
				goto usage;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 0)
		goto usage;
	
	/* Prepare the parameters. */
	memset(&readTOCParam,0,sizeof(readTOCParam));
	memset(buffer,0,sizeof(buffer));
	readTOCParam.format = (u_int8_t)format;
	readTOCParam.formatAsTime = 1;
	readTOCParam.address.track = 0;
	readTOCParam.bufferLength = sizeof(buffer);
	readTOCParam.buffer = buffer;
	
	/* Issue the ioctl. */
	if (ioctlWithDRDevice(device,DKIOCCDREADTOC,(char*)&readTOCParam) || errno)
	{
		perror("ioctl failed");
		return;
	}
	
	/* If we're doing formatted output, switch based on the format of the data. */
	if (!raw &&
		(format == kCDTOCFormatTOC || format == kCDTOCFormatPMA) &&
		readTOCParam.bufferLength >= sizeof(CDTOC))
	{
		CDTOC	*toc = (CDTOC*)buffer;
		UInt32	i, numDescriptors = CDTOCGetDescriptorCount(toc);
		
		/* Print the header for the TOC */
		if (format == kCDTOCFormatTOC)
		{
			printf("First session:             %d\n", (int)toc->sessionFirst);
			printf("Last session:              %d\n", (int)toc->sessionLast);
		}
		
		/* Loop through each descriptor in the TOC/PMA and print it. */
		for (i=0; i<numDescriptors; ++i)
		{
			CDTOCDescriptor *desc = &toc->descriptors[i];
			
			if (desc->point <= 99 && desc->adr == 1)
			{
				if (format == kCDTOCFormatTOC)
				{
					printf("Session %2d, Track %2d:      ",
						(int)desc->session, (int)desc->point);
				}
				else
					printf("Track %2d:                   ",(int)desc->point);
				
				printf("%02d:%02d.%02d",
					(int)desc->p.minute, (int)desc->p.second, (int)desc->p.frame);

				printf("  %s\n",
					(desc->control == 0x00) ? "2ch audio, no pre-emphasis, digital copy prohibited":
					(desc->control == 0x01) ? "2ch audio, with pre-emphasis, digital copy prohibited":
					(desc->control == 0x02) ? "2ch audio, no pre-emphasis, digital copy permitted":
					(desc->control == 0x03) ? "2ch audio, with pre-emphasis, digital copy permitted":
					(desc->control == 0x04) ? "data track, digital copy prohibited":
					(desc->control == 0x06) ? "data track, digital copy permitted":
					(desc->control == 0x08) ? "4ch audio, no pre-emphasis, digital copy prohibited":
					(desc->control == 0x09) ? "4ch audio, with pre-emphasis, digital copy prohibited":
					(desc->control == 0x0A) ? "4ch audio, no pre-emphasis, digital copy permitted":
					(desc->control == 0x0B) ? "4ch audio, with pre-emphasis, digital copy permitted":
					"unknown");
			}
			else if (desc->point == 0xA0 && desc->adr == 1)
			{
				printf("First track:               %d\n", (int)desc->p.minute);
				printf("Disc type:                 %d (%s)\n", (int)desc->p.second,
					(desc->p.second == 0x00) ? "CD-DA, or CD-ROM with first track in Mode 1":
					(desc->p.second == 0x10) ? "CD-I disc":
					(desc->p.second == 0x20) ? "CD-ROM XA disc with first track in Mode 2":"unknown");
			}
			else if (desc->point == 0xA1 && desc->adr == 1)
			{
				printf("Last track:                %d\n", (int)desc->p.minute);
			}
			else if (desc->point == 0xA2 && desc->adr == 1)
			{
				printf("Lead-out:                  %02d:%02d.%02d\n",
					(int)desc->p.minute, (int)desc->p.second, (int)desc->p.frame);
			}
			else if (desc->point == 0xB0 && desc->adr == 5)
			{
				printf("Next possible track start: %02d:%02d.%02d\n",
					(int)desc->address.minute, (int)desc->address.second, (int)desc->address.frame);
				printf("Number of ptrs in Mode 5:  %d\n",
					(int)desc->zero);
				printf("Last possible lead-out:    %02d:%02d.%02d\n",
					(int)desc->p.minute, (int)desc->p.second, (int)desc->p.frame);
			}
			else if (desc->point == 0xB1 && desc->adr == 5)
			{
				printf("Skip interval pointers:    %d\n", (int)desc->p.minute);
				printf("Skip track pointers:       %d\n", (int)desc->p.second);
			}
			else if (desc->point >= 0xB2 && desc->point <= 0xB4 && desc->adr == 5)
			{
				printf("Skip numbers:              %d, %d, %d, %d, %d, %d, %d\n",
					(int)desc->address.minute, (int)desc->address.second, (int)desc->address.frame,
					(int)desc->zero, (int)desc->p.minute, (int)desc->p.second, (int)desc->p.frame);
			}
			else if (desc->point >= 1 && desc->point <= 40 && desc->adr == 5)
			{
				printf("Skip from %02d:%02d.%02d to %02d:%02d.%02d\n",
					(int)desc->p.minute, (int)desc->p.second, (int)desc->p.frame,
					(int)desc->address.minute, (int)desc->address.second, (int)desc->address.frame);
			}
			else if (desc->point == 0xC0 && desc->adr == 5)
			{
				printf("Optimum recording power:   %d\n", (int)desc->address.minute);
				printf("Application code:          %d\n", (int)desc->address.second);
				printf("Start of first lead-in:    %02d:%02d.%02d\n",
					(int)desc->p.minute, (int)desc->p.second, (int)desc->p.frame);
			}
			else
			{
				printf("Unrecognized descriptor: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					(int)((char*)desc)[0], (int)((char*)desc)[1], (int)((char*)desc)[2], 
					(int)((char*)desc)[3], (int)((char*)desc)[4], (int)((char*)desc)[5], 
					(int)((char*)desc)[6], (int)((char*)desc)[7], (int)((char*)desc)[8], 
					(int)((char*)desc)[9], (int)((char*)desc)[10]);
			}
		}
	}
	else if (!raw && (format == kCDTOCFormatATIP))
	{
		CDATIP *atip = (CDATIP*)buffer;
		
		printf("Recommended writing power:  %d\n", (int)atip->indicativeTargetWritingPower);
		printf("Reference speed:            %d\n", (int)atip->referenceSpeed);
		printf("Unrestricted use:           %s\n",
			(atip->unrestrictedUse == 0) ? "NO":"YES");
		printf("Disc type:                  %s\n",
			(atip->discType == 0) ? "CD-R":"CD-RW");
		printf("Disc subtype:               %d (%s)\n", (int)atip->discSubType,
			(atip->discType == 1 && atip->discSubType == 0) ? "low speed CD-RW":
			(atip->discType == 1 && atip->discSubType == 1) ? "high speed CD-RW":"unknown");
		printf("A1 valid:                   %s\n",
			(atip->a1Valid == 0) ? "NO":"YES");
		printf("A2 valid:                   %s\n",
			(atip->a2Valid == 0) ? "NO":"YES");
		printf("A3 valid:                   %s\n",
			(atip->a3Valid == 0) ? "NO":"YES");
		printf("Start of lead-in:           %02d:%02d.%02d\n",
			(int)atip->startTimeOfLeadIn.minute,
			(int)atip->startTimeOfLeadIn.second,
			(int)atip->startTimeOfLeadIn.frame);
		printf("Last possible lead-out:     %02d:%02d.%02d\n",
			(int)atip->lastPossibleStartTimeOfLeadOut.minute,
			(int)atip->lastPossibleStartTimeOfLeadOut.second,
			(int)atip->lastPossibleStartTimeOfLeadOut.frame);
		
		if (atip->a1Valid)
		{
			printf("A1:                     $%02x $%02x $%02x\n",
				(int)atip->a1[0], (int)atip->a1[1], (int)atip->a1[2]);
		}
		if (atip->a2Valid)
		{
			printf("A2:                     $%02x $%02x $%02x\n",
				(int)atip->a2[0], (int)atip->a2[1], (int)atip->a2[2]);
		}
		if (atip->a3Valid)
		{
			printf("A3:                     $%02x $%02x $%02x\n",
				(int)atip->a3[0], (int)atip->a3[1], (int)atip->a3[2]);
		}
	}
	else if (!raw && (format == kCDTOCFormatTEXT) && readTOCParam.bufferLength >= sizeof(CDTEXT))
	{
		CDTEXT *text = (CDTEXT*)buffer;
		UInt32 i, j, numDescriptors = (text->dataLength + 2 - sizeof(CDTEXT)) / sizeof(CDTEXTDescriptor);
		
		printf("Number of CD-Text PACKs:  %d\n", (int)numDescriptors);
		if (numDescriptors > 0)
		{
			/* Print out each individual PACK. */
			printf("Type Trk Seq DBCC Blk Pos Data                                  CRC    ASCII\n");
			for (i=0; i<numDescriptors; ++i)
			{
				CDTEXTDescriptor *desc = &text->descriptors[i];
				
				printf("%02x   %2d  %02x  %d    %d   %-2d  ",
					(int)desc->packType, (int)desc->trackNumber, (int)desc->sequenceNumber,
					(int)desc->doubleByteCharacterCode, (int)desc->blockNumber,
					(int)desc->characterPosition);
				
				for (j=0; j<12; ++j)
					printf("%02x ", (int)desc->textData[j]);
				printf("  %02x%02x  |", (int)desc->reserved[0], (int)desc->reserved[1]);
				for (j=0; j<12; ++j)
					printf("%c", isprint((char)desc->textData[j]) ? (char)desc->textData[j]:(char)'.');
				printf("|\n");
			}
		}
	}
	else
	{
		/* Just print a hex dump. */
		hexdump(buffer,readTOCParam.bufferLength);
	}
}



/* --------------------------------------------------------------
	ReadMCN
   --------------------------------------------------------------
*/
void
ReadMCN(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	dk_cd_read_mcn_t	readMCNParam;
	
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
	usage:
		printf("mcn\n");
		printf("   Displays the UPC/EAN bar code, also known as the media catalog number.\n");
		printf("   No arguments are defined.  This command is only applicable to CD media.\n");
		return;
	}
	
	/* Parse arguments. */
	if (argc != 1)
		goto usage;
	
	/* Prepare the parameters. */
	memset(&readMCNParam,0,sizeof(readMCNParam));
	
	/* Issue the ioctl. */
	if (ioctlWithDRDevice(device,DKIOCCDREADMCN,(char*)&readMCNParam) || errno)
	{
		perror("ioctl failed");
		return;
	}
	
	/* Print the MCN. */
	printf("%s\n", (char*)readMCNParam.mcn);
}



/* --------------------------------------------------------------
	ReadISRC
   --------------------------------------------------------------
*/
void
ReadISRC(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	dk_cd_read_isrc_t		readISRCParam;
	int						which = 0, start = 0, end = 0;
	
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
	usage:
		printf("isrc [track]\n");
		printf("   Displays the ISRC (international standard recording code).\n");
		printf("     track    Print track ISRC for only the specified track.\n");
		printf("              Default is to print ISRC for all tracks.\n");
		return;
	}
	
	/* Parse options. */
	if (argc > 1)
	{
		if (sscanf(argv[1],"%i",&which) != 1)
			goto usage;
		argc -= 1;
		argv += 1;
		start = end = which;
	}
	if (argc != 1)
		goto usage;
	
	/* If no parameters were specified, find out the number of tracks/rzones
		and loop through them all. */
	if (start == 0)
	{
		dk_cd_read_disc_info_t	discInfoParam;
		CDDiscInfo				discInfo;
		
		/* Issue an ioctl to read the disc info. */
		memset(&discInfoParam,0,sizeof(discInfoParam));
		memset(&discInfo,0,sizeof(discInfo));
		discInfoParam.bufferLength = sizeof(discInfo);
		discInfoParam.buffer = &discInfo;
		if (ioctlWithDRDevice(device,DKIOCCDREADDISCINFO,(char*)&discInfoParam) || errno)
		{
			perror("ioctl failed");
			return;
		}
		
		/* Set start and end to loop through all tracks. */
		start = (int)discInfo.numberOfFirstTrack;
		end = (int)discInfo.lastTrackNumberInLastSessionLSB +
				256*(int)discInfo.lastTrackNumberInLastSessionMSB;
	}
	
	/* Loop through selected tracks and print the ISRC. */
	for (which=start; which<=end; ++which)
	{
		/* Prepare the parameters. */
		memset(&readISRCParam,0,sizeof(readISRCParam));
		readISRCParam.track = (u_int8_t)which;
		
		/* Issue the ioctl. */
		if (ioctlWithDRDevice(device,DKIOCCDREADISRC,(char*)&readISRCParam) || errno)
		{
			perror("ioctl failed");
			continue;
		}
		
		/* Print the ISRC. */
		printf("Track %2d ISRC: %s\n", (int)readISRCParam.track, (char*)readISRCParam.isrc);
	}
}



/* --------------------------------------------------------------
	ExitShell
   --------------------------------------------------------------
*/
void
ExitShell(
	DRDeviceRef device,
	int argc,
	char **argv)
{
	/* Display help if requested. */
	if (argc > 1 && !strcasecmp(argv[1],"help"))
	{
		printf("exit\n");
		printf("   Terminates the shell.\n");
		return;
	}
	
	exit(0);
}



