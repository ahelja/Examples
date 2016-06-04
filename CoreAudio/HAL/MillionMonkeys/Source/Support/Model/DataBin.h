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
/* DataBin.h */

#ifndef __DATABIN_H__
#define __DATABIN_H__

#define FEEDER_THREAD_TRACE		0
#define HAL_IOPROC_TRACE		1
#define WORK_LOOP_TRACE			2

#include <CoreServices/../Frameworks/CarbonCore.framework/Headers/MacTypes.h>

#include "TraceBin.h"

// contained structure
typedef struct CycleData {
	// cycle time stamp
	UInt64		mHostTimeStamp;
	// number data
	SInt32		mWorkDoneInCycle;
	Float64		mWakeUpLatencyForHAL;
	Float64		mWakeUpLatencyForFeederThread;
	bool		mHALThreadDidOverload;
	bool		mHALThreadTraceDidOverload;
	bool		mFeederThreadDidOverloadLastCycle;
	bool		mFeederThreadNeverRan;
	bool		mFeederThreadTraceDidOverload;
	bool		mWorkLoopTraceDidOverload;
	
	// trace data pointers
	TraceBin *	mHALIOProcTraceStorage;
	TraceBin *	mFeederThreadTraceStorage;
	TraceBin *	mWorkLoopTrace;
} CycleData;

// containing structure
typedef struct DataBin {
	CycleData *				mCycles;
	UInt32					mCurrentCycle;
	UInt32					mNumCycles;
	bool					mCanDoTrace;
	
	bool					mDataBinDidWrap;
	bool					mDataBinDidWrapSticky;
} DataBin;

DataBin*	DataBinNew (UInt32 inNumCycles, bool inRequireTrace);
void		DataBinDealloc (DataBin* dataBinIn);
TraceBin*	DataBinGetCurrentCycleTraceMemory (DataBin* dataBinIn, TraceBinCollection* TBCin, int whichTrace);
TraceBin*	DataBinGetCycleTraceMemoryForIndex (DataBin* dataBinIn, UInt32 indexIn, TraceBinCollection* TBCin, int whichTrace);
void		DataBinWriteDataForCurrentCycle (	DataBin* dataBinIn, UInt64 inHostTimeStamp, Float64 inHALLatency, Float64 inFeederLatency,
												UInt32 inWorkDone, bool inHALOverloaded, bool inHALTraceOverloaded, bool inFTOverloaded,
												bool inFTNeverRan, bool inFTTraceOverloaded, bool inWLTraceOverloaded);
UInt32		DataBinNumCycles (DataBin* dataBinIn);
UInt32		DataBinCurrentIndex (DataBin* dataBinIn);
bool		DataBinWrapped (DataBin* dataBinIn);
bool		DataBinFull (DataBin* dataBinIn);

CycleData*	DataBinGetCurrentCycle (DataBin* dataBinIn);
CycleData*	DataBinGetCycleAtIndex (DataBin* dataBinIn, UInt32 index);
void		DataBinReset (DataBin* dataBinIn);

#endif	// __DATABIN_H__

/*
 */
