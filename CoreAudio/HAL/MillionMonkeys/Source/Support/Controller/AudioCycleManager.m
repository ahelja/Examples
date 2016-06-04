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
/* AudioCycleManager.m */

typedef void*	(*ThreadRoutine)(void* inParameter);

#import <stdlib.h>
#import <stdio.h>
#import <unistd.h>
#import <pthread.h>

#import <mach/policy.h>
#import <mach/thread_act.h>
#import <mach/thread_policy.h>
#import <mach/mach_time.h>

#import <Foundation/NSObjCRuntime.h>
#import <CoreAudio/CoreAudio.h>

#import "MillionMonkeys.h"

#import "DataBinCollection.h"
#import "DataCollectionManager.h"

#import "AudioData.h"
#import "MovingAverage.h"
#import "pThreadUtilities.h"

#import "AudioCycleManager.h"

/* "private" function declarations */
Boolean notifyOnError (int fctnResult, char* errMsg);

/* latency.c declarations */
extern	double	divisor;

extern	void	InitializeLatencyTracing(const char* inLogFile);
extern	void	getdivisor();
extern	void	sample_sc();

// thread run loops
OSStatus HALIOProc (	AudioDeviceID inDevice,
						const AudioTimeStamp* inNow,
						const AudioBufferList* inInputData,
						const AudioTimeStamp* inInputTime,
						AudioBufferList* outOutputData,
						const AudioTimeStamp* inOutputTime,
						void* inClientData);

OSStatus HALOverloadListener (	AudioDeviceID	inDevice,
								UInt32			inChannel,
								Boolean			isInput,
								AudioDevicePropertyID	inPropertyID,
								void *					inVoidACM);

void feederThreadRunnable (AudioCycleManager* inACM);

void _AudioCycleManagerLoadOneCycle (AudioCycleManager* inACM);

// listener procs
OSStatus StreamFormatChangedProc (	AudioDeviceID inDevice,
									UInt32 inChannel,
									Boolean isInput,
									AudioDevicePropertyID inPropertyID,
									void* inVoidACM);

@class MillionMonkeys;

AudioCycleManager* AudioCycleManagerNew () {
	AudioCycleManager *	retVal;
	
	// [A] CREATE OBJECT
	retVal = malloc (sizeof (AudioCycleManager));
	
	// [B] INITIALIZE VARIABLES
    retVal->_AudioDeviceASBD = NULL;
	retVal->_AudioDevice = NULL;
    retVal->_AudioDevicesInSystem = NULL;
	retVal->_testStartTime = 0;
	retVal->_WhenShouldIWakeUpNextTime = 0;
	retVal->_HALWakeupDelta = 0;
	retVal->_HALCyclesRun = 0;
	retVal->_cycleCounter = 0;
	retVal->_DeviceSafetyOffsetInFrames = 0;
	retVal->_SimulatedWorkShouldBeDoneInSeparateThread = TRUE;
	retVal->_timeWhenDoneForThisCycle = 0;
	retVal->_BufferSizeInNanos = 0;
	retVal->_timeWhenFeederThreadMadeRunnable = 0;
	retVal->_timeWhenFeederThreadRuns = 0;
	retVal->_feederThreadOverloaded = FALSE;
	retVal->_feederThreadNeverRan = FALSE;
	retVal->_HALThreadOverloaded = TRUE;
	retVal->_FTMadeRunnableWaitingToBeScheduled = FALSE;
	retVal->_HALLatencyBeingTraced = FALSE;
	retVal->_FTLatencyBeingTraced = FALSE;
	retVal->_WLLatencyBeingTraced = FALSE;
	retVal->_testNeedsToStop = FALSE;
	retVal->_traceBinCollection = NULL;
    retVal->_tracingEnabled = FALSE;
	retVal->_testIsRunning = FALSE;
	retVal->_testJustFinished = FALSE;
	retVal->_DataCollectionBufferOverrun = FALSE;
	retVal->_SliceHostTimeStamp = 0;
    retVal->_MovingAverage = NULL;
    retVal->_WorkLoopTraceThreshold = 0;
    retVal->_BlittedBuffer = NULL;
    retVal->_SilentBuffer = NULL;
	retVal->_AudioBuffer = NULL;
    
	return retVal;
}

void AudioCycleManagerDestroy (AudioCycleManager* inACM)
{
    TraceBinCollectionDestroy (inACM->_traceBinCollection);
    
    if (inACM->_AudioDevicesInSystem != NULL) {
        free (inACM->_AudioDevicesInSystem);
        inACM->_AudioDevicesInSystem = NULL;
    }
    
    MovingAverageDestroy (inACM->_MovingAverage);
    
	free (inACM);
}

