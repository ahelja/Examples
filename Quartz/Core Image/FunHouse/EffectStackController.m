// EffectStackController.m
// Author: Mark Zimmer
// 12/08/04
// Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.

#import "EffectStackController.h"
#import "FunHouseWindowController.h"
#import "FunHouseDocument.h"
#import "FunHouseApplication.h"
#import "FilterView.h"
#import "EffectStack.h"
#import "CoreImageView.h"
#import "ParameterView.h"

// this class controls the automatically resizeable effect stack inspector
// it must also be able to resize and reconfigure itself when switching documents

@implementation EffectStackController

// since the effect stack inspector window is global to all documents, we here provide a way of accessing the shared window
+ (id)sharedEffectStackController
{
    static EffectStackController *_sharedEffectStackController = nil;

    if (!_sharedEffectStackController)
        _sharedEffectStackController = [[EffectStackController allocWithZone:[self zone]] init];
    return _sharedEffectStackController;
}

// load from nib (really only the stuff at the top of the inspector)
- (id)init
{
    self = [self initWithWindowNibName:@"EffectStack"];
    if (self)
    {
        [self setWindowFrameAutosaveName:@"EffectStack"];
        // set up an array to hold the representations of the layers from the effect stack we inspect
        boxes = [[NSMutableArray arrayWithCapacity:10] retain];
        needsUpdate = YES;
    }
    return self;
}

// free up the stuff we allocate
- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [boxes release];
    [super dealloc];
}

// this allows us to set up the right pointers when changing documents
// in particular the core image view and the effect stack
- (void)setMainWindow:(NSWindow *)mainWindow
{
    NSWindowController *controller;
    FunHouseDocument *document;
    
    // note: if mainWindow is nil, then controller becomes nil here too
    controller = [mainWindow windowController];
    if (controller && [controller isKindOfClass:[FunHouseWindowController class]])
    {
        // we have a core image fun house document window (by controller)
        // get the core image view pointer from it
        _inspectingCoreImageView = [(FunHouseWindowController *)controller coreImageView];
        // load up the FunHouseDocument pointer
        document = (FunHouseDocument *)[controller document];
        // and get the effect stack pointer from it
        _inspectingEffectStack = [document effectStack];
    }
    else
    {
        // we inspect nothing at the moment
        _inspectingCoreImageView = nil;
        _inspectingEffectStack = nil;
    }
    [self updateLayout];
}

// reset the core image view (used when going to into full screen mode and back out)
- (void)setCoreImageView:(CoreImageView *)v
{
    _inspectingCoreImageView = v;
}

// flag that we need to reconfigure ourselves after some effect stack change
- (void)setNeedsUpdate:(BOOL)b
{
    needsUpdate = b;
}

- (void)enablePlayButton
{
    BOOL enabled;
    int i, count;
    NSString *type;
    CIFilter *f;
    NSDictionary *attr;
    
    count = [_inspectingEffectStack layerCount];
    enabled = NO;
    for (i = 0; i < count; i++)
    {
        type = [_inspectingEffectStack typeAtIndex:i];
        if (![type isEqualToString:@"filter"])
            continue;
        // first find time slider
        f = [_inspectingEffectStack filterAtIndex:i];
        attr = [f attributes];
        if ([attr objectForKey:@"inputTime"] != nil)
        {
            enabled = YES;
            break;
        }
    }
    [playButton setEnabled:enabled];
}

// when a window loads from the nib file, we set up the core image view pointer and effect stack pointers
// and set up notifications
- (void)windowDidLoad
{
    [super windowDidLoad];
    [self setMainWindow:[NSApp mainWindow]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(mainWindowChanged:) name:NSWindowDidBecomeMainNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(mainWindowResigned:) name:NSWindowDidResignMainNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidUpdate:) name:NSWindowDidUpdateNotification object:nil];
    [[NSColorPanel sharedColorPanel] setShowsAlpha:YES];
}

// when window changes, update the pointers
- (void)mainWindowChanged:(NSNotification *)notification
{
    [self setMainWindow:[notification object]];
}

// dissociate us when the window is gone.
- (void)mainWindowResigned:(NSNotification *)notification
{
    [self setMainWindow:nil];
}

// when we see an update, check for the flag that tells us to reconfigure our effect stack inspection
- (void)windowDidUpdate:(NSNotification *)notification
{
    int i, count;
    CIFilter *filter;
    NSWindow *w;
    NSString *type, *string;
    CIImage *image;
    
    w = (NSWindow *)[notification object];
    if (w != [self window])
        return;
    if (needsUpdate)
    {
        // we need an update
        needsUpdate = NO;
        // remove tthe old boxes from the UI
        count = [boxes count];
        for (i = 0; i < count; i++)
        {
            [[boxes objectAtIndex:i] removeFromSuperview];
        }
        // and clear out the boxes array
        [boxes removeAllObjects];
        // now, if required, automatically generate the effect stack UI into separate boxes for each layer
        if (_inspectingEffectStack != nil)
        {
            // create all boxes shown in the effect stack inspector from scratch, and place them into an array for layout purposes
            count = [_inspectingEffectStack layerCount];
            for (i = 0; i < count; i++)
            {
                type = [_inspectingEffectStack typeAtIndex:i];
                if ([type isEqualToString:@"filter"])
                {
                    filter = [_inspectingEffectStack filterAtIndex:i];
                    [boxes addObject:[self createUIForFilter:filter index:i]];
                }
                else if ([type isEqualToString:@"image"])
                {
                    image = [_inspectingEffectStack imageAtIndex:i];
                    [boxes addObject:[self createUIForImage:image filename:[_inspectingEffectStack filenameAtIndex:i] index:i]];
                }
                else if ([type isEqualToString:@"text"])
                {
                    string = [_inspectingEffectStack stringAtIndex:i];
                    [boxes addObject:[self createUIForText:string index:i]];
                }
            }
        }
        // now lay it out
        [self layoutInspector];
    }
}

