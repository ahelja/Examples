#import "LiveVideoMixerController.h"

#import <CoreGraphics/CGDirectDisplay.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <QTKit/QTKit.h>

#import "VideoChannel.h"

GLuint				gTexNames[kNumTextures];

#pragma mark -
#pragma mark LiveVideoMixerController
//--------------------------------------------------------------------------------------------------
// LiveVideoMixerController
//--------------------------------------------------------------------------------------------------

@interface LiveVideoMixerController (private)

- (void)_loadTextureFromFile:(NSString*)path 
					intoTextureName:(GLuint)texture 
					pixelFormat:(GLuint)pixelFormat;  //GL_ALPHA or GL_BGRA
					
- (SelectionHandleID)_selectionHandleTest:(NSPoint)inPoint inRect:(NSRect)inRect;
- (NSRect)_calcTargetRect:(NSRect)targetRect videoRect:(NSRect)videoRect;

- (void)_qtHousekeeping;

@end


@implementation LiveVideoMixerController

//--------------------------------------------------------------------------------------------------

- (void)awakeFromNib
{
    CVReturn		error = kCVReturnSuccess;
    CGOpenGLDisplayMask totalDisplayMask = 0;
    int                 virtualScreen;
    long                displayMask;
    NSOpenGLPixelFormat	*openGLPixelFormat = [NSOpenGLView defaultPixelFormat];
    
    
    // build up list of displays from OpenGL's pixel format
    for (virtualScreen = 0; virtualScreen < [openGLPixelFormat  numberOfVirtualScreens]; virtualScreen++)
    {
	    [openGLPixelFormat getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
	    totalDisplayMask |= displayMask;
    }

    // create display link
    mainViewDisplayID = CGMainDisplayID();  // we start with our view on the main display
    error = CVDisplayLinkCreateWithOpenGLDisplayMask(totalDisplayMask, &displayLink);
    if(error)
    {
		NSLog(@"DisplayLink created with error:%d", error);
		displayLink = nil;
		return;
    }
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChangedScreen:) name:NSWindowDidMoveNotification object:nil];
    error = CVDisplayLinkSetOutputCallback(displayLink, myCVDisplayLinkOutputCallback, self);
	
    currentChannelPositioned = -1;	// no channel is currently resized
    
    [mainView setupSharedContext];
    [mainView lock];
    glGenTextures(kNumTextures, gTexNames);
    [self _loadTextureFromFile:[[NSBundle mainBundle] pathForResource:@"Star" ofType:@"tif"] intoTextureName:gTexNames[0] pixelFormat:GL_ALPHA]; 
    [self _loadTextureFromFile:[[NSBundle mainBundle] pathForResource:@"SpeechBubbleMask" ofType:@"tif"] intoTextureName:gTexNames[1] pixelFormat:GL_ALPHA]; 
    [self _loadTextureFromFile:[[NSBundle mainBundle] pathForResource:@"brushmask" ofType:@"tif"] intoTextureName:gTexNames[2] pixelFormat:GL_ALPHA];
    [self _loadTextureFromFile:[[NSBundle mainBundle] pathForResource:@"SpeechBubble" ofType:@"tif"] intoTextureName:gTexNames[kSpeechBubbleTextureID] pixelFormat:GL_BGRA]; 
    [mainView unlock];

}

//--------------------------------------------------------------------------------------------------

- (BOOL)windowShouldClose:(id)sender	//close box quits the app
{
    [NSApp terminate:self];
    return YES;
}

//--------------------------------------------------------------------------------------------------

- (void)dealloc
{
    if(isPlaying)
	[self togglePlayback:nil];
    [mainView lock];
    glDeleteTextures(kNumTextures, gTexNames);
    [mainView unlock];
    CVDisplayLinkRelease(displayLink);
    [super dealloc];
}

//--------------------------------------------------------------------------------------------------
#pragma mark -
//--------------------------------------------------------------------------------------------------

- (IBAction)openChannelFile:(id)sender
{
    int  channelID = [sender tag];
    
    int result;

    NSArray     *fileTypes = [QTMovie movieFileTypes:QTIncludeCommonTypes];
    NSOpenPanel *theOpenPanel = [NSOpenPanel openPanel];


    [theOpenPanel setAllowsMultipleSelection:NO];

    result = [theOpenPanel runModalForDirectory:[NSHomeDirectory() stringByAppendingPathComponent:@"Movies"] file:nil types:fileTypes];

    if (result == NSOKButton) 
    {
        NSArray		    *filesToOpen = [theOpenPanel filenames];
        NSString	    *theFilePath = [filesToOpen objectAtIndex:0];
	VideoChannel	    *theVideoChannel = [VideoChannel createWithFilePath:theFilePath forView:mainView];
	
	switch(channelID)
	{
		case 0:
			//set the background video to be opaque by default
			[theVideoChannel setValue:[NSNumber numberWithFloat:100.0] forKey:@"channelOpacity"];
			[channelController0 addObject:theVideoChannel];
			[positionSwitch0 setEnabled:YES]; 
			break;
		case 1:
			[channelController1 addObject:theVideoChannel];
			[positionSwitch1 setEnabled:YES]; 
			break;
		case 2:
			[channelController2 addObject:theVideoChannel];
			[positionSwitch2 setEnabled:YES]; 
			break;
		
	}
    }
}

