/*
 
 File: CanvasView.m
 
 Abstract: Canvas NSView subclass
 
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


#import "CanvasView.h"
#import "CanvasObjectView.h"
#import "CanvasTextObject.h"
#import "CanvasDoc.h"
#import "CanvasInspectorController.h"

@implementation CanvasView
- (void)awakeFromNib {
	scaleFactor = 1.0;
	[self setScaleFactor:1.0];
	isMouseDown = NO;
	selectionStartPoint = NSZeroPoint;
	selectionRect = NSZeroRect;	
}

#pragma mark Attribute accessors
- (float)scaleFactor {
	return scaleFactor;
}

- (void)setScaleFactor:(float)value {
	if(value != scaleFactor) {
		scaleFactor = value;
		
		// To zoom, we preserve the bounds size, but change the frame size.
		// Cocoa resizes the bounds when we change the frame, so the bounds need to be
		// preserved until the end.
		NSSize frameSize, boundsSize;
		frameSize = boundsSize = [self bounds].size;
		
		frameSize.height *= scaleFactor;
		frameSize.width *= scaleFactor;
		
		[self setFrameSize:frameSize];
		[self setBoundsSize:boundsSize];
		[self setNeedsDisplay:YES];
		[[self superview] setNeedsDisplay:YES];
	}
}

#pragma mark Drawing methods
- (BOOL)isFlipped {
	// Use the top-left coordinate system more natural to users.
	return YES;
}

- (void)drawRect:(NSRect)rect {
	[[document backgroundColor] set];
	NSRectFill([self bounds]);
	
	// Draw selection rectangle.
	if(isMouseDown)
	{
		NSBezierPath *path = [NSBezierPath bezierPathWithRect:[self currentSelectionRect]];
		NSColor *selectionColor = [NSColor colorWithDeviceWhite:0.75 alpha:1.0];
		[[selectionColor colorWithAlphaComponent:0.2] set];		
		[path fill];
		[selectionColor set];
		[path setLineWidth:0.5/scaleFactor];
		[path stroke];
	}	
}

#pragma mark NSResponder methods
- (BOOL)acceptsFirstResponder {
	// Allow this view to have the focus.
	return YES;
}

- (void)keyDown:(NSEvent *)e {
	// Process key events of interest.
	NSString *characters = [e charactersIgnoringModifiers];
	unichar keyChar = ([characters length] ? [characters characterAtIndex: 0] : -1);

	// Move objects by natural increments: screen pixels, not canvas pixels.
	// If shift is down, move ten pixels at a time.
	float movement = (([e modifierFlags] & NSShiftKeyMask) ? 10 : 1) / [self scaleFactor];
	switch(keyChar) {
		case NSUpArrowFunctionKey:
			if([[document selectedObjects] count] > 0)
				[[document selectedObjects] makeObjectsPerformSelector:@selector(setRelativePosition:) withObject:[NSValue valueWithPoint:NSMakePoint(0, -movement)]];
			else
				NSBeep();
			break;
		case NSDownArrowFunctionKey:
			if([[document selectedObjects] count] > 0)
				[[document selectedObjects] makeObjectsPerformSelector:@selector(setRelativePosition:) withObject:[NSValue valueWithPoint:NSMakePoint(0, movement)]];
			else
				NSBeep();
			break;
		case NSLeftArrowFunctionKey:
			if([[document selectedObjects] count] > 0)
				[[document selectedObjects] makeObjectsPerformSelector:@selector(setRelativePosition:) withObject:[NSValue valueWithPoint:NSMakePoint(-movement, 0)]];
			else
				NSBeep();
			break;
		case NSRightArrowFunctionKey:
			if([[document selectedObjects] count] > 0)
				[[document selectedObjects] makeObjectsPerformSelector:@selector(setRelativePosition:) withObject:[NSValue valueWithPoint:NSMakePoint(movement, 0)]];
			else
				NSBeep();
			break;
		case NSCarriageReturnCharacter: // Return key
			if([[document selectedObjects] count] == 1)
				[[(CanvasTextObject *)[[document selectedObjects] anyObject] view] startEditingWithEvent:e];
			else
				NSBeep();
			break;
		case NSTabCharacter:
			if([[document selectedObjects] count] > 0) {
				// Select the next canvas object, wrapping if necessary
				CanvasObject *selectedObject = [[document selectedObjects] anyObject];
				NSArray *canvasObjects = [document canvasObjects];
				unsigned int nextIndex = ([canvasObjects indexOfObject:selectedObject] + 1) % [canvasObjects count];
				[document setSelectedObjects:[NSSet setWithObject:[canvasObjects objectAtIndex:nextIndex]]];
			} else if([[document canvasObjects] count] > 0)
				[document setSelectedObjects:[NSSet setWithObject:[[document canvasObjects] objectAtIndex:0]]];
			else
				NSBeep();
			break;
		case NSBackTabCharacter:
			if([[document selectedObjects] count] > 0) {
				// Select the previous canvas object, wrapping if necessary
				CanvasObject *selectedObject = [[document selectedObjects] anyObject];
				NSArray *canvasObjects = [document canvasObjects];
				
				int nextIndex = ([canvasObjects indexOfObject:selectedObject] - 1);
				if(nextIndex < 0)
					nextIndex = [canvasObjects count] - 1;
				
				[document setSelectedObjects:[NSSet setWithObject:[canvasObjects objectAtIndex:nextIndex]]];
			} else if([[document canvasObjects] count] > 0)
				[document setSelectedObjects:[NSSet setWithObject:[[document canvasObjects] objectAtIndex:[[document canvasObjects] count] - 1]]];
			else
				NSBeep();
			break;
		case NSBackspaceCharacter:
		case NSDeleteCharacter:
			[self delete:nil];
			break;
		default:
			// Pass to superclass implementation.
			[super keyDown:e];
			break;
	}
}

- (void)mouseDown:(NSEvent *)e {
	// If the mouse is clicked on empty canvas area, unselect all objects.
	if([e clickCount] == 1)
		[document setSelectedObjects:[NSSet set]];
	
	// Store mouse down state for selection rect.
	isMouseDown = YES;
	selectionStartPoint = selectionRect.origin = [self convertPoint:[e locationInWindow] fromView:nil];	
}

- (void)mouseDragged:(NSEvent *)e {
	// Grab the current selection rectangle.
	NSRect currentSelectionRect = [self currentSelectionRect];
	
	// Redraw selection rect.
	[self setNeedsDisplayInRect:NSInsetRect(selectionRect, -1, -1)]; // Outset by 1 pixel to avoid drawing artifacts while zoomed out
	[self setNeedsDisplayInRect:NSInsetRect(currentSelectionRect, -1, -1)];
	
	// Scroll if the mouse has left the clip view.
	if(!NSPointInRect([e locationInWindow], [self convertRect:[self visibleRect] toView:nil]))
		[self autoscroll:e];
	
	// Select any objects which overlap the selection rectangle.
	NSEnumerator *enumerator = [[document canvasObjects] objectEnumerator];
	CanvasObject *canvasObject;
	NSMutableSet *selectedObjects = [[[NSMutableSet alloc] init] autorelease];
	while(canvasObject = [enumerator nextObject]) {
		if(NSIntersectsRect(currentSelectionRect, [canvasObject frame])) {
			[selectedObjects addObject:canvasObject];
		}
	}
	[document setSelectedObjects:selectedObjects];
	
	// Store the current selection rect for next time.
	selectionRect = currentSelectionRect;
	
}

- (void)mouseUp:(NSEvent *)e {
	
	// End selection and invalidate selection rectangle drawing region.
	isMouseDown = NO;
	if((selectionStartPoint.x != -1)  && (selectionStartPoint.y != -1))
		[self setNeedsDisplayInRect:selectionRect];
	
	selectionStartPoint = NSMakePoint( -1, -1 );
	selectionRect = NSZeroRect;
}

- (BOOL)performKeyEquivalent:(NSEvent *)e {
	// Handle Cmd-1, Cmd-2, Cmd-3 and Cmd-4 keypresses to activate Inspector tabs.
	NSString *characters = [e charactersIgnoringModifiers];
	unichar keyChar = ([characters length] ? [characters characterAtIndex: 0] : -1);
	BOOL retVal = NO;
	switch(keyChar) {
		case '1':
		case '2':
		case '3':
		case '4':
			if(!([e modifierFlags] & (NSShiftKeyMask | NSControlKeyMask | NSAlternateKeyMask))) {
				[[CanvasInspectorController sharedController] showWindow:nil];
				[[CanvasInspectorController sharedController] selectTabWithTag:(keyChar - '1')];				
			}
			retVal = YES;
			break;
		default:
			retVal = [super performKeyEquivalent:e];
	}
	
	return retVal;
}

#pragma mark Action methods
- (IBAction)addNewObject:(id)sender {
	// Create a new text object and ad it to the document.
	CanvasTextObject *canvasObject = [[[CanvasTextObject alloc] initWithDocument:document] autorelease];
	[canvasObject setTextContent:@""];
	[document addObjectInCanvasObjects:canvasObject];
	[[canvasObject view] startEditingWithEvent:nil];
}

- (void)changeAttributes:(id)sender {
	// Update the text attributes of any selected objects.
	NSEnumerator *e = [[document selectedObjects] objectEnumerator];
	CanvasObject *canvasObject;
	while(canvasObject = (CanvasObject *)[e nextObject])
		if([canvasObject isKindOfClass:[CanvasTextObject class]])
			[(CanvasTextObject *)canvasObject setTextAttributes:[sender convertAttributes:[(CanvasTextObject *)canvasObject textAttributes]]];
	
}

- (void)changeDocumentBackgroundColor:(id)sender {
	// Update the background color of any selected objects.
	[[document selectedObjects] makeObjectsPerformSelector:@selector(setFillColor:) withObject:[sender color]];
}

- (void)changeFont:(id)sender {
	// Update the font of any selected objects.
	NSEnumerator *e = [[document selectedObjects] objectEnumerator];
	CanvasObject *canvasObject;
	while(canvasObject = (CanvasObject *)[e nextObject]) {
		if([canvasObject isKindOfClass:[CanvasTextObject class]]) {
			NSMutableDictionary *attributes = [[[(CanvasTextObject *)canvasObject textAttributes] mutableCopy] autorelease];
			NSFont *font = [attributes objectForKey:NSFontAttributeName];
			
			[attributes setObject:[sender convertFont:font] forKey:NSFontAttributeName];
			[(CanvasTextObject *)canvasObject setTextAttributes:attributes];					
		}
	}
}

- (void)delete:(id)sender {
	// Delete any selected objects.
//	NSEnumerator *enumerator = [[document selectedObjects] objectEnumerator];
//	CanvasObject *canvasObject;
//	while(canvasObject = [enumerator nextObject])
//		[document removeObjectFromCanvasObjects:canvasObject];	
	[[document mutableArrayValueForKey:@"canvasObjects"] removeObjectsInArray:[document selectedObjectsArray]];
}

- (void)selectAll:(id)sender {
	// Select all canvas objects.
	[document setSelectedObjects:[NSSet setWithArray:[document canvasObjects]]];
}

- (void)moveBackward:(id)sender {
	// Move the selected object backward (reduce z-order), resort subviews
	// and redraw.
	NSEnumerator *enumerator = [[document selectedObjects] objectEnumerator];
	CanvasObject *canvasObject;
	while(canvasObject = [enumerator nextObject])
		[document moveCanvasObjectBackward:canvasObject];
	
	[self sortSubviewsUsingFunction:&CanvasObjectSortByZOrder context:document];
	[self setNeedsDisplay:YES];
}

- (void)moveForward:(id)sender {
	// Move the selected object forward (increase z-order), resort subviews
	// and redraw.
	NSEnumerator *enumerator = [[document selectedObjects] objectEnumerator];
	CanvasObject *canvasObject;
	while(canvasObject = [enumerator nextObject])
		[document moveCanvasObjectForward:canvasObject];
	
	[self sortSubviewsUsingFunction:&CanvasObjectSortByZOrder context:document];
	[self setNeedsDisplay:YES];	
}

#pragma mark Selection helper methods
- (NSRect)currentSelectionRect {
	// Calculates the current selection rect, anchoring one vertex of the rectangle
	// at the original click point.
	NSPoint mouseLocation = [self convertPoint:[[self window] mouseLocationOutsideOfEventStream] fromView:nil];
	NSRect newSelectionRect = NSMakeRect(selectionStartPoint.x, selectionStartPoint.y, mouseLocation.x - selectionStartPoint.x, mouseLocation.y - selectionStartPoint.y);
	
	if(newSelectionRect.size.width < 0)
	{
		newSelectionRect.origin.x += newSelectionRect.size.width;
		newSelectionRect.size.width = -newSelectionRect.size.width;
	}
	
	if(newSelectionRect.size.height < 0)
	{
		newSelectionRect.origin.y += newSelectionRect.size.height;
		newSelectionRect.size.height = -newSelectionRect.size.height;
	}
	
	return NSOffsetRect(newSelectionRect, 0.5, 0.5);
}

#pragma mark User interface validation
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
	if([menuItem action] == @selector(delete:)) {
		return ([[document selectedObjects] count] > 0);
	} else
		return YES;
}

- (BOOL)validateToolbarItem:(NSToolbarItem *)toolbarItem {
	if([toolbarItem action] == @selector(delete:)) {
		return ([[document selectedObjects] count] > 0);
	} else if([toolbarItem action] == @selector(moveForward:)) {
		// Only allow objects that aren't already at the front to advance.
		BOOL hasSelection = ([[document selectedObjects] count] > 0);
		BOOL isFrontmost = NO;

		NSEnumerator *enumerator = [[document selectedObjects] objectEnumerator];
		CanvasObject *canvasObject;
		while((canvasObject = [enumerator nextObject]) && !isFrontmost)
			isFrontmost = isFrontmost | ([[document canvasObjects] indexOfObject:canvasObject] == ([document countOfCanvasObjects] - 1));
		
		return hasSelection && !isFrontmost;
	} else if([toolbarItem action] == @selector(moveBackward:)) {
		// Only allow objects that aren't already at the back to move backward.
		BOOL hasSelection = ([[document selectedObjects] count] > 0);
		BOOL isRearmost = NO;
		
		NSEnumerator *enumerator = [[document selectedObjects] objectEnumerator];
		CanvasObject *canvasObject;
		while((canvasObject = [enumerator nextObject]) && !isRearmost)
			isRearmost = isRearmost | ([[document canvasObjects] indexOfObject:canvasObject] == 0);
		
		return hasSelection && !isRearmost;
	} else
		return YES;
}

#pragma mark KVO observation methods
- (void)observeValueForKeyPath:(NSString *)keyPath
					  ofObject:(id)object
						change:(NSDictionary *)change
					   context:(void *)context {
	
	if([keyPath isEqualToString:@"canvasObjects"]) {
		// Update subviews for any objects that have changed in the document.
		
		NSArray *objects;
		NSEnumerator *e;
		CanvasObject *canvasObject;
		switch([(NSNumber *)[change objectForKey:NSKeyValueChangeKindKey] intValue]) {
			case NSKeyValueChangeInsertion:
				// Add the view for an inserted object.
				objects = [change objectForKey:NSKeyValueChangeNewKey];
				e = [objects objectEnumerator];
				while(canvasObject = (CanvasObject *)[e nextObject]) {
					[self addSubview:[canvasObject view]];
				}
					
					break;
				
			case NSKeyValueChangeRemoval:
				// Remove the view for a removed object.
				objects = [change objectForKey:NSKeyValueChangeOldKey];
				e = [objects objectEnumerator];
				while(canvasObject = (CanvasObject *)[e nextObject])
					[[canvasObject view] removeFromSuperview];
					break;
				
			case NSKeyValueChangeReplacement:
				// Swap views for a replaced object.
				objects = [change objectForKey:NSKeyValueChangeOldKey];
				e = [objects objectEnumerator];
				while(canvasObject = (CanvasObject *)[e nextObject])
					[[canvasObject view] removeFromSuperview];
					
					objects = [change objectForKey:NSKeyValueChangeNewKey];
				e = [objects objectEnumerator];
				while(canvasObject = (CanvasObject *)[e nextObject])
					[self addSubview:[canvasObject view]];
					break;
				
			default:
				break;
		}
	} else if([keyPath isEqualToString:@"size"]) {
		// Update the receiver's size to match the document's size.
		NSSize frameSize, boundsSize;
		frameSize = boundsSize = [document size];
		
		frameSize.height *= scaleFactor;
		frameSize.width *= scaleFactor;
		
		[self setFrameSize:frameSize];
		[self setBoundsSize:boundsSize];
		[self setNeedsDisplay:YES];
		
	} else if([keyPath isEqualToString:@"backgroundColor"]) {
		// Redraw the background of the document.
		[self setNeedsDisplay:YES];
	} else if([keyPath isEqualToString:@"selectedObjects"]) {
		// Update the list of selected fonts.
		
		NSEnumerator *enumerator = [[document selectedObjects] objectEnumerator];
		CanvasObject *canvasObject;
		BOOL isMultiple = ([[document selectedObjects] count] > 1);
		while(canvasObject = (CanvasObject *)[enumerator nextObject]) {
			if(![canvasObject isKindOfClass:[CanvasTextObject class]])
				continue;
			
			[[NSFontManager sharedFontManager] setSelectedFont:[[(CanvasTextObject *)canvasObject textAttributes] objectForKey:NSFontAttributeName] isMultiple:isMultiple];
			[[NSFontManager sharedFontManager] setSelectedAttributes:[(CanvasTextObject *)canvasObject textAttributes] isMultiple:isMultiple];
		}		
	}
}

@end
