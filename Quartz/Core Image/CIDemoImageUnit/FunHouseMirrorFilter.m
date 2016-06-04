//
//  FunHouseMirrorFilter.m
//  FunHouseMirrorFilterPlugin
//
//  Created by Mark Zimmer on Tuesday, June 1, 2004
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import "FunHouseMirrorFilter.h"
#import <Foundation/Foundation.h>
#import <ApplicationServices/ApplicationServices.h>

@implementation FunHouseMirrorFilter

static CIKernel *_funHouseMirrorKernel = nil;

- (id)init
{
    if(_funHouseMirrorKernel == nil)
    {
		NSBundle    *bundle = [NSBundle bundleForClass:NSClassFromString(@"FunHouseMirrorFilter")];
		NSString    *code = [NSString stringWithContentsOfFile:[bundle pathForResource:@"funHouseMirror" ofType:@"cikernel"]];
		NSArray     *kernels = [CIKernel kernelsWithString:code];

		_funHouseMirrorKernel = [[kernels objectAtIndex:0] retain];
    }
    return [super init];
}


- (CGRect)regionOf: (int)sampler  destRect: (CGRect)rect  userInfo: (NSNumber *)radius
{
    return CGRectInset(rect, -[radius floatValue], 0);
}


- (NSDictionary *)customAttributes
{
    return [NSDictionary dictionaryWithObjectsAndKeys:

        [NSDictionary dictionaryWithObjectsAndKeys:
            [CIVector vectorWithX:200.0 Y:200.0],       kCIAttributeDefault,
            kCIAttributeTypePosition,           kCIAttributeType,
            nil],                               @"inputCenter",

        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithDouble:  1.00], kCIAttributeMin,
            [NSNumber numberWithDouble:  1.00], kCIAttributeSliderMin,
            [NSNumber numberWithDouble:1000.00], kCIAttributeSliderMax,
            [NSNumber numberWithDouble:400.00], kCIAttributeDefault,
            [NSNumber numberWithDouble:400.00], kCIAttributeIdentity,
            kCIAttributeTypeDistance,           kCIAttributeType,
            nil],                               @"inputWidth",

        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithDouble:  0.00], kCIAttributeMin,
            [NSNumber numberWithDouble:  0.00], kCIAttributeSliderMin,
            [NSNumber numberWithDouble:  2.00], kCIAttributeSliderMax,
            [NSNumber numberWithDouble:  0.50], kCIAttributeDefault,
            [NSNumber numberWithDouble:  0.00], kCIAttributeIdentity,
            kCIAttributeTypeDistance,           kCIAttributeType,
            nil],                               @"inputAmount",

        nil];
}

// called when setting up for fragment program and also calls fragment program
- (CIImage *)outputImage
{
    float radius;
    CISampler *src;
    
    src = [CISampler samplerWithImage:inputImage];
    radius = [inputWidth floatValue] * 0.5;
    return [self apply:_funHouseMirrorKernel, src,
        [NSNumber numberWithFloat:[inputCenter X]],
        [NSNumber numberWithFloat:1.0 / radius],
        [NSNumber numberWithFloat:radius],
        [NSNumber numberWithFloat: 1.0 / pow(10.0, [inputAmount floatValue])],
	    kCIApplyOptionDefinition, [[src definition] insetByX:-radius Y:-radius],
            kCIApplyOptionUserInfo, [NSNumber numberWithFloat:radius], nil];
}

@end
