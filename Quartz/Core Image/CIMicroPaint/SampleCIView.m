/* SampleCIView.m - simple OpenGL based CoreImage view */

#import "SampleCIView.h"

#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

@implementation SampleCIView

+ (NSOpenGLPixelFormat *)defaultPixelFormat
{
    static NSOpenGLPixelFormat *pf;

    if (pf == nil)
    {
	/* Making sure the context's pixel format doesn't have a recovery
	 * renderer is important - otherwise CoreImage may not be able to
	 * create deeper context's that share textures with this one. */

	static const NSOpenGLPixelFormatAttribute attr[] = {
	    NSOpenGLPFAAccelerated,
	    NSOpenGLPFANoRecovery,
	    NSOpenGLPFAColorSize, 32,
	    0
	};

	pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:(void *)&attr];
    }

    return pf;
}

- (void)dealloc
{
    [_image release];
    [_context release];

    [super dealloc];
}

- (CIImage *)image
{
    return [[_image retain] autorelease];
}

- (void)setImage:(CIImage *)image dirtyRect:(CGRect)r
{
    if (_image != image)
    {
	[_image release];
	_image = [image retain];

	if (CGRectIsInfinite (r))
	    [self setNeedsDisplay:YES];
	else
	    [self setNeedsDisplayInRect:*(NSRect *)&r];
    }
}

- (void)setImage:(CIImage *)image
{
    [self setImage:image dirtyRect:CGRectInfinite];
}

- (void)prepareOpenGL
{
    long parm = 1;

    /* Enable beam-synced updates. */

    [[self openGLContext] setValues:&parm forParameter:NSOpenGLCPSwapInterval];

    /* Make sure that everything we don't need is disabled. Some of these
     * are enabled by default and can slow down rendering. */

    glDisable (GL_ALPHA_TEST);
    glDisable (GL_DEPTH_TEST);
    glDisable (GL_SCISSOR_TEST);
    glDisable (GL_BLEND);
    glDisable (GL_DITHER);
    glDisable (GL_CULL_FACE);
    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask (GL_FALSE);
    glStencilMask (0);
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
    glHint (GL_TRANSFORM_HINT_APPLE, GL_FASTEST);
}

- (void)viewBoundsDidChange:(NSRect)bounds
{
    /* For subclasses. */
}

- (void)updateMatrices
{
    NSRect r = [self bounds];

    if (!NSEqualRects (r, _lastBounds))
    {
	[[self openGLContext] update];

	/* Install an orthographic projection matrix (no perspective)
	 * with the origin in the bottom left and one unit equal to one
	 * device pixel. */

	glViewport (0, 0, r.size.width, r.size.height);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, r.size.width, 0, r.size.height, -1, 1);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	_lastBounds = r;

	[self viewBoundsDidChange:r];
    }
}

- (void)drawRect:(NSRect)r
{
    CGRect ir, rr;
    CGImageRef cgImage;

    [[self openGLContext] makeCurrentContext];

    /* Allocate a CoreImage rendering context using the view's OpenGL
     * context as its destination if none already exists. */

    if (_context == nil)
    {
	NSOpenGLPixelFormat *pf;

	pf = [self pixelFormat];
	if (pf == nil)
	    pf = [[self class] defaultPixelFormat];

	_context = [[CIContext contextWithCGLContext: CGLGetCurrentContext()
		     pixelFormat: [pf CGLPixelFormatObj] options: nil] retain];
    }

    ir = CGRectIntegral (*(CGRect *)&r);

    if ([NSGraphicsContext currentContextDrawingToScreen])
    {
	[self updateMatrices];

	/* Clear the specified subrect of the OpenGL surface then
	 * render the image into the view. Use the GL scissor test to
	 * clip to * the subrect. Ask CoreImage to generate an extra
	 * pixel in case * it has to interpolate (allow for hardware
	 * inaccuracies) */

	rr = CGRectIntersection (CGRectInset (ir, -1.0f, -1.0f),
				 *(CGRect *)&_lastBounds);

	glScissor (ir.origin.x, ir.origin.y, ir.size.width, ir.size.height);
	glEnable (GL_SCISSOR_TEST);

	glClear (GL_COLOR_BUFFER_BIT);

	if ([self respondsToSelector:@selector (drawRect:inCIContext:)])
	{
	    [self drawRect:*(NSRect *)&rr inCIContext:_context];
	}
	else if (_image != nil)
	{
	    [_context drawImage:_image atPoint:rr.origin fromRect:rr];
	}

	glDisable (GL_SCISSOR_TEST);

	/* Flush the OpenGL command stream. If the view is double
	 * buffered this should be replaced by [[self openGLContext]
	 * flushBuffer]. */

	glFlush ();
    }
    else
    {
	/* Printing the view contents. Render using CG, not OpenGL. */

	if ([self respondsToSelector:@selector (drawRect:inCIContext:)])
	{
	    [self drawRect:*(NSRect *)&ir inCIContext:_context];
	}
	else if (_image != nil)
	{
	    cgImage = [_context createCGImage:_image fromRect:ir];

	    if (cgImage != NULL)
	    {
		CGContextDrawImage ([[NSGraphicsContext currentContext]
				     graphicsPort], ir, cgImage);
		CGImageRelease (cgImage);
	    }
	}
    }
}

@end
