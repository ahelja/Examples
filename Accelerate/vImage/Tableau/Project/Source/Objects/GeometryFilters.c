/*
 *  GeometryFilters.c
 *  Tableau
 *
 *  Created by Ian Ollmann on Fri Feb 14 2003.
 *  Copyright (c) 2003 Apple Computer. All rights reserved.
 *
 */

#include <objc/objc.h>

#include "GeometryFilters.h"
#include <math.h>
#include <limits.h>


float			gKernelScale = -INFINITY;
ResamplingFilter	gKernel = NULL;


int ShearXFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float translate = info->xTranslation;
    float shear = info->xShear;
    float scale = info->xScale;
    Pixel_8 backColor = UCHAR_MAX * (&info->a)[ colorChannel ];
    
    if( scale != gKernelScale )
    {
        vImageDestroyResamplingFilter( gKernel );
        gKernel = vImageNewResamplingFilter( scale, kvImageNoFlags );
        gKernelScale = scale;
    }

    return vImageHorizontalShear_Planar8( &in, &out, 0, 0, translate, shear, gKernel, backColor, flags );
}

int ShearYFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float translate = info->yTranslation;
    float shear = info->yShear;
    float scale = info->yScale;
    Pixel_8 backColor = UCHAR_MAX * (&info->a)[ colorChannel ];

    if( scale != gKernelScale )
    {
        vImageDestroyResamplingFilter( gKernel );
        gKernel = vImageNewResamplingFilter( scale, kvImageNoFlags );
        gKernelScale = scale;
    }

    return vImageVerticalShear_Planar8( &in, &out, 0, 0, translate, shear, gKernel, backColor, flags );
}

int RotateFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float angle = info->rotate * 2.0f * M_PI / 360.0f;
    Pixel_8 backColor = UCHAR_MAX * (&info->a)[ colorChannel ];

    return vImageRotate_Planar8( &in, &out, NULL, angle, backColor, flags );
}

int ScaleFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, 0, 0, outRowBytes };
    TransformInfo *info = kernel; 
    float newHeight = height * info->yScale;	//Our slider uses powers of 2
    float newWidth = width * info->xScale;		//Our slider uses powers of 2
    
    out.height = newHeight;
    out.width = newWidth;
    
    return vImageScale_Planar8( &in, &out, NULL, flags); 
}

int AffineTransformFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    Pixel_8 backColor = UCHAR_MAX * (&info->a)[ colorChannel ];

    return vImageAffineWarp_Planar8( &in, &out, NULL, &info->transformMatrix, backColor, flags );
}

int ReflectXFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };

    return vImageHorizontalReflect_Planar8( &in, &out, flags );
}

int ReflectYFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };

    return vImageVerticalReflect_Planar8( &in, &out, flags );
}


int ShearXFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float translate = info->xTranslation;
    float shear = info->xShear;
    float scale = info->xScale;
    Pixel_F backColor = (&info->a)[ colorChannel ];

    if( scale != gKernelScale )
    {
        vImageDestroyResamplingFilter( gKernel );
        gKernel = vImageNewResamplingFilter( scale, kvImageNoFlags );
        gKernelScale = scale;
    }

    return vImageHorizontalShear_PlanarF( &in, &out, 0, 0, translate, shear, gKernel, backColor, flags );
}

int ShearYFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float translate = info->yTranslation;
    float shear = info->yShear;
    float scale = info->yScale;
    Pixel_F backColor = (&info->a)[ colorChannel ];

    if( scale != gKernelScale )
    {
        vImageDestroyResamplingFilter( gKernel );
        gKernel = vImageNewResamplingFilter( scale, kvImageNoFlags );
        gKernelScale = scale;
    }

    return vImageVerticalShear_PlanarF( &in, &out, 0, 0, translate,  shear, gKernel, backColor, flags );
}

int RotateFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float angle = info->rotate * 2.0f * M_PI / 360.0f;
    Pixel_F backColor = (&info->a)[ colorChannel ];

    return vImageRotate_PlanarF( &in, &out, NULL, angle, backColor, flags );
}

int ScaleFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, 0, 0, outRowBytes };
    TransformInfo *info = kernel; 
    float newHeight = height * info->yScale;	//Our slider uses powers of 2
    float newWidth = width * info->xScale;		//Our slider uses powers of 2
    
    out.height = newHeight;
    out.width = newWidth;
    
    return vImageScale_PlanarF( &in, &out, NULL, flags ); 
}

int AffineTransformFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    Pixel_F backColor = 1.0f;
    TransformInfo *info = kernel; 

    return vImageAffineWarp_PlanarF( &in, &out, NULL, &info->transformMatrix, backColor, flags );
}

int ReflectXFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };

    return vImageHorizontalReflect_PlanarF( &in, &out, flags );
}

int ReflectYFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };

    return vImageVerticalReflect_PlanarF( &in, &out, flags );
}


int ShearXFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float translate = info->xTranslation;
    float shear = info->xShear;
    float scale = info->xScale;
    Pixel_8888 backColor;
    
    backColor[0] = UCHAR_MAX * info->a;
    backColor[1] = UCHAR_MAX * info->r;
    backColor[2] = UCHAR_MAX * info->g;
    backColor[3] = UCHAR_MAX * info->b;

    if( scale != gKernelScale )
    {
        vImageDestroyResamplingFilter( gKernel );
        gKernel = vImageNewResamplingFilter( scale, kvImageNoFlags );
        gKernelScale = scale;
    }

    return vImageHorizontalShear_ARGB8888( &in, &out, 0, 0, translate, shear, gKernel, backColor, flags );
}

int ShearYFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{

    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float translate = info->yTranslation;
    float shear = info->yShear;
    float scale = info->yScale;
    Pixel_8888 backColor;
    
    backColor[0] = UCHAR_MAX * info->a;
    backColor[1] = UCHAR_MAX * info->r;
    backColor[2] = UCHAR_MAX * info->g;
    backColor[3] = UCHAR_MAX * info->b;
 
    if( scale != gKernelScale )
    {
        vImageDestroyResamplingFilter( gKernel );
        gKernel = vImageNewResamplingFilter( scale, kvImageNoFlags );
        gKernelScale = scale;
    }

    return vImageVerticalShear_ARGB8888( &in, &out, 0, 0, translate, shear, gKernel, backColor, flags );
}

int RotateFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float angle = info->rotate * 2.0f * M_PI / 360.0f;
    Pixel_8888 backColor;
    
    backColor[0] = UCHAR_MAX * info->a;
    backColor[1] = UCHAR_MAX * info->r;
    backColor[2] = UCHAR_MAX * info->g;
    backColor[3] = UCHAR_MAX * info->b;

    return vImageRotate_ARGB8888( &in, &out, NULL, angle, backColor, flags );
}

int ScaleFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, 0, 0, outRowBytes };
    TransformInfo *info = kernel; 
    float newHeight = height * info->yScale;	//Our slider uses powers of 2
    float newWidth = width * info->xScale;		//Our slider uses powers of 2
    
    out.height = newHeight;
    out.width = newWidth;
    
    return vImageScale_ARGB8888( &in, &out, NULL, flags ); 
}

int AffineTransformFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel; 
    Pixel_8888 backColor;
    
    backColor[0] = UCHAR_MAX * info->a;
    backColor[1] = UCHAR_MAX * info->r;
    backColor[2] = UCHAR_MAX * info->g;
    backColor[3] = UCHAR_MAX * info->b;

    return vImageAffineWarp_ARGB8888( &in, &out, NULL, &info->transformMatrix, backColor, flags );
}

int ReflectXFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };

    return vImageHorizontalReflect_ARGB8888( &in, &out, flags );
}

int ReflectYFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };

    return vImageVerticalReflect_ARGB8888( &in, &out, flags );
}


int ShearXFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float translate = info->xTranslation;
    float shear = info->xShear;
    float scale = info->xScale;
    Pixel_FFFF backColor = { info->a, info->r, info->g, info->b };

    if( scale != gKernelScale )
    {
        vImageDestroyResamplingFilter( gKernel );
        gKernel = vImageNewResamplingFilter( scale, kvImageNoFlags );
        gKernelScale = scale;
    }

    return vImageHorizontalShear_ARGBFFFF( &in, &out, 0, 0, translate, shear, gKernel, backColor, flags );
}

int ShearYFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float translate = info->yTranslation;
    float shear = info->yShear;
    float scale = info->yScale;
    Pixel_FFFF backColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    if( scale != gKernelScale )
    {
        vImageDestroyResamplingFilter( gKernel );
        gKernel = vImageNewResamplingFilter( scale, kvImageNoFlags );
        gKernelScale = scale;
    }

    return vImageVerticalShear_ARGBFFFF( &in, &out, 0, 0, translate, shear, gKernel, backColor, flags );
}

int RotateFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    TransformInfo *info = kernel;
    float angle = info->rotate * 2.0f * M_PI / 360.0f;
    Pixel_FFFF backColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    return vImageRotate_ARGBFFFF( &in, &out, NULL, angle, backColor, flags );
}

int ScaleFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, 0, 0, outRowBytes };
    TransformInfo *info = kernel; 
    float newHeight = height * info->yScale;	//Our slider uses powers of 2
    float newWidth = width * info->xScale;		//Our slider uses powers of 2
    
    out.height = newHeight;
    out.width = newWidth;
    
    return vImageScale_ARGBFFFF( &in, &out, NULL, flags ); 
}

int AffineTransformFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };
    Pixel_FFFF backColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    TransformInfo *info = kernel; 

    return vImageAffineWarp_ARGBFFFF( &in, &out, NULL, &info->transformMatrix, backColor, flags );
}

int ReflectXFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };

    return vImageHorizontalReflect_ARGBFFFF( &in, &out, flags );
}

int ReflectYFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags )
{
    vImage_Buffer in = { inData, height, width, inRowBytes };
    vImage_Buffer out = { outData, height, width, outRowBytes };

    return vImageVerticalReflect_ARGBFFFF( &in, &out, flags );
}

