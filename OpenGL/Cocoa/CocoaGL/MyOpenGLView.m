#include <OpenGL/gl.h>

#import "MyOpenGLView.h"

@implementation MyOpenGLView

/*
	Override NSView's initWithFrame: to specify our pixel format:
	
	Note: initWithFrame is called only if a "Custom View" is used in Interface BBuilder 
	and the custom class is a subclass of NSView. For more information on resource loading
	see: developer.apple.com (ADC Home > Documentation > Cocoa > Resource Management > Loading Resources)
*/	

- (id) initWithFrame: (NSRect) frame
{
	GLuint attribs[] = 
	{
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAWindow,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAStencilSize, 8,
		NSOpenGLPFAAccumSize, 0,
		0
	};

	NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: (NSOpenGLPixelFormatAttribute*) attribs]; 
	
	if (!fmt)
		NSLog(@"No OpenGL pixel format");

	return self = [super initWithFrame:frame pixelFormat: [fmt autorelease]];
}

- (void) awakeFromNib
{
	color_index = alphaIndex;
}

/*
	Override the view's drawRect: to draw our GL content.
*/	 

- (void) drawRect: (NSRect) rect
{
	glViewport(0, 0, (GLsizei) rect.size.width, (GLsizei) rect.size.height);

	GLfloat clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	clear_color[color_index] = 1.0f;
	
	glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);

	glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT+GL_STENCIL_BUFFER_BIT);
	
	[[self openGLContext] flushBuffer];
}

/*
	The UI buttons are targetted to call this action method:
*/

- (IBAction) setClearColor: (id) sender
{
	color_index = [sender tag];
	
	[self setNeedsDisplay: YES];
}

@end
