#import "CIBevelView.h"


@implementation CIBevelView

- (id)initWithFrame: (NSRect)frameRect
{
    if((self = [super initWithFrame:frameRect]) != nil)
    {
        points[0] = CGPointMake(0.5 * frameRect.size.width, frameRect.size.height - 100.0);
        points[1] = CGPointMake(150.0, 100.0);
        points[2] = CGPointMake(frameRect.size.width - 150.0, 100.0);
        points[3] = CGPointMake(0.7*points[0].x + 0.3*points[2].x, 0.7*points[0].y + 0.3*points[2].y);
                
        NSURL   *url = [NSURL fileURLWithPath: [[NSBundle mainBundle]
            pathForResource: @"lightball" ofType: @"tiff"]];
        CIImage *lightball = [CIImage imageWithContentsOfURL: url];
                
        heightFieldFilter = [CIFilter filterWithName:@"CIHeightFieldFromMask" keysAndValues:
            @"inputRadius", [NSNumber numberWithFloat: 15.0], nil];
        twirlFilter = [CIFilter filterWithName:@"CITwirlDistortion" keysAndValues: 
            @"inputCenter",[CIVector vectorWithX: 0.5*frameRect.size.width Y: 0.5*frameRect.size.height],
            @"inputRadius", [NSNumber numberWithFloat: 300.0], 
            @"inputAngle", [NSNumber numberWithFloat: 0.0], nil];
        shadedFilter = [CIFilter filterWithName:@"CIShadedMaterial" keysAndValues:
            @"inputShadingImage", lightball,
            @"inputScale", [NSNumber numberWithFloat: 20.0], nil];
        [twirlFilter retain];
        [heightFieldFilter retain];
        [shadedFilter retain];
        
        // 1/30 second should give us decent animation
        [NSTimer scheduledTimerWithTimeInterval: 1.0/30.0 target: self selector: @selector(changeTwirlAngle:) userInfo: nil repeats: YES];
    }

    return self;
}

- (void)dealloc
{
    [twirlFilter release];
    [heightFieldFilter release];
    [shadedFilter release];
    [lineImage release];
    [super dealloc];
}


- (void) changeTwirlAngle: (NSTimer*)timer
{
    angleTime += [timer timeInterval];
    [twirlFilter setValue: [NSNumber numberWithFloat: -0.2 * sin(angleTime*5.0)] forKey: @"inputAngle"];
    [self updateImage];
}

- (void)mouseDragged: (NSEvent *)event
{
    NSPoint  loc;

    loc = [self convertPoint: [event locationInWindow] fromView: nil];
    points[currentPoint].x = loc.x;
    points[currentPoint].y = loc.y;
    [lineImage release];
    lineImage = nil;

    // normally we'd want this, but the timer will cause us to redisplay anyway
    // [self setNeedsDisplay: YES];
}

- (void)mouseDown: (NSEvent *)event
{
    size_t   best, i;
    float    x,y, d,t;
    NSPoint  loc;

    d   = 1e4;
    loc = [self convertPoint: [event locationInWindow] fromView: nil];
    for(i=0 ; i<NUM_POINTS ; i++)
    {
        x = points[i].x - loc.x;
        y = points[i].y - loc.y;
        t = x*x + y*y;

        if(t < d)  currentPoint = i,  d = t;
    }

    [self mouseDragged: event];
}


- (void)updateImage
{
    CIContext    *context;
    CIFilter     *filter;

    context = [[NSGraphicsContext currentContext] CIContext];
    if(!lineImage)
    {
        CGContextRef        cg;
        CGLayerRef        layer;
        NSRect                bounds;
        int                i, j;

        bounds  = [self bounds];
        layer   = [context createCGLayerWithSize: CGSizeMake(NSWidth(bounds), NSHeight(bounds))  info: nil];
        cg      = CGLayerGetContext(layer);

        CGContextSetRGBStrokeColor(cg, 1,1,1,1);
        CGContextSetLineCap(cg, kCGLineCapRound);

        CGContextSetLineWidth(cg, 60.0);
        CGContextMoveToPoint(cg, points[0].x, points[0].y);
        for(i = 1; i < NUM_POINTS; ++i)
                CGContextAddLineToPoint(cg, points[i].x, points[i].y);
        CGContextStrokePath(cg);

        lineImage = [[CIImage alloc] initWithCGLayer: layer];
        CGLayerRelease(layer);
    }

    [heightFieldFilter setValue: lineImage  forKey:@"inputImage"];
    [twirlFilter setValue:[heightFieldFilter valueForKey: @"outputImage"] forKey:@"inputImage"];
    [shadedFilter setValue:[twirlFilter valueForKey: @"outputImage"] forKey:@"inputImage"];

    [self setImage: [shadedFilter valueForKey: @"outputImage"]];
}

@end
