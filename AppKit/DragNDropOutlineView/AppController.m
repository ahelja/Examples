/*
	AppController.m
	Copyright (c) 2001-2004, Apple Computer, Inc., all rights reserved.
	Author: Chuck Pisula

	Milestones:
	Initially created 3/1/01

        Application Controller Object, and Outline View Data Source.
*/

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "AppController.h"
#import "SimpleTreeNode.h"
#import "ImageAndTextCell.h"
#import "NSArray_Extensions.h"
#import "NSOutlineView_Extensions.h"
#import <fcntl.h>

@interface AppController (Private)
- (void)_addNewDataToSelection:(SimpleTreeNode *)newChild;
- (NSImage *)_randomIconImage;
@end


// ================================================================
// Useful Macros
// ================================================================

#define DragDropSimplePboardType 	@"MyCustomOutlineViewPboardType"
#define INITIAL_INFODICT		@"InitInfo"
#define COLUMNID_IS_EXPANDABLE	 	@"IsExpandableColumn"
#define COLUMNID_NAME		 	@"NameColumn"
#define COLUMNID_NODE_KIND	 	@"NodeKindColumn"

// Conveniences for accessing nodes, or the data in the node.
#define NODE(n)			((SimpleTreeNode*)n)
#define NODE_DATA(n) 		((SimpleNodeData*)[NODE((n)) nodeData])
#define SAFENODE(n) 		((SimpleTreeNode*)((n)?(n):(treeData)))

@implementation AppController

- (id)init {
    NSDictionary *initData = nil;
    
    self = [super init];
    if (self==nil) return nil;
    
    // Load our initial outline view data from INITIAL_INFODICT.
    initData = [NSDictionary dictionaryWithContentsOfFile: [[NSBundle mainBundle] pathForResource: INITIAL_INFODICT ofType: @"dict"]];
    treeData = [[SimpleTreeNode treeFromDictionary: initData] retain];
    
    return self; 
}

- (void)dealloc {
    [treeData release];
    [draggedNodes release];
    [iconImages release];
    treeData = nil;
    draggedNodes = nil;
    iconImages = nil;
    [super dealloc];
}

- (void)awakeFromNib {
    NSTableColumn *tableColumn = nil;
    ImageAndTextCell *imageAndTextCell = nil;
    NSButtonCell *buttonCell = nil;

    // Insert custom cell types into the table view, the standard one does text only.
    // We want one column to have text and images, and one to have check boxes.
    tableColumn = [outlineView tableColumnWithIdentifier: COLUMNID_NAME];
    imageAndTextCell = [[[ImageAndTextCell alloc] init] autorelease];
    [imageAndTextCell setEditable: YES];
    [tableColumn setDataCell:imageAndTextCell];

    tableColumn = [outlineView tableColumnWithIdentifier: COLUMNID_IS_EXPANDABLE];
    buttonCell = [[[NSButtonCell alloc] initTextCell: @""] autorelease];
    [buttonCell setEditable: YES];
    [buttonCell setButtonType: NSSwitchButton];
    [tableColumn setDataCell:buttonCell];
    
    // Register to geet our custom type, strings, and filenames.... try dragging each into the view!
    [outlineView registerForDraggedTypes:[NSArray arrayWithObjects:DragDropSimplePboardType, NSStringPboardType, NSFilenamesPboardType, nil]];
    [outlineView setDraggingSourceOperationMask:NSDragOperationEvery forLocal:YES];
    [outlineView setDraggingSourceOperationMask:NSDragOperationAll_Obsolete forLocal:NO];
}

- (NSArray*)draggedNodes   { return draggedNodes; }
- (NSArray *)selectedNodes { return [outlineView allSelectedItems]; }
- (BOOL)allowOnDropOnGroup { return (BOOL)[allowOnDropOnGroupCheck state]; }
- (BOOL)allowOnDropOnLeaf  { return (BOOL)[allowOnDropOnLeafCheck state]; }
- (BOOL)allowBetweenDrop   { return (BOOL)[allowBetweenDropCheck state]; }
- (BOOL)onlyAcceptDropOnRoot { return (BOOL)[onlyAcceptDropOnRoot state]; }

