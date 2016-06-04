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
	PCMBlitterLibDispatch.h
	
=============================================================================*/

#ifndef __PCMBlitterLibDispatch_h__
#define __PCMBlitterLibDispatch_h__

#include "PCMBlitterLib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	PCM int<->float library.

	These are the high-level interfaces which dispatch to (often processor-specific) optimized routines.
	Avoid calling the lower-level routines directly; they are subject to renaming etc.
	
	There are two sets of interfaces:
	[1] integer formats are either "native" or "swap"
	[2] integer formats are "BE" or "LE", signifying big or little endian. These are simply macros for the other functions.
	
	All floating point numbers are 32-bit native-endian.
	Supports 16, 24, and 32-bit integers, big and little endian.
	
	32-bit floats and ints must be 4-byte aligned.
	24-bit samples have no alignment requirements.
	16-bit ints must be 2-byte aligned.
	
	On Intel, the haveVector argument is ignored and some implementations assume SSE2.
*/

// ____________________________________________________________________________________
// utility macros

#if TARGET_CPU_PPC
#pragma mark -
#pragma mark PPC Altivec macros
#define PCMBlit_BufferAligned(ptr, size)	( (((uintptr_t)ptr) & (size-1)) == 0 )
	// size must be a power of 2

// use Altivec when possible
#define PCMBlit_F2IAltivecOK(intalign) \
	(haveVector && count >= 16 && PCMBlit_BufferAligned(src, 4) && PCMBlit_BufferAligned(dest, intalign))

#define PCMBlit_I2FAltivecOK(intalign) \
	(haveVector && count >= 16 && PCMBlit_BufferAligned(dest, 4) && PCMBlit_BufferAligned(src, intalign))
#endif

// ____________________________________________________________________________________
// Inline functions to choose between scalar and Altivec versions on PPC.

#if TARGET_OS_MAC
#pragma mark -
#pragma mark MacOS

inline void NativeInt16ToFloat32( int haveVector, const SInt16 *src, Float32 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_I2FAltivecOK(2))
		Int16ToFloat32_Altivec(src, dest, count, true);
	else
		NativeInt16ToFloat32_Scalar(src, dest, count);
#elif TARGET_CPU_X86
	NativeInt16ToFloat32_X86(src, dest, count);
#endif
}

inline void SwapInt16ToFloat32( int haveVector, const SInt16 *src, Float32 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_I2FAltivecOK(2))
		Int16ToFloat32_Altivec(src, dest, count, false);
	else
		SwapInt16ToFloat32_Scalar(src, dest, count);
#elif TARGET_CPU_X86
	SwapInt16ToFloat32_X86(src, dest, count);
#endif
}

inline void NativeInt24ToFloat32( int haveVector, const UInt8 *src, Float32 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_I2FAltivecOK(1))
		Int24ToFloat32_Altivec(src, dest, count, true);
	else
		NativeInt24ToFloat32_Scalar((const SInt32 *)src, dest, count);
#elif TARGET_CPU_X86
	NativeInt24ToFloat32_Portable(src, dest, count);
#endif
}

inline void SwapInt24ToFloat32( int haveVector, const UInt8 *src, Float32 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_I2FAltivecOK(1))
		Int24ToFloat32_Altivec(src, dest, count, false);
	else
		SwapInt24ToFloat32_Scalar((const SInt32 *)src, dest, count);
#elif TARGET_CPU_X86
	SwapInt24ToFloat32_Portable(src, dest, count);
#endif
}

inline void NativeInt32ToFloat32( int haveVector, const SInt32 *src, Float32 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_I2FAltivecOK(4))
		Int32ToFloat32_Altivec(src, dest, count, true);
	else
		NativeInt32ToFloat32_Scalar(src, dest, count);
#elif TARGET_CPU_X86
	NativeInt32ToFloat32_X86(src, dest, count);
#endif
}

inline void SwapInt32ToFloat32( int haveVector, const SInt32 *src, Float32 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_I2FAltivecOK(4))
		Int32ToFloat32_Altivec(src, dest, count, false);
	else
		SwapInt32ToFloat32_Scalar(src, dest, count);
#elif TARGET_CPU_X86
	SwapInt32ToFloat32_X86(src, dest, count);
#endif
}


inline void Float32ToNativeInt16( int haveVector, const Float32 *src, SInt16 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_F2IAltivecOK(2))
		Float32ToInt16_Altivec(src, dest, count, true);
	else
		Float32ToNativeInt16_Scalar(src, dest, count);
#elif TARGET_CPU_X86
	Float32ToNativeInt16_X86(src, dest, count);
#endif
}

inline void Float32ToSwapInt16( int haveVector, const Float32 *src, SInt16 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_F2IAltivecOK(2))
		Float32ToInt16_Altivec(src, dest, count, false);
	else
		Float32ToSwapInt16_Scalar(src, dest, count);
#elif TARGET_CPU_X86
	Float32ToSwapInt16_X86(src, dest, count);
#endif
}

inline void Float32ToNativeInt24( int haveVector, const Float32 *src, UInt8 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_F2IAltivecOK(1))
		Float32ToInt24_Altivec(src, dest, count, true);
	else
		Float32ToNativeInt24_Scalar(src, (SInt32 *)dest, count);
#elif TARGET_CPU_X86
	Float32ToNativeInt24_X86(src, dest, count);
#endif
}

