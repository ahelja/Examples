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
/* DataBin.c */

#include <stdlib.h>

#include "DataBin.h"

//#define DEBUGPRINT

DataBin*	DataBinNew (UInt32 inNumCycles, bool inRequireTrace)
{
	UInt32		i;
	DataBin*	newDataBin;
	
	newDataBin = malloc (sizeof (DataBin));
	
	newDataBin->mCurrentCycle = 0;
	newDataBin->mNumCycles = inNumCycles;
	newDataBin->mCanDoTrace = inRequireTrace;
	newDataBin->mDataBinDidWrap = false;
	newDataBin->mDataBinDidWrapSticky = false;
	        
	newDataBin->mCycles = (CycleData *) malloc(inNumCycles *  sizeof(CycleData));
	// set all bin pointers to NULL
    for (i = 0; i < inNumCycles; ++i) {
        newDataBin->mCycles[i].mFeederThreadTraceStorage = NULL;
        newDataBin->mCycles[i].mHALIOProcTraceStorage = NULL;
        newDataBin->mCycles[i].mWorkLoopTrace = NULL;
    }
	
	return newDataBin;
}

void		DataBinDealloc (DataBin* dataBinIn)
{
	free (dataBinIn->mCycles);
	free (dataBinIn);
}

TraceBin*	DataBinGetCurrentCycleTraceMemory (DataBin* dataBinIn, TraceBinCollection* TBCin, int whichTrace)
{
	return DataBinGetCycleTraceMemoryForIndex (dataBinIn, dataBinIn->mCurrentCycle, TBCin, whichTrace);
}

TraceBin*	DataBinGetCycleTraceMemoryForIndex (DataBin* dataBinIn, UInt32 indexIn, TraceBinCollection* TBCin, int whichTrace)
{
    TraceBin *	nextTraceBinMemory;
	
	nextTraceBinMemory = TraceBinCollectionGetNextTraceBin (TBCin);
	
	switch (whichTrace) {
		case FEEDER_THREAD_TRACE:
            dataBinIn->mCycles[indexIn].mFeederThreadTraceStorage = nextTraceBinMemory;
			return dataBinIn->mCycles[indexIn].mFeederThreadTraceStorage;
            break;
		case HAL_IOPROC_TRACE:
            dataBinIn->mCycles[indexIn].mHALIOProcTraceStorage = nextTraceBinMemory;
			return dataBinIn->mCycles[indexIn].mHALIOProcTraceStorage;
            break;
		case WORK_LOOP_TRACE:
            dataBinIn->mCycles[indexIn].mWorkLoopTrace = nextTraceBinMemory;
			return dataBinIn->mCycles[indexIn].mWorkLoopTrace;
            break;
	}
	
	return NULL;
}



void		DataBinWriteDataForCurrentCycle (	DataBin* dataBinIn, UInt64 inHostTimeStamp, Float64 inHALLatency, Float64 inFeederLatency,
												UInt32 inWorkDone, bool inHALOverloaded, bool inHALTraceOverloaded, bool inFTOverloaded,
												bool inFTNeverRan, bool inFTTraceOverloaded, bool inWLTraceOverloaded)
{
	dataBinIn->mCycles[dataBinIn->mCurrentCycle].mHostTimeStamp = inHostTimeStamp;
	dataBinIn->mCycles[dataBinIn->mCurrentCycle].mWakeUpLatencyForHAL = inHALLatency;
	dataBinIn->mCycles[dataBinIn->mCurrentCycle].mWakeUpLatencyForFeederThread = inFeederLatency;
	dataBinIn->mCycles[dataBinIn->mCurrentCycle].mWorkDoneInCycle = inWorkDone;
	dataBinIn->mCycles[dataBinIn->mCurrentCycle].mHALThreadDidOverload = inHALOverloaded;
	dataBinIn->mCycles[dataBinIn->mCurrentCycle].mHALThreadTraceDidOverload = inHALTraceOverloaded;
	dataBinIn->mCycles[dataBinIn->mCurrentCycle].mFeederThreadDidOverloadLastCycle = inFTOverloaded;
	dataBinIn->mCycles[dataBinIn->mCurrentCycle].mFeederThreadNeverRan = inFTNeverRan;
	dataBinIn->mCycles[dataBinIn->mCurrentCycle].mFeederThreadTraceDidOverload = inFTTraceOverloaded;
	dataBinIn->mCycles[dataBinIn->mCurrentCycle].mWorkLoopTraceDidOverload = inWLTraceOverloaded;
	
	dataBinIn->mCurrentCycle = (dataBinIn->mCurrentCycle) + 1;
	if (dataBinIn->mCurrentCycle == dataBinIn->mNumCycles) {
		dataBinIn->mCurrentCycle = 0;
		dataBinIn->mDataBinDidWrap = true;
		dataBinIn->mDataBinDidWrapSticky = true;
	}
	
#ifdef DEBUGPRINT
printf ("Current DataBin index:%d\n", dataBinIn->mCurrentCycle);
#endif
}

UInt32		DataBinNumCycles (DataBin* dataBinIn)
{
	return dataBinIn->mNumCycles;
}

UInt32		DataBinCurrentIndex (DataBin* dataBinIn)
{
	return dataBinIn->mCurrentCycle;
}

bool		DataBinWrapped (DataBin* dataBinIn)
{
	// this function resets itself.  i.e., if true, when called, it will return true 
    // and reset its value to false (for subsequent calls.)
	if (dataBinIn->mDataBinDidWrap) {
		dataBinIn->mDataBinDidWrap = false;
		
		return true;
	}
	
    return false;
}

CycleData*	DataBinGetCycleAtIndex (DataBin* dataBinIn, UInt32 index)
{
	return &(dataBinIn->mCycles[index]);
}

CycleData*	DataBinGetCurrentCycle (DataBin* dataBinIn)
{
	return &(dataBinIn->mCycles [dataBinIn->mCurrentCycle]);
}

void		DataBinReset (DataBin* dataBinIn)
{
	dataBinIn->mCurrentCycle = 0;
}

/*
 */
