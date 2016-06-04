// FunHouseDocument.h
// Author: Mark Zimmer
// 12/08/04
// Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.

#import <Cocoa/Cocoa.h>

@class EffectStack;
@class FunHouseWindowController;

@interface FunHouseDocument : NSDocument
{
    EffectStack *effectStack;                           // the effect stack we own (the document's data)
    BOOL fullScreen;                                    // YES if this document is in full screen, NO if it's just a typical window
    FunHouseWindowController *windowController;         // standard (typical) window controller
    FunHouseWindowController *fullScreenController;     // (full screen) window controller
    CGColorSpaceRef colorspace;
}

- (EffectStack *)effectStack;
- (BOOL)openPreset:(NSFileWrapper *)fileWrapper error:(NSError **)outError;
- (IBAction)zoomToFullScreenAction:(id)sender;
- (void)undo;
- (void)redo;
- (void)reconfigureWindowToSize:(NSSize)size andPath:(NSString *)path;
@end
