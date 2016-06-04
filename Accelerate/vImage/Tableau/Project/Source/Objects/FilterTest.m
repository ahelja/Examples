//
//  FilterTest.m
//  Erode
//
//  Created by Ian Ollmann on Fri Oct 04 2002.
//  Copyright (c) 2002 Apple. All rights reserved.
//

#import "FilterTest.h"
#import "ImageFilterController.h"
#include "Filters.h"
#include "MyTimes.h"
#include "Kernel.h"
#include "GeometryPane.h"
#include "AlphaPane.h"
#include "GeometryFilters.h"

#include <objc/objc-runtime.h>

extern void SetvImageVectorAvailable( int whatIsAvailable ); 

//Possible values stored in gIsVectorAvailable
enum
{
    kScalarOnly 		= 0,
    kAltiVec			= 1,
    kDualFPU			= 2,
    kAltiVecAndDualFPU	= kAltiVec | kDualFPU,		/* 3 */

    kOtherVector		= 4
};

#if defined ( __ppc__ )
    #define VECTOR_ON		kAltiVec
    #define VECTOR_OFF		kScalarOnly
#else
    #define VECTOR_ON		kOtherVector
    #define VECTOR_OFF		kScalarOnly
#endif

@implementation FilterTest

+ (FilterTest*) constructTestForFilter:(int)the_filter 
                usingVector:(BOOL)usingVector 
                isPlanar:(BOOL)isPlanar  
                usingDataFormat:(int)format
                withKernel:(Kernel*)the_kernel 
                andFlags: (vImage_Flags) new_flags
{
    FilterTest *test = nil; 
    int set = the_filter >> 16;
    int item = the_filter & 0xFFFFUL;
    int index = 0;
    int a_divisor = 0;
    TestFunc	f;

    //sanity checking for non-filters
    if( the_filter == -1L || set >= kListCount || item > filterLists[set].count )
        return nil;

    //See if the filter has a filter function
    if( format == Depth_32_bit_FP_per_channel )
        index = 1;

    if( ! isPlanar )
        index += 2;

    //cheezy hack where we pretend four function pointers in the FilterInfo struct are an array
    f = (TestFunc) (&filterLists[set].list[item].function)[ index ];

	if( nil != the_kernel )
		a_divisor = [ the_kernel divisor ];
    

    if( NULL == f )
        return nil;

    switch( format )
    {
        case Depth_8_bits_per_channel:
            if( isPlanar )
                test = [[ FilterTest_8_Bit_Planar alloc ] init ];
            else
                test = [[ FilterTest_8_Bit_Interleaved alloc ] init ];
            break;
            
        case Depth_32_bit_FP_per_channel:
            if( isPlanar )
                test = [[ FilterTest_FP_Planar alloc ] init ];
            else
                test = [[ FilterTest_FP_Interleaved alloc ] init ];
            break;
    }

    if( filterLists[ set ].list[ item ].flags & kDoOperationInPlace )
        [ test setDoOperationInPlace: YES ];
    
    [ test setFilter: f ];    
    [ test setKernel: the_kernel ];
    [ test setVector: usingVector];
    [ test setDivisor: a_divisor ];
    [ test setFunctionID: the_filter ];
    [ test setFlags: new_flags ];

    [ test autorelease ];
        
    return test;
}

- (void) setDivisor:(int)a_divisor
{	
	divisor = a_divisor;
}

- (void) setFlags: (vImage_Flags) new_flags
{
    flags = new_flags;
}


- (void) setVector: (BOOL) usingVector
{
    isUsingVector = usingVector;
}

- (void) doAmberTrace:(BOOL)shouldDoAmberTrace
{
    doAmberTrace = shouldDoAmberTrace;
}

- (BOOL) isInterleaved { return NO; }

- (id) init
{
    int i;
    
    for( i = 0; i < 5; i++ )
        inChannels[i] = outChannels[i] = NULL;

    filter = NULL;    
    kernel = NULL;
    kernel_height = 0;
    kernel_width = 0;
    channelCount = 0;
    height = width = 0;
    bytesPerRow = 0;
    isDoneInPlace = NO;
    hasAlpha = NO;
    doAmberTrace = NO;
    setupMethod = @selector( setupStandardTest );
    
    setupLock = [[ NSLock alloc ] init ];
    [ setupLock tryLock ];


    return self;
}

- (void) setFilter:(void*)the_filter
{
    filter = the_filter;
}

