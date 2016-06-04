/*
 *  POSTExampleDelegate.h
 *  POST Example
 *
 *  Created by Jeremy Wyld on Dec 05 2001.
 *  Copyright (c) 2001, 2002 Apple Computer, Inc. All rights reserved.
 *
 */


#import <Cocoa/Cocoa.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreServices/CoreServices.h>


@interface PostExampleDelegate : NSObject
{
    IBOutlet id resultsTextView;
    IBOutlet id statusTextField;
    IBOutlet id urlTextField;
	IBOutlet id postTextView;
    
    CFReadStreamRef		_stream;
}

- (IBAction)fetchUrl:(id)sender;

- (void)handleNetworkEvent:(CFStreamEventType)type;

- (void)handleBytesAvailable;
- (void)handleStreamComplete;
- (void)handleStreamError;

@end
