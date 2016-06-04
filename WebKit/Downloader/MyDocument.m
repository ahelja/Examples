/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//
//  MyDocument.m
//  Downloader
//
//  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//

#import "MyDocument.h"

#import <WebKit/WebKit.h>

@implementation MyDocument

- (void)setDownloading:(BOOL)downloading
{
    if (isDownloading != downloading) {
        isDownloading = downloading;
        if (isDownloading) {
            [progressIndicator setIndeterminate:YES];
            [progressIndicator startAnimation:self];
            [downloadCancelButton setKeyEquivalent:@"."];
            [downloadCancelButton setKeyEquivalentModifierMask:NSCommandKeyMask];
            [downloadCancelButton setTitle:@"Cancel"];
            [self setFileName:nil];
        } else {
            [progressIndicator setIndeterminate:NO];
            [progressIndicator setDoubleValue:0];
            [downloadCancelButton setKeyEquivalent:@"\r"];
            [downloadCancelButton setKeyEquivalentModifierMask:0];
            [downloadCancelButton setTitle:@"Download"];
            [download release];
            download = nil;
            receivedContentLength = 0;
        }
    }
}

- (void)cancel
{
    [download cancel];
    [self setDownloading:NO];
}

- (void)open
{    
    if ([openButton state] == NSOnState) {
        [[NSWorkspace sharedWorkspace] openFile:[self fileName]];
    }
}

- (NSWindow *)window
{
	NSWindowController *windowController = [[self windowControllers] objectAtIndex:0];
    return [windowController window];
}

- (IBAction)downloadOrCancel:(id)sender
{
    if (isDownloading) {
        [self cancel];
    } else {
        NSURL *URL = [NSURL URLWithString:[URLField stringValue]];
        if (URL) {
            download = [[WebDownload alloc] initWithRequest:[NSURLRequest requestWithURL:URL] delegate:self];
        }
        if (!download) {
            NSBeginAlertSheet(@"Invalid or unsupported URL", nil, nil, nil, [self window], nil, nil, nil, nil,
                              @"The entered URL is either invalid or unsupported.");
        }
    }
}

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{    
    if (returnCode == NSOKButton) {
        [download setDestination:[sheet filename] allowOverwrite:YES];
    } else {
        [self cancel];
    }
}

#pragma mark NSURLDownloadDelegate methods

- (void)downloadDidBegin:(NSURLDownload *)download
{
    [self setDownloading:YES];
}

- (NSWindow *)downloadWindowForAuthenticationSheet:(WebDownload *)download
{
    return [self window];
}

- (void)download:(NSURLDownload *)theDownload didReceiveResponse:(NSURLResponse *)response
{
    expectedContentLength = [response expectedContentLength];

    if (expectedContentLength > 0) {
        [progressIndicator setIndeterminate:NO];
        [progressIndicator setDoubleValue:0];
    }
}

- (void)download:(NSURLDownload *)theDownload decideDestinationWithSuggestedFilename:(NSString *)filename
{
    if ([[directoryMatrix selectedCell] tag] == 0) {
        NSString *path = [[NSHomeDirectory() stringByAppendingPathComponent:@"Desktop"] stringByAppendingPathComponent:filename];
        [download setDestination:path allowOverwrite:NO];
    } else {
        [[NSSavePanel savePanel] beginSheetForDirectory:NSHomeDirectory()
                                                   file:filename
                                         modalForWindow:[self window]
                                          modalDelegate:self
                                         didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:)
                                            contextInfo:nil];
    }
}

- (void)download:(NSURLDownload *)theDownload didReceiveDataOfLength:(unsigned)length
{
    if (expectedContentLength > 0) {
        receivedContentLength += length;
        [progressIndicator setDoubleValue:(double)receivedContentLength / (double)expectedContentLength];
    }
}

- (BOOL)download:(NSURLDownload *)download shouldDecodeSourceDataOfMIMEType:(NSString *)encodingType;
{
    return ([decodeButton state] == NSOnState);
}

- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{
    [self setFileName:path];
}

- (void)downloadDidFinish:(NSURLDownload *)theDownload
{
    [self setDownloading:NO];
    [self open];
}

- (void)download:(NSURLDownload *)theDownload didFailWithError:(NSError *)error
{
    [self setDownloading:NO];
        
    NSString *errorDescription = [error localizedDescription];
    if (!errorDescription) {
        errorDescription = @"An error occured during download.";
    }
    
    NSBeginAlertSheet(@"Download Failed", nil, nil, nil, [self window], nil, nil, nil, nil, errorDescription);
}

#pragma mark NSDocument methods

- (NSString *)windowNibName
{
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)windowController
{
    [progressIndicator setMinValue:0];
    [progressIndicator setMaxValue:1.0];
}

- (void)close
{
    [self cancel];
    [super close];
}

@end
