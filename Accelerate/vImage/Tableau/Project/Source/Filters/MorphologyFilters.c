/*
 *  MorphologyFilters.c
 *  Erode
 *
 *  Created by Ian Ollmann on Wed Nov 06 2002.
 *  Copyright (c) 2002 Apple. All rights reserved.
 *
 */

#include "MorphologyFilters.h"
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

#ifndef USING_ACCELERATE
    #include <vImage/Morphology.h>
#endif

#ifndef FLT_MAX
    #define FLT_MAX  3.40282347e+38F
#endif

int bufferSize = 0;
void *tempBuffer = NULL;


// This filter simply copies data from in to out, inverting black for white and colors inbetween
int InvertFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #pragma unused( kernel )
    #pragma unused( kernel_height )
    #pragma unused( kernel_width )

    unsigned int i, j;
    char *src = (char*) inData;
    char *dest = (char*) outData;
    
    for( i = 0; i < height; i++ )
    {
        for( j = 0; j < width; j++ ) 
            dest[j] = 255 - src[j];
        src += inRowBytes;
        dest += outRowBytes;
    }
    
    return kvImageNoError;
} 	

int InvertFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #pragma unused( kernel )
    #pragma unused( kernel_height )
    #pragma unused( kernel_width )

    int i, j;
    float *src = (float*) inData;
    float *dest = (float*) outData;
    
    for( i = 0; i < (int) height; i++ )
    {
        for( j = 0; j < (int) width; j++ ) 
            dest[j] = 1.0f - src[j];
        src = (float*) ((char*) src + inRowBytes);
        dest = (float*) ((char*) dest + outRowBytes);
    }
    
    return kvImageNoError;
} 	

// This filter simply copies data from in to out, inverting black for white and colors inbetween
int InvertFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    return InvertFilter( inData, inRowBytes, outData, outRowBytes, height, width * 4, kernel, kernel_height, kernel_width, divisor, flags );
} 	

int InvertFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    return InvertFilterFP( inData, inRowBytes, outData, outRowBytes, height, width * 4, kernel, kernel_height, kernel_width, divisor, flags );
} 	

// This filter simply copies data from in to out, inverting black for white and colors inbetween
int BlackFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #pragma unused( kernel )
    #pragma unused( kernel_height )
    #pragma unused( kernel_width )

    unsigned int i, j;
    char *src = (char*) inData;
    char *dest = (char*) outData;
    
    for( i = 0; i < height; i++ )
    {
        for( j = 0; j < width; j++ ) 
            dest[j] = 0;
        src += inRowBytes;
        dest += outRowBytes;
    }
    
    return kvImageNoError;
} 	

int BlackFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #pragma unused( kernel )
    #pragma unused( kernel_height )
    #pragma unused( kernel_width )

    int i, j;
    float *src = (float*) inData;
    float *dest = (float*) outData;
    
    for( i = 0; i < (int) height; i++ )
    {
        for( j = 0; j < (int) width; j++ ) 
            dest[j] = 0.0f;
        src = (float*) ((char*) src + inRowBytes);
        dest = (float*) ((char*) dest + outRowBytes);
    }
    
    return kvImageNoError;
} 	

// This filter simply copies data from in to out, inverting black for white and colors inbetween
int BlackFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    return InvertFilter( inData, inRowBytes, outData, outRowBytes, height, width * 4, kernel, kernel_height, kernel_width, divisor, flags );
} 	

int BlackFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    return InvertFilterFP( inData, inRowBytes, outData, outRowBytes, height, width * 4, kernel, kernel_height, kernel_width, divisor, flags );
} 	

int CopyFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #define TYPE u_int8_t
    
    int i;
    TYPE *in = (TYPE*) inData; 
    TYPE *out = (TYPE*) outData; 
    size_t rowSize = width * sizeof( TYPE );
    
    for( i = 0; i < (int) height; i++ )
    {
        memcpy( out, in, rowSize );
    
        in = (TYPE*) ( (char*) in + inRowBytes );
        out = (TYPE*) ( (char*) out + outRowBytes );
    }

    return 0;
    #undef TYPE
}

int RedFilterFP_ARGB( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #define TYPE float
    int x, y;
    
    TYPE *in = inData;
    TYPE *out = outData;
    
    for( y = 0; y < (int) height; y++ )
    {
        TYPE *src = in;
        TYPE *dest = out;
        
        for( x = 0; x < (int) width; x++ )
        {
            dest[4*x] = 0.0f;
            dest[4*x+1] = src[4*x+1];
            dest[4*x+2] = 0.0f;
            dest[4*x+3] = 0.0f;
        }
        
        in = (TYPE*) ((char*) in + inRowBytes );
        out = (TYPE*) ((char*) out + outRowBytes );
    }

    return 0;
    #undef TYPE
}