// this method brings up the "image units palette" (we call it the filter palette) - and it also has buttons for images and text layers
- (NSDictionary *)collectFilterImageOrText
{
    int i;
    CIImage *im;
    NSString *path;
    NSOpenPanel *op;
    
    // when running the filter palette, if a filter is chosen (as opposed to an image or text) then filterClassname returns the
    // class name of the chosen filter
    [filterClassname release];
    filterClassname = nil;
    // load the nib for the filter palette
    [NSBundle loadNibNamed:@"FilterPalette" owner:self];
    // set up the categories data structure, that enumerates all filters for use by the filter palette
    if (categories == nil)
    {
        categories = [[NSMutableDictionary alloc] init];
        [self _loadFilterListIntoInspector];
    }
    else
        [filterTableView reloadData];
    // set up the usual target-action stuff for the filter palette
    [filterTableView setTarget:self];
    [filterTableView setDoubleAction:@selector(tableViewDoubleClick:)];
    [filterOKButton setEnabled:NO];
    // re-establish the current position in the filters palette
    [categoryTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:currentCategory] byExtendingSelection:NO];
    [filterTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:currentFilterRow] byExtendingSelection:NO];
    // run the modal filter palette now
    i = [NSApp runModalForWindow:filterPalette];
    [filterPalette close];
    if (i == 100)
        // Apply
        // create the filter layer dictionary
        return [NSDictionary dictionaryWithObjectsAndKeys:@"filter", @"type", [CIFilter filterWithName:filterClassname], @"filter", nil];
    else if (i == 101)
        // Cancel
        return nil;
    else if (i == 102)
    {
        // Image
        // use the open panel to open an image
        op = [NSOpenPanel openPanel];
        [op setAllowsMultipleSelection:NO];
        [op setCanChooseDirectories:NO];
        [op setResolvesAliases:YES];
        [op setCanChooseFiles:YES];
        // run the open panel with the allowed types
        int j = [op runModalForTypes:[NSArray arrayWithObjects:@"jpg", @"jpeg", @"tif", @"tiff", @"png", @"crw", @"cr2", @"raf", @"mrw", @"nef", @"srf", @"exr", nil]];
        if (j == NSOKButton)
        {
        
            // get image from open panel
            path = [[op filenames] objectAtIndex:0];
            im = [[[CIImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path]] autorelease];
            // create the image layer dictionary
            return [NSDictionary dictionaryWithObjectsAndKeys:@"image", @"type", im, @"image", [path lastPathComponent], @"filename", path, @"imageFilePath", nil];
        }
        else if (j == NSCancelButton)
            return nil;
    }
    else if (i == 103)
        // Text
        // create the text layer dictionary
        return [NSDictionary dictionaryWithObjectsAndKeys:@"text", @"type", @"text", @"string", [NSNumber numberWithFloat:10.0], @"scale", nil];
    return nil;
}

// get the currently associated document
- (FunHouseDocument *)doc
{
    return (FunHouseDocument *)[[NSDocumentController sharedDocumentController] currentDocument];
}

// set changes (dirty the document)
- (void)setChanges
{
    [[self doc] updateChangeCount:NSChangeDone];
}

// call to directly update (and re-layout) the configuration of the effect stack inspector
- (void)updateLayout
{
    needsUpdate = YES;
    [[self window] update];
}

// this is the glue code you call to insert a filter layer into the effect stack. this handles save for undo, etc.
- (void)insertFilter:(CIFilter *)f atIndex:(NSNumber *)index
{
    // actually insert the filter layer into the effect stack
    [_inspectingEffectStack insertFilterLayer:f atIndex:[index intValue]];
    // set filter attributes to their defaults
    [[_inspectingEffectStack filterAtIndex:[index intValue]] setDefaults];
    // set any automatic defaults we need (generally the odd image parameter)
    [self setAutomaticDefaults:[_inspectingEffectStack filterAtIndex:[index intValue]] atIndex:[index intValue]];
    // do "save for undo"
    [[[[self doc] undoManager] prepareWithInvocationTarget:self] removeFilterImageOrTextAtIndex:index];
    [[[self doc] undoManager] setActionName:[NSString stringWithFormat:@"Filter %@", [CIFilter localizedNameForFilterName:NSStringFromClass([f class])], nil]];
    // dirty the documdent
    [self setChanges];
    // redo the effect stack inspector's layout after the change
    [self updateLayout];
    // finally, let core image render the view
    [_inspectingCoreImageView setNeedsDisplay:YES];
}

// this is the high-level glue code you call to insert an image layer into the effect stack. this handles save for undo, etc.
- (void)insertImage:(CIImage *)image withFilename:(NSString *)filename andImageFilePath:(NSString *)path atIndex:(NSNumber *)index
{
    // actually insert the image layer into the effect stack
    [_inspectingEffectStack insertImageLayer:image withFilename:filename atIndex:[index intValue]];
    [_inspectingEffectStack setImageLayer:[index intValue] imageFilePath:path];
    // do "save for undo"
    [[[[self doc] undoManager] prepareWithInvocationTarget:self] removeFilterImageOrTextAtIndex:index];
    [[[self doc] undoManager] setActionName:[NSString stringWithFormat:@"Image %@", [filename lastPathComponent], nil]];
    // dirty the documdent
    [self setChanges];
    // redo the effect stack inspector's layout after the change
    [self updateLayout];
    // finally, let core image render the view
    [_inspectingCoreImageView setNeedsDisplay:YES];
}

// this is the high-level glue code you call to insert a text layer into the effect stack. this handles save for undo, etc.
- (void)insertString:(NSString *)string withImage:(CIImage *)image atIndex:(NSNumber *)index
{
    // actually insert the text layer into the effect stack
    [_inspectingEffectStack insertTextLayer:string withImage:image atIndex:[index intValue]];
    // do "save for undo"
    [[[[self doc] undoManager] prepareWithInvocationTarget:self] removeFilterImageOrTextAtIndex:index];
    [[[self doc] undoManager] setActionName:@"Text"];
    // dirty the documdent
    [self setChanges];
    // redo the effect stack inspector's layout after the change
    [self updateLayout];
    // finally, let core image render the view
    [_inspectingCoreImageView setNeedsDisplay:YES];
}

