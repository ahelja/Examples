/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*=============================================================================
	PCMBlitterLibPPC.c
	
=============================================================================*/

#include <TargetConditionals.h>

/*
	N.B. This file should be compiled with:
		-fno-schedule-insns -fno-schedule-insns2 -mtune=G5
*/

#if TARGET_CPU_PPC
#include "PCMBlitterLib.h"


//#undef __lhbrx
#define __lhbrx(base, offset) \
	({ register UInt32 result; \
	__asm__ ("lhbrx %0, %2, %1" : "=r" (result) : "r"  (base), "bO" (offset)); \
	/*return*/ result; })

#define __lwbrx(base, offset)	\
	({ register long result; \
	__asm__ __volatile__("lwbrx %0, %1, %2" : "=r" (result) : "b%" (offset), "r" (base) : "memory" ); \
	/*return*/ result; } )

#define __rlwimi( rA, rS, cnt, mb, me ) \
	({ __asm__ __volatile__( "rlwimi %0, %2, %3, %4, %5" : "=r" (rA) : "0" (rA), "r" (rS), "n" (cnt), "n" (mb), "n" (me) ); /*return*/ rA; })

#define __stwbrx( value, index, base ) \
	__asm__( "stwbrx %0, %1, %2" : : "r" (value), "b%" (index), "r" (base) : "memory" )

/*
 * __sthbrx - Store Half Word Byte-Reverse Indexed
 *
 *   int __sthbrx(unsigned short, void *, int);
 */
#define __sthbrx(value, base, index)    \
  __asm__ ("sthbrx %0, %1, %2" : : "r" (value), "b%" (index), "r" (base) : "memory")

#define __stfiwx( value, offset, addr )			\
	asm( "stfiwx %0, %1, %2" : /*no result*/ : "f" (value), "b%" (offset), "r" (addr) : "memory" )

#define __rlwimi_volatile( rA, rS, cnt, mb, me ) \
	({ __asm__ __volatile__( "rlwimi %0, %2, %3, %4, %5" : "=r" (rA) : "0" (rA), "r" (rS), "n" (cnt), "n" (mb), "n" (me) ); /*return*/ rA; })

static inline double __fctiw( register double B )
{
	register double result;
	asm( "fctiw %0, %1" : "=f" (result) : "f" (B)  );
	return result;
}

void NativeInt16ToFloat32_Scalar( const SInt16 *src, Float32 *dest, unsigned int count)
{
	register Float32 bias;
	register SInt32 exponentMask = (0x87UL << 23) | 0x8000;	//FP exponent + bias for sign
	register SInt32 int0, int1, int2, int3;
	register Float32 float0, float1, float2, float3;
	register unsigned long loopCount;
	volatile union		// making these volatile keeps the optimizer from reusing them and introducing stalls
	{
		Float32	f;
		SInt32	i;
	} buffer0, buffer1, buffer2, buffer3;

	buffer0.i = exponentMask;
	bias = buffer0.f;

	if( count >= 8 )
	{
		//Software Cycle 1
		int0 = src[0];

		//Software Cycle 2
		int1 = src[1];
		int0 += exponentMask;

		//Software Cycle 3
		int2 = src[2];
		int1 += exponentMask;
		buffer0.i = int0;

		//Software Cycle 4
		int3 = src[3];
		int2 += exponentMask;
		buffer1.i = int1;
		//delay one loop for the store to complete

		//Software Cycle 5
		int0 = src[4];
		int3 += exponentMask;
		buffer2.i = int2;
		float0 = buffer0.f;

		//Software Cycle 6
		int1 = src[5];
		int0 += exponentMask;
		buffer3.i = int3;
		float1 = buffer1.f;
		float0 -= bias;

		//Software Cycle 7
		int2 = src[6];
		int1 += exponentMask;
		buffer0.i = int0;
		float2 = buffer2.f;
		float1 -= bias;

		dest--;
		//Software Cycle 8
		int3 = src[7];
		int2 += exponentMask;
		buffer1.i = int1;
		float3 = buffer3.f;
		float2 -= bias;
		*++dest = float0;
		
		src += 8;
		count -= 8;
		loopCount = count / 4;
		count &= 3;
		while( loopCount-- )
		{

			//Software Cycle A
			int0 = src[0];
			int3 += exponentMask;
			buffer2.i = int2;
			float0 = buffer0.f;
			float3 -= bias;
			*++dest = float1;

			//Software Cycle B
			int1 = src[1];
			int0 += exponentMask;
			buffer3.i = int3;
			float1 = buffer1.f;
			float0 -= bias;
			*++dest = float2;

			//Software Cycle C
			int2 = src[2];
			int1 += exponentMask;
			buffer0.i = int0;
			float2 = buffer2.f;
			float1 -= bias;
			*++dest = float3;

			//Software Cycle D
			int3 = src[3];
			int2 += exponentMask;
			buffer1.i = int1;
			float3 = buffer3.f;
			float2 -= bias;
			*++dest = float0;
			
			src += 4;
		}

		//Software Cycle 7
		int3 += exponentMask;
		buffer2.i = int2;
		float0 = buffer0.f;
		float3 -= bias;
		*++dest = float1;

		//Software Cycle 6
		buffer3.i = int3;
		float1 = buffer1.f;
		float0 -= bias;
		*++dest = float2;

		//Software Cycle 5
		float2 = buffer2.f;
		float1 -= bias;
		*++dest = float3;

		//Software Cycle 4
		float3 = buffer3.f;
		float2 -= bias;
		*++dest = float0;

		//Software Cycle 3
		float3 -= bias;
		*++dest = float1;

		//Software Cycle 2
		*++dest = float2;

		//Software Cycle 1
		*++dest = float3;

		++dest;
	}


	while( count-- )
	{
		register SInt32 value = *src++;
		value += exponentMask;
		((SInt32 *) dest)[0] = value;
		dest[0] -= bias;
		dest++;
	}
}

void SwapInt16ToFloat32_Scalar( const SInt16 *src, Float32 *dest, unsigned int count)
{
	register Float32 bias;
	register SInt32 exponentMask = (0x87UL << 23) | 0x8000;	//FP exponent + bias for sign
	register UInt32 short0, short1, short2, short3;
	register SInt32 int0, int1, int2, int3;
	register Float32 float0, float1, float2, float3;
	volatile union		// making these volatile keeps the optimizer from reusing them and introducing stalls
	{
		Float32	f;
		SInt32	i;
	} buffer0, buffer1, buffer2, buffer3;

	buffer0.i = exponentMask;
	bias = buffer0.f;
	--dest;	// will preincrement stores
	
	/*
		$short = __lhbrx(src++, 0);
		$int = $short;
		$int += exponentMask;
		$buffer.i = $int;
		$float = $buffer.f;
		$float -= bias;
		*++dest = $float;
	*/

	if (count >= 8) {
		// Cycle 1
		short0 = __lhbrx(src, 0);

		// Cycle 2
		short1 = __lhbrx(src, 2);
		int0 = (SInt16)short0;

		// Cycle 3
		short2 = __lhbrx(src, 4);
		int1 = (SInt16)short1;
		int0 += exponentMask;

		// Cycle 4
		short3 = __lhbrx(src, 6);
		int2 = (SInt16)short2;
		int1 += exponentMask;
		buffer0.i = int0;

		// Cycle 5
		short0 = __lhbrx(src, 8);
		int3 = (SInt16)short3;
		int2 += exponentMask;
		buffer1.i = int1;
		float0 = buffer0.f;

		// Cycle 6
		short1 = __lhbrx(src, 10);
		int0 = (SInt16)short0;
		int3 += exponentMask;
		buffer2.i = int2;
		float1 = buffer1.f;
		float0 -= bias;

		// Cycle 7
		short2 = __lhbrx(src, 12);
		int1 = (SInt16)short1;
		int0 += exponentMask;
		buffer3.i = int3;
		float2 = buffer2.f;
		float1 -= bias;
		*++dest = float0;

		// Cycle 8
		short3 = __lhbrx(src, 14);
		int2 = (SInt16)short2;
		int1 += exponentMask;
		buffer0.i = int0;
		float3 = buffer3.f;
		float2 -= bias;
		*++dest = float1;
		src += 8;
		count -= 8;

		for ( ; count >= 4; count -= 4) {
			// Cycle A
			short0 = __lhbrx(src, 0);
			int3 = (SInt16)short3;
			int2 += exponentMask;
			buffer1.i = int1;
			float0 = buffer0.f;
			float3 -= bias;
			*++dest = float2;

			// Cycle B
			short1 = __lhbrx(src, 2);
			int0 = (SInt16)short0;
			int3 += exponentMask;
			buffer2.i = int2;
			float1 = buffer1.f;
			float0 -= bias;
			*++dest = float3;

			// Cycle C
			short2 = __lhbrx(src, 4);
			int1 = (SInt16)short1;
			int0 += exponentMask;
			buffer3.i = int3;
			float2 = buffer2.f;
			float1 -= bias;
			*++dest = float0;

			// Cycle D
			short3 = __lhbrx(src, 6);
			int2 = (SInt16)short2;
			int1 += exponentMask;
			buffer0.i = int0;
			float3 = buffer3.f;
			float2 -= bias;
			*++dest = float1;
			src += 4;
		}

		// Cycle 6
		int3 = (SInt16)short3;
		int2 += exponentMask;
		buffer1.i = int1;
		float0 = buffer0.f;
		float3 -= bias;
		*++dest = float2;

		// Cycle 5
		int3 += exponentMask;
		buffer2.i = int2;
		float1 = buffer1.f;
		float0 -= bias;
		*++dest = float3;

		// Cycle 4
		buffer3.i = int3;
		float2 = buffer2.f;
		float1 -= bias;
		*++dest = float0;

		// Cycle 3
		float3 = buffer3.f;
		float2 -= bias;
		*++dest = float1;

		// Cycle 2
		float3 -= bias;
		*++dest = float2;

		// Cycle 1
		*++dest = float3;
	}

	while (count--) {
		short0 = __lhbrx(src++, 0);
		int0 = (SInt16)short0;
		int0 += exponentMask;
		buffer0.i = int0;
		float0 = buffer0.f;
		float0 -= bias;
		*++dest = float0;
	}
}

