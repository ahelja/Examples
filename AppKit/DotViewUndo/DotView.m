/*
 DotView.m
 Simple NSView subclass showing how to draw, handle simple events, and
   target/action methods. 
 This version also adds ability to undo all changes by simple use of
   NSUndoManager class in the setters.

 Ali Ozer

 Copyright (c) 2000-2004, Apple Computer, Inc., all rights reserved.
*/
/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
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

#import <Cocoa/Cocoa.h>
#import "DotView.h"

@implementation DotView

// initWithFrame: is NSView's designated initializer (meaning it should be
// overridden in the subclassers if needed, and it should call super, that is
// NSView's implementation).  In DotView we do just that, and also set the
// instance variables.
//
// Note that we initialize the instance variables here in the same way they are
// initialized in the nib file. This is adequate, but a better solution is to make
// sure the two places are initialized from the same place. Slightly more
// sophisticated apps which load nibs for each document or window would initialize
// UI elements at the time they're loaded from values in the program.

- (id)initWithFrame:(NSRect)frame {
    [super initWithFrame:frame];
    center.x = 50.0;
    center.y = 50.0;
    radius = 10.0;
    color = [[NSColor redColor] retain];
    return self;
}

// dealloc is the method called when objects are being freed. (Note that "release"
// is called to release objects; when the number of release calls reduce the
// total reference count on an object to zero, dealloc is called to free
// the object.  dealloc should free any memory allocated by the subclass
// and then call super to get the superclass to do additional cleanup.

- (void)dealloc {
    [color release];
    [super dealloc];
}

// drawRect: should be overridden in subclassers of NSView to do necessary
// drawing in order to recreate the the look of the view. It will be called
// to draw the whole view or parts of it (pay attention the rect argument);
// it will also be called during printing if your app is set up to print.
// In DotView we first clear the view to white, then draw the dot at its
// current location and size.

- (void)drawRect:(NSRect)rect {
    // Draw the background
    [[NSColor whiteColor] set];
    NSRectFill([self bounds]);

    // Draw the dot
    NSRect dotRect = NSMakeRect(center.x - radius, center.y - radius, 2 * radius, 2 * radius);
    [color set];
    [[NSBezierPath bezierPathWithOvalInRect:dotRect] fill];
}

// Views which totally redraw their whole bounds without needing any of the
// views behind it should override isOpaque to return YES. This is a performance
// optimization hint for the display subsystem. This applies to DotView, whose
// drawRect: does fill the whole rect its given with a solid, opaque color.

- (BOOL)isOpaque {
    return YES;
}

// Accessors for the attributes of the dot.  One difference between this version
// of DotView and the "original" version is the use of explicit accessors, which
// of course provide the funnel points for hooking in undo.
//
// Note that when an attribute is changed, we call setNeedsDisplay:YES to 
// mark that the view needs to be redisplayed (which then is done automatically
// by the AppKit).


- (void)setCenter:(NSPoint)newCenter {
    if (!NSEqualPoints(center, newCenter)) {
	// The undo
	[[[[self window] undoManager] prepareWithInvocationTarget:self] setCenter:center];
	// Update the instance variable
	center = newCenter;
	// Mark view as needing redisplay
	[self setNeedsDisplay:YES];
    }
}

- (NSPoint)center {
    return center;
}

- (void)setRadius:(float)newRadius {
    if (radius != newRadius) {
	// The undo
	[[[[self window] undoManager] prepareWithInvocationTarget:self] setRadius:radius];
	// Update the instance variable
	radius = newRadius;
	// Mark view as needing redisplay
	[self setNeedsDisplay:YES];
    }
}

- (float)radius {
    return radius;
}

/* setColor: shows how an object is held on to in an instance variable; the previous instance is released, and the new instance is retained (or if appropriate, copied). We also check to see if the new and the old value are the same, both as a performance optimization, and to avoid prematurely releasing the object if the previous value is passed back in.  
*/
- (void)setColor:(NSColor *)newColor {
    if (newColor != color) {
	// The undo
	[[[self window] undoManager] registerUndoWithTarget:self selector:@selector(setColor:) object:color];
	// Update the color
	[color release];
	color = [newColor retain];
	// Mark view as needing to redisplay
	[self setNeedsDisplay:YES];
    }
}

/* The color getter.  The retain/autorelease is for added safety of returned objects and the recommended way to return objects except in performance-critical situations or cases where the lifetime of the returned object should not be extended. In those cases the appropriate thing would be to just return the instance variable.
*/
- (NSColor *)color {
    return [[color retain] autorelease];
}

// Recommended way to handle events is to override NSResponder (superclass
// of NSView) methods in the NSView subclass. One such method is mouseUp:.
// These methods get the event as the argument. The event has the mouse
// location in window coordinates; use convertPoint:fromView: (with "nil"
// as the view argument) to convert this point to local view coordinates.

- (void)mouseUp:(NSEvent *)event {
    NSPoint eventLocationInViewCoords = [self convertPoint:[event locationInWindow] fromView:nil];
    [self setCenter:eventLocationInViewCoords];
}

// Target/action methods. In this version of DotView the action methods simply call the 
// setters.

- (void)changeSize:(id)sender {
    [self setRadius:[sender floatValue]];
}

- (void)changeColor:(id)sender {
    [self setColor:[sender color]];
}

@end

