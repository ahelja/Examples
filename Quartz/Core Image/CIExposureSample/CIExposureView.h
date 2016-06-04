#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>


@interface CIExposureView: NSView
{
    CIFilter    *filter;
    float        exposureValue;
}

- (void)sliderChanged: (id)sender;

@end
