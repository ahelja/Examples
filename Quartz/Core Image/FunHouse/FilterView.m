// FilterView.m
// Author: Mark Zimmer
// 12/08/04
// Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.
#import <QuartzCore/QuartzCore.h>
#import "ParameterView.h"
#import "FilterView.h"
#import "CoreImageView.h"
#import "EffectStackController.h"

#define kSliderVerticalAdvance (18)
#define kVerticalGap (0)

// the filter view is really the box that contains all the UI widgets to edit a layer
// each of the widgets (slider with label and readout, color well with label, check box, image view with choose button, etc)
// gets encoded in its own parameter view

@implementation FilterView

- (void)setTag:(int)index
{
    tag = index;
}

- (id)initWithFrame:(NSRect)frame 
{
    self = [super initWithFrame:frame];
    if (self != nil) 
    {
        controlLeftPosition = 5;
        controlTopPosition = frame.size.height;
        lastControlType = ctNone;
    }
    return self;
}

// trim the box down to include only the allocated widgets
- (void)trimBox
{
    NSRect R;
    
    R = [self frame];
    controlTopPosition -= 3;
    R.size.height -= controlTopPosition;
    R.origin.y += controlTopPosition;
    [self setFrame:R];
    lastControlType = ctNone;
}

// allocate the space required for a filter layer header
- (void)tryFilterHeader:(CIFilter *)filter
{
    NSRect R;
    
    R = [self bounds];
    R.origin.y += R.size.height - 38;
    R.size.height = 22;
    R.origin.x += 63;
    R.size.width -= 62 + 63;
    controlTopPosition = R.origin.y - 8;
}

// add a filter layer header
- (void)addFilterHeader:(CIFilter *)f tag:(int)index enabled:(BOOL)enabled
{
    NSRect R, S, cbR;
    NSString *name;
    NSCell *c;
    
    R = [self bounds];
    R.origin.y += R.size.height - 38;
    R.size.height = 22;
    R.origin.x += 33;
    R.size.width -= 62 + 63;
    // add enable/disable check box
    cbR = R;
    cbR.origin.x = 0;
    cbR.size.width = 58;
    cbR.size.height = 22;
    // label
    filterNameField = [[NSTextField alloc] initWithFrame:R];
    name = [[f attributes] objectForKey:kCIAttributeFilterDisplayName];
    // set text label to 9 point
    c = [filterNameField cell];
    // determine if we need to ellipsize
    name = [ParameterView ellipsizeField:[c drawingRectForBounds:[filterNameField bounds]].size.width font:[c font] string:name];
    [filterNameField setStringValue:name];
    [filterNameField setEditable:NO];
    [filterNameField setBezeled:NO];
    [filterNameField setDrawsBackground:NO];
    [filterNameField setAutoresizingMask:NSViewMaxXMargin|NSViewMinYMargin];
    [self addSubview:filterNameField];
    checkBox = [[NSButton alloc] initWithFrame:cbR];
    [checkBox setTarget:master];
    [checkBox setAction:@selector(enableCheckBoxAction:)];
    [checkBox setButtonType:NSSwitchButton];
    [checkBox setState:(enabled ? NSOnState : NSOffState)];
    [checkBox setTitle:@""];
    [checkBox setTag:index];
    [self addSubview:checkBox];
    // add + button
    S = R;
    S.origin.x = R.origin.x + R.size.width + 33;
    S.origin.y += 3;
    S.size.height -= 4;
    S.size.width = 22;
    plusbutton = [[NSButton alloc] initWithFrame:S];
    [[plusbutton cell] setButtonType:NSMomentaryPushButton];
    [[plusbutton cell] setBezelStyle:NSShadowlessSquareBezelStyle];
    [[plusbutton cell] setGradientType:NSGradientConcaveWeak];
    [plusbutton setImagePosition:NSImageOnly];
    [plusbutton setImage:[NSImage imageNamed:@"plusbutton"]];
    [plusbutton setBordered:NO];
    [plusbutton setTarget:master];
    [plusbutton setAction:@selector(plusButtonAction:)];
    [plusbutton setTag:index];
    [self addSubview:plusbutton];
    // add - button
    S.origin.x += 21;
    minusbutton = [[NSButton alloc] initWithFrame:S];
    [[minusbutton cell] setButtonType:NSMomentaryPushButton];
    [[minusbutton cell] setBezelStyle:NSShadowlessSquareBezelStyle];
    [[minusbutton cell] setGradientType:NSGradientConcaveWeak];
    [minusbutton setImagePosition:NSImageOnly];
    [minusbutton setImage:[NSImage imageNamed:@"minusbutton"]];
    [minusbutton setBordered:NO];
    [minusbutton setTarget:master];
    [minusbutton setAction:@selector(minusButtonAction:)];
    [minusbutton setTag:index];
    [self addSubview:minusbutton];
    controlTopPosition = R.origin.y - 8;
}