// this is the high-level glue code you call to remove a layer (of any kind) from the effect stack. this handles save for undo, etc.
- (void)removeFilterImageOrTextAtIndex:(NSNumber *)index
{
    NSString *type;
    CIFilter *filter;
    CIImage *image;
    NSString *filename, *string, *path;
    NSPoint offset;
    
    // first get handles to parameters we want to retain for "save for undo"
    type = [_inspectingEffectStack typeAtIndex:[index intValue]];
    if ([type isEqualToString:@"filter"])
        filter = [[_inspectingEffectStack filterAtIndex:[index intValue]] retain];
    else if ([type isEqualToString:@"image"])
        {
        image = [[_inspectingEffectStack imageAtIndex:[index intValue]] retain];
        filename = [[_inspectingEffectStack filenameAtIndex:[index intValue]] retain];
        offset = [_inspectingEffectStack offsetAtIndex:[index intValue]];
        path = [_inspectingEffectStack imageFilePathAtIndex:[index intValue]];
        }
    else if ([type isEqualToString:@"text"])
        {
        image = [[_inspectingEffectStack imageAtIndex:[index intValue]] retain];
        string = [[_inspectingEffectStack stringAtIndex:[index intValue]] retain];
        offset = [_inspectingEffectStack offsetAtIndex:[index intValue]];
        }
    // actually remove the layer from the effect stack here
    [_inspectingEffectStack removeLayerAtIndex:[index intValue]];
    // do "save for undo"
    if ([type isEqualToString:@"filter"])
    {
        [[[[self doc] undoManager] prepareWithInvocationTarget:self] insertFilter:filter atIndex:index];
        [[[self doc] undoManager] setActionName:[NSString stringWithFormat:@"Filter %@", [CIFilter localizedNameForFilterName:NSStringFromClass([filter class])], nil]];
        [filter release];
    }
    else if ([type isEqualToString:@"image"])
    {
        [[[[self doc] undoManager] prepareWithInvocationTarget:self] insertImage:image withFilename:filename
          andImageFilePath:path atIndex:index];
        [[[self doc] undoManager] setActionName:[NSString stringWithFormat:@"Image %@", [filename lastPathComponent], nil]];
        [image release];
        [filename release];
    }
    else if ([type isEqualToString:@"string"])
    {
        [[[[self doc] undoManager] prepareWithInvocationTarget:self] insertString:string withImage:image atIndex:index];
        [[[self doc] undoManager] setActionName:@"Text"];
        [image release];
        [string release];
    }
    // dirty the documdent
    [self setChanges];
    // redo the effect stack inspector's layout after the change
    [self updateLayout];
    // finally, let core image render the view
    [_inspectingCoreImageView setNeedsDisplay:YES];
}

// the "global" plus button inserts a layer before the first layer
- (IBAction)topPlusButtonAction:(id)sender
{
    NSDictionary *d;
    
    d = [self collectFilterImageOrText];
    if (d == nil)
        return;
    if ([[d valueForKey:@"type"] isEqualToString:@"filter"])
        [self insertFilter:[d valueForKey:@"filter"] atIndex:[NSNumber numberWithInt:0]];
    else if ([[d valueForKey:@"type"] isEqualToString:@"image"])
        [self insertImage:[d valueForKey:@"image"] withFilename:[d valueForKey:@"filename"]
          andImageFilePath:[d valueForKey:@"imageFilePath"] atIndex:[NSNumber numberWithInt:0]];
    else if ([[d valueForKey:@"type"] isEqualToString:@"text"])
        [self insertString:[d valueForKey:@"string"] withImage:[d valueForKey:@"image"] atIndex:[NSNumber numberWithInt:0]];
    [self enablePlayButton];
}

// this handles a change to each layer's "enable" check box
- (IBAction)enableCheckBoxAction:(id)sender
{
    [_inspectingEffectStack setLayer:[sender tag] enabled:([sender state] == NSOnState)?YES:NO];
    [self setChanges];
    [_inspectingCoreImageView setNeedsDisplay:YES];
}

// a layer's plus button inserts another new layer after this one
- (IBAction)plusButtonAction:(id)sender
{
    NSDictionary *d;
    int index;
    
    d = [self collectFilterImageOrText];
    if (d == nil)
        return;
    index = [sender tag] + 1;
    if ([[d valueForKey:@"type"] isEqualToString:@"filter"])
        [self insertFilter:[d valueForKey:@"filter"] atIndex:[NSNumber numberWithInt:index]];
    else if ([[d valueForKey:@"type"] isEqualToString:@"image"])
        [self insertImage:[d valueForKey:@"image"] withFilename:[d valueForKey:@"filename"]
          andImageFilePath:[d valueForKey:@"imageFilePath"] atIndex:[NSNumber numberWithInt:index]];
    else if ([[d valueForKey:@"type"] isEqualToString:@"text"])
        [self insertString:[d valueForKey:@"string"] withImage:[d valueForKey:@"image"] atIndex:[NSNumber numberWithInt:index]];
    [self enablePlayButton];
}

// for a new filter, set up the odd image parameter
- (void)setAutomaticDefaults:(CIFilter *)f atIndex:(int)index
{
    if ([NSStringFromClass([f class]) isEqualToString:@"CIGlassDistortion"])
    {
        // glass distortion gets a default texture file
        [f setValue:[NSApp defaultTexture] forKey:@"inputTexture"];
        [_inspectingEffectStack setFilterLayer:index imageFilePathValue:[NSApp defaultTexturePath] forKey:@"inputTexture"];
    }
    else if ([NSStringFromClass([f class]) isEqualToString:@"CIRippleTransition"])
    {
        // ripple gets a material map for shading the ripple that has a transparent alpha except specifically for the shines and darkenings
        [f setValue:[NSApp defaultAlphaEMap] forKey:@"inputShadingImage"];
        [_inspectingEffectStack setFilterLayer:index imageFilePathValue:[NSApp defaultAlphaEMapPath] forKey:@"inputShadingImage"];
    }
    else if ([NSStringFromClass([f class]) isEqualToString:@"CIPageCurlTransition"])
    {
        // we set up a good page curl default material map (like that for the ripple transition)
        [f setValue:[NSApp defaultAlphaEMap] forKey:@"inputShadingImage"];
        [_inspectingEffectStack setFilterLayer:index imageFilePathValue:[NSApp defaultAlphaEMapPath] forKey:@"inputShadingImage"];
        // the angle chosen shows off the alpha material map's shine on the leading curl
        [f setValue:[NSNumber numberWithFloat:-M_PI*0.25] forKey:@"inputAngle"];
    }
    else if ([NSStringFromClass([f class]) isEqualToString:@"CIShadedMaterial"])
    {
        // shaded material gets an opaque material map that shows off surfaces well
        [f setValue:[NSApp defaultShadingEMap] forKey:@"inputShadingImage"];
        [_inspectingEffectStack setFilterLayer:index imageFilePathValue:[NSApp defaultShadingEMapPath] forKey:@"inputShadingImage"];
    }
    else if ([NSStringFromClass([f class]) isEqualToString:@"CIColorMap"])
    {
        // color map gets a gradient image that's a color spectrum
        [f setValue:[NSApp defaultRamp] forKey:@"inputGradientImage"];
        [_inspectingEffectStack setFilterLayer:index imageFilePathValue:[NSApp defaultRampPath] forKey:@"inputGradientImage"];
    }
    else if ([NSStringFromClass([f class]) isEqualToString:@"CIDisintegrateWithMaskTransition"])
    {
        // disintegrate with mask transition gets a mask that has a growing star
        [f setValue:[NSApp defaultMask] forKey:@"inputMaskImage"];
        [_inspectingEffectStack setFilterLayer:index imageFilePathValue:[NSApp defaultMaskPath] forKey:@"inputMaskImage"];
    }
    else if ([NSStringFromClass([f class]) isEqualToString:@"CICircularWrap"])
    {
        // circular wrap needs to be aware of the size of the screen to put its data in the right place
        NSRect bounds = [_inspectingCoreImageView bounds];
        float cx = bounds.origin.x + 0.5 * bounds.size.width;
        float cy = bounds.origin.y + 0.5 * bounds.size.height;
        [f setValue:[CIVector vectorWithX:cx Y:cy] forKey:@"inputCenter"];
    }
}

