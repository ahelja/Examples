#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import "SampleCIView.h"

#define NUM_POINTS  (4)

@interface CIBevelView: SampleCIView
{
    size_t          currentPoint;
    CGPoint         points[NUM_POINTS];
    NSTimeInterval	angleTime;
    CIImage        *lineImage;
    CIFilter       *twirlFilter;
    CIFilter       *heightFieldFilter;
    CIFilter       *shadedFilter;
}

- (void)updateImage;

@end
