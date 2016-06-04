#import "VideoView.h"
#include <mach/mach_time.h>


@interface VideoView (private)

- (CVReturn)renderTime:(const CVTimeStamp *)timeStamp;

@end

static CVReturn renderCallback(CVDisplayLinkRef displayLink, 
                                                const CVTimeStamp *inNow, 
                                                const CVTimeStamp *inOutputTime, 
                                                CVOptionFlags flagsIn, 
                                                CVOptionFlags *flagsOut, 
                                                void *displayLinkContext)
{
    return [(VideoView*)displayLinkContext renderTime:inOutputTime];
}

						
@implementation VideoView

//--------------------------------------------------------------------------------------------------

- (BOOL)isOpaque
{
    return YES;
}

//--------------------------------------------------------------------------------------------------

- (void)windowChangedScreen:(NSNotification*)inNotification
{
    NSWindow *window = [inNotification object]; 
    CGDirectDisplayID displayID = (CGDirectDisplayID)[[[[window screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];

    if((displayID != NULL) && (viewDisplayID != displayID))
    {
	CVDisplayLinkSetCurrentCGDisplay(displayLink, displayID);
	viewDisplayID = displayID;
    }
}


//--------------------------------------------------------------------------------------------------
- (void)prepareOpenGL
{
	CVReturn			    ret;

	lock = [[NSRecursiveLock alloc] init];
	
	/* Create CGColorSpaceRef */
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		
	/* Create CIContext */
	ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[[self openGLContext] CGLContextObj]
	pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
	options:[NSDictionary dictionaryWithObjectsAndKeys:
	(id)colorSpace,kCIContextOutputColorSpace,
	(id)colorSpace,kCIContextWorkingColorSpace,nil]] retain];
	CGColorSpaceRelease(colorSpace);
	
	/* Create CIFilter */
	effectFilter = [[CIFilter filterWithName:@"CILineScreen"] retain]; // Effect filter	
	[effectFilter setDefaults];
	  	    		
	/* Create display link */
	CGOpenGLDisplayMask	totalDisplayMask = 0;
	int			virtualScreen;
	long			displayMask;
	NSOpenGLPixelFormat	*openGLPixelFormat = [self pixelFormat];
	viewDisplayID = (CGDirectDisplayID)[[[[[self window] screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];  // we start with our view on the main display
	
	// build up list of displays from OpenGL's pixel format
	for (virtualScreen = 0; virtualScreen < [openGLPixelFormat  numberOfVirtualScreens]; virtualScreen++)
	{
		[openGLPixelFormat getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
		totalDisplayMask |= displayMask;
	}
	ret = CVDisplayLinkCreateWithOpenGLDisplayMask(totalDisplayMask, &displayLink);
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChangedScreen:) name:NSWindowDidMoveNotification object:nil];
	/* Set up display link callbacks */
	CVDisplayLinkSetOutputCallback(displayLink, renderCallback, self);
		
	
	fontAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:[NSFont labelFontOfSize:24.0f], NSFontAttributeName,
										    [NSColor colorWithCalibratedRed:1.0f green:0.2f blue:0.2f alpha:0.60f], NSForegroundColorAttributeName,
										    nil];
	
	

}

//--------------------------------------------------------------------------------------------------

- (void)awakeFromNib
{
    OSStatus			    error;
    /* Create QT Visual context */
    NSDictionary	    *attributes = nil;
    attributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithFloat:720.0], kQTVisualContextTargetDimensions_WidthKey, 
													[NSNumber numberWithFloat:480.0], kQTVisualContextTargetDimensions_HeightKey, nil], 
							    kQTVisualContextTargetDimensionsKey, 
							    [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithFloat:720.0], kCVPixelBufferWidthKey, 
													[NSNumber numberWithFloat:480.0], kCVPixelBufferHeightKey, nil], 
							    kQTVisualContextPixelBufferAttributesKey,
							    nil];
    error = QTOpenGLTextureContextCreate(NULL, 
	(CGLContextObj)[[self openGLContext] CGLContextObj],
	(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj],
	(CFDictionaryRef)attributes, &qtVisualContext);

}

//--------------------------------------------------------------------------------------------------

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [qtMovie release];
    [effectFilter release];
    if(qtVisualContext)
		QTVisualContextRelease(qtVisualContext);
	[ciContext release];
    [super dealloc];
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

- (void)update
{
    [lock lock];
    [super update];
    [lock unlock];
}

//--------------------------------------------------------------------------------------------------

- (void)reshape		// scrolled, moved or resized
{
    needsReshape = YES;	// reset the viewport etc. on the next draw
    
    // if we are not playing, force an immediate draw otherwise it will update with the next frame 
    // coming through. This makes the resize performance better as it reduces the number of redraws
    // espcially on the main thread
    if(!CVDisplayLinkIsRunning(displayLink))	
    {
		[self display];
    }
}