// ================================================================
// Target / action methods. (most wired up in IB)
// ================================================================

- (void)addGroup:(id)sender {
    // Adds a new expandable entry to the current selection, by inserting a new group node.
    [self _addNewDataToSelection: [SimpleTreeNode treeNodeWithData: [SimpleNodeData groupDataWithName:@"New Group"]]];
}

- (void)addLeaf:(id)sender {
    // Adds a new leaf entry to the current selection, by inserting a new leaf node.
    [self _addNewDataToSelection: [SimpleTreeNode treeNodeWithData: [SimpleNodeData leafDataWithName:@"New Leaf"]]];
}

- (void)outlineViewAction:(id)olv {
    // This message is sent from the outlineView as it's action (see the connection in IB).
    NSArray *selectedNodes = [self selectedNodes];
    
    if ([selectedNodes count]>1) [selectionOutput setStringValue: @"Multiple Rows Selected"];
    else if ([selectedNodes count]==1) [selectionOutput setStringValue: [NODE_DATA([selectedNodes objectAtIndex:0]) description]];
    else [selectionOutput setStringValue: @"Nothing Selected"];
}

- (void)deleteSelections:(id)sender {
    NSArray *selection = [self selectedNodes];
    
    // Tell all of the selected nodes to remove themselves from the model.
    [selection makeObjectsPerformSelector: @selector(removeFromParent)];
    [outlineView deselectAll:nil];
    [outlineView reloadData];
}

- (void)toggleVerticalBeginsDrag:(id)sender {
    // This message is sent from the "Vertical Motion Begins Drag" check box.
    [outlineView setVerticalMotionCanBeginDrag: [sender state]];
}

- (void)sortData:(id)sender {
    NSArray *itemsToSelect = [self selectedNodes];
    [treeData recursiveSortChildren];
    [outlineView reloadData];
    [outlineView selectItems: itemsToSelect byExtendingSelection: NO];
}

- (BOOL)validateMenuItem:(id <NSMenuItem>)menuItem {
    if ([menuItem action]==@selector(deleteSelections:)) {
	// The delete selection item should be disabled if nothing is selected.
	return ([[self selectedNodes] count]>0);
    }    
    return YES;
}

// ================================================================
//  NSOutlineView data source methods. (The required ones)
// ================================================================

// Required methods.
- (id)outlineView:(NSOutlineView *)olv child:(int)index ofItem:(id)item {
    return [SAFENODE(item) childAtIndex:index];
}
- (BOOL)outlineView:(NSOutlineView *)olv isItemExpandable:(id)item {
    return [NODE_DATA(item) isGroup];
}
- (int)outlineView:(NSOutlineView *)olv numberOfChildrenOfItem:(id)item {
    return [SAFENODE(item) numberOfChildren];
}
- (id)outlineView:(NSOutlineView *)olv objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item {
    id objectValue = nil;
    
    // The return value from this method is used to configure the state of the items cell via setObjectValue:
    if([[tableColumn identifier] isEqualToString: COLUMNID_NAME]) {
	objectValue = [NODE_DATA(item) name];
    } else if([[tableColumn identifier] isEqualToString: COLUMNID_IS_EXPANDABLE] && [NODE_DATA(item) isGroup]) {
	// Here, object value will be used to set the state of a check box.
	objectValue = [NSNumber numberWithBool: [NODE_DATA(item) isExpandable]];
    } else if([[tableColumn identifier] isEqualToString: COLUMNID_NODE_KIND]) {
	objectValue = ([NODE_DATA(item) isLeaf] ? @"Leaf" : @"Group");
    }
    
    return objectValue;
}

// Optional method: needed to allow editing.
- (void)outlineView:(NSOutlineView *)olv setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item  {
    if([[tableColumn identifier] isEqualToString: COLUMNID_NAME]) {
	[NODE_DATA(item) setName: object];
    } else if ([[tableColumn identifier] isEqualToString: COLUMNID_IS_EXPANDABLE]) {
	[NODE_DATA(item) setIsExpandable: [object boolValue]];
	if (![NODE_DATA(item) isExpandable] && [outlineView isItemExpanded: item]) [outlineView collapseItem: item];
    } else if([[tableColumn identifier] isEqualToString: COLUMNID_NODE_KIND]) {
	// We don't allow editing of this column, so we should never actually get here.
    }

}