// allocate the space required for an image layer header
- (void)tryImageHeader:(CIImage *)im
{
    NSRect R;
    
    R = [self bounds];
    R.origin.y += R.size.height - 38;
    R.size.height = 22;
    R.origin.x += 63;
    R.size.width -= 62 + 63;
    controlTopPosition = R.origin.y - 8;
}

// add an image layer header
- (void)addImageHeader:(CIImage *)im filename:(NSString *)filename tag:(int)index enabled:(BOOL)enabled
{
    NSRect R, S, cbR;
    NSString *name;
    NSCell *c;
    
    R = [self bounds];
    R.origin.y += R.size.height - 38;
    R.size.height = 22;
    R.origin.x += 33;
    R.size.width -= 62 + 63;
    // add enable/disable check box
    cbR = R;
    cbR.origin.x = 0;
    cbR.size.width = 58;
    cbR.size.height = 22;
    // label
    filterNameField = [[NSTextField alloc] initWithFrame:R];
    name = [filename copy];
    // set text label to 9 point
    c = [filterNameField cell];
    // determine if we need to ellipsize
    name = [ParameterView ellipsizeField:[c drawingRectForBounds:[filterNameField bounds]].size.width font:[c font] string:name];
    [filterNameField setStringValue:name];
    [filterNameField setEditable:NO];
    [filterNameField setBezeled:NO];
    [filterNameField setDrawsBackground:NO];
    [filterNameField setAutoresizingMask:NSViewMaxXMargin|NSViewMinYMargin];
    [self addSubview:filterNameField];
    checkBox = [[NSButton alloc] initWithFrame:cbR];
    [checkBox setTarget:master];
    [checkBox setAction:@selector(enableCheckBoxAction:)];
    [checkBox setButtonType:NSSwitchButton];
    [checkBox setState:(enabled ? NSOnState : NSOffState)];
    [checkBox setTitle:@""];
    [checkBox setTag:index];
    [self addSubview:checkBox];
    // add + button
    S = R;
    S.origin.x = R.origin.x + R.size.width + 33;
    S.origin.y += 3;
    S.size.height -= 4;
    S.size.width = 22;
    plusbutton = [[NSButton alloc] initWithFrame:S];
    [[plusbutton cell] setButtonType:NSMomentaryPushButton];
    [[plusbutton cell] setBezelStyle:NSShadowlessSquareBezelStyle];
    [[plusbutton cell] setGradientType:NSGradientConcaveWeak];
    [plusbutton setImagePosition:NSImageOnly];
    [plusbutton setImage:[NSImage imageNamed:@"plusbutton"]];
    [plusbutton setBordered:NO];
    [plusbutton setTarget:master];
    [plusbutton setAction:@selector(plusButtonAction:)];
    [plusbutton setTag:index];
    [self addSubview:plusbutton];
    // add - button
    S.origin.x += 21;
    minusbutton = [[NSButton alloc] initWithFrame:S];
    [[minusbutton cell] setButtonType:NSMomentaryPushButton];
    [[minusbutton cell] setBezelStyle:NSShadowlessSquareBezelStyle];
    [[minusbutton cell] setGradientType:NSGradientConcaveWeak];
    [minusbutton setImagePosition:NSImageOnly];
    [minusbutton setImage:[NSImage imageNamed:@"minusbutton"]];
    [minusbutton setBordered:NO];
    [minusbutton setTarget:master];
    [minusbutton setAction:@selector(minusButtonAction:)];
    [minusbutton setTag:index];
    [self addSubview:minusbutton];
    controlTopPosition = R.origin.y - 8;
}

