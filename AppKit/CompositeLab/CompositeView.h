/*
   CompositeView.h
*/

#import <Cocoa/Cocoa.h>

// The possible draw modes for the source.

typedef enum {	// Various pictures that can be drawn in the source (or even destination) areas
  TrianglePicture = 0,
  CirclePicture,
  DiamondPicture,
  HeartPicture,
  FlowerPicture,
  CustomPicture
} CompositeViewPicture;

@interface CompositeView:NSView {
    id source, destination, result, customImage;
    NSRect sRect, dRect, rRect;
    NSCompositingOperation operator;
    CompositeViewPicture sourcePicture;
    NSColor *sourceColor, *destColor, *backgroundColor;

    // Outlets...
    id sourceColorWell;
    id destColorWell;
    id backColorWell;
    id sourcePictureMatrix;
}

// Init/dealloc
- (id)initWithFrame:(NSRect)rect;
- (void)dealloc;

// Target-action methods
- (void)setSourcePicture:(id)sender;
- (void)setOperator:(id)sender;
- (void)changeCustomImage:(id)sender;
- (void)changeSourceColor:(id)sender;
- (void)changeDestColor:(id)sender;
- (void)changeBackgroundColor:(id)sender;

// Outlet-setting methods (we need these to set the initial colors)
- (void)setSourceColorWell:(id)anObject;
- (void)setDestColorWell:(id)anObject;
- (void)setBackColorWell:(id)anObject;

// Methods to change the colors and display
- (void)changeSourceColorTo:(NSColor *)color andDisplay:(BOOL)flag;
- (void)changeDestColorTo:(NSColor *)color andDisplay:(BOOL)flag;
- (void)changeBackgroundColorTo:(NSColor *)color andDisplay:(BOOL)flag;
- (BOOL)changeCustomImageTo:(NSImage *)newImage;

// Callbacks from NSImage drawing methods
- (void)drawSource:(NSCustomImageRep *)imageRep;
- (void)drawDestination:(NSCustomImageRep *)imageRep;
- (void)drawResult:(NSCustomImageRep *)imageRep;

// Other methods
- (void)drawRect:(NSRect)rect;
- (NSCompositingOperation)operator;

// Dragging methods
- (unsigned int)draggingEntered:(id <NSDraggingInfo>)sender;
- (unsigned int)draggingUpdated:(id <NSDraggingInfo>)sender;
- (void)draggingExited:(id <NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
- (void)concludeDragOperation:(id <NSDraggingInfo>)sender;
- (void)doColorDrag:(id <NSDraggingInfo>)sender;

@end



