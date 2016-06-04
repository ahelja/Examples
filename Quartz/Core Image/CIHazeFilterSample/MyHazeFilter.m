#import "MyHazeFilter.h"


@implementation MyHazeFilter

static CIKernel *hazeRemovalKernel = nil;

+ (void)initialize
{
    [CIFilter registerFilterName: @"MyHazeRemover"  constructor: self
        classAttributes: [NSDictionary dictionaryWithObjectsAndKeys:

        @"Haze Remover",                       kCIAttributeFilterDisplayName,

        [NSArray arrayWithObjects:
            kCICategoryColorAdjustment, kCICategoryVideo, kCICategoryStillImage,
            kCICategoryInterlaced, kCICategoryNonSquarePixels,
            nil],                              kCIAttributeFilterCategories,

        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithDouble:  0.0], kCIAttributeMin,
            [NSNumber numberWithDouble:  1.0], kCIAttributeMax,
            [NSNumber numberWithDouble:  0.0], kCIAttributeSliderMin,
            [NSNumber numberWithDouble:  0.7], kCIAttributeSliderMax,
            [NSNumber numberWithDouble:  0.2], kCIAttributeDefault,
            [NSNumber numberWithDouble:  0.0], kCIAttributeIdentity,
            kCIAttributeTypeScalar,            kCIAttributeType,
            nil],                              @"inputDistance",

        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithDouble: -0.01], kCIAttributeSliderMin,
            [NSNumber numberWithDouble:  0.01], kCIAttributeSliderMax,
            [NSNumber numberWithDouble:  0.00], kCIAttributeDefault,
            [NSNumber numberWithDouble:  0.00], kCIAttributeIdentity,
            kCIAttributeTypeScalar,             kCIAttributeType,
            nil],                               @"inputSlope",

        [NSDictionary dictionaryWithObjectsAndKeys:
            [CIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:1.0], kCIAttributeDefault,
            nil],                               @"inputColor",

        nil]];
}

+ (CIFilter *)filterWithName: (NSString *)name
{
    CIFilter  *filter;

    filter = [[self alloc] init];
    return [filter autorelease];
}

- (id)init
{
    if(hazeRemovalKernel == nil)
    {
        NSBundle    *bundle = [NSBundle bundleForClass: [self class]];
        NSString    *code = [NSString stringWithContentsOfFile: [bundle
            pathForResource: @"MyHazeRemoval" ofType: @"cikernel"]];
        NSArray     *kernels = [CIKernel kernelsWithString: code];

        hazeRemovalKernel = [[kernels objectAtIndex:0] retain];
    }

    return [super init];
}


- (CIImage *)outputImage
{
    CISampler *src = [CISampler samplerWithImage: inputImage];

    return [self apply: hazeRemovalKernel, src, inputColor, inputDistance,
        inputSlope, @"definition", [src definition], nil];
}

@end
