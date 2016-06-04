/* TLayer - TLayerView.m
 *
 * Author: Derek Clegg
 * Created 17 February 2003
 *
 * Copyright (c) 2003-2004 Apple Computer, Inc.
 * All rights reserved.
 */

/* IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.

 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE. */

#import "TLayerView.h"
#import "Circle.h"
#import "Extras.h"

#define CIRCLE_COUNT 3

@interface NSEvent (TLayerViewInternal)
- (NSPoint)locationInView:(NSView *)view;
@end

@implementation TLayerView

- (id)initWithFrame:(NSRect)frame
{
    size_t k;
    Circle *circle;
    NSColor *color;
    static const float circleRadius = 100;
    static const float colors[CIRCLE_COUNT][4] = {
	{ 0.5, 0.0, 0.5, 1 },
	{ 1.0, 0.7, 0.0, 1 },
	{ 0.0, 0.5, 0.0, 1 }
    };

    self = [super initWithFrame:frame];
    if (self == nil)
	return nil;

    useTLayer = false;

    circles = [[NSMutableArray alloc] initWithCapacity:CIRCLE_COUNT];
    
    for (k = 0; k < CIRCLE_COUNT; k++) {
	color = [NSColor colorWithCalibratedRed:colors[k][0] green:colors[k][1]
			 blue:colors[k][2] alpha:colors[k][3]];
	circle = [[Circle alloc] init];
	[circle setColor:color];
	[circle setRadius:circleRadius];
	[circle setCenter:makeRandomPointInRect([self bounds])];
	[circles addObject:circle];
	[circle release];
    }

    [self registerForDraggedTypes:[NSArray arrayWithObject:NSColorPboardType]];

    [self setNeedsDisplay:YES];

    return self;
}

- (void)dealloc
{
    [circles release];
    [super dealloc];
}

- (float)shadowRadius
{
    return shadowRadius;
}

- (void)setShadowRadius:(float)radius
{
    if (shadowRadius != radius) {
	shadowRadius = radius;
	[self setNeedsDisplay:YES];
    }
}

- (CGSize)shadowOffset
{
    return shadowOffset;
}

- (void)setShadowOffset:(CGSize)offset
{
    if (!CGSizeEqualToSize(shadowOffset, offset)) {
	shadowOffset = offset;
	[self setNeedsDisplay:YES];
    }
}

- (bool)usesTransparencyLayers
{
    return useTLayer;
}

- (void)setUsesTransparencyLayers:(bool)state
{
    if (useTLayer != state) {
	useTLayer = state;
	[self setNeedsDisplay:YES];
    }
}

- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    return YES;
}

- (NSRect)boundsForCircle:(Circle *)circle
{
    float dx, dy;

    dx = 2 * fabs(shadowOffset.width) + 2 * shadowRadius;
    dy = 2 * fabs(shadowOffset.height) + 2 * shadowRadius;
    return NSInsetRect([circle bounds], -dx, -dy);
}

- (void)dragCircleAtIndex:(size_t)index withEvent:(NSEvent *)event
{
    Circle *circle;
    unsigned int mask;
    NSPoint start, point, center;

    circle = [[circles objectAtIndex:index] retain];
    [circles removeObjectAtIndex:index];
    [circles addObject:circle];
    [circle release];

    [self setNeedsDisplayInRect:[self boundsForCircle:circle]];

    mask = NSLeftMouseDraggedMask | NSLeftMouseUpMask;

    start = [event locationInView:self];

    while (1) {
        event = [[self window] nextEventMatchingMask:mask];
	if ([event type] == NSLeftMouseUp)
	    break;

	[self setNeedsDisplayInRect:[self boundsForCircle:circle]];

	center = [circle center];
	point = [event locationInView:self];
	center.x += point.x - start.x;
	center.y += point.y - start.y;
	[circle setCenter:center];
	
	[self setNeedsDisplayInRect:[self boundsForCircle:circle]];

	start = point;
    }
}

- (int)indexOfCircleAtPoint:(NSPoint)point
{
    Circle *circle;
    size_t k, count;
    float dx, dy, radius;
    NSPoint center;

    count = [circles count];
    for (k = 0; k < count; k++) {
	circle = [circles objectAtIndex:(count - 1 - k)];
	center = [circle center];
	radius = [circle radius];
	dx = point.x - center.x;
	dy = point.y - center.y;
	if (dx * dx + dy * dy < radius * radius)
	    return count - 1 - k;
    }
    return -1;
}

- (void)mouseDown:(NSEvent *)event
{
    int index;
    NSPoint point;

    point = [event locationInView:self];
    index = [self indexOfCircleAtPoint:point];
    if (index >= 0)
	[self dragCircleAtIndex:index withEvent:event];
}

- (void)setFrame:(NSRect)frame
{
    [super setFrame:frame];
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect
{
    NSRect bounds;
    Circle *circle;
    size_t k, count;
    CGContextRef context;

    context = [[NSGraphicsContext currentContext] graphicsPort];

    CGContextSetRGBFillColor(context, 0.7, 0.7, 0.9, 1);
    CGContextFillRect(context, convertToCGRect(rect));

    CGContextSetShadow(context, shadowOffset, shadowRadius);

    if (useTLayer)
	CGContextBeginTransparencyLayer(context, NULL);

    count = [circles count];
    for (k = 0; k < count; k++) {
	circle = [circles objectAtIndex:k];
	bounds = [self boundsForCircle:circle];
	if (NSIntersectsRect(bounds, rect))
	    [circle draw];
    }

    if (useTLayer)
	CGContextEndTransparencyLayer(context);
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    NSPasteboard *pasteboard;
    
    /* Since we have only registered for NSColorPboardType drags, this is
     * actually unneeded. If you were to register for any other drag types,
     * though, this code would be necessary. */

    if (([sender draggingSourceOperationMask] & NSDragOperationGeneric) != 0) {
	pasteboard = [sender draggingPasteboard];
	if ([[pasteboard types] containsObject:NSColorPboardType])
	    return NSDragOperationGeneric;
    }

    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    int index;
    NSPoint point;
    
    point = [self convertPoint:[sender draggingLocation] fromView:nil];
    index = [self indexOfCircleAtPoint:point];
    
    if (index >= 0) {
        /* The current drag location is inside the bounds of a circle so we
	 * accept the drop and move on to concludeDragOperation:. */
	return YES;
    }
    return NO;
}

- (void)concludeDragOperation:(id <NSDraggingInfo>)sender
{
    int index;
    NSPoint point;
    NSColor *color;
    Circle *circle;

    color = [NSColor colorFromPasteboard:[sender draggingPasteboard]];
    point = [self convertPoint:[sender draggingLocation] fromView:nil];
    index = [self indexOfCircleAtPoint:point];
    
    if (index >= 0) {
        circle = [circles objectAtIndex:index];
	[circle setColor:color];
        [self setNeedsDisplayInRect:[self boundsForCircle:circle]];
    }
}

@end

@implementation NSEvent (TLayerViewInternal)

- (NSPoint)locationInView:(NSView *)view
{
    return [view convertPoint:[self locationInWindow] fromView:nil];
}

@end
