#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>


@interface MyHazeFilter: CIFilter
{
    CIImage   *inputImage;
    CIColor   *inputColor;
    NSNumber  *inputDistance;
    NSNumber  *inputSlope;
}

@end
