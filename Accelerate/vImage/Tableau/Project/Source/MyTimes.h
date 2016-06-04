
/*
 *  MyTimes.h
 *  
 *
 *  Created by iollmann on Fri Apr 05 2002.
 *  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 */

#if defined( __APPLE__ )
    #if defined( __MWERKS__ )
            typedef unsigned long long u_int64_t;
    #else
            #include <Carbon/Carbon.h>
    #endif
#endif


#if defined( __ppc__ )

    static uint64_t	MyGetTime( void )
    {
        AbsoluteTime theTime = UpTime();

        return ((uint64_t*) &theTime )[0];
    }

    static double MySubtractTime( uint64_t endTime, uint64_t startTime )
    {
        union
        {
            Nanoseconds	ns;
            u_int64_t	i;
        }time;

        time.ns = AbsoluteToNanoseconds( SubAbsoluteFromAbsolute( ((AbsoluteTime*) &endTime)[0], ((AbsoluteTime*) &startTime)[0] ) );
        return time.i * 1e-9;
    }

#endif

#if defined( __i386__ )

    #include <unistd.h>

    #if ! defined( __APPLE__ )
 	typedef unsigned long long uint64_t;
    #endif

    static double 			conversionFactor = 0.0;
    static inline uint64_t	MyGetTime( void );
    static double 			GetCPUFrequency( void );
    static double 			MySubtractTime( uint64_t endTime, uint64_t startTime );

    static inline uint64_t	MyGetTime( void )
    {
        register uint64_t	time;
        asm volatile ( "rdtsc": "=A" (time ) );
        return time;
    }


    static double MySubtractTime( uint64_t endTime, uint64_t startTime )
    {
        double diff = (double) ( endTime - startTime );

        if( 0.0 == conversionFactor )
            conversionFactor = 1.0 / GetCPUFrequency();

        return diff * conversionFactor;
    }


    #if defined( __APPLE__ )

        static double GetCPUFrequency( void )
        {
            uint64_t 	startCycles, endCycles;
            uint64_t	startNanoseconds, endNanoseconds;
            int		kLoopDuration = 10000;

            //record the start time on both clocks
            ((Nanoseconds*) &startNanoseconds)[0] = AbsoluteToNanoseconds( UpTime() );
            asm volatile ( "rdtsc": "=A" (startCycles ) );
    
            do
            {
                //wait 1 millisecond
                usleep( 1000 );
    
                //record the new time
                ((Nanoseconds*) &endNanoseconds)[0] = AbsoluteToNanoseconds( UpTime() );
                asm volatile ( "rdtsc": "=A" (endCycles ) );
    
                //loop just in case the kernel doesn't sleep
            }while( startNanoseconds + kLoopDuration > endNanoseconds );
    
            //WARNING possible race condition if we get preempted between sysclk_gettime_internal and rdtsc
    
            return 1e9 * (double) (endCycles - startCycles) / (double) (endNanoseconds - startNanoseconds);
        }

    #else

        #include <sys/time.h>
        #include <stdlib.h>
    
        static double GetCPUFrequency( void )
        {
            int x, y;
            uint64_t 		startCycles, endCycles;
            uint64_t		startNanoseconds, endNanoseconds;
            struct timeval	startTime, endTime;
            struct timezone	zone;
            int			kLoopDuration = 10000;
            double		elapsedTime;
        
            //record the start time on both clocks
            gettimeofday( &startTime, &zone );
            asm volatile ( "rdtsc": "=A" (startCycles ) );
            startNanoseconds = (uint64_t) startTime.tv_sec * 1000000000 + (uint64_t) startTime.tv_usec * 1000;
    
            do
            {
                //wait 1 millisecond
                usleep( 1000 );
    
                //record the new time
                gettimeofday( &endTime, &zone );
                asm volatile ( "rdtsc": "=A" (endCycles ) );
                endNanoseconds = (uint64_t) endTime.tv_sec * 1000000000 + (uint64_t) endTime.tv_usec * 1000;
    
                //loop just in case the kernel doesn't sleep
            }while( startNanoseconds + kLoopDuration > endNanoseconds );
    
            //WARNING possible race condition if we get preempted between sysclk_gettime_internal and rdtsc
    
            return 1e9 * (double) (endCycles - startCycles) / (double) (endNanoseconds - startNanoseconds);
        }
        
    #endif

#endif
