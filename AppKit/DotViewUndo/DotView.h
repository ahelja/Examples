#import <Cocoa/Cocoa.h>

@interface DotView : NSView {
    // Instance variables
    NSPoint center;
    float radius;
    NSColor *color;
}

// Standard view create/free methods
- (id)initWithFrame:(NSRect)frame;
- (void)dealloc;

// Drawing
- (void)drawRect:(NSRect)rect;
- (BOOL)isOpaque;

// Event handling
- (void)mouseUp:(NSEvent *)event;

// Action methods declared
- (void)changeSize:(id)sender;
- (void)changeColor:(id)sender;

// Accessors for attributes for the dot
- (void)setCenter:(NSPoint)newCenter;
- (NSPoint)center;
- (void)setRadius:(float)newRadius;
- (float)radius;
- (void)setColor:(NSColor *)newColor;
- (NSColor *)color;

@end
