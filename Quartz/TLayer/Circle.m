/* TLayer - Circle.m
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

#import "Circle.h"

@implementation Circle

- (id)init
{
    self = [super init];
    if (self == nil)
	return nil;

    return self;
}

- (void)dealloc
{
    [_color release];
    [super dealloc];
}

- (float)radius
{
    return _radius;
}

- (void)setRadius:(float)radius
{
    _radius = radius;
}

- (NSPoint)center
{
    return _center;
}

- (void)setCenter:(NSPoint)center
{
    _center = center;
}

- (NSColor *)color
{
    return _color;
}

- (void)setColor:(NSColor *)color
{
    if (_color != color) {
	[_color release];
	_color = [color retain];
    }
}

- (NSRect)bounds
{
    return NSMakeRect(_center.x - _radius, _center.y - _radius,
		      2 * _radius, 2 * _radius);
}

- (void)draw
{
    CGContextRef context;

    context = [[NSGraphicsContext currentContext] graphicsPort];

    [[self color] set];
    CGContextSetGrayStrokeColor(context, 0, 1);
    CGContextSetLineWidth(context, 1.5);

    CGContextSaveGState(context);

    CGContextTranslateCTM(context, _center.x, _center.y);
    CGContextScaleCTM(context, _radius, _radius);
    CGContextMoveToPoint(context, 1, 0);
    CGContextAddArc(context, 0, 0, 1, 0, 2 * M_PI, false);
    CGContextClosePath(context);

    CGContextRestoreGState(context);
    CGContextDrawPath(context, kCGPathFill);
}

@end