// a layer's mins button removes the layer
- (IBAction)minusButtonAction:(id)sender
{
    [self removeFilterImageOrTextAtIndex:[NSNumber numberWithInt:[sender tag]]];
    [self enablePlayButton];
}

// the reset button removes all layers from the effect stack
- (IBAction)resetButtonAction:(id)sender
{
    int i, count;
    
    // kill off all layers from the effect stack
    count = [_inspectingEffectStack layerCount];
    if (count == 0)
        return;
    // note: done using glue primitives so it will be an undoable operation
    if (![[_inspectingEffectStack typeAtIndex:0] isEqualToString:@"image"])
    {
        for (i = count - 1; i >= 0; i--)
            [self removeFilterImageOrTextAtIndex:[NSNumber numberWithInt:i]];
    }
    else
    {
        for (i = count - 1; i > 0; i--) // note: spare the image at the start
            [self removeFilterImageOrTextAtIndex:[NSNumber numberWithInt:i]];
    }
    // dirty the document
    [self setChanges];
    // update the configuration of the effect stack inspector
    [self updateLayout];
    // let core image recompute the view
    [_inspectingCoreImageView setNeedsDisplay:YES];
    [self enablePlayButton];
}

// stop the transition timer
- (void)stopTimer
{
    if (timer)
    {
        [timer invalidate];
        [timer release];
        timer = nil;
    }
}

// start the transition timer
- (void)startTimer
{
    if (!timer)
    {
        timer = [NSTimer scheduledTimerWithTimeInterval:1.0 / 30.0 target:self selector:@selector(autoTimer:) userInfo:nil repeats:YES];
        [timer retain];
    }
}

// called by the transition timer every 1/30 second
// this animates the transitions in sequence - one after another
- (void)autoTimer:(id)sender
{
    int count, i, transitionIndex;
    CIFilter *f;
    NSString *type;
    NSDictionary *attr;
    double now;
    float transitionValue, value, lastTimeValue;

    now = [NSDate timeIntervalSinceReferenceDate];
    // compute where the global time index within the state of the "n" transitions that are animating
    transitionValue = (now - transitionStartTime) / transitionDuration;
    if (transitionValue < 0.0)
    {
        [self stopTimer];
        return;
    }
    // set all times now
    if (transitionValue >= 0.0)
    {
        // assign an index to each transition
        transitionIndex = 0;
        count = [_inspectingEffectStack layerCount];
        for (i = 0; i < count; i++)
        {
            type = [_inspectingEffectStack typeAtIndex:i];
            if (![type isEqualToString:@"filter"])
                continue;
            // first find time slider
            f = [_inspectingEffectStack filterAtIndex:i];
            attr = [f attributes];
            if ([attr objectForKey:@"inputTime"])
            {
                // for this transition decide where it is within its time sequence
                // by subtracting the transition index from the global time index
                value = transitionValue - (float)transitionIndex;
                // clamp to the time sequence of the transition
                if (value <= 0.0)
                    value = 0.0;
                else if (value > 1.0)
                    value = 1.0;
                lastTimeValue = value;
                // set the inputTime value
                [f setValue:[NSNumber numberWithFloat:value] forKey:@"inputTime"];
                // increment the transition index
                transitionIndex++;
            }
        }
    }
    // let core image recompute the view
    [_inspectingCoreImageView setNeedsDisplay:YES]; // force a redisplay
    // terminate the animation if we've animated all transitions to their completion point
    if (now >= transitionEndTime && lastTimeValue == 1.0)
    {
        // when all transitions are done, update the sliders in the effect stack inspector
        [self updateLayout];
        // and turn off the timer
        [self stopTimer];
        return;
    }
}

// handle the play button - play all transitions
- (void)playButtonAction:sender
{
    int i, count, nTransitions;
    NSString *type;
    CIFilter *f;
    NSDictionary *attr, *d;
    
    count = [_inspectingEffectStack layerCount];
    // first determine the number of transitions
    nTransitions = 0;
    for (i = 0; i < count; i++)
    {
        type = [_inspectingEffectStack typeAtIndex:i];
        // find only filter layers
        if (![type isEqualToString:@"filter"])
            continue;
        // first find time slider
        f = [_inspectingEffectStack filterAtIndex:i];
        attr = [f attributes];
        // basically anything with an "inputTime" is a transition by definition
        d = [attr objectForKey:@"inputTime"];
        if (d == nil)
            continue;
        // we have a transition 
        nTransitions++;
    }
    if (nTransitions == 0)
        return;
    // set up the information governing the global time index over all transitions
    transitionStartTime = [NSDate timeIntervalSinceReferenceDate];
    transitionDuration = 1.5;
    transitionEndTime = transitionStartTime + transitionDuration * (float)nTransitions;
    // start the timer now
    [self startTimer];
    // set all inputTime parameters to 0.0
    for (i = 0; i < count; i++)
    {
        type = [_inspectingEffectStack typeAtIndex:i];
        // find only filters
        if (![type isEqualToString:@"filter"])
            continue;
        // first find time slider
        f = [_inspectingEffectStack filterAtIndex:i];
        attr = [f attributes];
        d = [attr objectForKey:@"inputTime"];
        if (d == nil)
            continue;
        // set the value to zero
        [f setValue:[NSNumber numberWithFloat: 0.0] forKey:@"inputTime"];
    }
    // let core image recompute the view
    [_inspectingCoreImageView setNeedsDisplay:YES]; // force a redisplay
}

// this must be in synch with EffectStack.nib
#define inspectorTopY 36

// lay out the effect stack inspector - this takes the NSBox'es in the boxes array and places them
// as subviews to the our owned window's content view
- (void)layoutInspector
    {
    // decide how inspector is to be sized and layed out
    // boxes are all internally sized properly at this point
    int i, count, inspectorheight, fvtop;
    FilterView *fv;
    
    // first estimate the size of the effect stack inspector (with the boxes placed one after another vertically)
    count = [boxes count];
    inspectorheight = inspectorTopY;
    for (i = 0; i < count; i++)
        {
        fv = [boxes objectAtIndex:i];
        float height = [fv bounds].size.height;
        // add the height of the box plus some spacing
        inspectorheight += height + 6;
        }
    // resize the effect stack inspector now
    NSRect frm = [[self window] frame];
    float delta = inspectorheight + [[self window] frame].size.height
      - [[[self window] contentView] frame].size.height - frm.size.height;
    frm.size.height += delta;
    frm.origin.y -= delta;
    [[self window] setFrame:frm display:YES animate:YES]; // animate the window size change
    // and move all the boxes into place
    fvtop = inspectorheight - inspectorTopY;
    for (i = 0; i < count; i++)
        {
        fv = [boxes objectAtIndex:i];
        frm = [fv frame];
        frm.origin.y = fvtop - frm.size.height;
        [fv setFrame:frm];
        fvtop -= frm.size.height + 6;
        // unhide the box
        [fv setHidden:NO];
        }
    // finally call for a redisplay of the effect stack inspector
    [[[self window] contentView] setNeedsDisplay:YES];
    }

