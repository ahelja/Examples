/*

File: mandeltool.c

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
#include <libkern/OSByteOrder.h>
#include "mandelTool.h"

/*
 The ratio pixwidth:pixheight must be identical to height:width,
 otherwise the returned image will be scaled
*/

void vectorcalc(MandelbrotIterationRecPtr param)
{
#if __i386__
#endif
#if __ppc__
    unsigned long			i, j, iterations;
    scalar_float	 		stepx = ((scalar_float)(param->imgwidth))/param->pixwidth;
    scalar_float 			stepy = ((scalar_float)(param->imgheight))/param->pixheight;
    vector float			vStepX;
    vector float			vStepY;
    vector float			vReal;
    vector float			vImag;
    vector float			vRealTemp;
    vector float			vImagTemp;
    vector float			vMagnitude;
    vector float			vLimit;
    vector unsigned int		vIterations;
    vector bool int			vComparison;
    vector float			vRealRowStart;
    vector float			vImagRowStart;
    vector float			vRealBase;
    vector float			vImagBase;
    floatsvec				vTransfer1, vTransfer2;
    unsigned long			stepwidth, stepheight;
	unsigned long			maxIterations = param->maxIterations;

    vector unsigned int	*pOutput = (vector unsigned int*)param->itArray;

    stepwidth = param->pixwidth/4;
    stepheight = param->pixheight;

    vTransfer1.floats[0] = param->imgleft;
    vTransfer1.floats[1] = param->imgleft+stepx;
    vTransfer1.floats[2] = param->imgleft+2*stepx;
    vTransfer1.floats[3] = param->imgleft+3*stepx;

    vTransfer2.floats[0] = BREAK_VALUE;
    vTransfer2.floats[1] = 4*stepx;
    vTransfer2.floats[2] = stepy;
    vTransfer2.floats[3] = param->imgtop;

    vRealRowStart = vTransfer1.vec;

    vLimit = vec_splat(vTransfer2.vec, 0);
    vStepX = vec_splat(vTransfer2.vec, 1);
    vStepY = vec_splat(vTransfer2.vec, 2);
    vImagRowStart = vec_splat(vTransfer2.vec, 3);

    for (i=0; i<stepheight; i++) {

        vImagBase = vImagRowStart;
        vRealBase = vRealRowStart;

        for (j=0; j<stepwidth; j++) {

            vReal = vRealBase;
            vImag = vImagBase;

            vIterations = (vector unsigned int)(0);

            for (iterations = 0; iterations < maxIterations; iterations++) {

                // real = re*re - im*im + RealBase
                vRealTemp = vec_madd(vReal, vReal, vRealBase);
                vRealTemp = vec_nmsub(vImag, vImag, vRealTemp);

                // imag = re*im + re*im + ImBase
                vImagTemp = vec_madd(vImag, vReal, vImagBase);
                vImagTemp = vec_madd(vImag, vReal, vImagTemp);

                // do calculation to see if we've reached an escape point
                vMagnitude = vec_madd(vRealTemp, vRealTemp, (vector float)(0));
                vMagnitude = vec_madd(vImagTemp, vImagTemp, vMagnitude);

                vReal	= vRealTemp;
                vImag 	= vImagTemp;

                vComparison = vec_cmplt(vMagnitude, vLimit);

                vIterations = vec_add(vComparison, vIterations);

                if (vec_all_eq(vComparison, (vector bool int)(0))) {
                    break;
                }
            }

            *pOutput++ = vec_sub((vector unsigned int)(0), vIterations);

            vRealBase = vec_add(vRealBase, vStepX);
        }

        vImagRowStart = vec_add(vStepY, vImagRowStart);
    }
#endif
}

void scalarcalc(MandelbrotIterationRecPtr param)
{
    scalar_float	 				stepx = ((scalar_float)(param->imgwidth))/(scalar_float)param->pixwidth;
    scalar_float 					stepy = ((scalar_float)(param->imgheight))/(scalar_float)param->pixheight;
    scalar_float					re;
    scalar_float					im;
    scalar_float					rebase;
    scalar_float					imbase;
    unsigned long					i, j;
    unsigned long					*steps;
    long							iterations;
    scalar_float					xTemp;
    scalar_float					yTemp;
    unsigned long					stepwidth, stepheight;
    scalar_float					imgleft = param->imgleft;
    scalar_float					imgtop = param->imgtop;
    scalar_float					realrowstart;
    scalar_float					imagrowstart;
	unsigned long					maxIterations = param->maxIterations;

    stepwidth 				= param->pixwidth;
    stepheight 				= param->pixheight;

    steps 					= param->itArray;

    realrowstart 			= imgleft;
    imagrowstart 			= imgtop;

    for (i=0; i<stepheight; i++) {

        imbase = imagrowstart + i*stepy;

        for (j=0; j<stepwidth; j++) {

	        rebase = realrowstart + j*stepx;

            re = rebase;
            im = imbase;

            for (iterations = 0; iterations < maxIterations; iterations++) {
			
                xTemp = re*re - im*im;
                yTemp = 2*re*im;

                xTemp += rebase;
                yTemp += imbase;

                re = xTemp;
                im = yTemp;

                if (re*re+im*im >= BREAK_VALUE) break;
            }

            *steps++ = OSSwapHostToBigInt32(iterations);
        }
    }
}
