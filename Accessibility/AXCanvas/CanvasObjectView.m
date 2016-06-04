/*
 
 File: CanvasObjectView.m
 
 Abstract: Canvas object NSView subclass
 
 Version: 1.0
 
 © Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to 
 you by Apple Computer, Inc. ("Apple") in 
 consideration of your agreement to the following 
 terms, and your use, installation, modification 
 or redistribution of this Apple software 
 constitutes acceptance of these terms.  If you do 
 not agree with these terms, please do not use, 
 install, modify or redistribute this Apple 
 software.
 
 In consideration of your agreement to abide by 
 the following terms, and subject to these terms, 
 Apple grants you a personal, non-exclusive 
 license, under Apple's copyrights in this 
 original Apple software (the "Apple Software"), 
 to use, reproduce, modify and redistribute the 
 Apple Software, with or without modifications, in 
 source and/or binary forms; provided that if you 
 redistribute the Apple Software in its entirety 
 and without modifications, you must retain this 
 notice and the following text and disclaimers in 
 all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or 
 logos of Apple Computer, Inc. may be used to 
 endorse or promote products derived from the 
 Apple Software without specific prior written 
 permission from Apple.  Except as expressly 
 stated in this notice, no other rights or 
 licenses, express or implied, are granted by 
 Apple herein, including but not limited to any 
 patent rights that may be infringed by your 
 derivative works or by other works in which the 
 Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS 
 IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
 THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
 OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
 SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
 THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
 UNDER THEORY OF CONTRACT, TORT (INCLUDING 
 NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
 IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.
 
 */ 


#import "CanvasObjectView.h"
#import "CanvasView.h"
#import "CanvasDoc.h"

// Provides a view for CanvasObject.
@implementation CanvasObjectView

- (id)initWithCanvasObject:(CanvasObject *)canvasObject {
	// Designated initializer.
	self = [super initWithFrame:[canvasObject frame]];
	if(self) {
		[self setRepresentedObject:canvasObject];
		isDragging = NO;
	}
	
	return self;
}

- (void)dealloc {
	[representedObject release];
	[super dealloc];
}

#pragma mark Attribute accessors
- (CanvasObject *)representedObject {
	return [[representedObject retain] autorelease];
}

- (void)setRepresentedObject:(CanvasObject *)canvasObject {
	if(canvasObject != representedObject) {
		
		if(representedObject)
			[self stopObservingRepresentedObject];
		
		[representedObject release];
		representedObject = [canvasObject retain];
		
		// Observe keys which require special handling.
		[representedObject addObserver:self
							forKeyPath:@"frame"
							   options:NSKeyValueObservingOptionOld
							   context:nil];
		
		[representedObject addObserver:self
							forKeyPath:@"isSelected"
							   options:nil
							   context:nil];		
		
		// Observe any key which requires a redraw.
		NSEnumerator *e = [[representedObject drawingKeys] objectEnumerator];
		NSString *key;
		while(key = (NSString *)[e nextObject])
			[representedObject addObserver:self
								forKeyPath:key
								   options:nil
								   context:nil];
	}
}

#pragma mark Drawing methods
- (void)drawRect:(NSRect)rect {
	[self drawFill:rect];
	[self drawStroke:rect];
}

- (void)drawFill:(NSRect)rect {
	if([representedObject hasFill]) {
		BOOL isSelected = [representedObject isSelected];
		
		[[representedObject fillColor] set];
		[[NSBezierPath bezierPathWithRect:rect] fill];
		
	}
}

- (void)drawStroke:(NSRect)rect {
	if([representedObject hasStroke]) {
		[[representedObject strokeColor] set];
		NSBezierPath *path = [NSBezierPath bezierPathWithRect:[self bounds]];
		[path setLineWidth:[representedObject strokeWidth]];
		[path stroke];
	}
	
	if([representedObject isSelected]) {
		[[NSColor blueColor] set];
		NSFrameRect(NSInsetRect([self bounds], 1, 1));
	}
}

#pragma mark NSResponder methods
- (BOOL)acceptsFirstResponder {
	return YES;
}

- (void)mouseDown:(NSEvent *)e {
	if([e clickCount] == 1) {
		if([e modifierFlags] & (NSCommandKeyMask | NSShiftKeyMask)) {
			// Toggle the selection state of the receiver.
			if([representedObject isSelected])
				[[representedObject document] removeCanvasObjectsFromSelection:[NSArray arrayWithObject:representedObject]];
			else
				[[representedObject document] addCanvasObjectsToSelection:[NSArray arrayWithObject:representedObject]];
		} else if(![representedObject isSelected]) {
			// Make the receiver the only selected object.
			[[representedObject document] selectCanvasObject:representedObject];			
		}
	}
	isDragging = YES;
}

