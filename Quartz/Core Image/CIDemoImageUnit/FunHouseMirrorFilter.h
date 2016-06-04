//
//  FunHouseMirrorFilter.h
//  FunHouseMirrorFilterPlugin
//
//  Created by Mark Zimmer on Tuesday, June 1, 2004
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QuartzCore/CoreImage.h>

@interface FunHouseMirrorFilter : CIFilter
{
    CIImage      *inputImage;
    CIVector     *inputCenter;
    NSNumber     *inputWidth;
    NSNumber     *inputAmount;
}

@end