void	AudioCycleManagerInitialize (AudioCycleManager* inACM)
{
	pthread_attr_t		theThreadAttributes;
    
	// [A] SETUP LATENCY FACILITIES
    // [1] if user is root, enable tracing
    if ( geteuid() == 0 ) {
        // latency.c initialization
        InitializeLatencyTracing ((const char *)(stdout));
        AudioCycleManagerSetTracingEnabled (inACM, TRUE);
    }
	getdivisor();
	
	inACM->_divisor = divisor;
	// [2] allocate trace data
    inACM->_traceBinCollection = TraceBinCollectionNew (500, (48 * 1024));	// create 500 trace bins @ 48KB each
    
	// [B] PRELIMINARY THREAD SETUP
	// create thread condition variable
	notifyOnError (pthread_cond_init(&inACM->_feederThreadConditionAttributes, NULL), "Error creating condition variable for feeder thread.");
	// create mutex
	notifyOnError (pthread_mutex_init(&inACM->_feederThreadMutex, NULL), "Error creating mutex.");
	
	// [C] SPAWN THREADS
	// setup
	notifyOnError (pthread_attr_init(&theThreadAttributes), "Error getting thread attributes.");
	notifyOnError (pthread_attr_setdetachstate(&theThreadAttributes, PTHREAD_CREATE_DETACHED), "Error setting thread detachable.");
	
	// feeder thread
	notifyOnError (pthread_create (&inACM->_feederThread, &theThreadAttributes, (ThreadRoutine)feederThreadRunnable, inACM), "Error creating feeder thread.");
	setThreadToPriority (inACM->_feederThread, 63, FALSE, 0);
    
	// cleanup
	pthread_attr_destroy (&theThreadAttributes);
    
    // [D] OTHER SETUP
    inACM->_MovingAverage = MovingAverageNew (10);
}

void	AudioCycleManagerStart (AudioCycleManager* inACM, UInt32 inWaveformType, Float32 inFrequency, Float32 inAmplitude)
{
    // render audio to be used
    if (inACM->_SilentBuffer == NULL) {
        inACM->_SilentBuffer = AudioDataNew (SQUARE_WAVE, inACM->_AudioDeviceASBD->mSampleRate, 441.0, 0.0);
    }
    
    if (inACM->_BlittedBuffer != NULL) {
        AudioDataDestroy (inACM->_BlittedBuffer);
    }
    inACM->_BlittedBuffer = AudioDataNew (inWaveformType, inACM->_AudioDeviceASBD->mSampleRate, inFrequency, inAmplitude);    
    
    inACM->_AudioBuffer = inACM->_BlittedBuffer;
    inACM->_cycleCounter = inACM->_HALCyclesRun = 0;
	inACM->_HALLatencyBeingTraced = FALSE;
	
	// start test & mark start timestamp
	inACM->_WhenShouldIWakeUpNextTime = mach_absolute_time();
	inACM->_testStartTime = mach_absolute_time();
	notifyOnError (AudioDeviceStart (*inACM->_AudioDevice, HALIOProc), "Error starting HAL.");
	inACM->_testIsRunning = TRUE;
}

void AudioCycleManagerStop (AudioCycleManager* inACM)
{
	notifyOnError (AudioDeviceStop (*inACM->_AudioDevice, HALIOProc), "Error stopping HAL.");
	inACM->_testIsRunning = FALSE;
	inACM->_testJustFinished = TRUE;
}

/* Device information */
UInt32	AudioCycleManagerGetNumberOfDevices (AudioCycleManager* inACM)
{
    UInt32		dataSize;
    Boolean		isWritable;
    
    if (inACM->_AudioDevicesInSystem != NULL) {
        free (inACM->_AudioDevicesInSystem);
    }
    
	notifyOnError (AudioHardwareGetPropertyInfo (kAudioHardwarePropertyDevices, &dataSize, &isWritable), "Error getting device list size.");
	inACM->_AudioDevicesInSystem = malloc (dataSize);
	notifyOnError (AudioHardwareGetProperty (kAudioHardwarePropertyDevices, &dataSize, inACM->_AudioDevicesInSystem), "Error getting device list.");
    return (dataSize / sizeof(AudioDeviceID));
}

/*	AudioCycleManagerGetDeviceNameAtIndex returns a new string.
    It is the calling function's responsibility to release that memory */
