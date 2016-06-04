//
//  AppController.h
//
//  Copyright (c) 2001-2002, Apple. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class SimpleTreeNode;

@interface AppController : NSObject {
@private    
    SimpleTreeNode		*treeData;
    NSArray	 		*draggedNodes;
    NSArray			*iconImages;

    IBOutlet NSOutlineView 	*outlineView;
    IBOutlet NSButton 		*autoSortCheck;
    IBOutlet NSButton 		*allowOnDropOnGroupCheck;
    IBOutlet NSButton 		*allowOnDropOnLeafCheck;
    IBOutlet NSButton 		*allowBetweenDropCheck;
    IBOutlet NSButton	 	*onlyAcceptDropOnRoot;
    IBOutlet NSFormCell   	*selectionOutput;
    
    IBOutlet NSButton 		*cleanUpAutoExpansionCheck;
}

- (void)addGroup:(id)sender;
- (void)addLeaf:(id)sender;
- (void)deleteSelections:(id)sender;
- (void)outlineViewAction:(id)sender;
- (void)toggleVerticalBeginsDrag:(id)sender;

- (NSArray*)draggedNodes;
- (NSArray *)selectedNodes;

- (BOOL)allowOnDropOnGroup;
- (BOOL)allowOnDropOnLeaf;
- (BOOL)allowBetweenDrop;

@end
