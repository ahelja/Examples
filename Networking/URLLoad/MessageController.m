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

#import "MessageController.h"

@implementation MessageController
- init {
    if (self = [super init]) {
        headers = [[NSMutableDictionary alloc] init];
        headerOrder = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)awakeFromNib {
    [headerTable setAllowsMultipleSelection:NO];
    [headerTable setAllowsColumnReordering:NO];
    [headerTable setDrawsGrid:YES];
    [headerTable setAllowsColumnSelection:NO];
}

- (IBAction)addHeader:(id)sender {
    NSString *newHeader = @"Header";
    if ([headerOrder indexOfObject:newHeader] != NSNotFound) {
        int i = 0;
        do {
            newHeader = [NSString stringWithFormat:@"Header %d", i];
            i ++;
        } while ([headerOrder indexOfObject:newHeader] != NSNotFound);
    }
    [headerOrder addObject:newHeader];
    [headers setObject:@"Value" forKey:newHeader];
    [headerTable reloadData];
    [headerTable selectRow:[headerOrder count] - 1 byExtendingSelection:NO];
}

- (IBAction)removeHeader:(id)sender {
    int i = [headerTable selectedRow];
    if (i != NSNotFound) {
        NSString *header = [headerOrder objectAtIndex:i];
        [headers removeObjectForKey:header];
        [headerOrder removeObjectAtIndex:i];
        [headerTable reloadData];
    }
}

- (int)numberOfRowsInTableView:(NSTableView *)tableView {
    return [headerOrder count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row {
    NSString *header = [headerOrder objectAtIndex:row];
    if ([[tableColumn identifier] isEqual:@"Header"]) {
        return header;
    } else {
        return [headers objectForKey:header];
    }
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(int)row {
    NSString *oldHeader = [headerOrder objectAtIndex:row];
    if ([[tableColumn identifier] isEqual:@"Header"]) {
        if (![oldHeader isEqual:object]) {
            [headers setObject:[headers objectForKey:oldHeader] forKey:object];
            [headers removeObjectForKey:oldHeader];
            [headerOrder replaceObjectAtIndex:row withObject:object];
        }
    } else if (![[headers objectForKey:oldHeader] isEqual:object]) {
        [headers setObject:object forKey:oldHeader];
    }
}

- (CFHTTPMessageRef)requestForURL:(NSURL *)url {
    CFHTTPMessageRef request;
    NSString *requestMethod = [requestTextField stringValue];
    int i;
    if (!requestMethod || [requestMethod length] == 0) {
        requestMethod = @"GET";
    }
    if ([httpVersionMatrix selectedColumn] == 0) {
        request = CFHTTPMessageCreateRequest(NULL, (CFStringRef)requestMethod, (CFURLRef)url, kCFHTTPVersion1_0); 
    } else {
        request = CFHTTPMessageCreateRequest(NULL, (CFStringRef)requestMethod, (CFURLRef)url, kCFHTTPVersion1_1); 
    }
    for (i = 0; i < [headerOrder count]; i ++) {
        NSString *header = [headerOrder objectAtIndex:i];
        NSString *value = [headers objectForKey:header];
        CFHTTPMessageSetHeaderFieldValue(request, (CFStringRef)header, (CFStringRef)value);
    }
    if ([[payloadPathTextField stringValue] length] != 0) {
        NSData *payload = [[NSData alloc] initWithContentsOfFile:[payloadPathTextField stringValue]];
        CFHTTPMessageSetBody(request, (CFDataRef)payload);
        [payload release];
    }
    return request;
}

- (NSDictionary *) headers {
    return headers;
}

- (void)openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    if (returnCode == NSOKButton) {
        [payloadPathTextField setStringValue:[sheet filename]];
    }
}

- (IBAction)setPayloadFile:(id)sender {
    NSOpenPanel *op = [NSOpenPanel openPanel];
    [op setAllowsMultipleSelection:NO];
    [op setCanChooseDirectories:NO];
    [op setResolvesAliases:YES];
    [op beginSheetForDirectory:@"/" file:nil types:nil modalForWindow:[payloadPathTextField window] modalDelegate:self didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

@end