//--------------------------------------------------------------------------------------------------

- (IBAction)setChannelPosition:(id)sender
{
    if(sender != positionSwitch0)
	    [positionSwitch0 setState:NSOffState];
    if(sender != positionSwitch1)
	    [positionSwitch1 setState:NSOffState];
    if(sender != positionSwitch2)
	    [positionSwitch2 setState:NSOffState];

    if([sender state] == NSOnState)
    {
	    currentChannelPositioned = [sender tag];
    } else {
	    currentChannelPositioned = -1;
    }
}

//--------------------------------------------------------------------------------------------------

- (IBAction)togglePlayback:(id)sender
{
    if(isPlaying)
    {
	CVDisplayLinkStop(displayLink);
	isPlaying = NO;
	[qtHousekeepingTimer invalidate];
	[qtHousekeepingTimer release];
	qtHousekeepingTimer = nil;
	[[channelController0 content] stopMovie];
	[[channelController1 content] stopMovie];
	[[channelController2 content] stopMovie];
    } else {
	if(CVDisplayLinkStart(displayLink) == kCVReturnSuccess)
	{
	    isPlaying = YES;
	    [[channelController0 content] startMovie];
	    [[channelController1 content] startMovie];
	    [[channelController2 content] startMovie];
	    qtHousekeepingTimer = [[NSTimer scheduledTimerWithTimeInterval:0.015	    // 60fps
			target:self 
			selector:@selector(_qtHousekeeping) 
			userInfo:nil 
			repeats:YES] retain];
	    [[NSRunLoop currentRunLoop] addTimer:qtHousekeepingTimer forMode:NSDefaultRunLoopMode];
	}
    }   
    [playButton setImage:isPlaying ? [NSImage imageNamed:@"FS_Pause_Normal.tif"] : [NSImage imageNamed:@"FS_Play_Normal.tif"]];
}

//--------------------------------------------------------------------------------------------------

- (BOOL)isPlaying
{
    return isPlaying;
}

//--------------------------------------------------------------------------------------------------

- (void)_qtHousekeeping
{
    MoviesTask(NULL, 0);    // MoviesTask has to happen on the main thread as it is not thread safe
}

//--------------------------------------------------------------------------------------------------

- (CVReturn)render:(const CVTimeStamp*)syncTimeStamp
{
    BOOL dirty = NO;
    
    dirty |= [[channelController0 content] renderChannel:syncTimeStamp];
    dirty |= [[channelController1 content] renderChannel:syncTimeStamp];
    dirty |= [[channelController2 content] renderChannel:syncTimeStamp];

    if (dirty) 
	[mainView setDirty:YES];

    [mainView flushOutput];
    return kCVReturnSuccess;
}

//--------------------------------------------------------------------------------------------------

- (NSRect)mainVideoRect
{
    NSRect		destRect = [mainView bounds];
    NSRect		mainVideoRect = NSZeroRect;

    mainVideoRect.size.height = destRect.size.height - 105.0;
    mainVideoRect.size.width = destRect.size.width;
    
    if((mainVideoRect.size.width / 4.0) > (mainVideoRect.size.height / 3.0))
    {
	mainVideoRect.size.width = (mainVideoRect.size.height / 3.0) * 4.0;
    } else {
	mainVideoRect.size.height = (mainVideoRect.size.width / 4.0) * 3.0;
    }
    mainVideoRect.origin.y = 0.0;
    mainVideoRect.origin.x = (destRect.size.width - mainVideoRect.size.width) * 0.5;
    return mainVideoRect;
}

//--------------------------------------------------------------------------------------------------

- (NSRect)_calcTargetRect:(NSRect)targetRect videoRect:(NSRect)videoRect
{
    targetRect.origin.x = videoRect.origin.x + (targetRect.origin.x * (videoRect.size.width / kVideoWidth));
    targetRect.origin.y = videoRect.origin.y + (targetRect.origin.y * (videoRect.size.height / kVideoHeight));
    targetRect.size.width = targetRect.size.width * (videoRect.size.width / kVideoWidth);
    targetRect.size.height = targetRect.size.height * (videoRect.size.height / kVideoHeight);

    return targetRect;

}

