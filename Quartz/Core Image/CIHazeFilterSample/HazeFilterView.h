#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@interface HazeFilterView: NSView
{
    CIFilter     *filter;
    float         distance, slope;
}

- (void)distanceSliderChanged: (id)sender;
- (void)slopeSliderChanged: (id)sender;

@end
