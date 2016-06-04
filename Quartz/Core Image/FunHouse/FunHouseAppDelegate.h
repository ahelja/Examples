// FunHouseAppDelegate.h
// Author: Mark Zimmer
// 12/08/04
// Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.

#import <AppKit/AppKit.h>

@interface FunHouseAppDelegate : NSObject
{
}

- (IBAction)showEffectStackAction:(id)sender;
- (IBAction)zoomToFullScreenAction:(id)sender;
- (IBAction)undo:(id)sender;
- (IBAction)redo:(id)sender;

@end
