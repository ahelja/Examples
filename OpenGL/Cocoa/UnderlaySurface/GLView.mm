#import "GLView.h"

void DrawCubeMinMax(const Vec& _min, const Vec& _max)
{
	glBegin(GL_QUADS);
	// Front
	glNormal3f(0.0f,0.0f,1.0f);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(_max.x,_min.y,_max.z);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(_max.x,_max.y,_max.z);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(_min.x,_max.y,_max.z);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(_min.x,_min.y,_max.z);

	// Top
	glNormal3f(0.0f,1.0f,0.0f);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(_max.x,_max.y,_max.z);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(_max.x,_max.y,_min.z);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(_min.x,_max.y,_min.z);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(_min.x,_max.y,_max.z);
	
	// Left
	glNormal3f(-1.0f,0.0f,0.0f);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(_min.x,_min.y,_max.z);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(_min.x,_max.y,_max.z);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(_min.x,_max.y,_min.z);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(_min.x,_min.y,_min.z);
	
	// Back
	glNormal3f(0.0f,0.0f,-1.0f);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(_min.x,_min.y,_min.z);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(_min.x,_max.y,_min.z);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(_max.x,_max.y,_min.z);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(_max.x,_min.y,_min.z);
	
	// Right
	glNormal3f(1.0f,0.0f,0.0f);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(_max.x,_min.y,_min.z);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(_max.x,_max.y,_min.z);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(_max.x,_max.y,_max.z);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(_max.x,_min.y,_max.z);
	
	// Bottom
	glNormal3f(0.0f,-1.0f,0.0f);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(_min.x,_min.y,_max.z);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(_min.x,_min.y,_min.z);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(_max.x,_min.y,_min.z);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(_max.x,_min.y,_max.z);
	glEnd();
}

void DrawUnitCube(float alpha)
{
	GLfloat color[4] = { 0.2, 0.2, 0.8, alpha };
	GLfloat shininess = 128 * 0.1;
	GLfloat specular[4] = { 0.6, 0.8, 0.9, alpha };

	glMaterialfv(GL_FRONT, GL_AMBIENT, color);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);				

	DrawCubeMinMax(Vec::New(-1.0f,-1.0f,-1.0f), Vec::New(1.0f,1.0f,1.0f));
}

@implementation GLView

- (id) initWithFrame: (NSRect) frame
{
	GLuint attribs[] = 
	{
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADepthSize, 16,
		0
	};

	NSOpenGLPixelFormat* fmt = [[[NSOpenGLPixelFormat alloc] initWithAttributes: (NSOpenGLPixelFormatAttribute*) attribs] autorelease]; 
	
	if (!fmt)
		NSLog(@"No OpenGL pixel format");

	self = [super initWithFrame:frame pixelFormat:fmt];

	_alpha = 0.5;
	_mode = NO;

	QMatrix::Reset(_cube_xform);
	_cube_xform.scale *= 10.0f;

	[self setMode: YES];

	return self;
}

- (void) viewDidMoveToWindow
{
	[super viewDidMoveToWindow];
	[[self window] setOpaque:NO];
	[[self window] setAlphaValue:.999f];
}

- (BOOL) isOpaque
{
	return YES;
}


- (void) drawBackingQuad
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);

	glColor4f(0.0, 0.1, 0.0, 1.0);
	glVertex3f(-1.0, -1.0, 0.0);
	glVertex3f( 1.0, -1.0, 0.0);

	glColor4f(0.0, 0.5, 0.1, 1.0);
	glVertex3f( 1.0,  1.0, 0.0);
	glVertex3f(-1.0,  1.0, 0.0);

	glEnd();
}
	
- (void) drawGLScene
{
	glShadeModel(GL_SMOOTH);

	NSRect bounds = [self bounds];
	glViewport(0, 0, (GLint) bounds.size.width, (GLint) bounds.size.height);
	
	[self drawBackingQuad];
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,bounds.size.width/bounds.size.height,1.0f,1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_RESCALE_NORMAL);
	
	glTranslatef(0.0f,0.0f,-50.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);

	{
		MatrixFrame cube_frame;
		
		cube_frame <<= _cube_xform;

		glCullFace(GL_FRONT);
		DrawUnitCube(_alpha);

		glCullFace(GL_BACK);
		DrawUnitCube(_alpha);
	}
	
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);


	if([self inLiveResize])
		glFlush();
	else
		[[self openGLContext] flushBuffer];
}

- (void) drawRect: (NSRect) rect
{
    NSRect bounds = [self bounds];
    [[NSColor clearColor] set];
    NSRectFill(bounds);

	[self drawGLScene];
}

- (void) paintWithEvent: (NSEvent*) theEvent
{
	NSPoint location;
	NSBezierPath* path;
	
	location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	
	path = [NSBezierPath bezierPath];
	
	[self lockFocus];

	[[NSColor colorWithCalibratedRed:0.0f green:1.0f blue:0.0f alpha:0.2f] set];
	[path appendBezierPathWithArcWithCenter:location radius:10.0f
			 		startAngle:0.0f
				 endAngle:360.0f
				clockwise:NO];
	[path fill];
	[self unlockFocus];
	
	[[self window] flushWindow];
	
}

- (void) mouseDown: (NSEvent*) theEvent
{
	[self paintWithEvent:theEvent];
}

- (void) mouseDragged: (NSEvent*) theEvent
{
	[self paintWithEvent:theEvent];
}

- (void) mouseUp: (NSEvent*) theEvent
{
}

- (void) updateTextStatus
{
	// limit updates to 1/second:

	static double last_update = 0.0;
	
	if (sys.d_t && sys.now - last_update > 1.0)
	{
		NSSize size = [self bounds].size;
		[fpsText setStringValue: [NSString stringWithFormat: @"FPS: %.1f (%.0f x %0.f)",  1/sys.d_t, size.width, size.height]];
		last_update = sys.now;
	}
}

- (void) timerUpdate
{
	_cube_xform.rot *= Quat::New(y_axis, PI*0.25*sys.d_t);

	[[self openGLContext] makeCurrentContext];
	[self drawGLScene];
	[NSOpenGLContext clearCurrentContext];

	[self updateTextStatus];
}

- (void) setMode: (BOOL) mode
{
	if (mode == _mode)
		return;

	_mode = mode;

	long order = _mode ? -1 : 1;

	[[self openGLContext] setValues:&order forParameter: NSOpenGLCPSurfaceOrder];
}

- (void) setAlpha: (float) alpha
{
	_alpha = alpha;
}

@end