- (void) setKernel:(Kernel*)the_kernel
{
    int size;

    kernel_height = [ the_kernel height ];
    kernel_width = [ the_kernel width ];

    if( NULL != kernel )
        free( kernel );
    
    kernel = NULL;
    size = kernel_height * kernel_width * [ the_kernel sizeofElement ];
    
    if( size > 0 )
        kernel = malloc( size );
    
    if( NULL != kernel )
        memcpy( kernel, [ the_kernel data], size );
}



- (double) runTest:(id)the_controller
{
    uint64_t	startTime, endTime;
    double	clockLatency, currentTime, bestTime;
    TestFunc	function = (TestFunc) filter; 
    int 	i, j;
    int 	testChannelCount = bufferCount;
    
    controller = the_controller;
    
    if( hasAlpha && NO == [ self isInterleaved ] )
        testChannelCount--;
        
    clockLatency = 1e20;
    for( i = 0; i < 100; i++ )
    {
        startTime = MyGetTime();
        endTime = MyGetTime();
        currentTime = MySubtractTime( endTime, startTime );
        if( currentTime < clockLatency )
            clockLatency = currentTime;
    }

    if( isUsingVector )
        SetvImageVectorAvailable( VECTOR_ON );
    else
        SetvImageVectorAvailable( VECTOR_OFF );
    
    //Set up data for the filter
    objc_msgSend( self, setupMethod, controller );
    
    if( doAmberTrace )
    {
    #if defined( __ppc__ )
        asm volatile( "mfspr %0, 1023" : "=r" (i) );
        (*function)( inChannels[0], bytesPerRow, outChannels[0], bytesPerRow, height, width, kernel, kernel_height, kernel_width, divisor, flags );       
    #endif    
    }
    
    bestTime = 1e20;

    if( YES == useChannelNumberForDivisor )
    {
        for( i = 0; i < iterationCount; i++ )
        {
            [ (ImageFilterController*) controller setProgress: (double) (i+1) / (double) iterationCount ];
            startTime = MyGetTime();
            for( j = 0; j < testChannelCount; j++ )
                (*function)( inChannels[j], bytesPerRow, outChannels[j], bytesPerRow, height, width, kernel, kernel_height, kernel_width, j, flags );
            endTime = MyGetTime();
            currentTime = MySubtractTime( endTime, startTime );
            if( currentTime < bestTime )
                bestTime = currentTime;
        }
    }
    else
    {
        for( i = 0; i < iterationCount; i++ )
        {
            [ (ImageFilterController*) controller setProgress: (double) (i+1) / (double) iterationCount ];
            startTime = MyGetTime();
            for( j = 0; j < testChannelCount; j++ )
                (*function)( inChannels[j], bytesPerRow, outChannels[j], bytesPerRow, height, width, kernel, kernel_height, kernel_width, divisor, flags );
            endTime = MyGetTime();
            currentTime = MySubtractTime( endTime, startTime );
            if( currentTime < bestTime )
                bestTime = currentTime;
        }
    }

    [ (ImageFilterController*) controller setProgress: 1.0 ];
    [ (ImageFilterController*) controller endFilter ];
    
    return bestTime - clockLatency;
}

- (void) setupStandardTest
{
    useChannelNumberForDivisor = NO;
}

- (void) setupAlphaTest
{
    AlphaPane *pane = [ controller alphaPane ];

    if( nil != pane )
    {
        [ pane	initWithTarget: self
                action: @selector( updateViewUsingNewAlphaKernel:)
                function: functionID
                height: height
                width: width				];
    
        [ setupLock lock ];
    }
}


- (void) setupGeometryTest
{
    GeometryPane *pane = [ controller geometryPane ];
    useChannelNumberForDivisor = YES;
    

    if( nil != pane )
    {
        [ pane	initWithTarget: self 
                    action: @selector( updateViewUsingNewGeometryKernel: ) 
                    function: functionID 
                    height: height 
                    width: width 			];
    
        [ setupLock lock ];
    }
}

- (void) setFunctionID: (int) the_id
{
    int set = the_id >> 16;
    int item = the_id & 0xFFFF;

    functionID = the_id;
    
    if( filterLists[ set ].list[item].flags & kGeometryFlags )
        setupMethod = @selector( setupGeometryTest );
    else if( filterLists[ set ].list[item].flags & kIsAlpha )
        setupMethod = @selector( setupAlphaTest );
    else
        setupMethod = @selector( setupStandardTest );

}


