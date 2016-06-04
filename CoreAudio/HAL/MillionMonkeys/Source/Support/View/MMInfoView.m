/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* MMInfoView.m */

#import "MMInfoView.h"


@implementation MMInfoView

- (id)initWithFrame:(NSRect) frame
{
    self = [super initWithFrame:frame];
	
    if (self) {
        mTitleAttributes = [NSDictionary dictionaryWithObject:[NSFont boldSystemFontOfSize:14]
 forKey:NSFontAttributeName];
		[mTitleAttributes retain];
	}
    return self;
}

- (void) dealloc
{
    [mTitleAttributes release];
	
	[super dealloc];
}

- (void)drawRect:(NSRect) rect
{
	float					x, y;
	float					textSpacing, minorSpacing, majorSpacing;
	
	x = 5.0;
	y = 5.0;
	textSpacing = 1.0;
	minorSpacing = 5.0;
	majorSpacing = 15.0;
	
	// clear background
	[[NSColor whiteColor] set];
    [NSBezierPath fillRect:rect];
	
	// draw border
	[[NSColor blackColor] set];
    [NSBezierPath strokeRect:rect];
		
	// title
	y += [self drawTitle:@"Graph Key" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing];
	
	y += majorSpacing;
	
	// work done key
	y += [self drawTitle:@"Work Done:" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing];
	
	y += [self	drawTitledKeySwatch:@"minimum" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:0.00f green:0.00f blue:1.00f alpha:1.00f]	];
	
	y += [self	drawTitledKeySwatch:@"mean" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:0.00f green:0.50f blue:1.00f alpha:1.00f]	];
	
	y += [self	drawTitledKeySwatch:@"maximum" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:0.00f green:1.00f blue:1.00f alpha:1.00f]	];
	
	y += majorSpacing;
	
	// HAL latency key
	y += [self drawTitle:@"HAL I/O Thread Latency:" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing];
	
	y += [self	drawTitledKeySwatch:@"minimum" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:1.00f green:0.00f blue:0.00f alpha:1.00f]	];
	
	y += [self	drawTitledKeySwatch:@"mean" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:1.00f green:0.50f blue:0.00f alpha:1.00f]	];
	
	y += [self	drawTitledKeySwatch:@"maximum" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:1.00f green:1.00f blue:0.00f alpha:1.00f]	];
	
	y += [self	drawTitledKeySwatch:@"overload (vertical line)" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:1.00f green:0.00f blue:0.00f alpha:0.50f]	];	
	
	y += majorSpacing;
	
	// Feeder Thread latency key
	y += [self drawTitle:@"Feeder Thread Latency:" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing];
	
	y += [self	drawTitledKeySwatch:@"minimum" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:0.00f green:0.50f blue:0.00f alpha:1.00f]	];
	
	y += [self	drawTitledKeySwatch:@"mean" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:0.00f green:1.00f blue:0.00f alpha:1.00f]	];
	
	y += [self	drawTitledKeySwatch:@"maximum" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:0.50f green:1.00f blue:0.00f alpha:1.00f]	];
	
	y += [self	drawTitledKeySwatch:@"overload (vertical)" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:0.66f green:0.00f blue:1.00f alpha:0.50f]	];	
	
	y += [self	drawTitledKeySwatch:@"FT blocked for cycle (vertical)" atPoint:NSMakePoint (x, y) withSpacing:minorSpacing
				withColor:[NSColor colorWithCalibratedRed:1.00f green:0.66f blue:0.33f alpha:0.50f]	];
	
	y += majorSpacing;
	
	// notes
	// note about dashed-line overloads
	y += [self drawTitle:@"A note about overloads:" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"when performing a CPU trace," atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"you may get some dashed-line" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"overloads.  These indicate that" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"the overload was caused purely" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"because the application took" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"a CPU trace, and would not have" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"occurred otherwise." atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	
	y += majorSpacing;
	
	// note about HAL latency immediately following overloads
	y += [self drawText:@"Also: the HAL cycle(s) immediately" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"following an overload will be" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"skipped to allow the thread that" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"overloaded to finish." atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"Consequently, the HAL latency for" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"that/those cycle(s) will be a" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"standard latency + the size of the" atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	y += [self drawText:@"IOProc(s)." atPoint:NSMakePoint (x, y) withSpacing:textSpacing];
	
	y += majorSpacing;
}

- (float) drawTitle:(NSString *)inString atPoint:(NSPoint)inPoint withSpacing:(float)inSpacing
{
	float					height;
	NSAttributedString *	newString;
		
	newString = [[NSAttributedString alloc] initWithString:inString attributes:mTitleAttributes];
	[newString drawAtPoint:inPoint];
	height = [newString size].height + inSpacing;
	
	return height;
}

- (float) drawText:(NSString *)inString atPoint:(NSPoint)inPoint withSpacing:(float)inSpacing
{
	float					height;
	NSAttributedString *	newString;
		
	newString = [[NSAttributedString alloc] initWithString:inString];
	[newString drawAtPoint:inPoint];
	height = [newString size].height + inSpacing;
	
	return height;
}

- (float) drawTitledKeySwatch:(NSString *)inString atPoint:(NSPoint)inPoint withSpacing:(float)inSpacing withColor:(NSColor *)inColor
{
	float					height;
	NSAttributedString *	newString;
	NSRect					newRect;
	
	newString = [[NSAttributedString alloc] initWithString:inString];
	height = [newString size].height;
	newRect = NSMakeRect (inPoint.x + (inSpacing * 3), inPoint.y, height, height);
	[newString drawAtPoint:NSMakePoint (inPoint.x + height + (inSpacing * 4), inPoint.y)];
	[inColor set];
    [NSBezierPath fillRect:newRect];
	
	return height + inSpacing;
}

- (BOOL)isFlipped	{ return YES; }
- (BOOL)isOpaque	{ return YES; }

@end

/*
 */
