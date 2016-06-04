/*
 
 File: CanvasTextObject.m
 
 Abstract: Canvas text model object
 
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


#import "CanvasTextObject.h"
#import "CanvasTextObjectView.h"

@implementation CanvasTextObject
- (id)init {
	self = [super init];
	
	if(self) {
		storage = [[NSTextStorage alloc] initWithString:@""];
		textAttributes = [[NSDictionary dictionaryWithObjectsAndKeys:
			[NSColor blackColor], NSForegroundColorAttributeName,
			[NSFont systemFontOfSize:[NSFont smallSystemFontSize]], NSFontAttributeName,
			nil] retain];
	}
	
	return self;
}

- (void)dealloc {
	[storage release];
	[textAttributes release];
	
	[super dealloc];
}


#pragma mark NSCoding methods
- (id)initWithCoder:(NSCoder *)decoder {
	// Loads the canvas object from a (keyed) archive.
	self = [super initWithCoder:decoder];
	if(self) {
		storage = [[decoder decodeObjectForKey:@"COStorage"] retain];
		textAttributes = [[decoder decodeObjectForKey:@"COTextAttributes"] retain];
	}
	
	return self;
}

- (void)encodeWithCoder:(NSCoder *)encoder {
	// Saves the canvas object into a (keyed) archive.
	[super encodeWithCoder:encoder];
	[encoder encodeObject:storage forKey:@"COStorage"];
	[encoder encodeObject:textAttributes forKey:@"COTextAttributes"];
}

#pragma mark Managed undo support (CanvasManagedObject)
+ (NSArray *)undoableKeys {
	// Returns an array of keys whose modification should be undoable.
	// See CanvasManagedUndoController for further documentation.
	static NSArray *undoableKeys = nil;
	if(!undoableKeys)
		undoableKeys = [[[super undoableKeys] arrayByAddingObjectsFromArray:[NSArray arrayWithObjects:@"textContent", @"textAttributes", nil]] retain];
	
	return undoableKeys;
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

- (Class) viewClass {
	// Returns the CanvasObjectView subclass to be used to draw the receiver. 
	return [CanvasTextObjectView class];
}

#pragma mark Attribute accessors
- (NSString *)textContent {

	// The string returned by the NSTextStorage needs to be copied instead of simply
	// retained, as NSTextStorage's -string returns a retained copy of the storage's
	// backing store instead of a separate copy. KVO's NSKeyValueObservingOptionOld
	// option will not function if the old value can not be reliably retained by the
	// observer.
	return [[[storage string] copy] autorelease];
}

- (void)setTextContent:(NSString *)aString {
	if(![[self textContent] isEqualToString:aString]) {
		BOOL contentWasEmpty = ([storage length] == 0);
		[storage replaceCharactersInRange:NSMakeRange(0, [storage length]) withString:aString];

		// Attributes must be manually set if the string was previously empty, as empty
		// attributed strings have no attributes.
		if(contentWasEmpty)
			[storage setAttributes:textAttributes range:NSMakeRange(0, [storage length])];
	}
}

- (NSDictionary *)textAttributes {
	return [[textAttributes retain] autorelease];
}

- (void)setTextAttributes:(NSDictionary *)aDictionary {
	if(![[self textAttributes] isEqual:aDictionary]) {
		[storage setAttributes:aDictionary range:NSMakeRange(0, [storage length])];

		[textAttributes release];
		textAttributes = [aDictionary copy];
	}
}

#pragma mark Text system accessors
- (NSTextStorage *)storage {
	return [[storage retain] autorelease];
}

@end