- (int) loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) input
{

    return 0;
}


- (void) copyResultToNSBitmapImageRep: (NSBitmapImageRep*) result
{}

- (void) setDoOperationInPlace:(BOOL)yes_or_no
{
    isDoneInPlace = yes_or_no;
}

-(void) dealloc
{
    int i;
    
    for( i = 0; i < 5; i++ )
    {
        if( nil != inChannels[i] )
            free( inChannels[i] );
        inChannels[i] = nil;
        if( nil != outChannels[i] )
            free( outChannels[i] );
        outChannels[i] = nil;
    }
    
    [ setupLock release ];
}

- (void) setIterationCount:(int) count
{
    iterationCount = count;
}

- (void) updateViewUsingNewGeometryKernel:(void*) the_kernel
{
    uint64_t	startTime, endTime;
    double	currentTime;
    TestFunc	function = (TestFunc) filter; 
    int 	testChannelCount = bufferCount;
    int 	j;

    if( kernel )
        memcpy( kernel, the_kernel, sizeof( TransformInfo ) );

    if( ((TransformInfo*)the_kernel)->quitting == YES )
    {
        [ setupLock unlock ];
    }
    else
    {
        startTime = MyGetTime();
        for( j = 0; j < testChannelCount; j++ )
            (*function)( inChannels[j], bytesPerRow, outChannels[j], bytesPerRow, height, width, kernel, kernel_height, kernel_width, j, flags );
        endTime = MyGetTime();
        currentTime = MySubtractTime( endTime, startTime );
        
        [ controller showTime: currentTime ]; 
        [ controller flushTestFrame: self ];
    }
}

@end


@implementation FilterTest_8_Bit_Planar : FilterTest



- (void) copyResultToNSBitmapImageRep: (NSBitmapImageRep*) result
{
    NSSize		size = [ result size ];
    int 		outBytesPerRow = [ result bytesPerRow ];
    int			i;
    int 		writeChannelCount = channelCount;
    int 		the_height = size.height;
    int 		the_width = size.width;
    
    if( height < the_height )
        the_height = height;

    if( width < the_width )
        the_width = width;

    if( hasAlpha == YES )
        writeChannelCount--;
    
    if( [ result isPlanar ] || channelCount == 1 )
    {
        unsigned char	*outData[5];

        [ result getBitmapDataPlanes: outData ];
        
        for( i = 0; i < writeChannelCount; i++ )
        {
            int j;

            for( j = 0; j < the_height; j++ )
                memcpy( outData[i] + j * outBytesPerRow, ((char*) (outChannels[i])) + j * width,  the_width ); 
        }
    }
    else
    {
        unsigned char *data = [ result bitmapData ];
        
        for( i = 0; i < writeChannelCount; i++ )
        {
            int j, k, l;
            unsigned char *outsamples = data + i;
            
            for( j = 0; j < the_height; j++ )
            {
                unsigned char *d = outChannels[i] + j * bytesPerRow;
                
                for( k = 0, l = 0; k <= the_width - 4; k+=4 )
                {
                    int d0 = d[k];
                    int d1 = d[k+1];
                    int d2 = d[k+2];
                    int d3 = d[k+3];
                    
                    outsamples[l] = d0;		l += writeChannelCount;
                    outsamples[l] = d1;		l += writeChannelCount;
                    outsamples[l] = d2;		l += writeChannelCount;
                    outsamples[l] = d3;		l += writeChannelCount;
                }

                for( ; k < the_width; k++ )
                {
                    outsamples[l] = d[k];
                    l += writeChannelCount;
                }
                
                outsamples += outBytesPerRow;
            }
        }
    }
}

- (BOOL) isInterleaved { return NO; }

