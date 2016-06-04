/*
 *  AlphaFilters.h
 *  Tableau
 *
 *  Created by iano on Tue May 27 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef USING_ACCELERATE
    #include <vImage/vImage_Types.h>
#endif

//Kernel info for a Geometry function
typedef struct
{
    float	alpha;			
    void	*alphaBuffer;
}AlphaInfo;

int Alpha_Planar8(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  
int Alpha_PlanarF(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  
int Alpha_ARGB8888(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  
int Alpha_ARGBFFFF(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int Alpha_Premultiplied_Planar8(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  
int Alpha_Premultiplied_PlanarF(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  
int Alpha_Premultiplied_ARGB8888(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  
int Alpha_Premultiplied_ARGBFFFF(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  