// allocate the space required for a filter layer slider
- (void)trySliderForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
{
    controlTopPosition -= kSliderVerticalAdvance + kVerticalGap;
    lastControlType = ctSlider;
}

// add a filter layer slider
- (void)addSliderForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
{
    NSRect pRect, frame;
    ParameterView *pView;
    
    frame = [self bounds];
    pRect = NSMakeRect(0, controlTopPosition - kSliderVerticalAdvance, frame.size.width - 12, kSliderVerticalAdvance);
    pView = [[ParameterView alloc] initWithFrame:pRect];
    [pView addSliderForFilter:f key:k displayView:v master:master];
    [self addSubview:pView];
    [pView setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    controlTopPosition -= kSliderVerticalAdvance + kVerticalGap;
    lastControlType = ctSlider;
}

// allocate the space required for a filter layer check box
- (void)tryCheckBoxForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
{
    controlTopPosition -= 17;
    lastControlType = ctCheckBox;
}

// add a filter layer check box
- (void)addCheckBoxForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
{
    NSRect pRect, frame;
    ParameterView *pView;
    
    frame = [self frame];
    pRect = NSMakeRect(controlLeftPosition, controlTopPosition - 16, frame.size.width - 10, 16);
    pView = [[ParameterView alloc] initWithFrame:pRect];
    [pView addCheckBoxForFilter:f key:k displayView:v master:master];
    [self addSubview:pView];
    [pView setAutoresizingMask:NSViewMaxXMargin|NSViewMinYMargin];
    controlTopPosition -= 17;
    lastControlType = ctCheckBox;
}

// allocate the space required for a filter layer color well
// note: we can pack two color wells next to each other, if they follow one another!
- (void)tryColorWellForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
{
    NSRect frame;

    frame = [self frame];
    if (lastControlType != ctColorWell)
        colorWellOffset = controlLeftPosition;
    else
        {
        controlTopPosition += 28;
        colorWellOffset += floor((frame.size.width - 20)/2);
        if (colorWellOffset > controlLeftPosition + floor((frame.size.width - 20)/2))
            {
            colorWellOffset = controlLeftPosition;
            controlTopPosition -= 28;
            }
        }
    controlTopPosition -= 28;
    lastControlType = ctColorWell;
}

// add a filter layer color well
// note: we can pack two color wells next to each other, if they follow one another!
- (void)addColorWellForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
{
    NSRect pRect, frame;
    ParameterView *pView;

    frame = [self frame];
    if (lastControlType != ctColorWell)
        colorWellOffset = controlLeftPosition;
    else
    {
        controlTopPosition += 28;
        colorWellOffset += floor((frame.size.width - 20)/2);
        if (colorWellOffset > controlLeftPosition + floor((frame.size.width - 20)/2))
        {
            colorWellOffset = controlLeftPosition;
            controlTopPosition -= 28;
        }
    }
    pRect = NSMakeRect(colorWellOffset, controlTopPosition - 24, floor((frame.size.width - 20)/2), 24);
    pView = [[ParameterView alloc] initWithFrame:pRect];
    [pView addColorWellForFilter:f key:k displayView:v master:master];
    [self addSubview:pView];
    if (colorWellOffset == controlLeftPosition)
        [pView setAutoresizingMask:NSViewWidthSizable|NSViewMaxXMargin|NSViewMinYMargin];
    else
        [pView setAutoresizingMask:NSViewWidthSizable|NSViewMinXMargin|NSViewMinYMargin];
    controlTopPosition -= 28;
    lastControlType = ctColorWell;
}

// allocate the space required for a filter layer image view
- (void)tryImageWellForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
{
    controlTopPosition -= 48;
    lastControlType = ctImageWell;
}

// add a filter layer image view
- (void)addImageWellForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
{
    NSRect pRect, frame;
    ParameterView *pView;

    frame = [self frame];
    pRect = NSMakeRect(controlLeftPosition, controlTopPosition - 44, frame.size.width - 10, 44);
    pView = [[ParameterView alloc] initWithFrame:pRect];
    [pView addImageWellForFilter:f key:k displayView:v master:master];
    [self addSubview:pView];
    [pView setAutoresizingMask:NSViewWidthSizable|NSViewMaxXMargin|NSViewMinYMargin];
    controlTopPosition -= 48;
    lastControlType = ctImageWell;
}

// allocate the space required for a filter layer transform (4 sliders)
- (void)tryTransformForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
    {
    controlTopPosition -= 68;
    lastControlType = ctTransform;
    }

// add a filter layer transform (4 sliders)
- (void)addTransformForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
    {
    NSRect pRect, frame;
    ParameterView *pView;
    
    frame = [self frame];
    pRect = NSMakeRect(controlLeftPosition, controlTopPosition - 67, frame.size.width - 10, 67);
    pView = [[ParameterView alloc] initWithFrame:pRect];
    [pView addTransformForFilter:f key:k displayView:v master:master];
    [self addSubview:pView];
    [pView setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    controlTopPosition -= 68;
    lastControlType = ctTransform;
    }

// allocate the space required for a filter layer naked CIVector (4 editable text fields)
- (void)tryVectorForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
    {
    controlTopPosition -= 17;
    lastControlType = ctVector;
    }

// add a filter layer naked CIVector (4 editable text fields)
- (void)addVectorForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
    {
    NSRect pRect, frame;
    ParameterView *pView;
    
    frame = [self frame];
    pRect = NSMakeRect(controlLeftPosition, controlTopPosition - 16, frame.size.width - 10, 16);
    pView = [[ParameterView alloc] initWithFrame:pRect];
    [pView addVectorForFilter:f key:k displayView:v master:master];
    [self addSubview:pView];
    [pView setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    controlTopPosition -= 17;
    lastControlType = ctVector;
    }

// allocate the space required for a filter layer offset CIVector (2 editable text fields)
- (void)tryOffsetForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
    {
    controlTopPosition -= 17;
    lastControlType = ctOffset;
    }

// add a filter layer offset CIVector (2 editable text fields)
- (void)addOffsetForFilter:(CIFilter *)f key:(NSString *)k displayView:(CoreImageView *)v
    {
    NSRect pRect, frame;
    ParameterView *pView;
    
    frame = [self frame];
    pRect = NSMakeRect(controlLeftPosition, controlTopPosition - 16, frame.size.width - 10, 16);
    pView = [[ParameterView alloc] initWithFrame:pRect];
    [pView addOffsetForFilter:f key:k displayView:v master:master];
    [self addSubview:pView];
    [pView setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    controlTopPosition -= 17;
    lastControlType = ctOffset;
    }

// allocate the space required for an image layer image view
- (void)tryImageWellForImage:(CIImage *)im tag:(int)tag displayView:(CoreImageView *)v
{
    controlTopPosition -= 48;
    lastControlType = ctImageWell;
}

// add an image layer image view
- (void)addImageWellForImage:(CIImage *)im tag:(int)index displayView:(CoreImageView *)v
{
    NSRect pRect, frame;
    ParameterView *pView;

    frame = [self frame];
    pRect = NSMakeRect(controlLeftPosition, controlTopPosition - 44, frame.size.width - 10, 44);
    pView = [[ParameterView alloc] initWithFrame:pRect];
    [pView addImageWellForImage:im tag:index displayView:v master:master];
    [self addSubview:pView];
    [pView setAutoresizingMask:NSViewWidthSizable|NSViewMaxXMargin|NSViewMinYMargin];
    controlTopPosition -= 48;
    lastControlType = ctImageWell;
}

// allocate the space required for a text layer header
- (void)tryTextHeader:(NSString *)string
{
    NSRect R;
    
    R = [self bounds];
    R.origin.y += R.size.height - 38;
    R.size.height = 22;
    R.origin.x += 63;
    R.size.width -= 62 + 63;
    controlTopPosition = R.origin.y - 8;
}

// add a text layer header
- (void)addTextHeader:(NSString *)string tag:(int)index enabled:(BOOL)enabled
{
    NSRect R, S, cbR;
    NSString *name;
    NSCell *c;
    
    R = [self bounds];
    R.origin.y += R.size.height - 38;
    R.size.height = 22;
    R.origin.x += 33;
    R.size.width -= 62 + 63;
    // add enable/disable check box
    cbR = R;
    cbR.origin.x = 0;
    cbR.size.width = 58;
    cbR.size.height = 22;
    // label
    filterNameField = [[NSTextField alloc] initWithFrame:R];
    name = @"Text";
    // set text label to 9 point
    c = [filterNameField cell];
    // determine if we need to ellipsize
    name = [ParameterView ellipsizeField:[c drawingRectForBounds:[filterNameField bounds]].size.width font:[c font] string:name];
    [filterNameField setStringValue:name];
    [filterNameField setEditable:NO];
    [filterNameField setBezeled:NO];
    [filterNameField setDrawsBackground:NO];
    [filterNameField setAutoresizingMask:NSViewMaxXMargin|NSViewMinYMargin];
    [self addSubview:filterNameField];
    checkBox = [[NSButton alloc] initWithFrame:cbR];
    [checkBox setTarget:master];
    [checkBox setAction:@selector(enableCheckBoxAction:)];
    [checkBox setButtonType:NSSwitchButton];
    [checkBox setState:(enabled ? NSOnState : NSOffState)];
    [checkBox setTitle:@""];
    [checkBox setTag:index];
    [self addSubview:checkBox];
    // add + button
    S = R;
    S.origin.x = R.origin.x + R.size.width + 33;
    S.origin.y += 3;
    S.size.height -= 4;
    S.size.width = 22;
    plusbutton = [[NSButton alloc] initWithFrame:S];
    [[plusbutton cell] setButtonType:NSMomentaryPushButton];
    [[plusbutton cell] setBezelStyle:NSShadowlessSquareBezelStyle];
    [[plusbutton cell] setGradientType:NSGradientConcaveWeak];
    [plusbutton setImagePosition:NSImageOnly];
    [plusbutton setImage:[NSImage imageNamed:@"plusbutton"]];
    [plusbutton setBordered:NO];
    [plusbutton setTarget:master];
    [plusbutton setAction:@selector(plusButtonAction:)];
    [plusbutton setTag:index];
    [self addSubview:plusbutton];
    // add - button
    S.origin.x += 21;
    minusbutton = [[NSButton alloc] initWithFrame:S];
    [[minusbutton cell] setButtonType:NSMomentaryPushButton];
    [[minusbutton cell] setBezelStyle:NSShadowlessSquareBezelStyle];
    [[minusbutton cell] setGradientType:NSGradientConcaveWeak];
    [minusbutton setImagePosition:NSImageOnly];
    [minusbutton setImage:[NSImage imageNamed:@"minusbutton"]];
    [minusbutton setBordered:NO];
    [minusbutton setTarget:master];
    [minusbutton setAction:@selector(minusButtonAction:)];
    [minusbutton setTag:index];
    [self addSubview:minusbutton];
    controlTopPosition = R.origin.y - 8;
}

// allocate the space required for a text layer text view
- (void)tryTextViewForString
{
    controlTopPosition -= 88;
    lastControlType = ctTextView;
}

// add a text layer text view
- (void)addTextViewForString:(NSMutableDictionary *)d key:(NSString *)key displayView:(CoreImageView *)v
{
    NSRect pRect, frame;
    ParameterView *pView;
    
    frame = [self bounds];
    pRect = NSMakeRect(0, controlTopPosition - 88, frame.size.width, 88);
    pView = [[ParameterView alloc] initWithFrame:pRect];
    [pView addTextViewForString:d key:key displayView:v master:master];
    [self addSubview:pView];
    [pView setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    controlTopPosition -= 88;
    lastControlType = ctTextView;
}

// allocate the space required for a text layer scale slider
- (void)trySliderForText
{
    controlTopPosition -= kSliderVerticalAdvance + kVerticalGap;
    lastControlType = ctSlider;
}

// add a text layer scale slider
- (void)addSliderForText:(NSMutableDictionary *)d key:(NSString *)key lo:(float)lo hi:(float)hi displayView:(CoreImageView *)v
{
    NSRect pRect, frame;
    ParameterView *pView;
    
    frame = [self bounds];
    pRect = NSMakeRect(0, controlTopPosition - kSliderVerticalAdvance, frame.size.width - 12, kSliderVerticalAdvance);
    pView = [[ParameterView alloc] initWithFrame:pRect];
    [pView addSliderForText:d key:key lo:lo hi:hi displayView:v master:master];
    [self addSubview:pView];
    [pView setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    controlTopPosition -= kSliderVerticalAdvance + kVerticalGap;
    lastControlType = ctSlider;
}

@end