// close down the effect stack inspector: this must be done before quit so our owned window's popsition
// can be properly saved and subsequently restored on the next launch
- (void)closeDown
    {
    // resize inspector now
    NSRect frm = [[self window] frame];
    float delta = inspectorTopY + [[self window] frame].size.height
      - [[[self window] contentView] frame].size.height - frm.size.height;
    frm.size.height += delta;
    frm.origin.y -= delta;
    [[self window] setFrame:frm display:YES animate:NO]; // skip animation on quit!
    }

// automatically generate the UI for an effect stack filter layer
// returning an NSBox (actually FilterView is a subclass of NSBox)
- (FilterView *)createUIForFilter:(CIFilter *)f index:(int)index
{
    BOOL hasBackground;
    NSDictionary *attr;
    NSArray *inputKeys;
    NSString *key, *typestring, *classstring;
    NSEnumerator *enumerator;
    NSRect frame;
    FilterView *fv;
    NSView *view;
    
    // create box first
    view = [[self window] contentView];
    frame = [view bounds];
    frame.size.width -= 12;
    frame.origin.x += 6;
    frame.size.height -= inspectorTopY;
    fv = [[FilterView alloc] initWithFrame:frame];
    [fv setFilter:f];
    [fv setHidden:YES];
    [[[self window] contentView] addSubview:fv];
    [fv setTitlePosition:NSNoTitle];
    [fv setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    [fv setBorderType:NSGrooveBorder];
    [fv setBoxType:NSBoxPrimary];
    [fv setMaster:self];
    [fv setTag:index];
    // first compute size of box with all the controls
    [fv tryFilterHeader:f];
    attr = [f attributes];
    inputKeys = [f inputKeys];
    // decide if this filter has a background image parameter (true for blend modes and Porter-Duff modes)
    hasBackground = NO;
    enumerator = [inputKeys objectEnumerator];
    while ((key = [enumerator nextObject]) != nil) 
    {
        id parameter = [attr objectForKey:key];
        if ([parameter isKindOfClass:[NSDictionary class]])
        {
            classstring = [(NSDictionary *)parameter objectForKey: kCIAttributeClass];
            if ([classstring isEqualToString:@"CIImage"] && [key isEqualToString:@"inputBackgroundImage"])
                hasBackground = YES;
        }
    }
    // enumerate all input parameters and reserve space for their generated UI
    enumerator = [inputKeys objectEnumerator];
    while ((key = [enumerator nextObject]) != nil) 
    {
        id parameter = [attr objectForKey:key];
        if ([parameter isKindOfClass:[NSDictionary class]])
        {
            classstring = [(NSDictionary *)parameter objectForKey:kCIAttributeClass];
            if ([classstring isEqualToString:@"NSNumber"])
            {
                typestring = [(NSDictionary *)parameter objectForKey:kCIAttributeType];
                if ([typestring isEqualToString:kCIAttributeTypeBoolean])
                    // if it's a boolean type, save space for a check box
                    [fv tryCheckBoxForFilter:f key:key displayView:_inspectingCoreImageView];
                else
                    // otherwise space space for a slider
                    [fv trySliderForFilter:f key:key displayView:_inspectingCoreImageView];
            }
            else if ([classstring isEqualToString:@"CIColor"])
                // save space for a color well
                [fv tryColorWellForFilter:f key:key displayView:_inspectingCoreImageView];
            else if ([classstring isEqualToString:@"CIImage"])
            {
                // don't bother to create a UI element for the chained image
                if (hasBackground)
                {
                    // the chained image is the background image for blend modes and Porter-Duff modes
                    // it is provided by what's above this layer in the effect stack
                    if (![key isEqualToString:@"inputBackgroundImage"])
                        // save space for an image well
                        [fv tryImageWellForFilter:f key:key displayView:_inspectingCoreImageView];
                }
                else
                {
                    // the chained image is the input image for all other filters
                    // it is provided by what's above this layer in the effect stack
                    if (![key isEqualToString:@"inputImage"])
                        // save space for an image well
                        [fv tryImageWellForFilter:f key:key displayView:_inspectingCoreImageView];
                }
            }
            else if ([classstring isEqualToString:@"NSAffineTransform"])
                // save space for transform inspection widgets
                [fv tryTransformForFilter:f key:key displayView:_inspectingCoreImageView];
            else if ([classstring isEqualToString:@"CIVector"])
            {
                // check for a vector with no attributes
                typestring = [(NSDictionary *)parameter objectForKey:kCIAttributeType];
                if (typestring == nil)
                    // save space for a 4-element vector inspection widget (4 text fields)
                    [fv tryVectorForFilter:f key:key displayView:_inspectingCoreImageView];
                else if ([typestring isEqualToString:kCIAttributeTypeOffset])
                    [fv tryOffsetForFilter:f key:key displayView:_inspectingCoreImageView];
                // note: the other CIVector parameters are handled in mouse down processing of the core image view
            } 
        }
    }
    // now resize the box to hold the controls we're about to make
    [fv trimBox];
    // now add all the controls
    [fv addFilterHeader:f tag:index enabled:[_inspectingEffectStack layerEnabled:index]];
    attr = [f attributes];
    inputKeys = [f inputKeys];
    // enumerate all input parameters and generate their UI
    enumerator = [inputKeys objectEnumerator];
    while ((key = [enumerator nextObject]) != nil) 
    {
        id parameter = [attr objectForKey:key];
        if ([parameter isKindOfClass:[NSDictionary class]])
        {
            classstring = [(NSDictionary *)parameter objectForKey: kCIAttributeClass];
            if ([classstring isEqualToString:@"NSNumber"])
            {
                typestring = [(NSDictionary *)parameter objectForKey: kCIAttributeType];
                if ([typestring isEqualToString:kCIAttributeTypeBoolean])
                    // if it's a boolean type, generate a check box
                    [fv addCheckBoxForFilter:f key:key displayView:_inspectingCoreImageView];
                else
                    // otherwise generate a slider
                    [fv addSliderForFilter:f key:key displayView:_inspectingCoreImageView];
            }
            else if ([classstring isEqualToString:@"CIImage"])
            {
                if (hasBackground)
                {
                    // the chained image is the background image for blend modes and Porter-Duff modes
                    // it is provided by what's above this layer in the effect stack
                    if (![key isEqualToString:@"inputBackgroundImage"])
                        // generate an image well
                        [fv addImageWellForFilter:f key:key displayView:_inspectingCoreImageView];
                }
                else
                {
                    // the chained image is the input image for all other filters
                    // it is provided by what's above this layer in the effect stack
                    if (![key isEqualToString:@"inputImage"])
                        // generate an image well
                        [fv addImageWellForFilter:f key:key displayView:_inspectingCoreImageView];
                }
            }
            else if ([classstring isEqualToString:@"CIColor"])
                // generate a color well
                [fv addColorWellForFilter:f key:key displayView:_inspectingCoreImageView];
            else if ([classstring isEqualToString:@"NSAffineTransform"])
                // generate transform inspection widgets
                [fv addTransformForFilter:f key:key displayView:_inspectingCoreImageView];
            else if ([classstring isEqualToString:@"CIVector"])
            {
                // check for a vector with no attributes
                typestring = [(NSDictionary *)parameter objectForKey:kCIAttributeType];
                if (typestring == nil)
                    // generate a 4-element vector inspection widget (4 text fields)
                    [fv addVectorForFilter:f key:key displayView:_inspectingCoreImageView];
                else if ([typestring isEqualToString:kCIAttributeTypeOffset])
                    [fv addOffsetForFilter:f key:key displayView:_inspectingCoreImageView];
                // the rest are handled in mouse down processing
            } 
        }
    }
    // retrun the box with the filter's UI
    return fv;
}

// automatically generate the UI for an effect stack image layer
// returning an NSBox (actually FilterView is a subclass of NSBox)
- (FilterView *)createUIForImage:(CIImage *)im filename:(NSString *)filename index:(int)index
{
    NSRect frame;
    FilterView *fv;
    NSView *view;
    
    // create the box first
    view = [[self window] contentView];
    frame = [view bounds];
    frame.size.width -= 12;
    frame.origin.x += 6;
    frame.size.height -= inspectorTopY;
    fv = [[FilterView alloc] initWithFrame:frame];
    [fv setFilter:nil];
    [fv setHidden:YES];
    [[[self window] contentView] addSubview:fv];
    [fv setTitlePosition:NSNoTitle];
    [fv setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    [fv setBorderType:NSGrooveBorder];
    [fv setBoxType:NSBoxPrimary];
    [fv setMaster:self];
    [fv setTag:index];
    // first compute size of box with all the controls
    [fv tryImageHeader:im];
    [fv tryImageWellForImage:im tag:index displayView:_inspectingCoreImageView];
    // now resize the box to hold the controls we're about to make
    [fv trimBox];
    // now add all the controls
    [fv addImageHeader:im filename:filename tag:index enabled:[_inspectingEffectStack layerEnabled:index]];
    [fv addImageWellForImage:im tag:index displayView:_inspectingCoreImageView];
    return fv;
}

// automatically generate the UI for an effect stack text layer
// returning an NSBox (actually FilterView is a subclass of NSBox)
- (FilterView *)createUIForText:(NSString *)string index:(int)index
{
    NSRect frame;
    FilterView *fv;
    NSView *view;
    
    // create the box first
    view = [[self window] contentView];
    frame = [view bounds];
    frame.size.width -= 12;
    frame.origin.x += 6;
    frame.size.height -= inspectorTopY;
    fv = [[FilterView alloc] initWithFrame:frame];
    [fv setFilter:nil];
    [fv setHidden:YES];
    [[[self window] contentView] addSubview:fv];
    [fv setTitlePosition:NSNoTitle];
    [fv setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    [fv setBorderType:NSGrooveBorder];
    [fv setBoxType:NSBoxPrimary];
    [fv setMaster:self];
    [fv setTag:index];
    // first compute size of box with all the controls
    [fv tryTextHeader:string];
    [fv tryTextViewForString];
    [fv trySliderForText];
    // now resize the box to hold the controls we're about to make
    [fv trimBox];
    // now add all the controls
    [fv addTextHeader:string tag:index enabled:[_inspectingEffectStack layerEnabled:index]];
    [fv addTextViewForString:[_inspectingEffectStack mutableDictionaryAtIndex:index] key:@"string" displayView:_inspectingCoreImageView];
    [fv addSliderForText:[_inspectingEffectStack mutableDictionaryAtIndex:index] key:@"scale" lo:1.0 hi:100.0 displayView:_inspectingCoreImageView];
    return fv;
}

// handle the filter palette apply button
- (IBAction)filterOKButtonAction:(id)sender
{
    // signal to apply filter
    [NSApp stopModalWithCode:100];
}

// handle the filter palette cancel button
- (IBAction)filterCancelButtonAction:(id)sender
{
    // signal cancel
    [NSApp stopModalWithCode:101];
}

// handle the filter palette image button
- (IBAction)filterImageButtonAction:(id)sender
{
    // signal to get an image
    [NSApp stopModalWithCode:102];
}

- (IBAction)filterTextButtonAction:(id)sender
{
    // signal to setup a text layer
    [NSApp stopModalWithCode:103];
}

// return the category name for the category index - used by filter palette category table view
- (NSString *)categoryNameForIndex:(int)i
    {
    NSString *s;

    switch (i)
        {
    case 0:
        s = [CIFilter localizedNameForCategory:kCICategoryGeometryAdjustment];
        break;
    case 1:
        s = [CIFilter localizedNameForCategory:kCICategoryDistortionEffect];
        break;
    case 2:
        s = [CIFilter localizedNameForCategory:kCICategoryBlur];
        break;
    case 3:
        s = [CIFilter localizedNameForCategory:kCICategorySharpen];
        break;
    case 4:
        s = [CIFilter localizedNameForCategory:kCICategoryColorAdjustment];
        break;
    case 5:
        s = [CIFilter localizedNameForCategory:kCICategoryColorEffect];
        break;
    case 6:
        s = [CIFilter localizedNameForCategory:kCICategoryStylize];
        break;
    case 7:
        s = [CIFilter localizedNameForCategory:kCICategoryHalftoneEffect];
        break;
    case 8:
        s = [CIFilter localizedNameForCategory:kCICategoryTileEffect];
        break;
    case 9:
        s = [CIFilter localizedNameForCategory:kCICategoryGenerator];
        break;
    case 10:
        s = [CIFilter localizedNameForCategory:kCICategoryGradient];
        break;
    case 11:
        s = [CIFilter localizedNameForCategory:kCICategoryTransition];
        break;
    case 12:
        s = [CIFilter localizedNameForCategory:kCICategoryCompositeOperation];
        break;
    default:
        s = @"";
        break;
        }
    return s;
    }

// return the category index for the category name - used by filter palette category table view
- (int)indexForCategory:(NSString *)nm
    {
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryGeometryAdjustment]])
        return 0;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryDistortionEffect]])
        return 1;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryBlur]])
        return 2;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategorySharpen]])
        return 3;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryColorAdjustment]])
        return 4;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryColorEffect]])
        return 5;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryStylize]])
        return 6;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryHalftoneEffect]])
        return 7;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryTileEffect]])
        return 8;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryGenerator]])
        return 9;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryGradient]])
        return 10;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryTransition]])
        return 11;
    if ([nm isEqualToString:[CIFilter localizedNameForCategory:kCICategoryCompositeOperation]])
        return 12;
    return -1;
    }

