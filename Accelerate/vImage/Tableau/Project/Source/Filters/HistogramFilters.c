/*
 *  ConvolutionFilters.c
 *
 *  Created by Robert Murley on Fri Feb 21 2003.
 *  Copyright (c) 2003 Apple. All rights reserved.
 *
 */

#ifndef USING_ACCELERATE
    #include <vImage/vImage.h>
#endif

#define Entries 8
#define Min     -3.0
#define Max     -1.0

void StdToSpecified_PlanarF( vImage_Buffer *src )
{
    float	*srcRow;
    int		h, w;
    
    srcRow = (float *) src->data;
    for ( h = 0; h < src->height; h++ )
    {
        for ( w = 0; w < src->width; w++ )
            srcRow[w] = srcRow[w] * ( Max - Min ) + Min;
        srcRow += src->rowBytes/4;
    }
}

void SpecifiedToStd_PlanarF( vImage_Buffer *src )
{
    float	*srcRow;
    int		h, w;
    
    srcRow = (float *) src->data;
    for ( h = 0; h < src->height; h++ )
    {
        for ( w = 0; w < src->width; w++ )
            srcRow[w] = ( srcRow[w] - Min ) / ( Max - Min );
        srcRow += src->rowBytes/4;
    }
}

void StdToSpecified_ARGBFFFF( vImage_Buffer *src )
{
    float	*srcRow;
    int		h, w;
    
    srcRow = (float *) src->data;
    for ( h = 0; h < src->height; h++ )
    {
        for ( w = 0; w < src->width; w++ )
        {
            srcRow[4*w] = srcRow[4*w] * ( Max - Min ) + Min;
            srcRow[4*w+1] = srcRow[4*w+1] * ( Max - Min ) + Min;
            srcRow[4*w+2] = srcRow[4*w+2] * ( Max - Min ) + Min;
            srcRow[4*w+3] = srcRow[4*w+3] * ( Max - Min ) + Min;
        }
        srcRow += src->rowBytes/4;
    }
}

void SpecifiedToStd_ARGBFFFF( vImage_Buffer *src )
{
    float	*srcRow;
    int		h, w;
    
    srcRow = (float *) src->data;
    for ( h = 0; h < src->height; h++ )
    {
        for ( w = 0; w < src->width; w++ )
        {
            srcRow[4*w] = ( srcRow[4*w] - Min ) / ( Max - Min );
            srcRow[4*w+1] = ( srcRow[4*w+1] - Min ) / ( Max - Min );
            srcRow[4*w+2] = ( srcRow[4*w+2] - Min ) / ( Max - Min );
            srcRow[4*w+3] = ( srcRow[4*w+3] - Min ) / ( Max - Min );
        }
        srcRow += src->rowBytes/4;
    }
}

int Histogram(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    unsigned int   histogram[256];

    err = vImageHistogramCalculation_Planar8(	&src, 	//const vImage_Buffer *src
                                        histogram,	//array to receive histogram, 
                                        flags
                                    );

    return err;
}


int HistogramFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    unsigned int   histogram[256];

    err = vImageHistogramCalculation_PlanarF(	&src, 	//const vImage_Buffer *src
                                        histogram,	//array to receive histogram, 
										256, 0.0, 1.0,
                                        flags
                                    );
    
    return err;
}


int Histogram_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    unsigned int   hist0[256], hist1[256], hist2[256], hist3[256];
	unsigned int   *histogram[4] = { hist0, hist1, hist2, hist3 };

    err = vImageHistogramCalculation_ARGB8888(	&src, 	//const vImage_Buffer *src
                                        histogram,	//array to receive histogram, 
                                        flags
                                    );

    return err;
}


int HistogramFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    unsigned int   hist0[256], hist1[256], hist2[256], hist3[256];
    unsigned int   *histogram[4] = { hist0, hist1, hist2, hist3 };

    err = vImageHistogramCalculation_ARGBFFFF(	&src, 	//const vImage_Buffer *src
                                        histogram,	//array to receive histogram, 
										256, 0.0, 1.0,
                                        flags
                                    );
    
    return err;
}


int Histogram_Equalization(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageEqualization_Planar8(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
                                        flags
                                    );

    return err;
}


int Histogram_EqualizationFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageEqualization_PlanarF(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
										NULL,	//temp buffer 
										256, 0.0, 1.0,
                                        flags
                                    );
    
    return err;
}


int Histogram_Equalization_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageEqualization_ARGB8888(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
                                        flags
                                    );

    return err;
}


int Histogram_EqualizationFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageEqualization_ARGBFFFF(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
										NULL,	//temp buffer 
										256, 0.0, 1.0,
                                        flags
                                    );
    
    return err;
}


int Histogram_Specification(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };
    unsigned int   histogram[256];

    err = vImageHistogramCalculation_Planar8(	&src, 	//const vImage_Buffer *src
                                        histogram,	//array to receive histogram, 
                                        flags
                                    );
    
    {
        int i;
        for ( i = 0; i < 256; i++ )
            histogram[i] *= 0.8;
    }

    err = vImageHistogramSpecification_Planar8(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
                                        histogram,	//array to receive histogram, 
                                        flags
                                    );

    return err;
}


