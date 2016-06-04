/* TLayer - ShadowOffsetView.m
 *
 * Author: Derek Clegg
 * Created 3 March 2003
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

#import "ShadowOffsetView.h"
#import "Extras.h"

NSString *ShadowOffsetChanged = @"ShadowOffsetChanged";

@interface ShadowOffsetView (Internal)
- (NSCell *)cell;
@end

@implementation ShadowOffsetView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self == nil)
	return nil;

    _offset = CGSizeZero;

    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (float)scale
{
    return _scale;
}

- (void)setScale:(float)scale
{
    _scale = scale;
}

- (CGSize)offset
{
    return CGSizeMake(_offset.width * _scale, _offset.height * _scale);
}

- (void)setOffset:(CGSize)offset
{
    offset = CGSizeMake(offset.width / _scale, offset.height / _scale);
    if (!CGSizeEqualToSize(_offset, offset)) {
	_offset = offset;
	[self setNeedsDisplay:YES];
    }
}

- (BOOL)isOpaque
{
    return NO;
}

- (void)setOffsetFromPoint:(NSPoint)point
{
    float radius;
    CGSize offset;
    NSRect bounds;
    
    bounds = [self bounds];
    offset.width = (point.x - NSMidX(bounds)) / (NSWidth(bounds) / 2);
    offset.height = (point.y - NSMidY(bounds)) / (NSHeight(bounds) / 2);
    radius = sqrt(offset.width * offset.width + offset.height * offset.height);
    if (radius > 1) {
	offset.width /= radius;
	offset.height /= radius;
    }
    if (!CGSizeEqualToSize(_offset, offset)) {
	_offset = offset;
	[self setNeedsDisplay:YES];
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter]
	    postNotificationName:ShadowOffsetChanged object:self];
    }
}

- (void)mouseDown:(NSEvent *)event
{
    NSPoint point;

    point = [self convertPoint:[event locationInWindow] fromView:nil];
    [self setOffsetFromPoint:point];
}

- (void)mouseDragged:(NSEvent *)event
{
    NSPoint point;

    point = [self convertPoint:[event locationInWindow] fromView:nil];
    [self setOffsetFromPoint:point];
}

- (void)drawRect:(NSRect)rect
{
    NSRect bounds;
    CGContextRef context;
    float x, y, w, h, r;

    bounds = [self bounds];
    x = NSMinX(bounds);
    y = NSMinY(bounds);
    w = NSWidth(bounds);
    h = NSHeight(bounds);
    r = MIN(w / 2, h / 2);
    
    context = [[NSGraphicsContext currentContext] graphicsPort];

    CGContextTranslateCTM(context, x + w/2, y + h/2);

    CGContextAddArc(context, 0, 0, r, 0, 2*M_PI, true);
    CGContextClip(context);

    CGContextSetGrayFillColor(context, 0.910, 1);
    CGContextFillRect(context, CGRectMake(-w/2, -h/2, w, h));

    CGContextAddArc(context, 0, 0, r, 0, 2*M_PI, true);
    CGContextSetGrayStrokeColor(context, 0.616, 1);
    CGContextStrokePath(context);

    CGContextAddArc(context, 0, -2, r, 0, 2*M_PI, true);
    CGContextSetGrayStrokeColor(context, 0.784, 1);
    CGContextStrokePath(context);

    CGContextMoveToPoint(context, 0, 0);
    CGContextAddLineToPoint(context, r * _offset.width, r * _offset.height);

    CGContextSetLineWidth(context, 2);
    CGContextSetGrayStrokeColor(context, 0.33, 1);
    CGContextStrokePath(context);
}

@end