char *	AudioCycleManagerGetDeviceNameAtIndex (AudioCycleManager* inACM, UInt32 inIndex)
{
    AudioDeviceID*	theDevice;
    char*			nameString;
    UInt32			dataSize;
    Boolean			isWritable;
    
    theDevice = inACM->_AudioDevicesInSystem + inIndex;
    
    notifyOnError (AudioDeviceGetPropertyInfo (*theDevice, 0, FALSE, kAudioDevicePropertyDeviceName, &dataSize, &isWritable), "Error getting deviceName.");
    nameString = (char *)malloc (dataSize);
	notifyOnError (AudioDeviceGetProperty (*theDevice, 0, FALSE, kAudioDevicePropertyDeviceName, &dataSize, nameString), "Error getting deviceName.");
	
    return nameString;
}

Boolean AudioCycleManagerGetDeviceAtIndexHasOutput (AudioCycleManager* inACM, UInt32 inIndex)
{
    UInt32			dataSize;
    Boolean			isWritable;
	
	notifyOnError (AudioDeviceGetPropertyInfo (*(inACM->_AudioDevicesInSystem + inIndex), 0, FALSE, kAudioDevicePropertyStreams, &dataSize, &isWritable), "Error getting devicename.");
	
	if (dataSize == 0) return FALSE;
	
	return TRUE;
}

void	AudioCycleManagerSetDeviceToDeviceAtIndex (AudioCycleManager* inACM, UInt32 inIndex)
{
	UInt32							dataSize;
	
    // remove old IOProc (if exists) or allocate memory for AudioDevice (if does not exist)
    if (inACM->_AudioDevice == NULL) {
        inACM->_AudioDevice = malloc (sizeof (AudioDeviceID));
    } else {
        notifyOnError (AudioDeviceRemoveIOProc (*inACM->_AudioDevice, HALIOProc), "Error removing IOProc from current device.");
		notifyOnError (AudioDeviceRemovePropertyListener (*inACM->_AudioDevice, 0, FALSE, kAudioDevicePropertyStreamFormat, StreamFormatChangedProc), "Error removing StreamFormatChangedListenerProc from current device.");
		notifyOnError (AudioDeviceRemovePropertyListener (*inACM->_AudioDevice, 0, FALSE, kAudioDeviceProcessorOverload, HALOverloadListener), "Error removing HALOverloadListenerProc from current device.");
    }
    // allocate memory for _AudioDeviceASBD if necessary
    if (inACM->_AudioDeviceASBD == NULL) inACM->_AudioDeviceASBD = malloc (sizeof(AudioStreamBasicDescription));
    
    // set new device
    memcpy (inACM->_AudioDevice, inACM->_AudioDevicesInSystem + inIndex, sizeof(AudioDeviceID) );
    
    // get & store properties for the new device
	// get device stream format:
	dataSize = sizeof (AudioStreamBasicDescription);
	notifyOnError (AudioDeviceGetProperty (*inACM->_AudioDevice, 0, FALSE, kAudioDevicePropertyStreamFormat, &dataSize, inACM->_AudioDeviceASBD), "Error setting new device.");
	// get buffer size:
	dataSize = sizeof (UInt32);
	notifyOnError (AudioDeviceGetProperty (*inACM->_AudioDevice, 0, FALSE, kAudioDevicePropertyBufferFrameSize, &dataSize, &inACM->_BufferSizeInFrames), "Error setting new device.");
	inACM->_BufferSizeInNanos = (UInt64) (inACM->_BufferSizeInFrames / inACM->_AudioDeviceASBD->mSampleRate * 1000.0f * 1000.0f * 1000.0f); // milli * micro * nano
	// get safety offset in frames:
	dataSize = sizeof (UInt32);
	notifyOnError (AudioDeviceGetProperty (*inACM->_AudioDevice, 0, FALSE, kAudioDevicePropertySafetyOffset, &dataSize, &inACM->_DeviceSafetyOffsetInFrames), "Error setting new device.");
	// add IOProc
	notifyOnError (AudioDeviceAddIOProc (*inACM->_AudioDevice, HALIOProc, inACM), "Error setting new device.");
    
	// add HALOverloaded Property Listener
	notifyOnError (AudioDeviceAddPropertyListener (*inACM->_AudioDevice, 0, FALSE, kAudioDeviceProcessorOverload, HALOverloadListener, inACM), "Unable to add property listener.");
	
	// add StreamFormatChanged Property Listener
	notifyOnError (AudioDeviceAddPropertyListener (*inACM->_AudioDevice, 0, FALSE, kAudioDevicePropertyStreamFormat, StreamFormatChangedProc, inACM), "Unable to add property listener.");
}

