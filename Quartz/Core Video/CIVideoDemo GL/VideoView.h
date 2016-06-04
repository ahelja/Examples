/* VideoView */

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
#import <QuartzCore/QuartzCore.h>

@interface VideoView : NSOpenGLView
{

    NSRecursiveLock			*lock;
    QTMovie				*qtMovie;
    QTTime				movieDuration;
    QTVisualContextRef			qtVisualContext;
    CVDisplayLinkRef			displayLink;
    CGDirectDisplayID			viewDisplayID;
    CVImageBufferRef			currentFrame;
    CIFilter				*effectFilter;
    CIContext				*ciContext;
    
    NSDictionary			*fontAttributes;
    
    BOOL				needsReshape;
    id					delegate;
}

- (void)setQTMovie:(QTMovie*)inMovie;

- (IBAction)setMovieTime:(id)sender;
- (IBAction)nextFrame:(id)sender;
- (IBAction)prevFrame:(id)sender;
- (IBAction)scrub:(id)sender;
- (IBAction)togglePlay:(id)sender;
- (IBAction)setFilterParameter:(id)sender;
- (IBAction)safeFrameToFile:(id)sender;

- (void)setFilterCenterFromMouseLocation:(NSPoint)where;

- (void)updateCurrentFrame;
- (void)renderCurrentFrame;

- (QTTime)movieDuration;
- (QTTime)currentTime;
- (void)setTime:(QTTime)inTime;

@end
