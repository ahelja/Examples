//
//  AppController.m
//
//  Copyright (c) 2001-2002, Apple. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppController : NSObject {
@private
    IBOutlet NSBrowser    *fsBrowser;
    IBOutlet NSImageView  *nodeIconWell;  // Image well showing the selected items icon.
    IBOutlet NSTextField  *nodeInspector; // Text field showing the selected items attributes.
}

// Force a reload of column zero and thus, all the data.
- (IBAction)reloadData:(id)sender;

// Methods sent by the browser to us from theBrowser.
- (IBAction)browserSingleClick:(id)sender;
- (IBAction)browserDoubleClick:(id)sender;

@end
