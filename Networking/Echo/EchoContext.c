/*
	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.

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

#pragma mark Includes
#include "EchoContext.h"

#include <CoreServices/CoreServices.h>

#pragma mark -
#pragma mark Type Declarations

struct __EchoContext {
	CFAllocatorRef		_alloc;			// Allocator used to allocate this
	UInt32				_retainCount;	// Number of times retained.
	
	CFRunLoopTimerRef	_timer;			// Timer for controlling timeouts
	
	CFReadStreamRef		_inStream;		// Incoming data stream
	CFWriteStreamRef	_outStream;		// Outgoing data stream
	
	CFMutableDataRef	_rcvdBytes;		// Buffer of received bytes
};


#pragma mark -
#pragma mark Constant Definitions

static const CFTimeInterval kTimeOutInSeconds = 60;

static const CFOptionFlags kReadEvents = kCFStreamEventHasBytesAvailable |
                                         kCFStreamEventErrorOccurred |
                                         kCFStreamEventEndEncountered;

static const CFOptionFlags kWriteEvents = kCFStreamEventCanAcceptBytes |
										  kCFStreamEventErrorOccurred;


#pragma mark -
#pragma mark Static Function Declarations

static void _EchoContextHandleHasBytesAvailable(EchoContextRef context);
static void _EchoContextHandleEndEncountered(EchoContextRef context);
static void _EchoContextHandleCanAcceptBytes(EchoContextRef context);
static void _EchoContextHandleErrorOccurred(EchoContextRef context);
static void _EchoContextHandleTimeOut(EchoContextRef context);

static void _ReadStreamCallBack(CFReadStreamRef inStream, CFStreamEventType type, EchoContextRef context);
static void _WriteStreamCallBack(CFWriteStreamRef outStream, CFStreamEventType type, EchoContextRef context);
static void _TimerCallBack(CFRunLoopTimerRef timer, EchoContextRef context);


#pragma mark -
#pragma mark Extern Function Definitions (API)

/* extern */ EchoContextRef
EchoContextCreate(CFAllocatorRef alloc, CFSocketNativeHandle nativeSocket) {

	EchoContextRef context = NULL;

	do {
		// Allocate the buffer for the context.
		context = CFAllocatorAllocate(alloc, sizeof(context[0]), 0);
		
		// Fail if unable to create the context
		if (context == NULL)
			break;
		
		memset(context, 0, sizeof(context[0]));
		
		// Save the allocator for deallocating later.
		if (alloc)
			context->_alloc = CFRetain(alloc);
		
		// Bump the retain count.
		EchoContextRetain(context);
		
		// Create the streams for the incoming socket connection.
		CFStreamCreatePairWithSocket(alloc, nativeSocket, &(context->_inStream), &(context->_outStream));

		// Require both streams in order to create.
		if ((context->_inStream == NULL) || (context->_outStream == NULL))
			break;
				
		// Give up ownership of the native socket.
		CFReadStreamSetProperty(context->_inStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
			
		// Create the receive buffer of no fixed size.
		context->_rcvdBytes = CFDataCreateMutable(alloc, 0);
		
		// Fail if unable to create receive buffer.
		if (context->_rcvdBytes == NULL)
			break;
			
		return context;
		
	} while (0);
	
	// Something failed, so clean up.
	EchoContextRelease(context);
	
	return NULL;
}


/* extern */ EchoContextRef
EchoContextRetain(EchoContextRef context) {
	
	// Bump the retain count if context is good.
	if (context != NULL)
		context->_retainCount++;
		
	return context;
}


/* extern */ void
EchoContextRelease(EchoContextRef context) {

	if (context != NULL) {
		
		// Decrease the retain count.
		context->_retainCount--;
		
		// Don't dispose until the count goes to zero.
		if (context->_retainCount > 0)
			return;

        // The streams and timers retain the context.  If any of these are still set, it means
        // that somehow we hit a retain count of zero, but are still held by them!  This would
        // happen if the context was over-released.
        assert(!context->_inStream && !context->_outStream && !context->_timer);

        // Hold locally so deallocation can happen and then safely release.
		CFAllocatorRef alloc = context->_alloc;
		
		// Release the buffer if there is one.
		if (context->_rcvdBytes != NULL)
			CFRelease(context->_rcvdBytes);
		
		// Free the memory in use by the context.
		CFAllocatorDeallocate(alloc, context);
		
		// Release the allocator.
		if (alloc)
			CFRelease(alloc);
	}
}


/* extern */ Boolean
EchoContextOpen(EchoContextRef context) {

	do {
		Boolean didSet;
		CFRunLoopRef runLoop = CFRunLoopGetCurrent();
		
		CFStreamClientContext streamCtxt = {0, context, (void*(*)(void*))&EchoContextRetain, (void(*)(void*))&EchoContextRelease, NULL};
		CFRunLoopTimerContext timerCtxt = {0, context, (const void*(*)(const void*))&EchoContextRetain, (void(*)(const void*))&EchoContextRelease, NULL};
		
		// Set the client on the read stream.
		didSet = CFReadStreamSetClient(context->_inStream,
									   kReadEvents,
									   (CFReadStreamClientCallBack)&_ReadStreamCallBack,
									   &streamCtxt);
									   
		// Fail if unable to set the client.
		if (!didSet)
			break;
		
		// Set the client on the write stream.
		didSet = CFWriteStreamSetClient(context->_outStream,
										kWriteEvents,
										(CFWriteStreamClientCallBack)&_WriteStreamCallBack,
										&streamCtxt);
		
		// Fail if unable to set the client.
		if (!didSet)
			break;
			
		// Schedule the streams on the current run loop and default mode.
		CFReadStreamScheduleWithRunLoop(context->_inStream, runLoop, kCFRunLoopCommonModes);
		CFWriteStreamScheduleWithRunLoop(context->_outStream, runLoop, kCFRunLoopCommonModes);
		
		// Open the stream for reading.
		if (!CFReadStreamOpen(context->_inStream))
			break;
			
		// Open the stream for writing.
		if (!CFWriteStreamOpen(context->_outStream))
			break;
		
		// Create the timeout timer
		context->_timer = CFRunLoopTimerCreate(CFGetAllocator(context->_inStream),
											   CFAbsoluteTimeGetCurrent() + kTimeOutInSeconds,
											   0,		// interval
											   0,		// flags
											   0,		// order
											   (CFRunLoopTimerCallBack)_TimerCallBack,
											   &timerCtxt);
		
		// Fail if unable to create the timer.
		if (context->_timer == NULL)
			break;
			
        CFRunLoopAddTimer(runLoop, context->_timer, kCFRunLoopCommonModes);
            
		return TRUE;
		
	} while (0);
	
	// Something failed, so clean up.
	EchoContextClose(context);
	
	return FALSE;
}


/* extern */ void
EchoContextClose(EchoContextRef context) {

	CFRunLoopRef runLoop = CFRunLoopGetCurrent();

	// Check if the read stream exists.
	if (context->_inStream) {
		
		// Unschedule, close, and release it.
		CFReadStreamSetClient(context->_inStream, 0, NULL, NULL);
		CFReadStreamUnscheduleFromRunLoop(context->_inStream, runLoop, kCFRunLoopCommonModes);
		CFReadStreamClose(context->_inStream);
		CFRelease(context->_inStream);
		
		// Remove the reference.
		context->_inStream = NULL;
	}

	// Check if the write stream exists.
	if (context->_outStream) {
		
		// Unschedule, close, and release it.
		CFWriteStreamSetClient(context->_outStream, 0, NULL, NULL);
		CFWriteStreamUnscheduleFromRunLoop(context->_outStream, runLoop, kCFRunLoopCommonModes);
		CFWriteStreamClose(context->_outStream);
		CFRelease(context->_outStream);
		
		// Remove the reference.
		context->_outStream = NULL;
	}

    // Get rid of the timer, if it still exists
    if (context->_timer != NULL) {

        CFRunLoopTimerInvalidate(context->_timer);
        CFRelease(context->_timer);
        context->_timer = NULL;
    }
}


#pragma mark -
#pragma mark Static Function Definitions

/* static */ void
_EchoContextHandleHasBytesAvailable(EchoContextRef context) {

	UInt8 buffer[2048];
	
	// Try reading the bytes into the buffer.
	CFIndex bytesRead = CFReadStreamRead(context->_inStream, buffer, sizeof(buffer));
	
	// Reset the timeout.
	CFRunLoopTimerSetNextFireDate(context->_timer, CFAbsoluteTimeGetCurrent() + kTimeOutInSeconds);
	
	// If there wasn't an error (-1) and not end (0), process the data.
	if (bytesRead > 0) {
		
		// Add the bytes of data to the receive buffer.
		CFDataAppendBytes(context->_rcvdBytes, buffer, bytesRead);
		
		// If the ouput stream can write, try sending the bytes.
		if (CFWriteStreamCanAcceptBytes(context->_outStream))
			_EchoContextHandleCanAcceptBytes(context);
	}
}


/* static */ void
_EchoContextHandleEndEncountered(EchoContextRef context) {

	// End was hit, so destroy the context.
    EchoContextClose(context);
    EchoContextRelease(context);
}


/* static */ void
_EchoContextHandleCanAcceptBytes(EchoContextRef context) {
	
	/*
	** Echo looks for a '/n' in the data.  If one is found, all bytes up to and including
	** the linefeed are sent back to the client.  The write of this buffer may not send
	** all of the data, so the bytes that are successfully sent are removed from the
	** buffer.  When told that the stream can accept bytes again, the whole process will
	** fire again.
	*/
	
	// Get the start of the buffer to send.
	const UInt8* start = CFDataGetBytePtr(context->_rcvdBytes);
	
	// Find the linefeed if it exists.
	const UInt8* lf = (const UInt8*)memchr(start, '\n', CFDataGetLength(context->_rcvdBytes));

	// Writing resets the timer.
	CFRunLoopTimerSetNextFireDate(context->_timer, CFAbsoluteTimeGetCurrent() + kTimeOutInSeconds);
	
	// If there was a linefeed, take care of sending the data.
	if (lf != NULL) {
		
		// Write all of the bytes inbetween and including the linefeed.
		CFIndex bytesWritten = CFWriteStreamWrite(context->_outStream, start, lf - start + 1);
		
		// If successfully sent the data, remove the bytes from the buffer.
		if (bytesWritten > 0)
			CFDataDeleteBytes(context->_rcvdBytes, CFRangeMake(0, bytesWritten));
	}
}


/* static */ void
_EchoContextHandleErrorOccurred(EchoContextRef context) {

	// Hit an error, so destroy the context which will close the streams.
    EchoContextClose(context);
	EchoContextRelease(context);
}


/* static */ void
_EchoContextHandleTimeOut(EchoContextRef context) {

	// Haven't heard from the client so kill everything.
    EchoContextClose(context);
    EchoContextRelease(context);
}


/* static */ void
_ReadStreamCallBack(CFReadStreamRef inStream, CFStreamEventType type, EchoContextRef context) {

	assert(inStream == context->_inStream);
	
	// Dispatch the event properly.
    switch (type) {
        case kCFStreamEventHasBytesAvailable:
            _EchoContextHandleHasBytesAvailable(context);
            break;
			
        case kCFStreamEventEndEncountered:
			_EchoContextHandleEndEncountered(context);
            break;
       
        case kCFStreamEventErrorOccurred:
			_EchoContextHandleErrorOccurred(context);
			break;
            
        default:
            break;
    }
}


/* static */ void
_WriteStreamCallBack(CFWriteStreamRef outStream, CFStreamEventType type, EchoContextRef context) {

	assert(outStream == context->_outStream);

	// Dispatch the event properly.
    switch (type) {
		case kCFStreamEventCanAcceptBytes:
			_EchoContextHandleCanAcceptBytes(context);
			break;
			
        case kCFStreamEventErrorOccurred:
			_EchoContextHandleErrorOccurred(context);
			break;
            
        default:
            break;
    }
}


/* static */ void
_TimerCallBack(CFRunLoopTimerRef timer, EchoContextRef context) {

	assert(timer == context->_timer);

	// Dispatch the timer event.
	_EchoContextHandleTimeOut(context);
}
