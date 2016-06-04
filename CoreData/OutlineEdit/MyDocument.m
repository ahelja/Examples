/*

File: MyDocument.m

Abstract: Implementation for OutlineEdit's NSPersistentDocument subclass

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/

#import "Note.h"
#import "MyDocument.h"

@implementation MyDocument

- (id)initWithType:(NSString *)typeName error:(NSError **)outError;
{
    // this method is invoked exactly once per document at the initial creation
    // of the document.  It will not be invoked when a document is opened after
    // being saved to disk.
    self = [super initWithType:typeName error:outError];
    if (self == nil)
        return nil;
    
    NSManagedObjectContext *managedObjectContext = [self managedObjectContext];
    
    // set up priority instances
    int aPriorityValue;
    for (aPriorityValue = 0; aPriorityValue < 5; aPriorityValue++) {
        NSManagedObject *aPriority = [NSEntityDescription insertNewObjectForEntityForName: @"Priority"
                                                                   inManagedObjectContext: managedObjectContext];
        [aPriority setValue: [NSNumber numberWithInt: aPriorityValue + 1] forKey:@"value"];
    }
    
    // create a single note
    [NSEntityDescription insertNewObjectForEntityForName: @"Note"
                                  inManagedObjectContext: managedObjectContext];

    // clear the undo manager and change count for the document such that
    // untitled documents start with zero unsaved changes
    [managedObjectContext processPendingChanges];
    [[managedObjectContext undoManager] removeAllActions];
    [self updateChangeCount:NSChangeCleared];
    
    return self;
}

- (NSString *)windowNibName 
{
    return @"MyDocument";
}

- (NSArray *) prioritySortDescriptions;
{
    static NSArray *prioritySortDescriptions = nil;
    if (prioritySortDescriptions == nil) {
        NSSortDescriptor *sortDescriptor = [[NSSortDescriptor alloc] initWithKey:@"value" ascending:YES];
        prioritySortDescriptions = [[NSArray arrayWithObject: sortDescriptor] retain];
        [sortDescriptor release];
    }

    return prioritySortDescriptions;
}

- (IBAction) createNote: sender;
{
    [outlineTreeController add:sender];
}

- (IBAction) createChildNote: sender;
{
    [outlineTreeController addChild:sender];
}

- (IBAction) indentNote: sender;
{
    NSIndexPath *selectionPath = [outlineTreeController selectionIndexPath];
    if (!selectionPath) {
        NSBeep();
        return;
    }
    
	id selection = [outlineTreeController selection];

	// The selection is one of the root notes.
	// Get all the root notes to find our sibling.
	Note *parentNote = [selection valueForKeyPath:@"parent"];
	NSArray *children = (parentNote  == nil) ? (NSArray *)[outlineTreeController content] : [[parentNote valueForKeyPath:@"children"] allObjects];

	children = [children sortedArrayUsingDescriptors:[outlineTreeController sortDescriptors]];

	unsigned index = [selectionPath indexAtPosition:([selectionPath length] - 1)];
    if (index == 0) {   // Cannot indent the top root node
        NSBeep();
    }
    else {
        Note *sibling = [children objectAtIndex:index - 1];
        [selection setValue:sibling forKeyPath:@"parent"];
    }
}

- (IBAction) dedentNote: sender;
{
    /* The controller's -selection method will return a proxy to the object or objects that are actually selected.
    */
	id selection = [outlineTreeController selection];
    Note *parent = [selection valueForKeyPath:@"parent"];
    
    // make sure exactly one object is selected.
	if (parent == nil || parent == NSMultipleValuesMarker || parent == NSNoSelectionMarker || parent == NSNotApplicableMarker) {
        NSBeep();
        return;
    }
        
	parent = [parent valueForKeyPath:@"parent"];
	    
    [selection setValue:parent forKeyPath:@"parent"];
}

@end