void	AudioCycleManagerGetIOProcFrameSizeRange (AudioCycleManager* inACM, UInt32* outMinimum, UInt32* outMaximum)
{
	UInt32					outSize;
	struct AudioValueRange	AVR;
	
	outSize = sizeof(AudioValueRange);
	notifyOnError (AudioDeviceGetProperty (*inACM->_AudioDevice, 0, FALSE, kAudioDevicePropertyBufferFrameSizeRange, &outSize, &AVR), "Error getting buffer frame size range.");
	
	*outMinimum = AVR.mMinimum;
	*outMaximum = AVR.mMaximum;
}

UInt32	AudioCycleManagerGetBufferFrameSize (AudioCycleManager* inACM)
{
	UInt32 outSize = sizeof(UInt32);
	UInt32 retVal = 0;
	
	notifyOnError (AudioDeviceGetProperty (*inACM->_AudioDevice, 0, FALSE, kAudioDevicePropertyBufferFrameSize, &outSize, &retVal), "Error getting current buffer frame size.");
	
	return retVal;
}
void	AudioCycleManagerSetBufferFrameSize (AudioCycleManager* inACM, UInt32 inFrameSize)
{
	struct AudioTimeStamp	timeStamp;
	timeStamp.mHostTime = AudioGetCurrentHostTime();
	timeStamp.mFlags = kAudioTimeStampHostTimeValid;
	
	notifyOnError (AudioDeviceSetProperty(*inACM->_AudioDevice, &timeStamp, 0, FALSE, kAudioDevicePropertyBufferFrameSize, sizeof(UInt32), &inFrameSize), "Unable to set new frame size.");
}


#pragma mark ____STATE MANAGEMENT
Boolean AudioCycleManagerGetTracingEnabled (AudioCycleManager* inACM)
{
	return inACM->_tracingEnabled;
}

void AudioCycleManagerSetTracingEnabled (AudioCycleManager* inACM, Boolean inShouldTrace)
{
	inACM->_tracingEnabled = inShouldTrace;
}

Boolean AudioCycleManagerDataCollectionBufferOverrun (AudioCycleManager* inACM)
{
	if (inACM->_DataCollectionBufferOverrun) {
		// if flag is set, reset flag, but STILL RETURN TRUE
		inACM->_DataCollectionBufferOverrun = FALSE;
		return TRUE;
	}
	
	return FALSE;
}

