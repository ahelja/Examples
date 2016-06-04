#import "CIExposureView.h"

@implementation CIExposureView

- (void)sliderChanged: (id)sender
{
    exposureValue = [sender floatValue];
    [self setNeedsDisplay: YES];
}

- (void)drawRect: (NSRect)rect
{
    CGRect  cg = CGRectMake(NSMinX(rect), NSMinY(rect),
        NSWidth(rect), NSHeight(rect));

	CIContext* context = [[NSGraphicsContext currentContext] CIContext];
    
    if(filter == nil)
    {
        CIImage   *image;

        image    = [CIImage imageWithContentsOfURL: [NSURL fileURLWithPath:
            [[NSBundle mainBundle] pathForResource: @"Rose" ofType: @"jpg"]]];
        filter   = [CIFilter filterWithName: @"CIExposureAdjust"
            keysAndValues: @"inputImage", image, nil];

        [filter retain];
    }

    [filter setValue: [NSNumber numberWithFloat: exposureValue]
        forKey: @"inputEV"];
	if (context != nil)
		[context drawImage: [filter valueForKey: @"outputImage"]
			atPoint: cg.origin  fromRect: cg];
}

@end
