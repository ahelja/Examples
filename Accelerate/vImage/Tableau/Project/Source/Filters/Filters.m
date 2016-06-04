/*
 *  Filters.c
 *  Erode
 *
 *  Created by Ian Ollmann on Thu Oct 03 2002.
 *  Copyright (c) 2002 Apple. All rights reserved.
 *
 */

#include "Filters.h"
#include <string.h>
#include <limits.h>

#include "MorphologyFilters.h"
#include "ConvolutionFilters.h"
#include "HistogramFilters.h"
#include "MiscFilters.h"
#include "GeometryFilters.h"
#include "AlphaFilters.h"
#include "ConversionFilters.h"


int ZeroFunc( void *inData, int height, int width ){ memset( inData, 0, width * height * sizeof( uint8_t) ); return 0; }
int ZeroFuncFP( void *inData, int height, int width ){ memset( inData, 0, width * height * sizeof( float ) );  return 0; }
int Median( void *inData, int height, int width )
{
    int i;
    short *d = (short*) inData;
    
    for( i = 0; i < width * height; i++ )
        d[i] = 1;

	return width * height;
}
int MedianFP( void *inData, int height, int width )
{
    int i;
    float *d = (float*) inData;
    float value = 1.0f / (float)( width * height);
    
    for( i = 0; i < width * height; i++ )
        d[i] = value;

	return 0;
}
int Emboss( void *inData, int height, int width )
{
    int i;
    short *d = (short*) inData;
    
	*d++ = 2;
    for( i = 1; i < width * height - 1; i++ )
        *d++ = 0;
	*d = -2;

	return 1;
}
int EmbossFP( void *inData, int height, int width )
{
    int i;
    float *d = (float*) inData;
	
	*d++ = 2.0;
    for( i = 1; i < width * height - 1; i++ )
        *d++ = 0.0;
	*d = -2.0;

	return 0;
}

int MotionBlurFP( void *inData, int height, int width )
{
    float *d = (float*) inData;
    int x, y;
    int xSpan = (width-1) / 2;
    int ySpan = (height-1) / 2;
    float slope = (float) ySpan / (float) xSpan;
    
    //zero the buffer
    memset( inData, 0, height * width * sizeof( float ) );

    //Set the center to 1
    d[ width * ySpan + xSpan] = 1.0f;
    for( x = 1; x <= xSpan; x++ )
    {
        float fy =  slope * (float) x;
        float fract = fy - floor( fy );
        float distance = (float) x * (float) x + fy * fy + 1.0f;
        y = fy;
        d[ (ySpan - y) * width + xSpan + x ] = (1.0f - fract) / distance;
        if( y < ySpan )
            d[ (ySpan - y - 1) * width + xSpan + x ] = fract / distance;

    }
    
    return 1;
}

int MotionBlur( void *inData, int height, int width )
{
    short *d = (short*) inData;
    int x, y;
    int xSpan = (width-1) / 2;
    int ySpan = (height-1) / 2;
    float slope = (float) ySpan / (float) xSpan;
    
    //zero the buffer
    memset( inData, 0, height * width * sizeof( short ) );

    //Set the center to 1
    d[ width * ySpan + xSpan] = 16384;
    for( x = 1; x <= xSpan; x++ )
    {
        float fy =  slope * (float) x;
        float fract = fy - floor( fy );
        float distance = (float) x * (float) x + fy * fy + 1.0f;
        y = fy;
        d[ (ySpan - y) * width + xSpan + x ] = 16384.0 * (1.0f - fract) / distance;
        if( y < ySpan )
            d[ (ySpan - y - 1) * width + xSpan + x ] = 16384.0f * fract / distance;
    }
    
    return 16384;
}

