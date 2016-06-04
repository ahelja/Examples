/* PathDemo - DemoView.m
 *
 * Author: Nick Kocharhook (the Cocoa portion) + other CG engineers
 * Created 11 July 2003
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

#import "DemoView.h"
#import <ApplicationServices/ApplicationServices.h>

#define PI M_PI /* From <math.h>. */

static void rectangles(CGContextRef, CGRect);
static void circles(CGContextRef, CGRect);
static void bezierPaths(CGContextRef, CGRect);
static void circleClipping(CGContextRef, CGRect);

/* A convenience function to get a CGRect from an NSRect. You can also use
 * the *(CGRect *)&nsRect sleight-of-hand, but this way is clearer. */

static inline CGRect
convertToCGRect(NSRect rect)
{
    return CGRectMake(rect.origin.x, rect.origin.y,
		      rect.size.width, rect.size.height);
}

@implementation DemoView

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self == nil)
	return nil;

    demoNumber = 0;

    return self;
}

- (void)drawRect:(NSRect)rect
{
    CGRect r;
    CGContextRef context;

    r = convertToCGRect(rect);
    context = [[NSGraphicsContext currentContext] graphicsPort];
    
    CGContextSetGrayFillColor(context, 1.0, 1.0);
    CGContextFillRect(context, r);
    
    switch (demoNumber) {
    case 0:
	rectangles(context, r);
	break;
    case 1:
	circles(context, r);
	break;
    case 2:
	bezierPaths(context, r);
	break;
    case 3:
	circleClipping(context, r);
	break;
    default:
	NSLog(@"Invalid demo number.");
	break;
    }
}

- (void)setDemoNumber:(int)number
{
    demoNumber = number;
}

/* The various demo functions. */

static float
randf(float min, float max)
{
    return min + (max - min) * rand() / (float)RAND_MAX;
}

static void
setRandomFillColor(CGContextRef context)
{
    CGContextSetRGBFillColor(context, randf(0, 1), randf(0, 1),
			       randf(0, 1), randf(0, 1));
}

static void
setRandomStrokeColor(CGContextRef context)
{
    CGContextSetRGBStrokeColor(context, randf(0, 1), randf(0, 1),
			       randf(0, 1), randf(0, 1));
}

static CGPoint
randomPointInRect(CGRect rect)
{
    CGPoint p;

    p.x = randf(CGRectGetMinX(rect), CGRectGetMaxX(rect));
    p.y = randf(CGRectGetMinY(rect), CGRectGetMaxY(rect));
    return p;
}

static CGRect
randomRectInRect(CGRect rect)
{
    CGPoint p, q;

    p = randomPointInRect(rect);
    q = randomPointInRect(rect);
    return CGRectMake(p.x, p.y, q.x - p.x, q.y - p.y);
}

static void
rectangles(CGContextRef context, CGRect rect)
{
    size_t k;

    /* Draw random rectangles (some stroked, some filled). */

    for (k = 0; k < 20; k++) {
        if (k % 2 == 0) {
	    setRandomFillColor(context);
            CGContextFillRect(context, randomRectInRect(rect));
        } else {
	    setRandomStrokeColor(context);
            CGContextSetLineWidth(context, 2 + rand() % 10);
            CGContextStrokeRect(context, randomRectInRect(rect));
        }
    }
}

static void
circles(CGContextRef context, CGRect rect)
{
    size_t k;
    float w, h;
    CGRect r;

    /* Draw random circles (some stroked, some filled). */

    for (k = 0; k < 20; k++) {
	r = randomRectInRect(rect);
	w = CGRectGetWidth(r);
	h = CGRectGetHeight(r);
        CGContextBeginPath(context);
        CGContextAddArc(context, CGRectGetMidX(r), CGRectGetMidY(r),
			(w < h) ? w : h, 0, 2*PI, false);
        CGContextClosePath(context);
        if (k % 2 == 0) {
	    setRandomFillColor(context);
            CGContextFillPath(context);
        } else {
	    setRandomStrokeColor(context);
            CGContextSetLineWidth(context, 2 + rand() % 10);
            CGContextStrokePath(context);
        }
    }
}

static void
bezierPaths(CGContextRef context, CGRect rect)
{
    CGPoint c1, c2, p;
    size_t j, k, numberOfSegments;

    for (k = 0; k < 20; k++) {
        numberOfSegments = 1 + rand() % 8;
        CGContextBeginPath(context);
	p = randomPointInRect(rect);
        CGContextMoveToPoint(context, p.x, p.y);
        for (j = 0; j < numberOfSegments; j++) {
	    p = randomPointInRect(rect);
            if (j % 2 == 0) {
                CGContextAddLineToPoint(context, p.x, p.y);
            } else {
		c1 = randomPointInRect(rect);
		c2 = randomPointInRect(rect);
                CGContextAddCurveToPoint(context, c1.x, c1.y,
					 c2.x, c2.y, p.x, p.y);
            }
        }
        if (k % 2 == 0) {
	    setRandomFillColor(context);
	    CGContextClosePath(context);
            CGContextFillPath(context);
        } else {
	    setRandomStrokeColor(context);
            CGContextSetLineWidth(context, 2 + rand() % 10);
            CGContextStrokePath(context);
        }
    }
}

static void
circleClipping(CGContextRef context, CGRect rect)
{
    float w, h;

    /* Draw a random path through a circular clip. */

    w = CGRectGetWidth(rect);
    h = CGRectGetHeight(rect);
    CGContextBeginPath(context);
    CGContextAddArc(context, CGRectGetMidX(rect), CGRectGetMidY(rect),
		    ((w < h) ? w : h)/2, 0, 2*PI, false);
    CGContextClosePath(context);
    CGContextClip(context);
    
    /* Draw something into the clip. */
    bezierPaths(context, rect);
    
    /* Draw a clip path on top as a black stroked circle. */
    CGContextBeginPath(context);
    CGContextAddArc(context, CGRectGetMidX(rect), CGRectGetMidY(rect),
		    ((w < h) ? w : h)/2, 0, 2*PI, false);
    CGContextClosePath(context);
    CGContextSetLineWidth(context, 1);
    CGContextSetRGBStrokeColor(context, 0, 0, 0, 1);
    CGContextStrokePath(context);
}

@end