// build a dictionary of approved filters in a given category for the filter inspector
- (NSMutableDictionary *)buildFilterDictionary:(NSArray *)names
{
    BOOL inspectable;
    NSDictionary *attr, *parameter;
    NSArray *inputKeys;
    NSEnumerator *enumerator;
    NSMutableDictionary *td, *catfilters;
    NSString *classname, *classstring, *key, *typestring;
    CIFilter *filter;
    int i;

    catfilters = [NSMutableDictionary dictionary];
    for (i = 0; i < [names count]; i++)
    {
        // load the filter class name
        classname = [names objectAtIndex:i];
        // create an instance of the filter
        filter = [CIFilter filterWithName:classname];
        if (filter != nil)
        {
            // search the filter for any input parameters we can't inspect
            inspectable = YES;
            attr = [filter attributes];
            inputKeys = [filter inputKeys];
            // enumerate all input parameters and generate their UI
            enumerator = [inputKeys objectEnumerator];
            while ((key = [enumerator nextObject]) != nil) 
            {
                parameter = [attr objectForKey:key];
                classstring = [parameter objectForKey:kCIAttributeClass];
                if ([classstring isEqualToString:@"CIImage"]
                  || [classstring isEqualToString:@"CIColor"]
                  || [classstring isEqualToString:@"NSAffineTransform"]
                  || [classstring isEqualToString:@"NSNumber"])
                    continue; // all inspectable
                else if ([classstring isEqualToString:@"CIVector"])
                {
                    // check for a vector with no attributes
                    typestring = [parameter objectForKey:kCIAttributeType];
                    if (typestring != nil
                      && ![typestring isEqualToString:kCIAttributeTypePosition]
                      && ![typestring isEqualToString:kCIAttributeTypeRectangle]
                      && ![typestring isEqualToString:kCIAttributeTypePosition3]
                      && ![typestring isEqualToString:kCIAttributeTypeOffset])
                        inspectable = NO;
                }
                else
                    inspectable = NO;
            }
            if (!inspectable)
                continue; // if we can't inspect it, it's not approved and must be omitted from the list
            // create a dictionary for the filter with filter's class name
            td = [NSMutableDictionary dictionary];
            [td setObject:classname forKey:kCIAttributeClass];
            // set it as the value for a key which is the filter's localized name
            [catfilters setObject:td forKey:[CIFilter localizedNameForFilterName:classname]];
        }
        else
            NSLog(@" could not create '%@' filter", classname);
    }
    return catfilters;
}