- (int) loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) input
{
    NSSize		size = [ input size ];
    int			i;
    int 		inBytesPerRow = [ input bytesPerRow ];
    int			bitsPerChannel = [ input bitsPerPixel ];

    hasAlpha = [ input hasAlpha ];
    

    bufferCount = channelCount = [input samplesPerPixel ];
    height = size.height;
    bytesPerRow = width = size.width;

    if( 0 == (bytesPerRow & (bytesPerRow - 1)))
        bytesPerRow += 128;

    for( i = 0; i < 5; i++ )
    {
        if( nil != inChannels[i] )
            free( inChannels[i] );
        inChannels[i] = nil;
    
        if( nil != outChannels[i] )
            free( outChannels[i] );
        outChannels[i] = nil;
    }
    
    if( [ input isPlanar ] || channelCount == 1 )
    {
        unsigned char	*inData[5];

        if( bitsPerChannel != 8 )
            return -1;

        [ input getBitmapDataPlanes: inData ];
        
        for( i = 0; i < channelCount; i++ )
        {
            int the_width = size.width;
            int data_size;
            int j;
                        
            data_size = bytesPerRow * size.height;
            
            inChannels[i] = malloc( data_size );
            outChannels[i] = malloc( data_size );
            for( j = 0; j < height; j++ )
            {
                memcpy( ((char*) (inChannels[i])) + j * the_width, inData[i] + j * inBytesPerRow, the_width ); 
                memcpy( ((char*) (outChannels[i])) + j * the_width, inData[i] + j * inBytesPerRow, the_width ); 
            }
        }
    }
    else
    {
        unsigned char *data = [ input bitmapData ];

        bitsPerChannel /= channelCount;
        
        for( i = 0; i < channelCount; i++ )
        {
            int j, k, l;
            unsigned char *insamples = data + i;
            int the_width = size.width;
            int data_size;
                        
            data_size = bytesPerRow * size.height;
            
            inChannels[i] = malloc( data_size );
            outChannels[i] = malloc( data_size );
            
            for( j = 0; j < size.height; j++ )
            {
                unsigned char *d = inChannels[i] + j * bytesPerRow;
                
                for( k = 0, l = 0; k < the_width; k++ )
                {
                    d[k] = insamples[l];
                    l += channelCount;
                }
                insamples += inBytesPerRow;
            }
        }
    }
    
    return 0;
}

@end

@implementation FilterTest_8_Bit_Interleaved : FilterTest

- (void) copyResultToNSBitmapImageRep: (NSBitmapImageRep*) result
{
    NSSize		size = [ result size ];
    int 		outBytesPerRow = [ result bytesPerRow ];
    int			i;
    int 		writeChannelCount = channelCount;
    int 		the_height = size.height;
    int 		the_width = size.width;
    int 		samplesPerPixel = [result samplesPerPixel ];
    
    if( height < the_height )
        the_height = height;

    if( width < the_width )
        the_width = width;

    if( hasAlpha == YES )
        writeChannelCount--;
    
    if( [ result isPlanar ] || channelCount == 1 )
    {
        unsigned char	*outData[5];

        [ result getBitmapDataPlanes: outData ];
        
        for( i = 0; i < writeChannelCount; i++ )
        {
            int index = (i + 1) & 3; //rotate alpha to last
            unsigned char *src = outChannels[0] + index;
            unsigned char *dest = outData[i];
            int j, k;

            if( i == samplesPerPixel - 1 && [ result hasAlpha ] )
                src = outChannels[0];
            

            for( j = 0; j < the_height; j++ )
            {
                dest = (uint8_t*)( (char*) outData[i] + j * outBytesPerRow );
                for( k = 0; k < the_width; k++ )
                    dest[k] = src[ k * 4 ];
                src += 4 * the_width;
            }
        }
    }
    else
    {
        unsigned char *data = [ result bitmapData ];
        unsigned char *src = outChannels[0];
        int i = 0;
        int j,k;
        unsigned char *target;
        
        if( [ result hasAlpha ] )
        {
            target = data;
        
            for( j = 0; j < the_height; j++ )
            {
                for( k = 0; k < the_width; k++ )
                    target[ samplesPerPixel * k ] = src[ 4 * k ];
            
                target = (uint8_t*)((char*) target + outBytesPerRow );
                src += 4 * the_width;
            }
        
            i++;
            data++;
        }

        src = outChannels[0] + 1;
        
        for( ; i < writeChannelCount; i++ )
        {
            uint8_t *localSrc = src;
            target = data;
            
            for( j = 0; j < the_height; j++ )
            {
                for( k = 0; k < the_width; k++ )
                    target[ samplesPerPixel * k ] = localSrc[ k * 4 ];
                
                target = (uint8_t*)((char*) target + outBytesPerRow );
                localSrc = (uint8_t*) ((char*) localSrc + bytesPerRow );
            }
            
            src++;
            data++;
        }
    }
}

- (BOOL) isInterleaved { return YES; }


