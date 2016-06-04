/*

File: mandelToolMain.c

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright 2002,2004 Apple Computer, Inc., All Rights Reserved

*/ 

#include <stdio.h>
#include <stdlib.h>
#include "mandelTool.h"

#include <sys/sysctl.h>
#include <libkern/OSByteOrder.h>

static int sysctlbynameuint64 ( const char* name, uint64_t * value )
{   
    int result;
    size_t  size = sizeof( *value );
    result = sysctlbyname ( name, value, & size, NULL, 0 );
    if ( result == 0 )
    {
        if ( size == sizeof( uint64_t ) )
            ;
        else if ( size == sizeof( uint32_t ) )
            *value = * ( uint32_t *) value;
        else if ( size == sizeof( uint16_t ) )
            *value = * ( uint16_t *) value;
        else if ( size == sizeof( uint8_t ) )
            *value = * ( uint8_t *) value;
        else
        {
#if DEBUG
            fprintf ( stderr, "** ERROR: sysctlbyname() returned something other than a known sized item uint64_t sized result when called for %s, size=%d\n", name ? name : "NULL", size );
#endif
        }
    }
    return result;
}

int hasNamedVectorUnit(const char* name)
{
    char buffer[1024];
    uint64_t val;
    snprintf(buffer, sizeof(buffer), "hw.optional.%s", name);
    return sysctlbynameuint64(buffer, &val) == 0? val == 1 : 0;
}

enum
{
    kScalarOnly = 0,
    kAltiVec = 1,
    kMMXPresent = 2,
    kMMXandSSEPresent = 3,
    kMMXandSSEandSSE2Present = 4,
    kMMXandSSEandSSE2andSSE3Present = 5
    /* larger values reserved for future expansion */
};

int GetVectorTypeAvailable( void )
{
#if __i386__
    if (hasNamedVectorUnit("sse3"))
        return kMMXandSSEandSSE2andSSE3Present;
    if (hasNamedVectorUnit("sse2"))
        return kMMXandSSEandSSE2Present;
    if (hasNamedVectorUnit("sse"))
        return kMMXandSSEPresent;
    if (hasNamedVectorUnit("mmx"))
        return kMMXPresent;
#endif
#if __ppc__
    if (hasNamedVectorUnit("altivec"))
        return kAltiVec;
#endif
    return kScalarOnly;
}

int main ( int argc, char **argv){

    unsigned long *itArray;
    MandelbrotIterationRec	info;
    int xLoc, yLoc, width, height;
    int useVector;
    int vectorTypeAvailable;
    int hasAltivec = FALSE;
    
    int counter = 1;
	
    if ( argc != 11){
        fprintf( stderr, "Usage: \n%s xLocInPixels yLocInPixels pixWidth pixHeight imgWidthFloat imgHeightFloat imgLeftFloat imgTopFloat maxIterations useAltivec[0|1]\n",argv[0]);
        exit(-1);
    }

    xLoc = atoi(argv[1]);
    yLoc = atoi(argv[2]);
    width = atoi(argv[3]);
    height = atoi(argv[4]);
    
    itArray = (unsigned long *)malloc( sizeof(long) * width * height );

    info.itArray   = itArray;
    info.pixwidth  = width;
    info.pixheight = height ;
    info.imgwidth  = atof( argv[5] ) ;
    info.imgheight = atof( argv[6] ) ;
    info.imgleft   = atof( argv[7] ) ;
    info.imgtop    = atof( argv[8] ) ;
    info.maxIterations = atoi( argv[9] );

    useVector = atoi( argv[10] );

    vectorTypeAvailable = GetVectorTypeAvailable();
    
    if (vectorTypeAvailable == kAltiVec) hasAltivec = TRUE;
    
    while (counter--) {
    
        if (useVector && hasAltivec) {
                vectorcalc(&info);
        } else {
                scalarcalc(&info);
        }

    }
    
    uint32_t xLocSwapped = OSSwapHostToBigInt32(xLoc);
    uint32_t yLocSwapped = OSSwapHostToBigInt32(yLoc);
    uint32_t widthSwapped = OSSwapHostToBigInt32(width);
    uint32_t heightSwapped = OSSwapHostToBigInt32(height);
    
    fwrite( &xLocSwapped,   sizeof(uint32_t), 1, stdout);
    fwrite( &yLocSwapped,   sizeof(uint32_t), 1, stdout);
    fwrite( &widthSwapped,  sizeof(uint32_t), 1, stdout);
    fwrite( &heightSwapped, sizeof(uint32_t), 1, stdout);

    fwrite(info.itArray, sizeof(long), info.pixwidth * info.pixheight, stdout);

    return 0;
}
