#import "CIMicroPaintView.h"


@implementation CIMicroPaintView

- (id)initWithFrame:(NSRect)frame 
{
    self = [super initWithFrame:frame];
    if (self == nil)
	return nil;

    brushSize = 25.0;

    color = [NSColor colorWithDeviceRed: 0.0 green: 0.0 blue: 0.0 alpha: 1.0];
    [color retain];

    brushFilter = [CIFilter filterWithName: @"CIRadialGradient" keysAndValues:
		   @"inputColor1", [CIColor colorWithRed:0.0 green:0.0
		   blue:0.0 alpha:0.0], @"inputRadius0", [NSNumber
		   numberWithFloat: 0.0], nil];
    [brushFilter retain];

    compositeFilter = [CIFilter filterWithName: @"CISourceOverCompositing"];
    [compositeFilter retain];

    return self;
}

- (void)dealloc
{
    [imageAccumulator release];
    [brushFilter release];
    [compositeFilter release];
    [color release];
    [super dealloc];
}

- (void)viewBoundsDidChange:(NSRect)bounds
{
    CIImageAccumulator *c;
    CIFilter *f;

    if (imageAccumulator != nil
	&& CGRectEqualToRect (*(CGRect *)&bounds, [imageAccumulator extent]))
    {
	return;
    }

    /* Create a new accumulator and composite the old one over the it. */

    c = [[CIImageAccumulator alloc]
	 initWithExtent:*(CGRect *)&bounds format:kCIFormatRGBA16];
    f = [CIFilter filterWithName:@"CIConstantColorGenerator"
	 keysAndValues:@"inputColor",
	 [CIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:1.0], nil];
    [c setImage:[f valueForKey:@"outputImage"]];

    if (imageAccumulator != nil)
    {
	f = [CIFilter filterWithName:@"CISourceOverCompositing"
	     keysAndValues:@"inputImage", [imageAccumulator image],
	     @"inputBackgroundImage", [c image], nil];
	[c setImage:[f valueForKey:@"outputImage"]];
    }

    [imageAccumulator release];
    imageAccumulator = c;

    [self setImage:[imageAccumulator image]];
}

- (void)mouseDragged:(NSEvent *)event
{
    CGRect   rect;
    NSPoint  loc = [self convertPoint: [event locationInWindow] fromView: nil];
    CIColor *cicolor;

    rect = CGRectMake(loc.x-brushSize, loc.y-brushSize,
        2.0*brushSize, 2.0*brushSize);
    [brushFilter setValue: [NSNumber numberWithFloat: brushSize]
        forKey: @"inputRadius1"];
    cicolor = [[CIColor alloc] initWithColor: color];
    [brushFilter setValue: cicolor  forKey: @"inputColor0"];
    [cicolor release];	//cicolor is retained by the brushFilter
    [brushFilter setValue: [CIVector vectorWithX: loc.x Y:loc.y]
        forKey: @"inputCenter"];
    
    [compositeFilter setValue: [brushFilter valueForKey: @"outputImage"]
        forKey: @"inputImage"];
    [compositeFilter setValue: [imageAccumulator image]
        forKey: @"inputBackgroundImage"];
    
    [imageAccumulator setImage: [compositeFilter valueForKey: @"outputImage"]
        dirtyRect: rect];

    [self setImage: [imageAccumulator image] dirtyRect: rect];
}

- (void)mouseDown:(NSEvent *)event
{
    [self mouseDragged: event];
}

@end
