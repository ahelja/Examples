//
//  Item_Extensions.h
//  DragNDropOLV
//
//  Created by chuck on Wed May 31 2000.
//  Copyright (c) 2000 __CompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSObject (ItemStuff)
- (void)insertChildItem:(id)item atIndex:(int)index;
- (void)removeChildItem:(id)item;
- (void)replaceChildAtIndex:(unsigned int)index withItem:(id)replacment;
- (id)childItemAtIndex:(int)index;
- (int)indexOfChildItem:(id)item;
- (int)numberOfChildItems;
- (id)itemDescription;
- (void)setItemDescription:(NSString*)desc;
- (BOOL)isExpandable;
- (BOOL)isLeaf;
- (NSArray *)itemsByFlattening;
- (id)deepMutableCopy;
+ (id)newGroup;
+ (id)newLeaf;
+ (id)newGroupFromLeaf:(id)leaf;
- (void)sortRecursively: (BOOL) recurse;
@end

