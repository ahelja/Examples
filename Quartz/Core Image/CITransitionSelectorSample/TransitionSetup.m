#import "TransitionSelectorView.h"


@implementation TransitionSelectorView (TransitionSetup)

- (void)setupTransitions
{
    CIVector  *extent;
    float      w,h;
    int        i;

    w      = thumbnailWidth;
    h      = thumbnailHeight;

    extent = [CIVector vectorWithX: 0  Y: 0  Z: w  W: h];

    transitions[0] = [CIFilter filterWithName: @"CISwipeTransition"
            keysAndValues: @"inputExtent", extent,
                @"inputColor", [CIColor colorWithRed:0  green:0 blue:0  alpha:0],
                @"inputAngle", [NSNumber numberWithFloat: 0.3*M_PI],
                @"inputWidth", [NSNumber numberWithFloat: 80.0],
                @"inputOpacity", [NSNumber numberWithFloat: 0.0], nil];

    transitions[1] = [CIFilter filterWithName: @"CIDissolveTransition"];
    
    transitions[2] = [CIFilter filterWithName: @"CISwipeTransition"			// dupe
            keysAndValues: @"inputExtent", extent,
                @"inputColor", [CIColor colorWithRed:0  green:0 blue:0  alpha:0],
                @"inputAngle", [NSNumber numberWithFloat: M_PI],
                @"inputWidth", [NSNumber numberWithFloat: 2.0],
                @"inputOpacity", [NSNumber numberWithFloat: 0.2], nil];

    transitions[3] = [CIFilter filterWithName: @"CIModTransition"
            keysAndValues:
                @"inputCenter",[CIVector vectorWithX: 0.5*w Y: 0.5*h],
                @"inputAngle", [NSNumber numberWithFloat: M_PI*0.1],
                @"inputRadius", [NSNumber numberWithFloat: 30.0],
                @"inputCompression", [NSNumber numberWithFloat: 10.0], nil];

    transitions[4] = [CIFilter filterWithName: @"CIFlashTransition"
            keysAndValues: @"inputExtent", extent,
                @"inputCenter",[CIVector vectorWithX: 0.3*w Y: 0.7*h],
                @"inputColor", [CIColor colorWithRed:1 green:.8 blue:.6 alpha:1],
                @"inputMaxStriationRadius", [NSNumber numberWithFloat: 2.5],
                @"inputStriationStrength", [NSNumber numberWithFloat: 0.5],
                @"inputStriationContrast", [NSNumber numberWithFloat: 1.37],
                @"inputFadeThreshold", [NSNumber numberWithFloat: 0.85], nil];

    transitions[5] = [CIFilter filterWithName: @"CIDisintegrateWithMaskTransition"
            keysAndValues:
                @"inputMaskImage", [self maskImage],
				@"inputShadowRadius", [NSNumber numberWithFloat: 10.0],
                @"inputShadowDensity", [NSNumber numberWithFloat:0.7],
                @"inputShadowOffset", [CIVector vectorWithX: 0.0  Y: -0.05*h], nil];

    transitions[6] = [CIFilter filterWithName: @"CIRippleTransition"
            keysAndValues: @"inputExtent", extent,
                @"inputShadingImage", [self shadingImage],
                @"inputCenter",[CIVector vectorWithX: 0.5*w Y: 0.5*h],
                @"inputWidth", [NSNumber numberWithFloat: 80],
                @"inputScale", [NSNumber numberWithFloat: 30.0], nil];

    transitions[7] = [CIFilter filterWithName: @"CICopyMachineTransition"
            keysAndValues: @"inputExtent", extent,
                @"inputColor", [CIColor colorWithRed:.6 green:1 blue:.8 alpha:1],
                @"inputAngle", [NSNumber numberWithFloat: 0],
                @"inputWidth", [NSNumber numberWithFloat: 40],
                @"inputOpacity", [NSNumber numberWithFloat: 1.0], nil];

    transitions[8] = [CIFilter filterWithName: @"CIPageCurlTransition"
            keysAndValues: @"inputExtent", extent,
                @"inputShadingImage", [self shadingImage],
				@"inputBacksideImage", [self blankImage],
                @"inputAngle",[NSNumber numberWithFloat: -0.2*M_PI],
                @"inputRadius", [NSNumber numberWithFloat: 70], nil];

    for(i=0 ; i<TRANSITION_COUNT ; i++)
        [transitions[i] retain];
}

@end
