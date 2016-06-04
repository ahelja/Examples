//
//  Kernel.m
//  Erode
//
//  Created by Ian Ollmann on Wed Nov 20 2002.
//  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
//

#import "Kernel.h"
#import "KernelPane.h"
#include "Filters.h"

@implementation Kernel


-(id)init
{
    height = 0;
    width = 0;
    type = kNoKernelType;
    filter = kNoFilter;
    data = [[ NSArray alloc ] init ];
    
    return self;
}

-(void)setFilter:(int)the_filter
{
    if( the_filter != filter )
    {
        filter  = the_filter;
        function = 0;
        wasInitFunctionModifiedFromOriginal = YES;
    }
}

-(int)filter
{
    return filter;
}



-(id)initWithData:(const KernelInfo*)kernelData  allocFromZone:(NSZone*)zone;
{
    int size;

    if( NULL == kernelData )
    {    
        height = width = 0;
        wasDataModifiedFromLastPrefab = NO;
        wasDataModifiedFromOriginal = NO;
        wasInitFunctionModifiedFromOriginal = NO;
        type = kNoKernelData;
        data = NULL;
    }
    else
    {
        height = kernelData->height;
        width = kernelData->width;
        type = kernelData->type;
        divisor = kernelData->divisor;
        data = NULL;
        wasDataModifiedFromLastPrefab = NO;
        wasDataModifiedFromOriginal = NO;
        wasInitFunctionModifiedFromOriginal = NO;

        size =  height * width * [ self sizeofElement ] ;

        if( 0 == size )
            return self;

        if( nil == zone )
            zone = NSDefaultMallocZone();

        data = NSZoneMalloc( zone, size );

        if( NULL == data )
        {
            [ self release ];
            return nil;
        }
        
        memcpy( data, kernelData->kernel, size );
    }
    
    return self;
}

-(id)initWithFunctionInt:( const KernelInitFunctionList *)func allocFromZone:(NSZone*)zone
{
    int size;
    
    if( NULL == func || func->count == 0 )
    {
        height = width = 0;
        type = kNoKernelData;
        data = NULL;    
        function = 0;
        wasDataModifiedFromLastPrefab = NO;
        wasDataModifiedFromOriginal = NO;
        wasInitFunctionModifiedFromOriginal = NO;
    }
    else
    {
        const KernelInitFunction *f = &func->list[0];
    
        height = f->defaultHeight;
        width = f->defaultWidth;
        type = f->typeInt;
        data = NULL;
        function = 0;
        wasDataModifiedFromLastPrefab = NO;
        wasDataModifiedFromOriginal = NO;
        wasInitFunctionModifiedFromOriginal = NO;
        
        size = height*width*[ self sizeofElement ];
        
        if( 0 == size )
            return self;
        
        if( nil == zone )
            zone = NSDefaultMallocZone();

        data = NSZoneMalloc( zone, size );

        if( NULL == data )
        {
            [ self release ];
            return nil;
        }
    
        [ self applyInitFunction ];
    }
    
    return self;
}

-(id)initWithFunctionFP:( const KernelInitFunctionList *)func allocFromZone:(NSZone*)zone
{
    int size;
    
    if( NULL == func || func->count == 0 )
    {
        height = width = 0;
        type = kNoKernelData;
        data = NULL;
        function = 0;
        wasDataModifiedFromOriginal = NO;   
        wasDataModifiedFromLastPrefab = NO; 
        wasInitFunctionModifiedFromOriginal = NO;
    }
    else
    {
        const KernelInitFunction *f = &func->list[0];
    
        height = f->defaultHeight;
        width = f->defaultWidth;
        type = f->typeFP;
        data = NULL;
        function = 0;
        wasDataModifiedFromOriginal = NO;
        wasDataModifiedFromLastPrefab = NO;
        wasInitFunctionModifiedFromOriginal = NO;
        
        size = height*width*[ self sizeofElement ];
        
        if( 0 == size )
            return self;
        
        if( nil == zone )
            zone = NSDefaultMallocZone();

        data = NSZoneMalloc( zone, size );

        if( NULL == data )
        {
            [ self release ];
            return nil;
        }
    
        [ self applyInitFunction ];
    }
    
    return self;
}