// ================================================================
//  NSOutlineView delegate methods.
// ================================================================

- (BOOL)outlineView:(NSOutlineView *)olv shouldExpandItem:(id)item {
    return [NODE_DATA(item) isExpandable];
}

- (void)outlineView:(NSOutlineView *)olv willDisplayCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item {    
    if ([[tableColumn identifier] isEqualToString: COLUMNID_NAME]) {
	// Make sure the image and text cell has an image.  If not, give it something random.
	if (item && ![NODE_DATA(item) iconRep]) [NODE_DATA(item) setIconRep: [self _randomIconImage]];
	// Set the image here since the value returned from outlineView:objectValueForTableColumn:... didn't specify the image part...
	[(ImageAndTextCell*)cell setImage: [NODE_DATA(item) iconRep]];
	// For fun, lets display in upper case!
	[(ImageAndTextCell*)cell setStringValue: [[cell stringValue] uppercaseString]];
    } else if ([[tableColumn identifier] isEqualToString: COLUMNID_IS_EXPANDABLE]) {
	[cell setEnabled: [NODE_DATA(item) isGroup]];
    } else if ([[tableColumn identifier] isEqualToString: COLUMNID_NODE_KIND]) {
	// Don't do anything unusual for the kind column.
    }
}

// ================================================================
//  NSOutlineView data source methods. (dragging related)
// ================================================================

// Create a fileHandle for writing to a new file located in the directory specified by 'dirpath'.  If the file basename.extension already exists at that location, then append "-N" (where N is a whole number starting with 1) until a unique basename-N.extension file is found.  On return oFilename contains the name of the newly created file referenced by the returned NSFileHandle.
NSFileHandle *NewFileHandleForWritingFile(NSString *dirpath, NSString *basename, NSString *extension, NSString **oFilename) {
    NSString *filename = nil;
    BOOL done = NO;
    int fdForWriting = -1, uniqueNum = 0;
    while (!done) {
        filename = [NSString stringWithFormat:@"%@%@.%@", basename, (uniqueNum ? [NSString stringWithFormat:@"-%d", uniqueNum] : @""), extension];
        fdForWriting = open([[NSString stringWithFormat:@"%@/%@", dirpath, filename] UTF8String], O_WRONLY | O_CREAT | O_EXCL, 0666);
        if (fdForWriting < 0 && errno == EEXIST) {
            // Try another name.
            uniqueNum++;
        } else {
            done = YES;
        }
    }

    NSFileHandle *fileHandle = nil;
    if (fdForWriting>0) {
        fileHandle = [[NSFileHandle alloc] initWithFileDescriptor:fdForWriting closeOnDealloc:YES];
    }
    
    if (oFilename) {
        *oFilename = (fileHandle ? filename : nil);
    }
    
    return fileHandle;
}

// We promised the files, so now lets make good on that promise!
- (NSArray *)outlineView:(NSTableView *)tv namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination forDraggedItems:(NSArray *)items {
    int i = 0, count = [items count];
    NSMutableArray *filenames = [NSMutableArray array];
    for (i=0; i<count; i++) {
        
        NSString *filename  = nil;
        NSFileHandle *fileHandle = NewFileHandleForWritingFile([dropDestination path], @"PromiseTestFile", @"txt", &filename);
        if (fileHandle) {
            NSData *fileData  = [[[items objectAtIndex:i] description] dataUsingEncoding:NSUTF8StringEncoding allowLossyConversion:NO];
            [fileHandle writeData: fileData];
            [fileHandle release];
            fileHandle = nil;

            [filenames addObject: filename];
        }
    }
    return ([filenames count] ? filenames : nil);
}

