/*
File:			TransparentView.m

Description: 	This is the implementation file for the TransparentView class, which handles
				the drawing of a semi-tranparent view which shows the login window behind it.
				Adapted from RoundTransparentWindow sample
*/

#import "TransparentView.h"

#define kViewOpacity 0.9

@implementation TransparentView

// This routine is called at app launch time when this class is unpacked from the nib.
// We get set up here.

- (void)awakeFromNib
{
    // Load the images we'll use from the bundle's Resources directory
	// The default mask image is the same as the background: a copy of SolidAquaDarkBlue
	// from Desktop Pictures
	mImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle bundleForClass:[self class]]
		pathForImageResource:@"mask"]];
	[mImage setScalesWhenResized:YES];
	[mImage setSize:[self bounds].size];

	//tell ourselves that we need displaying
    [self setNeedsDisplay:YES];
}

// When it's time to draw, this routine is called.

- (void)drawRect:(NSRect)rect
{
    //erase whatever graphics were there before with clear
	[[NSColor clearColor] set];
	NSRectFill([self bounds]);

	[mImage dissolveToPoint:NSZeroPoint fraction:kViewOpacity];

    //the next line resets the CoreGraphics window shadow (calculated around our custom window shape content)
    //so it's recalculated for the new shape, etc.  The API to do this was introduced in 10.2.
	[[self window] invalidateShadow];
}

@end

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation,
 modification or redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject to these
 terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
 this original Apple software (the "Apple Software"), to use, reproduce, modify and
 redistribute the Apple Software, with or without modifications, in source and/or binary
 forms; provided that if you redistribute the Apple Software in its entirety and without
 modifications, you must retain this notice and the following text and disclaimers in all
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
