//
//  FilterTest.h
//  Erode
//
//  Created by Ian Ollmann on Fri Oct 04 2002.
//  Copyright (c) 2002 Apple. All rights reserved.
//

#ifndef USING_ACCELERATE
    #import<vImage/vImage_Types.h>
#endif

@class Kernel;

//Test Methods
enum
{
    kStandardTest = 0,
    kGeometryTest = 1
};

//This is an abstract class
@interface FilterTest : NSObject 
{
    void		*inChannels[5];
    void		*outChannels[5];
    int			bufferCount;
    void		*filter;
    void		*kernel;
    int			kernel_height;
    int			kernel_width;
    int 		channelCount;
    int			height;
    int			width;
    int			bytesPerRow;
    int 		divisor;
    int 		functionID;
    volatile int	iterationCount;
    SEL			setupMethod;
    id			controller;
    BOOL		isDoneInPlace;		//default NO
    BOOL		hasAlpha;
    BOOL		isUsingVector;
    BOOL		doAmberTrace;
    BOOL		useChannelNumberForDivisor;
    vImage_Flags	flags;
    NSLock		*setupLock;
}

+ (FilterTest*) constructTestForFilter:(int)filter 
                usingVector:(BOOL)usingVector 
                isPlanar:(BOOL)isPlanar  
                usingDataFormat:(int)format
                withKernel:(Kernel*)kernel
                andFlags: (vImage_Flags) flags;
- (BOOL) isInterleaved;
- (double) runTest:(id)controller;
- (void) setupStandardTest;
- (void) setupAlphaTest;
- (void) setupGeometryTest;
- (void) setFilter:(void*)filter;
- (void) setKernel:(Kernel*)kernel;
- (void) setDivisor:(int)divisor;
- (void) setDoOperationInPlace: (BOOL)yes_or_no;
- (int)  loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) input;
- (void) copyResultToNSBitmapImageRep: (NSBitmapImageRep*) result;
- (void) doAmberTrace:(BOOL)shouldDoAmberTrace;
- (void) setIterationCount:(int) count;
- (void) setVector: (BOOL) usingVector;
- (void) setFunctionID: (int) the_id;
- (void) setFlags: (vImage_Flags) flags;

- (void) updateViewUsingNewGeometryKernel:(void*) kernel;
@end


@interface FilterTest_8_Bit_Planar : FilterTest
{}
- (BOOL) isInterleaved;
- (int) loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) input;
- (void) copyResultToNSBitmapImageRep: (NSBitmapImageRep*) result;
@end

@interface FilterTest_8_Bit_Interleaved : FilterTest
{}
- (BOOL) isInterleaved;
- (int) loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) input;
- (void) copyResultToNSBitmapImageRep: (NSBitmapImageRep*) result;
@end

@interface FilterTest_FP_Planar : FilterTest
{}
- (BOOL) isInterleaved;
- (int) loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) input;
- (void) copyResultToNSBitmapImageRep: (NSBitmapImageRep*) result;
@end

@interface FilterTest_FP_Interleaved : FilterTest
{}
- (BOOL) isInterleaved;
- (int) loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) input;
- (void) copyResultToNSBitmapImageRep: (NSBitmapImageRep*) result;
@end