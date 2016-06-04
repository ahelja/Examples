/*
 *  Filters.h
 *  Erode
 *
 *  Created by Ian Ollmann on Thu Oct 03 2002.
 *  Copyright (c) 2002 Apple. All rights reserved.
 *
 */

#ifndef _FILTERS_H_
#define _FILTERS_H_	1

#ifndef USING_ACCELERATE
    #include <vImage/vImage_Types.h>
#endif

typedef int (*TestFunc)(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags ); 
typedef int (*KernelInitFunc)( void *inData, int height, int width );

//Bit depths
enum
{
    Depth_8_bits_per_channel = 8,
    Depth_32_bit_FP_per_channel = 32
};

//Filter Flags
enum
{
    kNoFilterFlags = 0,
    kDoOperationInPlace = (1UL << 0),
    
    //Geometry controls to enable
    kTranslateX = 	(1UL << 8),
    kTranslateY = 	(1UL << 9),
    kRotate = 		(1UL << 10),
    kShearX = 		(1UL << 11),
    kShearY = 		(1UL << 12),
    kScaleX = 		(1UL << 13),
    kScaleY = 		(1UL << 14),
    kNeedsFunction = 	(1UL << 15),
    kGeometryFlags = kTranslateX | kTranslateY | kRotate | kShearX | kShearY | kScaleX | kScaleY,
    
    //Alpha flags
    kIsAlpha = 		(1UL << 16)
};

enum
{
    kNoKernelType	=	0,
    kNoKernelData,		//For cases like the Min/Max filters that have a kernel size but no data in the kernel
    kUInt8KernelType,
    kSInt8KernelType,
    kUInt16KernelType,
    kSInt16KernelType,
    kUInt32KernelType,
    kSInt32KernelType,
    kFloatKernelType,
    kDoubleKernelType,
    kGeometryKernelType,	//A TransformInfo struct. See Geometry Filters.h
    kAlphaKernelType,		//A AlphaInfo struct. See AlphaFilters.h
    
    kLastKernelType
};

//Index with the kernelTypes in the enum above
extern const unsigned char kKernelTypeSizes[];


typedef struct
{
    KernelInitFunc	intFunc;
    KernelInitFunc	fpFunc;
    int			defaultHeight;
    int			defaultWidth;
    int			typeInt;
    int			typeFP;
    NSString		*name;
}KernelInitFunction;

typedef struct 
{
    int 		count;
    KernelInitFunction	list[1];		//The first item in the list is the default
}KernelInitFunctionList;

typedef struct
{
    TestFunc			function;
    TestFunc			functionFP;
    TestFunc			functionInterleaved;
    TestFunc			functionFPInterleaved;
    NSString			*name;
    int				flags;		// See Filter Flags
    KernelInitFunctionList	*kernelInitList;
}FilterInfo;

typedef struct 
{
    FilterInfo			*list;
    NSString			*name;
    int				count;
}FilterList;

extern FilterList		filterLists[];
extern int			kListCount;



#define	kNoFilter 	-1L

#endif