- (BOOL)outlineView:(NSOutlineView *)olv writeItems:(NSArray*)items toPasteboard:(NSPasteboard*)pboard {
    draggedNodes = items; // Don't retain since this is just holding temporaral drag information, and it is only used during a drag!  We could put this in the pboard actually.
    
    // Provide data for our custom type, and simple NSStrings.
    [pboard declareTypes:[NSArray arrayWithObjects: DragDropSimplePboardType, NSStringPboardType, NSFilesPromisePboardType, nil] owner:self];

    // the actual data doesn't matter since DragDropSimplePboardType drags aren't recognized by anyone but us!.
    [pboard setData:[NSData data] forType:DragDropSimplePboardType]; 
    
    // Put string data on the pboard... notice you candrag into TextEdit!
    [pboard setString: [draggedNodes description] forType: NSStringPboardType];
    
    // Put the promised type we handle on the pasteboard.
    [pboard setPropertyList:[NSArray arrayWithObjects:@"txt", nil] forType:NSFilesPromisePboardType];

    return YES;
}

- (unsigned int)outlineView:(NSOutlineView*)olv validateDrop:(id <NSDraggingInfo>)info proposedItem:(id)item proposedChildIndex:(int)childIndex {
    // This method validates whether or not the proposal is a valid one. Returns NO if the drop should not be allowed.
    SimpleTreeNode *targetNode = item;
    BOOL targetNodeIsValid = YES;

    if ([self onlyAcceptDropOnRoot]) {
	targetNode = nil;
	childIndex = NSOutlineViewDropOnItemIndex;
    } else {
	BOOL isOnDropTypeProposal = childIndex==NSOutlineViewDropOnItemIndex;
	
	// Refuse if: dropping "on" the view itself unless we have no data in the view.
	if (targetNode==nil && childIndex==NSOutlineViewDropOnItemIndex && [treeData numberOfChildren]!=0) 
	    targetNodeIsValid = NO;
    
	if (targetNode==nil && childIndex==NSOutlineViewDropOnItemIndex && [self allowOnDropOnLeaf]==NO)
	    targetNodeIsValid = NO;
	
	// Refuse if: we are trying to do something which is not allowed as specified by the UI check boxes.
	if ((targetNodeIsValid && isOnDropTypeProposal==NO && [self allowBetweenDrop]==NO) ||
	    ([NODE_DATA(targetNode) isGroup] && isOnDropTypeProposal==YES && [self allowOnDropOnGroup]==NO) ||
	    ([NODE_DATA(targetNode) isLeaf ] && isOnDropTypeProposal==YES && [self allowOnDropOnLeaf]==NO))
	    targetNodeIsValid = NO;
	    
	// Check to make sure we don't allow a node to be inserted into one of its descendants!
	if (targetNodeIsValid && ([info draggingSource]==outlineView) && [[info draggingPasteboard] availableTypeFromArray:[NSArray arrayWithObject: DragDropSimplePboardType]] != nil) {
	    NSArray *_draggedNodes = [[[info draggingSource] dataSource] draggedNodes];
	    targetNodeIsValid = ![targetNode isDescendantOfNodeInArray: _draggedNodes];
	}
    }
    
    // Set the item and child index in case we computed a retargeted one.
    [outlineView setDropItem:targetNode dropChildIndex:childIndex];
    
    return targetNodeIsValid ? NSDragOperationGeneric : NSDragOperationNone;
}