void NativeInt24ToFloat32_Scalar( const SInt32 *src, Float32 *dest, unsigned int count )
{
	union
	{
		double		d[4];
		unsigned int	i[8];
	}transfer;
	register double			dBias;
	register unsigned int	loopCount, load0SignMask;
	register unsigned long	load0, load1, load2;
	register unsigned long	int0, int1, int2, int3;
	register double			d0, d1, d2, d3;
	register Float32		f0, f1, f2, f3;

	transfer.i[0] = transfer.i[2] = transfer.i[4] = transfer.i[6] = (0x434UL - 24) << 20; //0x41C00000UL;
	transfer.i[1] = 0x00800000;
	int0 = int1 = int2 = int3 = 0;
	load0SignMask = 0x80000080UL;

	dBias = transfer.d[0];

	src--;
	dest--;

	if( count >= 8 )
	{
		count -= 8;
		loopCount = count / 4;
		count &= 3;

		//Virtual cycle 1
		load0 = (++src)[0];

		//Virtual cycle 2
		load1 = (++src)[0];
		load0 ^= load0SignMask;

		//Virtual cycle 3
		load2 = (++src)[0];
		load1 ^= 0x00008000UL;
		int0 = load0 >> 8;
		int1 = __rlwimi( int1, load0, 16, 8, 15);

		//Virtual cycle 4
		//No load3 -- already loaded last cycle
		load2 ^= 0x00800000UL;
		int1 = __rlwimi( int1, load1, 16, 16, 31);
		int2 = __rlwimi( int2, load1, 8, 8, 23 );
		transfer.i[1] = int0;

		//Virtual cycle 5
		load0 = (++src)[0];
		int2 = __rlwimi( int2, load2, 8, 24, 31 );
		int3 = load2 & 0x00FFFFFF;
		transfer.i[3] = int1;

		//Virtual cycle 6
		load1 = (++src)[0];
		load0 ^= load0SignMask;
		transfer.i[5] = int2;
		d0 = transfer.d[0];

		//Virtual cycle 7
		load2 = (++src)[0];
		load1 ^= 0x00008000UL;
		int0 = load0 >> 8;
		int1 = __rlwimi( int1, load0, 16, 8, 15 );
		transfer.i[7] = int3;
		d1 = transfer.d[1];
		d0 -= dBias;

		//Virtual cycle 8
		//No load3 -- already loaded last cycle
		load2 ^= 0x00800000UL;
		int1 = __rlwimi( int1, load1, 16, 16, 31);
		int2 = __rlwimi( int2, load1, 8, 8, 23 );
		transfer.i[1] = int0;
		d2 = transfer.d[2];
		d1 -= dBias;
		f0 = d0;

		while( loopCount-- )
		{
			//Virtual cycle A
			load0 = (++src)[0];
			int2 = __rlwimi( int2, load2, 8, 24, 31 );
			int3 = load2 & 0x00FFFFFF;
			transfer.i[3] = int1;
			d3 = transfer.d[3];
			d2 -= dBias;
			f1 = d1;
			(++dest)[0] = f0;

			//Virtual cycle B
			load1 = (++src)[0];
			load0 ^= load0SignMask;
			transfer.i[5] = int2;
			d0 = transfer.d[0];
			d3 -= dBias;
			f2 = d2;
			(++dest)[0] = f1;

			//Virtual cycle C
			load2 = (++src)[0];
			load1 ^= 0x00008000UL;
			int0 = load0 >> 8;
			int1 = __rlwimi( int1, load0, 16, 8, 15 );
			transfer.i[7] = int3;
			d1 = transfer.d[1];
			d0 -= dBias;
			f3 = d3;
			(++dest)[0] = f2;

			//Virtual cycle D
			load2 ^= 0x00800000UL;
			int1 = __rlwimi( int1, load1, 16, 16, 31);
			int2 = __rlwimi( int2, load1, 8, 8, 23 );
			transfer.i[1] = int0;
			d2 = transfer.d[2];
			d1 -= dBias;
			f0 = d0;
			(++dest)[0] = f3;
		}

		//Virtual cycle 8
		int2 = __rlwimi( int2, load2, 8, 24, 31 );
		int3 = load2 & 0x00FFFFFF;
		transfer.i[3] = int1;
		d3 = transfer.d[3];
		d2 -= dBias;
		f1 = d1;
		(++dest)[0] = f0;

		//Virtual cycle 7
		transfer.i[5] = int2;
		d0 = transfer.d[0];
		d3 -= dBias;
		f2 = d2;
		(++dest)[0] = f1;

		//Virtual cycle 6
		transfer.i[7] = int3;
		d1 = transfer.d[1];
		d0 -= dBias;
		f3 = d3;
		(++dest)[0] = f2;

		//Virtual cycle 5
		d2 = transfer.d[2];
		d1 -= dBias;
		f0 = d0;
		(++dest)[0] = f3;

		//Virtual cycle 4
		d3 = transfer.d[3];
		d2 -= dBias;
		f1 = d1;
		(++dest)[0] = f0;

		//Virtual cycle 3
		d3 -= dBias;
		f2 = d2;
		(++dest)[0] = f1;

		//Virtual cycle 2
		f3 = d3;
		(++dest)[0] = f2;

		//Virtual cycle 1
		(++dest)[0] = f3;
	}

	src = (SInt32*) ((char*) src + 1 );
	while( count-- )
	{
		int0 = ((unsigned char*)(src = (SInt32*)( (char*) src + 3 )))[0];
		int1 = ((unsigned short*)( (char*) src + 1 ))[0];
		int0 ^= 0x00000080UL;
		int1 = __rlwimi( int1, int0, 16, 8, 15 );
		transfer.i[1] = int1;

		d0 = transfer.d[0];
		d0 -= dBias;
		f0 = d0;
		(++dest)[0] = f0;
   }
}

void SwapInt24ToFloat32_Scalar( const SInt32 *src, Float32 *dest, unsigned int count )
{
	union
	{
		double		d[4];
		unsigned int	i[8];
	}transfer;
	register double			dBias;
	register unsigned int	loopCount, load2SignMask;
	register unsigned long	load0, load1, load2;
	register unsigned long	int0, int1, int2, int3;
	register double			d0, d1, d2, d3;
	register Float32		f0, f1, f2, f3;

	transfer.i[0] = transfer.i[2] = transfer.i[4] = transfer.i[6] = (0x42CUL - 24) << 20; //0x41400000UL;
	transfer.i[1] = 0x80000000;
	int0 = int1 = int2 = int3 = 0;
	load2SignMask = 0x80000080UL;

	dBias = transfer.d[0];

	src--;
	dest--;

	if( count >= 8 )
	{
		count -= 8;
		loopCount = count / 4;
		count &= 3;

		//Virtual cycle 1
		load0 = (++src)[0];

		//Virtual cycle 2
		load1 = (++src)[0];
		load0 ^= 0x00008000;

		//Virtual cycle 3
		load2 = (++src)[0];
		load1 ^= 0x00800000UL;
		int0 = load0 >> 8;
		int1 = __rlwimi( int1, load0, 16, 8, 15);

		//Virtual cycle 4
		//No load3 -- already loaded last cycle
		load2 ^= load2SignMask;
		int1 = __rlwimi( int1, load1, 16, 16, 31);
		int2 = __rlwimi( int2, load1, 8, 8, 23 );
		__stwbrx( int0, 0, &transfer.i[1]);

		//Virtual cycle 5
		load0 = (++src)[0];
		int2 = __rlwimi( int2, load2, 8, 24, 31 );
		int3 = load2 & 0x00FFFFFF;
		__stwbrx( int1, 0, &transfer.i[3]);

		//Virtual cycle 6
		load1 = (++src)[0];
		load0 ^= 0x00008000;
		__stwbrx( int2, 0, &transfer.i[5]);
		d0 = transfer.d[0];

		//Virtual cycle 7
		load2 = (++src)[0];
		load1 ^= 0x00800000UL;
		int0 = load0 >> 8;
		int1 = __rlwimi( int1, load0, 16, 8, 15 );
		__stwbrx( int3, 0, &transfer.i[7]);
		d1 = transfer.d[1];
		d0 -= dBias;

		//Virtual cycle 8
		//No load3 -- already loaded last cycle
		load2 ^= load2SignMask;
		int1 = __rlwimi( int1, load1, 16, 16, 31);
		int2 = __rlwimi( int2, load1, 8, 8, 23 );
		__stwbrx( int0, 0, &transfer.i[1]);
		d2 = transfer.d[2];
		d1 -= dBias;
		f0 = d0;

		while( loopCount-- )
		{
			//Virtual cycle A
			load0 = (++src)[0];
			int2 = __rlwimi( int2, load2, 8, 24, 31 );
			int3 = load2 & 0x00FFFFFF;
			__stwbrx( int1, 0, &transfer.i[3]);
			d3 = transfer.d[3];
			d2 -= dBias;
			f1 = d1;
			(++dest)[0] = f0;

			//Virtual cycle B
			load1 = (++src)[0];
			load0 ^= 0x00008000;
			__stwbrx( int2, 0, &transfer.i[5]);
			d0 = transfer.d[0];
			d3 -= dBias;
			f2 = d2;
			(++dest)[0] = f1;

			//Virtual cycle C
			load2 = (++src)[0];
			load1 ^= 0x00800000UL;
			int0 = load0 >> 8;
			int1 = __rlwimi( int1, load0, 16, 8, 15 );
			__stwbrx( int3, 0, &transfer.i[7]);
			d1 = transfer.d[1];
			d0 -= dBias;
			f3 = d3;
			(++dest)[0] = f2;

			//Virtual cycle D
			load2 ^= load2SignMask;
			int1 = __rlwimi( int1, load1, 16, 16, 31);
			int2 = __rlwimi( int2, load1, 8, 8, 23 );
			__stwbrx( int0, 0, &transfer.i[1]);
			d2 = transfer.d[2];
			d1 -= dBias;
			f0 = d0;
			(++dest)[0] = f3;
		}

		//Virtual cycle 8
		int2 = __rlwimi( int2, load2, 8, 24, 31 );
		int3 = load2 & 0x00FFFFFF;
		__stwbrx( int1, 0, &transfer.i[3]);
		d3 = transfer.d[3];
		d2 -= dBias;
		f1 = d1;
		(++dest)[0] = f0;

		//Virtual cycle 7
		__stwbrx( int2, 0, &transfer.i[5]);
		d0 = transfer.d[0];
		d3 -= dBias;
		f2 = d2;
		(++dest)[0] = f1;

		//Virtual cycle 6
		__stwbrx( int3, 0, &transfer.i[7]);
		d1 = transfer.d[1];
		d0 -= dBias;
		f3 = d3;
		(++dest)[0] = f2;

		//Virtual cycle 5
		d2 = transfer.d[2];
		d1 -= dBias;
		f0 = d0;
		(++dest)[0] = f3;

		//Virtual cycle 4
		d3 = transfer.d[3];
		d2 -= dBias;
		f1 = d1;
		(++dest)[0] = f0;

		//Virtual cycle 3
		d3 -= dBias;
		f2 = d2;
		(++dest)[0] = f1;

		//Virtual cycle 2
		f3 = d3;
		(++dest)[0] = f2;

		//Virtual cycle 1
		(++dest)[0] = f3;
	}

	if( count > 0 )
	{

		int1 = ((unsigned char*) src)[6];
		int0 = ((unsigned short*)(++src))[0];
		int1 ^= 0x80;
		int1 = __rlwimi( int1, int0, 8, 8, 23 );
		__stwbrx( int1, 0, &transfer.i[1]);
		d0 = transfer.d[0];
		d0 -= dBias;
		f0 = d0;
		(++dest)[0] = f0;

		src = (SInt32*) ((char*)src - 1 );
		while( --count )
		{
			int0 = (src = (SInt32*)( (char*) src + 3 ))[0];
			int0 ^= 0x80UL;
			int0 &= 0x00FFFFFFUL;
			__stwbrx( int0, 0, &transfer.i[1]);

			d0 = transfer.d[0];
			d0 -= dBias;
			f0 = d0;
			(++dest)[0] = f0;
		}
	}
}

void NativeInt32ToFloat32_Scalar( const SInt32 *src, Float32 *dest, unsigned int count )
{
	union
	{
		double		d[4];
		unsigned int	i[8];
	}transfer;
	register double dBias;
	register unsigned int loopCount;
	register long	int0, int1, int2, int3;
	register double		d0, d1, d2, d3;
	register Float32	f0, f1, f2, f3;

	transfer.i[0] = transfer.i[2] = transfer.i[4] = transfer.i[6] = (0x434UL - 32) << 20;
		//0x41400000UL;
	transfer.i[1] = 0x80000000;

	dBias = transfer.d[0];

	src--;
	dest--;

	if( count >= 8 )
	{
		count -= 8;
		loopCount = count / 4;
		count &= 3;

		//Virtual cycle 1
		int0 = (++src)[0];

		//Virtual cycle 2
		int1 = (++src)[0];
		int0 ^= 0x80000000UL;

		//Virtual cycle 3
		int2 = (++src)[0];
		int1 ^= 0x80000000UL;
		transfer.i[1] = int0;

		//Virtual cycle 4
		int3 = (++src)[0];
		int2 ^= 0x80000000UL;
		transfer.i[3] = int1;

		//Virtual cycle 5
		int0 = (++src)[0];
		int3 ^= 0x80000000UL;
		transfer.i[5] = int2;

		//Virtual cycle 6
		int1 = (++src)[0];
		int0 ^= 0x80000000UL;
		transfer.i[7] = int3;
		d0 = transfer.d[0];

		//Virtual cycle 7
		int2 = (++src)[0];
		int1 ^= 0x80000000UL;
		transfer.i[1] = int0;
		d1 = transfer.d[1];
		d0 -= dBias;

		//Virtual cycle 8
		int3 = (++src)[0];
		int2 ^= 0x80000000UL;
		transfer.i[3] = int1;
		d2 = transfer.d[2];
		d1 -= dBias;
		f0 = d0;

		while( loopCount-- )
		{
			//Virtual cycle A
			int0 = (++src)[0];
			int3 ^= 0x80000000UL;
			transfer.i[5] = int2;
			d3 = transfer.d[3];
			d2 -= dBias;
			f1 = d1;
			(++dest)[0] = f0;

			//Virtual cycle B
			int1 = (++src)[0];
			int0 ^= 0x80000000UL;
			transfer.i[7] = int3;
			d0 = transfer.d[0];
			d3 -= dBias;
			f2 = d2;
			(++dest)[0] = f1;

			//Virtual cycle C
			int2 = (++src)[0];
			int1 ^= 0x80000000UL;
			transfer.i[1] = int0;
			d1 = transfer.d[1];
			d0 -= dBias;
			f3 = d3;
			(++dest)[0] = f2;

			//Virtual cycle D
			int3 = (++src)[0];
			int2 ^= 0x80000000UL;
			transfer.i[3] = int1;
			d2 = transfer.d[2];
			d1 -= dBias;
			f0 = d0;
			(++dest)[0] = f3;
		}

		//Virtual cycle 8
		int3 ^= 0x80000000UL;
		transfer.i[5] = int2;
		d3 = transfer.d[3];
		d2 -= dBias;
		f1 = d1;
		(++dest)[0] = f0;

		//Virtual cycle 7
		transfer.i[7] = int3;
		d0 = transfer.d[0];
		d3 -= dBias;
		f2 = d2;
		(++dest)[0] = f1;

		//Virtual cycle 6
		d1 = transfer.d[1];
		d0 -= dBias;
		f3 = d3;
		(++dest)[0] = f2;

		//Virtual cycle 5
		d2 = transfer.d[2];
		d1 -= dBias;
		f0 = d0;
		(++dest)[0] = f3;

		//Virtual cycle 4
		d3 = transfer.d[3];
		d2 -= dBias;
		f1 = d1;
		(++dest)[0] = f0;

		//Virtual cycle 3
		d3 -= dBias;
		f2 = d2;
		(++dest)[0] = f1;

		//Virtual cycle 2
		f3 = d3;
		(++dest)[0] = f2;

		//Virtual cycle 1
		(++dest)[0] = f3;
	}


	while( count-- )
	{
		int0 = (++src)[0];
		int0 ^= 0x80000000UL;
		transfer.i[1] = int0;

		d0 = transfer.d[0];
		d0 -= dBias;
		f0 = d0;
		(++dest)[0] = f0;
	}
}

void SwapInt32ToFloat32_Scalar( const SInt32 *src, Float32 *dest, unsigned int count )
{
	union
	{
		double		d[4];
		unsigned int	i[8];
	}transfer;
	register double dBias;
	register unsigned int loopCount;
	register long	int0, int1, int2, int3;
	register double		d0, d1, d2, d3;
	register Float32	f0, f1, f2, f3;

	transfer.i[0] = transfer.i[2] = transfer.i[4] = transfer.i[6] = (0x434UL - 32) << 20;
		//0x41400000UL;
	transfer.i[1] = 0x80000000;

	dBias = transfer.d[0];

	dest--;

	if( count >= 8 )
	{
		count -= 8;
		loopCount = count / 4;
		count &= 3;

		//Virtual cycle 1
		int0 = __lwbrx(src, 0);

		//Virtual cycle 2
		int1 = __lwbrx(src, 4);
		int0 ^= 0x80000000UL;

		//Virtual cycle 3
		int2 = __lwbrx(src, 8);
		int1 ^= 0x80000000UL;
		transfer.i[1] = int0;

		//Virtual cycle 4
		int3 = __lwbrx(src, 12);
		int2 ^= 0x80000000UL;
		transfer.i[3] = int1;

		//Virtual cycle 5
		int0 = __lwbrx(src, 16);
		int3 ^= 0x80000000UL;
		transfer.i[5] = int2;

		//Virtual cycle 6
		int1 = __lwbrx(src, 20);
		int0 ^= 0x80000000UL;
		transfer.i[7] = int3;
		d0 = transfer.d[0];

		//Virtual cycle 7
		int2 = __lwbrx(src, 24);
		int1 ^= 0x80000000UL;
		transfer.i[1] = int0;
		d1 = transfer.d[1];
		d0 -= dBias;

		//Virtual cycle 8
		int3 = __lwbrx(src, 28);
		int2 ^= 0x80000000UL;
		transfer.i[3] = int1;
		d2 = transfer.d[2];
		d1 -= dBias;
		f0 = d0;

		src += 8;

		while( loopCount-- )
		{
			//Virtual cycle A
			int0 = __lwbrx(src, 0);
			int3 ^= 0x80000000UL;
			transfer.i[5] = int2;
			d3 = transfer.d[3];
			d2 -= dBias;
			f1 = d1;
			(++dest)[0] = f0;

			//Virtual cycle B
			int1 = __lwbrx(src, 4);
			int0 ^= 0x80000000UL;
			transfer.i[7] = int3;
			d0 = transfer.d[0];
			d3 -= dBias;
			f2 = d2;
			(++dest)[0] = f1;

			//Virtual cycle C
			int2 = __lwbrx(src, 8);
			int1 ^= 0x80000000UL;
			transfer.i[1] = int0;
			d1 = transfer.d[1];
			d0 -= dBias;
			f3 = d3;
			(++dest)[0] = f2;

			//Virtual cycle D
			int3 = __lwbrx(src, 12);
			int2 ^= 0x80000000UL;
			transfer.i[3] = int1;
			d2 = transfer.d[2];
			d1 -= dBias;
			f0 = d0;
			(++dest)[0] = f3;
			
			src += 4;
		}

		//Virtual cycle 8
		int3 ^= 0x80000000UL;
		transfer.i[5] = int2;
		d3 = transfer.d[3];
		d2 -= dBias;
		f1 = d1;
		(++dest)[0] = f0;

		//Virtual cycle 7
		transfer.i[7] = int3;
		d0 = transfer.d[0];
		d3 -= dBias;
		f2 = d2;
		(++dest)[0] = f1;

		//Virtual cycle 6
		d1 = transfer.d[1];
		d0 -= dBias;
		f3 = d3;
		(++dest)[0] = f2;

		//Virtual cycle 5
		d2 = transfer.d[2];
		d1 -= dBias;
		f0 = d0;
		(++dest)[0] = f3;

		//Virtual cycle 4
		d3 = transfer.d[3];
		d2 -= dBias;
		f1 = d1;
		(++dest)[0] = f0;

		//Virtual cycle 3
		d3 -= dBias;
		f2 = d2;
		(++dest)[0] = f1;

		//Virtual cycle 2
		f3 = d3;
		(++dest)[0] = f2;

		//Virtual cycle 1
		(++dest)[0] = f3;
	}


	while( count-- )
	{
		int0 = __lwbrx(src, 0);	++src;
		int0 ^= 0x80000000UL;
		transfer.i[1] = int0;

		d0 = transfer.d[0];
		d0 -= dBias;
		f0 = d0;
		(++dest)[0] = f0;
	}
}

void Float32ToNativeInt16_Scalar( const Float32 *src, SInt16 *dst, unsigned int count )
{
	register double		scale = 2147483648.0;
	register double		round = 32768.0;
	SInt32				buffer0, buffer1, buffer2;
	register Float32	loadFloat0, loadFloat1, loadFloat2;
	register double 	double0, double1, double2;
	register SInt16	short0, short1, short2;
	unsigned int		loopCount;

	/*
	When there are 4 or more remaining to store:
		$loadFloat = *++src;
		$double = $loadFloat * scale + round;
		$double = __fctiw($double);
		__stfiwx($double, 0, dst); ++dst;
		??? or would the overlapping stores be a performance hit ???
	
	When there are 3 or fewer remaining to store:
		$loadFloat = *++src;
		$double = $loadFloat * scale + round;
		$double = __fctiw($double);
		__stfiwx($double, 0, &$buffer);
		$short = *(short *)&$buffer;
		*++dst = $short;
		
		This inner loop unrolls/interleaves for 3 samples. If we do it by 4, we get extra register copies (to avoid using
		more FP registers?) and stalls.
	*/
	--src;
	--dst;
	
	if (count >= 6) {
		// Cycle 1
		loadFloat0 = *++src;

		// Cycle 2
		loadFloat1 = *++src;
		double0 = loadFloat0 * scale + round;

		// Cycle 3
		loadFloat2 = *++src;
		double1 = loadFloat1 * scale + round;
		double0 = __fctiw(double0);

		// Cycle 4
		loadFloat0 = *++src;
		double2 = loadFloat2 * scale + round;
		double1 = __fctiw(double1);
		__stfiwx(double0, 0, &buffer0);

		// Cycle 5
		loadFloat1 = *++src;
		double0 = loadFloat0 * scale + round;
		double2 = __fctiw(double2);
		__stfiwx(double1, 0, &buffer1);
		short0 = *(short *)&buffer0;

		// Cycle 6
		loadFloat2 = *++src;
		double1 = loadFloat1 * scale + round;
		double0 = __fctiw(double0);
		__stfiwx(double2, 0, &buffer2);
		short1 = *(short *)&buffer1;
		*++dst = short0;
		
		count -= 6;
		loopCount = count / 3;	// gcc has a cool trick for efficient divides by 3 (and its multiples)
		count -= 3 * loopCount;

		while (loopCount--) {
			// Cycle A
			loadFloat0 = *++src;
			double2 = loadFloat2 * scale + round;
			double1 = __fctiw(double1);
			__stfiwx(double0, 0, &buffer0);
			short2 = *(short *)&buffer2;
			*++dst = short1;

			// Cycle B
			loadFloat1 = *++src;
			double0 = loadFloat0 * scale + round;
			double2 = __fctiw(double2);
			__stfiwx(double1, 0, &buffer1);
			short0 = *(short *)&buffer0;
			*++dst = short2;

			// Cycle C
			loadFloat2 = *++src;
			double1 = loadFloat1 * scale + round;
			double0 = __fctiw(double0);
			__stfiwx(double2, 0, &buffer2);
			short1 = *(short *)&buffer1;
			*++dst = short0;
		}

		// Cycle 5
		double2 = loadFloat2 * scale + round;
		double1 = __fctiw(double1);
		__stfiwx(double0, 0, &buffer0);
		short2 = *(short *)&buffer2;
		*++dst = short1;

		// Cycle 4
		double2 = __fctiw(double2);
		__stfiwx(double1, 0, &buffer1);
		short0 = *(short *)&buffer0;
		*++dst = short2;

		// Cycle 3
		__stfiwx(double2, 0, &buffer2);
		short1 = *(short *)&buffer1;
		*++dst = short0;

		// Cycle 2
		short2 = *(short *)&buffer2;
		*++dst = short1;

		// Cycle 1
		*++dst = short2;
	}

	while (count--) {
		loadFloat0 = *++src;
		double0 = loadFloat0 * scale + round;
		double0 = __fctiw(double0);
		__stfiwx(double0, 0, &buffer0);
		short0 = *(short *)&buffer0;
		*++dst = short0;
	}
}

void Float32ToSwapInt16_Scalar( const Float32 *src, SInt16 *dst, unsigned int count )
{
	register double		scale = 2147483648.0;
	register double		round = 32768.0;
	SInt32				buffer0, buffer1, buffer2;
	register Float32	loadFloat0, loadFloat1, loadFloat2;
	register double 	double0, double1, double2;
	register UInt32		short0, short1, short2;
	unsigned int		loopCount;

	/*
		$loadFloat = *++src;
		$double = $loadFloat * scale + round;
		$double = __fctiw($double);
		__stfiwx($double, 0, &$buffer);
		$short = __lhbrx(&$buffer, 0);
		*++dst = $short;
		
		This inner loop unrolls/interleaves for 3 samples. If we do it by 4, we get extra register copies (to avoid using
		more FP registers?) and stalls.
	*/
	--src;
	
	if (count >= 6) {
		// Cycle 1
		loadFloat0 = *++src;

		// Cycle 2
		loadFloat1 = *++src;
		double0 = loadFloat0 * scale + round;

		// Cycle 3
		loadFloat2 = *++src;
		double1 = loadFloat1 * scale + round;
		double0 = __fctiw(double0);

		// Cycle 4
		loadFloat0 = *++src;
		double2 = loadFloat2 * scale + round;
		double1 = __fctiw(double1);
		__stfiwx(double0, 0, &buffer0);

		// Cycle 5
		loadFloat1 = *++src;
		double0 = loadFloat0 * scale + round;
		double2 = __fctiw(double2);
		__stfiwx(double1, 0, &buffer1);
		short0 = *(short *)&buffer0;

		// Cycle 6
		loadFloat2 = *++src;
		double1 = loadFloat1 * scale + round;
		double0 = __fctiw(double0);
		__stfiwx(double2, 0, &buffer2);
		short1 = *(short *)&buffer1;
		__sthbrx(short0, dst, 0);	dst++;
		count -= 6;
		loopCount = count / 3;	// gcc has a cool trick for efficient divides by 3 (and its multiples)
		count -= 3 * loopCount;

		while (loopCount--) {
			// Cycle A
			loadFloat0 = *++src;
			double2 = loadFloat2 * scale + round;
			double1 = __fctiw(double1);
			__stfiwx(double0, 0, &buffer0);
			short2 = *(short *)&buffer2;
			__sthbrx(short1, dst, 0);	dst++;

			// Cycle B
			loadFloat1 = *++src;
			double0 = loadFloat0 * scale + round;
			double2 = __fctiw(double2);
			__stfiwx(double1, 0, &buffer1);
			short0 = *(short *)&buffer0;
			__sthbrx(short2, dst, 0);	dst++;

			// Cycle C
			loadFloat2 = *++src;
			double1 = loadFloat1 * scale + round;
			double0 = __fctiw(double0);
			__stfiwx(double2, 0, &buffer2);
			short1 = *(short *)&buffer1;
			__sthbrx(short0, dst, 0);	dst++;
		}

		// Cycle 5
		double2 = loadFloat2 * scale + round;
		double1 = __fctiw(double1);
		__stfiwx(double0, 0, &buffer0);
		short2 = *(short *)&buffer2;
		__sthbrx(short1, dst, 0);	dst++;

		// Cycle 4
		double2 = __fctiw(double2);
		__stfiwx(double1, 0, &buffer1);
		short0 = *(short *)&buffer0;
		__sthbrx(short2, dst, 0);	dst++;

		// Cycle 3
		__stfiwx(double2, 0, &buffer2);
		short1 = *(short *)&buffer1;
		__sthbrx(short0, dst, 0);	dst++;

		// Cycle 2
		short2 = *(short *)&buffer2;
		__sthbrx(short1, dst, 0);	dst++;

		// Cycle 1
		__sthbrx(short2, dst, 0);	dst++;
	}

	while (count--) {
		loadFloat0 = *++src;
		double0 = loadFloat0 * scale + round;
		double0 = __fctiw(double0);
		__stfiwx(double0, 0, &buffer0);
		short0 = *(short *)&buffer0;
		__sthbrx(short0, dst, 0);	dst++;
	}
}

void Float32ToNativeInt24_Scalar( const Float32 *src, SInt32 *dst, unsigned int count )
{
	register double		scale = 2147483648.0;
	register double		round = 0.5 * 256.0;
	unsigned long	loopCount = count / 4;
	long		buffer[4];
	register Float32	startingFloat, startingFloat2;
	register double scaled, scaled2;
	register double converted, converted2;
	register long	copy1;//, merge1, rotate1;
	register long	copy2;//, merge2, rotate2;
	register long	copy3;//, merge3, rotate3;
	register long	copy4;//, merge4, rotate4;
	register double		oldSetting;


	//Set the FPSCR to round to -Inf mode
	{
		union
		{
			double	d;
			int		i[2];
		}setting;
		register double newSetting;

		//Read the the current FPSCR value
		asm volatile ( "mffs %0" : "=f" ( oldSetting ) );

		//Store it to the stack
		setting.d = oldSetting;

		//Read in the low 32 bits and mask off the last two bits so they are zero
		//in the integer unit. These two bits set to zero means round to nearest mode.
		//Finally, then store the result back
		setting.i[1] |= 3;

		//Load the new FPSCR setting into the FP register file again
		newSetting = setting.d;

		//Change the FPSCR to the new setting
		asm volatile( "mtfsf 7, %0" : : "f" (newSetting ) );
	}


	//
	//	The fastest way to do this is to set up a staggered loop that models a 7 stage virtual pipeline:
	//
	//		stage 1:		load the src value
	//		stage 2:		scale it to LONG_MIN...LONG_MAX and add a rounding value to it
	//		stage 3:		convert it to an integer within the FP register
	//		stage 4:		store that to the stack
	//		stage 5:		 (do nothing to let the store complete)
	//		stage 6:		load the high half word from the stack
		//		stage 7:		merge with later data to form a 32 bit word
		//		stage 8:		possible rotate to correct byte order
	//		stage 9:		store it to the destination
	//
	//	We set it up so that at any given time 7 different pieces of data are being worked on at a time.
	//	Because of the do nothing stage, the inner loop had to be unrolled by one, so in actuality, each
	//	inner loop iteration represents two virtual clock cycles that push data through our virtual pipeline.
	//
	//	The reason why this works is that this allows us to break data dependency chains and insert 5 real
	//	operations in between every virtual pipeline stage. This means 5 instructions between each data
	//	dependency, which is just enough to keep all of our real pipelines happy. The data flow follows
	//	standard pipeline diagrams:
	//
	//				stage1	stage2	stage3	stage4	stage5	stage6	stage7	stage8	stage9
	//	virtual cycle 1:	data1	-	-	-		-		-		-		-		-
	//	virtual cycle 2:	data2	data1	-	-		-		-		-		-		-
	//	virtual cycle 3:	data3	data2	data1	-		-		-		-		-		-
	//	virtual cycle 4:	data4	data3	data2	data1	-		-		-		-		-
	//	virtual cycle 5:	data5	data4	data3	data2	data1	-		-		-		-
	//	virtual cycle 6:	data6	data5	data4	data3	data2	data1	-		-		-
	//	virtual cycle 7:	data7	data6	data5	data4	data3	data2	data1	-		-
	//	virtual cycle 8:	data8	data7	data6	data5	data4	data3	data2	data1	-
	//
	//	inner loop:
	//	  virtual cycle A:	data9	data8	data7	data6	data5	data4	data3	data2	data1
	//	  virtual cycle B:	data10	data9	data8	data7	data6	data5	data4	data3	data2
	//	  virtual cycle C:	data11	data10	data9	data8	data7	data6	data5	data4	data3
	//	  virtual cycle D:	data12	data11	data10	data9	data8	data7	data6	data5	data4
	//
	//	virtual cycle 9		-	dataH	dataG	dataF	dataE	dataD	dataC	dataB	dataA
	//	virtual cycle 10	-	-	dataH	dataG	dataF	dataE	dataD	dataC	dataB
	//	virtual cycle 11	-	-	-	dataH	dataG	dataF	dataE	dataD	dataC
	//	virtual cycle 12	-	-	-	-		dataH	dataG	dataF	dataE	dataD
	//	virtual cycle 13	-	-	-	-		-		dataH	dataG	dataF	dataE
	//	virtual cycle 14	-	-	-	-		-		-		dataH	dataG	dataF
	//	virtual cycle 15	-	-	-	-		-		-		-	dataH	dataG	
	//	virtual cycle 16	-	-	-	-		-		-		-	-	dataH

	src--;
	dst--;

	if( count >= 8 )
	{
		//virtual cycle 1
		startingFloat = (++src)[0];

		//virtual cycle 2
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 3
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 4
		__stfiwx( converted, 0, buffer );
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 5
		__stfiwx( converted, sizeof(Float32), buffer );
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 6
		copy1 = buffer[0];
		__stfiwx( converted, 2 * sizeof( Float32), buffer );
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 7
		copy2 = buffer[1];
		__stfiwx( converted, 3 * sizeof(Float32), buffer );
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 8
		copy1 = __rlwimi( copy1, copy2, 8, 24, 31 );
		copy3 = buffer[2];
		__stfiwx( converted, 0, buffer );
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		count -= 8;
		loopCount = count / 4;
		count &= 3;
		while( loopCount-- )
		{
			//virtual cycle A
			//no store yet						//store
			//no rotation needed for copy1,				//rotate
			__asm__ __volatile__( "fmadds %0, %1, %2, %3" : "=f"(scaled2) : "f" (startingFloat), "f" ( scale ), "f" ( round ));		//scale for clip and add rounding
			startingFloat2 = (++src)[0];				//load the float
			__asm__ __volatile__( "fctiw %0, %1" : "=f" (converted2) : "f" ( scaled ) );				//convert to int and clip
			 copy4 = buffer[3];						//load clipped int back in
			copy2 = __rlwimi_volatile( copy2, copy3, 8, 24, 7 );			//merge
			__stfiwx( converted, 1 * sizeof(Float32), buffer );		//store clipped int

			//virtual Cycle B
			__asm__ __volatile__( "fmadds %0, %1, %2, %3" : "=f"(scaled) : "f" (startingFloat2), "f" ( scale ), "f" ( round ));		//scale for clip and add rounding
			startingFloat = (++src)[0];					//load the float
			 __asm__ __volatile__( "fctiw %0, %1" : "=f" (converted) : "f" ( scaled2 ) );				//convert to int and clip
			(++dst)[0] = copy1;						//store
			copy3 = __rlwimi_volatile( copy3, copy4, 8, 24, 15 );	//merge with adjacent pixel
			copy1 = buffer[0];						//load clipped int back in
			copy2 = __rlwimi_volatile( copy2, copy2, 8, 0, 31 );	//rotate
			__stfiwx( converted2, 2 * sizeof(Float32), buffer );		//store clipped int

			//virtual Cycle C
			__asm__ __volatile__( "fmadds %0, %1, %2, %3" : "=f"(scaled2) : "f" (startingFloat), "f" ( scale ), "f" ( round ));		//scale for clip and add rounding
			startingFloat2 = (++src)[0];				//load the float
			//We dont store copy 4 so no merge needs to be done to it	//merge with adjacent pixel
			converted2 = __fctiw( scaled );				//convert to int and clip
			(++dst)[0] = copy2;						//store
			copy3 = __rlwimi_volatile( copy3, copy3, 16, 0, 31 );		//rotate
			copy2 = buffer[1];						//load clipped int back in
			__stfiwx( converted, 3 * sizeof(Float32), buffer );		//store clipped int

			//virtual Cycle D
			__asm__ ( "fmadds %0, %1, %2, %3" : "=f"(scaled) : "f" (startingFloat2), "f" ( scale ), "f" ( round ));		//scale for clip and add rounding
			startingFloat = (++src)[0];					//load the float
			converted = __fctiw( scaled2 );				//convert to int and clip
			//We dont store copy 4 so no rotation needs to be done to it//rotate
			(++dst)[0] = copy3;						//store
			copy1 = __rlwimi_volatile( copy1, copy2, 8, 24, 31 );		//merge with adjacent pixel
			 __stfiwx( converted2, 0 * sizeof(Float32), buffer );		//store clipped int
			copy3 = buffer[2];						//load clipped int back in
		}

		//virtual cycle 9
		//no store yet						//store
		//no rotation needed for copy1,				//rotate
		copy2 = __rlwimi( copy2, copy3, 8, 24, 7 );		//merge
		copy4 = buffer[3];					//load clipped int back in
		__stfiwx( converted, 1 * sizeof(Float32), buffer );	//store clipped int
		converted2 = __fctiw( scaled );				//convert to int and clip
		scaled2 = startingFloat * scale + round;		//scale for clip and add rounding

		//virtual Cycle 10
		(++dst)[0] = copy1;						//store
		copy2 = __rlwimi( copy2, copy2, 8, 0, 31 );			//rotate
		copy3 = __rlwimi( copy3, copy4, 8, 24, 15 );		//merge with adjacent pixel
		copy1 = buffer[0];					//load clipped int back in
		__stfiwx( converted2, 2 * sizeof(Float32), buffer );	//store clipped int
		converted = __fctiw( scaled2 );				//convert to int and clip

		//virtual Cycle 11
		(++dst)[0] = copy2;						//store
		copy3 = __rlwimi( copy3, copy3, 16, 0, 31 );		//rotate
		//We dont store copy 4 so no merge needs to be done to it//merge with adjacent pixel
		copy2 = buffer[1];					//load clipped int back in
		__stfiwx( converted, 3 * sizeof(Float32), buffer );	//store clipped int

		//virtual Cycle 12
		(++dst)[0] = copy3;						//store
		//We dont store copy 4 so no rotation needs to be done to it//rotate
		copy1 = __rlwimi( copy1, copy2, 8, 24, 31 );		//merge with adjacent pixel
		copy3 = buffer[2];						//load clipped int back in

		//virtual cycle 13
		//no store yet						//store
		//no rotation needed for copy1,				//rotate
		copy2 = __rlwimi( copy2, copy3, 8, 24, 7 );		//merge
		copy4 = buffer[3];					//load clipped int back in

		//virtual Cycle 14
		(++dst)[0] = copy1;						//store
		copy2 = __rlwimi( copy2, copy2, 8, 0, 31 );			//rotate
		copy3 = __rlwimi( copy3, copy4, 8, 24, 15 );		//merge with adjacent pixel

		//virtual Cycle 15
		(++dst)[0] = copy2;						//store
		copy3 = __rlwimi( copy3, copy3, 16, 0, 31 );		//rotate

		//virtual Cycle 16
		(++dst)[0] = copy3;						//store
	}

	//clean up any extras
	dst++;
	while( count-- )
	{
		startingFloat = (++src)[0];				//load the float
		scaled = startingFloat * scale + round;			//scale for clip and add rounding
		converted = __fctiw( scaled );				//convert to int and clip
		__stfiwx( converted, 0, buffer );			//store clipped int
		copy1 = buffer[0];					//load clipped int back in
		((signed char*) dst)[0] = copy1 >> 24;
		dst = (SInt32*) ((signed char*) dst + 1 );
		((unsigned short*) dst)[0] = copy1 >> 8;
		dst = (SInt32*) ((unsigned short*) dst + 1 );
	}

	//restore the old FPSCR setting
	__asm__ __volatile__ ( "mtfsf 7, %0" : : "f" (oldSetting) );
}

void Float32ToSwapInt24_Scalar( const Float32 *src, SInt32 *dst, unsigned int count )
{
	register double		scale = 2147483648.0;
	register double		round = 0.5 * 256.0;
	unsigned long	loopCount = count / 4;
	long		buffer[4];
	register Float32	startingFloat, startingFloat2;
	register double scaled, scaled2;
	register double converted, converted2;
	register long	copy1;
	register long	copy2;
	register long	copy3;
	register long	copy4;
	register double		oldSetting;


	//Set the FPSCR to round to -Inf mode
	{
		union
		{
			double	d;
			int		i[2];
		}setting;
		register double newSetting;

		//Read the the current FPSCR value
		asm volatile ( "mffs %0" : "=f" ( oldSetting ) );

		//Store it to the stack
		setting.d = oldSetting;

		//Read in the low 32 bits and mask off the last two bits so they are zero
		//in the integer unit. These two bits set to zero means round to nearest mode.
		//Finally, then store the result back
		setting.i[1] |= 3;

		//Load the new FPSCR setting into the FP register file again
		newSetting = setting.d;

		//Change the FPSCR to the new setting
		asm volatile( "mtfsf 7, %0" : : "f" (newSetting ) );
	}


	//
	//	The fastest way to do this is to set up a staggered loop that models a 7 stage virtual pipeline:
	//
	//		stage 1:		load the src value
	//		stage 2:		scale it to LONG_MIN...LONG_MAX and add a rounding value to it
	//		stage 3:		convert it to an integer within the FP register
	//		stage 4:		store that to the stack
	//		stage 5:		 (do nothing to let the store complete)
	//		stage 6:		load the high half word from the stack
		//		stage 7:		merge with later data to form a 32 bit word
		//		stage 8:		possible rotate to correct byte order
	//		stage 9:		store it to the destination
	//
	//	We set it up so that at any given time 7 different pieces of data are being worked on at a time.
	//	Because of the do nothing stage, the inner loop had to be unrolled by one, so in actuality, each
	//	inner loop iteration represents two virtual clock cycles that push data through our virtual pipeline.
	//
	//	The reason why this works is that this allows us to break data dependency chains and insert 5 real
	//	operations in between every virtual pipeline stage. This means 5 instructions between each data
	//	dependency, which is just enough to keep all of our real pipelines happy. The data flow follows
	//	standard pipeline diagrams:
	//
	//				stage1	stage2	stage3	stage4	stage5	stage6	stage7	stage8	stage9
	//	virtual cycle 1:	data1	-	-	-		-		-		-		-		-
	//	virtual cycle 2:	data2	data1	-	-		-		-		-		-		-
	//	virtual cycle 3:	data3	data2	data1	-		-		-		-		-		-
	//	virtual cycle 4:	data4	data3	data2	data1	-		-		-		-		-
	//	virtual cycle 5:	data5	data4	data3	data2	data1	-		-		-		-
	//	virtual cycle 6:	data6	data5	data4	data3	data2	data1	-		-		-
	//	virtual cycle 7:	data7	data6	data5	data4	data3	data2	data1	-		-
	//	virtual cycle 8:	data8	data7	data6	data5	data4	data3	data2	data1	-
	//
	//	inner loop:
	//	  virtual cycle A:	data9	data8	data7	data6	data5	data4	data3	data2	data1
	//	  virtual cycle B:	data10	data9	data8	data7	data6	data5	data4	data3	data2
	//	  virtual cycle C:	data11	data10	data9	data8	data7	data6	data5	data4	data3
	//	  virtual cycle D:	data12	data11	data10	data9	data8	data7	data6	data5	data4
	//
	//	virtual cycle 9		-	dataH	dataG	dataF	dataE	dataD	dataC	dataB	dataA
	//	virtual cycle 10	-	-	dataH	dataG	dataF	dataE	dataD	dataC	dataB
	//	virtual cycle 11	-	-	-	dataH	dataG	dataF	dataE	dataD	dataC
	//	virtual cycle 12	-	-	-	-		dataH	dataG	dataF	dataE	dataD
	//	virtual cycle 13	-	-	-	-		-		dataH	dataG	dataF	dataE
	//	virtual cycle 14	-	-	-	-		-		-		dataH	dataG	dataF
	//	virtual cycle 15	-	-	-	-		-		-		-	dataH	dataG	
	//	virtual cycle 16	-	-	-	-		-		-		-	-	dataH

	src--;
	dst--;

	if( count >= 8 )
	{
		//virtual cycle 1
		startingFloat = (++src)[0];

		//virtual cycle 2
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 3
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 4
		__stfiwx( converted, 0, buffer );
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 5
		__stfiwx( converted, sizeof(Float32), buffer );
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 6
		copy1 = __lwbrx( 0, buffer );
		__stfiwx( converted, 2 * sizeof( Float32), buffer );
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 7
		copy2 = __lwbrx( 4, buffer );
		__stfiwx( converted, 3 * sizeof(Float32), buffer );
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		//virtual cycle 8
		copy1 = __rlwimi( copy1, copy2, 8, 0, 7 );
		copy3 = __lwbrx( 8, buffer );;
		__stfiwx( converted, 0, buffer );
		converted = __fctiw( scaled );
		scaled = startingFloat * scale + round;
		startingFloat = (++src)[0];

		count -= 8;
		loopCount = count / 4;
		count &= 3;
		while( loopCount-- )
		{
			//virtual cycle A
			//no store yet						//store
			copy1 = __rlwimi( copy1, copy1, 8, 0, 31 );			//rotate
			__asm__ __volatile__( "fmadds %0, %1, %2, %3" : "=f"(scaled2) : "f" (startingFloat), "f" ( scale ), "f" ( round ));		//scale for clip and add rounding
			startingFloat2 = (++src)[0];				//load the float
			__asm__ __volatile__( "fctiw %0, %1" : "=f" (converted2) : "f" ( scaled ) );				//convert to int and clip
			 copy4 = __lwbrx( 12, buffer );						//load clipped int back in
			copy2 = __rlwimi_volatile( copy2, copy3, 8, 0, 15 );			//merge
			__stfiwx( converted, 1 * sizeof(Float32), buffer );		//store clipped int

			//virtual Cycle B
			__asm__ __volatile__( "fmadds %0, %1, %2, %3" : "=f"(scaled) : "f" (startingFloat2), "f" ( scale ), "f" ( round ));		//scale for clip and add rounding
			startingFloat = (++src)[0];					//load the float
			 __asm__ __volatile__( "fctiw %0, %1" : "=f" (converted) : "f" ( scaled2 ) );				//convert to int and clip
			(++dst)[0] = copy1;						//store
			copy4 = __rlwimi_volatile( copy4, copy3, 24, 0, 7 );	//merge with adjacent pixel
			copy1 = __lwbrx( 0, buffer );						//load clipped int back in
			copy2 = __rlwimi_volatile( copy2, copy2, 16, 0, 31 );	//rotate
			__stfiwx( converted2, 2 * sizeof(Float32), buffer );		//store clipped int

			//virtual Cycle C
			__asm__ __volatile__( "fmadds %0, %1, %2, %3" : "=f"(scaled2) : "f" (startingFloat), "f" ( scale ), "f" ( round ));		//scale for clip and add rounding
			startingFloat2 = (++src)[0];				//load the float
			converted2 = __fctiw( scaled );				//convert to int and clip
			(++dst)[0] = copy2;						//store
			copy2 = __lwbrx( 4, buffer );						//load clipped int back in
			__stfiwx( converted, 3 * sizeof(Float32), buffer );		//store clipped int


			//virtual Cycle D
			__asm__ ( "fmadds %0, %1, %2, %3" : "=f"(scaled) : "f" (startingFloat2), "f" ( scale ), "f" ( round ));		//scale for clip and add rounding
			startingFloat = (++src)[0];					//load the float
			converted = __fctiw( scaled2 );				//convert to int and clip
			(++dst)[0] = copy4;						//store
			copy1 = __rlwimi_volatile( copy1, copy2, 8, 0, 7 );		//merge with adjacent pixel
			 __stfiwx( converted2, 0 * sizeof(Float32), buffer );		//store clipped int
			copy3 = __lwbrx( 8, buffer );						//load clipped int back in
		}

		//virtual cycle A
		//no store yet						//store
		copy1 = __rlwimi( copy1, copy1, 8, 0, 31 );			//rotate
		__asm__ __volatile__( "fmadds %0, %1, %2, %3" : "=f"(scaled2) : "f" (startingFloat), "f" ( scale ), "f" ( round ));		//scale for clip and add rounding
		__asm__ __volatile__( "fctiw %0, %1" : "=f" (converted2) : "f" ( scaled ) );				//convert to int and clip
			copy4 = __lwbrx( 12, buffer );						//load clipped int back in
		copy2 = __rlwimi_volatile( copy2, copy3, 8, 0, 15 );			//merge
		__stfiwx( converted, 1 * sizeof(Float32), buffer );		//store clipped int

		//virtual Cycle B
			__asm__ __volatile__( "fctiw %0, %1" : "=f" (converted) : "f" ( scaled2 ) );				//convert to int and clip
		(++dst)[0] = copy1;						//store
		copy4 = __rlwimi_volatile( copy4, copy3, 24, 0, 7 );	//merge with adjacent pixel
		copy1 = __lwbrx( 0, buffer );						//load clipped int back in
		copy2 = __rlwimi_volatile( copy2, copy2, 16, 0, 31 );	//rotate
		__stfiwx( converted2, 2 * sizeof(Float32), buffer );		//store clipped int

		//virtual Cycle C
		(++dst)[0] = copy2;						//store
		copy2 = __lwbrx( 4, buffer );						//load clipped int back in
		__stfiwx( converted, 3 * sizeof(Float32), buffer );		//store clipped int


		//virtual Cycle D
		(++dst)[0] = copy4;						//store
		copy1 = __rlwimi_volatile( copy1, copy2, 8, 0, 7 );		//merge with adjacent pixel
		copy3 = __lwbrx( 8, buffer );						//load clipped int back in

		//virtual cycle A
		//no store yet						//store
		copy1 = __rlwimi( copy1, copy1, 8, 0, 31 );			//rotate
		copy4 = __lwbrx( 12, buffer );						//load clipped int back in
		copy2 = __rlwimi_volatile( copy2, copy3, 8, 0, 15 );			//merge

		//virtual Cycle B
		(++dst)[0] = copy1;						//store
		copy4 = __rlwimi_volatile( copy4, copy3, 24, 0, 7 );	//merge with adjacent pixel
		copy2 = __rlwimi_volatile( copy2, copy2, 16, 0, 31 );	//rotate

		//virtual Cycle C
		(++dst)[0] = copy2;						//store


		//virtual Cycle D
		(++dst)[0] = copy4;						//store
	}

	//clean up any extras
	dst++;
	while( count-- )
	{
		startingFloat = (++src)[0];				//load the float
		scaled = startingFloat * scale + round;			//scale for clip and add rounding
		converted = __fctiw( scaled );				//convert to int and clip
		__stfiwx( converted, 0, buffer );			//store clipped int
		copy1 = __lwbrx( 0, buffer);					//load clipped int back in
		((signed char*) dst)[0] = copy1 >> 16;
		dst = (SInt32*) ((signed char*) dst + 1 );
		((unsigned short*) dst)[0] = copy1;
		dst = (SInt32*) ((unsigned short*) dst + 1 );
	}

	//restore the old FPSCR setting
	__asm__ __volatile__ ( "mtfsf 7, %0" : : "f" (oldSetting) );
}