int Blur( void *inData, int height, int width )
{
    short *d = (short*) inData;
    int sum = 0;
    int x, y, value;
    
    for( x = 0; x <= width / 2; x++ )
    {
        for( y = 0; y <= height / 2; y++ )
        {
            value = x + y - width / 2;
            if( value >= 0 )
                value = 1 << ( value );
            else
                value = 0;
                
            d[ y * width + x ] = value;
            d[ y * width + width - x - 1 ] = value;
            d[ ( height - y - 1) * width + x ] = value;
            d[ ( height - y - 1) * width + width - x - 1 ] = value;
        }
    }
    
    for( x = 0; x < width * height; x ++ )
	sum += d[x];
        
    return sum;
        
/*	if ( height == 3 && width == 3 )
    {
		*d++ = 1;
		*d++ = 2;
		*d++ = 1;
		*d++ = 2;
		*d++ = 4;
		*d++ = 2;
		*d++ = 1;
		*d++ = 2;
		*d++ = 1;
		return 16;
	}
	else if ( height == 5 && width == 5 )
	{
		*d++ = 0;
		*d++ = 1;
		*d++ = 2;
		*d++ = 1;
		*d++ = 0;
		*d++ = 1;
		*d++ = 2;
		*d++ = 4;
		*d++ = 2;
		*d++ = 1;
		*d++ = 2;
		*d++ = 4;
		*d++ = 8;
		*d++ = 4;
		*d++ = 2;
		*d++ = 1;
		*d++ = 2;
		*d++ = 4;
		*d++ = 2;
		*d++ = 1;
		*d++ = 0;
		*d++ = 1;
		*d++ = 2;
		*d++ = 1;
		*d++ = 0;
		return 48;
	}
*/
	return 0;
}
int BlurFP( void *inData, int height, int width )
{
    float *d = (float*) inData;
	
	if ( height == 3 && width == 3 )
    {
		*d++ = 1.0/16.0;
		*d++ = 2.0/16.0;
		*d++ = 1.0/16.0;
		*d++ = 2.0/16.0;
		*d++ = 4.0/16.0;
		*d++ = 2.0/16.0;
		*d++ = 1.0/16.0;
		*d++ = 2.0/16.0;
		*d++ = 1.0/16.0;
	}
	else if ( height == 5 && width == 5 )
	{
		*d++ = 0.0;
		*d++ = 1.0/48.0;
		*d++ = 2.0/48.0;
		*d++ = 1.0/48.0;
		*d++ = 0.0;
		*d++ = 1.0/48.0;
		*d++ = 2.0/48.0;
		*d++ = 4.0/48.0;
		*d++ = 2.0/48.0;
		*d++ = 1.0/48.0;
		*d++ = 2.0/48.0;
		*d++ = 4.0/48.0;
		*d++ = 8.0/48.0;
		*d++ = 4.0/48.0;
		*d++ = 2.0/48.0;
		*d++ = 1.0/48.0;
		*d++ = 2.0/48.0;
		*d++ = 4.0/48.0;
		*d++ = 2.0/48.0;
		*d++ = 1.0/48.0;
		*d++ = 0.0;
		*d++ = 1.0/48.0;
		*d++ = 2.0/48.0;
		*d++ = 1.0/48.0;
		*d++ = 0.0;
		return 0;
	}

	return 0;
}

int gammaTable_3_4( void *inData, int height, int width )
{
    memset( inData, 0, width * height * sizeof( uint8_t ) );

    if( width > 0 && height >= 256 )
    {
        uint8_t *d = (uint8_t*) inData;
        int i;
    
        for( i = 0; i < 256; i++ )
        {
            double value = pow( (double) i / 255.0, 3.0/4.0);
            d[ i * width ] = (uint8_t)(value * 255.0 + 0.5);        
        }
    }
    
    return 0;
}

int gammaTable_4_3( void *inData, int height, int width )
{
    memset( inData, 0, width * height * sizeof( uint8_t ) );

    if( width > 0 && height >= 256 )
    {
        uint8_t *d = (uint8_t*) inData;
        int i;
    
        for( i = 0; i < 256; i++ )
        {
            double value = pow( (double) i / 255.0, 4.0/3.0);
            d[ i * width ] = (uint8_t)(value * 255.0 + 0.5);        
        }
    }

    return 0;
}

int ClipFP( void *inData, int height, int width )
{
    float *d = (float*) inData;
    
    if( NULL == inData )
        return 0;
        
    memset( inData, 0, width * height * sizeof( float ) );
    
    d[0] = 0.1f;
    if( width > 1 )
        d[1] = 0.9f;
        
    return 0;
}