//--------------------------------------------------------------------------------------------------

- (void)drawContent
{
    NSRect			mainVideoRect = [self mainVideoRect];
    NSRect			destRect = NSZeroRect;
    NSRect			thumbnailVideoRect = NSZeroRect;
    VideoChannel		*positionedChannel = nil;
	
    
    thumbnailVideoRect.size = NSMakeSize(120.0, 90.0);
    thumbnailVideoRect.origin.x = ([mainView bounds].size.width - (3 * 120.0 + 20.0) ) * 0.5;
    thumbnailVideoRect.origin.y = [mainView bounds].size.height - thumbnailVideoRect.size.height;
	
    // clear content
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if([channelController0 content])
    {
	destRect = [self _calcTargetRect:[[channelController0 content] targetRect] videoRect:mainVideoRect];
	[[channelController0 content] compositeChannelThumbnailInRect:thumbnailVideoRect];
	[[channelController0 content] compositeChannelInRect:destRect];
    }
    thumbnailVideoRect.origin.x += 130.0;
    if([channelController1 content])
    {
	destRect = [self _calcTargetRect:[[channelController1 content] targetRect] videoRect:mainVideoRect];
	[[channelController1 content] compositeChannelThumbnailInRect:thumbnailVideoRect];
	[[channelController1 content] compositeChannelInRect:destRect];
    }
    thumbnailVideoRect.origin.x += 130.0;
    if([channelController2 content])
    {
	destRect = [self _calcTargetRect:[[channelController2 content] targetRect] videoRect:mainVideoRect];
	[[channelController2 content] compositeChannelThumbnailInRect:thumbnailVideoRect];
	[[channelController2 content] compositeChannelInRect:destRect];
    }
    switch(currentChannelPositioned)
    {
	case 0:
		positionedChannel = [channelController0 content];
		break;
	case 1:
		positionedChannel = [channelController1 content];
		break;
	case 2:
		positionedChannel = [channelController2 content];
		break;
	default:
		positionedChannel = nil;
		break;
    }
    if(positionedChannel)
    {
	destRect = [self _calcTargetRect:[positionedChannel targetRect] videoRect:mainVideoRect];
	[positionedChannel drawOutline:destRect];
    }
    glFlush();
}

//--------------------------------------------------------------------------------------------------