int Histogram_SpecificationFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };
    unsigned int   histogram[256];

    StdToSpecified_PlanarF( &src );

    err = vImageHistogramCalculation_PlanarF(	&src, 	//const vImage_Buffer *src
                                        histogram,	//array to receive histogram, 
										Entries, Min, Max,
                                        flags
                                    );
	
    {
        int i;
        for ( i = 0; i < Entries; i++ )
            histogram[i] *= 0.8;
    }

    err = vImageHistogramSpecification_PlanarF(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
										NULL,	//temp buffer 
                                        histogram,	//specification histogram, 
										Entries, Min, Max,
                                        flags
                                    );
    
    SpecifiedToStd_PlanarF( &dest );
    
    return err;
}


int Histogram_Specification_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error		err;
    vImage_Buffer		src = { inData, height, width, inRowBytes };
    vImage_Buffer		dest = { outData, height, width, outRowBytes };
    unsigned int		hist0[256], hist1[256], hist2[256], hist3[256];
	unsigned int		*histogram[4] = { hist0, hist1, hist2, hist3 };

    err = vImageHistogramCalculation_ARGB8888(	&src, 	//const vImage_Buffer *src
                                        histogram,	//array to receive histogram, 
                                        flags
                                    );

    {
        int i;
        for ( i = 0; i < 256; i++ )
        {
            hist0[i] *= 0.8;
            hist1[i] *= 0.8;
            hist2[i] *= 0.8;
            hist3[i] *= 0.8;
        }
    }

    err = vImageHistogramSpecification_ARGB8888(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
                                        (const unsigned int **) histogram,	//array to receive histogram, 
                                        flags
                                    );

    return err;
}


int Histogram_SpecificationFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error		err;
    vImage_Buffer		src = { inData, height, width, inRowBytes };
    vImage_Buffer		dest = { outData, height, width, outRowBytes };
    unsigned int		hist0[256], hist1[256], hist2[256], hist3[256];
	unsigned int		*histogram[4] = { hist0, hist1, hist2, hist3 };

    StdToSpecified_ARGBFFFF( &src );

    err = vImageHistogramCalculation_ARGBFFFF(	&src, 	//const vImage_Buffer *src
                                        histogram,	//array to receive histogram, 
										Entries, Min, Max,
                                        flags
                                    );
	
    {
        int i;
        for ( i = 0; i < Entries; i++ )
        {
            hist0[i] *= 0.8;
            hist1[i] *= 0.8;
            hist2[i] *= 0.8;
            hist3[i] *= 0.8;
        }
    }

    err = vImageHistogramSpecification_ARGBFFFF(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
										NULL,	//temp buffer 
                                        (const unsigned int **) histogram,	//array to receive histogram, 
										Entries, Min, Max,
                                        flags
                                    );
    
    SpecifiedToStd_ARGBFFFF( &dest );
    
    return err;
}


int Histogram_Contrast_Stretch(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageContrastStretch_Planar8(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
                                        flags
                                    );
    
    return err;
}


int Histogram_Contrast_StretchFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageContrastStretch_PlanarF(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
										NULL,	//temp buffer 
										256, 0.0, 1.0,
                                        flags
                                    );
    
    return err;
}


int Histogram_Contrast_Stretch_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageContrastStretch_ARGB8888(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
                                        flags
                                    );
    
    return err;
}


int Histogram_Contrast_StretchFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageContrastStretch_ARGBFFFF(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
										NULL,	//temp buffer 
										256, 0.0, 1.0,
                                        flags
                                    );
    
    return err;
}


int Histogram_Ends_In_Contrast_Stretch(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageEndsInContrastStretch_Planar8(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
										kernel_height,	// low end
										kernel_width,	// high end
                                        flags
                                    );
    
    return err;
}


int Histogram_Ends_In_Contrast_StretchFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };

    err = vImageEndsInContrastStretch_PlanarF(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
										NULL,	//temp buffer 
										kernel_height,	// low end
										kernel_width,	// high end
										256, 0.0, 1.0,
                                        flags
                                    );
    
    return err;
}


int Histogram_Ends_In_Contrast_Stretch_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };
	unsigned int   lowend[4] = { kernel_height, kernel_height, kernel_height, kernel_height };
	unsigned int   highend[4] = { kernel_width, kernel_width, kernel_width, kernel_width };

    err = vImageEndsInContrastStretch_ARGB8888(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
                                        lowend,		// low end
										highend,	// high end
                                        flags
                                    );
    
    return err;
}


int Histogram_Ends_In_Contrast_StretchFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Error err;
    vImage_Buffer src = { inData, height, width, inRowBytes };
    vImage_Buffer	   dest = { outData, height, width, outRowBytes };
	unsigned int   lowend[4] = { kernel_height, kernel_height, kernel_height, kernel_height };
	unsigned int   highend[4] = { kernel_width, kernel_width, kernel_width, kernel_width };

    err = vImageEndsInContrastStretch_ARGBFFFF(	&src, 	//const vImage_Buffer *src
                                        &dest,	//const vImage_Buffer *dest, 
										NULL,	//temp buffer 
                                        lowend,		// low end
										highend,	// high end
										256, 0.0, 1.0,
                                        flags
                                    );
    
    return err;
}