- (int)numberOfRowsInTableView:(NSTableView *)aTableView 
{ 
    if( [ self sizeofElement ] > 0 )
        return height; 

    return 0;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
    int column = [ [ aTableColumn identifier ] intValue ];
    void *dataPtr = (void*) ((char*) data + (width * rowIndex + column) * kKernelTypeSizes[ type ] );
    
    switch( type )
    {
        case kUInt8KernelType:
            return [ NSNumber numberWithUnsignedChar: ((uint8_t*) dataPtr)[0] ];

        case kSInt8KernelType:
            return [ NSNumber numberWithChar: ((int8_t*) dataPtr)[0] ];

        case kUInt16KernelType:
            return [ NSNumber numberWithUnsignedShort: ((uint16_t*) dataPtr)[0] ];

        case kSInt16KernelType:
            return [ NSNumber numberWithShort: ((int16_t*) dataPtr)[0] ];

        case kUInt32KernelType:
            return [ NSNumber numberWithUnsignedInt: ((uint32_t*) dataPtr)[0] ];

        case kSInt32KernelType:
            return [ NSNumber numberWithInt: ((int32_t*) dataPtr)[0] ];

        case kFloatKernelType:
            return [ NSNumber numberWithFloat: ((float*) dataPtr)[0] ];

        case kDoubleKernelType:
            return [ NSNumber numberWithDouble: ((double*) dataPtr)[0] ];

        default:
            return [ NSNumber numberWithInt: 0 ];
    }
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
    int column = [ [ aTableColumn identifier ] intValue ];
    void *dataPtr = (void*) ((char*) data + (width * rowIndex + column) * kKernelTypeSizes[ type ] );
    BOOL dataChanged = NO;
    int  i;
    float f;
    double d;
    
    switch( type )
    {
        case kUInt8KernelType:
            i = [ anObject intValue  ];
            if( i != ((uint8_t*) dataPtr)[0] )
            {
                ((uint8_t*) dataPtr)[0] = i;
                dataChanged = YES;
            }
            break;
        case kSInt8KernelType:
            i = [ anObject intValue  ];
            if( i != ((int8_t*) dataPtr)[0] )
            {
                ((int8_t*) dataPtr)[0] = i;
                dataChanged = YES;
            }
            break;
        case kUInt16KernelType:
            i = [ anObject intValue  ];
            if( i != ((uint16_t*) dataPtr)[0] )
            {
                ((uint16_t*) dataPtr)[0] = i;
                dataChanged = YES;
            }
            break;
        case kSInt16KernelType:
            i = [ anObject intValue  ];
            if( i != ((int16_t*) dataPtr)[0] )
            {
                ((int16_t*) dataPtr)[0] = i;
                dataChanged = YES;
            }
            break;
        case kUInt32KernelType:
            i = [ anObject intValue  ];
            if( i != ((int32_t*) dataPtr)[0] )
            {
                ((uint32_t*) dataPtr)[0] = i;
                dataChanged = YES;
            }
            break;
        case kSInt32KernelType:
            i = [ anObject intValue  ];
            if( i != ((int32_t*) dataPtr)[0] )
            {
                ((int32_t*) dataPtr)[0] = i;
                dataChanged = YES;
            }
            break;
        case kFloatKernelType:
            f = [ anObject floatValue  ];
            if( f != ((float*) dataPtr)[0] )
            {
                ((float*) dataPtr)[0] = f;
                dataChanged = YES;
            }
            break;
        case kDoubleKernelType:
            d = [ anObject doubleValue  ];
            if( d != ((double*) dataPtr)[0] )
            {
                ((double*) dataPtr)[0] = d;
                dataChanged = YES;
            }
            break;
        default:
            break;
    }
    
    if( YES == dataChanged )
        if( NO == wasDataModifiedFromLastPrefab || NO == wasDataModifiedFromOriginal )
        {
            KernelPane *pane = (KernelPane*) [ aTableView window ];
        
            wasDataModifiedFromLastPrefab = YES;
            wasDataModifiedFromOriginal = YES;
    
            [ pane setupPrefabMenu ];        
        }
    

}


-(int)sizeofElement
{
    if( type < kNoKernelType || type >= kLastKernelType )
        return 0;

    return kKernelTypeSizes[ type ];
}

-(void)resizeToHeight:(int)newHeight width:(int)newWidth  type:(int)dataType
{
    int newSize = 0;
    
    if( newHeight == height && newWidth == width && dataType == type )
        return;

    wasDataModifiedFromOriginal = YES;

    if( NULL != data )
        free( data );
    
    data = NULL;
    height = newHeight;
    width = newWidth;
    type = dataType; 
    
    newSize = [ self sizeofElement ];
    if( 0 < newSize )
        data = calloc( height * width, newSize );
    
    [ self applyInitFunction ];
}


-(void)dealloc
{
    if( NULL != data )
        free( data );
        
    [ super dealloc ];
}

-(int)height	{ return height; }
-(int)width	{ return width; }
-(int)dataType	{ return type; }
-(void*)data	{ return data; }
-(int)divisor	{ return divisor; }
-(int)initFunction	{ return function; }


- (id)copyWithZone:(NSZone *)zone
{
    id copy = [ Kernel allocWithZone: zone ];
    KernelInfo	info;
    
    info.height = height;
    info.width = width;
    info.type = type;
    info.kernel = data;
    info.divisor = divisor;
    
    [ copy 	initWithData: 	&info
                allocFromZone:	zone	];
    
    [ copy 	setFilter:	filter ];
    [ copy 	setInitFunction:	function ];
    ((Kernel*)copy)->wasDataModifiedFromLastPrefab = wasDataModifiedFromLastPrefab;
    ((Kernel*)copy)->wasDataModifiedFromOriginal = wasDataModifiedFromOriginal;
    ((Kernel*)copy)->wasInitFunctionModifiedFromOriginal =wasInitFunctionModifiedFromOriginal;
;

    
    return copy;
}


-(void)setInitFunction:(int)f
{
    if( function != f )
    {
        wasInitFunctionModifiedFromOriginal = YES;

        function = f;
    }
}


-(void)setIntValueAtRow:(int)row column:(int)column toValue:(int)value
{
    void *ptr = (char*) data + kKernelTypeSizes[ type ] * height * width;
    
    switch( type )
    {
        case kUInt8KernelType:
            ((uint8_t*) ptr)[0] = value;
            break;
        case kSInt8KernelType:
            ((int8_t*) ptr)[0] = value;
            break;
        case kUInt16KernelType:
            ((uint16_t*) ptr)[0] = value;
            break;
        case kSInt16KernelType:
            ((int16_t*) ptr)[0] = value;
            break;
        case kUInt32KernelType:
            ((uint32_t*) ptr)[0] = value;
            break;
        case kSInt32KernelType:
            ((int32_t*) ptr)[0] = value;
            break;
        case kFloatKernelType:
            ((float*) ptr)[0] = value;
            break;
        case kDoubleKernelType:
            ((double*) ptr)[0] = value;
            break;
    }
}

-(void)setDoubleValueAtRow:(int)row column:(int)column toValue:(double)value
{
    void *ptr = (char*) data + kKernelTypeSizes[ type ] * height * width;

    switch( type )
    {
        case kUInt8KernelType:
            ((uint8_t*) ptr)[0] = value;
            break;
        case kSInt8KernelType:
            ((int8_t*) ptr)[0] = value;
            break;
        case kUInt16KernelType:
            ((uint16_t*) ptr)[0] = value;
            break;
        case kSInt16KernelType:
            ((int16_t*) ptr)[0] = value;
            break;
        case kUInt32KernelType:
            ((uint32_t*) ptr)[0] = value;
            break;
        case kSInt32KernelType:
            ((int32_t*) ptr)[0] = value;
            break;
        case kFloatKernelType:
            ((float*) ptr)[0] = value;
            break;
        case kDoubleKernelType:
            ((double*) ptr)[0] = value;
            break;
    }
}

-(BOOL) wasDataModifiedFromLastPrefab
{
    return wasDataModifiedFromLastPrefab;
}

-(BOOL) wasDataModifiedFromOriginal
{
    return wasDataModifiedFromOriginal;
}

-(BOOL) wasInitFunctionModifiedFromOriginal
{
    return wasInitFunctionModifiedFromOriginal;
}

-(void)setDataModifiedFromOriginal:(BOOL)wasModified
{
    wasDataModifiedFromOriginal = wasModified; 
}

-(void)setFuncModifiedFromOriginal:(BOOL)wasModified
{
    wasInitFunctionModifiedFromOriginal = wasModified; 
}


-(void)applyInitFunction
{
    int set = filter >> 16;
    int item = filter & 0xFFFF;
    KernelInitFunctionList *k = filterLists[set].list[item].kernelInitList;
    KernelInitFunc f = NULL;

    wasDataModifiedFromLastPrefab = NO;
    wasDataModifiedFromOriginal |= wasInitFunctionModifiedFromOriginal;

    //Check for no init function and init function out of bounds
    if( NULL == k || NULL == data || function < 0 || function >= k->count  )
        return;

    //Get the init function
    switch( type )
    {
        case kUInt8KernelType:
        case kSInt8KernelType:
        case kUInt16KernelType:
        case kSInt16KernelType:
        case kUInt32KernelType:
        case kSInt32KernelType:
            f = k->list[ function ].intFunc;
			if( NULL != f  )
				divisor = f( data, height, width );
			break;
            
        case kFloatKernelType:
        case kDoubleKernelType:
        case kGeometryKernelType:
            f= k->list[ function ].fpFunc;
			if( NULL != f  )
				f( data, height, width );
            break;
        
        default:
            break;
    }
    

}


@end