// build the filter list (enumerates all filters)
- (void)_loadFilterListIntoInspector
{
    NSString *cat;
    NSArray *attrs;
    NSMutableArray *all;
    int i, m;

    // here's a list of all categories
    attrs = [NSArray arrayWithObjects:
      kCICategoryGeometryAdjustment,
      kCICategoryDistortionEffect,
      kCICategoryBlur,
      kCICategorySharpen,
      kCICategoryColorAdjustment,
      kCICategoryColorEffect,
      kCICategoryStylize,
      kCICategoryHalftoneEffect,
      kCICategoryTileEffect,
      kCICategoryGenerator,
      kCICategoryGradient,
      kCICategoryTransition,
      kCICategoryCompositeOperation,
      nil];
    // call to load all plug-in image units
    [CIPlugIn loadAllPlugIns];
    // enumerate all filters in the chosen categories
    m = [attrs count];
    for (i = 0; i < m; i++)
    {
        // get this category
        cat = [attrs objectAtIndex:i];
        // make a list of all filters in this category
        all = [NSMutableArray arrayWithArray:[CIFilter filterNamesInCategory:cat]];
        // make this category's list of approved filters
        [categories setObject:[self buildFilterDictionary:all] forKey:[CIFilter localizedNameForCategory:cat]];
    }
    currentCategory = 0;
    currentFilterRow = 0;
    // load up the filter list into the table view
    [filterTableView reloadData];
}

// table view data source methods
- (int)numberOfRowsInTableView:(NSTableView *)tv
{
    int count;
    NSString *s;
    NSDictionary *dict;
    NSArray *filterNames;
    
    switch ([tv tag])
    {
    case 0:
        // category table view
        count = 13;
        break;
    case 1:
        // filter table view
        s = [self categoryNameForIndex:currentCategory];
        // use category name to get dictionary of filter names
        dict = [categories objectForKey:s];
        // create an array
        filterNames = [dict allKeys];
        // return number of filters in this category
        count = [filterNames count];
        break;
    }
    return count;
}

static int stringCompare(id o1, id o2, void *context)
{
    NSString *str1, *str2;
    
    str1 = o1;
    str2 = o2;
    return [str1 compare:str2];
}

- (id)tableView:(NSTableView *)tv objectValueForTableColumn:(NSTableColumn *)tc row:(int)row
{
    NSString *s;
    NSDictionary *dict;
    NSArray *filterNames;
    NSTextFieldCell *tfc;
    
    switch ([tv tag])
    {
    case 0:
        // category table view
        s = [self categoryNameForIndex:row];
        tfc = [tc dataCell];
        // handle names that are too long by ellipsizing the name
        s = [ParameterView ellipsizeField:[tc width] font:[tfc font] string:s];
        break;
    case 1:
        // filter table view
        // we need to maintain the filter names in a sorted order.
        s = [self categoryNameForIndex:currentCategory];
        // use label (category name) to get dictionary of filter names
        dict = [categories objectForKey:s];
        // create an array of the sorted names (this is inefficient since we don't cache the sorted array)
        filterNames = [[dict allKeys] sortedArrayUsingFunction:stringCompare context:nil];
        // return filter name
        s = [filterNames objectAtIndex:row];
        tfc = [tc dataCell];
        // handle names that are too long by ellipsizing the name
        s = [ParameterView ellipsizeField:[tc width] font:[tfc font] string:s];
        break;
    }
    return s;
}

