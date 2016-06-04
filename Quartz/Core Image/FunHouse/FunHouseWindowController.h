// FunHouseWindowController.h
// Author: Mark Zimmer
// 12/08/04
// Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.

#import <AppKit/AppKit.h>

@class CoreImageView;
@class FunHouseDocument;

@interface FunHouseWindowController : NSWindowController
{
    @private
    IBOutlet CoreImageView *coreImageView;  // pointer to the core image view buried in our owned window view structure
    FunHouseDocument *fhdoc;                // back pointer to the owning document
}

- (id)initFullScreen;
- (CoreImageView *)coreImageView;
- (void)prepFullScreenWindow;
- (void)configureToSize:(NSSize)size andFilename:(NSString *)filename;

@end
