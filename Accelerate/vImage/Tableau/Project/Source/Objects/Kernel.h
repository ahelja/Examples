//
//  Kernel.h
//  Erode
//
//  Created by Ian Ollmann on Wed Nov 20 2002.
//  Copyright (c) 2002 Apple. All rights reserved.
//

#include "Filters.h"
#include <AppKit/NSTableView.h>

typedef struct
{
    int		height;
    int		width;
    int		type;
    void	*kernel;
    int 	divisor;
}KernelInfo;

@interface Kernel : NSObject <NSCopying> 
{
    int			height;
    int			width;
    int			type;
    int			filter;
    void		*data;
    int			function;
    int			divisor;
    BOOL		wasDataModifiedFromLastPrefab;
    BOOL		wasDataModifiedFromOriginal;
    BOOL		wasInitFunctionModifiedFromOriginal;
}

//+(Kernel*)kernelWithData:(const KernelInfo*)data;
-(id)init;
//-(id)initWithData:(const KernelInfo*)data  allocFromZone:(NSZone*)zone;
-(id)initWithFunctionInt:( const KernelInitFunctionList *)func allocFromZone:(NSZone*)zone;
-(id)initWithFunctionFP:( const KernelInitFunctionList *)func allocFromZone:(NSZone*)zone;
-(void)dealloc;
-(int)height;
-(int)width;
-(int)sizeofElement;
-(int)dataType;
-(int)filter;
-(int)divisor;
-(int)initFunction;
-(void*)data;
-(BOOL)wasDataModifiedFromLastPrefab;
-(BOOL)wasDataModifiedFromOriginal;
-(BOOL)wasInitFunctionModifiedFromOriginal;

-(void)applyInitFunction;
-(void)setFilter:(int)filter;
-(void)setInitFunction:(int)f;
-(void)setDataModifiedFromOriginal:(BOOL)wasModified;
-(void)setFuncModifiedFromOriginal:(BOOL)wasModified;
-(void)resizeToHeight:(int)newHeight width:(int)newWidth type:(int)dataType; //all data set to 0
-(void)setIntValueAtRow:(int)row column:(int)column toValue:(int)value;
-(void)setDoubleValueAtRow:(int)row column:(int)column toValue:(double)value;

//NSCopying Protocol
- (id)copyWithZone:(NSZone *)zone;

//Implemented items from the NSTableDataSource Protocol
- (int)numberOfRowsInTableView:(NSTableView *)aTableView;
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;
- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;


@end
