/*
 *  GeometryFilters.h
 *  Tableau
 *
 *  Created by Ian Ollmann on Fri Feb 14 2003.
 *  Copyright (c) 2003 Apple Computer. All rights reserved.
 *
 */


//Kernel info for a Geometry function
typedef struct
{
    float	xShear;			//degrees
    float	yShear;			//degrees
    float	xTranslation;		//fraction of the image width -1.0 ... 1.0
    float	yTranslation;		//fraction of the image width -1.0 ... 1.0
    float	xScale;			//log2 scale
    float	yScale;			//log2 scale
    float	rotate;
    float	a, r, g, b;		//background color
    vImage_AffineTransform	transformMatrix;	//3x3 array of transformation matrix elements
    BOOL	quitting;
}TransformInfo;

int ShearXFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int ShearYFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int ShearXFilterCustom(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int ShearYFilterCustom(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int RotateFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int ScaleFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags ); 

int AffineTransformFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags ); 

int ReflectXFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags ); 

int ReflectYFilter(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags ); 


int ShearXFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ShearYFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ShearXFilterCustomFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int ShearYFilterCustomFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int RotateFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ScaleFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int AffineTransformFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ReflectXFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ReflectYFilterFP(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );


int ShearXFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ShearYFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ShearXFilterCustom_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int ShearYFilterCustom_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int RotateFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ScaleFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int AffineTransformFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ReflectXFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ReflectYFilter_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );


int ShearXFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ShearYFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ShearXFilterCustom_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int ShearYFilterCustom_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );  

int RotateFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ScaleFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int AffineTransformFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ReflectXFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

int ReflectYFilterFP_ARGB(void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int colorChannel, vImage_Flags flags );