inline void Float32ToSwapInt24( int haveVector, const Float32 *src, UInt8 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_F2IAltivecOK(1))
		Float32ToInt24_Altivec(src, dest, count, false);
	else
		Float32ToSwapInt24_Scalar(src, (SInt32 *)dest, count);
#elif TARGET_CPU_X86
	Float32ToSwapInt24_Portable(src, dest, count);
#endif
}

inline void Float32ToNativeInt32( int haveVector, const Float32 *src, SInt32 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_F2IAltivecOK(4))
		Float32ToInt32_Altivec(src, dest, count, true);
	else
		Float32ToNativeInt32_Scalar(src, dest, count);
#elif TARGET_CPU_X86
	Float32ToNativeInt32_X86(src, dest, count);
#endif
}

inline void Float32ToSwapInt32( int haveVector, const Float32 *src, SInt32 *dest, unsigned int count )
{
#if TARGET_CPU_PPC
	if (PCMBlit_F2IAltivecOK(4))
		Float32ToInt32_Altivec(src, dest, count, false);
	else
		Float32ToSwapInt32_Scalar(src, dest, count);
#elif TARGET_CPU_X86
	Float32ToSwapInt32_X86(src, dest, count);
#endif
}

#else // !TARGET_OS_MAC
#pragma mark -
#pragma mark Not MacOS
inline void Float32ToNativeInt16( int haveVector, const Float32 *src, SInt16 *dest, unsigned int count )
{
	Float32ToNativeInt16_Portable(src, dest, count);
}

inline void Float32ToSwapInt16( int haveVector, const Float32 *src, SInt16 *dest, unsigned int count )
{
	Float32ToSwapInt16_Portable(src, dest, count);
}

inline void NativeInt16ToFloat32( int haveVector, const SInt16 *src, Float32 *dest, unsigned int count )
{
	NativeInt16ToFloat32_Portable(src, dest, count);
}

inline void SwapInt16ToFloat32( int haveVector, const SInt16 *src, Float32 *dest, unsigned int count )
{
	SwapInt16ToFloat32_Portable(src, dest, count);
}

inline void Float32ToNativeInt24( int haveVector, const Float32 *src, UInt8 *dest, unsigned int count )
{
	Float32ToNativeInt24_Portable(src, dest, count);
}

inline void Float32ToSwapInt24( int haveVector, const Float32 *src, UInt8 *dest, unsigned int count )
{
	Float32ToSwapInt24_Portable(src, dest, count);
}

inline void NativeInt24ToFloat32( int haveVector, const UInt8 *src, Float32 *dest, unsigned int count )
{
	NativeInt24ToFloat32_Portable(src, dest, count);
}

inline void SwapInt24ToFloat32( int haveVector, const UInt8 *src, Float32 *dest, unsigned int count )
{
	SwapInt24ToFloat32_Portable(src, dest, count);
}

inline void Float32ToNativeInt32( int haveVector, const Float32 *src, SInt32 *dest, unsigned int count )
{
	Float32ToNativeInt32_Portable(src, dest, count);
}

inline void Float32ToSwapInt32( int haveVector, const Float32 *src, SInt32 *dest, unsigned int count )
{
	Float32ToSwapInt32_Portable(src, dest, count);
}

inline void NativeInt32ToFloat32( int haveVector, const SInt32 *src, Float32 *dest, unsigned int count )
{
	NativeInt32ToFloat32_Portable(src, dest, count);
}

inline void SwapInt32ToFloat32( int haveVector, const SInt32 *src, Float32 *dest, unsigned int count )
{
	SwapInt32ToFloat32_Portable(src, dest, count);
}

#endif

// Alternate names for the above: these explicitly specify the endianism of the integer format instead of "native"/"swap"
#pragma mark -
#pragma mark Alternate names
#if TARGET_RT_BIG_ENDIAN
	#define	BEInt16ToFloat32	NativeInt16ToFloat32
	#define	LEInt16ToFloat32	SwapInt16ToFloat32
	#define	BEInt24ToFloat32	NativeInt24ToFloat32
	#define	LEInt24ToFloat32	SwapInt24ToFloat32
	#define	BEInt32ToFloat32	NativeInt32ToFloat32
	#define	LEInt32ToFloat32	SwapInt32ToFloat32

	#define Float32ToBEInt16	Float32ToNativeInt16
	#define Float32ToLEInt16	Float32ToSwapInt16
	#define Float32ToBEInt24	Float32ToNativeInt24
	#define Float32ToLEInt24	Float32ToSwapInt24
	#define Float32ToBEInt32	Float32ToNativeInt32
	#define Float32ToLEInt32	Float32ToSwapInt32
#else
	#define	LEInt16ToFloat32	NativeInt16ToFloat32
	#define	BEInt16ToFloat32	SwapInt16ToFloat32
	#define	LEInt24ToFloat32	NativeInt24ToFloat32
	#define	BEInt24ToFloat32	SwapInt24ToFloat32
	#define	LEInt32ToFloat32	NativeInt32ToFloat32
	#define	BEInt32ToFloat32	SwapInt32ToFloat32

	#define Float32ToLEInt16	Float32ToNativeInt16
	#define Float32ToBEInt16	Float32ToSwapInt16
	#define Float32ToLEInt24	Float32ToNativeInt24
	#define Float32ToBEInt24	Float32ToSwapInt24
	#define Float32ToLEInt32	Float32ToNativeInt32
	#define Float32ToBEInt32	Float32ToSwapInt32
#endif

#ifdef __cplusplus
};
#endif

#endif // __PCMBlitterLibDispatch_h__