#pragma mark ____THREAD RUN LOOPS
/* ----------------------------- */
/* - IOProc / THREAD RUNNABLES - */
/* ----------------------------- */
//   [A] HAL
OSStatus HALIOProc (AudioDeviceID inDevice, const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime, AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime, void* inVoidACM)
{
	AudioCycleManager * 	inACM;
	
	DataBin *				dataBinPointer;
	TraceBin *				HALLatencyTraceBin;
	TraceBinWriteTracker	TBWT;
    
	Boolean					shouldBlitAndExit;
	
    // blitting variables
    AudioData *				CopyFromBuffer;
    UInt32					FramesLeftToBlit;
    UInt32					FramesToBlitThisLoop;
    UInt32					FullBlockCopyCount;
    UInt32					FrameSizeInBytes;
    Float32 *				CurrentWriteByte;
    UInt32					i;
    
	inACM = (AudioCycleManager *)inVoidACM;
	shouldBlitAndExit = FALSE;
    
    // [1] OVERLOAD CHECKS:
    //   [A] FT (FeederThread) overloads
	//		2 potential FT overloads:
    // 		[i] FT is ready, but has not yet run: (flag as FTBlocked)
    //		[ii] FT is running, and was pre-empted by this HAL IOProc (flag as FTOverloaded)
	if (inACM->_cycleCounter != 0) {
		if ( inACM->_FTMadeRunnableWaitingToBeScheduled ) {
			inACM->_feederThreadNeverRan = TRUE;
			inACM->_cycleCounter = 0;
		} else {
            // NO: we've overloaded: set flag & return from function (to finish out pre-empted feeder thread)
            inACM->_feederThreadOverloaded = TRUE;
		}
		return noErr;
	}
	
    //   [B] Trace overloads (overloads introduced purely because we're taking a CPU Execution trace)
	if ( inACM->_HALLatencyBeingTraced == TRUE) {
		inACM->_HALTraceOverloaded = TRUE;
		shouldBlitAndExit = TRUE;
	}
		
	if ( inACM->_FTLatencyBeingTraced == TRUE) {
		inACM->_FTTraceOverloaded = TRUE;
		shouldBlitAndExit = TRUE;
	}
	
	if ( inACM->_WLLatencyBeingTraced == TRUE ) {
		inACM->_WLTraceOverloaded = TRUE;
		shouldBlitAndExit = TRUE;
	}
    
	// [2] CALCULATIONS / DATA RECORDING
	inACM->_WhenIWokeUpThisTime = inNow->mHostTime;
	++inACM->_cycleCounter;
	
	// keep track of slice start time
	inACM->_SliceHostTimeStamp = inNow->mHostTime;
    
	// calculate feeder thread's load-termination-time for this cycle (for use in time-based test only)
	inACM->_timeWhenDoneForThisCycle = inACM->_WhenIWokeUpThisTime + (AudioConvertNanosToHostTime (inACM->_BufferSizeInNanos * inACM->_feederThreadLoadingValue * 0.01));
	
	// store HAL's wake up time this IOProc cycle
	if (inACM->_WhenIWokeUpThisTime > inACM->_WhenShouldIWakeUpNextTime) {
		inACM->_HALWakeupDelta = ((Float64)AudioConvertHostTimeToNanos (inACM->_WhenIWokeUpThisTime - inACM->_WhenShouldIWakeUpNextTime)) * 0.001;	// get value in us --> woke up late
	} else {
		inACM->_HALWakeupDelta = ((Float64)AudioConvertHostTimeToNanos (inACM->_WhenShouldIWakeUpNextTime - inACM->_WhenIWokeUpThisTime)) * 0.001 * -1;	// get value in us --> woke up early (thus the "* -1")
	}
	
	// get feeder thread's output from last buffer, & set feeder thread's "output" buffer to silence ("clean the slate")
	CopyFromBuffer = inACM->_AudioBuffer;
	if (shouldBlitAndExit == FALSE) {
		inACM->_AudioBuffer = inACM->_SilentBuffer;
	}
	
	// [3] BLIT AUDIO DATA INTO HAL'S BUFFER
    FramesLeftToBlit = inACM->_BufferSizeInFrames;
    CurrentWriteByte = outOutputData->mBuffers[0].mData;
	FrameSizeInBytes = 2 * sizeof(Float32);
    
    // [COPY STAGE 1] write up to end of buffer (or less, if not that much is required)
    if ( (CopyFromBuffer->mDataSizeInFrames - CopyFromBuffer->mPlaybackHead) >= FramesLeftToBlit ) {
        FramesToBlitThisLoop = FramesLeftToBlit;
    } else {
        FramesToBlitThisLoop = CopyFromBuffer->mDataSizeInFrames - CopyFromBuffer->mPlaybackHead;
    }
    memcpy (CurrentWriteByte, CopyFromBuffer->mData + (CopyFromBuffer->mPlaybackHead * 2), FramesToBlitThisLoop * FrameSizeInBytes);
    CurrentWriteByte += (FramesToBlitThisLoop * 2);
    AudioDataMovePlaybackHead (CopyFromBuffer, FramesToBlitThisLoop);
    FramesLeftToBlit -= FramesToBlitThisLoop;
    
    if (FramesLeftToBlit > 0) {
        // [COPY STAGE 2] blit as many full buffers as necessary
        FullBlockCopyCount = (UInt32) ( (Float32)FramesLeftToBlit / (Float32)CopyFromBuffer->mDataSizeInFrames );
        for (i = 0; i < FullBlockCopyCount; i++) {
            FramesToBlitThisLoop = CopyFromBuffer->mDataSizeInFrames - CopyFromBuffer->mPlaybackHead;	// should just equal mDataSizeInFrames
            memcpy (CurrentWriteByte, CopyFromBuffer->mData + (CopyFromBuffer->mPlaybackHead * 2), FramesToBlitThisLoop * FrameSizeInBytes);
            CurrentWriteByte += (FramesToBlitThisLoop * 2);
            AudioDataMovePlaybackHead (CopyFromBuffer, FramesToBlitThisLoop);	// doing this for procedure, but functionally it shouldn't
                                                                                // change anything
            FramesLeftToBlit -= FramesToBlitThisLoop;
        }
        
        if (FramesLeftToBlit > 0) {
            // [COPY STAGE 3] blit beginning portion of buffer
            FramesToBlitThisLoop = FramesLeftToBlit;
            memcpy (CurrentWriteByte, CopyFromBuffer->mData + (CopyFromBuffer->mPlaybackHead * 2), FramesToBlitThisLoop * FrameSizeInBytes);
            CurrentWriteByte += (FramesToBlitThisLoop * 2);
            AudioDataMovePlaybackHead (CopyFromBuffer, FramesToBlitThisLoop);
            FramesLeftToBlit -= FramesToBlitThisLoop;
        }
    }
    
    // [4] EXECUTE LOADING (DIRECTLY, OR VIA FT)
	if ( shouldBlitAndExit == FALSE ) {
		// signal feeder thread to wake up, or perform loading in HAL
		if (inACM->_SimulatedWorkShouldBeDoneInSeparateThread) {
			inACM->_timeWhenFeederThreadMadeRunnable = mach_absolute_time();
			notifyOnError(pthread_cond_broadcast (&inACM->_feederThreadConditionAttributes), "Error signalling feeder thread to wake up.");
			inACM->_FTMadeRunnableWaitingToBeScheduled = TRUE;
		} else {
			_AudioCycleManagerLoadOneCycle (inACM);
		}
		++inACM->_HALCyclesRun;
	}
    
    // [5] TRACE HAL WAKEUP LATENCY
	// reset traceOverloaded flag
	inACM->_HALTraceOverloaded = FALSE;
	
	// Do HAL trace
	if ( shouldBlitAndExit == FALSE ) {
		if ( (inACM->_logCPUExecutionTraceBoxChecked) && (inACM->_WhenIWokeUpThisTime > inACM->_WhenShouldIWakeUpNextTime) ) {
			// When the HAL overloads, it misses a cycle.  If we try to trace that whole cycle (which is usually on the order of 10's of
			// milliseconds) and do it IN the HAL IOProc, we'll just bog down the system, creating an overload on the next cycle because
			// we tried to take the trace, etc..., so we'll only trace latencies less than or equal to 10000us (10ms).
			if ( (AudioConvertHostTimeToNanos ((SInt64)inACM->_WhenIWokeUpThisTime - (SInt64)inACM->_WhenShouldIWakeUpNextTime) <= 10000 * 1000) &&
				(AudioConvertHostTimeToNanos ((SInt64)inACM->_WhenIWokeUpThisTime - (SInt64)inACM->_WhenShouldIWakeUpNextTime) > inACM->_ThreadLatencyThreshold * 1000) ) {
				inACM->_HALLatencyBeingTraced = TRUE;
                
				dataBinPointer = DataCollectionManagerGetCurrentDataBin([inACM->_AppInstance getDCM]);
				HALLatencyTraceBin = DataBinGetCurrentCycleTraceMemory (dataBinPointer, inACM->_traceBinCollection, HAL_IOPROC_TRACE);
				HALLatencyTraceBin->mHostTimeStamp = inACM->_SliceHostTimeStamp;
				HALLatencyTraceBin->mTraceType = HAL_IOPROC_TRACE;
					TBWT.mTraceBin = HALLatencyTraceBin->mTrace;
					TBWT.mCurrentWritePointer = HALLatencyTraceBin->mTrace;
					TBWT.mTraceBinCapacity = HALLatencyTraceBin->mTraceSize;
				sample_sc (inACM->_WhenShouldIWakeUpNextTime, inACM->_WhenIWokeUpThisTime, &TBWT);
				inACM->_HALLatencyBeingTraced = FALSE;
			}
		}
	}
	
    // [6] CLEANUP
	// keep track of last start time
	inACM->_WhenShouldIWakeUpNextTime = inNow->mHostTime + AudioConvertNanosToHostTime ( (UInt64) (((Float64)inACM->_BufferSizeInFrames) / inACM->_AudioDeviceASBD->mSampleRate * 1000.0f * 1000.0f * 1000.0f));
	
	if (shouldBlitAndExit) {
		inACM->_cycleCounter = 0;
	}
	
	return noErr;
}