int DefaultGeometryKernel( void *inData, int height, int width )
{
    TransformInfo *info = inData;
    
    info->xShear = 0.0f;			//degrees
    info->yShear = 0.0f;			//degrees
    info->xTranslation = 0.0f;		//fraction of the image width -1.0 ... 1.0
    info->yTranslation = 0.0f;		//fraction of the image width -1.0 ... 1.0
    info->xScale = 1.0f;			//log2 scale
    info->yScale = 1.0f;			//log2 scale
    info->rotate = 0.0f;
    info->a = info->r = info->g = info->b = 1.0f;		//background color
    info->transformMatrix.a = 1.0f;
    info->transformMatrix.b = 0.0f;
    info->transformMatrix.c = 0.0f;
    info->transformMatrix.d = 1.0f;
    info->transformMatrix.tx = 0.0f;
    info->transformMatrix.ty = 0.0f;
    info->quitting = NO;
    
    return 0;
}


//Specify defaults for Filter kernels

const unsigned char kKernelTypeSizes[] = { 	0, 	/* no kernel type */
                                                0, 	/* no kernel data */
                                                sizeof( int8_t),
                                                sizeof( uint8_t),
                                                sizeof( int16_t),
                                                sizeof( uint16_t),
                                                sizeof( int32_t),
                                                sizeof( uint32_t),
                                                sizeof( float),
                                                sizeof( double ),
                                                sizeof( TransformInfo ) 
                                            };


//Kernels.
// These are used to init the default values for kernels. The user can change the 
// kernel, but the changes will be applied to a copy. 

//Morphology 
#pragma mark Morphology kernels   

struct
{
    int 		count;
    KernelInitFunction	zeros;		
}morphologyInitFunctions = { 
                                1,
                                {
                                    ZeroFunc,
                                    ZeroFuncFP,
                                    3,
                                    3,
                                    kUInt8KernelType,
                                    kFloatKernelType,
                                    @"All Zeros"
                                }
                            };
    

//Convolution
#pragma mark Convolution kernels   

struct
{
    int 		count;
    KernelInitFunction	emboss;		
    KernelInitFunction	average;		
    KernelInitFunction	blur;
    KernelInitFunction  motionBlur;		
}convolutionInitFunctions = { 
                                4,
                                {
                                    Emboss,
                                    EmbossFP,
                                    3,
                                    3,
                                    kSInt16KernelType,
                                    kFloatKernelType,
                                    @"Emboss"
                                },
                                {
                                    Median,
                                    MedianFP,
                                    3,
                                    3,
                                    kSInt16KernelType,
                                    kFloatKernelType,
                                    @"Average"
                                },
                                {
                                    Blur,
                                    BlurFP,
                                    3,
                                    3,
                                    kSInt16KernelType,
                                    kFloatKernelType,
                                    @"Blur"
                                },
                                {
                                    MotionBlur,
                                    MotionBlurFP,
                                    3,
                                    3,
                                    kSInt16KernelType,
                                    kFloatKernelType,
                                    @"Motion Blur"
                                }
                            };
    

//Clipping
struct
{
    int 		count;
    KernelInitFunction	clip;		
}clipInitFunctions = { 1,
                                {
                                    NULL,
                                    ClipFP,
                                    1,
                                    2,
                                    kNoKernelData,
                                    kFloatKernelType,
                                    @"Clip Limits (min, max)"
                                }
                            };
                            
//Table lookup
struct
{
    int 		count;
    KernelInitFunction	gamma1_33;		
    KernelInitFunction	gamma0_75;		
}lookupFunctions = { 2,
                                {
                                    gammaTable_4_3,
                                    NULL,
                                    256,
                                    1,
                                    kUInt8KernelType,
                                    kNoKernelData,
                                    @"gamma 4/3"
                                },
                                {
                                    gammaTable_3_4,
                                    NULL,
                                    256,
                                    1,
                                    kUInt8KernelType,
                                    kNoKernelData,
                                    @"gamma 3/4"
                                }
                            };

//Geometry Filters
struct
{
    int 		count;
    KernelInitFunction	geometryDefault;		
}geometryFunctions = { 1,
                        {
                            DefaultGeometryKernel,
                            DefaultGeometryKernel,
                            1,
                            1,
                            kGeometryKernelType,
                            kGeometryKernelType,
                            @"(Geometry Data)"
                        }
                    };

