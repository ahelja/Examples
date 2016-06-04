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
/* UnitsView.m */

#import "UnitsView.h"


@implementation UnitsView

- (void)drawRect:(NSRect) rect
{
    NSString *					theString;
    NSAttributedString *		theAttrString;
    float						i, length;
    float						granularity;
    float						theNumber;
    float						yPos;
    
    // clear background
    [[NSColor whiteColor] set];
    [NSBezierPath fillRect:rect];
    
    // border rectangle
    [[NSColor blackColor] set];
    [NSBezierPath strokeRect:rect];
    
    granularity = 50;
    [[NSColor blackColor] set];
    
    // draw value ticks & labels    
    if ( (mDataSet == WORK_LOOP) || (mDataSet == FT_LATENCY_TRACE) ) {
        length = NSHeight ([self frame]) / granularity;
        // FT & WORK LOOP 
        for (i = 0; i < length; i++) {
            if (mScaleFactor == 0) {
                theNumber = 0;
            } else {
                theNumber = i * granularity / mScaleFactor;
            }
            
			if (mDataSet == WORK_LOOP) {
				theNumber *= 0.001;		// scale down by 1000 (to equal advertised unit of 'kiloloop')
			}
			
            theString = [[NSString alloc] initWithFormat:@"%4.0f", theNumber, nil];
            theAttrString = [[NSAttributedString alloc] initWithString:theString];
            [NSBezierPath strokeLineFromPoint:NSMakePoint (0, i * granularity) toPoint:NSMakePoint (5, i * granularity)];
            yPos = i * granularity - [theAttrString size].height * 0.5;
            if (yPos < 0) yPos = 0;
            [theAttrString drawAtPoint:NSMakePoint (6, yPos)];
            [theAttrString release];
            [theString release];
        }
    } else {
        float						halfHeight;
        
        halfHeight = NSHeight([self frame]) * 0.5;
        
        // HAL LATENCY MARKS
        length = NSHeight ([self frame]) * 0.5 / granularity;
        for (i = 0; i < length; i++) {
            if (mScaleFactor == 0) {
                theNumber = 0;
            } else {
                theNumber = i * granularity / mScaleFactor;
            }
            
            theString = [[NSString alloc] initWithFormat:@"%3.0f", theNumber, nil];
            theAttrString = [[NSAttributedString alloc] initWithString:theString];
            yPos = i * granularity - [theAttrString size].height * 0.5;
            [theAttrString drawAtPoint:NSMakePoint (6, halfHeight + yPos)];
            [NSBezierPath	strokeLineFromPoint:NSMakePoint (0, halfHeight + i * granularity)
                            toPoint:NSMakePoint (5, halfHeight + i * granularity)];
            if (i != 0) {
                [theAttrString release];
                [theString release];
                theString = [[NSString alloc] initWithFormat:@"%3.0f", theNumber * -1.0, nil];
                theAttrString = [[NSAttributedString alloc] initWithString:theString];
                yPos = i * granularity + [theAttrString size].height * 0.5;
                [theAttrString drawAtPoint:NSMakePoint (6, halfHeight - yPos)];
                [NSBezierPath	strokeLineFromPoint:NSMakePoint (0, halfHeight - i * granularity)
                                toPoint:NSMakePoint (5, halfHeight - i * granularity)];
            }
            [theAttrString release];
            [theString release];
        }
    }
    
}

- (void) setScaleFactor:(float)inScaleFactor forDataSet:(int)inDataSet
{
    mScaleFactor = inScaleFactor;
    mDataSet = inDataSet;
    
    [self setNeedsDisplay:YES];
}

- (void) setDataSet:(int)inDataSet
{
    mDataSet = inDataSet;
    
    [self setNeedsDisplay:YES];
}

- (BOOL) isFlipped	{	return NO;	}
- (BOOL) isOpaque	{	return YES;	}


@end

/*
 */
