//
//  BlotApp.h
//  Blot
//
//  Created by John Sullivan on Mon Jan 19 2004.
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface BlotApp : NSApplication 
{
    IBOutlet NSPanel *operationsPanel;
}

- (IBAction)showPreferences:(id)sender;
- (IBAction)debugShowOperations:(id)sender;

@end