//Specify the filters 
FilterInfo	testFilters[] = 
    {
        { InvertFilter,	 InvertFilterFP, InvertFilter_ARGB, InvertFilterFP_ARGB, @"Invert", kNoFilterFlags, NULL },
        { BlackFilter, BlackFilterFP, BlackFilter_ARGB, BlackFilterFP_ARGB, @"Black", kNoFilterFlags, NULL },
        { CopyFilter, CopyFilterFP, CopyFilter_ARGB, CopyFilterFP_ARGB, @"memcpy", kNoFilterFlags, NULL },
        { NULL, NULL, NULL, RedFilterFP_ARGB, @"red", kNoFilterFlags, NULL },
        { NULL, NULL, NULL, GreenFilterFP_ARGB, @"green", kNoFilterFlags, NULL },
        { NULL, NULL, NULL, BlueFilterFP_ARGB, @"blue", kNoFilterFlags, NULL }
    };


FilterInfo	morphologyFilters[] = 
    {// int scalar	FP scalar	int scalar ARGB		FP scalar ARGB		func name	data format	Init Functions		
     // ----------	---------	---------------		--------------		---------	-----------	--------------
        { DilateFilter,	DilateFilterFP, DilateFilter_ARGB,	DilateFilterFP_ARGB,	@"Dilate",	kNoFilterFlags, (KernelInitFunctionList*) &morphologyInitFunctions }, 	
        { ErodeFilter,	ErodeFilterFP,	ErodeFilter_ARGB,	ErodeFilterFP_ARGB, 	@"Erode",	kNoFilterFlags, (KernelInitFunctionList*) &morphologyInitFunctions }, 	
        { MaxFilter,	MaxFilterFP, 	MaxFilter_ARGB,		MaxFilterFP_ARGB, 	@"Max",		kNoFilterFlags, (KernelInitFunctionList*) &morphologyInitFunctions }, 	
        { MinFilter,	MinFilterFP, 	MinFilter_ARGB,		MinFilterFP_ARGB, 	@"Min",		kNoFilterFlags, (KernelInitFunctionList*) &morphologyInitFunctions } 	
    };
                                
FilterInfo	convolutionFilters[] = 
    {// int scalar		FP scalar		int scalar ARGB		FP scalar ARGB		func name			data format		Init Functions			
     // ----------		---------		---------------		--------------		---------			-----------		-------------------	
        { Convolution,	ConvolutionFP,	Convolution_ARGB,	ConvolutionFP_ARGB,	@"Convolution",		kNoFilterFlags, 	(KernelInitFunctionList*) &convolutionInitFunctions }, 	
        { NULL,			NULL,			NULL,			NULL,			@"Convolution_Separable",	kNoFilterFlags,		(KernelInitFunctionList*) &convolutionInitFunctions } 	
    };

FilterInfo	geometryFilters[] =
    {
        { ShearXFilter, 	ShearXFilterFP,	ShearXFilter_ARGB, 	ShearXFilterFP_ARGB,	@"Horiz Shear...",	kTranslateX | kShearX | kScaleX, 	(KernelInitFunctionList*) &geometryFunctions },
        { ShearYFilter, 	ShearYFilterFP,	ShearYFilter_ARGB, 	ShearYFilterFP_ARGB,	@"Vert Shear...",	kTranslateY | kShearY | kScaleY, 	(KernelInitFunctionList*) &geometryFunctions },
        { RotateFilter, 	RotateFilterFP,	RotateFilter_ARGB, 	RotateFilterFP_ARGB,	@"Rotate...",	 kRotate,  	(KernelInitFunctionList*) &geometryFunctions },
        { ScaleFilter, 		ScaleFilterFP,	ScaleFilter_ARGB, 	ScaleFilterFP_ARGB,	@"Scale...",	kScaleX | kScaleY, 	(KernelInitFunctionList*) &geometryFunctions },
        { AffineTransformFilter, AffineTransformFilterFP, AffineTransformFilter_ARGB, 	AffineTransformFilterFP_ARGB,	@"Warp Affine...",	kTranslateX | kTranslateY | kRotate | kScaleX | kScaleY | kShearX | kShearY, 	(KernelInitFunctionList*) &geometryFunctions },
        { ReflectXFilter, 	ReflectXFilterFP, ReflectXFilter_ARGB, 	ReflectXFilterFP_ARGB,	@"Reflect Horiz",	kNoFilterFlags, 	NULL },
        { ReflectYFilter, 	ReflectYFilterFP, ReflectYFilter_ARGB, 	ReflectYFilterFP_ARGB,	@"Reflect Vert",	kNoFilterFlags, 	NULL }
    };

