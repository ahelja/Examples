/*
 *  MiscFilters.c
 *  Tableau
 *
 *  Created by iano on Thu Feb 06 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#include "MiscFilters.h"
#include <math.h>

#ifndef USING_ACCELERATE
    #include <vImage/Conversion.h>
#endif

/*
//Not reentrant
int FFTFilter_FP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor , vImage_Flags flags )
{
    static unsigned int oldRows = 0;
    static unsigned int oldCols = 0;
    static FFTSetup setup = NULL;
    
    unsigned int newRows, newCols;
    unsigned int log2Rows, log2Cols;
    vImage_Buffer	in, out;
    

    //Round down to the nearest power of 2
    log2Rows = floor( log2( height ) );
    log2Cols = floor( log2( width ) );
    newRows = 1UL << log2Rows;
    newCols = 1UL << log2Cols;
    
    //Do the FFT setup if the old size is not the same as the new one
    if( newRows != oldRows || newCols != oldCols || setup == NULL )
    {
        if( NULL != setup )
            fft2d_free( setup, 0 );
            
        setup = NULL;
        oldRows = newRows; 
        oldCols = newCols; 
        
        setup = fft2d_setup( log2Rows, log2Cols, 0 );

        if( NULL == setup )
            return -1;
    }

    //Set up the ROI's
    in.data = inData;		out.data = outData;
    in.height = newRows;	out.height = newRows;
    in.width = newCols;		out.width = newCols;
    in.rowBytes = inRowBytes;	out.rowBytes = outRowBytes;

    //Do the FFT
    return fft2d_real_out_of_place( &in, &out, setup, kFFTDirection_Forward, kScaleFFTResult | flags);

}


//Not reentrant
int IFFTFilter_FP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    static unsigned int oldRows = 0;
    static unsigned int oldCols = 0;
    static FFTSetup setup = NULL;
    
    unsigned int newRows, newCols;
    unsigned int log2Rows, log2Cols;
    vImage_Buffer	in, out;
    

    //Round down to the nearest power of 2
    log2Rows = floor( log2( height ) );
    log2Cols = floor( log2( width ) );
    newRows = 1UL << log2Rows;
    newCols = 1UL << log2Cols;
    
    //Do the FFT setup if the old size is not the same as the new one
    if( newRows != oldRows || newCols != oldCols || setup == NULL )
    {
        if( NULL != setup )
            fft2d_free( setup, 0 );
            
        setup = NULL;
        oldRows = newRows; 
        oldCols = newCols; 
        
        setup = fft2d_setup( log2Rows, log2Cols, 0 );

        if( NULL == setup )
            return -1;
    }

    //Set up the ROI's
    in.data = inData;		out.data = outData;
    in.height = newRows;	out.height = newRows;
    in.width = newCols;		out.width = newCols;
    in.rowBytes = inRowBytes;	out.rowBytes = outRowBytes;

    //Do the FFT
    return fft2d_real_out_of_place( &in, &out, setup, kFFTDirection_Inverse, kScaleFFTResult | flags);

}
*/

int ClipFilter_FP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    return 0;
}

int ClipFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    return 0;
}

int Lookup(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer	out = { outData, height, width, outRowBytes };

    if( kernel_width != 1 || kernel_height != 256 )
        return kvImageInvalidKernelSize;

    return vImageTableLookUp_Planar8(&in, &out, kernel, flags);

}

int Lookup_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer	out = { outData, height, width, outRowBytes };

    if( kernel_width != 1 || kernel_height != 256 )
        return kvImageInvalidKernelSize;

    return vImageTableLookUp_ARGB8888(&in, &out, kernel, kernel, kernel, kernel, flags);

}


