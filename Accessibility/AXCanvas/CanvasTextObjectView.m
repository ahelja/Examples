/*
 
 File: CanvasTextObjectView.m
 
 Abstract: Canvas text object NSView subclass
 
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


#import "CanvasTextObjectView.h"
#import "CanvasTextObject.h"
#import "CanvasDoc.h"

@implementation CanvasTextObjectView
+ (NSLayoutManager *)sharedLayoutManager {
	static NSLayoutManager *layoutManager = nil;
	if(!layoutManager) {
		layoutManager = [[NSLayoutManager alloc] init];
		
		NSTextContainer *container = [[NSTextContainer alloc] initWithContainerSize:NSMakeSize(1.0e6, 1.0e6)];
		[container setWidthTracksTextView:NO];
		[container setHeightTracksTextView:NO];
		[layoutManager addTextContainer:container];
		
		[container release];
	}
	return layoutManager;
}

- (id)initWithCanvasObject:(CanvasObject *)canvasObject {
	self = [super initWithCanvasObject:canvasObject];
	if(self) {
		fieldEditor = nil;
	}
	
	return self;
}

#pragma mark Drawing methods
- (BOOL)isFlipped {
	return YES;
}

- (void)drawRect:(NSRect)rect {
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;
	
	[super drawRect:rect];
	
	if([[representedTextObject textContent] length] > 0) {
		NSLayoutManager *layoutManager = [CanvasTextObjectView sharedLayoutManager];
		NSTextContainer *container = [[layoutManager textContainers] objectAtIndex:0];
		
		// Create a container inset by one pixel.
		[container setContainerSize:NSMakeSize([self bounds].size.width - 2, [self bounds].size.height - 2)];
		
		NSTextStorage *storage = [representedTextObject storage];
		[storage addLayoutManager:layoutManager];
		
		// Get the drawable glyph range and draw it at the inner edge of the frame.
		NSRange glyphRange = [layoutManager glyphRangeForTextContainer:container];
		if(glyphRange.length > 0)
			[layoutManager drawGlyphsForGlyphRange:glyphRange atPoint:NSMakePoint([self bounds].origin.x + 1,[self bounds].origin.y + 1)];
		[storage removeLayoutManager:layoutManager];		
		
	}
	
}

#pragma mark NSResponder methods
- (void)mouseUp:(NSEvent *)e {
	// Start editing on a double-click.
	if([e clickCount] == 2)
		[self startEditingWithEvent:e];
	else 
		[super mouseUp:e];
}

#pragma mark Action methods
- (void)cancel:(id)sender {
	if(fieldEditor)
		// Cancel the edit in progress.
		[self stopEditing:NO];
	else
		[super cancel:sender];
}

#pragma mark NSText methods
- (void)underline:(id)sender {
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;

	NSMutableDictionary *newAttributes = [[representedTextObject textAttributes] mutableCopy];
	BOOL hasUnderline = ([newAttributes objectForKey:NSUnderlineStyleAttributeName] != nil);
	if(hasUnderline)
		[newAttributes removeObjectForKey:NSUnderlineStyleAttributeName];
	else
		[newAttributes setObject:[NSNumber numberWithInt:NSUnderlineStyleSingle] forKey:NSUnderlineStyleAttributeName];
	[representedTextObject setTextAttributes:newAttributes];
	[newAttributes release];
}

- (void)useStandardLigatures:(id)sender {
	// Turns on standard ligatures. The appropriate constant values are listed in NSAttributedString.h.
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;

	NSMutableDictionary *newAttributes = [[representedTextObject textAttributes] mutableCopy];
	[newAttributes removeObjectForKey:NSLigatureAttributeName];
	[representedTextObject setTextAttributes:newAttributes];
	[newAttributes release];
}

- (void)turnOffLigatures:(id)sender {
	// Turns off ligatures. The appropriate constant values are listed in NSAttributedString.h.
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;

	NSMutableDictionary *newAttributes = [[representedTextObject textAttributes] mutableCopy];
	[newAttributes setObject:[NSNumber numberWithInt:0] forKey:NSLigatureAttributeName];
	[representedTextObject setTextAttributes:newAttributes];
	[newAttributes release];	
}

- (void)useAllLigatures:(id)sender {
	// Turns on all ligatures. The appropriate constant values are listed in NSAttributedString.h.
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;

	NSMutableDictionary *newAttributes = [[representedTextObject textAttributes] mutableCopy];
	[newAttributes setObject:[NSNumber numberWithInt:2] forKey:NSLigatureAttributeName];
	[representedTextObject setTextAttributes:newAttributes];
	[newAttributes release];	
}

#pragma mark Text editing
- (void)textDidEndEditing:(NSNotification *)aNotification {
	// Check if the user left the field before committing changes and end the edit.
	BOOL success = [[[aNotification userInfo] objectForKey:@"NSTextMovement"] intValue] != NSIllegalTextMovement;
	[self stopEditing:success];
}

- (void)startEditingWithEvent:(NSEvent *)anEvent {
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;

	if(fieldEditor)
		return;
		
	// Create a field editor.
	fieldEditor = (NSTextView *)[[self window] fieldEditor:YES forObject:self];
	[fieldEditor setDelegate:self];
	
	[fieldEditor setFrame:NSInsetRect([self bounds], 1, 1)];
	[fieldEditor setMinSize:NSInsetRect([self bounds], 1, 1).size];
	[fieldEditor setMaxSize:NSInsetRect([self bounds], 1, 1).size];
	[[fieldEditor textContainer] setContainerSize:NSMakeSize(NSWidth(NSInsetRect([self bounds], 1, 1)), 1.0e6)];
	[fieldEditor setSelectedRange:NSMakeRange(0, [[representedTextObject textContent] length])];
	[fieldEditor setTextContainerInset:NSZeroSize];
	
	[[fieldEditor textStorage] setAttributedString:[representedTextObject storage]];
	[fieldEditor setRichText:NO];
	[fieldEditor setAllowsDocumentBackgroundColorChange:NO];
	[fieldEditor setBackgroundColor:[NSColor whiteColor]];
	[fieldEditor setTypingAttributes:[representedTextObject textAttributes]];
	
	[[fieldEditor textContainer] setWidthTracksTextView:NO];
	[[fieldEditor textContainer] setHeightTracksTextView:NO];
	[fieldEditor setHorizontallyResizable:NO];
	[fieldEditor setVerticallyResizable:YES];
	
	// Activate the field editor.
	[self addSubview:fieldEditor];
	[[self window] makeFirstResponder:fieldEditor];
	
}

- (void)stopEditing:(BOOL)shouldApplyChanges {
	// End an in progress edit.
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;

	if(fieldEditor) {
		[fieldEditor removeFromSuperview];
		
		if(shouldApplyChanges) {
			// Save changes back to model object.
			if([[fieldEditor textStorage] length] > 0)
				[representedTextObject setTextAttributes:[[fieldEditor textStorage] attributesAtIndex:0 effectiveRange:nil]];
			
			[representedTextObject setTextContent:[fieldEditor string]];
		}
		
		[[self window] makeFirstResponder:self];
		[[representedObject document] selectCanvasObject:representedObject];
		[self setNeedsDisplay:YES];
		fieldEditor = nil;
	}
}	

@end
