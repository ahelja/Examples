#import "VideoMixView.h"
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import "LiveVideoMixerController.h"

@implementation VideoMixView


//--------------------------------------------------------------------------------------------------

- (void)awakeFromNib
{
    pthread_mutexattr_t attr;
    
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);	// the recursive thread allows us to aquire the same lock again from the same thread
								// but still lock against another thread

    pthread_mutex_init(&drawingLock, &attr);
		
}

//--------------------------------------------------------------------------------------------------

- (void)setupSharedContext
{
    sharedContext = [[NSOpenGLContext alloc] initWithFormat:[NSOpenGLView defaultPixelFormat] shareContext:nil];
    
    [self setOpenGLContext:[[[NSOpenGLContext alloc] initWithFormat:[NSOpenGLView defaultPixelFormat] shareContext:sharedContext] autorelease]];
    [[self openGLContext] setView:self];
}

//--------------------------------------------------------------------------------------------------
- (NSOpenGLContext*)sharedContext
{
    return sharedContext;
}

//--------------------------------------------------------------------------------------------------

- (void)dealloc
{
    pthread_mutex_destroy(&drawingLock);
    [super dealloc];
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

- (BOOL)isFlipped
{
    return YES;
}

//--------------------------------------------------------------------------------------------------

- (void)lock
{
    pthread_mutex_lock(&drawingLock);
    if(contextIsInitialized)
		[[self openGLContext] makeCurrentContext];
}

//--------------------------------------------------------------------------------------------------

- (void)unlock
{
    pthread_mutex_unlock(&drawingLock);
}

//--------------------------------------------------------------------------------------------------

- (void)update
{
    // The NSOpenGLView issues OpenGL calls in its update method. Therefore it is important to lock
    // around this call as it would otherwise run in conflict with our rendering thread
    [self lock];
    [super update];
    [self unlock];
}

//--------------------------------------------------------------------------------------------------

- (void)reshape		// scrolled, moved or resized
{
    needsReshape = YES;	// reset the viewport etc. on the next draw
    
    // if we are not playing, force an immediate draw otherwise it will update with the next frame 
    // coming through. This makes the resize performance better as it reduces the number of redraws
    // espcially on the main thread
    if(![controller isPlaying])	
    {
	[self setDirty:YES];
	[self flushOutput];
    }
}

//--------------------------------------------------------------------------------------------------

- (void)clear
{
    [self lock];
    // clear content
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    [self unlock];
}

//--------------------------------------------------------------------------------------------------

- (void)flushOutput
{
    [self lock];
    if(needsReshape)
    {
	NSRect		frame = [self frame];
	NSRect		bounds = [self bounds];
	GLfloat 	minX, minY, maxX, maxY;

	minX = NSMinX(bounds);
	minY = NSMinY(bounds);
	maxX = NSMaxX(bounds);
	maxY = NSMaxY(bounds);

	[[self openGLContext] makeCurrentContext];
	[self update]; 

	// the first time we need to setup the OpenGL environment in this view
	if(!contextIsInitialized)
	{
	    long swapInterval = 1;
	    
	    [[self openGLContext] setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];	// synchronize the glFlush with the VBL
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glEnable( GL_BLEND );				    // enable blending
	    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    // blend func for non premultiplied images
	    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	    glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
	    contextIsInitialized = YES;
	}

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
	glOrtho(minX, maxX, maxY, minY, -1.0, 1.0);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT);
	needsReshape = NO;
    }
    if([self dirty])
	[controller drawContent];   // invoke the controller to draw all the video channels
    [self setDirty:NO];
    [self unlock];
}

//--------------------------------------------------------------------------------------------------

- (void)setDirty:(BOOL)inDirty
{
    dirty = inDirty;
}

//--------------------------------------------------------------------------------------------------

- (BOOL)dirty
{
    return dirty;
}

//--------------------------------------------------------------------------------------------------

- (void)mouseDown:(NSEvent *)theEvent
{
    // mouse handling is done in the controller as it handles the video channels 
    [controller mouseDown:(NSEvent *)theEvent];	
}


@end
