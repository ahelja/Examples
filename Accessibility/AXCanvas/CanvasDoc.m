/*
 
 File: CanvasDoc.m
 
 Abstract: Document model object
 
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

#import "CanvasDoc.h"
#import "CanvasObjectView.h"
#import "NSSet_Intersection.h"

// Archiving keys
static NSString *CanvasDocCanvasItemsKey                  = @"CanvasDocCanvasItems";
static NSString *CanvasDocSizeKey                         = @"CanvasDocSize";
static NSString *CanvasDocBackgroundColorKey              = @"CanvasDocBackgroundColor";

@implementation CanvasDoc

+ (void)initialize {
	// Sets up key dependence hierarchy.
	// Note that selectedObjectsArray is not strictly necessary, as NSSets are KVO-compliant as of Tiger.
	[self setKeys:[NSArray arrayWithObject:@"selectedObjects"] triggerChangeNotificationsForDependentKey:@"selectedObjectsArray"];

	[self setKeys:[NSArray arrayWithObjects:@"sizeWidth", @"sizeHeight", nil]
triggerChangeNotificationsForDependentKey:@"size"];
	[self setKeys:[NSArray arrayWithObjects:@"size", nil] triggerChangeNotificationsForDependentKey:@"sizeWidth"];
	[self setKeys:[NSArray arrayWithObjects:@"size", nil] triggerChangeNotificationsForDependentKey:@"sizeHeight"];
	
}

- (id)init
{
	// Designated initializer.
    self = [super init];
    if (self) {
		canvasObjects = [[NSMutableArray alloc] initWithCapacity:10];
		selectedObjects = [[NSMutableSet alloc] init];
		size = NSMakeSize(400, 500);
		backgroundColor = [[NSColor whiteColor] retain];
		
		undoController = [[CanvasManagedUndoController alloc] initWithUndoManager:[self undoManager] owner:self];
		[undoController registerObject:self];
		
		isDocumentClosed = NO;
    }
	
    return self;
}

- (void)dealloc {	
	// [self close] should have been called by now!
	
	[selectedObjects release];
	[canvasObjects release];
	[undoController release];

	[super dealloc];
}

#pragma mark NSDocument methods
- (void)close {
	
	// -[NSDocument close] can potentially be called multiple times while an
	// application is quitting, so there needs to be a lock to prevent duplicate
	// removeObserver: calls, which have the nasty side effect of caushing a crash.

	if(!isDocumentClosed) {
		// We remove this observer here, because CanvasView is not managed
		// by the CanvasManagedUndoController and never gets a callback to
		// remove itself from any observer relationships.
		[self removeObserver:canvas forKeyPath:@"canvasObjects"];
		[self removeObserver:canvas forKeyPath:@"selectedObjects"];
		[self removeObserver:canvas forKeyPath:@"size"];
		[self removeObserver:canvas forKeyPath:@"backgroundColor"];
		
		isDocumentClosed = YES;
	}
	
	// Undo controller implements its own lock, soo
	[undoController unregisterAllObjects];
	[super close];
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
	// Saves the document into a (keyed) archive.
	NSMutableData *data = [NSMutableData data];
	NSKeyedArchiver *archiver = [[NSKeyedArchiver alloc] initForWritingWithMutableData:data];
	[archiver encodeObject:canvasObjects forKey:CanvasDocCanvasItemsKey];
	[archiver encodeSize:size forKey:CanvasDocSizeKey];
	[archiver encodeObject:backgroundColor forKey:CanvasDocBackgroundColorKey];
	[archiver finishEncoding];
	[archiver release];
	
	return data;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
	// Loads the document from a (keyed) archive.
	
	// Since the addition of objects to a clean document from a file should not be undoable,
	// undo registration is suppressed while loading the keyed archive.
	[[self undoManager] disableUndoRegistration]; 
	
	NSKeyedUnarchiver *unarchiver = [[NSKeyedUnarchiver alloc] initForReadingWithData:data];
	NSArray *newObjects = [unarchiver decodeObjectForKey:CanvasDocCanvasItemsKey];
	size = [unarchiver decodeSizeForKey:CanvasDocSizeKey];
	backgroundColor = [[unarchiver decodeObjectForKey:CanvasDocBackgroundColorKey] retain];
	[unarchiver release];
	
	// Objects are manually added to the array to ensure that they are properly observed
	// by the managed undo controller.
    NSEnumerator *e = [newObjects objectEnumerator];
	CanvasObject *canvasObject;
	while(canvasObject = (CanvasObject *)[e nextObject]) {
		[self addObjectInCanvasObjects:canvasObject];
	}
	
	// Now that the document has loaded its canvas objects, it is safe to re-enable undo.
	[[self undoManager] enableUndoRegistration];
	
	return YES;
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController
{
	// Initializes objects in the document's nib
    [super windowControllerDidLoadNib:aController];
	
	// Since the canvas doesn't exist while loading canvas objects in loadDataRepresentation:ofType:,
	// the objects need to be manually added to the canvas.
	NSEnumerator *e = [canvasObjects objectEnumerator];
	CanvasObject *canvasObject;
	while(canvasObject = (CanvasObject *)[e nextObject]) {
		[canvas addSubview:[canvasObject view]];
	}
	
	// We add the observer here to avoid retain loops since the CanvasView does not receive
	// notifications from the CanvasManagedUndoController.
	[self addObserver:canvas
		   forKeyPath:@"canvasObjects"
			  options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld)
			  context:nil];
	
	[self addObserver:canvas
		   forKeyPath:@"selectedObjects"
			  options:nil
			  context:nil];	
	
	[self addObserver:canvas
		   forKeyPath:@"size"
			  options:nil
			  context:nil];

	[self addObserver:canvas
		   forKeyPath:@"backgroundColor"
			  options:nil
			  context:nil];

	// This needs to be moved to application initialization time
	[[NSColorPanel sharedColorPanel] setShowsAlpha:YES];
	
	// Sync up the canvas with the document.
	[canvas setBoundsSize:[self size]];
	[canvas setScaleFactor:1.0];
}

- (NSString *)windowNibName
{
	// Returns the name of the nib for this document.
    return @"CanvasDoc";
}

#pragma mark Canvas object accessors
- (NSArray *)canvasObjects {
	// Returns an array of the document's canvas objects.
	return [[canvasObjects retain] autorelease];
}

- (int)countOfCanvasObjects {
	// Returns the number of canvas objects in this document.
	return [canvasObjects count];
}

- (id)objectInCanvasObjectsAtIndex:(int)index {
	// Returns the canvas object at index.
	return [canvasObjects objectAtIndex:index];
}

- (void)addObjectInCanvasObjects:(id)anObject {
	// Adds anObject to the document's canvas objects.
	[self insertObject:anObject inCanvasObjectsAtIndex:[canvasObjects count]];
}

- (void)insertObject:(id)anObject inCanvasObjectsAtIndex:(int)index {
	// Inserts anObject in the document's canvas objects at anIndex, handling undo
	// registration as necessary. This is the designated object insertion method.
	if(![anObject isKindOfClass:[CanvasObject class]])
		return;
	
	// Though KVO should handle the willChange:... and didChange:... messages automatically,
	// it doesn't always do it properly inside an observation method. Therefore, we just send
	// the messages ourselves to be sure.
	[self willChange:NSKeyValueChangeInsertion valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:@"canvasObjects"];
	[(CanvasObject *)anObject setDocument:self];
	[canvasObjects insertObject:anObject atIndex:index];
	[undoController registerObject:anObject];
	[self didChange:NSKeyValueChangeInsertion valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:@"canvasObjects"];
}

- (void)removeObjectFromCanvasObjects:(id)anObject {
	// Removes anObject from the document's canvas objects.
	int index = [canvasObjects indexOfObject:anObject];
	if(index != NSNotFound)
		[self removeObjectFromCanvasObjectsAtIndex:index];
}

- (void)removeObjectFromCanvasObjectsAtIndex:(int)index {
	// Removes the canvas object at anIndex, handling undo unregistration as necessary.
	// This is the designated object removal method.

	CanvasObject *canvasObject = [canvasObjects objectAtIndex:index];

	// Though KVO should handle the willChange:... and didChange:... messages automatically,
	// it doesn't always do it properly inside an observation method. Therefore, we just send
	// the messages ourselves to be sure.
	[self willChange:NSKeyValueChangeRemoval valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:@"canvasObjects"];
	[undoController unregisterObject:canvasObject];
	
	// Remove the object from the selection, if necessary.
	if([selectedObjects containsObject:canvasObject]) {
		[self willChangeValueForKey:@"selectedObjects"];
		[selectedObjects removeObject:canvasObject];
		[canvasObject setIsSelected:NO];
		[self didChangeValueForKey:@"selectedObjects"];
	}

	[canvasObjects removeObjectAtIndex:index];
	[self didChange:NSKeyValueChangeRemoval valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:@"canvasObjects"];

}

- (void)replaceObjectInCanvasObjectsAtIndex:(int)index withObject:(id)anObject {
	// Replaces the object at index with anObject, handling undo registration as necessary.
	
	// Though KVO should handle the willChange:... and didChange:... messages automatically,
	// it doesn't always do it properly inside an observation method. Therefore, we just send
	// the messages ourselves to be sure.
	[self willChange:NSKeyValueChangeReplacement valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:@"canvasObjects"];
	
	[undoController unregisterObject:[canvasObjects objectAtIndex:index]];
	[canvasObjects replaceObjectAtIndex:index withObject:anObject];
	[undoController registerObject:anObject];
	
	[self didChange:NSKeyValueChangeReplacement valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:@"canvasObjects"];
}

#pragma mark Canvas object ordering
- (void)moveCanvasObjectForward:(CanvasObject *)anObject {
	int count = [canvasObjects count];
	int index = [canvasObjects indexOfObject:anObject];
	if((count >= 2) && (index < (count - 1)))
		[canvasObjects exchangeObjectAtIndex:index withObjectAtIndex:index + 1];
}

- (void)moveCanvasObjectBackward:(CanvasObject *)anObject {
	int count = [canvasObjects count];
	int index = [canvasObjects indexOfObject:anObject];
	if((count >= 2) && (index > 0))
		[canvasObjects exchangeObjectAtIndex:index withObjectAtIndex:index - 1];	
}

#pragma mark Canvas object selection
- (NSSet *)selectedObjects {
	// Returns the set of currently selected objects.
	return [[selectedObjects retain] autorelease];
}

- (NSArray *)selectedObjectsArray {
	// Returns an array of currently selected objects; used primarily for bindings, though
	// NSSets are KVO-compliant as of Tiger.
	return [selectedObjects allObjects];
}

- (void)setSelectedObjects:(NSSet *)aSelectionSet {
	// Replaces the current set of selected objects with aSelectionSet.
	
	if((![aSelectionSet isEqual:selectedObjects]) && ([aSelectionSet isSubsetOfSet:[NSSet setWithArray:canvasObjects]])) {
		// To avoid unnecessary dirtying objects, only the objects whose state is actually changing
		// is retrieved.
		NSSet *addedObjects = [aSelectionSet differenceWithSet:selectedObjects];
		NSSet *removedObjects = [selectedObjects differenceWithSet:aSelectionSet];
		
		NSEnumerator *enumerator;
		CanvasObject *anObject;
		
		enumerator = [addedObjects objectEnumerator];
		while(anObject = [enumerator nextObject]) {
			[anObject setIsSelected:YES];
		}
		
		enumerator = [removedObjects objectEnumerator];
		while(anObject = [enumerator nextObject]) {
			[anObject setIsSelected:NO];
		}
		
		[selectedObjects release];
		selectedObjects = [aSelectionSet mutableCopy];
		[objectController setContent:[self selectedObjectsArray]];
		[objectController setSelectionIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, [[self selectedObjects] count])]];
	}
}

- (void)addCanvasObjectsToSelection:(NSArray *)objectsToAdd {
	// Adds objectsToAdd to the current selection.
	NSMutableSet *newSelection = [selectedObjects mutableCopy];
	
	NSEnumerator *e = [objectsToAdd objectEnumerator];
	CanvasObject *canvasObject;
	while(canvasObject = (CanvasObject *)[e nextObject])
		if([canvasObjects containsObject:canvasObject])
			[newSelection addObject:canvasObject];
	
	[self setSelectedObjects:newSelection];
	[newSelection release];
}

- (void)removeCanvasObjectsFromSelection:(NSArray *)objectsToRemove {
	// Removes objectToRemoves from the current selection.
	NSMutableSet *newSelection = [selectedObjects mutableCopy];
	
	NSEnumerator *e = [objectsToRemove objectEnumerator];
	CanvasObject *canvasObject;
	while(canvasObject = (CanvasObject *)[e nextObject])
		if([canvasObjects containsObject:canvasObject])
			[newSelection removeObject:canvasObject];
	
	[self setSelectedObjects:newSelection];
	[newSelection release];
}

- (void)selectCanvasObject:(CanvasObject *)canvasObject {
	if([canvasObjects containsObject:canvasObject])
		[self setSelectedObjects:[NSSet setWithObject:canvasObject]];
}

#pragma mark Document attribute accessors
- (NSColor *)backgroundColor {
	return [[backgroundColor retain] autorelease];
}

- (void)setBackgroundColor:(NSColor *)aColor {
	if(![backgroundColor isEqual:aColor]) {
		[backgroundColor release];
		backgroundColor = [aColor copy];
	}
}

- (NSSize)size {
	return size;
}

- (void)setSize:(NSSize)aSize {
	if(!NSEqualSizes(aSize, size))
		size = aSize;
}

- (float)sizeWidth {
	return size.width;
}

- (void)setSizeWidth:(float)aFloat {
	size.width = aFloat;
}

- (float)sizeHeight {
	return size.height;
}

- (void)setSizeHeight:(float)aFloat {
	size.height = aFloat;
}

#pragma mark Managed undo support (CanvasManagedObject)
+ (NSArray *)undoableKeys {
	// Returns an array of keys whose modification should be undoable.
	// See CanvasManagedUndoController for further documentation.
	static NSArray *undoableKeys = nil;
	if(!undoableKeys)
		undoableKeys = [[NSArray arrayWithObjects:@"backgroundColor", @"canvasObjects", @"size", nil] retain];
	return undoableKeys;
}

- (void)setValue:(id)aValue forUndoableKeyPath:(NSString *)keyPath {
	// Sets the receiver's keyPath to aValue. See CanvasManagedUndoController for
	// further documentation.
	[self setValue:aValue forKeyPath:keyPath];
}

#pragma mark Managed undo support (CanvasManagedObjectCollection)
- (void)insertObjects:(NSArray *)anArray intoUndoableKeyPath:(NSString *)keyPath {
	// Inserts objects in anArray into the receiver's collection key at keyPath.
	// See CanvasManagedUndoController for further documentation.
	[[self mutableArrayValueForKeyPath:keyPath] addObjectsFromArray:anArray];
}

- (void)removeObjects:(NSArray *)anArray fromUndoableKeyPath:(NSString *)keyPath {
	// Removes objects in anArray from the receiver's collection key at keyPath.
	// See CanvasManagedUndoController for further documentation.
	[[self mutableArrayValueForKeyPath:keyPath] removeObjectsInArray:anArray];
}

- (void)replaceObjects:(NSArray *)oldObjects withObjects:(NSArray *)newObjects inUndoableKeyPath:(NSString *)keyPath {
	// Replaces oldObjects in receiver's collection key keyPath with the newObjects.
	// See CanvasManagedUndoController for further documentation.
	NSMutableArray *mutableKeyArray = [self mutableArrayValueForKeyPath:keyPath];
	int i;
	for(i = 0; i < [newObjects count]; i++) {
		[mutableKeyArray replaceObjectAtIndex:[mutableKeyArray indexOfObject:[newObjects objectAtIndex:i]] withObject:[oldObjects objectAtIndex:i]];
	}
}
@end

#pragma mark Comparator functions
int CanvasObjectSortByZOrder(CanvasObjectView *firstView, CanvasObjectView *secondView, void *context) {
	// This function could be greatly optimized by keeping a separate order
	// list, but it gets the job done.
	NSArray *canvasObjects = [(CanvasDoc *)context canvasObjects];
	int firstIndex = [canvasObjects indexOfObject:[firstView representedObject]];
	int secondIndex = [canvasObjects indexOfObject:[secondView representedObject]];
	return (firstIndex < secondIndex) ? NSOrderedAscending : NSOrderedDescending;
}
