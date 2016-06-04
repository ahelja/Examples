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

#import <CoreFoundation/CFStream.h>
#import "URLAccessHandle.h"
#import "URLController.h"

@implementation URLController
- (id)init {
    if (self = [super init]) {
        writeStreams = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void)dealloc {
    [writeStreams release];
	[super dealloc];
}

- (void)appendStatusString:(NSString *)str forURL:(NSURL *)url {
    NSTextStorage *ts = [statusTextView textStorage];
    NSString *fullString = [NSString stringWithFormat:@"%@: %@\n", [url absoluteString], str];
    [ts replaceCharactersInRange:NSMakeRange([ts length], 0) withString:fullString];
}

-(IBAction)clear:(id)sender {
    NSTextStorage *ts = [statusTextView textStorage];
    [ts replaceCharactersInRange:NSMakeRange(0, [ts length]) withString:@""];
}

- (IBAction)go:(id)sender {
    NSURL *url = [NSURL URLWithString:[urlTextField stringValue]];
    NSString *title = [[sourceRadio selectedCell] title];
    if ([title isEqual:@"CFStream"]) {
        [self beginCFStreamDownload:url toFile:nil];
    } else {
        if ([title isEqual:@"URLAccess"]) {
            [URLAccessHandle acceptURL:url];
        }
        [url loadResourceDataNotifyingClient:self usingCache:[cacheCheckbox state]];
    }
}

- (IBAction)save:(id)sender {
    NSURL *url = [NSURL URLWithString:[urlTextField stringValue]];
    NSString *lastPathComp = [(id)CFURLCopyLastPathComponent((CFURLRef)url) autorelease];
    NSString *title;
    NSString *fileName;
    if (!url) {
        [self appendStatusString:[NSString stringWithFormat:@"Could not create URL for %@", [urlTextField stringValue]] forURL:nil];
        return;
    }
    if (!lastPathComp || [lastPathComp length] == 0 || [lastPathComp isEqualToString:@"/"]) {
        lastPathComp = @"urlLoad";
    }
    fileName = [NSString stringWithFormat:@"/tmp/%@", lastPathComp];

    title = [[sourceRadio selectedCell] title];
    if ([title isEqual:@"CFStream"]) {
        [self beginCFStreamDownload:url toFile:fileName];
    } else {
        NSData *data;
        if ([title isEqual:@"URLAccess"]) {
            [URLAccessHandle acceptURL:url];
        }
        data = [url resourceDataUsingCache:[cacheCheckbox state]];
        if (!data) {
            [self appendStatusString:@"No data at URL" forURL:url];
        } else if ([data writeToFile:fileName atomically:NO]) {
            [self appendStatusString:[NSString stringWithFormat:@"Saved to file %@", fileName] forURL:url];
        } else {
            [self appendStatusString:[NSString stringWithFormat:@"Couldn't write to file %@", fileName] forURL:url];
        }
    }
}

/* NSURLClient category */
- (void)URL:(NSURL *)sender resourceDataDidBecomeAvailable:(NSData *)newBytes {
    [self appendStatusString:[NSString stringWithFormat:@"Received %d bytes", [newBytes length]] forURL:sender];
}

- (void)URLResourceDidFinishLoading:(NSURL *)sender {
    [self appendStatusString:@"Finished loading" forURL: sender];
}

- (void)URLResourceDidCancelLoading:(NSURL *)sender {
    [self appendStatusString:@"Load cancelled" forURL:sender];
}

- (void)URL:(NSURL *)sender resourceDidFailLoadingWithReason:(NSString *)reason {
    [self appendStatusString:[NSString stringWithFormat:@"Load failed - %@", reason] forURL:sender];
}

/* CFStream callback*/
void streamLoadCB(CFReadStreamRef stream, CFStreamEventType type, void *info);

-(void)beginCFStreamDownload:(NSURL *)inURL toFile:(NSString *)file {
    CFHTTPMessageRef request;
    CFReadStreamRef readStream;
    Boolean fail = FALSE;
	
	request = [msgController requestForURL:inURL];
	if (request == nil) {
		[self appendStatusString:[NSString stringWithFormat:@"Could not create HTTP request for URL: %@", inURL] forURL:inURL];	
	}
	else
	{
		if ([proxyController proxyType] != kProxyNone) {
			NSMutableDictionary* proxyDict = [NSMutableDictionary dictionaryWithCapacity: 2];
			[proxyDict setObject: [proxyController host] forKey: (id)kCFStreamPropertyHTTPProxyHost];
			[proxyDict setObject: [NSNumber numberWithInt: [proxyController port]] forKey: (id)kCFStreamPropertyHTTPProxyPort];
			if ([proxyController username]) {
				CFHTTPMessageAddAuthentication(request, NULL, (CFStringRef)[proxyController username], (CFStringRef)[proxyController password], kCFHTTPAuthenticationSchemeBasic, TRUE);
			}
			readStream = CFReadStreamCreateForHTTPRequest(NULL, request);
			CFReadStreamSetProperty(readStream, kCFStreamPropertyHTTPProxy, proxyDict);
		} else {
			readStream = CFReadStreamCreateForHTTPRequest(NULL, request);
		}
		CFRelease(request);
		CFReadStreamSetProperty(readStream, kCFStreamPropertyHTTPShouldAutoredirect, kCFBooleanTrue);
		
		if (file) {
			NSURL *url = [NSURL fileURLWithPath:file];
			CFWriteStreamRef writeStream = CFWriteStreamCreateWithFile(NULL, (CFURLRef)url);
			if (writeStream) {
				CFWriteStreamOpen(writeStream);
				[writeStreams setObject:(id)writeStream forKey:inURL];
				[self appendStatusString:[NSString stringWithFormat:@"writing to %@", file] forURL:url];
				CFRelease(writeStream);
			} else {
				[self appendStatusString:[NSString stringWithFormat:@"Could not open output stream to %@", file] forURL:url];
				fail = TRUE;
			}
		}
		if (!fail) {
			if (!CFReadStreamOpen(readStream)) {
				CFStreamError error = CFReadStreamGetError(readStream);
				[self appendStatusString:[NSString stringWithFormat:@"Could not open read stream (%d, %d)", error.domain, error.error] forURL:inURL];
				CFRelease(readStream);
			} else {
				CFStreamClientContext callbackContext = {0, self, NULL, NULL, NULL};
				CFReadStreamSetClient(readStream, kCFStreamEventOpenCompleted | kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, streamLoadCB, &callbackContext);
				CFReadStreamScheduleWithRunLoop(readStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
			}
		}
	}
}

- (void)streamEndEncountered:(CFReadStreamRef)stream forURL:(NSURL *)url {
    CFHTTPMessageRef responseHeaders = (CFHTTPMessageRef)CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);
    CFDictionaryRef dict = responseHeaders ? CFHTTPMessageCopyAllHeaderFields(responseHeaders) : NULL;
    [self appendStatusString:@"download complete" forURL:url];
    [self appendStatusString:[NSString stringWithFormat:@"response headers %@, %@", responseHeaders ? CFCopyDescription(responseHeaders) : CFSTR("<nil>"), dict] forURL:url];
    if (dict) CFRelease(dict);
    if (responseHeaders) CFRelease(responseHeaders);

}

- (void)handleEvent:(CFStreamEventType)type forStream:(CFReadStreamRef)stream {
    NSURL *url = (NSURL*)CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPFinalURL);
    CFWriteStreamRef writeStream = (CFWriteStreamRef)[writeStreams objectForKey:url];
    switch (type) {
    case kCFStreamEventOpenCompleted:
        [self appendStatusString:[NSString stringWithFormat:@"stream 0x%x opened", stream] forURL:url];
        break;
    case kCFStreamEventEndEncountered:
        [self streamEndEncountered:stream forURL:url];
        break;
    case kCFStreamEventHasBytesAvailable:
    {
        UInt8 buf[512];
        CFIndex totalBytesRead = 0, bytesRead;
        CFIndex totalBytesWritten = 0;
        CFStreamStatus status;
        do {
            bytesRead = CFReadStreamRead(stream, buf, 512); 
            totalBytesRead += bytesRead;
            if (writeStream) {
                totalBytesWritten += CFWriteStreamWrite(writeStream, buf, bytesRead);
            }
        } while (bytesRead == 512);
        if (writeStream) {
            [self appendStatusString:[NSString stringWithFormat:@"read %d bytes; wrote %d bytes", totalBytesRead, totalBytesWritten] forURL:url];
        } else {
            [self appendStatusString:[NSString stringWithFormat:@"read %d bytes", totalBytesRead] forURL:url];
        }        
        status = CFReadStreamGetStatus(stream);
        if (status == kCFStreamStatusAtEnd || status == kCFStreamStatusError) {
            if (status == kCFStreamStatusAtEnd) {
                [self streamEndEncountered:stream forURL:url];
            } else {
                CFStreamError error = CFReadStreamGetError(stream);
                [self appendStatusString:[NSString stringWithFormat:@"error occurred <domain = %d; error = %d>", error.domain, error.error] forURL:url];
            }
            if (writeStream) {
                CFWriteStreamClose(writeStream);
                [writeStreams removeObjectForKey:url];
            }
            CFReadStreamClose(stream);
            CFReadStreamUnscheduleFromRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
            [(id)stream autorelease];
        }
        break;
    }
    case kCFStreamEventErrorOccurred: {
        CFStreamError error = CFReadStreamGetError(stream);
        [self appendStatusString:[NSString stringWithFormat:@"error <domain = %d; error = %d> detected", error.domain, error.error] forURL:url];
        [(id)stream autorelease];
        if (writeStream) {
            CFWriteStreamClose(writeStream);
            [writeStreams removeObjectForKey:url];
        }
        CFReadStreamUnscheduleFromRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        break;
    }
    default:
        [self appendStatusString:[NSString stringWithFormat:@"Received unexpected CFStream event <%d>", type] forURL:url];
    }
}
@end

@interface URLController (_HandleEvent)
- (void)handleEvent:(CFStreamEventType)type forStream:(CFReadStreamRef)stream;
@end

void streamLoadCB(CFReadStreamRef stream, CFStreamEventType type, void *info) {
    URLController *self = (URLController *)info;
    [self handleEvent:type forStream:stream];
}