- (void)_loadTextureFromFile:(NSString*)path intoTextureName:(GLuint)texture pixelFormat:(GLuint)pixelFormat //GL_ALPHA or GL_BGRA
{

    NSBitmapImageRep	*bitmap = [[NSBitmapImageRep alloc] initWithData: [NSData dataWithContentsOfFile: path]];

    if(bitmap)
    {
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_CACHED_APPLE);
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
	if(pixelFormat == GL_ALPHA)
	{
		glBindTexture(kTextureTarget, texture);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, [bitmap bytesPerRow] );
		glTexImage2D(kTextureTarget, 0, pixelFormat, [bitmap pixelsWide], [bitmap pixelsHigh], 0, pixelFormat, GL_UNSIGNED_BYTE, [bitmap bitmapData]);
	
	} else {
		glBindTexture(kTextureTarget, texture);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, [bitmap bytesPerRow] / 4);
		glTexImage2D(kTextureTarget, 0, GL_RGBA, [bitmap pixelsWide], [bitmap pixelsHigh], 0, pixelFormat, GL_UNSIGNED_INT_8_8_8_8_REV, [bitmap bitmapData]);
	}
	glTexParameteri(kTextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(kTextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(kTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(kTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

//--------------------------------------------------------------------------------------------------

- (SelectionHandleID)_selectionHandleTest:(NSPoint)inPoint inRect:(NSRect)inRect
{
	NSRect					selectionHandleRect = NSMakeRect(0.0, 0.0, kSelectionHandleSize, kSelectionHandleSize);
	
	selectionHandleRect.origin = inRect.origin;
	if(NSPointInRect(inPoint, selectionHandleRect))
		return kTopLeftCorner;
	selectionHandleRect.origin.x += (inRect.size.width - kSelectionHandleSize);
	if(NSPointInRect(inPoint, selectionHandleRect))
		return kTopRightCorner;
	selectionHandleRect.origin.y += (inRect.size.height - kSelectionHandleSize);
	if(NSPointInRect(inPoint, selectionHandleRect))
		return kBottomRightCorner;
	selectionHandleRect.origin.x = inRect.origin.x;
	if(NSPointInRect(inPoint, selectionHandleRect))
		return kBottomLeftCorner;
	return kNoSelectionHandle;
}

//--------------------------------------------------------------------------------------------------

- (void)mouseDown:(NSEvent *)theEvent
{
	NSRect			videoRect = [self mainVideoRect];
	NSPoint			where = [mainView convertPoint:[theEvent locationInWindow] fromView:nil];
	NSRect			channelTargetRect, destRect;
	VideoChannel		*targetChannel = nil;
	SelectionHandleID	theSelectionHandle;
	
	if(currentChannelPositioned != -1)
	{
		switch(currentChannelPositioned)
		{
			case 0:
				targetChannel = [channelController0 content];
				break;
			case 1:
				targetChannel = [channelController1 content];
				break;
			case 2:
				targetChannel = [channelController2 content];
				break;
				
			default:
				break;
		}
		channelTargetRect = [targetChannel targetRect];
		where.x -= videoRect.origin.x;
		where.y -= videoRect.origin.y;
		destRect.origin.x = channelTargetRect.origin.x * (videoRect.size.width / kVideoWidth);
		destRect.origin.y = channelTargetRect.origin.y * (videoRect.size.height / kVideoHeight);
		destRect.size.width = channelTargetRect.size.width * (videoRect.size.width / kVideoWidth);
		destRect.size.height = channelTargetRect.size.height * (videoRect.size.height / kVideoHeight);
		theSelectionHandle = [self _selectionHandleTest:where inRect:destRect];

		if(NSPointInRect(where, destRect))
		{
			NSPoint				currentPoint, startPoint, delta;
			BOOL				dragActive = YES;
			NSAutoreleasePool	*myPool = nil;
			NSEvent* 			event = NULL;
			NSWindow			*targetWindow = [mainView window];
		
			startPoint = where;

			myPool = [[NSAutoreleasePool alloc] init];
			while (dragActive)
			{
				NSRect				outRect = channelTargetRect;
				
				
				event = [targetWindow nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)
														untilDate:[NSDate distantFuture]
														inMode:NSEventTrackingRunLoopMode
														dequeue:YES];
			
				if(!event)
					continue;
				currentPoint = [mainView convertPoint:[event locationInWindow] fromView:nil];
				currentPoint.x -= videoRect.origin.x;
				currentPoint.y -= videoRect.origin.y;
				switch ([event type])
				{
					case NSLeftMouseDragged:
						delta.x = currentPoint.x - startPoint.x;
						delta.y = currentPoint.y - startPoint.y;
						switch(theSelectionHandle)
						{
							case kTopLeftCorner:
								outRect.origin.x += delta.x;
								outRect.origin.y += delta.y;
								outRect.size.width -= delta.x;
								outRect.size.height -= delta.y;
								break;
								
							case kTopRightCorner:
								outRect.origin.y += delta.y;
								outRect.size.width += delta.x;
								outRect.size.height -= delta.y;
								break;
							
							case kBottomLeftCorner:
								outRect.origin.x += delta.x;
								outRect.size.width -= delta.x;
								outRect.size.height += delta.y;
								break;
								
							case kBottomRightCorner:
								outRect.size.width += delta.x;
								outRect.size.height += delta.y;
								break;
								
							default:
								outRect.origin.x += delta.x;
								outRect.origin.y += delta.y;
								break;
						}
						[targetChannel setTargetRect:outRect];
						break;
					
				
					case NSLeftMouseUp:
						dragActive = NO;
						break;
						
					default:
						break;
				}
			}
			[myPool release];
			myPool = nil;

		}
	}
}

//--------------------------------------------------------------------------------------------------

- (void)windowChangedScreen:(NSNotification*)inNotification
{
	NSWindow *window = [mainView window]; 
	CGDirectDisplayID displayID = (CGDirectDisplayID)[[[[window screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];
	
	if((displayID != NULL) && (mainViewDisplayID != displayID))
	{
	    CVDisplayLinkSetCurrentCGDisplay(displayLink, displayID);
	    mainViewDisplayID = displayID;
	}
}

@end

#pragma mark -
#pragma mark CALLBACKS
//--------------------------------------------------------------------------------------------------
// CALLBACKS
//--------------------------------------------------------------------------------------------------

CVReturn myCVDisplayLinkOutputCallback(CVDisplayLinkRef displayLink, 
                                                const CVTimeStamp *inNow, 
                                                const CVTimeStamp *inOutputTime, 
                                                CVOptionFlags flagsIn, 
                                                CVOptionFlags *flagsOut, 
                                                void *displayLinkContext)
{
    CVReturn    error = [(LiveVideoMixerController*)displayLinkContext render:inOutputTime];
    return error;
}


//--------------------------------------------------------------------------------------------------
