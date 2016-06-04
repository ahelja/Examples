/*
 	ProgressCell.m
 	Copyright (c) 1997-2001 Apple Computer, Inc.
 	All rights reserved.

        IMPORTANT: This Apple software is supplied to you by Apple Computer,
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
        OF SUCH DAMAGE.
*/

#import "ProgressCell.h"

@implementation ProgressCell

- (id)init
{
    [super init];
    color               = [[NSColor redColor] retain]; /* set the default color */
    percentageIncrement = 5.0;           /* set the default percentageIncrement */
    percentage          = 0.0;                    /* set the default percentage */
    tag                 = 0;
    return self;
}

- (void)dealloc
{
    [color release];
    [super dealloc];
}


/*
 * Its important that if you allow, the user to alt drag and add cells to a matrix made up of
 * these cells (ProgressCell) that you implement the NSCopying Protocol, which means implementing
 * copyWithZone:
 */
- (id)copyWithZone:(NSZone *)zone
{
    ProgressCell *copy;

    copy = [[ProgressCell allocWithZone:zone] init];

    [copy setColor:[self color]];
    [copy setPercentageIncrement:[self percentageIncrement]];
    [copy setPercentage:[self percentage]];
    [copy setTag:[self tag]];
    return copy;
}

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)view
{
    NSRect	bounds;
    NSRect	r;

    [super drawInteriorWithFrame:cellFrame inView:controlView];
    controlView = view;
    [[NSColor controlColor] set];
    NSRectFill(cellFrame);

    bounds = cellFrame;
    if (percentage > 0) {
	r = bounds;
        r.size.width = NSWidth(r) * [self percentage] / 100;
        [color set];
        NSRectFill(r);
    }

    [[NSColor controlDarkShadowColor] set];
    NSFrameRect(cellFrame);
}

- (NSView *)controlView
{
    return controlView;
}

- (int)tag
{
    return tag;
}

- (void)setTag:(int)anInt
{
    tag = anInt;
}

- (float)percentageIncrement
{
    return percentageIncrement;
}

- (void)setPercentageIncrement:(float)newPercentageIncrement
{
    percentageIncrement = newPercentageIncrement;
}

- (float)percentage
{
    return percentage;
}

- (void)setPercentage:(float)newPercentage
{
    if (newPercentage > 100) {
	newPercentage = 100;
    } else {
        if (newPercentage < 0) {
            newPercentage = 0;
        }
    }
    if (percentage != newPercentage) {
        percentage = newPercentage;
        [(NSControl *)[self controlView] updateCellInside:self];
    }
}

- (NSColor *)color
{
    return color;
}

- (void)setColor:(NSColor *)aColor
{
    if (aColor != color){
        [color release];
        color = [aColor retain];
        [(NSControl *)[self controlView] updateCellInside:self];
    }
}

- (void)increment:(id)sender
{
        /* Let the above method do bounds checking & redisplay for us. */
    [self setPercentage:([self percentage] + [self percentageIncrement])];
}

- (id)initWithCoder:(NSCoder *)coder
{
    [super initWithCoder:coder];

    if ([coder allowsKeyedCoding]){
        percentageIncrement = [coder decodeFloatForKey:@"percentageIncrement"];
        percentage          = [coder decodeFloatForKey:@"percentage"];
        tag                 = [coder decodeFloatForKey:@"tag"];
        color               = [[coder decodeObjectForKey:@"color"] retain];
    }else{
        [coder decodeValueOfObjCType:@encode(float) at:&percentageIncrement];
        [coder decodeValueOfObjCType:@encode(float) at:&percentage];
        [coder decodeValueOfObjCType:@encode(int)   at:&tag];
        color = [[coder decodeObject] retain];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];

    if ([coder allowsKeyedCoding]){
        [coder encodeFloat:percentageIncrement forKey:@"percentageIncrement"];
        [coder encodeFloat:percentage forKey:@"percentage"];
        [coder encodeFloat:tag forKey:@"tag"];
        [coder encodeObject:color forKey:@"color"];
    }else{
        [coder encodeValueOfObjCType:@encode(float) at:&percentageIncrement];
        [coder encodeValueOfObjCType:@encode(float) at:&percentage];
        [coder encodeValueOfObjCType:@encode(int)   at:&tag];
        [coder encodeObject:color];
    }
}

@end
