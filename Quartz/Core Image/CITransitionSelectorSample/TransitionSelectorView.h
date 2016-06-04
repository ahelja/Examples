#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>


#define TRANSITION_COUNT   (9)


@interface TransitionSelectorView: NSView
{
    CIImage        *sourceImage, *targetImage;
    CIImage        *shadingImage;
	CIImage		   *blankImage;
	CIImage		   *maskImage;

    CIFilter       *transitions[TRANSITION_COUNT];

    NSTimeInterval  base;

    float           thumbnailWidth, thumbnailHeight;
    float           thumbnailGap;
}

- (void)setSourceImage: (CIImage *)source;
- (void)setTargetImage: (CIImage *)target;

- (CIImage *)shadingImage;
- (CIImage *)blankImage;
- (CIImage *)maskImage;

@end


@interface TransitionSelectorView (TransitionSetup)

- (void)setupTransitions;

@end
