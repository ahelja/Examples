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


#import "PostExampleDelegate.h"


static const CFOptionFlags kNetworkEvents = kCFStreamEventOpenCompleted |
                                            kCFStreamEventHasBytesAvailable |
                                            kCFStreamEventEndEncountered |
                                            kCFStreamEventErrorOccurred;


static void
ReadStreamClientCallBack(CFReadStreamRef stream, CFStreamEventType type, void *clientCallBackInfo) {
    // Pass off to the object to handle.
    [((PostExampleDelegate*)clientCallBackInfo) handleNetworkEvent: type];
}


#pragma mark -
#pragma mark Implementation PostExampleDelegate
@implementation PostExampleDelegate


- (IBAction)fetchUrl:(id)sender
{
    CFHTTPMessageRef request;
    CFStreamClientContext ctxt = {0, self, NULL, NULL, NULL};
    NSString* url_string = [urlTextField stringValue];
	NSURL* url;
	NSData* data;
    
    // Don't fetch if there is a fetch in progress.
    if (!_stream)
        [statusTextField setStringValue: @""];
    else {
        [statusTextField setStringValue: @"There is a request in progress."];
        return;
    }
    
    // Make sure there is a string in the text field
    if (!url_string || ![url_string length]) {
        [statusTextField setStringValue: @"Enter a valid URL."];
        return;
    }
    
    // If there is no scheme of http or https, slap "http://" on the front.
    if (![url_string hasPrefix: @"http://"] && ![url_string hasPrefix: @"https://"])
        url_string = [NSString stringWithFormat: @"http://%@", url_string];
    
    // Create a new url based upon the user entered string
    url = [NSURL URLWithString: url_string];
    
    // Make sure it succeeded
    if (!url) {
        [statusTextField setStringValue: @"Enter a valid URL."];
        return;
    }
	
	// Get data for POST body.
	data = [[postTextView string] dataUsingEncoding: NSUTF8StringEncoding];
	
    // Make sure it succeeded
	if (!data) {
		[statusTextField setStringValue: @"Enter valid data to send."];
		return;
	}

    // Create a new HTTP request.
    request = CFHTTPMessageCreateRequest(kCFAllocatorDefault, CFSTR("POST"), (CFURLRef)url, kCFHTTPVersion1_1);
    if (!request) {
        [statusTextField setStringValue: @"Creating the request failed."];
        return;
    }
    
	// Set the body.
	CFHTTPMessageSetBody(request, (CFDataRef)data);
	
    // Create the stream for the request.
    _stream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, request);
    
    // Release the request.  The fetch should've retained it if it
    // is performing the fetch.
    CFRelease(request);

    // Make sure it succeeded.
    if (!_stream) {
        [statusTextField setStringValue: @"Creating the stream failed."];
        return;
    }
    
    // Set the client
    if (!CFReadStreamSetClient(_stream, kNetworkEvents, ReadStreamClientCallBack, &ctxt)) {
        CFRelease(_stream);
        _stream = NULL;
        [statusTextField setStringValue: @"Setting the stream's client failed."];
        return;
    }
    
    // Schedule the stream
    CFReadStreamScheduleWithRunLoop(_stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    
    // Start the HTTP connection
    if (!CFReadStreamOpen(_stream)) {
        CFReadStreamSetClient(_stream, 0, NULL, NULL);
        CFReadStreamUnscheduleFromRunLoop(_stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFRelease(_stream);
        _stream = NULL;
        [statusTextField setStringValue: @"Opening the stream failed."];
        return;
    }
    
    // Clear out the old results
    [resultsTextView scrollRangeToVisible: NSMakeRange(0, 0)];
    [resultsTextView replaceCharactersInRange: NSMakeRange(0, [[resultsTextView string] length])
                     withString: @""];
}


- (void)handleNetworkEvent:(CFStreamEventType)type {
    
    // Dispatch the stream events.
    switch (type) {
        case kCFStreamEventHasBytesAvailable:
            [self handleBytesAvailable];
            break;
            
        case kCFStreamEventEndEncountered:
            [self handleStreamComplete];
            break;
            
        case kCFStreamEventErrorOccurred:
            [self handleStreamError];
            break;
            
        default:
            break;
    }
}


- (void)handleBytesAvailable {

    UInt8 buffer[2048];
    CFIndex bytesRead = CFReadStreamRead(_stream, buffer, sizeof(buffer));
    
    // Less than zero is an error
    if (bytesRead < 0)
        [self handleStreamError];
    
    // If zero bytes were read, wait for the EOF to come.
    else if (bytesRead) {
        
        // This would not work for binary data!  Build a string to add
        // to the results.
        NSString* to_add = [NSString stringWithCString: (char*)buffer length: bytesRead];
        
        // Append and scroll the results field.
        [resultsTextView replaceCharactersInRange: NSMakeRange([[resultsTextView string] length], 0)
                         withString: to_add];
        
        [resultsTextView scrollRangeToVisible: NSMakeRange([[resultsTextView string] length], 0)];
    }
}


- (void)handleStreamComplete {
    
    // Don't need the stream any more, and indicate complete.
    CFReadStreamSetClient(_stream, 0, NULL, NULL);
    CFReadStreamUnscheduleFromRunLoop(_stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    CFReadStreamClose(_stream);
    CFRelease(_stream);
    _stream = NULL;
    [statusTextField setStringValue: @"Fetch is complete."];
}


- (void)handleStreamError {

	CFStreamError error = CFReadStreamGetError(_stream);

    // Lame error handling.  Simply state that an error did occur.
    CFReadStreamSetClient(_stream, 0, NULL, NULL);
    CFReadStreamUnscheduleFromRunLoop(_stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    CFReadStreamClose(_stream);
    CFRelease(_stream);
    _stream = NULL;
    [statusTextField setStringValue: [NSString stringWithFormat: @"Error occurred (%ld, %ld).", error.domain, error.error]];
}


@end