// this is called when we select a filter from the list
- (void)addEffect
{
    int row;
    NSTableView *tv;
    NSDictionary *dict, *td;
    NSArray *filterNames;
    
    // get current category item
    tv = filterTableView;
    // decide current filter name from selected row (or none selected) in the filter name list
    row = [tv selectedRow];
    if (row == -1)
    {
        [filterClassname release];
        filterClassname = nil;
        [filterOKButton setEnabled:NO];
        return;
    }
    // use label (category name) to get dictionary of filter names
    dict = [categories objectForKey:[self categoryNameForIndex:currentCategory]];
    // create an array of all filter names for this category
    filterNames = [[dict allKeys] sortedArrayUsingFunction:stringCompare context:nil];
    // return filter name
    td = [dict objectForKey:[filterNames objectAtIndex:row]];
    // retain the name in filterClassname for use outside the modal
    [filterClassname release];
    filterClassname = [[td objectForKey:kCIAttributeClass] retain];
    // enable the apply button
    [filterOKButton setEnabled:YES];
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
    int row;
    NSTableView *tv;

    tv = [aNotification object];
    row = [tv selectedRow];
    switch ([tv tag])
    {
    case 0:
        // category table view
        // select the category
        currentCategory = row;
        // reload the filter table based on the current category
        [filterTableView reloadData];
        [filterTableView deselectAll:self];
        [filterTableView noteNumberOfRowsChanged];
        break;
    case 1:
        // filter table view
        // select a filter
        // add an effect to current effects list
        currentFilterRow = row;
        [self addEffect];
        break;
    }
}

// if we see a double-click in the filter list, it's like hitting apply
- (IBAction)tableViewDoubleClick:(id)sender
{
    [NSApp stopModalWithCode:100];
}

// glue code for determining if a filter layer has a missing image (and should be drawn red to indicate as such)
- (BOOL)effectStackFilterHasMissingImage:(CIFilter *)f
{
    return [_inspectingEffectStack filterHasMissingImage:f];
}

// glue code to set up an image layer
- (void)setLayer:(int)index image:(CIImage *)im andFilename:(NSString *)filename
{
    [_inspectingEffectStack setImageLayer:index image:im andFilename:filename];
}

- (void)registerImageLayer:(int)index imageFilePath:(NSString *)path
{
    [_inspectingEffectStack setImageLayer:index imageFilePath:path];
}

- (void)registerFilterLayer:(CIFilter *)filter key:(NSString *)key imageFilePath:(NSString *)path
{
    int i, count;
    NSString *type;
    
    count = [_inspectingEffectStack layerCount];
    for (i = 0; i < count; i++)
    {
        type = [_inspectingEffectStack typeAtIndex:i];
        if (![type isEqualToString:@"filter"])
            continue;
        if (filter == [_inspectingEffectStack filterAtIndex:i])
        {
            [_inspectingEffectStack setFilterLayer:i imageFilePathValue:path forKey:key];
            break;
        }
    }
}

- (NSString *)imageFilePathForImageLayer:(int)index
{
    return [_inspectingEffectStack imageFilePathAtIndex:index];
}

- (NSString *)imageFilePathForFilterLayer:(CIFilter *)filter key:(NSString *)key
{
    int i, count;
    NSString *type;
    
    count = [_inspectingEffectStack layerCount];
    for (i = 0; i < count; i++)
    {
        type = [_inspectingEffectStack typeAtIndex:i];
        if (![type isEqualToString:@"filter"])
            continue;
        if (filter == [_inspectingEffectStack filterAtIndex:i])
            return [_inspectingEffectStack filterLayer:i imageFilePathValueForKey:key];
    }
    return nil;
}

- (void)reconfigureWindow
{
    NSString *path;
    CIImage *image;
    CGRect extent;
    
    path = [_inspectingEffectStack imageFilePathAtIndex:0];
    image = [_inspectingEffectStack imageAtIndex:0];
    extent = [image extent];
    [[self doc] reconfigureWindowToSize:NSMakeSize(extent.size.width, extent.size.height) andPath:path];
}

@end

@implementation EffectStackBox

// this is a subclass of NSBox required so we can draw the interior of the box as red when there's something
// in the box (namely an image well) that still needs filling

#define boxInset 3.0
#define boxFillet 7.0
// control point distance from rectangle corner
#define cpdelta (boxFillet * 0.35)

- (void)setFilter:(CIFilter *)f
{
    filter = f;
}

- (void)setMaster:(EffectStackController *)m
{
    master = m;
}

- (void)drawRect:(NSRect)r
{
    NSBezierPath *path;
    NSPoint bl, br, tr, tl;
    NSRect R;
    
    [super drawRect:r];
    if ([master effectStackFilterHasMissingImage:filter])
    {
        // overlay the box now - colorized
        [[NSColor colorWithDeviceRed:1.0 green:0.0 blue:0.0 alpha:0.15] set];
        path = [NSBezierPath bezierPath];
        R = NSOffsetRect(NSInsetRect([self bounds], boxInset, boxInset), 0, 1);
        bl = R.origin;
        br = NSMakePoint(R.origin.x + R.size.width, R.origin.y);
        tr = NSMakePoint(R.origin.x + R.size.width, R.origin.y + R.size.height);
        tl = NSMakePoint(R.origin.x, R.origin.y + R.size.height);
        [path moveToPoint:NSMakePoint(bl.x + boxFillet, bl.y)];
        [path lineToPoint:NSMakePoint(br.x - boxFillet, br.y)];
        [path curveToPoint:NSMakePoint(br.x, br.y + boxFillet)
          controlPoint1:NSMakePoint(br.x - cpdelta, br.y)
          controlPoint2:NSMakePoint(br.x, br.y + cpdelta)];
        [path lineToPoint:NSMakePoint(tr.x, tr.y - boxFillet)];
        [path curveToPoint:NSMakePoint(tr.x - boxFillet, tr.y)
          controlPoint1:NSMakePoint(tr.x, tr.y - cpdelta)
          controlPoint2:NSMakePoint(tr.x - cpdelta, tr.y)];
        [path lineToPoint:NSMakePoint(tl.x + boxFillet, tl.y)];
        [path curveToPoint:NSMakePoint(tl.x, tl.y - boxFillet)
          controlPoint1:NSMakePoint(tl.x + cpdelta, tl.y)
          controlPoint2:NSMakePoint(tl.x, tl.y - cpdelta)];
        [path lineToPoint:NSMakePoint(bl.x, bl.y + boxFillet)];
        [path curveToPoint:NSMakePoint(bl.x + boxFillet, bl.y)
          controlPoint1:NSMakePoint(bl.x, bl.y + cpdelta)
          controlPoint2:NSMakePoint(bl.x + cpdelta, bl.y)];
        [path closePath];
        [path fill];
    }
}

@end