- (int) loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) input
{
    NSSize		size = [ input size ];
    int			i;
    int 		inBytesPerRow = [ input bytesPerRow ];
    int			bitsPerChannel = [ input bitsPerPixel ];
    int			data_size;
    unsigned char 	*targetIn, *targetOut, *src;
    int j, k, loopCount;

    hasAlpha = [ input hasAlpha ];
    
    bufferCount = 1;
    channelCount = [input samplesPerPixel ];
    height = size.height;
    width = size.width;
    bytesPerRow = width * 4 * sizeof( uint8_t);
    
    //Prevent bytesPerRow from becomming a power of 2
    if( 0 == ( bytesPerRow & (bytesPerRow - 1 ) ) )
        bytesPerRow += 128;
    
    data_size = height * bytesPerRow;
 
    for( i = 0; i < 5; i++ )
    {
        if( nil != inChannels[i] )
            free( inChannels[i] );
        inChannels[i] = nil;
    
        if( nil != outChannels[i] )
            free( outChannels[i] );
        outChannels[i] = nil;
    }

    targetIn = inChannels[0] = malloc( data_size );
    targetOut = outChannels[0] = malloc( data_size );
    loopCount = channelCount;
    
    if( [ input isPlanar ] || channelCount == 1 )
    {
        unsigned char	*inData[5];

        if( bitsPerChannel != 8 )
            return -1;

        [ input getBitmapDataPlanes: inData ];
        
        //Init the alpha channel
        if( [ input hasAlpha ] )
        {
            src = inData[ channelCount - 1 ];
            for( j = 0; j < height; j++ )
            {
                for( i = 0; i < width; i++ )
                {
                    targetIn[0] = targetOut[0] = src[i];
                    targetIn += 4;
                    targetOut += 4;
                }
                src = (uint8_t*)((char*) src + inBytesPerRow);
            }
            
            loopCount--;
        }
        else
        {
            for( j = 0; j < height; j++ )
            {
                for( i = 0; i < width; i++ )
                {
                    targetIn[0] = targetOut[0] = 0xFF;
                    targetIn += 4;
                    targetOut += 4;
                }
            }            
        }

        targetIn = inChannels[0] + 1;
        targetOut = outChannels[0] + 1;
        
        for( i = 0; i < loopCount; i++ )
        {
            uint8_t *dest1 = targetIn;
            uint8_t *dest2 = targetOut;
            
            src = inData[ i ];
            
            for( j = 0; j < height; j++ )
            {
                for( k = 0; k < width; k++ )
                {
                    dest1[0] = dest2[0] = src[k];
                    dest1 += 4;
                    dest2 += 4;
                }

                src = (uint8_t*)((char*) src + inBytesPerRow);
            }
            targetIn++;
            targetOut++;
        }
    }
    else
    {
        unsigned char *data = [ input bitmapData ];
        
        if( hasAlpha )
        {
            src = data;
            for( j = 0; j < height; j++ )
            {
                uint8_t *localSrc = src;
                for( i = 0; i < width; i++ )
                {
                    targetIn[0] = targetOut[0] = localSrc[0];
                    targetIn += 4;
                    targetOut += 4;
                    localSrc += channelCount;
                }
                src = (uint8_t*)((char*) src + inBytesPerRow);
            }
            
            loopCount--;
            data++;
        }
        else
        {
            for( j = 0; j < height; j++ )
            {
                for( i = 0; i < width; i++ )
                {
                    targetIn[0] = targetOut[0] = 0xFF;
                    targetIn += 4;
                    targetOut += 4;
                }
            }            
        }

        bitsPerChannel /= channelCount;
        targetIn = inChannels[0] + 1;
        targetOut = outChannels[0] + 1;
        
        if( loopCount > 3 )
            loopCount = 3;
        
        for( i = 0; i < loopCount; i++ )
        {
            uint8_t *dest1 = targetIn;
            uint8_t *dest2 = targetOut;
            
            src = data;
            
            for( j = 0; j < height; j++ )
            {
                uint8_t *localSrc = src; 
                uint8_t *localDest1 = dest1;
                uint8_t *localDest2 = dest2;
                
                for( k = 0; k < width; k++ )
                {
                    localDest1[0] = localDest2[0] = localSrc[0];
                    localDest1 += 4;
                    localDest2 += 4;
                    localSrc += channelCount;
                }

                src = (uint8_t*)((char*) src + inBytesPerRow);
                dest1 = (uint8_t*)((char*) dest1 + bytesPerRow );
                dest2 = (uint8_t*)((char*) dest2 + bytesPerRow );
            }
            targetIn++;
            targetOut++;
            data++;
        }
    }
    
    return 0;
}