FilterInfo	histogramFilters[] =
    {
        { Histogram, 	HistogramFP,	Histogram_ARGB, 	HistogramFP_ARGB,	@"Histogram",	kNoFilterFlags, 	NULL },
        { Histogram_Equalization, 	Histogram_EqualizationFP,	Histogram_Equalization_ARGB, 	Histogram_EqualizationFP_ARGB,	@"Hist Equalization",	kNoFilterFlags, 	NULL },
        { Histogram_Specification, 	Histogram_SpecificationFP,	Histogram_Specification_ARGB, 	Histogram_SpecificationFP_ARGB,	@"Hist Specification",	kNoFilterFlags, 	NULL },
        { Histogram_Contrast_Stretch, 	Histogram_Contrast_StretchFP,	Histogram_Contrast_Stretch_ARGB, 	Histogram_Contrast_StretchFP_ARGB,	@"Contrast Stretch",	kNoFilterFlags, 	NULL },
        { Histogram_Ends_In_Contrast_Stretch, 	Histogram_Ends_In_Contrast_StretchFP,	Histogram_Ends_In_Contrast_Stretch_ARGB, 	Histogram_Ends_In_Contrast_StretchFP_ARGB,	@"Ends-In Stretch",	kNoFilterFlags, 	NULL }
    };

FilterInfo	alphaFilters[] =
    {
        { Alpha_Planar8, Alpha_PlanarF, Alpha_ARGB8888, Alpha_ARGBFFFF, @"Alpha Compositing...", kAlphaKernelType, NULL },
        { Alpha_Premultiplied_Planar8, Alpha_Premultiplied_PlanarF, Alpha_Premultiplied_ARGB8888, Alpha_Premultiplied_ARGBFFFF, @"Premultiplied Alpha Compositing...", kAlphaKernelType, NULL }    
    };
    
FilterInfo	ConvertFilters[] =
    {
        { Convert_Planar8ToARGB8888, Convert_PlanarFToARGBFFFF, Convert_ARGB8888ToPlanar8, Convert_ARGBFFFFToPlanarF, @"Data Conversion", kNoFilterFlags, NULL },
    };
    
FilterInfo	otherFilters[] =
    {
//        { NULL, 	FFTFilter_FP,	NULL, 	NULL,	@"2D FFT",	kNoFilterFlags, 	NULL },
//        { NULL, 	IFFTFilter_FP,	NULL, 	NULL,	@"2D IFFT",	kNoFilterFlags, 	NULL },
        { NULL, 	ClipFilter_FP, 	NULL, 	ClipFilterFP_ARGB, @"Clip (FP)", kNoFilterFlags, (KernelInitFunctionList*) &clipInitFunctions },
        { Lookup, 	NULL, 	Lookup_ARGB, 	NULL,	@"Lookup (int)", kNoFilterFlags, (KernelInitFunctionList*) &lookupFunctions }
    };


//Put it all together in the master struct. Entries here define the high level filter category menus.
FilterList 	filterLists[] = 
    {
        { testFilters,		@"Simple Test Filters",	sizeof( testFilters ) / sizeof( FilterInfo ) },
        { morphologyFilters,	@"Morphology Filters",	sizeof( morphologyFilters ) / sizeof( FilterInfo ) },
        { convolutionFilters,	@"Convolution Filters",	sizeof( convolutionFilters ) / sizeof( FilterInfo ) },
        { geometryFilters,	@"Geometry Filters",	sizeof( geometryFilters ) / sizeof( FilterInfo ) },
        { histogramFilters,	@"Histogram Filters",	sizeof( histogramFilters ) / sizeof( FilterInfo ) },
        { alphaFilters,		@"Alpha Compositing",	sizeof( alphaFilters ) / sizeof( FilterInfo ) },
        { ConvertFilters,	@"Format Conversion",	sizeof( ConvertFilters ) / sizeof( FilterInfo ) },
        { otherFilters,		@"Other Filters",	sizeof( otherFilters ) / sizeof( FilterInfo ) }
    };
                                
int		kListCount = sizeof( filterLists ) / sizeof( FilterList );

