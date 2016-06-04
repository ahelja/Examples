/*
 *  ConvolutionFilters.c
 *  Erode
 *
 *  Created by Ian Ollmann on Wed Nov 06 2002.
 *  Copyright (c) 2002 Apple. All rights reserved.
 *
 */

#ifndef USING_ACCELERATE
    #include <vImage/vImage.h>
#endif

int Convolution(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };
    

    err = vImageConvolve_Planar8(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
                                        NULL,
                                        0,	//unsigned int srcOffsetToROI_X, 
                                        0,	//unsigned int srcOffsetToROI_Y,  
                                        kernel,	//const signed int *kernel, 
                                        kernel_height, 	//unsigned int kernel_height, 
                                        kernel_width,	//unsigned int kernel_width,
                                        divisor,	//int 
                                        0,
                                        kvImageEdgeExtend | flags //vImage_Flags flags 
                                    );

        
    return err;
}


int ConvolutionFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageConvolve_PlanarF(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
                                        NULL,
                                        0,	//unsigned int srcOffsetToROI_X, 
                                        0,	//unsigned int srcOffsetToROI_Y,  
                                        kernel,	//const signed int *kernel, 
                                        kernel_height, 	//unsigned int kernel_height, 
                                        kernel_width,	//unsigned int kernel_width,
                                        0,
                                        kvImageBackgroundColorFill | flags	//vImage_Flags flags 
                                    );
    
    return err;
}


int Convolution_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
	unsigned char edgeFill[4] = { 0, 0, 0, 0 };
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };
    

    err = vImageConvolve_ARGB8888(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
                                        NULL,
                                        0,	//unsigned int srcOffsetToROI_X, 
                                        0,	//unsigned int srcOffsetToROI_Y,  
                                        kernel,	//const signed int *kernel, 
                                        kernel_height, 	//unsigned int kernel_height, 
                                        kernel_width,	//unsigned int kernel_width,
                                        divisor,	//int 
                                        edgeFill,
                                        kvImageBackgroundColorFill | flags	//vImage_Flags flags 
                                    );

        
    return err;
}


int ConvolutionFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
	float edgeFill[4] = { 0, 0, 0, 0 };
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageConvolve_ARGBFFFF(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest,
                                        NULL, 
                                        0,	//unsigned int srcOffsetToROI_X, 
                                        0,	//unsigned int srcOffsetToROI_Y,  
                                        kernel,	//const signed int *kernel, 
                                        kernel_height, 	//unsigned int kernel_height, 
                                        kernel_width,	//unsigned int kernel_width,
                                        edgeFill,
                                        kvImageBackgroundColorFill | flags	//vImage_Flags flags 
                                    );
    
    return err;
}


