/*

File: filesupport.c

Abstract: Contains helper routines to load the contents of a UTF-16 text
file into a CFString.

Version: <1.0>

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2004 Apple Computer, Inc., All Rights Reserved

*/ 

#include "filesupport.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>


// ----------------------------------------------------------------------------
// MapFile
//
// Uses mmap to map a file into a read-only, private memory region. Size of the
// data is returned on success, zero is returned on error. Caller is responsible
// for unmapping the file.
//
size_t MapFile (const char *iPath, void **oData)
{
    struct stat statInfo;
    int fd;
        
    // Return safe value on error
    *oData = NULL;
        
    // Open the file
    fd = open(iPath, O_RDONLY, 0);
    require(( fd >= 0 ), MapFile_err);

    // Determine file size
    require(( fstat(fd, &statInfo) == 0 ), MapFile_err);

    // Map the file into a private memory region.
    *oData = mmap(NULL, statInfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    require(( *oData != MAP_FAILED ), MapFile_err);

    // On success, return the size of the mapped file.
    close(fd); // Okay because the kernel does not use this file descriptor
    return statInfo.st_size;

MapFile_err:
    perror(iPath);
    close(fd);
    return 0;
}


// ----------------------------------------------------------------------------
// LoadFileIntoCFString
//
// For a given UTF-16 plain text file located in the app bundle, this function
// will load it into a CFString and return it. Caller must release the CFString
// when done. Use TextEdit.app or BBEdit to save plain text files as UTF-16.
//
CFStringRef LoadFileIntoCFString (const char *filename)
{
    void *fileContents;
    size_t fileLength;
    CFDataRef data;
	CFStringRef string, filenameCFString, filePathCFString;
	CFURLRef fileLocation;
	char path[PATH_MAX]; // PATH_MAX is defined in <sys/syslimits.h>
	
	// Get the path to the file
	filenameCFString = CFStringCreateWithFileSystemRepresentation(NULL, filename);
	fileLocation = CFBundleCopyResourceURL(CFBundleGetMainBundle(), filenameCFString, NULL, NULL);
	CFRelease(filenameCFString);
	if ( fileLocation == NULL ) return NULL;
	filePathCFString = CFURLCopyFileSystemPath(fileLocation, kCFURLPOSIXPathStyle);
	CFRelease(fileLocation);
	CFStringGetFileSystemRepresentation(filePathCFString, path, PATH_MAX);
	CFRelease(filePathCFString);

    // Map the file into memory
    fileLength = MapFile(path, &fileContents);
    if ( fileLength == 0 ) return NULL;

    // Load the file into a CFString
    data = CFDataCreateWithBytesNoCopy(NULL, fileContents, fileLength, kCFAllocatorNull);
	string = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, data, kCFStringEncodingUnicode);
    CFRelease(data);

    // Unmap the file from memory now that we have finished loading it
    munmap(fileContents, fileLength);

    // Return the string to the caller (caller must release it)
    return string;
}
