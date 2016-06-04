#import "VideoChannel.h"
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import "LiveVideoMixerController.h"

@implementation VideoChannel

//--------------------------------------------------------------------------------------------------

+ (id)createWithFilePath:(NSString*)theFilePath forView:(VideoMixView*)inView
{
    return [[[VideoChannel alloc] initWithFilePath:theFilePath forView:inView] autorelease];
}

//--------------------------------------------------------------------------------------------------

- (id)initWithFilePath:(NSString*)theFilePath forView:(VideoMixView*)inView;
{
    self = [super init];
    
    OSStatus			theError = noErr;
    Boolean			active = TRUE;
    UInt32			trackCount = 0;
    OSType			theTrackType;
    Track			theTrack = nil;
    Media			theMedia = nil;

    targetRect = NSMakeRect(0.0, 0.0, kVideoWidth, kVideoHeight);
    QTNewMoviePropertyElement newMovieProperties[] = {  {kQTPropertyClass_DataLocation, kQTDataLocationPropertyID_CFStringNativePath, sizeof(theFilePath), &theFilePath, 0},
							{kQTPropertyClass_NewMovieProperty, kQTNewMoviePropertyID_Active, sizeof(active), &active, 0},
							{kQTPropertyClass_Context, kQTContextPropertyID_VisualContext, sizeof(visualContext), &visualContext, 0},
							};
    
    theError = QTOpenGLTextureContextCreate(nil, [[inView sharedContext] CGLContextObj],
						[[NSOpenGLView defaultPixelFormat] CGLPixelFormatObj],
						nil,
						&visualContext);
    if(visualContext == NULL) 
    {
	NSLog(@"QTVisualContext creation failed with error:%d", theError);
	return nil;
    }
    theError = NewMovieFromProperties(sizeof(newMovieProperties) / sizeof(newMovieProperties[0]), newMovieProperties, 0, nil, &channelMovie);
    
    if(theError)
    {
	NSLog(@"NewMovieFromProperties failed with %d", theError);
	return nil;
    }
	
    // setup the movie
    GoToBeginningOfMovie(channelMovie);
    SetMovieRate(channelMovie, 1 << 16);
    SetTimeBaseFlags(GetMovieTimeBase(channelMovie), loopTimeBase);
    trackCount = GetMovieTrackCount(channelMovie);
    while(trackCount > 0)
    {
	theTrack = GetMovieIndTrack(channelMovie, trackCount);
	if(theTrack != nil)
	{
	    theMedia = GetTrackMedia(theTrack);
	    if(theMedia != nil)
	    {			    
		GetMediaHandlerDescription(theMedia, &theTrackType, 0, 0);
		if(theTrackType != VideoMediaType)
		{
		    SetTrackEnabled(theTrack, false);		// disable all non video tracks
		}
	    }
	}
	trackCount--;
    }

    return self;
}

//--------------------------------------------------------------------------------------------------

- (void)dealloc
{
    DisposeMovie(channelMovie);
    [super dealloc];
}

//--------------------------------------------------------------------------------------------------

- (void)startMovie
{
    GoToBeginningOfMovie(channelMovie);
    StartMovie(channelMovie);
}

//--------------------------------------------------------------------------------------------------

- (void)stopMovie
{
    StopMovie(channelMovie);
}

//--------------------------------------------------------------------------------------------------

- (void)_setCurrentTextureRef:(CVOpenGLTextureRef)inTextureRef
{
    CVOpenGLTextureRelease(currentTexture);
    currentTexture = inTextureRef;
    //get the clean aperture texture coordinates
    CVOpenGLTextureGetCleanTexCoords(currentTexture, lowerLeft, lowerRight, upperRight, upperLeft);
}

//--------------------------------------------------------------------------------------------------

- (BOOL)renderChannel:(const CVTimeStamp*)syncTimeStamp
{
    CVOpenGLTextureRef		newTextureRef = nil;
    
    QTVisualContextTask(visualContext);
    if(QTVisualContextIsNewImageAvailable(visualContext, syncTimeStamp))
    {
	QTVisualContextCopyImageForTime(visualContext, nil, syncTimeStamp, &newTextureRef);
	[self _setCurrentTextureRef:newTextureRef];
	return YES; // we got a frame from QT
    } else {
	//NSLog(@"No Frame ready");
    }
    return NO;	// no frame ready
}

//--------------------------------------------------------------------------------------------------

- (float)opacity
{
    return [channelOpacity floatValue] / 100.0;	// the slider ranges from zero to 100 while alpha is normalize (0.0 to 1.0)
}

//--------------------------------------------------------------------------------------------------

- (void)setTargetRect:(NSRect)inRect
{
    targetRect = inRect;
}

//--------------------------------------------------------------------------------------------------

- (NSRect)targetRect
{
    return targetRect;
}

//--------------------------------------------------------------------------------------------------

- (void)drawOutline:(NSRect)destRect

