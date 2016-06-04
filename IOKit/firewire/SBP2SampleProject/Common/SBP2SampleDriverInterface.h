/*
	File:		SBP2SampleDriverInterface.h

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.

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
 
#ifndef _IOKIT_SBP2SAMPLEDRIVERINTERFACE_H_
#define _IOKIT_SBP2SAMPLEDRIVERINTERFACE_H_

/* 631F68D2-B9E6-11D4-8147-000A277E7234 */
#define kSBP2SampleDriverTypeID CFUUIDGetConstantUUIDWithBytes(NULL,	    \
    0x63, 0x1F, 0x68, 0xD2, 0xB9, 0xE6, 0x11, 0xD4, 					\
	0x81, 0x47, 0x00, 0x0A, 0x27, 0x7E, 0x72, 0x34 )

/* 63B18520-B9E6-11D4-ADE9-000A277E7234 */
#define kSBP2SampleDriverFactoryID CFUUIDGetConstantUUIDWithBytes(NULL,   \
    0x63, 0xB1, 0x85, 0x20, 0xB9, 0xE6, 0x11, 0xD4,						\
	0xAD, 0xE9, 0x00, 0x0A, 0x27, 0x7E, 0x72, 0x34 )

/* 6443E5D8-B9E6-11D4-B7B5-000A277E7234 */
#define kSBP2SampleDriverInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL,	\
    0x64, 0x43, 0xE5, 0xD8, 0xB9, 0xE6, 0x11, 0xD4,						\
	0xB7, 0xB5, 0x00, 0x0A, 0x27, 0x7E, 0x72, 0x34 )

typedef void (*SBP2SampleIOCallback)( void * refCon, IOReturn status );

typedef struct 
{
	IUNKNOWN_C_GUTS;
	
	UInt16	version;						
    UInt16	revision;
	
	IOReturn (*setUpCallbacksWithRunLoop)( void *self, CFRunLoopRef cfRunLoopRef );
	void (*loginToDevice)( void * self );
	void (*logoutOfDevice)( void * self );
	
	// don't call readBlock on the runLoop
	IOReturn (*readBlock)( void * self,	UInt32 id, UInt32 block, void * buffer );
	UInt32 (*getNewTransactionID)( void * self );
	void (*abortReadsWithTransactionID)( void * self, UInt32 id );
	IOReturn (*writeBlock)( void * self, UInt32 id, UInt32 block, void * buffer );
} SBP2SampleDriverInterface;

#endif