int GreenFilterFP_ARGB( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #define TYPE float
    int x, y;
    
    TYPE *in = inData;
    TYPE *out = outData;
    
    for( y = 0; y < (int) height; y++ )
    {
        TYPE *src = in;
        TYPE *dest = out;
        
        for( x = 0; x < (int) width; x++ )
        {
            dest[4*x] = 0.0f;
            dest[4*x+1] = 0.0f;
            dest[4*x+2] = src[4*x+2];
            dest[4*x+3] = 0.0f;
        }
        
        in = (TYPE*) ((char*) in + inRowBytes );
        out = (TYPE*) ((char*) out + outRowBytes );
    }

    return 0;
    #undef TYPE
}

int BlueFilterFP_ARGB( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #define TYPE float
    int x, y;
    
    TYPE *in = inData;
    TYPE *out = outData;
    
    for( y = 0; y < (int) height; y++ )
    {
        TYPE *src = in;
        TYPE *dest = out;
        
        for( x = 0; x < (int) width; x++ )
        {
            dest[4*x] = 0.0f;
            dest[4*x+1] = 0.0f;
            dest[4*x+2] = 0.0f;
            dest[4*x+3] = src[4*x+3];
        }
        
        in = (TYPE*) ((char*) in + inRowBytes );
        out = (TYPE*) ((char*) out + outRowBytes );
    }

    return 0;
    #undef TYPE
}

int CopyFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #define TYPE float
    
    int i;
    TYPE *in = (TYPE*) inData; 
    TYPE *out = (TYPE*) outData; 
    size_t rowSize = width * sizeof( TYPE );
    
    for( i = 0; i < (int) height; i++ )
    {
        memcpy( out, in, rowSize );
    
        in = (TYPE*) ( (char*) in + inRowBytes );
        out = (TYPE*) ( (char*) out + outRowBytes );
    }

    return 0;
    #undef TYPE
}

int CopyFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #define TYPE u_int8_t
    
    int i;
    TYPE *in = (TYPE*) inData; 
    TYPE *out = (TYPE*) outData; 
    size_t rowSize = width * sizeof( TYPE ) * 4;
    
    for( i = 0; i < (int) height; i++ )
    {
        memcpy( out, in, rowSize );
    
        in = (TYPE*) ( (char*) in + inRowBytes );
        out = (TYPE*) ( (char*) out + outRowBytes );
    }

    return 0;
    #undef TYPE
}

int CopyFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    #define TYPE float
    
    int i;
    TYPE *in = (TYPE*) inData; 
    TYPE *out = (TYPE*) outData; 
    size_t rowSize = width * sizeof( TYPE ) * 4;
    
    for( i = 0; i < (int) height; i++ )
    {
        memcpy( out, in, rowSize );
    
        in = (TYPE*) ( (char*) in + inRowBytes );
        out = (TYPE*) ( (char*) out + outRowBytes );
    }

    return 0;
    #undef TYPE
}


int DilateFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;

    return  vImageDilate_Planar8( &in, &out, xOffset, yOffset, kernel, kernel_height, kernel_width, flags );
}

int ErodeFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;

    return  vImageErode_Planar8( &in, &out, xOffset, yOffset, kernel, kernel_height, kernel_width, flags );
}

int MaxFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_8) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMax_Planar8( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}

int MinFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;

    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_8) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMin_Planar8( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}

int DilateFilterFP( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;

    return  vImageDilate_PlanarF( &in, &out, xOffset, yOffset, kernel, kernel_height, kernel_width, flags );
}

int ErodeFilterFP( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;

    return  vImageErode_PlanarF( &in, &out, xOffset, yOffset, kernel, kernel_height, kernel_width, flags );
}

int MaxFilterFP( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_F) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMax_PlanarF( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}

int MinFilterFP( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_F) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMin_PlanarF( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}


int DilateFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;

    return  vImageDilate_ARGB8888( &in, &out, xOffset, yOffset, kernel, kernel_height, kernel_width, flags );
}

int ErodeFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;

    return  vImageErode_ARGB8888( &in, &out, xOffset, yOffset, kernel, kernel_height, kernel_width, flags );
}

int MaxFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_8888) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMax_ARGB8888( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}

int MinFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_8888) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMin_ARGB8888( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}

int DilateFilterFP_ARGB( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;

    return  vImageDilate_ARGBFFFF( &in, &out, xOffset, yOffset, kernel, kernel_height, kernel_width, flags );
}

int ErodeFilterFP_ARGB( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;

    return  vImageErode_ARGBFFFF( &in, &out, xOffset, yOffset, kernel, kernel_height, kernel_width, flags );
}

int MaxFilterFP_ARGB( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_FFFF) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMax_ARGBFFFF( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}

int MinFilterFP_ARGB( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_FFFF) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMin_ARGBFFFF( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}