{
    glLoadIdentity();
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_RECTANGLE_EXT);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_RECTANGLE_EXT);
    glDisable(GL_TEXTURE_2D);
    glPushAttrib(GL_POLYGON_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glMatrixMode(GL_TEXTURE);
    glLineWidth(1);
    glColor4f(1, 0, 0, 1);
    glBegin(GL_QUADS);            
	    glVertex2f(destRect.origin.x, destRect.origin.y);
	    glVertex2f(destRect.origin.x + destRect.size.width, destRect.origin.y);
	    glVertex2f(destRect.origin.x + destRect.size.width, destRect.origin.y + destRect.size.height);
	    glVertex2f(destRect.origin.x, destRect.origin.y + destRect.size.height);
    glEnd();
    glPopAttrib();
}

//--------------------------------------------------------------------------------------------------

- (void)compositeChannelInRect:(NSRect)destRect;

{
    float				opacity = [self opacity];
    
    if(!channelShape)
    {
	glEnable(CVOpenGLTextureGetTarget(currentTexture));
	glBindTexture(CVOpenGLTextureGetTarget(currentTexture), CVOpenGLTextureGetName(currentTexture));
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glColor4f(1, 1, 1, opacity);
	glBegin(GL_QUADS);            
		glTexCoord2f(upperLeft[0], upperLeft[1]);
		glVertex2f  (destRect.origin.x, destRect.origin.y);
		glTexCoord2f(upperRight[0], upperRight[1]);
		glVertex2f  (destRect.origin.x + destRect.size.width, destRect.origin.y);
		glTexCoord2f(lowerRight[0], lowerRight[1]);
		glVertex2f  (destRect.origin.x + destRect.size.width, destRect.origin.y + destRect.size.height);
		glTexCoord2f(lowerLeft[0], lowerLeft[1]);
		glVertex2f  (destRect.origin.x, destRect.origin.y + destRect.size.height);
	glEnd();
    } else {
	// setup the matte texture as a mask in texture unit 1
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(kTextureTarget);
	glBindTexture(kTextureTarget, gTexNames[[channelShape intValue]]);	
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// setup the image texture in texture unit 0
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(CVOpenGLTextureGetTarget(currentTexture));
	glBindTexture(CVOpenGLTextureGetTarget(currentTexture), CVOpenGLTextureGetName(currentTexture));
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// render both textures together
	glColor4f(1, 1, 1, opacity);
	glBegin(GL_QUADS);            
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 0);
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, upperLeft[0], upperLeft[1]);
		glVertex2f  ( destRect.origin.x, destRect.origin.y);
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, kVideoWidth, 0);
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, upperRight[0], upperRight[1]);
		glVertex2f  ( destRect.origin.x + destRect.size.width, destRect.origin.y);
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, kVideoWidth, kVideoHeight);
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, lowerRight[0], lowerRight[1]);
		glVertex2f  ( destRect.origin.x + destRect.size.width, destRect.origin.y + destRect.size.height);
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, kVideoHeight);
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, lowerLeft[0], lowerLeft[1]);
		glVertex2f  ( destRect.origin.x, destRect.origin.y + destRect.size.height);
	glEnd();	
	glActiveTextureARB(GL_TEXTURE1_ARB);    // make sure the texture unit 1 is now disabled 
	glDisable(GL_TEXTURE_RECTANGLE_EXT);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	if([channelShape intValue] == kSpeechBubble)
	{
	    glEnable(kTextureTarget);
	    glBindTexture(kTextureTarget, gTexNames[kSpeechBubbleTextureID]);
	    glMatrixMode(GL_TEXTURE);
	    glLoadIdentity();
	    glColor4f(1, 1, 1, 1);
	    glBegin(GL_QUADS);            
		    glTexCoord2f( 0, 0);
		    glVertex2f  ( destRect.origin.x, destRect.origin.y);
		    glTexCoord2f( kVideoWidth, 0);
		    glVertex2f  ( destRect.origin.x + destRect.size.width, destRect.origin.y);
		    glTexCoord2f( kVideoWidth, kVideoHeight);
		    glVertex2f  ( destRect.origin.x + destRect.size.width, destRect.origin.y + destRect.size.height);
		    glTexCoord2f( 0, kVideoHeight);
		    glVertex2f  ( destRect.origin.x, destRect.origin.y + destRect.size.height);
	    glEnd();
	}
    }
	
}

//--------------------------------------------------------------------------------------------------

- (void)compositeChannelThumbnailInRect:(NSRect)destRect
{
    glEnable(CVOpenGLTextureGetTarget(currentTexture));
    glBindTexture(CVOpenGLTextureGetTarget(currentTexture), CVOpenGLTextureGetName(currentTexture));
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);            
	    glTexCoord2f(upperLeft[0], upperLeft[1]);
	    glVertex2f  (destRect.origin.x, destRect.origin.y);
	    glTexCoord2f(upperRight[0], upperRight[1]);
	    glVertex2f  (destRect.origin.x + destRect.size.width, destRect.origin.y);
	    glTexCoord2f(lowerRight[0], lowerRight[1]);
	    glVertex2f  (destRect.origin.x + destRect.size.width, destRect.origin.y + destRect.size.height);
	    glTexCoord2f(lowerLeft[0], lowerLeft[1]);
	    glVertex2f  (destRect.origin.x, destRect.origin.y + destRect.size.height);
    glEnd();
}

//--------------------------------------------------------------------------------------------------

@end