OSStatus HALOverloadListener (	AudioDeviceID	inDevice,
								UInt32			inChannel,
								Boolean			isInput,
								AudioDevicePropertyID	inPropertyID,
								void *					inVoidACM)
{
	((AudioCycleManager *)inVoidACM)->_HALThreadOverloaded = TRUE;
	return noErr;
}

//   [B] Feeder Thread
void feederThreadRunnable (AudioCycleManager* inACM)
{	
	while (TRUE) {
        // [1] put feeder thread in wait state
        notifyOnError (pthread_cond_wait (&inACM->_feederThreadConditionAttributes, &inACM->_feederThreadMutex), "Error putting feeder thread in wait state.");
		// [2] run one cycle
        _AudioCycleManagerLoadOneCycle (inACM);
	};
}

void _AudioCycleManagerLoadOneCycle (AudioCycleManager* inACM)
{
	UInt32					count;
	UInt64					startTime, endTime;
	UInt64					workTraceStartTime, workTraceEndTime;
	UInt64					cachedSliceTime;
	
	DataBin*				currentDataBin;
	TraceBin*				FTLatencyTraceBin;
	UInt32					WorkLoopLatencyTraceBinIndex;
	TraceBin*				WorkLoopLatencyTraceBin;
	Float32					FTLatency;
	TraceBinWriteTracker	TBWT;
	
	// [1] DATA STAGING
	WorkLoopLatencyTraceBinIndex = 0;
	count = 0;
	inACM->_timeWhenFeederThreadRuns = mach_absolute_time();
    FTLatencyTraceBin = NULL;
    WorkLoopLatencyTraceBin = NULL;
	cachedSliceTime = inACM->_SliceHostTimeStamp;
	
	// [2] switch buffer pointer over to audio data
	inACM->_AudioBuffer = inACM->_BlittedBuffer;
	
	if (inACM->_SimulatedWorkShouldBeDoneInSeparateThread) {
		// [a] flag feeder thread done
		inACM->_FTMadeRunnableWaitingToBeScheduled = FALSE;
		
		// [b] calculate latency
		FTLatency = (double)(inACM->_timeWhenFeederThreadRuns - inACM->_timeWhenFeederThreadMadeRunnable) / inACM->_divisor;
	} else {
		FTLatency = 0;
	}
	
	// [3] get new data bin if current one is full ONLY IF we're doing multiple proc's per data point
	currentDataBin = DataCollectionManagerGetCurrentDataBin ([inACM->_AppInstance getDCM]);
	
	if ( DataBinWrapped (currentDataBin) && (DataCollectionManagerGetBinGranularity([inACM->_AppInstance getDCM]) > 1) ) {
		DataCollectionManagerSetCurrentDataBin ([inACM->_AppInstance getDCM], DataBinCollectionGetNextWriteBin (DataCollectionManagerGetDataBinQueue([inACM->_AppInstance getDCM])));
		if (currentDataBin == NULL) {
			// DataBinCollection buffer overrun encountered.  Stop test & flag for alert panel
			AudioCycleManagerStop (inACM);
			inACM->_testNeedsToStop = TRUE;
			inACM->_DataCollectionBufferOverrun = TRUE;
			
			return;
		}
	}
	
	// [4]	cache time stamp for FTTrace (to make sure it aligns with parent CycleData's timestamp) since the next HAL firing will trump it
	//		and make it look unaligned, which would invalidate the trace (which wouldn't be true!)  We want to cache this BEFORE we do our
	//		workload.
	if ( (inACM->_logCPUExecutionTraceBoxChecked) && (FTLatency >= inACM->_ThreadLatencyThreshold) ) {
		FTLatencyTraceBin = DataBinGetCurrentCycleTraceMemory (currentDataBin, inACM->_traceBinCollection, FEEDER_THREAD_TRACE);
		
        FTLatencyTraceBin->mHostTimeStamp = inACM->_SliceHostTimeStamp;
        FTLatencyTraceBin->mTraceType = FEEDER_THREAD_TRACE;
	}
	
	if ( inACM->_logWorkLoopTraceBoxChecked ) {
		WorkLoopLatencyTraceBinIndex = DataBinCurrentIndex (currentDataBin);
	}
	
	// [5] CPU Loading
	workTraceStartTime = AudioGetCurrentHostTime ();
	
	if (inACM->_feederThreadLoadingStyle == TIME_LOADING) {
		// TIME LOADING
		while (AudioGetCurrentHostTime() < inACM->_timeWhenDoneForThisCycle) ++count;
	} else {
		// WORK LOADING
		startTime = AudioGetCurrentHostTime ();
		while (count < inACM->_feederThreadCycles) ++count;
		endTime = AudioGetCurrentHostTime ();
		count = (UInt32)(endTime - startTime);
	}
	workTraceEndTime = AudioGetCurrentHostTime ();
    
	// [6] overload test: this will be true if the HAL fired before we completed a cycle (race condition)
	if (inACM->_cycleCounter > 1) {
		count = -inACM->_cycleCounter;
	}
	
	// [7] write out data:	THIS NEEDS TO HAPPEN AFTER EXECUTION TRACE BECAUSE THIS FUNCTION (IMPLICITLY) INCREMENTS
	//						THE COUNTER ON THE CURRENT BIN.
	
	DataBinWriteDataForCurrentCycle (	currentDataBin, inACM->_SliceHostTimeStamp,
                                        inACM->_HALWakeupDelta, FTLatency, count,
                                        inACM->_HALThreadOverloaded, inACM->_HALTraceOverloaded,
                                        inACM->_feederThreadOverloaded, inACM->_feederThreadNeverRan,
                                        inACM->_FTTraceOverloaded, inACM->_WLTraceOverloaded);
    
	// [8] Flag cycle complete for I/O thread
	inACM->_HALThreadOverloaded = FALSE;
	inACM->_feederThreadOverloaded = FALSE;
	inACM->_feederThreadNeverRan = FALSE;
	inACM->_cycleCounter = 0;
	
	// [9] Reset traceOverloaded flags
	inACM->_FTTraceOverloaded = FALSE;
	inACM->_WLTraceOverloaded = FALSE;
	
	// [10] FT Latency trace
	if ( (inACM->_logCPUExecutionTraceBoxChecked) && (FTLatency >= inACM->_ThreadLatencyThreshold) ) {
		inACM->_FTLatencyBeingTraced = TRUE;
			TBWT.mTraceBin = FTLatencyTraceBin->mTrace;
			TBWT.mCurrentWritePointer = FTLatencyTraceBin->mTrace;
			TBWT.mTraceBinCapacity = FTLatencyTraceBin->mTraceSize;
		sample_sc (inACM->_timeWhenFeederThreadMadeRunnable, inACM->_timeWhenFeederThreadRuns, &TBWT);
		inACM->_FTLatencyBeingTraced = FALSE;
	}
    
	// [11] Work loop trace
	if (inACM->_logWorkLoopTraceBoxChecked) {
		bool traceCondition;
		
		traceCondition = FALSE;
				
        MovingAverageAddValue (inACM->_MovingAverage, count);
		if ( (inACM->_feederThreadLoadingStyle == TIME_LOADING) && (count < (inACM->_WorkLoopTraceThreshold * MovingAverageGetAverage (inACM->_MovingAverage))) ) {
			traceCondition = TRUE;
		}
		if ( (inACM->_feederThreadLoadingStyle == WORK_LOADING) && (count > ( (1.0 / inACM->_WorkLoopTraceThreshold) * MovingAverageGetAverage (inACM->_MovingAverage))) ) {
			traceCondition = TRUE;
		}
        if ( traceCondition ) {
            inACM->_WLLatencyBeingTraced = TRUE;
			WorkLoopLatencyTraceBin = DataBinGetCycleTraceMemoryForIndex (	currentDataBin, WorkLoopLatencyTraceBinIndex,
																		inACM->_traceBinCollection, WORK_LOOP_TRACE);
			WorkLoopLatencyTraceBin->mHostTimeStamp = cachedSliceTime;
			WorkLoopLatencyTraceBin->mTraceType = WORK_LOOP_TRACE;
			
            TBWT.mTraceBin = WorkLoopLatencyTraceBin->mTrace;
            TBWT.mCurrentWritePointer = WorkLoopLatencyTraceBin->mTrace;
            TBWT.mTraceBinCapacity = WorkLoopLatencyTraceBin->mTraceSize;
            sample_sc (workTraceStartTime, workTraceEndTime, &TBWT);
            inACM->_WLLatencyBeingTraced = FALSE;
        }
    }
}

