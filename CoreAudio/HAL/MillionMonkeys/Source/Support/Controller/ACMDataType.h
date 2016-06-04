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
/* ACMDataType.h */

#ifndef __ACMDATATYPE_H__
#define __ACMDATATYPE_H__

#import <pthread.h>

#include <CoreServices/../Frameworks/CarbonCore.framework/Headers/MacTypes.h>
#import <CoreAudio/CoreAudio.h>

#import "AudioData.h"
#import "TraceBin.h"
#import "MovingAverage.h"

@class MillionMonkeys;

typedef struct AudioCycleManager {
/* global HAL / Device data */
    AudioDeviceID *					_AudioDevicesInSystem;
	AudioDeviceID *					_AudioDevice;
    AudioStreamBasicDescription*	_AudioDeviceASBD;
	UInt32							_BufferSizeInFrames;
    UInt64							_BufferSizeInNanos;
	UInt32							_DeviceSafetyOffsetInFrames;
    
	AudioData *						_AudioBuffer;
	AudioData *						_BlittedBuffer;
	AudioData *						_SilentBuffer;

/* data collected from slice */
	UInt64							_testStartTime;
    
	Boolean							_HALThreadOverloaded;
	Boolean							_HALTraceOverloaded;
	Boolean							_feederThreadOverloaded;
	Boolean							_FTTraceOverloaded;
	Boolean							_feederThreadNeverRan;
	Boolean							_WLTraceOverloaded;
    
	UInt64							_SliceHostTimeStamp;
	Float64							_HALWakeupDelta;
	UInt64							_WhenShouldIWakeUpNextTime;
	UInt64							_WhenIWokeUpThisTime;
    
    TraceBinCollection *			_traceBinCollection;
    MovingAverage *					_MovingAverage;
    
/* Test parameters */
	Boolean							_SimulatedWorkShouldBeDoneInSeparateThread;
	UInt32							_feederThreadLoadingStyle;
	UInt32							_feederThreadLoadingValue;
	UInt32							_feederThreadCycles;
	
	Float64							_sampleTimeToWaitFor;
    
    Boolean							_tracingEnabled;
	Boolean							_logCPUExecutionTraceBoxChecked;
	Boolean							_logWorkLoopTraceBoxChecked;
	UInt32							_cachedCycleDataIndexForHALTrace;
    UInt32							_ThreadLatencyThreshold;
    Float32							_WorkLoopTraceThreshold;
    
/* Internal test data */
	UInt32							_HALCyclesRun;
	UInt32							_cycleCounter;
	
	UInt64							_timeWhenDoneForThisCycle;
    
	Boolean							_FTMadeRunnableWaitingToBeScheduled;
	UInt64							_timeWhenFeederThreadMadeRunnable;
	UInt64							_timeWhenFeederThreadRuns;
	
	Boolean							_HALLatencyBeingTraced;
	Boolean							_FTLatencyBeingTraced;
    Boolean							_WLLatencyBeingTraced;
    
    Boolean							_testIsRunning;
	Boolean							_testNeedsToStop;
	Boolean							_testJustFinished;
	Boolean							_DataCollectionBufferOverrun;
    
	double							_divisor;
	
/* feeder thread data */
	pthread_t						_feederThread;
	pthread_attr_t					_feederThreadAttributes;
	pthread_cond_t					_feederThreadConditionAttributes;
	pthread_mutex_t					_feederThreadMutex;
    
/* parent reference */
	MillionMonkeys*					_AppInstance;
	
} AudioCycleManager;

#endif	/* __ACMDATATYPE_H__ */

/*
 */
