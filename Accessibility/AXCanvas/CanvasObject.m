/*
 
 File: CanvasObject.m
 
 Abstract: Canvas object model object
 
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

#import "CanvasObject.h"
#import "CanvasDoc.h"
#import "CanvasObjectView.h"

// Archiving keys
static NSString *CanvasObjectFrameKey       = @"CanvasObjectFrame";
static NSString *CanvasObjectHasFillKey     = @"CanvasObjectHasFill";
static NSString *CanvasObjectFillColorKey   = @"CanvasObjectFillColor";
static NSString *CanvasObjectHasStrokeKey   = @"CanvasObjectHasStroke";
static NSString *CanvasObjectStrokeColorKey = @"CanvasObjectStrokeColor";
static NSString *CanvasObjectStrokeWidthKey = @"CanvasObjectStrokeWidth";

@implementation CanvasObject

+ (void)initialize {
	// Sets up key dependence hierarchy.
	[self setKeys:[NSArray arrayWithObject:@"frame"] triggerChangeNotificationsForDependentKey:@"position"];
	[self setKeys:[NSArray arrayWithObject:@"frame"] triggerChangeNotificationsForDependentKey:@"size"];
		
	[self setKeys:[NSArray arrayWithObjects:@"position", nil] triggerChangeNotificationsForDependentKey:@"positionX"];
	[self setKeys:[NSArray arrayWithObjects:@"position", nil] triggerChangeNotificationsForDependentKey:@"positionY"];
	[self setKeys:[NSArray arrayWithObjects:@"size", nil] triggerChangeNotificationsForDependentKey:@"frameWidth"];
	[self setKeys:[NSArray arrayWithObjects:@"size", nil] triggerChangeNotificationsForDependentKey:@"frameHeight"];

}

- (id)init {
	self = [super init];
	
	if(self) {
		frame = NSMakeRect(0, 0, 50, 50);
		view = nil; document = nil;
		
		hasFill = NO;
		fillColor = [[NSColor whiteColor] retain];
		
		hasStroke = YES;
		strokeWidth = 1.0;
		strokeColor = [[NSColor blackColor] retain];
		
		isSelected = NO;
	}
	
	return self;
}

- (id)initWithDocument:(CanvasDoc *)aDocument {
	// Designated initializer.
	self = [self init];
	if(self) {
		[self setDocument:aDocument];
	}
	
	return self;
}

- (void)dealloc {
	[fillColor release];
	[strokeColor release];
	[view release];
	
	[super dealloc];
}

#pragma mark NSCoding methods
- (id)initWithCoder:(NSCoder *)decoder {
	// Loads the canvas object from a (keyed) archive.
	self = [self init];
	if(self) {
		frame = [decoder decodeRectForKey:CanvasObjectFrameKey];
		hasFill = [decoder decodeBoolForKey:CanvasObjectHasFillKey];
		fillColor = [[decoder decodeObjectForKey:CanvasObjectFillColorKey] retain];
		hasStroke = [decoder decodeBoolForKey:CanvasObjectHasStrokeKey];
		strokeColor = [[decoder decodeObjectForKey:CanvasObjectStrokeColorKey] retain];
		strokeWidth = [decoder decodeFloatForKey:CanvasObjectStrokeWidthKey];
	}
	
	return self;
}

- (void)encodeWithCoder:(NSCoder *)encoder {
	// Saves the canvas object into a (keyed) archive.
	[encoder encodeRect:frame forKey:CanvasObjectFrameKey];
	[encoder encodeBool:hasFill forKey:CanvasObjectHasFillKey];
	[encoder encodeObject:fillColor forKey:CanvasObjectFillColorKey];
	[encoder encodeBool:hasStroke forKey:CanvasObjectHasStrokeKey];
	[encoder encodeObject:strokeColor forKey:CanvasObjectStrokeColorKey];
	[encoder encodeFloat:strokeWidth forKey:CanvasObjectStrokeWidthKey];
}

#pragma mark Undo support (CanvasManagedObject)
+ (NSArray *)undoableKeys {
	// Returns an array of keys whose modification should be undoable.
	// See CanvasManagedUndoController for further documentation.
	static NSArray *undoableKeys = nil;
	if(!undoableKeys)
		undoableKeys = [[NSArray arrayWithObjects:@"frame", @"hasFill", @"fillColor", @"hasStroke", @"strokeColor", @"strokeWidth", nil] retain];

	return [[undoableKeys retain] autorelease];
}

- (void)setValue:(id)aValue forUndoableKeyPath:(NSString *)keyPath {
	// Sets the receiver's keyPath to aValue. See CanvasManagedUndoController for
	// further documentation.
	[self setValue:aValue forKeyPath:keyPath];
}

- (void)undoControllerWillUnregister:(CanvasManagedUndoController *)undoController {
	// Removes any observers on this class and stops observing other classes.
	// See CanvasManagedUndoController for further documentation.

	if(view)
		[view stopObservingRepresentedObject];
}

#pragma mark CanvasObjectView methods
- (NSArray *)drawingKeys {
	// Returns an array of keys whose modification should trigger a view update.
	// See CanvasObjectView's -setRepresentedObject: for documentation.
	static NSArray *keys = nil;
	if(!keys) {
		keys = [[[[self class] undoableKeys] arrayByAddingObject:@"isSelected"] retain];
	}
	return [[keys retain] autorelease];
}


- (void)prepareView {
	// Creates a new view for the receiver, if necessary.
	if(!view) {
		view = [(CanvasObjectView *)[[self viewClass] alloc] initWithCanvasObject:self];
	}
}

- (CanvasObjectView *)view {
	// Returns the view for the receiver.
	if(!view)
		[self prepareView];
	
	return [[view retain] autorelease];
}

- (Class)viewClass {
	// Returns the CanvasObjectView subclass to be used to draw the receiver. 
	return [CanvasObjectView class];
}

- (CanvasDoc *)document {
	return document;
}

- (void)setDocument:(CanvasDoc *)aDocument {
	if(aDocument != document)
		document = aDocument;
}

- (NSRect)frame {
	return frame;
}

- (void)setFrame:(NSRect)aRect {
	if(!NSEqualRects(aRect, frame))
		frame = aRect;
}

- (NSPoint)position {
	return frame.origin;
}

- (void)setPosition:(NSPoint)aPoint {
	if(!NSEqualPoints(aPoint, frame.origin)) {
		NSRect newFrame = frame;
		newFrame.origin = aPoint;
		[self setFrame:newFrame];
	}
}

- (void)setRelativePosition:(NSValue *)aValue {
	// Offsets the receiver's frame by the point value aValue.
	if(strcmp([aValue objCType], @encode(NSPoint)) != 0)
		return;

	NSPoint position = [self position];
	NSPoint aPoint = [aValue pointValue];
	position.x += aPoint.x;
	position.y += aPoint.y;
	[self setPosition:position];
}

- (float)positionX {
	return frame.origin.x;
}

- (void)setPositionX:(float)aFloat {
	NSPoint newOrigin = frame.origin;
	newOrigin.x = aFloat;
	[self setPosition:newOrigin];
}

- (float)positionY {
	return frame.origin.y;
}

- (void)setPositionY:(float)aFloat {
	NSPoint newOrigin = frame.origin;
	newOrigin.y = aFloat;
	[self setPosition:newOrigin];}

- (NSSize)size {
	return frame.size;
}

- (void)setSize:(NSSize)aSize {
	if(!NSEqualSizes(frame.size, aSize)) {
		NSRect newFrame = frame;
		newFrame.size = aSize;
		[self setFrame:newFrame];
	}
}

- (float)frameWidth {
	return frame.size.width;
}

- (void)setFrameWidth:(float)aFloat {
	NSSize newSize = frame.size;
	newSize.width = aFloat;
	[self setSize:newSize];
}

- (float)frameHeight {
	return frame.size.height;
}

- (void)setFrameHeight:(float)aFloat {
	NSSize newSize = frame.size;
	newSize.height = aFloat;
	[self setSize:newSize];
}

- (BOOL)hasFill {
    return hasFill;
}

- (void)setHasFill:(BOOL)value {
    if (hasFill != value) {
        hasFill = value;
    }
}

- (NSColor *)fillColor {
    return [[fillColor retain] autorelease];
}

- (void)setFillColor:(NSColor *)value {
    if (fillColor != value) {
        [fillColor release];
        fillColor = [value copy];
		
		if(![self hasFill])
			[self setHasFill:YES];
    }
}

- (BOOL)hasStroke {
    return hasStroke;
}

- (void)setHasStroke:(BOOL)value {
    if (hasStroke != value) {
        hasStroke = value;
    }
}

- (float)strokeWidth {
    return strokeWidth;
}

- (void)setStrokeWidth:(float)value {
    if (strokeWidth != value) {
		strokeWidth = value;
		
		if(![self hasStroke])
			[self setHasStroke:YES];
    }
}

- (NSColor *)strokeColor {
    return [[strokeColor retain] autorelease];
}

- (void)setStrokeColor:(NSColor *)value {
    if (strokeColor != value) {
        [strokeColor release];
        strokeColor = [value copy];
		
		if(![self hasStroke])
			[self setHasStroke:YES];
    }
}

- (BOOL)isSelected {
	// Returns whether the current object is selected.
	return isSelected;
}

#pragma mark Private methods
- (void)setIsSelected:(BOOL)flag {
	// Updates the receiver's internal selection state.
	isSelected = flag;
}

@end
