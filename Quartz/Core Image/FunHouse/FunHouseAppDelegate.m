// FunHouseAppDelegate.m
// Author: Mark Zimmer
// 12/08/04
// Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.

#import "FunHouseAppDelegate.h"
#import "EffectStackController.h"

@implementation FunHouseAppDelegate

// application delegates implement this to accomplish tasks right after the application launches
// in this case, I copy the example images from the application package to the ~/Library/Application Support/Core Image Fun House/Example Images folder
// we also automatically open a file on launch if we're not already opening one (by drag or double-click)
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    BOOL isDir;
    int i, count;
    NSError *err;
    NSOpenPanel *op;
    NSString *path, *path2, *source, *file, *sourcefile, *destfile;
    NSBundle *bundle;
    NSFileManager *manager;
    NSArray *files;
    
    [self showEffectStackAction:self];
    // decide if images have yet been copied to ~/Library/Application Support/Core Image Fun House/Example Images
    path = @"~/Library/Application Support/Core Image Fun House";
    path = [path stringByExpandingTildeInPath];
    path2 = [path stringByAppendingString:@"/Example Images Folder"];
    manager = [NSFileManager defaultManager];
    if (![manager fileExistsAtPath:path isDirectory:&isDir])
    {
        // otherwise we need to create the ~/Library/Application Support/Core Image Fun House folder
        [manager createDirectoryAtPath:path attributes:nil];
        [manager createDirectoryAtPath:path2 attributes:nil];
        bundle = [NSBundle bundleForClass:[self class]];
        source = [[bundle resourcePath] stringByAppendingString:@"/Images"];
        // and copy the files
        files = [manager directoryContentsAtPath:source];
        count = [files count];
        for (i = 0; i < count; i++)
        {
            file = [files objectAtIndex:i];
            sourcefile = [[source stringByAppendingString:@"/"] stringByAppendingString:file];
            destfile = [[path2 stringByAppendingString:@"/"] stringByAppendingString:file];
            [manager copyPath:sourcefile toPath:destfile handler:nil];
        }
    }
    // only automatically open a file if none has already been opened
    if ([[[NSDocumentController sharedDocumentController] documents] count] == 0)
    {
        // open a file at launch from the ~/Library/Application Support/Core Image Fun House/Example Images folder
        op = [NSOpenPanel openPanel];
        [op setAllowsMultipleSelection:NO];
        if ([op runModalForDirectory:path2 file:nil types:[NSArray arrayWithObjects:@"jpeg", @"jpg", @"tiff", @"tif", @"png", @"crw", @"cr2", @"raf", @"mrw", @"nef", @"srf", @"exr", @"funhouse", nil]] == NSOKButton)
            [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:[[op URLs] objectAtIndex:0] display:YES error:&err];
    }
}

// handle the show effect stack menu item action
- (IBAction)showEffectStackAction:(id)sender
{
    [[EffectStackController sharedEffectStackController] showWindow:sender];
}

// don't open an untitled file at the start
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
    return NO;
}

// this gets called when gthe application is just ready to quit
- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    // close down the effect stack controller so that it's position will be saved in a closed state
    // note: if it's not closed down when the application terminates, then the window will precess
    // down the screen the next time the application is launched
    [[EffectStackController sharedEffectStackController] closeDown];
}

// handle the zoom to full screen menu item action
- (IBAction)zoomToFullScreenAction:(id)sender
{
    [[[NSDocumentController sharedDocumentController] currentDocument] zoomToFullScreenAction:sender];
}

// handle the undo menu item action
- (IBAction)undo:(id)sender
{
    [[[NSDocumentController sharedDocumentController] currentDocument] undo];
}

// handle the redo menu item action
- (IBAction)redo:(id)sender
{
    [[[NSDocumentController sharedDocumentController] currentDocument] redo];
}

// validate (enable/disable) undo and redo menu items
- (BOOL)validateMenuItem:(NSMenuItem *)item
{
    NSUndoManager *um = [[[NSDocumentController sharedDocumentController] currentDocument] undoManager];
    if ([[[item menu] title] isEqualToString:@"Edit"])
    {
        if ([[[item title] substringToIndex:4] isEqualToString:@"Undo"])
        {
            [item setTitle:[[um undoMenuItemTitle] substringFromIndex:1]];
            return [um canUndo];
        }
        else if ([[[item title] substringToIndex:4] isEqualToString:@"Redo"])
        {
            [item setTitle:[[um redoMenuItemTitle] substringFromIndex:1]];
            return [um canRedo];
        }
    }
    return YES;
}

@end
