/*******************************************************************************
*                                                                              *
*     File Name: ConvolutionSample.c                                           *
*                                                                              *
*     A sample program to illustrate the usage of convolution and correlation  *
*     functions.  This program also times the functions using the microsecond  *
*     timer.                                                                   *
*                                                                              *
*     Copyright © 2000-2003 Apple Computer, Inc.  All rights reserved.         *
*                                                                              *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <CoreServices/CoreServices.h>
#include <Accelerate/Accelerate.h>


#include "main.h"
#include "javamode.h"



#define MAX_LOOP_NUM 1000    // Number of iterations used in the timing loop


void Compare           ( float *original, float *computed, long length );
void Dummy_Conv        ( float *signal, SInt32 signalStride, float  *filter, SInt32 filterStride, float  *result, SInt32 resultStride, SInt32 resultLength, SInt32 filterLength );

void ConvTiming ( );


void RunConvolutionSample(void)
	{
	ConvTiming();     
	}


      
/*******************************************************************************
*     Convolution.                                                             *
********************************************************************************
*                                                                              *
*     This function performs at 3.69 gigaflops for a convolution of size       *
*     ( 2048 x 256 ) on a 500mhz vector enabled processor.                     *
*                                                                              *
*******************************************************************************/

void ConvTiming ( )
      {
      float  *signal, *filter, *result;
      SInt32 signalStride, filterStride, resultStride;
      UInt32 lenSignal, filterLength, resultLength;
      UInt32 i;
      
      filterLength = 256;
      resultLength = 2048;
      lenSignal = ( ( filterLength + 3 ) & 0xFFFFFFFC ) + resultLength;
      
      signalStride = filterStride = resultStride = 1;
      
      printf("\nConvolution ( resultLength = %d, filterLength = %d )\n\n", (unsigned int)resultLength, (unsigned int)filterLength);
      
      // Allocate memory for the input operands and check its availability.
      signal = ( float* ) malloc ( lenSignal * sizeof ( float ) );
      filter = ( float* ) malloc ( filterLength * sizeof ( float ) );
      result = ( float* ) malloc ( resultLength * sizeof ( float ) );
      
      if( signal == NULL || filter == NULL || result == NULL )      
            {
            printf ( "\nmalloc failed to allocate memory for the convolution sample.\n");
            exit ( 0 );
            }      
      
      // Set the input signal of length "lenSignal" to [1,...,1].
      for( i = 0; i < lenSignal; i++ )
            signal[i] = 1.0;
      
      // Set the filter of length "filterLength" to [1,...,1].
      for( i = 0; i < filterLength; i++ )
            filter[i] = 1.0;
      
      // Correlation.
      conv ( signal, signalStride, filter, filterStride, result, resultStride, resultLength, filterLength );
      
      // Carry out a convolution.
      filterStride = -1;
      conv ( signal, signalStride, filter + filterLength - 1, filterStride, result, resultStride, resultLength, filterLength );
            
      // Timing section for the convolution.
            {
            float time, overheadTime;
            float GFlops;

#if defined(__VEC__)
            // Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
            // WARNING:  Java mode has to be treated with care.  Some algorithms may be
            // sensitive to flush to zero and may need proper IEEE-754 denormal handling.
            TurnJavaModeOffOnG4();
#endif

            StartClock ( );
            for ( i = 0; i < MAX_LOOP_NUM; i++ )
                  conv ( signal, signalStride, filter, filterStride, result, resultStride, resultLength, filterLength );
            StopClock ( &time );

#if defined(__VEC__)
            // Restore Java mode.
            RestoreJavaModeOnG4();
#endif

            // Measure and take off the calling overhead of the convolution (very minimal impact).
            StartClock ( );
            for ( i = 0; i < MAX_LOOP_NUM; i++ )
                  Dummy_Conv ( signal, signalStride, filter, filterStride, result, resultStride, resultLength, filterLength );
            StopClock ( &overheadTime );
            
            time -= overheadTime;   
            time /= MAX_LOOP_NUM;
            
            GFlops = ( 2.0f * filterLength - 1.0f ) * resultLength / ( time * 1000.0f );
            
            printf("Time for a ( %d x %d ) Convolution is %4.4f µsecs or %4.4f GFlops\n\n", (unsigned int)resultLength, (unsigned int)filterLength, time, GFlops );
            }
      
      // Free allocated memory.
      free ( signal );
      free ( filter );
      free ( result );
      }      
      

   


// Dummy functions that are used to measure the overhead time for the function call.
void Dummy_Conv ( float *signal, SInt32 signalStride, float *filter, SInt32 filterStride, float *result, SInt32 resultStride, SInt32 resultLength, SInt32 filterLength )
      {
      #pragma unused( signal )
      #pragma unused( signalStride )
      #pragma unused( filter )
      #pragma unused( filterStride )
      #pragma unused( result )
      #pragma unused( resultStride )
      #pragma unused( resultLength )
      #pragma unused( filterLength )
      }  
      
