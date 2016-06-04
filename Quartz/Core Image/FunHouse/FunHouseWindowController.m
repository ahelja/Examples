// FunHouseWindowController.m
// Author: Mark Zimmer
// 12/08/04
// Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.

#import "FunHouseWindowController.h"
#import "CoreImageView.h"
#import "EffectStackController.h"
#import "FunHouseDocument.h"
#import "EffectStack.h"
#import <ApplicationServices/ApplicationServices.h>
#import <Carbon/Carbon.h>

// this is a subclass of NSWindowController

@implementation FunHouseWindowController

// standard window init
- (id)init
{
    self = [super initWithWindowNibName:@"FunHouseWindow"];
    return self;
}

- (void)setUpCoreImageView
{
    [coreImageView setNeedsDisplay:YES];
}

// this is an official init procedure for full screen windwo use
- (id)initFullScreen
{
    NSWindow *w;
    NSRect frame;
    
    // set up the system to hide the menu bar
    SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);
    // get the frame of the screen
    frame = [[NSScreen mainScreen] frame];
    // create a new borderless window the size of the entire screen
    w = [[NSWindow alloc] initWithContentRect:frame
      styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:YES];
    // put it up front
    [w makeKeyAndOrderFront:self];
    // and initialize with this window
    self = [super initWithWindow:w];
    return self;
}

- (void)prepFullScreenWindow
{
    NSWindow *w;
    
    w = [self window];
    // create a new core image view (the size of the entire content view) for the full screen window
    coreImageView = [[[CoreImageView alloc] initWithFrame:[[w contentView] bounds]] autorelease];
    // tie us in as its controller
    [coreImageView setFunHouseWindowController:self];
    [self setUpCoreImageView];
    // add the core image view as a subview of the content view
    [[w contentView] addSubview:coreImageView];
    // force an initialization of the core image view
    [coreImageView awakeFromNib];
    // and make the view the first responder (for mouseEntered and mouseExited events...)
    [w makeFirstResponder:coreImageView];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    // Balance the -setFunHouseWindowController: that our -windowDidLoad does.
    [coreImageView setFunHouseWindowController:nil];
    [super dealloc];
}

// this gets called when a typical window is loaded from the FunHouseWindow.nib file
- (void)windowDidLoad
{
    float xwiden, ywiden, xscale, yscale, scale;
    CIImage *im;
    CGSize imagesize;
    NSSize screensize;
    NSRect R;
    
    [super windowDidLoad];
    // set up a backpointer from the core image view to us
    [coreImageView setFunHouseWindowController:self];
    [self setUpCoreImageView];
    // this is required for mouseEntered and mouseExited events
    [[self window] makeFirstResponder:coreImageView];
    // resize window to match file size
    im = [[fhdoc effectStack] baseImage];
    if (im != nil)
    {
        imagesize = [im extent].size;
        // if the image is too large to fit on screen in a document window, we must apply a view transform
        screensize = [[NSScreen mainScreen] frame].size;
        R = [NSWindow frameRectForContentRect:NSMakeRect(0, 0, 100, 100) styleMask:NSTitledWindowMask];
        xwiden = R.size.width - 100 - R.origin.x;
        ywiden = R.size.height - 100 - R.origin.y;
        screensize.width -= xwiden;
        screensize.height -= ywiden;
        if (imagesize.width > screensize.width || imagesize.height > screensize.height)
        {
            // compute scale needed
            xscale = screensize.width / imagesize.width;
            yscale = screensize.height / imagesize.height;
            scale = (yscale < xscale) ? yscale : xscale;
            imagesize.width *= scale;
            imagesize.height *= scale;
            imagesize.width = ceil(imagesize.width);
            imagesize.height = ceil(imagesize.height);
        }
        else
            scale = 1.0;
        [[self window] setContentSize:NSMakeSize(imagesize.width, imagesize.height)];
        // set up view transform using scale
        [coreImageView setViewTransformScale:scale];
        [coreImageView setViewTransformOffsetX:0.0 andY:0.0];
    }
}

// set up a document backpointer
- (void)setDocument:(NSDocument *)document
{
    [super setDocument:document];
    fhdoc = (FunHouseDocument *)document;
    [self setUpCoreImageView];
}

// we return a pointer to the core image view buried in our owned window view structure
- (CoreImageView *)coreImageView
{
    return coreImageView;
}

// required for implementing undo
- (NSUndoManager *)windowWillReturnUndoManager
{
    return [[self document] undoManager];
}

- (void)configureToSize:(NSSize)size andFilename:(NSString *)filename
{
    NSRect frame, frame2;
    
    frame = [[self window] frameRectForContentRect:NSMakeRect(0.0, 0.0, size.width, size.height)];
    frame2 = [[self window] frame];
    frame.origin.x = frame2.origin.x;
    frame.origin.y = frame2.origin.y + frame2.size.height - frame.size.height;
    [[self window] setFrame:frame display:YES];
}
@end