@end

@implementation FilterTest_FP_Planar : FilterTest

- (void) copyResultToNSBitmapImageRep: (NSBitmapImageRep*) result
{
    NSSize		size = [ result size ];
    int 		outBytesPerRow = [ result bytesPerRow ];
    int			i;
    int 		writeChannelCount = channelCount;
    int 		the_height = size.height;
    int 		the_width = size.width;
    
    if( height < the_height )
        the_height = height;

    if( width < the_width )
        the_width = width;

    if( hasAlpha == YES )
        writeChannelCount--;
    
    if( [ result isPlanar ] || channelCount == 1 )
    {
        unsigned char	*outData[5];

        [ result getBitmapDataPlanes: outData ];
        
        for( i = 0; i < writeChannelCount; i++ )
        {
            int j, k;

            for( j = 0; j < the_height; j++ )
            {
                float *inRow = (float*) ((char*) outChannels[i] + j * bytesPerRow );
                uint8_t *outRow = (uint8_t*) outData[i] + j * outBytesPerRow;

                for( k = 0; k < the_width; k++ )
                {
                    int result = inRow[k] * 255.0f + 0.5f;
                    if( result < 0 )
                        result = 0;
                    else
                        if( result > 255 )
                            result = 255;
                    outRow[k] = result;
                }
            }
        }
    }
    else
    {
        unsigned char *data = [ result bitmapData ];
        
        for( i = 0; i < writeChannelCount; i++ )
        {
            int j, k, l;
            unsigned char *outsamples = data + i;
            
            for( j = 0; j < the_height; j++ )
            {
                float *inRow = outChannels[i] + j * bytesPerRow;
                
                for( k = 0, l = 0; k < the_width; k++ )
                {
                    int result = inRow[k] * 255.0f + 0.5f;
                    if( result < 0 )
                        result = 0;
                    else
                        if( result > 255 )
                            result = 255;
                    outsamples[l] = result;
                    l += channelCount;
                }
                outsamples += outBytesPerRow;
            }
        }
    }
}

- (BOOL) isInterleaved { return NO; }


- (int) loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) input
{
    NSSize		size = [ input size ];
    int			i;
    int 		inBytesPerRow = [ input bytesPerRow ];
    int			bitsPerChannel = [ input bitsPerPixel ];

    hasAlpha = [ input hasAlpha ];
    

    bufferCount = channelCount = [input samplesPerPixel ];
    height = size.height;
    width = size.width;
    bytesPerRow = size.width * sizeof( float );

    if( 0 == ( bytesPerRow & (bytesPerRow - 1 ) ) )
        bytesPerRow += 128;

    for( i = 0; i < 5; i++ )
    {
        if( nil != inChannels[i] )
            free( inChannels[i] );
        inChannels[i] = nil;
    
        if( nil != outChannels[i] )
            free( outChannels[i] );
        outChannels[i] = nil;
    }
    
    if( [ input isPlanar ] || channelCount == 1 )
    {
        unsigned char	*inData[5];

        if( bitsPerChannel != 8 )
            return -1;

        [ input getBitmapDataPlanes: inData ];
        
        for( i = 0; i < channelCount; i++ )
        {
            int data_size = bytesPerRow * size.height;
            int j, k;
            
            inChannels[i] = malloc( data_size );
            outChannels[i] = malloc( data_size );

            for( j = 0; j < height; j++ )
            {
                float *inRow = (float*) ((char*) inChannels[i] + j * bytesPerRow);
                float *outRow = (float*) ((char*) outChannels[i] + j * bytesPerRow);
                uint8_t *inDataRow = inData[i] + j * inBytesPerRow;
//                memcpy( ((char*) (inChannels[i])) + j * width, inData[i] + j * inBytesPerRow, width ); 
//                memcpy( ((char*) (outChannels[i])) + j * width, inData[i] + j * inBytesPerRow, width ); 
                for( k = 0; k < width; k++ )
                    inRow[k] = outRow[k] = (float) inDataRow[k] / 255.0f;
                
            }
        }
    }
    else
    {
        unsigned char *data = [ input bitmapData ];

        bitsPerChannel /= channelCount;
        
        for( i = 0; i < channelCount; i++ )
        {
            int data_size = bytesPerRow * size.height;
            int j, k, l;
            unsigned char *insamples = data + i;
            
            inChannels[i] = malloc( data_size );
            outChannels[i] = malloc( data_size );
            
            for( j = 0; j < size.height; j++ )
            {
                float *d = (float*) ((char*) inChannels[i] + j * bytesPerRow);
                float *d2 = (float*) ((char*) outChannels[i] + j * bytesPerRow);
                uint8_t *inRow = insamples + j * inBytesPerRow;
                
                for( k = 0, l = 0; k < width; k++ )
                {
                    d[k] = d2[k] = (float) inRow[l] / 255.0f;
                    l += channelCount;
                }
            }
        }
    }
    
    return 0;
}

