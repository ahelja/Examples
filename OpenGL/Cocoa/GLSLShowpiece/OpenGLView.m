/*

File: OpenGLView.m

Abstract: Main rendering class
			 
Author: James A. McCombe

© Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#import "OpenGLView.h"

@implementation OpenGLView

- (id) initWithFrame: (NSRect) theFrame
{
	NSOpenGLPixelFormatAttribute attribs [] = {
		NSOpenGLPFARendererID, 0x00020400,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 32,

		0
   };
   NSOpenGLPixelFormat *fmt;
	
   /* Create a GL Context to use - i.e. init the superclass */
   fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];
	[super initWithFrame: theFrame pixelFormat: fmt];
   [[super openGLContext] makeCurrentContext];
   [fmt release];
	
	/* Set the project type */
   glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.3, 0.3, 0.0, 0.6, 1.0, 8.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -2.0);

#ifdef USE_BLENDING
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

	/* Turn on depth test */
	glEnable(GL_DEPTH_TEST);
	
	[self setFrameSize: theFrame.size];

	/* Create an update timer */
	timer = [NSTimer scheduledTimerWithTimeInterval: (1.0 / 40) target: self
                    selector: @selector(display) userInfo: nil
                    repeats: YES];
	[timer retain];

	/* Create the podium */
	podium = [[Podium alloc] init];
	
	return self;
}

- (void) dealloc
{
	/* Release the update timer */
	if (timer) {
		[timer invalidate];
		[timer release];
	}
	
	/* Release the podium */
	[podium release];
	
	/* Dealloc the superclass */
	[super dealloc];
}

- (void) drawRect: (NSRect) theRect
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClearColor(0.07, 0.21, 0.31, 1.0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	glPushMatrix();
	
	gluLookAt(0.0, 0.05, 0.15,   /* Camera Position */
				 0.0, 0.0, 0.0,     /* Reference Point */
				 0.0, 1.0, 0.0);    /* Up vector       */

	/* Constant rotation of the subject */
	glRotatef(angle, 0.0, 1.0, 0.0);
	angle += 0.5;
	if (angle >= 360.0)
		angle -= 360.0;

	/* Draw the granite podium */
	glColor4f(1.0, 1.0, 1.0, 1.0);
	
	[podium renderFrame];
	
	/* Draw the exhibit */
		if (target_exhibit) {
			if (exhibit_height > 0.0)
				exhibit_height -= 0.02;
			else {
				current_exhibit = target_exhibit;
				target_exhibit  = NULL;
			}
		} else {
			if (exhibit_height < 0.5)
				exhibit_height += 0.02;
		}

	glPushMatrix();
	glTranslatef(0.0, exhibit_height, 0.0);
	glScalef(0.4, 0.4, 0.4);
	if (current_exhibit)
		[current_exhibit renderFrame];
	glPopMatrix();
	
	glPopMatrix();

	[[self openGLContext] flushBuffer];
}

- (void) setFrameSize: (NSSize) newSize
{
	[super setFrameSize: newSize];

	glViewport(0, 0, newSize.width, newSize.height);
}

- (void) setExhibit: (Exhibit *) new_exhibit
{
	target_exhibit  = new_exhibit;
	//exhibit_height  = 0.0;
}

@end
