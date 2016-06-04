/*
 PicBrowserController.m
 NSObject subclass showing the use of NSNetServices to discover a lightweight thumbnail sharing service on the network.
 
 Chris Parker
 
 Copyright (c) 2002-2004, Apple Computer, Inc., all rights reserved.
 */

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

#import "PicBrowserController.h"

@implementation PicBrowserController

- (IBAction)serviceClicked:(id)sender {
    // The row that was clicked corresponds to the object in services we wish to contact.
    int index = [sender selectedRow];
    if (-1 != index && currentDownload == nil) {
        NSNetService * clickedService = [services objectAtIndex:index];
        [hostNameField setStringValue:[clickedService hostName]];
        NSInputStream * istream;
        [clickedService getInputStream:&istream outputStream:nil];
        [istream retain];
        [istream setDelegate:self];
        [istream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [progressIndicator startAnimation:nil];
        [istream open];
    }
}

- (void)awakeFromNib {
    browser = [[NSNetServiceBrowser alloc] init];
    services = [[NSMutableArray array] retain];
    [browser setDelegate:self];
    
    // Passing in "" for the domain causes us to browse in the default browse domain
    [browser searchForServicesOfType:@"_wwdcpic._tcp." inDomain:@""];
    [hostNameField setStringValue:@""];
}

// This object is the delegate of its NSNetServiceBrowser object. We're only interested in services-related methods, so that's what we'll call.
- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didFindService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
    [services addObject:aNetService];
    [aNetService resolveWithTimeout:5.0];
    
    if(!moreComing) {
        [pictureServiceList reloadData];
    }
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didRemoveService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
    [services removeObject:aNetService];
    
    if(!moreComing) {
        [pictureServiceList reloadData];        
    }
}

// This object is the data source of its NSTableView. servicesList is the NSArray containing all those services that have been discovered.
- (int)numberOfRowsInTableView:(NSTableView *)theTableView {
    return [services count];
}


- (id)tableView:(NSTableView *)theTableView objectValueForTableColumn:(NSTableColumn *)theColumn row:(int)rowIndex {
    return [[services objectAtIndex:rowIndex] name];
}

#pragma mark NSStream delegate method

- (void)stream:(NSStream *)aStream handleEvent:(NSStreamEvent)event {
    switch(event) {
        case NSStreamEventHasBytesAvailable:
            if (!currentDownload) {
                currentDownload = [[NSMutableData alloc] initWithCapacity:409600];
            }
            uint8_t readBuffer[4096];
            int amountRead = 0;
            NSInputStream * is = (NSInputStream *)aStream;
            amountRead = [is read:readBuffer maxLength:4096];
            [currentDownload appendBytes:readBuffer length:amountRead];
            break;
        case NSStreamEventEndEncountered:
            [(NSInputStream *)aStream close];
            NSImage * newImage = [[NSImage alloc] initWithData:currentDownload];
            [imageView setImage:newImage];
            [newImage release];
            [currentDownload release];
            currentDownload = nil;
            [progressIndicator stopAnimation:nil];
            break;
        default:
            break;
    }
}

@end
