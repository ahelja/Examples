#import <Cocoa/Cocoa.h>

@interface DotView : NSView {
    NSPoint center;
    NSColor *color;
    float radius;
}

// Standard view create/free methods
- (id)initWithFrame:(NSRect)frame;
- (void)dealloc;

// Drawing
- (void)drawRect:(NSRect)rect;
- (BOOL)isOpaque;

// Event handling
- (void)mouseUp:(NSEvent *)event;

// Custom methods for actions this view implements
- (IBAction)setRadius:(id)sender;
- (IBAction)setColor:(id)sender;

@end
