#import "TransitionSelectorView.h"




@implementation TransitionSelectorView

- (void)awakeFromNib
{
    NSTimer    *timer;
    NSURL      *url;

    thumbnailWidth  = 340.0;
    thumbnailHeight = 240.0;
    thumbnailGap    = 20.0;


    url   = [NSURL fileURLWithPath: [[NSBundle mainBundle]
        pathForResource: @"Rose" ofType: @"jpg"]];
    [self setSourceImage: [CIImage imageWithContentsOfURL: url]];


    url   = [NSURL fileURLWithPath: [[NSBundle mainBundle]
        pathForResource: @"Frog" ofType: @"jpg"]];
    [self setTargetImage: [CIImage imageWithContentsOfURL: url]];


    timer = [NSTimer scheduledTimerWithTimeInterval: 1.0/30.0  target: self
        selector: @selector(timerFired:)  userInfo: nil  repeats: YES];

    base = [NSDate timeIntervalSinceReferenceDate];
    [[NSRunLoop currentRunLoop] addTimer: timer  forMode: NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer: timer  forMode: NSEventTrackingRunLoopMode];
}


- (void)setSourceImage: (CIImage *)source
{
    [source retain];
    [sourceImage release];
    sourceImage = source;
}

- (void)setTargetImage: (CIImage *)target
{
    [target retain];
    [targetImage release];
    targetImage = target;
}

- (CIImage *)shadingImage
{
    if(!shadingImage)
    {
        NSURL  *url;

        url   = [NSURL fileURLWithPath: [[NSBundle mainBundle]
            pathForResource: @"Shading" ofType: @"tiff"]];
        shadingImage = [[CIImage alloc] initWithContentsOfURL: url];
    }

    return shadingImage;
}

- (CIImage *)blankImage
{
    if(!blankImage)
    {
        NSURL  *url;

        url   = [NSURL fileURLWithPath: [[NSBundle mainBundle]
            pathForResource: @"Blank" ofType: @"jpg"]];
        blankImage = [[CIImage alloc] initWithContentsOfURL: url];
    }

    return blankImage;
}

- (CIImage *)maskImage
{
    if(!maskImage)
    {
        NSURL  *url;

        url   = [NSURL fileURLWithPath: [[NSBundle mainBundle]
            pathForResource: @"Mask" ofType: @"jpg"]];
        maskImage = [[CIImage alloc] initWithContentsOfURL: url];
    }

    return maskImage;
}

- (void)timerFired: (id)sender
{
    [self setNeedsDisplay: YES];
}


- (CIImage *)imageForTransition: (int)transitionNumber  atTime: (float)t
{
    CIFilter  *transition, *crop;

    transition    = transitions[transitionNumber];

    if(fmodf(t, 2.0) < 1.0f)
    {
        [transition setValue: sourceImage  forKey: @"inputImage"];
        [transition setValue: targetImage  forKey: @"inputTargetImage"];
    }

    else
    {
        [transition setValue: targetImage  forKey: @"inputImage"];
        [transition setValue: sourceImage  forKey: @"inputTargetImage"];
    }

    [transition setValue: [NSNumber numberWithFloat: 0.5*(1-cos(fmodf(t, 1.0f) * M_PI))]
        forKey: @"inputTime"];

    crop = [CIFilter filterWithName: @"CICrop"
        keysAndValues: @"inputImage", [transition valueForKey: @"outputImage"],
            @"inputRectangle", [CIVector vectorWithX: 0  Y: 0
            Z: thumbnailWidth  W: thumbnailHeight], nil];

    return [crop valueForKey: @"outputImage"];
}

- (void)drawRect: (NSRect)rectangle
{
    CGPoint origin;
    CGRect  thumbFrame;
    float   t;
    int     w,i;

    thumbFrame = CGRectMake(0,0, thumbnailWidth,thumbnailHeight);
    t          = 0.4*([NSDate timeIntervalSinceReferenceDate] - base);

	CIContext* context = [[NSGraphicsContext currentContext] CIContext];

    if(transitions[0] == nil)
        [self setupTransitions];

    w = (int)ceil(sqrt((double)TRANSITION_COUNT));

    for(i=0 ; i<TRANSITION_COUNT ; i++)
    {
        origin.x = (i % w) * (thumbnailWidth  + thumbnailGap);
        origin.y = (i / w) * (thumbnailHeight + thumbnailGap);
	
		if (context != nil)
			[context drawImage: [self imageForTransition: i
				atTime: t + 0.1*i]  atPoint: origin  fromRect: thumbFrame];
    }
}

@end
