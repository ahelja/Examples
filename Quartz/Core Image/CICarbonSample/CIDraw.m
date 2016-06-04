/*
 *  MyCIHIView.c
 *  CICarbonSample
 *
 *  Created by frank on 2/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#import <QuartzCore/QuartzCore.h>
#include "CIDraw.h"

SInt32	    gGammaValue = 75;

const float opaqueBlack[] = { 0.0, 0.0, 0.0, 1.0 };

void drawRomanText(CGContextRef context, CGRect destRect)
{
    static const char *text = "CARBON";
    size_t textlen = strlen(text);
    static const float fontSize = 60;
    
        
    // Set the fill color space. This sets the 
    // fill painting color to opaque black.
    CGContextSetFillColorSpace(context, CGColorSpaceCreateDeviceRGB());
    // The Cocoa framework calls the draw method with an undefined
    // text matrix. It's best to set it to what is needed by
    // this code: the identity transform.
    CGContextSetTextMatrix(context, CGAffineTransformIdentity);
       
    // Choose the font with the PostScript name "Times-Roman", at
    // fontSize points, with the encoding MacRoman encoding.
    CGContextSelectFont(context, "Times-Roman", fontSize, 
					    kCGEncodingMacRoman);

    CGContextSetFillColor(context, opaqueBlack);
    // Default text drawing mode is fill. Draw the text at (70, 70).
    CGContextShowTextAtPoint(context, 0.0, 0.0, text, textlen);

}

CIImage* generateBackgroundImage(void)
{
    static CIImage  *sBackgroundImage = nil;
    
    if(!sBackgroundImage)
    {
	CIFilter *randomGeneratorFilter = [CIFilter filterWithName:@"CIRandomGenerator"];
	CIFilter *monochromeFilter = [CIFilter filterWithName:@"CIColorMonochrome"];
	CIFilter *heightFieldFilter = [CIFilter filterWithName:@"CIHeightFieldFromMask"];
	CIFilter *sepiaToneFilter = [CIFilter filterWithName:@"CISepiaTone"];
	
	[randomGeneratorFilter setDefaults];
	[monochromeFilter setDefaults];
	[monochromeFilter setValue:[CIColor colorWithRed:0.9 green:0.9 blue:0.9] forKey:@"inputColor"];
	[monochromeFilter setValue:[randomGeneratorFilter valueForKey: @"outputImage"] forKey:@"inputImage"];
	[heightFieldFilter setDefaults];
	[heightFieldFilter setValue:[NSNumber numberWithFloat:9.0] forKey:@"inputRadius"];
	[heightFieldFilter setValue:[monochromeFilter valueForKey: @"outputImage"] forKey:@"inputImage"];
	[sepiaToneFilter setDefaults];
	[sepiaToneFilter setValue:[NSNumber numberWithFloat:1.0] forKey:@"inputIntensity"];
	[sepiaToneFilter setValue:[heightFieldFilter valueForKey: @"outputImage"] forKey:@"inputImage"];	
	
	sBackgroundImage = [[sepiaToneFilter valueForKey: @"outputImage"] retain];
    }
    return sBackgroundImage;
}

void DoDraw(CGContextRef inContext, CGRect bounds)
{
    NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
    CIContext		*ciContext = [CIContext contextWithCGContext:inContext options:nil];
    CIFilter		*gloomFilter = [CIFilter filterWithName:@"CIGloom"];
    CIFilter		*gammaFilter = [CIFilter filterWithName:@"CIGammaAdjust"];
    CGRect		layerRect = CGRectMake(0.0, 0.0, 250.0, 70.0);
    
    static CIImage	*textImage = nil;
    
    if(!textImage)
    {
	CGLayerRef	layer = [ciContext createCGLayerWithSize: layerRect.size  info: nil];
	CGContextRef	context = CGLayerGetContext(layer);

	drawRomanText(context, layerRect);
	textImage = [[CIImage alloc] initWithCGLayer:layer];
	CGLayerRelease(layer);
    }
    
    // Set the fill color space. This sets the 
    // fill painting color to opaque black.
    CGContextSetFillColorSpace(inContext, CGColorSpaceCreateDeviceRGB());
    CGContextSetFillColor(inContext, opaqueBlack);
    CGContextFillRect(inContext, bounds);
    
    [gloomFilter setValue:[NSNumber numberWithFloat:0.5] forKey:@"inputIntensity"];
    [gloomFilter setValue:[NSNumber numberWithFloat:75.0] forKey:@"inputRadius"];
    [gloomFilter setValue:textImage forKey:@"inputImage"];
    layerRect = [[gloomFilter valueForKey: @"outputImage"] extent];
    
    [gammaFilter setDefaults];
    [gammaFilter setValue:[NSNumber numberWithFloat:((float)gGammaValue / 100.0)] forKey:@"inputPower"];
    [gammaFilter setValue:generateBackgroundImage() forKey:@"inputImage"];

    [ciContext drawImage: [gammaFilter valueForKey: @"outputImage"] atPoint: CGPointZero fromRect: bounds];
    [ciContext drawImage: [gloomFilter valueForKey: @"outputImage"] 
		    atPoint: CGPointMake(((bounds.size.width - layerRect.size.width) * 0.5), ((bounds.size.height - layerRect.size.height) * 0.5))
		    fromRect: layerRect];
    [pool release];

}