@end

@implementation FilterTest_FP_Interleaved : FilterTest

#define FLOAT_TO_INT( a )  ({								\
                                int result; 						\
                                float temp = (a) * 255.0f + 0.5; 			\
                                result = temp;						\
                                if( result > 255 ) result = 255; 			\
                                if( result < 0 ) result = 0;				\
                                /* return */ result;					\
                            })

#define INT_TO_FLOAT( a )  ( (float) (a) * ( 1.0f / 255.0f ))

- (void) copyResultToNSBitmapImageRep: (NSBitmapImageRep*) result
{
    NSSize		size = [ result size ];
    int 		outBytesPerRow = [ result bytesPerRow ];
    int			i;
    int 		writeChannelCount = channelCount;
    int 		the_height = size.height;
    int 		the_width = size.width;
    int 		samplesPerPixel = [result samplesPerPixel ];
    
    if( height < the_height )
        the_height = height;

    if( width < the_width )
        the_width = width;

    if( hasAlpha == YES )
        writeChannelCount--;
    
    if( [ result isPlanar ] || channelCount == 1 )
    {
        unsigned char	*outData[5];

        [ result getBitmapDataPlanes: outData ];
        
        for( i = 0; i < writeChannelCount; i++ )
        {
            int index = (i + 1) & 3; //rotate alpha to last
            float *src = (float*) outChannels[0] + index;
            unsigned char *dest = outData[i];
            int j, k;

            if( i == samplesPerPixel - 1 && [ result hasAlpha ] )
                src = outChannels[0];
            

            for( j = 0; j < the_height; j++ )
            {
                dest = (uint8_t*)( (char*) outData[i] + j * outBytesPerRow );
                for( k = 0; k < the_width; k++ )
                    dest[k] = FLOAT_TO_INT( src[ k * 4 ]  );
                src += 4 * the_width;
            }
        }
    }
    else
    {
        unsigned char *data = [ result bitmapData ];
        float *src = outChannels[0];
        int i = 0;
        int j,k;
        unsigned char *target;
        
        if( [ result hasAlpha ] )
        {
            target = data;
        
            for( j = 0; j < the_height; j++ )
            {
                for( k = 0; k < the_width; k++ )
                    target[ samplesPerPixel * k ] = FLOAT_TO_INT( src[ 4 * k ] );
            
                target = (uint8_t*)((char*) target + outBytesPerRow );
                src += 4 * the_width;
            }
        
            i++;
            data++;
        }

        src = (float*) outChannels[0] + 1;
        
        for( ; i < writeChannelCount; i++ )
        {
            float *localSrc = src;
            target = data;
            
            for( j = 0; j < the_height; j++ )
            {
                for( k = 0; k < the_width; k++ )
                    target[ samplesPerPixel * k ] = FLOAT_TO_INT( localSrc[ 4 * k ] );
                
                localSrc = (float*) ((char*) localSrc + bytesPerRow );
                target = (uint8_t*)((char*) target + outBytesPerRow );
            }
            
            src++;
            data++;
        }
    }
}

- (BOOL) isInterleaved { return YES; }


