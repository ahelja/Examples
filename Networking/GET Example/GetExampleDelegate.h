/*
 *  GetExampleDelegate.h
 *  GET Example
 *
 *  Created by Jeremy Wyld on Dec 05 2001.
 *  Copyright (c) 2001, 2002 Apple Computer, Inc. All rights reserved.
 *
 */


#import <Cocoa/Cocoa.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreServices/CoreServices.h>


@interface GetExampleDelegate : NSObject
{
    IBOutlet id authName;			// Username text field
    IBOutlet id authPanel;			// User/pass prompt panel
    IBOutlet id authPass;			// Password text field
    IBOutlet id authPrompt;			// Text prompt for user/pass
    IBOutlet id authRealm;			// Informative text for user display the authentication realm
    IBOutlet id realmPrompt;		// "Realm" or "Domain" prompt ("Domain" for NTLM)
    IBOutlet id domainField;		// Domain text field (for NTLM)

    IBOutlet id resultsTextView;
    IBOutlet id statusTextField;
    IBOutlet id urlTextField;
    
    NSURL*				_url;
    CFReadStreamRef		_stream;
    CFHTTPMessageRef	_request;
}

- (IBAction)fetchUrl:(id)sender;
- (IBAction)refetchUrl:(id)sender;

- (void)fetch:(CFHTTPMessageRef)request;

- (void)handleNetworkEvent:(CFStreamEventType)type;

- (void)handleBytesAvailable;
- (void)handleStreamComplete;
- (void)handleStreamError;

@end
