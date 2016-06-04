/* SampleCIView.h - simple OpenGL based CoreImage view */

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CoreImage.h>

@interface SampleCIView: NSOpenGLView
{
    CIContext *_context;
    CIImage   *_image;
    NSRect     _lastBounds;
}

- (void)setImage:(CIImage *)image;
- (void)setImage:(CIImage *)image dirtyRect:(CGRect)r;

- (CIImage *)image;

// Called when the view bounds have changed
- (void)viewBoundsDidChange:(NSRect)bounds;

@end

@interface NSObject (SampleCIViewDraw)

// If defined in the view subclass, called when rendering
- (void)drawRect:(NSRect)bounds inCIContext:(CIContext *)ctx;

@end
