#import <Cocoa/Cocoa.h>

typedef enum {	// Types of Bezier Paths.
    SquarePath,
    CirclePath,
    ArcPath,
    LinePath
} BezierPathType;

typedef enum { // Types of Cap Styles.
    ButtLine,
    SquareLine,
    RoundLine
} CapStyleType;

@interface BezierView : NSView
{
    NSColor *lineColor, *fillColor, *backgroundColor;
    NSBezierPath *path;
    float lineWidth, angle, dashCount;
    BezierPathType pathType;
    CapStyleType capStyle;
    BOOL filled;
    float dashArray[3];
    
    // Outlets
    id lineColorWell;
    id fillColorWell;
    id backgroundColorWell;
    id lineWidthSlider;
    id pathTypeMatrix;
    id filledBox;
    id angleSlider;
    id capStyleMatrix;
    id zoomSlider;
    id lineTypeMatrix;
}

// Outlet-setting methods (we need these to set the initial values for the controls)

- (void) setLineColor:(NSColor *)newColor;
- (void) setFillColor:(NSColor *)newColor;
- (void) setBackgroundColor:(NSColor *)newColor;
- (void) setPath:(NSBezierPath *)newPath;
- (void) setLineWidth:(float) newWidth;
- (void) setAngle:(float) newAngle;
- (void) setZoom:(float) scaleFactor;

// Methods to change the path attributes.

- (void) setPathType:(id)sender;
- (void) setFilled:(id)sender;
- (void) setCapStyle:(id)sender;
- (void) changeLineWidth:(id)sender;
- (void) changeAngleSlider:(id)sender;
- (void) changeLineType:(id)sender;
- (void) changeZoomSlider:(id)sender;
- (void) changeLineColor:(id)sender;
- (void) changeFillColor:(id)sender;
- (void) changeBackgroundColor:(id)sender;

@end
