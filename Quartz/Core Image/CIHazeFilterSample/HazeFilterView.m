#import "HazeFilterView.h"
#import "MyHazeFilter.h"


@implementation HazeFilterView

- (void)distanceSliderChanged: (id)sender
{
    distance = [sender floatValue];
    [self setNeedsDisplay: YES];
}

- (void)slopeSliderChanged: (id)sender
{
    slope = [sender floatValue];
    [self setNeedsDisplay: YES];
}

- (void)drawRect: (NSRect)rect
{
    CGRect  cg = CGRectMake(NSMinX(rect), NSMinY(rect), NSWidth(rect), NSHeight(rect));
    
	CIContext* context = [[NSGraphicsContext currentContext] CIContext];
	
    if(filter == nil)
    {
        NSURL       *url;

        [MyHazeFilter class];   // make sure initialize is called

        url      = [NSURL fileURLWithPath: [[NSBundle mainBundle]
            pathForResource: @"CraterLake"  ofType: @"jpg"]];
        filter   = [CIFilter filterWithName: @"MyHazeRemover"
            keysAndValues: @"inputImage", [CIImage imageWithContentsOfURL: url],
            @"inputColor", [CIColor colorWithRed: 0.7  green: 0.9  blue: 1], nil];
        [filter retain];
    }

    [filter setValue: [NSNumber numberWithFloat: distance]
        forKey: @"inputDistance"];
    [filter setValue: [NSNumber numberWithFloat: slope]
        forKey: @"inputSlope"];
	if (context != nil)
		[context drawImage: [filter valueForKey: @"outputImage"]
			atPoint: cg.origin  fromRect: cg];
}

@end