//--------------------------------------------------------------------------------------------------

- (void)drawRect:(NSRect)theRect
{
    [lock lock];    
    NSRect		frame = [self frame];
    NSRect		bounds = [self bounds];
    
	[[self openGLContext] makeCurrentContext];
    if(needsReshape)
    {
		GLfloat 	minX, minY, maxX, maxY;

		minX = NSMinX(bounds);
		minY = NSMinY(bounds);
		maxX = NSMaxX(bounds);
		maxY = NSMaxY(bounds);

		[self update]; 

		if(NSIsEmptyRect([self visibleRect])) 
		{
			glViewport(0, 0, 1, 1);
		} else {
			glViewport(0, 0,  frame.size.width ,frame.size.height);
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(minX, maxX, minY, maxY, -1.0, 1.0);

		needsReshape = NO;
    }
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT);
        
    if(!currentFrame)
		[self updateCurrentFrame];
    [self renderCurrentFrame];      
    glFlush();
    [lock unlock];
}

//--------------------------------------------------------------------------------------------------

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    return YES;
}

//--------------------------------------------------------------------------------------------------

- (void)mouseDown:(NSEvent *)theEvent
{
    BOOL	keepOn = YES;
    NSPoint	mouseLoc;
    
    while (keepOn) 
    {
        theEvent = [[self window] nextEventMatchingMask: NSLeftMouseUpMask | NSLeftMouseDraggedMask];
        mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	[self setFilterCenterFromMouseLocation:mouseLoc];
        if ([theEvent type] == NSLeftMouseUp)
			keepOn = NO;
    };
    return;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

- (void)setQTMovie:(QTMovie*)inMovie
{
    [inMovie retain];
    [qtMovie release];
    qtMovie = inMovie;
    if(qtMovie)
    {
		OSStatus error;
		error = SetMovieVisualContext([qtMovie quickTimeMovie],qtVisualContext);
		SetMoviePlayHints([qtMovie quickTimeMovie],hintsHighQuality, hintsHighQuality);	
		[qtMovie gotoBeginning];
		MoviesTask([qtMovie quickTimeMovie], 0);	//QTKit is not doing this automatically
		movieDuration = [[[qtMovie movieAttributes] objectForKey:QTMovieDurationAttribute] QTTimeValue];
		[self setNeedsDisplay:YES];
    }
    
}

//--------------------------------------------------------------------------------------------------

- (QTTime)currentTime
{
    return [qtMovie currentTime];
}

//--------------------------------------------------------------------------------------------------

- (QTTime)movieDuration
{
    return movieDuration;
}

//--------------------------------------------------------------------------------------------------

- (void)setTime:(QTTime)inTime
{
    [qtMovie setCurrentTime:inTime];
    if(CVDisplayLinkIsRunning(displayLink))
		[self togglePlay:nil];
    [self updateCurrentFrame];
    [self display];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)setMovieTime:(id)sender
{
    [self setTime:QTTimeFromString([sender stringValue])];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)nextFrame:(id)sender
{
    if(CVDisplayLinkIsRunning(displayLink))
		[self togglePlay:nil];
    [qtMovie stepForward];
    [self updateCurrentFrame];
    [self display];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)prevFrame:(id)sender
{
    if(CVDisplayLinkIsRunning(displayLink))
	[self togglePlay:nil];
    [qtMovie stepBackward];
    [self updateCurrentFrame];
    [self display];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)scrub:(id)sender
{
    if(CVDisplayLinkIsRunning(displayLink))
	[self togglePlay:nil];

    // Get movie time, duration
    QTTime currentTime;
    NSTimeInterval sliderTime = [sender floatValue];
    TimeValue tv;
        
    currentTime.timeValue = movieDuration.timeValue * sliderTime;
    currentTime.timeScale = movieDuration.timeScale;
    currentTime.flags = 0;
        
    [qtMovie setCurrentTime:currentTime];
    [self updateCurrentFrame];
    [self display];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)togglePlay:(id)sender
{
    if(CVDisplayLinkIsRunning(displayLink))
    {
		CVDisplayLinkStop(displayLink);
		[qtMovie stop];
    } else {
		[qtMovie play];
		CVDisplayLinkStart(displayLink);
    }
}

//--------------------------------------------------------------------------------------------------

- (IBAction)setFilterParameter:(id)sender
{
    [lock lock];
    switch([sender tag])
    {
	case 0:
	    [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputAngle"];
	    break;

	case 1:
	    [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputWidth"];
	    break;

	case 2:
	    [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputSharpness"];
	    break;
	    
	default:
	    break;
	    
    }
    [lock unlock];
    if(!CVDisplayLinkIsRunning(displayLink))
	[self display];
}


//--------------------------------------------------------------------------------------------------

- (void)setFilterCenterFromMouseLocation:(NSPoint)where
{
    CIVector	*centerVector = nil;
    
    [lock lock];
    centerVector = [CIVector vectorWithX:where.x Y:where.y];
    [effectFilter setValue:centerVector forKey:@"inputCenter"];
    [lock unlock];
    if(!CVDisplayLinkIsRunning(displayLink))
		[self display];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)safeFrameToFile:(id)sender
{
    int					outputWidth = 720;
    int					outputHeight = 480;
    NSImage				*returnImage = nil;
    CGImageRef			theCGImage = nil;
    CGDataProviderRef	theDataProvider = nil;
    UInt32				*contextPixels;
    UInt32				contextRowBytes;
    NSSavePanel			*savePanel;
    
    if(CVDisplayLinkIsRunning(displayLink))
	[self togglePlay:nil];

    savePanel = [NSSavePanel savePanel];
    [savePanel setRequiredFileType:@"tiff"];
    if([savePanel runModalForDirectory:nil file:@"MyVideoFrame"] == NSFileHandlingPanelOKButton)
    {
		contextRowBytes = 720 * 4;
		contextPixels = calloc(contextRowBytes * outputHeight, sizeof(char));
		glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);	/* Force 4-byte alignment */
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_SKIP_ROWS, 0);
		glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
		glReadPixels(0, 0, 720, outputHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, contextPixels);    
		glPopClientAttrib();
		returnImage = [[NSImage alloc] initWithSize:NSMakeSize(outputWidth, outputHeight)];
		[returnImage setFlipped:YES];
		theDataProvider = CGDataProviderCreateWithData(NULL, contextPixels, contextRowBytes * outputHeight, NULL);
		theCGImage = CGImageCreate(outputWidth,
						outputHeight,
						8,
						32,
						contextRowBytes,
						CGColorSpaceCreateDeviceRGB(),
						kCGImageAlphaNoneSkipFirst,
						theDataProvider,
						NULL,
						0,
						kCGRenderingIntentDefault);
		CGDataProviderRelease(theDataProvider);
		[returnImage lockFocus];
		CGContextDrawImage((CGContextRef)[[NSGraphicsContext currentContext]graphicsPort], CGRectMake(0, 0, outputWidth, outputHeight), theCGImage);
		[returnImage unlockFocus];
		[[returnImage TIFFRepresentation] writeToFile:[savePanel filename] atomically:NO];
		[returnImage release];
		CGImageRelease(theCGImage);
		free(contextPixels);
    }
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

- (void)renderCurrentFrame
{
    NSRect		frame = [self frame];
    NSRect		bounds = [self bounds];
    
        
    if(currentFrame)
    {
		CGRect	    imageRect;
		CIImage	    *inputImage, *outputImage;
		
		inputImage = [CIImage imageWithCVImageBuffer:currentFrame];

		imageRect = [inputImage extent];
		[effectFilter setValue:inputImage forKey:@"inputImage"];
		[ciContext drawImage:[effectFilter valueForKey:@"outputImage"] 
				atPoint:CGPointMake((int)((frame.size.width - imageRect.size.width) * 0.5), (int)((frame.size.height - imageRect.size.height) * 0.5)) // use integer coordinates to avoid interpolation
				fromRect:imageRect];
		    
    }
    QTVisualContextTask(qtVisualContext);
}



//--------------------------------------------------------------------------------------------------

- (BOOL)getFrameForTime:(const CVTimeStamp *)timeStamp
{
    OSStatus error = noErr;
    CFDataRef movieTimeData;

    // See if a new frame is available

	if(QTVisualContextIsNewImageAvailable(qtVisualContext,timeStamp))
	{	    
		CVOpenGLTextureRelease(currentFrame);
		QTVisualContextCopyImageForTime(qtVisualContext,
			NULL,
			timeStamp,
			&currentFrame);
				
		// In general this shouldn't happen, but just in case...
		if(error != noErr && !currentFrame)
		{
			NSLog(@"QTVisualContextCopyImageForTime: %ld\n",error);
			return NO;
		}
		
		[delegate performSelectorOnMainThread:@selector(movieTimeChanged:) withObject:self waitUntilDone:NO];
		return YES;
	} 
    return NO;
}

//--------------------------------------------------------------------------------------------------

- (void)updateCurrentFrame
{
    [self getFrameForTime:nil];    
}

//--------------------------------------------------------------------------------------------------

-(void)mydisplay:(id)sender
{
    [self display];
}

//--------------------------------------------------------------------------------------------------

- (CVReturn)renderTime:(const CVTimeStamp *)timeStamp
{
    CVReturn rv = kCVReturnError;
    NSAutoreleasePool *pool;
    CFDataRef movieTimeData;
    
    pool = [[NSAutoreleasePool alloc] init];
    if([self getFrameForTime:timeStamp])
    {
		[self drawRect:NSZeroRect];

		rv = kCVReturnSuccess;
    } else {
		rv = kCVReturnError;
    }
    [pool release];
    return rv;
}

//--------------------------------------------------------------------------------------------------

@end
