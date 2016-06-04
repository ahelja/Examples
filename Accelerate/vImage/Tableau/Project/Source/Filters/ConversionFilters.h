/*
 * ConversionFilters.h
 *
 *  Created by Robert Murley on May 29 2003.
 *  Copyright (c) 2003 Apple. All rights reserved.
 *
 */

#ifndef USING_ACCELERATE
    #include <vImage/vImage_Types.h>
#endif
int Convert_Planar8ToARGB8888(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags );
int Convert_ARGB8888ToPlanar8(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags );
int Convert_PlanarFToARGBFFFF(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags );
int Convert_ARGBFFFFToPlanarF(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags );

