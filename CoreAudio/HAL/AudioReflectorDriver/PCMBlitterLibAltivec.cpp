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
	PCMBlitterLibAltivec.c
	
=============================================================================*/

/*
	N.B. This file should be compiled with:
		-fno-schedule-insns -fno-schedule-insns2 -mtune=G5
	
	Function requirements:
		floats and int32's are 4-byte aligned
		int16's are 2-byte aligned
		int24's have no alignment requirement
		must process at least 16 frames
*/

#include <TargetConditionals.h>

//     vec_dss(0);


#if TARGET_CPU_PPC
#include "PCMBlitterLib.h"
#include <sys/types.h>

typedef vector unsigned char VecU8;
typedef vector signed short VecS16;
typedef vector signed int VecS32;
typedef vector unsigned int VecU32;
typedef vector float VecFloat;


#define PRINT 0
#define AMBER_TRACE 0

#if AMBER_TRACE
	#include <CHUD/CHUD.h>		// amber
	#define TOGGLE_AMBER chudStartStopAmber()
		// then run "amber -i <progname>"
#else
	#define TOGGLE_AMBER
#endif

#if PRINT
#define VEC_PRINT(TYPE, fmt) \
void	print(const char *label, vector TYPE v)	\
	{											\
		union {									\
			vector TYPE v;						\
			TYPE buf[16/sizeof(TYPE)];			\
		} u;									\
		u.v = v;								\
		if (label) printf("%-25.25s  ", label);	\
		for (int i = 0; i < 16/sizeof(TYPE); ++i) \
			printf(fmt, u.buf[i]);				\
		printf("\n");							\
	}

#define ARR_PRINT(TYPE, fmt) \
void	print(const char *label, const TYPE *a, int n = 16/sizeof(TYPE))	\
	{											\
		if (label) printf("%-25.25s", label);	\
		int align = (uintptr_t)a & 0xF;			\
		int i = 0;								\
		if (align) { align /= sizeof(TYPE); a -= align; i = -align; } \
		int col = 0, ncol = 16/sizeof(TYPE); /*if (ncol < 8) ncol = 8;*/ \
		for ( ; i < n; ++i, ++col, ++a) {			\
			if (col % ncol == 0) printf("\n%25p  ", a);	\
			if (i >= 0) printf(fmt, *a); else printf(sizeof(TYPE)==1 ? "   . " : "        . ");	\
		} printf("\n");							\
	}

// print 3 vectors of packed 24-bit ints
void print24(const char *label, VecU8 a, VecU8 b, VecU8 c)
{
	union {
		VecU8 v;
		char buf[16];
	} u;
	int count = 0, row, i;
	
	for (row = 0; row < 3; ++row) {
		if (label)
			printf("%-25.25s  ", row ? "" : label);
		u.v = a;
		for (i = 0; i < 16; ++i, ++count)
			if (count % 3 == 0)
				printf("| %02X ", u.buf[i]);
			else
				printf("  %02X ", u.buf[i]);
		a = b;
		b = c;
		printf("\n");
	}
}

void print24(const char *label, const unsigned char *a, int n)
{
	if (label) printf("%-25.25s  ", label);
	int align = (uintptr_t)a & 0xF;
	int i = 0;
	if (align) {a -= align; i = -align; }
	int col = 0, ncol = 16;
	for ( ; i < n; ++i, ++col, ++a) {
		if (col % ncol == 0) printf("\n%25p  ", a);
		if (i >= 0) {
			if (i % 3 == 0)
				printf("| %02X ", *a);
			else
				printf("  %02X ", *a);
		} else printf("   . ");
	}
	printf("\n");
}

VEC_PRINT(unsigned char, "  %02X ")
VEC_PRINT(signed short, " %8d ")
VEC_PRINT(signed int, " %8X ")
VEC_PRINT(float, "%9.5f ")

ARR_PRINT(unsigned char, "  %02X ")
ARR_PRINT(signed short, " %8d ")
ARR_PRINT(signed int, " %8X ")
ARR_PRINT(float, "%9.5f ")
#else
#define print(...)
#endif // PRINT

// Store all bytes in storevec from (addr + index) to the next 16 byte aligned boundary
#define LEADING_PARTIAL_VECTOR_STORE( storevec, index, addr )								\
	index = 0;																				\
	if ((uintptr_t)addr & 1 )	{															\
			vec_ste( (vector unsigned char) storevec, index, (unsigned char*) (addr) );		\
			index += 1;																		\
	}																						\
	if (((uintptr_t)addr + index) & 2 ) {													\
			vec_ste( (vector unsigned short) storevec, index, (unsigned short*) (addr) );	\
			index += 2;																		\
	}																						\
	while (((uintptr_t)addr + index) & 12) {												\
			vec_ste( (vector unsigned int) storevec, index, (unsigned int*) (addr) );		\
			index += 4;																		\
	}																						\
	if (index == 0) {																		\
			vec_st( (vector unsigned int) storevec, index, (unsigned int*) (addr) );		\
			index = 16;																		\
	}

// assumes 2-byte alignment of the elements
#define LEADING_PARTIAL_VECTOR_STORE_2( storevec, index, addr )								\
	index = 0;																				\
	if (((uintptr_t)addr + index) & 2 ) {													\
			vec_ste( (vector unsigned short) storevec, index, (unsigned short*) (addr) );	\
			index += 2;																		\
	}																						\
	while (((uintptr_t)addr + index) & 12) {													\
			vec_ste( (vector unsigned int) storevec, index, (unsigned int*) (addr) );		\
			index += 4;																		\
	}																						\
	if (index == 0) {																		\
			vec_st( (vector unsigned int) storevec, index, (unsigned int*) (addr) );		\
			index = 16;																		\
	}

// assumes 4-byte alignment of the elements
#define LEADING_PARTIAL_VECTOR_STORE_4( storevec, index, addr )								\
	index = 0;																				\
	while (((uintptr_t)addr + index) & 12) {												\
			vec_ste( (vector unsigned int) storevec, index, (unsigned int*) (addr) );		\
			index += 4;																		\
	}																						\
	if (index == 0) {																		\
			vec_st( (vector unsigned int) storevec, index, (unsigned int*) (addr) );		\
			index = 16;																		\
	}

int minint(int a, int b);
inline int minint(int a, int b) { return (a < b) ? a : b; }

// workaround gcc 4 screwing up codegen by invoking certain instructions through a macro
#define xvec_perm(result, lo, hi, pmap) \
	__asm__ volatile("vperm %0, %1, %2, %3" \
			/* outputs */	: "=v" (result) \
			/* inputs */	: "v" (lo), "v" (hi), "v" (pmap));		

#define xvec_ctf(result, src, n) \
	__asm__ volatile("vcfsx %0, %1, %2" \
			/* outputs */	: "=v" (result) \
			/* inputs */	: "v" (src), "K" (n));

// ==========================================================================================
#pragma mark -
#pragma mark Float -> Int

static const VecU8	gSwap16 = (VecU8)(	0x01, 0x00, 0x03, 0x02, 0x05, 0x04, 0x07, 0x06,
										0x09, 0x08, 0x0B, 0x0A, 0x0D, 0x0C, 0x0F, 0x0E);


void	Float32ToInt16_Altivec(const Float32 *fsrc, SInt16 *idst, unsigned int numToConvert, int bigEndian)
{
	int count = numToConvert;
	const UInt8 *src = (UInt8 *)fsrc;
	const UInt8 *srcEnd = src + count * sizeof(Float32);
	srcEnd = (UInt8 *)( ((uintptr_t)srcEnd + 0xF) & ~0xF );	// round up to vector boundary; this is how far we can safely read
	UInt8 *dst = (UInt8 *)idst;
	unsigned int endStoreIndex = 2 * count, storeIndex;	// how many bytes have been stored to dst (which is not advanced);
	int niter;

	VecFloat scale = (VecFloat)(32768.0);
	VecFloat round = (VecFloat)(0.5);

	VecFloat load0, load1, load2;
	VecS32 conv0, conv1, conv2;
	VecS16 short0, short1;
	VecS16 curStore, storeForward;
	
	// load 2 or 3 vectors without relignment tricks
	// this will guarantee a minimum of 8 floats -- we must have at least 8 shorts to store
	load0 = (VecFloat)vec_ld(0, src);
	__builtin_prefetch(src + 128);
	__builtin_prefetch(src + 256);
	__builtin_prefetch(src + 384);
	load1 = (VecFloat)vec_ld(15, src);
	load2 = (VecFloat)vec_ld(31, src);
	
	// convert
	load0 = vec_madd(load0, scale, round);
	load1 = vec_madd(load1, scale, round);
	load2 = vec_madd(load2, scale, round);
	load0 = vec_floor(load0);
	load1 = vec_floor(load1);
	load2 = vec_floor(load2);
	conv0 = vec_cts(load0, 0);
	conv1 = vec_cts(load1, 0);
	conv2 = vec_cts(load2, 0);
	short0 = vec_packs(conv0, conv0);
	short1 = vec_packs(conv1, conv2);

	/*
		Magic permute vectors to fix both alignments at once.
		These were constructed by looking at how short0 / short1 line up with the destination address.
		
		f/i	0		2		4		6		8		A		C		E
		4	0A-19	08-17	06-15	04-13	02-11	00-0F	-2-0D	-4-0B
		8	0C-1B	0A-19	08-17	06-15	04-13	02-11	00-0F	-2-0D
		C	0E-1D	04-13	0A-19	08-17	06-15	04-13	02-11	00-0F
		0	10-1F	0E-1D	0C-1B	0A-19	08-17	06-15	04-13	02-11

		The 3 odd cases in the top right... see comments below.
	*/
	int srcAlignUp = ((uintptr_t)(src - 1) & 0xF) + 1;	// 4,8,C,10
	int srcComponent = 8 + srcAlignUp / 2;				// A,C,E,10
	int dstAlign = (uintptr_t)dst & 0xF;
	int shift = srcComponent - dstAlign;
	VecU8 storePerm;
	
	if (shift < 0) {
		// what's special about these cases is that there is more in short0 than can be stored initially 
		storePerm = (VecU8)vec_lvsl(shift, (unsigned char *)0);	// -2-0D -> 12->0D
		if (!bigEndian)
			storePerm = vec_perm(storePerm, storePerm, gSwap16);
		curStore = vec_perm(short0, short0, storePerm);
	} else {
		if (shift == 16)
			storePerm = vec_lvsr(shift, (unsigned char *)0);	// 10..1F
		else
			storePerm = vec_lvsl(shift, (unsigned char *)0); // shift..shift+F
		if (!bigEndian)
			storePerm = vec_perm(storePerm, storePerm, gSwap16);
		curStore = vec_perm(short0, short1, storePerm);
	}

	// store the first part of the result vector, up to the first 16-byte aligned boundary
	LEADING_PARTIAL_VECTOR_STORE_2((VecU8)curStore, storeIndex, dst);
	
	if (shift < 0) {
		// the special case again
		// here we have an entire second vector to store
		curStore = vec_perm(short0, short1, storePerm);
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
	}

	// setup for next loop iteration.
	// do not increment the store pointers; that is handled by storeIndex.
	// do need to increment the load pointers. the loads lead the stores.
	src += 2 * sizeof(VecFloat);
	
	// count tracks how many source samples have been loaded
	count -= 8;
	
	// recycle
	storeForward = short1;
	
	// unrolled vector loop: 32 samples at a time
	niter = minint(count / 32, (endStoreIndex - storeIndex) / 64);
	UInt8 *dst2 = dst + storeIndex;
	storeIndex += niter * 64;
	count -= 32 * niter;
	
	for ( ; --niter >= 0; ) {
		VecFloat load3, load4, load5, load6, load7;
		VecS32 conv3, conv4, conv5, conv6, conv7;
		VecS16 short2, short3, short4;
		const UInt8 *srcAhead = src + 384;

		// load
		load0 = (VecFloat)vec_ld(15, src);
		__builtin_prefetch(srcAhead);
		load1 = (VecFloat)vec_ld(31, src);
		load2 = (VecFloat)vec_ld(47, src);
		load3 = (VecFloat)vec_ld(63, src);
		load4 = (VecFloat)vec_ld(79, src);
		load5 = (VecFloat)vec_ld(95, src);
		load6 = (VecFloat)vec_ld(111, src);
		load7 = (VecFloat)vec_ld(127, src);
		
		// convert
		load0 = vec_madd(load0, scale, round);
		load1 = vec_madd(load1, scale, round);
		load2 = vec_madd(load2, scale, round);
		load3 = vec_madd(load3, scale, round);
		load4 = vec_madd(load4, scale, round);
		load5 = vec_madd(load5, scale, round);
		load6 = vec_madd(load6, scale, round);
		load7 = vec_madd(load7, scale, round);

		load0 = vec_floor(load0);
		load1 = vec_floor(load1);
		load2 = vec_floor(load2);
		load3 = vec_floor(load3);
		load4 = vec_floor(load4);
		load5 = vec_floor(load5);
		load6 = vec_floor(load6);
		load7 = vec_floor(load7);

		conv0 = vec_cts(load0, 0);
		conv1 = vec_cts(load1, 0);
		conv2 = vec_cts(load2, 0);
		conv3 = vec_cts(load3, 0);
		conv4 = vec_cts(load4, 0);
		conv5 = vec_cts(load5, 0);
		conv6 = vec_cts(load6, 0);
		conv7 = vec_cts(load7, 0);
		
		// move this here from end of loop (vec_cts above are 8 cycles on G5 and stall)
		src += 8 * sizeof(VecFloat);

		short1 = vec_packs(conv0, conv1);
		short2 = vec_packs(conv2, conv3);
		short3 = vec_packs(conv4, conv5);
		short4 = vec_packs(conv6, conv7);

		xvec_perm(short0, storeForward, short1, storePerm);
		xvec_perm(short1, short1, short2, storePerm);
		xvec_perm(short2, short2, short3, storePerm);
		xvec_perm(short3, short3, short4, storePerm);
		
		// store
		vec_st((VecU8)short0, 0, dst2);
		vec_st((VecU8)short1, 16, dst2);
		vec_st((VecU8)short2, 32, dst2);
		vec_st((VecU8)short3, 48, dst2);
		
		// recycle
		storeForward = short4;
		dst2 += 64;
	}
	
	// cleanup loop: 8 samples at a time
	unsigned int endStoreVecBound = endStoreIndex - 16;
	for ( ; count > 0; count -= 8) {
		// load
		if (15+src < srcEnd) {
			load0 = (VecFloat)vec_ld(15, src);
			load0 = vec_madd(load0, scale, round);
			load0 = vec_floor(load0);
			conv0 = vec_cts(load0, 0);

			if (31+src < srcEnd && count >= 4) {
				load1 = (VecFloat)vec_ld(31, src);
				load1 = vec_madd(load1, scale, round);
				load1 = vec_floor(load1);
				conv1 = vec_cts(load1, 0);
			}
		}
		
		// convert
		short0 = vec_packs(conv0, conv1);
		
		// store
		curStore = vec_perm(storeForward, short0, storePerm);
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
		
		// recycle
		storeForward = short0;
		
		src += 2 * sizeof(VecFloat);
	}
	curStore = vec_perm(storeForward, storeForward, storePerm);
storeLeftovers:
	while (storeIndex < endStoreIndex) {
		vec_ste(curStore, storeIndex, (SInt16 *)dst);
		storeIndex += 2;
	}
}

// ==========================================================================================

void	Float32ToInt24_Altivec(const Float32 *fsrc, UInt8 *dst, unsigned int numToConvert, int bigEndian)
{
	int count = numToConvert;	// should be signed so loop counter can go negative at end
	const UInt8 *src = (UInt8 *)fsrc;
	const UInt8 *srcEnd = src + count * sizeof(Float32);
	srcEnd = (UInt8 *)( ((uintptr_t)srcEnd + 0xF) & ~0xF );	// round up to vector boundary; this is how far we can safely read
	unsigned int endStoreIndex = 3 * count, storeIndex;	// how many bytes have been stored to dst (which is not advanced);

	VecFloat scale = (VecFloat)(2147483648.0);
	VecFloat round = (VecFloat)(128.0);

	// how to extract 3 vectors of 24-bit samples from 4 vectors of 32-bit ints:
	// pluck the high 24 bits out of each 32-bit word
	VecU8 perm1, perm2, perm3;
	
	if (bigEndian) {
		perm1 = (VecU8)(	0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x08, 0x09, 
							0x0a, 0x0c, 0x0d, 0x0e, 0x10, 0x11, 0x12, 0x14);
		perm2 = (VecU8)(	0x15, 0x16, 0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x1e,
							0x20, 0x21, 0x22, 0x24, 0x25, 0x26, 0x28, 0x29);
		perm3 = (VecU8)(	0x2a, 0x2c, 0x2d, 0x2e, 0x30, 0x31, 0x32, 0x34, 
							0x35, 0x36, 0x38, 0x39, 0x3a, 0x3c, 0x3d, 0x3e);
	} else {
		perm1 = (VecU8)(	0x02, 0x01, 0x00, 0x06, 0x05, 0x04, 0x0a, 0x09, 
							0x08, 0x0e, 0x0d, 0x0c, 0x12, 0x11, 0x10, 0x16);
		perm2 = (VecU8)(	0x15, 0x14, 0x1a, 0x19, 0x18, 0x1e, 0x1d, 0x1c,
							0x22, 0x21, 0x20, 0x26, 0x25, 0x24, 0x2a, 0x29);
		perm3 = (VecU8)(	0x28, 0x2e, 0x2d, 0x2c, 0x32, 0x31, 0x30, 0x36, 
							0x35, 0x34, 0x3a, 0x39, 0x38, 0x3e, 0x3d, 0x3c);
	}

	VecFloat load0, load1, load2, load3, load4;
	VecS32 conv0, conv1, conv2, conv3;
	VecU8 pack0, pack1, pack2, packForward;
	VecU8 curStore;
	VecU8 one = vec_splat_u8(1);
	VecU8 alignLoads = vec_add( vec_lvsl(-1L, src), one);
		//same as lvsl(0,a), except 16 byte aligned case where we get { 16, 17, 18, ...}
	VecU8 alignStores = vec_lvsr(0, dst);
	
	// load 4 vectors and align into load0-load3
	load0 = (VecFloat)vec_ld(0, src);
	__builtin_prefetch(src + 128);
	__builtin_prefetch(src + 256);
	load1 = (VecFloat)vec_ld(15, src);
	load2 = (VecFloat)vec_ld(31, src);
	load3 = (VecFloat)vec_ld(47, src);
	load4 = (VecFloat)vec_ld(63, src);
	load0 = vec_perm(load0, load1, alignLoads);
	load1 = vec_perm(load1, load2, alignLoads);
	load2 = vec_perm(load2, load3, alignLoads);
	load3 = vec_perm(load3, load4, alignLoads);
	
	// convert to int32
	load0 = vec_madd(load0, scale, round);
	load1 = vec_madd(load1, scale, round);
	load2 = vec_madd(load2, scale, round);
	load3 = vec_madd(load3, scale, round);
	load0 = vec_floor(load0);
	load1 = vec_floor(load1);
	load2 = vec_floor(load2);
	load3 = vec_floor(load3);
	conv0 = vec_cts(load0, 0);
	conv1 = vec_cts(load1, 0);
	conv2 = vec_cts(load2, 0);
	conv3 = vec_cts(load3, 0);
	
	// pack to int24
	pack0 = (VecU8)vec_perm(conv0, conv1, perm1);
	pack1 = (VecU8)vec_perm(conv2, conv1, perm2);
	pack2 = (VecU8)vec_perm(conv2, conv3, perm3);

	curStore = vec_perm(pack0, pack0, alignStores);
	
	LEADING_PARTIAL_VECTOR_STORE(curStore, storeIndex, dst);

	curStore = vec_perm(pack0, pack1, alignStores);
	vec_st((VecU8)curStore, storeIndex, dst);
	storeIndex += 16;

	curStore = vec_perm(pack1, pack2, alignStores);
	vec_st((VecU8)curStore, storeIndex, dst);
	storeIndex += 16;

	// setup for next loop iteration.
	// do not increment the store pointers; that is handled by storeIndex.
	// do need to increment the load pointers. the loads lead the stores.
	src += 4 * sizeof(VecFloat);
	
	// count tracks how many source samples have been loaded
	count -= 16;
	
	// recycle
	load0 = load4;
	packForward = pack2;
	UInt8 *dst2 = dst + storeIndex;
	int niter = count / 32;
	count -= 32 * niter;
	storeIndex += 96 * niter;
	
	// unrolled vector loop, 32 samples at a time
	for ( ; --niter >= 0; ) {
		VecFloat load5, load6, load7, load8;
		VecS32 conv4, conv5, conv6, conv7;
		VecU8 pack3, pack4, pack5;
		const UInt8 *src2 = src + 256;
		
		// load
		load1 = (VecFloat)vec_ld(15, src);
		__builtin_prefetch(src2);
		load2 = (VecFloat)vec_ld(31, src);
		load3 = (VecFloat)vec_ld(47, src);
		load4 = (VecFloat)vec_ld(63, src);
		load5 = (VecFloat)vec_ld(79, src);
		load6 = (VecFloat)vec_ld(95, src);
		load7 = (VecFloat)vec_ld(111, src);
		load8 = (VecFloat)vec_ld(127, src);

		// align
		xvec_perm(load0, load0, load1, alignLoads);
		xvec_perm(load1, load1, load2, alignLoads);
		xvec_perm(load2, load2, load3, alignLoads);
		xvec_perm(load3, load3, load4, alignLoads);
		xvec_perm(load4, load4, load5, alignLoads);
		xvec_perm(load5, load5, load6, alignLoads);
		xvec_perm(load6, load6, load7, alignLoads);
		xvec_perm(load7, load7, load8, alignLoads);
		
		// convert to int32
		load0 = vec_madd(load0, scale, round);
		load1 = vec_madd(load1, scale, round);
		load2 = vec_madd(load2, scale, round);
		load3 = vec_madd(load3, scale, round);
		load4 = vec_madd(load4, scale, round);
		load5 = vec_madd(load5, scale, round);
		load6 = vec_madd(load6, scale, round);
		load7 = vec_madd(load7, scale, round);
		load0 = vec_floor(load0);
		load1 = vec_floor(load1);
		load2 = vec_floor(load2);
		load3 = vec_floor(load3);
		load4 = vec_floor(load4);
		load5 = vec_floor(load5);
		load6 = vec_floor(load6);
		load7 = vec_floor(load7);
		conv0 = vec_cts(load0, 0);
		conv1 = vec_cts(load1, 0);
		conv2 = vec_cts(load2, 0);
		conv3 = vec_cts(load3, 0);
		conv4 = vec_cts(load4, 0);
		conv5 = vec_cts(load5, 0);
		conv6 = vec_cts(load6, 0);
		conv7 = vec_cts(load7, 0);
		
		// pack to int24
		xvec_perm(pack0, conv0, conv1, perm1);
		xvec_perm(pack1, conv2, conv1, perm2);
		xvec_perm(pack2, conv2, conv3, perm3);

		xvec_perm(pack3, conv4, conv5, perm1);
		xvec_perm(pack4, conv6, conv5, perm2);
		xvec_perm(pack5, conv6, conv7, perm3);
		
		// store
		xvec_perm(conv0, packForward, pack0, alignStores);
		xvec_perm(conv1, pack0, pack1, alignStores);
		xvec_perm(conv2, pack1, pack2, alignStores);
		xvec_perm(conv3, pack2, pack3, alignStores);
		xvec_perm(conv4, pack3, pack4, alignStores);
		xvec_perm(conv5, pack4, pack5, alignStores);

		vec_st((VecU8)conv0, 0, dst2);
		vec_st((VecU8)conv1, 16, dst2);
		vec_st((VecU8)conv2, 32, dst2);
		vec_st((VecU8)conv3, 48, dst2);
		vec_st((VecU8)conv4, 64, dst2);
		vec_st((VecU8)conv5, 80, dst2);

		// recycle
		load0 = load8;
		packForward = pack5;
		
		src += 8 * sizeof(VecFloat);
		dst2 += 96;
	}

	// cleanup loop: 16 samples at a time, with bounds checking
	unsigned int endStoreVecBound = endStoreIndex - 16;
	for ( ; count > 0; count -= 16) {
		// load
		if (15+src < srcEnd) {
			load1 = (VecFloat)vec_ld(15, src);
			if (31+src < srcEnd) {
				load2 = (VecFloat)vec_ld(31, src);
				if (47+src < srcEnd) {
					load3 = (VecFloat)vec_ld(47, src);
					if (63+src < srcEnd) {
						load4 = (VecFloat)vec_ld(63, src);
					}
				}
			}
		}
		// align
		load0 = vec_perm(load0, load1, alignLoads);
		load1 = vec_perm(load1, load2, alignLoads);
		load2 = vec_perm(load2, load3, alignLoads);
		load3 = vec_perm(load3, load4, alignLoads);
		
		// convert to int32
		load0 = vec_madd(load0, scale, round);
		load1 = vec_madd(load1, scale, round);
		load2 = vec_madd(load2, scale, round);
		load3 = vec_madd(load3, scale, round);
		load0 = vec_floor(load0);
		load1 = vec_floor(load1);
		load2 = vec_floor(load2);
		load3 = vec_floor(load3);
		conv0 = vec_cts(load0, 0);
		conv1 = vec_cts(load1, 0);
		conv2 = vec_cts(load2, 0);
		conv3 = vec_cts(load3, 0);
		
		// pack to int24
		pack0 = (VecU8)vec_perm(conv0, conv1, perm1);
		pack1 = (VecU8)vec_perm(conv2, conv1, perm2);
		pack2 = (VecU8)vec_perm(conv2, conv3, perm3);

		curStore = vec_perm(packForward, pack0, alignStores);
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
	
		curStore = vec_perm(pack0, pack1, alignStores);
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;

		curStore = vec_perm(pack1, pack2, alignStores);
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;

		// recycle
		load0 = load4;
		packForward = pack2;
		
		src += 4 * sizeof(VecFloat);
	}
	curStore = vec_perm(packForward, packForward, alignStores);

storeLeftovers:
	while (storeIndex < endStoreIndex) {
		vec_ste(curStore, storeIndex, dst);
		++storeIndex;
	}
}

// ==========================================================================================

void	Float32ToInt32_Altivec(const Float32 *fsrc, SInt32 *idst, unsigned int numToConvert, int bigEndian)
{
	int count = numToConvert;	// needs to be signed
	const UInt8 *src = (UInt8 *)fsrc;
	const UInt8 *srcEnd = src + count * sizeof(Float32);
	srcEnd = (UInt8 *)( ((uintptr_t)srcEnd + 0xF) & ~0xF );	// round up to vector boundary; this is how far we can safely read
	UInt8 *dst = (UInt8 *)idst;
	unsigned int endStoreIndex = 4 * count, storeIndex;	// how many bytes have been stored to dst (which is not advanced);
	int niter;

	VecFloat scale = (VecFloat)(2147483648.0);
	VecFloat round = (VecFloat)(0.5);

	VecFloat load0, load1;
	VecS32 conv0, conv1;
	VecS32 curStore, storeForward;
	
	// load 2 vectors
	load0 = (VecFloat)vec_ld(0, src);
	__builtin_prefetch(src + 128);
	__builtin_prefetch(src + 256);
	load1 = (VecFloat)vec_ld(15, src);
	
	// convert to int32
	load0 = vec_madd(load0, scale, round);
	load1 = vec_madd(load1, scale, round);
	load0 = vec_floor(load0);
	load1 = vec_floor(load1);
	conv0 = vec_cts(load0, 0);
	conv1 = vec_cts(load1, 0);
	
	/*
		Permute vectors:
		
		f / i	0	4	8	C
		0		10	0C	08	04
		4		04	00	-4	-8
		8		08	04	00	-4
		C		0C	08	04	00
	*/
	int srcAlignUp = ((uintptr_t)(src - 1) & 0xF) + 1;	// 4,8,C,10
	int dstAlign = (uintptr_t)dst & 0xF;
	int shift = srcAlignUp - dstAlign;
	VecU8 storePerm;

	if (shift < 0) {
		// what's special about these cases is that there is more in conv0 than can be stored initially 
		storePerm = (VecU8)vec_lvsl(shift, (unsigned char *)0);	// -2-0D -> 12->0D
		if (!bigEndian) {
			VecU8 swap = (VecU8)(	0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04,
									0x0B, 0x0A, 0x09, 0x08, 0x0F, 0x0E, 0x0D, 0x0C);
			storePerm = vec_perm(storePerm, storePerm, swap);
		}
		curStore = vec_perm(conv0, conv0, storePerm);
	} else {
		if (shift == 16)
			storePerm = vec_lvsr(shift, (unsigned char *)0);	// 10..1F
		else
			storePerm = vec_lvsl(shift, (unsigned char *)0); // shift..shift+F
		if (!bigEndian) {
			VecU8 swap = (VecU8)(	0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04,
									0x0B, 0x0A, 0x09, 0x08, 0x0F, 0x0E, 0x0D, 0x0C);
			storePerm = vec_perm(storePerm, storePerm, swap);
		}
		curStore = vec_perm(conv0, conv1, storePerm);
	}

	// store the first part of the result vector, up to the first 16-byte aligned boundary
	LEADING_PARTIAL_VECTOR_STORE_4((VecU8)curStore, storeIndex, dst);
	
	if (shift < 0) {
		// the special case again
		// here we have an entire second vector to store
		curStore = vec_perm(conv0, conv1, storePerm);
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
	}

	// setup for next loop iteration.
	// do not increment the store pointers; that is handled by storeIndex.
	// do need to increment the load pointers. the loads lead the stores.
	src += sizeof(VecFloat);
	
	// count tracks how many source samples have been loaded
	count -= 4;
	
	// recycle
	storeForward = conv1;
	
	// unrolled loop
	// for edge cases where shift is negative -- don't write too far
	niter = minint(count / 32, (endStoreIndex - storeIndex) / 128);
	UInt8 *dst2 = dst + storeIndex;
	storeIndex += 128 * niter;
	count -= 32 * niter;
	
	for ( ; --niter >= 0; ) {
		VecFloat load2, load3, load4, load5, load6, load7;
		VecS32 conv2, conv3, conv4, conv5, conv6, conv7, conv8;
		const UInt8 *srcAhead = src + 256;

		// load
		load0 = (VecFloat)vec_ld(15, src);
		__builtin_prefetch(srcAhead);
		load1 = (VecFloat)vec_ld(31, src);
		load2 = (VecFloat)vec_ld(47, src);
		load3 = (VecFloat)vec_ld(63, src);
		load4 = (VecFloat)vec_ld(79, src);
		load5 = (VecFloat)vec_ld(95, src);
		load6 = (VecFloat)vec_ld(111, src);
		load7 = (VecFloat)vec_ld(127, src);
		
		// convert to int32
		load0 = vec_madd(load0, scale, round);
		load1 = vec_madd(load1, scale, round);
		load2 = vec_madd(load2, scale, round);
		load3 = vec_madd(load3, scale, round);
		load4 = vec_madd(load4, scale, round);
		load5 = vec_madd(load5, scale, round);
		load6 = vec_madd(load6, scale, round);
		load7 = vec_madd(load7, scale, round);
		load0 = vec_floor(load0);
		load1 = vec_floor(load1);
		load2 = vec_floor(load2);
		load3 = vec_floor(load3);
		load4 = vec_floor(load4);
		load5 = vec_floor(load5);
		load6 = vec_floor(load6);
		load7 = vec_floor(load7);
		conv1 = vec_cts(load0, 0);
		conv2 = vec_cts(load1, 0);
		conv3 = vec_cts(load2, 0);
		conv4 = vec_cts(load3, 0);
		conv5 = vec_cts(load4, 0);
		conv6 = vec_cts(load5, 0);
		conv7 = vec_cts(load6, 0);
		conv8 = vec_cts(load7, 0);
		
		// permute and store
		conv0 = vec_perm(storeForward, conv1, storePerm);
		conv1 = vec_perm(conv1, conv2, storePerm);
		conv2 = vec_perm(conv2, conv3, storePerm);
		conv3 = vec_perm(conv3, conv4, storePerm);
		conv4 = vec_perm(conv4, conv5, storePerm);
		conv5 = vec_perm(conv5, conv6, storePerm);
		conv6 = vec_perm(conv6, conv7, storePerm);
		conv7 = vec_perm(conv7, conv8, storePerm);

		vec_st((VecU8)conv0, 0, dst2);
		vec_st((VecU8)conv1, 16, dst2);
		vec_st((VecU8)conv2, 32, dst2);
		vec_st((VecU8)conv3, 48, dst2);
		vec_st((VecU8)conv4, 64, dst2);
		vec_st((VecU8)conv5, 80, dst2);
		vec_st((VecU8)conv6, 96, dst2);
		vec_st((VecU8)conv7, 112, dst2);
		
		src += 8 * sizeof(VecFloat);
		load0 = load7;
		dst2 += 128;
		storeForward = conv8;
	}
	
	unsigned int endStoreVecBound = endStoreIndex - 16;

	// cleanup loop: 4 samples at a time, with bounds checking
	for ( ; count > 0; count -= 4) {
		if (src+15 < srcEnd) {
			load0 = (VecFloat)vec_ld(15, src);
			load0 = vec_madd(load0, scale, round);
			load0 = vec_floor(load0);
			conv0 = vec_cts(load0, 0);
		}
		curStore = vec_perm(storeForward, conv0, storePerm);
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
		
		src += sizeof(VecFloat);
		storeForward = conv0;
	}
	curStore = vec_perm(storeForward, storeForward, storePerm);

storeLeftovers:
	while (storeIndex < endStoreIndex) {
		vec_ste((VecS32)curStore, storeIndex, (SInt32 *)dst);
		storeIndex += 4;
	}
}

// ==========================================================================================
#pragma mark -
#pragma mark Int->Float

void	Int16ToFloat32_Altivec(const SInt16 *isrc, Float32 *fdst, unsigned int numToConvert, int bigEndian)
{
	int count = numToConvert;	// needs to be signed
	const UInt8 *src = (UInt8 *)isrc;
	const UInt8 *srcEnd = src + count * sizeof(SInt16);
	srcEnd = (UInt8 *)( ((uintptr_t)srcEnd + 0xF) & ~0xF );	// round up to vector boundary; this is how far we can safely read
	UInt8 *dst = (UInt8 *)fdst;
	unsigned int endStoreIndex = 4 * count, storeIndex;	// how many bytes have been stored to dst (which is not advanced);
	int niter;

	VecS16 load0, load1, combinedLoad;
	VecS32 ext0, ext1;
	VecFloat conv0, conv1, curStore;
	
	// load 2 vectors of 16-bit ints
	load0 = (VecS16)vec_ld(0, src);
	__builtin_prefetch(src + 128);
	__builtin_prefetch(src + 256);
	load1 = (VecS16)vec_ld(15, src);
	
	/*
		Permute vectors, to be applied to the 2 loaded vectors of shorts
		
		f / i	0	2	4	6	8	A	C	E
		0		10	02	04	06	08	0A	0C	0E
		4		-2*	00	02	04	06	08	0A	0C
		8		-4*	-2@	00	02	04	06	08	0A
		C		-6*	-4@	-2@	00	02	04	06	08
	*/
	int srcAlign = (uintptr_t)src & 0xF;
	int dstAlign = (uintptr_t)dst & 0xF;
	int shift = srcAlign - (dstAlign / 2);
	VecU8 loadPerm;

	if (srcAlign == 0 && dstAlign == 0) {
		loadPerm = vec_lvsr(0, (unsigned char *)0);	// 10..1F
	} else {
		loadPerm = vec_lvsl(shift, (unsigned char *)0); // shift..shift+F
	}
	if (!bigEndian)
		loadPerm = vec_perm(loadPerm, loadPerm, gSwap16);
	combinedLoad = vec_perm(load0, shift < 0 ? load0 : load1, loadPerm);

	// expand to 2 vectors of 32-bit ints
	ext0 = vec_unpackh(combinedLoad);
	ext1 = vec_unpackl(combinedLoad);
	
	// convert to float
	conv0 = vec_ctf(ext0, 15);
	conv1 = vec_ctf(ext1, 15);
	
	LEADING_PARTIAL_VECTOR_STORE_4((VecU8)conv0, storeIndex, dst);
	vec_st((VecU8)conv1, storeIndex, dst);
	storeIndex += 16;

	// setup for next loop iteration.
	// do not increment the store pointers; that is handled by storeIndex.
	// do need to increment the load pointers. the loads lead the stores.
	src += sizeof(VecS16);
	
	// count tracks how many source samples have been loaded
	count -= 8;
	
	if (srcAlign != 0 && shift < 0) {
		// in these cases, we need to store another 2 vectors of floats
		combinedLoad = vec_perm(load0, load1, loadPerm);
		ext0 = vec_unpackh(combinedLoad);
		ext1 = vec_unpackl(combinedLoad);
		conv0 = vec_ctf(ext0, 15);
		conv1 = vec_ctf(ext1, 15);
		vec_st((VecU8)conv0, storeIndex, dst);
		storeIndex += 16;
		vec_st((VecU8)conv1, storeIndex, dst);
		storeIndex += 16;
	}
	
	// recycle
	load0 = load1;
	curStore = conv1;
	
	niter = minint(count / 32, (endStoreIndex - storeIndex) / 128);
		// for edge cases where shift is negative -- don't write too far
	UInt8 *dst2 = dst + storeIndex;
	storeIndex += 128 * niter;
	count -= 32 * niter;
	
	// unrolled vector loop, 32 samples at a time
	for ( ; --niter >= 0; ) {
		VecS16 load2, load3, load4;
		VecS32 ext2, ext3, ext4, ext5, ext6, ext7;
		VecFloat conv2, conv3, conv4, conv5, conv6;
		const UInt8 *srcAhead = src + 256;
		
		load1 = (VecS16)vec_ld(15, src);
		__builtin_prefetch(srcAhead);
		load2 = (VecS16)vec_ld(31, src);
		load3 = (VecS16)vec_ld(47, src);
		load4 = (VecS16)vec_ld(63, src);
		
		xvec_perm(load0, load0, load1, loadPerm);
		xvec_perm(load1, load1, load2, loadPerm);
		xvec_perm(load2, load2, load3, loadPerm);
		xvec_perm(load3, load3, load4, loadPerm);
		ext0 = vec_unpackh(load0);
		ext1 = vec_unpackl(load0);
		ext2 = vec_unpackh(load1);
		ext3 = vec_unpackl(load1);
		ext4 = vec_unpackh(load2);
		ext5 = vec_unpackl(load2);
		ext6 = vec_unpackh(load3);
		ext7 = vec_unpackl(load3);
		xvec_ctf(conv0, ext0, 15);
		xvec_ctf(conv1, ext1, 15);
		xvec_ctf(conv2, ext2, 15);
		xvec_ctf(conv3, ext3, 15);
		xvec_ctf(conv4, ext4, 15);
		xvec_ctf(conv5, ext5, 15);
		xvec_ctf(conv6, ext6, 15);
		xvec_ctf(curStore, ext7, 15);
		
		vec_st((VecU8)conv0, 0, dst2);
		vec_st((VecU8)conv1, 16, dst2);
		vec_st((VecU8)conv2, 32, dst2);
		vec_st((VecU8)conv3, 48, dst2);
		vec_st((VecU8)conv4, 64, dst2);
		vec_st((VecU8)conv5, 80, dst2);
		vec_st((VecU8)conv6, 96, dst2);
		vec_st((VecU8)curStore, 112, dst2);
		
		src += 4 * sizeof(VecS16);
		dst2 += 128;
		load0 = load4;
	}
	
	// cleanup loop: 8 samples at a time, with bounds checking
	unsigned int endStoreVecBound = endStoreIndex - 16;
	for ( ; count > 0; count -= 8) {
		if (src+15 < srcEnd) {
			load1 = (VecS16)vec_ld(15, src);
		}
		load0 = vec_perm(load0, load1, loadPerm);
		ext0 = vec_unpackh(load0);
		ext1 = vec_unpackl(load0);
		conv0 = vec_ctf(ext0, 15);
		conv1 = vec_ctf(ext1, 15);
		
		curStore = conv0;
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
		
		curStore = conv1;
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
		
		src += sizeof(VecS16);
		load0 = load1;
	}
	if (storeIndex < endStoreIndex) {
		load0 = vec_perm(load0, load0, loadPerm);
		ext0 = vec_unpackh(load0);
		curStore = vec_ctf(ext0, 15);
storeLeftovers:
		while (storeIndex < endStoreIndex) {
			vec_ste((VecFloat)curStore, storeIndex, (Float32 *)dst);
			storeIndex += 4;
		}
	}
}

void	Int24ToFloat32_Altivec(const UInt8 *isrc, Float32 *fdst, unsigned int numToConvert, int bigEndian)
{
	int count = numToConvert;	// needs to be signed
	const UInt8 *src = isrc;
	const UInt8 *srcEnd = src + 3 * count;
	srcEnd = (UInt8 *)( ((uintptr_t)srcEnd + 0xF) & ~0xF );	// round up to vector boundary; this is how far we can safely read
	UInt8 *dst = (UInt8 *)fdst;
	unsigned int endStoreIndex = 4 * count, storeIndex;	// how many bytes have been stored to dst (which is not advanced);

	VecU8 load0, load1, load2, load3;
	VecS32 ext0, ext1, ext2, ext3;
	VecFloat conv0, conv1, conv2, conv3, curStore, storeForward;
	VecU8 one = vec_splat_u8(1);
	VecU8 alignLoads = vec_add( vec_lvsl(-1L, src), one);
		//same as lvsl(0,a), except 16 byte aligned case where we get { 16, 17, 18, ...}
	VecU8 alignStores = vec_lvsr(0, dst);
	VecU32 shift = (VecU32)(8);
	
	VecU8 perm1, perm2, perm3, perm4;
	
	// how to expand 3 vectors of 24-bit ints into 4 vectors of 32-bit ints with
	// the 24-bit samples high-aligned, low 8 bits garbage (0x00)
	if (bigEndian) {
		perm1 = (VecU8)(	0x00, 0x01, 0x02, 0x00, 0x03, 0x04, 0x05, 0x00, 
							0x06, 0x07, 0x08, 0x00, 0x09, 0x0a, 0x0b, 0x00);
		perm2 = (VecU8)(	0x0c, 0x0d, 0x0e, 0x00, 0x0f, 0x10, 0x11, 0x00, 
							0x12, 0x13, 0x14, 0x00, 0x15, 0x16, 0x17, 0x00);
		perm3 = (VecU8)(	0x18, 0x19, 0x1a, 0x00, 0x1b, 0x1c, 0x1d, 0x00, 
							0x1e, 0x1f, 0x20, 0x00, 0x21, 0x22, 0x23, 0x00);
		perm4 = (VecU8)(	0x24, 0x25, 0x26, 0x00, 0x27, 0x28, 0x29, 0x00, 
							0x2a, 0x2b, 0x2c, 0x00, 0x2d, 0x2e, 0x2f, 0x00);
	} else {
		perm1 = (VecU8)(	0x02, 0x01, 0x00, 0x00, 0x05, 0x04, 0x03, 0x00, 
							0x08, 0x07, 0x06, 0x00, 0x0b, 0x0a, 0x09, 0x00);
		perm2 = (VecU8)(	0x0e, 0x0d, 0x0c, 0x00, 0x11, 0x10, 0x0f, 0x00, 
							0x14, 0x13, 0x12, 0x00, 0x17, 0x16, 0x15, 0x00);
		perm3 = (VecU8)(	0x1a, 0x19, 0x18, 0x00, 0x1d, 0x1c, 0x1b, 0x00, 
							0x20, 0x1f, 0x1e, 0x00, 0x23, 0x22, 0x21, 0x00);
		perm4 = (VecU8)(	0x26, 0x25, 0x24, 0x00, 0x29, 0x28, 0x27, 0x00, 
							0x2c, 0x2b, 0x2a, 0x00, 0x2f, 0x2e, 0x2d, 0x00);
	}
	
	// load 3 vectors and align into load0-load2
	load0 = vec_ld(0, src);
	__builtin_prefetch(src + 128);
	__builtin_prefetch(src + 256);
	load1 = vec_ld(15, src);
	load2 = vec_ld(31, src);
	load3 = vec_ld(47, src);
	load0 = vec_perm(load0, load1, alignLoads);
	load1 = vec_perm(load1, load2, alignLoads);
	load2 = vec_perm(load2, load3, alignLoads);
	
	// unpack to 32 bit big-endian ints, high-aligned
	ext0 = (VecS32)vec_perm(load0, load0, perm1);
	ext1 = (VecS32)vec_perm(load0, load1, perm2);
	ext2 = (VecS32)vec_perm(load2, load1, perm3);
	ext3 = (VecS32)vec_perm(load2, load2, perm4);
	// shift right to get rid of 8 low bits of garbage, get sign extension in high 8 bits
	ext0 = vec_sra(ext0, shift);
	ext1 = vec_sra(ext1, shift);
	ext2 = vec_sra(ext2, shift);
	ext3 = vec_sra(ext3, shift);
	// convert to float
	conv0 = vec_ctf(ext0, 23);
	conv1 = vec_ctf(ext1, 23);
	conv2 = vec_ctf(ext2, 23);
	conv3 = vec_ctf(ext3, 23);
	
	curStore = vec_perm(conv0, conv0, alignStores);
	
	LEADING_PARTIAL_VECTOR_STORE(curStore, storeIndex, dst);

	curStore = vec_perm(conv0, conv1, alignStores);
	vec_st((VecU8)curStore, storeIndex, dst);
	storeIndex += 16;

	curStore = vec_perm(conv1, conv2, alignStores);
	vec_st((VecU8)curStore, storeIndex, dst);
	storeIndex += 16;

	curStore = vec_perm(conv2, conv3, alignStores);
	vec_st((VecU8)curStore, storeIndex, dst);
	storeIndex += 16;

	// setup for next loop iteration.
	// do not increment the store pointers; that is handled by storeIndex.
	// do need to increment the load pointers. the loads lead the stores.
	src += 3 * sizeof(VecU8);
	
	// count tracks how many source samples have been loaded
	count -= 16;
	
	// recycle
	load0 = load3;
	storeForward = conv3;
	int niter = count / 32;
	UInt8 *dst2 = dst + storeIndex;
	count -= niter * 32;
	storeIndex += niter * 128;
	
	// unrolled vector loop, 32 samples at a time
	for ( ; --niter >= 0; ) {
		VecU8 load4, load5, load6;
		VecS32 ext4, ext5, ext6, ext7;
		VecFloat conv4, conv5, conv6, conv7, conv8;
		const UInt8 *srcAhead = src + 256;

		// load
		load1 = vec_ld(15, src);
		__builtin_prefetch(srcAhead);
		load2 = vec_ld(31, src);
		load3 = vec_ld(47, src);
		load4 = vec_ld(63, src);
		load5 = vec_ld(79, src);
		load6 = vec_ld(95, src);

		// align
		load0 = vec_perm(load0, load1, alignLoads);
		load1 = vec_perm(load1, load2, alignLoads);
		load2 = vec_perm(load2, load3, alignLoads);
		load3 = vec_perm(load3, load4, alignLoads);
		load4 = vec_perm(load4, load5, alignLoads);
		load5 = vec_perm(load5, load6, alignLoads);
		
		// unpack to 32 bit big-endian ints, high-aligned
		ext0 = (VecS32)vec_perm(load0, load0, perm1);
		ext1 = (VecS32)vec_perm(load0, load1, perm2);
		ext2 = (VecS32)vec_perm(load2, load1, perm3);
		ext3 = (VecS32)vec_perm(load2, load2, perm4);

		ext4 = (VecS32)vec_perm(load3, load3, perm1);
		ext5 = (VecS32)vec_perm(load3, load4, perm2);
		ext6 = (VecS32)vec_perm(load5, load4, perm3);
		ext7 = (VecS32)vec_perm(load5, load6, perm4);

		// shift right to get rid of 8 low bits of garbage, get sign extension in high 8 bits
		ext0 = vec_sra(ext0, shift);
		ext1 = vec_sra(ext1, shift);
		ext2 = vec_sra(ext2, shift);
		ext3 = vec_sra(ext3, shift);
		ext4 = vec_sra(ext4, shift);
		ext5 = vec_sra(ext5, shift);
		ext6 = vec_sra(ext6, shift);
		ext7 = vec_sra(ext7, shift);

		// convert to float
		conv1 = vec_ctf(ext0, 23);
		conv2 = vec_ctf(ext1, 23);
		conv3 = vec_ctf(ext2, 23);
		conv4 = vec_ctf(ext3, 23);
		conv5 = vec_ctf(ext4, 23);
		conv6 = vec_ctf(ext5, 23);
		conv7 = vec_ctf(ext6, 23);
		conv8 = vec_ctf(ext7, 23);
		
		conv0 = vec_perm(storeForward, conv1, alignStores);
		conv1 = vec_perm(conv1, conv2, alignStores);
		conv2 = vec_perm(conv2, conv3, alignStores);
		conv3 = vec_perm(conv3, conv4, alignStores);
		conv4 = vec_perm(conv4, conv5, alignStores);
		conv5 = vec_perm(conv5, conv6, alignStores);
		conv6 = vec_perm(conv6, conv7, alignStores);
		conv7 = vec_perm(conv7, conv8, alignStores);
		
		// store
		vec_st((VecU8)conv0, 0, dst2);
		vec_st((VecU8)conv1, 16, dst2);
		vec_st((VecU8)conv2, 32, dst2);
		vec_st((VecU8)conv3, 48, dst2);
		vec_st((VecU8)conv4, 64, dst2);
		vec_st((VecU8)conv5, 80, dst2);
		vec_st((VecU8)conv6, 96, dst2);
		vec_st((VecU8)conv7, 112, dst2);
		
		// recycle and advance
		load0 = load6;
		storeForward = conv8;
		
		src += 6 * sizeof(VecU8);
		dst2 += 128;
	}

	// cleanup loop: 16 samples at a time, with bounds checking
	unsigned int endStoreVecBound = endStoreIndex - 16;
	for ( ; count > 0; count -= 16) {
		// load
		if (15+src < srcEnd) {
			load1 = vec_ld(15, src);
			if (31+src < srcEnd) {
				load2 = vec_ld(31, src);
				if (47+src < srcEnd) {
					load3 = vec_ld(47, src);
				}
			}
		}
		// align
		load0 = vec_perm(load0, load1, alignLoads);
		load1 = vec_perm(load1, load2, alignLoads);
		load2 = vec_perm(load2, load3, alignLoads);

		// convert
		ext0 = (VecS32)vec_perm(load0, load0, perm1);
		ext1 = (VecS32)vec_perm(load0, load1, perm2);
		ext2 = (VecS32)vec_perm(load2, load1, perm3);
		ext3 = (VecS32)vec_perm(load2, load2, perm4);

		ext0 = vec_sra(ext0, shift);
		ext1 = vec_sra(ext1, shift);
		ext2 = vec_sra(ext2, shift);
		ext3 = vec_sra(ext3, shift);

		// convert to float
		conv0 = vec_ctf(ext0, 23);
		conv1 = vec_ctf(ext1, 23);
		conv2 = vec_ctf(ext2, 23);
		conv3 = vec_ctf(ext3, 23);
		
		curStore = vec_perm(storeForward, conv0, alignStores);
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
		
		curStore = vec_perm(conv0, conv1, alignStores);
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
		
		curStore = vec_perm(conv1, conv2, alignStores);
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
		
		curStore = vec_perm(conv2, conv3, alignStores);
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)curStore, storeIndex, dst);
		storeIndex += 16;
		
		load0 = load3;
		storeForward = conv3;
		src += 3 * sizeof(VecS32);
	}
	curStore = vec_perm(storeForward, storeForward, alignStores);

storeLeftovers:
	while (storeIndex < endStoreIndex) {
		vec_ste((VecFloat)curStore, storeIndex, (Float32 *)dst);
		storeIndex += 4;
	}
}

void	Int32ToFloat32_Altivec(const SInt32 *isrc, Float32 *fdst, unsigned int numToConvert, int bigEndian)
{
	int count = numToConvert;	// needs to be signed
	const UInt8 *src = (UInt8 *)isrc;
	const UInt8 *srcEnd = src + count * sizeof(SInt32);
	srcEnd = (UInt8 *)( ((uintptr_t)srcEnd + 0xF) & ~0xF );	// round up to vector boundary; this is how far we can safely read
	UInt8 *dst = (UInt8 *)fdst;
	unsigned int endStoreIndex = 4 * count, storeIndex;	// how many bytes have been stored to dst (which is not advanced);
	int niter;
	
	VecS32 load0, load1, loadx;
	VecFloat conv0;
	
	// load 2 vectors
	load0 = (VecS32)vec_ld(0, src);
	__builtin_prefetch(src + 128);
	__builtin_prefetch(src + 256);
	load1 = (VecS32)vec_ld(15, src);
	
	/*
		Permute vectors:
		
		f / i	0	4	8	C
		0		10	0C	08	04
		4		04	00	-4	-8
		8		08	04	00	-4
		C		0C	08	04	00
	*/
	int srcAlignUp = ((uintptr_t)(src - 1) & 0xF) + 1;	// 4,8,C,10
	int dstAlign = (uintptr_t)dst & 0xF;
	int shift = srcAlignUp - dstAlign;
	VecU8 storePerm;
	
	if (shift < 0) {
		// what's special about these cases is that there is more in store0 than can be stored initially 
		storePerm = (VecU8)vec_lvsl(shift, (unsigned char *)0);	// -2-0D -> 12->0D
		if (!bigEndian) {
			VecU8 swap = (VecU8)(	0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04,
									0x0B, 0x0A, 0x09, 0x08, 0x0F, 0x0E, 0x0D, 0x0C);
			storePerm = vec_perm(storePerm, storePerm, swap);
		}
		loadx = vec_perm(load0, load0, storePerm);
	} else {
		if (shift == 16)
			storePerm = vec_lvsr(shift, (unsigned char *)0);	// 10..1F
		else
			storePerm = vec_lvsl(shift, (unsigned char *)0); // shift..shift+F
		if (!bigEndian) {
			VecU8 swap = (VecU8)(	0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04,
									0x0B, 0x0A, 0x09, 0x08, 0x0F, 0x0E, 0x0D, 0x0C);
			storePerm = vec_perm(storePerm, storePerm, swap);
		}
		loadx = vec_perm(load0, load1, storePerm);
	}
	conv0 = vec_ctf(loadx, 31);
	
	// store the first part of the result vector, up to the first 16-byte aligned boundary
	LEADING_PARTIAL_VECTOR_STORE_4((VecU8)conv0, storeIndex, dst);
	
	if (shift < 0) {
		// the special case again
		// here we have an entire second vector to store
		load0 = vec_perm(load0, load1, storePerm);
		conv0 = vec_ctf(load0, 31);
		vec_st((VecU8)conv0, storeIndex, dst);
		storeIndex += 16;
	}

	// setup for next loop iteration.
	// do not increment the store pointers; that is handled by storeIndex.
	// do need to increment the load pointers. the loads lead the stores.
	src += sizeof(VecS32);
	
	// count tracks how many source samples have been loaded
	count -= 4;
	
	// recycle
	load0 = load1;
	
	// unrolled loop
	// for edge cases where shift is negative -- don't write too far
	niter = minint(count / 32, (endStoreIndex - storeIndex) / 128);
	count -= niter * 32;
	UInt8 *dst2 = (UInt8 *)dst + storeIndex;
	storeIndex += niter * 128;
	
	for ( ; --niter >= 0; ) {
		VecS32 load2, load3, load4, load5, load6, load7, load8;
		VecFloat conv1, conv2, conv3, conv4, conv5, conv6, conv7;
		const UInt8 *srcAhead = src + 256;

		// load
		load1 = (VecS32)vec_ld(15, src);
		__builtin_prefetch(srcAhead);
		load2 = (VecS32)vec_ld(31, src);
		load3 = (VecS32)vec_ld(47, src);
		load4 = (VecS32)vec_ld(63, src);
		load5 = (VecS32)vec_ld(79, src);
		load6 = (VecS32)vec_ld(95, src);
		load7 = (VecS32)vec_ld(111, src);
		load8 = (VecS32)vec_ld(127, src);
		
		// permute and convert to float
		load0 = vec_perm(load0, load1, storePerm);
		load1 = vec_perm(load1, load2, storePerm);
		load2 = vec_perm(load2, load3, storePerm);
		load3 = vec_perm(load3, load4, storePerm);
		load4 = vec_perm(load4, load5, storePerm);
		load5 = vec_perm(load5, load6, storePerm);
		load6 = vec_perm(load6, load7, storePerm);
		load7 = vec_perm(load7, load8, storePerm);
		
		conv0 = vec_ctf(load0, 31);
		conv1 = vec_ctf(load1, 31);
		conv2 = vec_ctf(load2, 31);
		conv3 = vec_ctf(load3, 31);
		conv4 = vec_ctf(load4, 31);
		conv5 = vec_ctf(load5, 31);
		conv6 = vec_ctf(load6, 31);
		conv7 = vec_ctf(load7, 31);
		
		// store
		vec_st((VecU8)conv0, 0, dst2);
		vec_st((VecU8)conv1, 16, dst2);
		vec_st((VecU8)conv2, 32, dst2);
		vec_st((VecU8)conv3, 48, dst2);
		vec_st((VecU8)conv4, 64, dst2);
		vec_st((VecU8)conv5, 80, dst2);
		vec_st((VecU8)conv6, 96, dst2);
		vec_st((VecU8)conv7, 112, dst2);
		
		src += 8 * sizeof(VecS32);
		load0 = load8;
		dst2 += 128;
	}
	
	unsigned int endStoreVecBound = endStoreIndex - 16;

	// cleanup loop: 4 samples at a time, with bounds checking
	for ( ; count > 0; count -= 4) {
		if (src+15 < srcEnd) {
			load1 = (VecS32)vec_ld(15, src);
		}
		load0 = vec_perm(load0, load1, storePerm);
		conv0 = vec_ctf(load0, 31);
		if (storeIndex > endStoreVecBound)
			goto storeLeftovers;
		vec_st((VecU8)conv0, storeIndex, dst);
		storeIndex += 16;
		
		src += sizeof(VecS32);
		load0 = load1;
	}
	load0 = vec_perm(load0, load0, storePerm);
	conv0 = vec_ctf(load0, 31);

storeLeftovers:
	while (storeIndex < endStoreIndex) {
		vec_ste((VecFloat)conv0, storeIndex, (Float32 *)dst);
		storeIndex += 4;
	}
}


// ==========================================================================================

#define __lhbrx(base, offset) \
	({ register UInt32 result; \
	__asm__ ("lhbrx %0, %2, %1" : "=r" (result) : "r"  (base), "bO" (offset)); \
	/*return*/ result; })

// Deinterleave stereo int16 to float32
void	DeinterleaveInt16ToFloat32_Altivec(const SInt16 *isrc, Float32 *dstA, Float32 *dstB, unsigned int numFrames, int bigEndian)
{
	if (numFrames < 16) {
		// scalar
		register Float32 bias;
		register SInt32 exponentMask = (0x87UL << 23) | 0x8000;	//FP exponent + bias for sign
		union
		{
			Float32	f;
			SInt32	i;
		} buffer0;

		buffer0.i = exponentMask;
		bias = buffer0.f;

		unsigned int i;
		if (bigEndian) {
			--isrc;
			for (i = 0; i < numFrames; ++i) {
				SInt32 ia = *++isrc;
				SInt32 ib = *++isrc;
				ia += exponentMask;
				ib += exponentMask;
				((SInt32 *)dstA)[0] = ia;
				((SInt32 *)dstB)[0] = ib;
				dstA[0] -= bias;
				dstB[0] -= bias;
				++dstA, ++dstB;
			}
		} else {
			for (i = 0; i < numFrames; ++i) {
				SInt32 ia = __lhbrx(isrc++, 0);
				SInt32 ib = __lhbrx(isrc++, 0);
				ia += exponentMask;
				ib += exponentMask;
				((SInt32 *)dstA)[0] = ia;
				((SInt32 *)dstB)[0] = ib;
				dstA[0] -= bias;
				dstB[0] -= bias;
				++dstA, ++dstB;
			}
		}
		return;
	}
	int count = numFrames * 2;	// needs to be signed
	const UInt8 *src = (UInt8 *)isrc;
	const UInt8 *srcEnd = src + 2 * count;
	srcEnd = (UInt8 *)( ((uintptr_t)srcEnd + 0xF) & ~0xF );	// round up to vector boundary; this is how far we can safely read
	unsigned int endStoreIndex = 4 * numFrames, storeIndexA, storeIndexB;	// how many bytes have been stored to dst (which is not advanced);

	VecS16 load0, load1, load2, load3, load4;
	VecS32 ext0A, ext0B, ext1A, ext1B, ext2A, ext2B, ext3A, ext3B;
	VecFloat conv0A, conv0B, conv1A, conv1B, conv2A, conv2B, conv3A, conv3B, curStoreA, curStoreB, storeForwardA, storeForwardB;
	VecU8 one = vec_splat_u8(1);
	VecU8 alignLoads = vec_add( vec_lvsl(-1L, src), one);
		//same as lvsl(0,a), except 16 byte aligned case where we get { 16, 17, 18, ...}
	VecU8 alignStoresA = vec_lvsr(0, dstA);
	VecU8 alignStoresB = vec_lvsr(0, dstB);
	VecS16 zero = vec_splat_s16(0);
	VecU8 deinterPermA, deinterPermB;
	
	// these copy the high 16 bits of 4 of the 8 samples; the 1f's come from "zero"
	if (bigEndian) {
		deinterPermA = (VecU8)(	0x00, 0x01, 0x1f, 0x1f, 0x04, 0x05, 0x1f, 0x1f,
								0x08, 0x09, 0x1f, 0x1f, 0x0c, 0x0d, 0x1f, 0x1f );
		deinterPermB = (VecU8)(	0x02, 0x03, 0x1f, 0x1f, 0x06, 0x07, 0x1f, 0x1f,
								0x0a, 0x0b, 0x1f, 0x1f, 0x0e, 0x0f, 0x1f, 0x1f );
	} else {
		deinterPermA = (VecU8)(	0x01, 0x00, 0x1f, 0x1f, 0x05, 0x04, 0x1f, 0x1f,
								0x09, 0x08, 0x1f, 0x1f, 0x0d, 0x0c, 0x1f, 0x1f );
		deinterPermB = (VecU8)(	0x03, 0x02, 0x1f, 0x1f, 0x07, 0x06, 0x1f, 0x1f,
								0x0b, 0x0a, 0x1f, 0x1f, 0x0f, 0x0e, 0x1f, 0x1f );
	}

	// load 2 vectors of int16s and align into load0
	load0 = (VecS16)vec_ld(0, src);
	__builtin_prefetch(src + 128);
	__builtin_prefetch(src + 256);
	load1 = (VecS16)vec_ld(15, src);
	load0 = vec_perm(load0, load1, alignLoads);
	
	// expand/deinterleave to 2 vectors of 32-bit ints
	ext0A = (VecS32)vec_perm(load0, zero, deinterPermA);
	ext0B = (VecS32)vec_perm(load0, zero, deinterPermB);
	
	// convert to float
	conv0A = vec_ctf(ext0A, 31);
	conv0B = vec_ctf(ext0B, 31);
	
	curStoreA = vec_perm(conv0A, conv0A, alignStoresA);
	curStoreB = vec_perm(conv0B, conv0B, alignStoresB);
	
	LEADING_PARTIAL_VECTOR_STORE_4((VecU8)curStoreA, storeIndexA, dstA);
	LEADING_PARTIAL_VECTOR_STORE_4((VecU8)curStoreB, storeIndexB, dstB);

	// setup for next loop iteration.
	// do not increment the store pointers; that is handled by storeIndex.
	// do need to increment the load pointers. the loads lead the stores.
	src += sizeof(VecS16);
	
	// count tracks how many source samples have been loaded
	count -= 8;
	
	// recycle
	load0 = load1;
	storeForwardA = conv0A;
	storeForwardB = conv0B;
	
	// unrolled vector loop: 32->4*8 samples at a time
	for ( ; count >= 32; count -= 32) {
		const UInt8 *srcAhead = src + 256;
		// load
		load1 = (VecS16)vec_ld(15, src);
		__builtin_prefetch(srcAhead);
		load2 = (VecS16)vec_ld(31, src);
		load3 = (VecS16)vec_ld(47, src);
		load4 = (VecS16)vec_ld(63, src);
		
		// align
		load0 = vec_perm(load0, load1, alignLoads);
		load1 = vec_perm(load1, load2, alignLoads);
		load2 = vec_perm(load2, load3, alignLoads);
		load3 = vec_perm(load3, load4, alignLoads);
		
		// expand/deinterleave
		ext0A = (VecS32)vec_perm(load0, zero, deinterPermA);
		ext0B = (VecS32)vec_perm(load0, zero, deinterPermB);
		ext1A = (VecS32)vec_perm(load1, zero, deinterPermA);
		ext1B = (VecS32)vec_perm(load1, zero, deinterPermB);
		ext2A = (VecS32)vec_perm(load2, zero, deinterPermA);
		ext2B = (VecS32)vec_perm(load2, zero, deinterPermB);
		ext3A = (VecS32)vec_perm(load3, zero, deinterPermA);
		ext3B = (VecS32)vec_perm(load3, zero, deinterPermB);
		
		// convert to float
		conv0A = vec_ctf(ext0A, 31);
		conv0B = vec_ctf(ext0B, 31);
		conv1A = vec_ctf(ext1A, 31);
		conv1B = vec_ctf(ext1B, 31);
		conv2A = vec_ctf(ext2A, 31);
		conv2B = vec_ctf(ext2B, 31);
		conv3A = vec_ctf(ext3A, 31);
		conv3B = vec_ctf(ext3B, 31);
		
		// align and store
		curStoreA = vec_perm(storeForwardA, conv0A, alignStoresA);
		curStoreB = vec_perm(storeForwardB, conv0B, alignStoresB);

		conv0A = vec_perm(conv0A, conv1A, alignStoresA);
		conv0B = vec_perm(conv0B, conv1B, alignStoresB);

		conv1A = vec_perm(conv1A, conv2A, alignStoresA);
		conv1B = vec_perm(conv1B, conv2B, alignStoresB);

		conv2A = vec_perm(conv2A, conv3A, alignStoresA);
		conv2B = vec_perm(conv2B, conv3B, alignStoresB);

		vec_st(curStoreA, storeIndexA, dstA);
		storeIndexA += 16;
		vec_st(curStoreB, storeIndexB, dstB);
		storeIndexB += 16;

		vec_st(conv0A, storeIndexA, dstA);
		storeIndexA += 16;
		vec_st(conv0B, storeIndexB, dstB);
		storeIndexB += 16;

		vec_st(conv1A, storeIndexA, dstA);
		storeIndexA += 16;
		vec_st(conv1B, storeIndexB, dstB);
		storeIndexB += 16;

		vec_st(conv2A, storeIndexA, dstA);
		storeIndexA += 16;
		vec_st(conv2B, storeIndexB, dstB);
		storeIndexB += 16;
		
		// recycle/advance
		src += 4 * sizeof(VecS16);
		load0 = load4;
		storeForwardA = conv3A;
		storeForwardB = conv3B;
	}
	
	// cleanup loop: 8->2*4 samples at a time, with bounds checking
	unsigned int endStoreVecBound = endStoreIndex - 16;
	int haveStoreA = 0, haveStoreB = 0;
	for ( ; count > 0; count -= 8) {
		// load
		if (15+src < srcEnd)
			load1 = (VecS16)vec_ld(15, src);

		// align
		load0 = vec_perm(load0, load1, alignLoads);
		
		// expand/deinterleave to 2 vectors of 32-bit ints
		ext0A = (VecS32)vec_perm(load0, zero, deinterPermA);
		ext0B = (VecS32)vec_perm(load0, zero, deinterPermB);
		
		// convert to float
		conv0A = vec_ctf(ext0A, 31);
		conv0B = vec_ctf(ext0B, 31);
		
		curStoreA = vec_perm(storeForwardA, conv0A, alignStoresA);
		curStoreB = vec_perm(storeForwardB, conv0B, alignStoresB);
		if (storeIndexA <= endStoreVecBound) {
			vec_st(curStoreA, storeIndexA, dstA);
			storeIndexA += 16;
		} else haveStoreA = 1;
		if (storeIndexB <= endStoreVecBound) {
			vec_st(curStoreB, storeIndexB, dstB);
			storeIndexB += 16;
		} else haveStoreB = 1;
		
		// recycle
		src += sizeof(VecS16);
		load0 = load1;
		storeForwardA = conv0A;
		storeForwardB = conv0B;
	}
	if (!haveStoreA) curStoreA = vec_perm(storeForwardA, storeForwardA, alignStoresA);
	if (!haveStoreB) curStoreB = vec_perm(storeForwardB, storeForwardB, alignStoresB);
	
	while (storeIndexA < endStoreIndex) {
		vec_ste((VecFloat)curStoreA, storeIndexA, (Float32 *)dstA);
		storeIndexA += 4;
	}
	while (storeIndexB < endStoreIndex) {
		vec_ste((VecFloat)curStoreB, storeIndexB, (Float32 *)dstB);
		storeIndexB += 4;
	}
}

// This requires that all pointers are vector-aligned
void	Interleave32_Altivec(const void *vsrcA, const void *vsrcB, void *vdst, unsigned int numFrames)
{
	int count = numFrames;
	SInt32 *srcA = (SInt32 *)vsrcA, *srcB = (SInt32 *)vsrcB, *dst = (SInt32 *)vdst;
	VecS32 load0A, load0B, load1A, load1B, load2A, load2B, store0, store1, store2, store3, store4, store5;
	
	__builtin_prefetch(srcA + 32);
	__builtin_prefetch(srcB + 32);

	VecU8 interPerm0 = (VecU8)(		0x00, 0x01, 0x02, 0x03,	0x10, 0x11, 0x12, 0x13, 
									0x04, 0x05, 0x06, 0x07,	0x14, 0x15, 0x16, 0x17 );
	VecU8 interPerm1 = (VecU8)(		0x08, 0x09, 0x0a, 0x0b, 0x18, 0x19, 0x1a, 0x1b,
									0x0c, 0x0d, 0x0e, 0x0f, 0x1c, 0x1d, 0x1e, 0x1f );
	
	for ( ; count >= 12; count -= 12) {
		load0A = vec_ld(0, srcA);
		__builtin_prefetch(srcA + 64);
		__builtin_prefetch(srcB + 64);
		load0B = vec_ld(0, srcB);
		load1A = vec_ld(16, srcA);
		load1B = vec_ld(16, srcB);
		load2A = vec_ld(32, srcA);
		load2B = vec_ld(32, srcB);
		
		store0 = vec_perm(load0A, load0B, interPerm0);
		store1 = vec_perm(load0A, load0B, interPerm1);
		store2 = vec_perm(load1A, load1B, interPerm0);
		store3 = vec_perm(load1A, load1B, interPerm1);
		store4 = vec_perm(load2A, load2B, interPerm0);
		store5 = vec_perm(load2A, load2B, interPerm1);
		
		vec_st(store0, 0, dst);
		vec_st(store1, 16, dst);
		vec_st(store2, 32, dst);
		vec_st(store3, 48, dst);
		vec_st(store4, 64, dst);
		vec_st(store5, 80, dst);
		
		srcA += 12;
		srcB += 12;
		dst += 24;
	}

	while (count--) {
		SInt32 a = *srcA++;
		SInt32 b = *srcB++;
		
		*dst++ = a;
		*dst++ = b;
	}
}

#if 0	// UNTESTED
// This requires that all pointers are vector-aligned
void	Deinterleave32_Altivec(const void *vsrc, void *vdstA, void *vdstB, unsigned int numFrames)
{
	int count = numFrames;
	SInt32 *src = (SInt32 *)vsrc, *dstA = (SInt32 *)vdstA, *dstB = (SInt32 *)vdstB;
	VecS32 load0, load1, load2, load3, store0A, store0B, store1A, store1B;
	
	__builtin_prefetch(src + 32);

	VecU8 deinterPermA = (VecU8)(	0x00, 0x01, 0x02, 0x03,	0x08, 0x09, 0x0a, 0x0b,
									0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1a, 0x1b );
	VecU8 deinterPermB = (VecU8)(	0x04, 0x05, 0x06, 0x07,	0x0c, 0x0d, 0x0e, 0x0f,
									0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f );
	
	for ( ; count >= 8; count -= 8) {
		load0 = vec_ld(0, src);
		__builtin_prefetch(src + 64);
		load1 = vec_ld(16, src);
		load2 = vec_ld(32, src);
		load3 = vec_ld(48, src);
		
		store0A = vec_perm(load0, load1, deinterPermA);
		store0B = vec_perm(load0, load1, deinterPermB);
		store1A = vec_perm(load2, load3, deinterPermA);
		store1B = vec_perm(load2, load3, deinterPermB);
		
		vec_st(store0A, 0, dstA);
		vec_st(store0B, 0, dstB);
		vec_st(store1A, 16, dstA);
		vec_st(store1B, 16, dstB);
		
		src += 16;
		dstA += 8;
		dstB += 8;
	}

	while (count--) {
		SInt32 srcA = src[0];
		SInt32 srcB = src[1];	src += 2;
		
		*dstA++ = srcA;
		*dstB++ = srcB;
	}
}
#endif // 0

#endif // TARGET_CPU_PPC