#pragma mark ____LISTENER PROCS
OSStatus StreamFormatChangedProc (	AudioDeviceID inDevice,
									UInt32 inChannel,
									Boolean isInput,
									AudioDevicePropertyID inPropertyID,
									void* inVoidACM)
{
	UInt32		dataSize;
	
	AudioCycleManager*	inACM;
	inACM = (AudioCycleManager*)inVoidACM;
	
    // allocate memory for _AudioDeviceASBD if necessary
    if (inACM->_AudioDeviceASBD == NULL) inACM->_AudioDeviceASBD = malloc (sizeof(AudioStreamBasicDescription));
	
	// get device stream format:
	dataSize = sizeof (AudioStreamBasicDescription);
	notifyOnError (AudioDeviceGetProperty (*inACM->_AudioDevice, 0, FALSE, kAudioDevicePropertyStreamFormat, &dataSize, inACM->_AudioDeviceASBD), "Error setting new device.");
	inACM->_BufferSizeInNanos = (UInt64) (inACM->_BufferSizeInFrames / inACM->_AudioDeviceASBD->mSampleRate * 1000.0f * 1000.0f * 1000.0f); // milli * micro * nano
    
    return noErr;
}

#pragma mark ____PRIVATE FUNCTIONS
Boolean notifyOnError (int fctnResult, char* errMsg)
{
	if (fctnResult != noErr) {
		printf ("Error code '%c%c%c%c' = %d\n\t%s\n",	fctnResult && 0xFF000000,
                                                        fctnResult && 0x00FF0000,
                                                        fctnResult && 0x0000FF00,
                                                        fctnResult && 0x000000FF, fctnResult, errMsg);
		return FALSE;
	}
	return TRUE;
}

/*
 */
