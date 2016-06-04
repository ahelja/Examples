/*
 
 File: CanvasManagedUndoController.m
 
 Abstract: KVO undo helper
 
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


#import "CanvasManagedUndoController.h"

// Uncomment this line to enable undo debugging.
//#define CANVAS_UNDO_DEBUG

#ifdef CANVAS_UNDO_DEBUG
#define CUNLog(...) NSLog(__VA_ARGS__)
#else
#define CUNLog(...) 
#endif

// This class provides key-value-observing--based undo support for objects which conform to 
// the CanvasManagedObject (for scalar/object values) and CanvasManagedObjectCollection (for
// collections) informal protocols. It provides a solution to the bug diagnosed at:
// http://www.cocoabuilder.com/archive/message/cocoa/2004/8/24/115466
//
// The underlying problem is that NSUndoManager implements the NSKeyValueCoding category on
// NSObject, and therefore executes KVC coding methods instead of storing the invocations.
// Objects implementing the aforementioned categories implement duplicate key-value coding
// methods with different method names to avoid this trap. (Bug 2650961)
//
// Due to a bug in key-value observation, -deallocing objects that are being observed throws
// a message to the console and removes the observers. CanvasManagedUndoController retains
// copies of all observed objects in its deadObject pool to avoid these traps, at the cost
// of memory overhead. When this controller's owner no longer requires undo management
// (most commonly when an NSDocument is being closed), it should call -unregisterAllObjects
// to notify all the managed objects that they are to stop observing other objects.
//
// There are a number of improvements that could be made to this class. Memory could be conserved
// by checking the -retainCount of objects in the dead pool and removing those with a -retainCount
// of 1, triggered by a periodic timer or other mechanism. However, this is probably unnecessary
// for most typical documents. The repetitious implementation of the two informal protocols could
// be replaced by an isa swizzling scheme similar to that used by key-value observing, injecting
// the required methods into observed classes automatically.
//
// CanvasManagedUndoController supports localized undo action names via strings files. For each
// class whose instances you wish to be undoable, create a strings file called "Undo(className).strings"
// where (className) is the name of the class.  For scalar values, the strings key should be the
// name of the file, and the action name the value. Collection keys should have three keys with
// corresponding action names: "(key)-insertion", "key-removal", and "key-replacement", where
// (key) is the name of the key.
//
// To use the CanvasManagedUndoController:
// - In your objects, implement the CanvasManagedObject informal protocol, and if your class has
//   any collection keys, the CanvasManagedObjectCollection informal protocol.
// - Create an undo controller for every NSUndoManager in your project.
// - Register any new, undoable objects with the undo controller using -registerObject:.
// - When finished with the undo manager, call -unregisterAllObjects before calling -release.
// - To support named undo actions, create a strings file as described above.

@implementation CanvasManagedUndoController

- (id)initWithUndoManager:(NSUndoManager *)anUndoManager owner:(id)anObject {
	// Designated initializer.
	
	self = [super init];
	if(self) {
		managedObjects = [[NSMutableSet alloc] init];
		deadObjects = [[NSMutableSet alloc] init];
		undoManager = [anUndoManager retain];

		owner = anObject;
		isObservingOwner = NO;
	}
	return self;
}

- (void)dealloc {
	[managedObjects release];
	[deadObjects release];
	[undoManager release];
	owner = nil;
	
	[super dealloc];
}

#pragma mark Attribute accessors
- (NSUndoManager *)undoManager {
	return [[undoManager retain] autorelease];
}

- (void)setUndoManager:(NSUndoManager *)anUndoManager {
	if(![undoManager isEqual:anUndoManager]) {
		[undoManager release];
		undoManager = [undoManager retain];
	}
}

#pragma mark Object registration
- (void)registerObject:(id)anObject {
	// Registers anObject for undo management by the receiver.
	if(hasUnregisteredAllObjects)
		return;	
	
	if(![self objectSupportsUndo:anObject])
		return;
	
	if([managedObjects containsObject:anObject])
		return;
	
	// Keep owner observation info separate to avoid a retain loop.
	if(anObject == owner)
		isObservingOwner = YES;
	else
		[managedObjects addObject:anObject];
	
	// Reanimate any zombie objects.
	if([deadObjects containsObject:anObject])
		[deadObjects removeObject:anObject];
	
	// Observe the undoable keys of the object.
	NSEnumerator *e = [[[anObject class] undoableKeys] objectEnumerator];
	NSString *keyPath;
	while(keyPath = (NSString *)[e nextObject])
		[anObject addObserver:self
				   forKeyPath:keyPath
					  options:(NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew)
					  context:nil];
	
}

- (void)unregisterObject:(id)anObject {
	// Unregisters anObject for undo management by the receiver.
	if(hasUnregisteredAllObjects)
		return;	
	
	if(![managedObjects containsObject:anObject] && (anObject != owner))
		return;
	
	// Keep owner observation info separate to avoid a retain loop.
	if(anObject == owner)
		isObservingOwner = NO;
	else {
		// Retain dead objects in a separate pool to avoid observation -dealloc bug.
		[deadObjects addObject:anObject];
		[managedObjects removeObject:anObject];
	}

	// Stop observing the undoable keys of the object.
	NSEnumerator *e = [[[anObject class] undoableKeys] objectEnumerator];
	NSString *keyPath;
	while(keyPath = (NSString *)[e nextObject])
		[anObject removeObserver:self forKeyPath:keyPath];	
}

- (void)unregisterAllObjects {
	// Unregisters all objects managed by the receiver and notifies them
	// of pending release.
	
	if(hasUnregisteredAllObjects)
		return;	
	
	NSEnumerator *enumerator = [managedObjects objectEnumerator];
	id anObject;
	while(anObject = [enumerator nextObject])
		[self unregisterObject:anObject];

	if(isObservingOwner)
		[self unregisterObject:owner];
	
	// Notify object that it is being unregistered from the undo controller.
	// Objects can use this callback to as a notification to stop observing other objects.
	enumerator = [deadObjects objectEnumerator];
	while(anObject = [enumerator nextObject]) {
		if([anObject respondsToSelector:@selector(undoControllerWillUnregister:)])
			[anObject performSelector:@selector(undoControllerWillUnregister:) withObject:self];
	}
	
	if([owner respondsToSelector:@selector(undoControllerWillUnregister:)])
		[owner performSelector:@selector(undoControllerWillUnregister:) withObject:self];
		
	// Lock the undo controller from observing any more objects.
	hasUnregisteredAllObjects = YES;
}

#pragma mark Private methods
- (BOOL)objectSupportsUndo:(id)anObject {
	// Checks support for CanvasManagedObject informal protocol.
	return [[anObject class] respondsToSelector:@selector(undoableKeys)] && [anObject respondsToSelector:@selector(setValue:forUndoableKeyPath:)];
}

- (BOOL)objectSupportsCollectionUndo:(id)anObject {
	// Checks support for CanvasManagedObjectCollection informal protocol.
	return [anObject respondsToSelector:@selector(insertObjects:intoUndoableKeyPath:)] && [anObject respondsToSelector:@selector(removeObjects:fromUndoableKeyPath:)] && [anObject respondsToSelector:@selector(replaceObjects:withObjects:inUndoableKeyPath:)];
}

- (NSString *)actionNameForObject:(id)object keyPath:(NSString *)keyPath changeKind:(int)changeKind {
	// Returns the localized undo action name for a changeKind change to object's keyPath.
	
	NSString *fullKeyPath;
	switch(changeKind) {
		case NSKeyValueChangeInsertion:
			fullKeyPath = [NSString stringWithFormat:@"%@-insertion", keyPath];
			break;
		case NSKeyValueChangeRemoval:
			fullKeyPath = [NSString stringWithFormat:@"%@-removal", keyPath];
			break;
		case NSKeyValueChangeReplacement:
			fullKeyPath = [NSString stringWithFormat:@"%@-replacement", keyPath];
			break;
		case NSKeyValueChangeSetting:
		default:
			fullKeyPath = keyPath;
			break;
	}

	NSString *tablePath = [NSString stringWithFormat:@"Undo%@", [object className]];
	return [[NSBundle mainBundle] localizedStringForKey:fullKeyPath value:[keyPath capitalizedString] table:tablePath];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
					  ofObject:(id)object
						change:(NSDictionary *)change
					   context:(void *)context {
	// Proceses changes to managed objects and registers undo invocations, targeting methods in
	// the CanvasManagedUndo and CanvasManagedUndoCollection protocols.
	
	NSArray *newObjects, *oldObjects;
	NSMutableArray *mutableKeyArray;
	id changedValue;
	int i;
	
	switch([(NSNumber *)[change objectForKey:NSKeyValueChangeKindKey] intValue]) {
		case NSKeyValueChangeInsertion:
			if(![self objectSupportsCollectionUndo:object]) {
				NSLog(@"CanvasManagedUndoController: [%@] The key path %@ is not KVO undo compliant.", [object className], keyPath);
				return;
			}
			
			newObjects = [change objectForKey:NSKeyValueChangeNewKey];
			[[undoManager prepareWithInvocationTarget:object] removeObjects:newObjects fromUndoableKeyPath:keyPath];
			CUNLog(@"Inserting %d object(s) into %@.%@", [newObjects count], [object className], keyPath);
			break;
		case NSKeyValueChangeRemoval:
			if(![self objectSupportsCollectionUndo:object]) {
				NSLog(@"CanvasManagedUndoController: [%@] The key path %@ is not KVO undo compliant.", [object className], keyPath);
				return;
			}
			
			oldObjects = [change objectForKey:NSKeyValueChangeOldKey];
			[[undoManager prepareWithInvocationTarget:object] insertObjects:oldObjects intoUndoableKeyPath:keyPath];
			CUNLog(@"Removing %d object(s) from %@.%@", [oldObjects count], [object className], keyPath);
			break;
		case NSKeyValueChangeReplacement:
			if(![self objectSupportsCollectionUndo:object]){
				NSLog(@"CanvasManagedUndoController: [%@] The key path %@ is not KVO undo compliant.", [object className], keyPath);
				return;
			}
			
			newObjects = [change objectForKey:NSKeyValueChangeNewKey];
			oldObjects = [change objectForKey:NSKeyValueChangeOldKey];

			// Choose not to handle uneven replacements right now.
			if([newObjects count] != [oldObjects count])
				return;
			
			[[undoManager prepareWithInvocationTarget:object] replaceObjects:newObjects withObjects:oldObjects inUndoableKeyPath:keyPath];
			
			CUNLog(@"Replacing %d object(s) in %@.%@", [newObjects count], [object className], keyPath);
			break;
		case NSKeyValueChangeSetting:
			changedValue = [change objectForKey:NSKeyValueChangeOldKey];
			[[undoManager prepareWithInvocationTarget:object] setValue:changedValue forUndoableKeyPath:keyPath];
			CUNLog(@"Changing value for %@.%@", [object className], keyPath);
			break;
	}	
	
	// The existing action name should be preserved if the action is being undone, as keys may be set in a separate
	// order while unwinding the undo stack.
	if(![undoManager isUndoing]) {
		[undoManager setActionName:[self actionNameForObject:object keyPath:keyPath changeKind:[(NSNumber *)[change objectForKey:NSKeyValueChangeKindKey] intValue]]];
		CUNLog(@" - Action name: %@", [self actionNameForObject:object keyPath:keyPath changeKind:[(NSNumber *)[change objectForKey:NSKeyValueChangeKindKey] intValue]]);
	}
}


@end