- (void)_performDropOperation:(id <NSDraggingInfo>)info onNode:(TreeNode*)parentNode atIndex:(int)childIndex {
    // Helper method to insert dropped data into the model. 
    NSPasteboard * pboard = [info draggingPasteboard];
    NSMutableArray * itemsToSelect = nil;
    
    // Do the appropriate thing depending on wether the data is DragDropSimplePboardType or NSStringPboardType.
    if ([pboard availableTypeFromArray:[NSArray arrayWithObjects:DragDropSimplePboardType, nil]] != nil) {
        AppController *dragDataSource = [[info draggingSource] dataSource];
        NSArray *_draggedNodes = [TreeNode minimumNodeCoverFromNodesInArray: [dragDataSource draggedNodes]];
        NSEnumerator *draggedNodesEnum = [_draggedNodes objectEnumerator];
        SimpleTreeNode *_draggedNode = nil, *_draggedNodeParent = nil;
        
	itemsToSelect = [NSMutableArray arrayWithArray:[self selectedNodes]];
	
        while ((_draggedNode = [draggedNodesEnum nextObject])) {
            _draggedNodeParent = (SimpleTreeNode*)[_draggedNode nodeParent];
            if (parentNode==_draggedNodeParent && [parentNode indexOfChild: _draggedNode]<childIndex) childIndex--;
            [_draggedNodeParent removeChild: _draggedNode];
        }
        [parentNode insertChildren: _draggedNodes atIndex: childIndex];
    } 
    else if ([pboard availableTypeFromArray:[NSArray arrayWithObject: NSStringPboardType]]) {
        NSString *string = [pboard stringForType: NSStringPboardType];
	SimpleTreeNode *newItem = [SimpleTreeNode treeNodeWithData: [SimpleNodeData leafDataWithName:string]];
	
	itemsToSelect = [NSMutableArray arrayWithObject: newItem];
	[parentNode insertChild: newItem atIndex:childIndex++];
    }

    [outlineView reloadData];
    [outlineView selectItems: itemsToSelect byExtendingSelection: NO];
}

- (BOOL)outlineView:(NSOutlineView*)olv acceptDrop:(id <NSDraggingInfo>)info item:(id)targetItem childIndex:(int)childIndex {
    TreeNode * 		parentNode = nil;
    
    // Determine the parent to insert into and the child index to insert at.
    if ([NODE_DATA(targetItem) isLeaf]) {
        parentNode = (SimpleTreeNode*)(childIndex==NSOutlineViewDropOnItemIndex ? [targetItem nodeParent] : targetItem);
        childIndex = (childIndex==NSOutlineViewDropOnItemIndex ? [[targetItem nodeParent] indexOfChild: targetItem]+1 : 0);
        if ([NODE_DATA(parentNode) isLeaf]) [NODE_DATA(parentNode) setIsLeaf:NO];
    } else {            
        parentNode = SAFENODE(targetItem);
	childIndex = (childIndex==NSOutlineViewDropOnItemIndex?0:childIndex);
    }
    
    [self _performDropOperation:info onNode:parentNode atIndex:childIndex];
        
    return YES;
}

@end

@implementation AppController (Private)

- (void)_addNewDataToSelection:(SimpleTreeNode *)newChild {
    int childIndex = 0, newRow = 0;
    NSArray *selectedNodes = [self selectedNodes];
    SimpleTreeNode *selectedNode = ([selectedNodes count] ? [selectedNodes objectAtIndex:0] : treeData);
    TreeNode *parentNode = nil;

    if ([NODE_DATA(selectedNode) isGroup]) { 
	parentNode = selectedNode; 
	childIndex = 0; 
    }
    else { 
	parentNode = [selectedNode nodeParent]; 
	childIndex = [parentNode indexOfChildIdenticalTo:selectedNode]+1; 
    }
    
    [parentNode insertChild: newChild atIndex: childIndex];
    [outlineView reloadData];
    
    newRow = [outlineView rowForItem: newChild];
    if (newRow>=0) [outlineView selectRow: newRow byExtendingSelection: NO];
    if (newRow>=0) [outlineView editColumn:0 row:newRow withEvent:nil select:YES];
}

- (NSImage *)_randomIconImage {
    static unsigned int imageNum = 0;
    NSImage *randimIconImage = nil;
    
    if (!iconImages) {
        int i;
        NSString *imageName = nil;
        NSImage *image = nil;
        iconImages = [[NSMutableArray array] retain]; 
    
        for( i=1,imageName = [NSString stringWithFormat:@"Image%d.tiff",i];
             (image = [NSImage imageNamed:imageName]) != nil;
             i++,imageName = [NSString stringWithFormat:@"Image%d.tiff",i]) [(NSMutableArray*)iconImages addObject: image];
    }
    
    randimIconImage = [iconImages objectAtIndex:imageNum];
    if ((++imageNum)>[iconImages count]-1) imageNum = 0;
    
    return randimIconImage;
}

@end
