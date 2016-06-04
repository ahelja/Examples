/*

File: GMRectangleDraggerView.m

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright 2002,2004 Apple Computer, Inc., All Rights Reserved

*/ 

#import "GMRectangleDraggerView.h"

NSString * const UserEnteredDraggedRectNotification = @"UserEnteredDraggedRectNotification";
NSString * const NewDraggedRecNotification = @"NewDraggedRecNotification";

@implementation GMRectangleDraggerView: NSImageView

- (id)init;
{
    self = [super init];

    return self;
}

- (void)awakeFromNib;
{
    [self setImageScaling:NSScaleToFit];
    
    [(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(userEnteredDraggedRect:)
                                                 name:UserEnteredDraggedRectNotification
                                               object:nil];

    _selectionMarker = [[GMCropMarker cropMarkerForView:self] retain];
}

- (NSRect)draggedRect;
{
    return [_selectionMarker selectedRect];
}

- (void)setDraggedRectWithX:(float)x y:(float)y w:(float)w h:(float)h;
{
    [_selectionMarker setSelectedRect:NSMakeRect(x, y, w, h)];
	
	[self setNeedsDisplay:YES];
}

- (void)mouseDown:(NSEvent *)theEvent;
{
    [_selectionMarker mouseDown:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent;
{ 
	[_selectionMarker mouseUp:theEvent]; 
	[self selectionChanged];
    
    [(NSNotificationCenter *)[NSNotificationCenter defaultCenter] postNotificationName:NewDraggedRecNotification
                                                        object:self];
    
}

- (void)mouseDragged:(NSEvent *)theEvent;
{ 
	[_selectionMarker mouseDragged:theEvent]; 
	[self selectionChanged];

    [(NSNotificationCenter *)[NSNotificationCenter defaultCenter] postNotificationName:NewDraggedRecNotification
                                                        object:self];
    	
}

- (void)selectionChanged;
{
	[self setNeedsDisplay:YES];
}

- (void)userEnteredDraggedRect:(NSNotification *)aNotification;
{
    [self setDraggedRectWithX:[[[aNotification userInfo] objectForKey:@"x"] floatValue]
                            y:[[[aNotification userInfo] objectForKey:@"y"] floatValue]
                            w:[[[aNotification userInfo] objectForKey:@"w"] floatValue]
                            h:[[[aNotification userInfo] objectForKey:@"h"] floatValue]];
		
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect;
{
    [super drawRect:rect];
    
    [_selectionMarker drawCropMarker];
}

@end
