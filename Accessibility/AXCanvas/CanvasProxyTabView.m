/*
 
 File: CanvasProxyTabView.m
 
 Abstract: Tab view UIElement proxy
 
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


#import "CanvasProxyTabView.h"
#import "AccessibleViewKVCUtilities.h"
#import "CanvasInspectorController.h"

// Acts as the accessibile version of the "tab view" in the Inspector window,
// which is really composed of a set of matrix buttons and some code that twiddles
// the window content (see CanvasInspectorController). 
@implementation CanvasProxyTabView
- (id)initWithButtons:(NSMatrix *)aMatrix contentBox:(NSBox *)aBox parentView:(NSView *)aView {
	// Designated initializer.
	self = [super init];
	if(self) {
		buttonMatrix = aMatrix;
		contentBox   = aBox;
		parentView   = aView;
		
		// Sets the parent of the content to the tab view.
		[contentBox accessibilitySetOverrideValue:self forAttribute:NSAccessibilityParentAttribute];
		
		// Sets the parent of the tab matrix to the tab view.
		[buttonMatrix accessibilitySetOverrideValue:self forAttribute:NSAccessibilityParentAttribute];
		
		// Sets the parent of the the tab buttons to the tab view and changes their role
		// to match those of normal tab buttons.
		NSEnumerator *e = [[buttonMatrix cells] objectEnumerator];
		NSButton *button;
		while(button = (NSButton *)[e nextObject]) {
			[button accessibilitySetOverrideValue:self forAttribute:NSAccessibilityParentAttribute];
			[button accessibilitySetOverrideValue:NSAccessibilityRadioButtonRole forAttribute:NSAccessibilityRoleAttribute];
			[button accessibilitySetOverrideValue:NSAccessibilityRoleDescription(NSAccessibilityRadioButtonRole, nil) forAttribute:NSAccessibilityRoleDescriptionAttribute];
		}
		
		// Prepares to receive notifications for tab value change.
		[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self
											                             selector:@selector(tabChanged:)
																			 name:(NSString *)CanvasInspectorTabChangedNotification
																		   object:buttonMatrix];
	}
	return self;
}

- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
	[super dealloc];
}

#pragma mark NSAccessibility methods
- (NSArray *)accessibilityAttributeNames {
	static NSArray *attributes = nil;
	if(attributes == nil) {
		attributes = [[NSArray arrayWithObjects:
			NSAccessibilityRoleAttribute,
			NSAccessibilityRoleDescriptionAttribute,
			NSAccessibilityParentAttribute,
			NSAccessibilityContentsAttribute,
			NSAccessibilityChildrenAttribute,
			NSAccessibilityWindowAttribute,
			NSAccessibilityPositionAttribute,
			NSAccessibilitySizeAttribute,
			NSAccessibilityFocusedAttribute,
			NSAccessibilityTabsAttribute,
			NSAccessibilityTopLevelUIElementAttribute,
			NSAccessibilityValueAttribute,
			nil] retain];
	}
	
	return attributes;
}

#pragma mark Attribute accessors
- (NSString *)caxRoleAttribute {
	return NSAccessibilityTabGroupRole;
}

- (BOOL)caxIsRoleAttributeSettable {
	return NO;
}

- (NSString *)caxRoleDescriptionAttribute {
	return NSAccessibilityRoleDescriptionForUIElement(self);
}

- (BOOL)caxIsRoleDescriptionAttributeSettable {
	return NO;
}

- (id)caxParentAttribute {
	return NSAccessibilityUnignoredAncestor(parentView);
}

- (BOOL)caxIsParentAttributeSettable {
	return NO;
}

- (NSArray *)caxContentsAttribute {
	return NSAccessibilityUnignoredChildrenForOnlyChild(contentBox);
}

- (BOOL)caxIsContentsAttributeSettable {
	return NO;
}

- (NSArray *)caxChildrenAttribute {
	// Children encompass all subelements of this one, so the tabs are added.
	NSMutableArray *kids = [[[self caxContentsAttribute] mutableCopy] autorelease];
	[kids addObjectsFromArray:[self caxTabsAttribute]];
	return kids;
}

- (BOOL)caxIsChildrenAttributeSettable {
	return NO;
}

- (NSWindow *)caxWindowAttribute {
	return [parentView window];
}

- (BOOL)caxIsWindowAttributeSettable {
	return NO;
}

- (NSValue *)caxPositionAttribute {
	return [NSValue valueWithPoint:[self caxRectHelper].origin];
}

- (BOOL)caxIsPositionAttributeSettable {
	return NO;
}

- (NSValue *)caxSizeAttribute {
	// Return size of scroll area
	return [NSValue valueWithSize:[self caxRectHelper].size];
}

- (BOOL)caxIsSizeAttributeSettable {
	return NO;
}

- (NSNumber *)caxFocusedAttribute {
	return [NSNumber numberWithBool:([[parentView window] firstResponder] == buttonMatrix)];
}

- (BOOL)caxIsFocusedAttributeSettable {
	return YES;
}

- (void)caxSetFocusedAttribute:(NSNumber *)aNumber {
	if([aNumber boolValue])
		[[parentView window] makeFirstResponder:buttonMatrix];
}

- (NSArray *)caxTabsAttribute {
	 return NSAccessibilityUnignoredChildren([buttonMatrix cells]);
}

- (BOOL)caxIsTabsAttributeSettable {
	return NO;
}

- (id)caxTopLevelUIElementAttribute {
	// Return the top level UI element to which the scroll area belongs
	return [parentView window];
}

- (BOOL)caxIsTopLevelUIElementAttributeSettable {
	return NO;
}

- (id)caxValueAttribute {
	return NSAccessibilityUnignoredDescendant([buttonMatrix selectedCell]);
}

- (BOOL)caxIsValueAttributeSettable {
	return NO;
}

#pragma mark Geometry utilities

- (NSRect)caxRectHelperForView:(NSView *)view {
	// Converts from view coordinates to screen coordinates.
	NSView *parent = [view superview];
	NSRect frame = [view frame];
	NSRect frameInWindow = (parent != nil) ? [parent convertRect:frame toView:nil] : frame;
	frameInWindow.origin = [[view window] convertBaseToScreen:frameInWindow.origin];
	return frameInWindow;
}

- (NSRect)caxRectHelper {
	// Finds the total frame of the receiver.
	NSRect contentFrame = [self caxRectHelperForView:contentBox];
	NSRect buttonFrame = [self caxRectHelperForView:buttonMatrix];
	
	return NSUnionRect(contentFrame, buttonFrame);
}

#pragma mark Notification observers
- (void)tabChanged:(NSNotification *)aNotification {
	NSAccessibilityPostNotification(self, NSAccessibilityValueChangedNotification);
}

@end