- (int) loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) input
{
    NSSize		size = [ input size ];
    int			i;
    int 		inBytesPerRow = [ input bytesPerRow ];
    int			bitsPerChannel = [ input bitsPerPixel ];
    int			data_size;
    float	 	*targetIn, *targetOut;
    unsigned char	*src;
    int j, k, loopCount;

    hasAlpha = [ input hasAlpha ];
    
    bufferCount = 1;
    channelCount = [input samplesPerPixel ];
    height = size.height;
    width = size.width;
    bytesPerRow = width * 4 * sizeof( float);

    //Prevent bytesPerRow from becomming a power of 2
    if( 0 == ( bytesPerRow & (bytesPerRow - 1 ) ) )
        bytesPerRow += 128;

    data_size = height * bytesPerRow;
 
    for( i = 0; i < 5; i++ )
    {
        if( nil != inChannels[i] )
            free( inChannels[i] );
        inChannels[i] = nil;
    
        if( nil != outChannels[i] )
            free( outChannels[i] );
        outChannels[i] = nil;
    }

    targetIn = inChannels[0] = malloc( data_size );
    targetOut = outChannels[0] = malloc( data_size );
    loopCount = channelCount;
    
    if( [ input isPlanar ] || channelCount == 1 )
    {
        unsigned char	*inData[5];

        if( bitsPerChannel != 8 )
            return -1;

        [ input getBitmapDataPlanes: inData ];
        
        //Init the alpha channel
        if( [ input hasAlpha ] )
        {
            src = inData[ channelCount - 1 ];
            for( j = 0; j < height; j++ )
            {
                for( i = 0; i < width; i++ )
                {
                    targetIn[0] = targetOut[0] = INT_TO_FLOAT( src[i] );
                    targetIn += 4;
                    targetOut += 4;
                }
                src = (uint8_t*)((char*) src + inBytesPerRow);
            }
            
            loopCount--;
        }
        else
        {
            for( j = 0; j < height; j++ )
            {
                for( i = 0; i < width; i++ )
                {
                    targetIn[0] = targetOut[0] = 1.0f;
                    targetIn += 4;
                    targetOut += 4;
                }
            }            
        }

        targetIn = (float*) inChannels[0] + 1;
        targetOut = (float*) outChannels[0] + 1;
        
        for( i = 0; i < loopCount; i++ )
        {
            float *dest1 = targetIn;
            float *dest2 = targetOut;
            
            src = inData[ i ];
            
            for( j = 0; j < height; j++ )
            {
                for( k = 0; k < width; k++ )
                {
                    dest1[0] = dest2[0] = INT_TO_FLOAT( src[k] );
                    dest1 += 4;
                    dest2 += 4;
                }

                src = (uint8_t*)((char*) src + inBytesPerRow);
            }
            targetIn++;
            targetOut++;
        }
    }
    else
    {
        unsigned char *data = [ input bitmapData ];
        
        if( hasAlpha )
        {
            src = data;
            for( j = 0; j < height; j++ )
            {
                uint8_t *localSrc = src;
                for( i = 0; i < width; i++ )
                {
                    targetIn[0] = targetOut[0] = INT_TO_FLOAT( localSrc[0] );
                    targetIn += 4;
                    targetOut += 4;
                    localSrc += channelCount;
                }
                src = (uint8_t*)((char*) src + inBytesPerRow);
            }
            
            loopCount--;
            data++;
        }
        else
        {
            float *target1 = targetIn;
            float *target2 = targetOut;
            
            for( j = 0; j < height; j++ )
            {
                for( i = 0; i < width; i++ )
                    targetIn[4 * i] = targetOut[4 * i] = 1.0f;
            
                target1 = (float*)((char*) target1  + bytesPerRow );
                target2 = (float*)((char*) target2  + bytesPerRow );
            }            
        }

        bitsPerChannel /= channelCount;
        targetIn = (float*) inChannels[0] + 1;
        targetOut = (float*) outChannels[0] + 1;
        
        if( loopCount > 3 )
            loopCount = 3;
        
        for( i = 0; i < loopCount; i++ )
        {
            float *dest1 = targetIn;
            float *dest2 = targetOut;
            
            src = data;
            
            for( j = 0; j < height; j++ )
            {
                uint8_t *localSrc = src; 
                
                for( k = 0; k < width; k++ )
                {
                    dest1[4 * k] = dest2[4 * k] = INT_TO_FLOAT( localSrc[0] );

                    localSrc += channelCount;
                }
    
                dest1 = (float*) ((char*) dest1 + bytesPerRow );
                dest2 = (float*) ((char*) dest2 + bytesPerRow );
                src = (uint8_t*)((char*) src + inBytesPerRow);
            }
            
            targetIn++;
            targetOut++;
            data++;
        }
    }
    
    return 0;
}
@end