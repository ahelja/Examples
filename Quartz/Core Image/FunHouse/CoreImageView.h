// CoreImageView.h
// Author: Mark Zimmer
// 12/08/04
// Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import "SampleCIView.h"

@class FunHouseWindowController;

// these are the possible item types that get moved
// within a mouseDown mouseDragged mouseUp loop
enum
    {
    pmNone = 0,
    pmPoint,            // moving a filter point (example: the inputCenter parameter to CIBumpDistortion)
    pmTopLeft,          // moving the top left point of a filter rectangle (example: the inputRectangle parameter to CICrop)
    pmBottomLeft,       // moving the bottom left point of a filter rectangle (example: the inputRectangle parameter to CICrop)
    pmTopRight,         // moving the top right point of a filter rectangle (example: the inputRectangle parameter to CICrop)
    pmBottomRight,      // moving the bottom right point of a filter rectangle (example: the inputRectangle parameter to CICrop)
    pmImageOffset,      // moving an image layer's offset
    pmTextOffset,       // moving a text layer's origin
    pmTransformOffset,  // moving the offset of a filter's affine transform parameter
    pmSpotLight,        // moving a CISpotLight position parameter
    pm3DPoint,          // moving the XY components of a filter 3D position parameter
    };

@interface CoreImageView : SampleCIView
{
    BOOL initialized;
    NSBundle *bundle;
    FunHouseWindowController *controller;
    // these fields are for mouse movement - they're set up in mouseDown, and used in mouseDragged and mouseUp
    int parmIndex;
    NSString *parmKey;
    int parmMode;
    NSString *savedActionName;
    BOOL movingNow;
    // this onee is used to indicate that the filter, image, and text layer origin handles are to be displayed
    BOOL displayingPoints;
    // the tracking rectangle is set up so mouseEntered and mouseExited events will be generated
    NSTrackingRectTag lastTrack;
    // view transform
    float viewTransformScale;
    float viewTransformOffsetX;
    float viewTransformOffsetY;
}

- (void)awakeFromNib;

- (void)setFunHouseWindowController:(FunHouseWindowController *)c;
- (CIContext *)context;

// view transform setters and getters
- (void)setViewTransformScale:(float)scale;
- (void)setViewTransformOffsetX:(float)x andY:(float)y;
- (BOOL)isScaled;

// setters for filter parameters (undo glue code)
- (void)setFilter:(CIFilter *)f value:(id)val forKey:(NSString *)key; // undoable operation
- (void)setDict:(NSMutableDictionary *)f value:(id)val forKey:(NSString *)key; // undoable operation
- (void)setActionNameForFilter:(CIFilter *)f key:(NSString *)key;
- (void)setActionNameForTextLayerKey:(NSString *)key;

@end
