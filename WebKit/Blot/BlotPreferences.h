//
//  BlotPreferences.h
//  Blot
//
//  Created by Ken Kocienda on Thu Jan 29 2004.
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import <AppKit/NSWindowController.h>

#import "BlotApp.h"

extern NSString *const BlotDefaultEditingFilePathPreferenceKey;
extern NSString *const BlotInlineSpellCheckingPreferenceKey;
extern NSString *const BlotPreferencesChangedNotification;

@interface BlotPreferences : NSWindowController
{
    IBOutlet NSTextField *defaultEditingFilePath;
}

+ (BlotPreferences *)sharedPreferences;
- (IBAction)defaultEditingFilePathChanged:sender;

@end