void Float32ToNativeInt32_Scalar( const Float32 *src, SInt32 *dst, unsigned int count )
{
	register double		scale = 2147483648.0;
	unsigned long	loopCount;
	register Float32		loadFloat0, loadFloat1, loadFloat2;
	register double		scaled0, scaled1, scaled2;
	register double		converted0, converted1, converted2;
	register double		oldSetting;

	//Set the FPSCR to round to -Inf mode
	{
		union
		{
			double	d;
			int		i[2];
		}setting;
		register double newSetting;
		
		//Read the the current FPSCR value
		asm volatile ( "mffs %0" : "=f" ( oldSetting ) );
		
		//Store it to the stack
		setting.d = oldSetting;
		
		//Read in the low 32 bits and mask off the last two bits so they are zero 
		//in the integer unit. These two bits set to zero means round to -infinity mode.
		//Finally, then store the result back
		setting.i[1] &= 0xFFFFFFFC;
		
		//Load the new FPSCR setting into the FP register file again
		newSetting = setting.d;
		
		//Change the FPSCR to the new setting
		asm volatile( "mtfsf 7, %0" : : "f" (newSetting ) );
	}

	/*
		$loadFloat = *++src;
		$scaled = $loadFloat * scale;
		$converted = __fctiw($scaled);
		__stfiwx($converted, 0, dst);	++dst;
	*/
	
	--src;
	
	if (count >= 6) {
		// Cycle 1
		loadFloat0 = *++src;

		// Cycle 2
		loadFloat1 = *++src;
		scaled0 = loadFloat0 * scale;

		// Cycle 3
		loadFloat2 = *++src;
		scaled1 = loadFloat1 * scale;
		converted0 = __fctiw(scaled0);

		// Cycle 4
		loadFloat0 = *++src;
		scaled2 = loadFloat2 * scale;
		converted1 = __fctiw(scaled1);
		__stfiwx(converted0, 0, dst);	++dst;

		// Cycle 5
		loadFloat1 = *++src;
		scaled0 = loadFloat0 * scale;
		converted2 = __fctiw(scaled2);
		__stfiwx(converted1, 0, dst);	++dst;

		// Cycle 6
		loadFloat2 = *++src;
		scaled1 = loadFloat1 * scale;
		converted0 = __fctiw(scaled0);
		__stfiwx(converted2, 0, dst);	++dst;
		count -= 6;
		loopCount = count / 3;	// gcc has a cool trick for efficient divides by 3 (and its multiples)
		count -= 3 * loopCount;

		while (loopCount--) {
			// Cycle A
			loadFloat0 = *++src;
			scaled2 = loadFloat2 * scale;
			converted1 = __fctiw(scaled1);
			__stfiwx(converted0, 0, dst);	++dst;

			// Cycle B
			loadFloat1 = *++src;
			scaled0 = loadFloat0 * scale;
			converted2 = __fctiw(scaled2);
			__stfiwx(converted1, 0, dst);	++dst;

			// Cycle C
			loadFloat2 = *++src;
			scaled1 = loadFloat1 * scale;
			converted0 = __fctiw(scaled0);
			__stfiwx(converted2, 0, dst);	++dst;
		}

		// Cycle 3
		scaled2 = loadFloat2 * scale;
		converted1 = __fctiw(scaled1);
		__stfiwx(converted0, 0, dst);	++dst;

		// Cycle 2
		converted2 = __fctiw(scaled2);
		__stfiwx(converted1, 0, dst);	++dst;

		// Cycle 1
		__stfiwx(converted2, 0, dst);	++dst;
	}

	while (count--) {
		loadFloat0 = *++src;
		scaled0 = loadFloat0 * scale;
		converted0 = __fctiw(scaled0);
		__stfiwx(converted0, 0, dst);	++dst;
	}

	//restore the old FPSCR setting
	asm volatile( "mtfsf 7, %0" : : "f" (oldSetting) );
}

void Float32ToSwapInt32_Scalar( const Float32 *src, SInt32 *dst, unsigned int count )
{
	register double		scale = 2147483648.0;
	register unsigned int loopCount;
	register Float32	loadFloat0, loadFloat1, loadFloat2;
	register double		scaled0, scaled1, scaled2, converted0, converted1, converted2;
	register SInt32		int0, int1, int2;
	register double		oldSetting;
	SInt32				buffer0, buffer1, buffer2;

	//Set the FPSCR to round to -Inf mode
	{
		union
		{
			double	d;
			int		i[2];
		}setting;
		register double newSetting;
		
		//Read the the current FPSCR value
		asm volatile ( "mffs %0" : "=f" ( oldSetting ) );
		
		//Store it to the stack
		setting.d = oldSetting;
		
		//Read in the low 32 bits and mask off the last two bits so they are zero 
		//in the integer unit. These two bits set to zero means round to nearest mode.
		//Finally, then store the result back
		setting.i[1] &= 0xFFFFFFFC;
		
		//Load the new FPSCR setting into the FP register file again
		newSetting = setting.d;
		
		//Change the FPSCR to the new setting
		asm volatile( "mtfsf 7, %0" : : "f" (newSetting ) );
	}
	/*
		$loadFloat = *++src;
		$scaled = $loadFloat * scale;
		$converted = __fctiw($scaled);
		__stfiwx($converted, 0, &$buffer);
		$int = __lwbrx(&$buffer, 0);
		*++dst = $int;
	*/
	--src;
	--dst;

	if (count >= 6) {
		// Cycle 1
		loadFloat0 = *++src;

		// Cycle 2
		loadFloat1 = *++src;
		scaled0 = loadFloat0 * scale;

		// Cycle 3
		loadFloat2 = *++src;
		scaled1 = loadFloat1 * scale;
		converted0 = __fctiw(scaled0);

		// Cycle 4
		loadFloat0 = *++src;
		scaled2 = loadFloat2 * scale;
		converted1 = __fctiw(scaled1);
		__stfiwx(converted0, 0, &buffer0);

		// Cycle 5
		loadFloat1 = *++src;
		scaled0 = loadFloat0 * scale;
		converted2 = __fctiw(scaled2);
		__stfiwx(converted1, 0, &buffer1);
		int0 = __lwbrx(&buffer0, 0);

		// Cycle 6
		loadFloat2 = *++src;
		scaled1 = loadFloat1 * scale;
		converted0 = __fctiw(scaled0);
		__stfiwx(converted2, 0, &buffer2);
		int1 = __lwbrx(&buffer1, 0);
		*++dst = int0;

		count -= 6;
		loopCount = count / 3;
		count -= 3 * loopCount;

		while (loopCount--) {
			// Cycle A
			loadFloat0 = *++src;
			scaled2 = loadFloat2 * scale;
			converted1 = __fctiw(scaled1);
			__stfiwx(converted0, 0, &buffer0);
			int2 = __lwbrx(&buffer2, 0);
			*++dst = int1;

			// Cycle B
			loadFloat1 = *++src;
			scaled0 = loadFloat0 * scale;
			converted2 = __fctiw(scaled2);
			__stfiwx(converted1, 0, &buffer1);
			int0 = __lwbrx(&buffer0, 0);
			*++dst = int2;

			// Cycle C
			loadFloat2 = *++src;
			scaled1 = loadFloat1 * scale;
			converted0 = __fctiw(scaled0);
			__stfiwx(converted2, 0, &buffer2);
			int1 = __lwbrx(&buffer1, 0);
			*++dst = int0;
		}

		// Cycle 5
		scaled2 = loadFloat2 * scale;
		converted1 = __fctiw(scaled1);
		__stfiwx(converted0, 0, &buffer0);
		int2 = __lwbrx(&buffer2, 0);
		*++dst = int1;

		// Cycle 4
		converted2 = __fctiw(scaled2);
		__stfiwx(converted1, 0, &buffer1);
		int0 = __lwbrx(&buffer0, 0);
		*++dst = int2;

		// Cycle 3
		__stfiwx(converted2, 0, &buffer2);
		int1 = __lwbrx(&buffer1, 0);
		*++dst = int0;

		// Cycle 2
		int2 = __lwbrx(&buffer2, 0);
		*++dst = int1;

		// Cycle 1
		*++dst = int2;
	}

	while (count--) {
		loadFloat0 = *++src;
		scaled0 = loadFloat0 * scale;
		converted0 = __fctiw(scaled0);
		__stfiwx(converted0, 0, &buffer0);
		int0 = __lwbrx(&buffer0, 0);
		*++dst = int0;
	}

	//restore the old FPSCR setting
	__asm__ __volatile__ ( "mtfsf 7, %0" : : "f" (oldSetting) );
}

#endif // TARGET_CPU_PPC