- (void)mouseDragged:(NSEvent *)e {
	// Ensure that the mouse actually moved.
	if(!isDragging) // Check if the user has cancelled the drag.
		return;
	
	if(([e deltaX] != 0) || ([e deltaY] != 0)) {
		
		// Grab the point before and after scrolling so the deltaX and deltaY
		// values can be appropriately adjusted for the scroll. This doesn't always
		// work when the users drags in the Dock or the menu bar, where coordinate
		// systems can work differently than expected. The right approach is to
		// save the mouseDown point and calculate the delta from that, but this
		// technique makes it easier to support the dragging of multiple canvas
		// objects at once.
		NSPoint aPoint = [e locationInWindow];
		NSPoint firstPoint = [self convertPoint:aPoint fromView:nil];
		[self autoscroll:e];
		NSPoint secondPoint = [self convertPoint:aPoint fromView:nil];
				
		// Since the scrolling delta is in the window's coordinate system, we
		// need to adjust for any potential zooming.
		float scaleFactor = [(CanvasView *)[self superview] scaleFactor];
		float deltaX = ([e deltaX] / scaleFactor) + (secondPoint.x - firstPoint.x);
		float deltaY = ([e deltaY] / scaleFactor) + (secondPoint.y - firstPoint.y);
		
		// Move each object inside the selection.
		NSEnumerator *enumerator = [[[representedObject document] selectedObjects] objectEnumerator];
		CanvasObject *canvasObject;
		NSView *view; NSPoint origin;
		while(canvasObject = [enumerator nextObject]) {
			// Calculate the new position of each selected canvas object.
			view = [canvasObject view];
			origin = [view frame].origin;
			origin.x += deltaX; origin.y += deltaY;

			// Redraw at the old and new positions.
			[[view superview] setNeedsDisplayInRect:[view frame]];
			[view setFrameOrigin:origin];
			[view setNeedsDisplay:YES];
		}
		
	}
}

- (void)mouseUp:(NSEvent *)e {
	// When the mouse is released, the represented object's position
	// can be updated, as long as the user hasn't cancelled.
	//if(!NSEqualPoints([representedObject position], [self frame].origin))
	//	[representedObject setPosition:[self frame].origin];
	if(!isDragging)
		return;
	
	NSEnumerator *enumerator = [[[representedObject document] selectedObjects] objectEnumerator];
	CanvasObject *canvasObject;
	while(canvasObject = [enumerator nextObject]) {
		if(NSEqualPoints([canvasObject position], [[canvasObject view] frame].origin))
			break;
		
		[canvasObject setPosition:[[canvasObject view] frame].origin];	
	}
	isDragging = NO;
	
}

#pragma mark Editing
- (void)startEditingWithEvent:(NSEvent *)anEvent {
	// Starts editing the object; overriden by subclasses.
	return;
}

- (void)stopEditing:(BOOL)shouldApplyChanges {
	// Stops editing the object, applying changes if shouldApplyChanges is YES.
	return;
}

#pragma mark Action methods
- (void)cancel:(id)sender {
	if(isDragging) {
		// Cancels the drag, returning any dragged objects to their original positions and redrawing.
		isDragging = NO;
		NSEnumerator *enumerator = [[[representedObject document] selectedObjects] objectEnumerator];
		CanvasObject *canvasObject;
		NSView *view;
		while(canvasObject = [enumerator nextObject]) {
			view = [canvasObject view];
			[[view superview] setNeedsDisplayInRect:[view frame]];
			[view setFrame:[canvasObject frame]];
			[view setNeedsDisplay:YES];
		}

	} else {
		// Clear the current selection.
		[[representedObject document] setSelectedObjects:[NSSet set]];
	}
}

#pragma mark KVO observation methods
- (void)observeValueForKeyPath:(NSString *)keyPath
					  ofObject:(id)object 
						change:(NSDictionary *)change
					   context:(void *)context
{
	// Observe the represented object for changes and update the view accordingly.
	
	if([keyPath isEqualToString:@"frame"]) {
		// Update the frame of the view and invalidate the old frame.
		[self setFrame:[representedObject frame]];
		[self setNeedsDisplay:YES];
		[[self superview] setNeedsDisplayInRect:NSInsetRect([[change objectForKey:NSKeyValueChangeOldKey] rectValue], -1, -1)]; // Outset by 1 pixel to avoid drawing artifacts while zoomed out
		
		// Some accessibility code will go in here to avoid a complicated refactoring, or even more observation.
		// This notification posting is not ideal, but one's better off being safe than sorry.
		NSAccessibilityPostNotification(self, NSAccessibilityMovedNotification);
		NSAccessibilityPostNotification(self, NSAccessibilityResizedNotification);
	} else if ([[representedObject drawingKeys] containsObject:keyPath]) {
		[self setNeedsDisplay:YES];
	}
}


- (void)stopObservingRepresentedObject {
	// Stops observing changes on representedObject.
	[representedObject removeObserver:self forKeyPath:@"frame"];
	[representedObject removeObserver:self forKeyPath:@"isSelected"];
	
	NSEnumerator *e = [[representedObject drawingKeys] objectEnumerator];
	NSString *key;
	while(key = (NSString *)[e nextObject])
		[representedObject removeObserver:self
							   forKeyPath:key];
	
}

@end
