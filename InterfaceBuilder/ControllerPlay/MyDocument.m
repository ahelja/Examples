/**
 *	filename: MyDocument.m
 *	created : Thu Jul 10 16:30:36 2003
 *	LastEditDate Was "Thu Jul 10 17:36:18 2003"
 *
 */

/*
 	Copyright (c) 2003 Apple Computer, Inc.
 	All rights reserved.

        IMPORTANT: This Apple software is supplied to you by Apple Computer,
        Inc. ("Apple") in consideration of your agreement to the following terms,
        and your use, installation, modification or redistribution of this Apple
        software constitutes acceptance of these terms.  If you do not agree with
        these terms, please do not use, install, modify or redistribute this Apple
        software.

        In consideration of your agreement to abide by the following terms, and
        subject to these terms, Apple grants you a personal, non-exclusive
        license, under Apple's copyrights in this original Apple software (the
        "Apple Software"), to use, reproduce, modify and redistribute the Apple
        Software, with or without modifications, in source and/or binary forms;
        provided that if you redistribute the Apple Software in its entirety and
        without modifications, you must retain this notice and the following text
        and disclaimers in all such redistributions of the Apple Software.
        Neither the name, trademarks, service marks or logos of Apple Computer,
        Inc. may be used to endorse or promote products derived from the Apple
        Software without specific prior written permission from Apple. Except as
        expressly stated in this notice, no other rights or licenses, express or
        implied, are granted by Apple herein, including but not limited to any
        patent rights that may be infringed by your derivative works or by other
        works in which the Apple Software may be incorporated.

        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
        NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
        IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
        PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
        ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
        SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
        MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
        WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
        LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
        OF SUCH DAMAGE.
*/

#import "MyDocument.h"

@implementation MyDocument

- (id)init
{
    self = [super init];
    if (self) {
            // If an error occurs here, send a [self release] message and return nil.
        contentArray = nil;
    }
    return self;
}

- (void)dealloc
{
    [contentArray release];
    [super dealloc];
}

- (NSString *)windowNibName
{
        // Override returning the nib file name of the document
        // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController
{
    [super windowControllerDidLoadNib:aController];

        // NOTE: If this is a new file, contentArray should still be nil, and the array controller
        // is set to automatically prepare content, so it will automatically create the array.
        // If this is a saved file, the contentArray should be set in the loadDataRepresentation: method.
        //
        // Note that this class' contentArray instance variable is basically used to hold the array between
        // loadDataRepresentation: and windowControllerDidLoadNib:.  When the data is saved out, the array controller
        // is asked for its content array directly.
    if ([self contentArray] != nil) {
        [controller setContent: [self contentArray]];
    }

        // Add any code here that needs to be executed once the windowController has loaded the document's window.
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    NSString *error;

    return [NSPropertyListSerialization dataFromPropertyList: [controller content]
                                                      format: NSPropertyListXMLFormat_v1_0
                                            errorDescription: &error];
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
        // Read in the data with all mutable containers
    NSString *error;

    [self setContentArray:[NSPropertyListSerialization propertyListFromData: data
                                                           mutabilityOption: NSPropertyListMutableContainers
                                                                     format: NULL
                                                           errorDescription: &error]];

    return YES;
}

- (void) setContentArray:(NSMutableArray *)newValue
{
    if (newValue != contentArray) {
        [contentArray release];
        contentArray = [newValue retain];
    }
}

- (NSMutableArray *)contentArray
{
    return contentArray;
}


@end
