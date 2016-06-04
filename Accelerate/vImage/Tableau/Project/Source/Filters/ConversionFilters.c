/*
 *  ConvolutionFilters.c
 *  Erode
 *
 *  Created by Robert Murley on May 29 2003.
 *  Copyright (c) 2003 Apple. All rights reserved.
 *
 */
#ifndef USING_ACCELERATE
    #include <vImage/vImage.h>
#endif

#include <CoreServices/CoreServices.h>

int Convert_Planar8ToARGB8888(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error	err = kvImageNoError;
        
    return err;
}

int Convert_ARGB8888ToPlanar8(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	src = { inData, height, width, inRowBytes };
    vImage_Buffer	dest = { outData, height, width, outRowBytes };
	vImage_Buffer	planeA = { NULL, height, width,  width };
	vImage_Buffer	planeR = { NULL, height, width, width };
	vImage_Buffer	planeG = { NULL, height, width, width };
	vImage_Buffer	planeB = { NULL, height, width, width };
	size_t			dataSize = height * width;
	Pixel_8			*dataBfr;
    vImage_Error	err = kvImageNoError;
	
	dataBfr = malloc( 4 * ( dataSize + 16 ) );
	if ( !dataBfr )
		return kvImageMemoryAllocationError;
	
	planeA.data = dataBfr;
	planeR.data = (void *) ( ( (int) dataBfr + dataSize +16 ) & ~0x0f );
	planeG.data = (void *) ( ( (int) planeR.data + dataSize +16 ) & ~0x0f );
	planeB.data = (void *) ( ( (int) planeG.data + dataSize +16 ) & ~0x0f );
	
	err = vImageConvert_ARGB8888toPlanar8( &src, &planeA, &planeR, &planeG, &planeB, flags);
	if ( err == kvImageNoError )
		err = vImageConvert_Planar8toARGB8888( &planeA, &planeR, &planeG, &planeB, &dest, flags);

    return err;
}

int Convert_PlanarFToARGBFFFF(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error	err = kvImageNoError;
        
    return err;
}

int Convert_ARGBFFFFToPlanarF(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	src = { inData, height, width, inRowBytes };
    vImage_Buffer	dest = { outData, height, width, outRowBytes };
	vImage_Buffer	planeA = { NULL, height, width,  width * sizeof(float) };
	vImage_Buffer	planeR = { NULL, height, width, width * sizeof(float) };
	vImage_Buffer	planeG = { NULL, height, width, width * sizeof(float) };
	vImage_Buffer	planeB = { NULL, height, width, width * sizeof(float) };
	size_t			dataSize = height * width * sizeof(float);
	Pixel_F			*dataBfr;
    vImage_Error	err = kvImageNoError;
	
	dataBfr = (Pixel_F *) malloc( 4 * ( dataSize + 16 ) );
	if ( !dataBfr )
		return kvImageMemoryAllocationError;
	
	planeA.data = dataBfr;
	planeR.data = (void *) ( ( (int) dataBfr + dataSize +16 ) & ~0x0f );
	planeG.data = (void *) ( ( (int) planeR.data + dataSize +16 ) & ~0x0f );
	planeB.data = (void *) ( ( (int) planeG.data + dataSize +16 ) & ~0x0f );
	
	err = vImageConvert_ARGBFFFFtoPlanarF( &src, &planeA, &planeR, &planeG, &planeB, flags);
	if ( err == kvImageNoError )
		err = vImageConvert_PlanarFtoARGBFFFF( &planeA, &planeR, &planeG, &planeB, &dest, flags);
        
    return err;
}



