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
/* MillionMonkeys.m */

#import <CoreAudio/CoreAudio.h>

#include <unistd.h>

#import "MillionMonkeys.h"
#import "pThreadUtilities.h"

/* global file collection data */
FILE *						_logFile;
Boolean						_isCollectingData = FALSE;

@implementation MillionMonkeys

#pragma mark ____MILLIONMONKEYS
- (id)init
{
	int i;
	
    _dataCollectionCounter = 0;
    // view display mask: display nothing at first
    for (i = 0; i < 9; i++) {
        _displayMask[i] = FALSE;
    }
	
    _displayMask[3] = TRUE;			// display mean work-done trace
    _displayOverload = TRUE;		// display overloads
	
	mACM = AudioCycleManagerNew ();
	mDCM = DataCollectionManagerNew ();
	
	// define colors for trace window text
	mHALOVLTextColor = [NSColor colorWithCalibratedRed:1.0f green:0.0f blue:0.0f alpha:1.0f];
	[mHALOVLTextColor retain];
	mFTOVLTextColor = [NSColor colorWithCalibratedRed:0.66f green:0.0f blue:1.0f alpha:1.0f];
	[mFTOVLTextColor retain];
	mFTBlockedTextColor = [NSColor colorWithCalibratedRed:0.50f green:0.33f blue:0.17f alpha:1.0f];
	[mFTBlockedTextColor retain];
	mWorkOVLTextColor = [NSColor colorWithCalibratedRed:0.0f green:0.5f blue:1.0f alpha:1.0f];
	[mWorkOVLTextColor retain];
	mGreyedOutTextColor = [NSColor colorWithCalibratedWhite:0.7f alpha:1.0f];
	[mGreyedOutTextColor retain];
	
    _traceDisplayIndex = 0;
    return self;
}

- (void)dealloc
{
	AudioCycleManagerDestroy (mACM);
	DataCollectionManagerDestroy (mDCM);
	DataBinCollectionDealloc (mDCM->_DataBinQueue);
	
	[super dealloc];
}

- (void)awakeFromNib
{
	// if user is root, label window
	if ( geteuid() == 0 ) {
		[window setTitle:@"Million Monkeys"];
	} else {
		[window setTitle:@"Million Monkeys: (CPU trace not available - run app as root to enable)"];
	}

	// init
	// create log file
	_logFile = fopen ("/tmp/MillionMonkeys.log", "w");
	if (_logFile == NULL) {
		NSRunInformationalAlertPanel(@"Unable to open log file", @"Million Monkeys was probably last run as root, leaving the log file permissions set that way.  Delete the old log file (as root user) and re-run Million Monkeys.", @"Quit", nil, nil);
		[NSApp terminate:self];
	}
    
	mACM->_AppInstance = self;
	mDCM->_AppInstance = self;
	AudioCycleManagerInitialize (mACM);
	DataCollectionManagerInitialize (mDCM);
	
	// setup trace-display & its notification recipient
	uiConsoleController = [[NSWindowController alloc] initWithWindow:uiConsolePanel];
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(showTrace:) name:@"showTrace" object:nil];
	[window setAcceptsMouseMovedEvents:TRUE];
	
	// hang on to mini-consoles
	[uiTestTypeTimeConsole retain];
	[uiTestTypeWorkConsole retain];
	
    // make execution trace panel well-behaved
    [uiConsolePanel setFloatingPanel:FALSE];
    [uiConsolePanel setBecomesKeyOnlyIfNeeded:TRUE];
    [uiTracePrevButton setContinuous:TRUE];
	[uiTracePrevButton setPeriodicDelay:1.0f interval:0.25f];
    [uiTraceNextButton setContinuous:TRUE];
	[uiTraceNextButton setPeriodicDelay:1.0f interval:0.25f];
		
    // link view to display mask
    [uiDataCollectionView setDisplayMask:_displayMask overload:_displayOverload];
    
	[uiSelectedTest selectCellAtRow:0 column:0];
	mACM->_feederThreadLoadingStyle = TIME_LOADING;
	[uiTestTypeContainer setContentView:uiTestTypeTimeConsole];
	
	// populate device list & IOProc buffer size list (chained call)
	[self _populateDeviceList];
    [self iaSelectedDeviceChanged:self];
    [self _populateIOProcSize];
	
	// setup initial setting (based on settings of .nib file)
	[self iaIOProcSizeChanged:self];
	[self iaFeederThreadCPULoadingChanged:self];
	[self iaFTPolicyChanged:self];
	[self iaFTPriorityChanged:self];
	[self iaTraceSelectionChanged:self];
	[self iaDisplayUpdateFrequencyChanged:self];
	[self iaScalarChanged:uiHALScalar];
	[self iaScalarChanged:uiFTScalar];
	[self iaScalarChanged:uiWorkScalar];
	
	// write initial state data to log file
	[self logStateData];
	
	// enable CPUTrace mode as appropriate
	[uiLatencyTrace setEnabled:AudioCycleManagerGetTracingEnabled(mACM)];
	[uiWorkLoopTrace setEnabled:AudioCycleManagerGetTracingEnabled(mACM)];
    	
	// setup display update timer (30 fps)
	[NSTimer scheduledTimerWithTimeInterval:0.033333 target:self selector:@selector(RunLoopDisplayUpdater) userInfo:nil repeats:true];
	
	// setup overload test notification
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didOverload) name:@"didOverload" object:nil];
	
	[uiMMInfoDrawer open:self];
}

#pragma mark ____GUI IMPLEMENTATION
/* ---------------------- */
/* - GUI implementation - */
/* ---------------------- */
- (IBAction)iaSelectedTestChanged:(id)sender
{
	if ([uiSelectedTest selectedColumn] == 0) {
		[uiTestTypeContainer setContentView:uiTestTypeTimeConsole];
		[uiPanelWorkloadTitle setStringValue:@"Work Done:"];
		[uiPanelWorkloadUnits setStringValue:@"cyc"];
		mACM->_feederThreadLoadingStyle = TIME_LOADING;
	} else {
		[uiTestTypeContainer setContentView:uiTestTypeWorkConsole];
		[uiPanelWorkloadTitle setStringValue:@"Time Taken:"];
		[uiPanelWorkloadUnits setStringValue:@"us"];
		mACM->_feederThreadLoadingStyle = WORK_LOADING;
	}
	
	[self logStateData];
	[uiDataCollectionView resetData];
}

- (IBAction)iaTestButtonPressed:(id)sender
{
	if ( [uiTestButton state] == NSOnState) {
		[self startTest];
	} else {
		[self stopTest];
	}
}

- (IBAction)iaSelectedDeviceChanged:(id)sender
{
	UInt32	selectedIndex;
	
	selectedIndex = [uiDeviceList indexOfSelectedItem];
	
	AudioCycleManagerSetDeviceToDeviceAtIndex (mACM, selectedIndex);
    [uiSafetyOffset setIntValue:mACM->_DeviceSafetyOffsetInFrames];
    [self _populateIOProcSize];
	
	if (!AudioCycleManagerGetDeviceAtIndexHasOutput (mACM, selectedIndex)) {
		NSRunInformationalAlertPanel(@"No Output Streams Available", @"The device you just selected has no output streams.  Please select another device (with at least 1 output stream) with which to run the test.", @"OK", nil, nil);
	}
}

- (IBAction)iaFTPolicyChanged:(id)sender
{
    // case for coming down from RealTime
    if ( ([uiFTPriority isEnabled] == FALSE) && ([uiFTPolicy selectedRow] < 2) ) {
        [uiFTPriority selectItemWithTitle:@"63"];
        [uiFTPriority setEnabled:TRUE];
    }
    // case for going up to RealTime
    if ([uiFTPolicy selectedRow] == 2) {
        [uiFTPriority selectItemWithTitle:@"96"];
        [uiFTPriority setEnabled:FALSE];
    }
    
    [self _setFTBasedOnUISettings];
}

- (IBAction)iaFTPriorityChanged:(id)sender
{
    if ( [[uiFTPriority titleOfSelectedItem] isEqualToString:@"96"] ) {
        [uiFTPolicy selectCellAtRow:2 column:0];
        [uiFTPriority setEnabled:FALSE];
    }
    
    [self _setFTBasedOnUISettings];
}

- (void)_setFTBasedOnUISettings
{
    Float64			samplesToNanos;
    UInt64			thePeriodNanos;
    
    switch ([uiFTPolicy selectedRow]) {
        case 0:
            setThreadToPriority (mACM->_feederThread, [[uiFTPriority titleOfSelectedItem] intValue], FALSE, 0);
            break;
        case 1:
            setThreadToPriority (mACM->_feederThread, [[uiFTPriority titleOfSelectedItem] intValue], TRUE, 0);
            break;
        case 2:
            samplesToNanos  = (1000.0f * 1000.0f * 1000.0f) / mACM->_AudioDeviceASBD->mSampleRate;
            thePeriodNanos = ((float)mACM->_BufferSizeInFrames) * samplesToNanos;
            setThreadToPriority (mACM->_feederThread, 96, FALSE, thePeriodNanos);
            break;
    }
}

- (IBAction)iaIOProcSizeChanged:(id)sender
{
	UInt32	newBufferSize = [[uiIOProcSizePopUp titleOfSelectedItem] intValue];
	AudioCycleManagerSetBufferFrameSize (mACM, newBufferSize);
	mACM->_BufferSizeInFrames  = AudioCycleManagerGetBufferFrameSize (mACM);
	
    if (mACM->_AudioBuffer) AudioDataZeroPlaybackHead (mACM->_AudioBuffer);
    
    mACM->_BufferSizeInNanos = (UInt64) ((mACM->_BufferSizeInFrames / mACM->_AudioDeviceASBD->mSampleRate) * 1000000000.0);
    
    // if FT is RT, need to reset it to get new period
    if ([[uiFTPriority titleOfSelectedItem] intValue] == 96) {
        Float64 	samplesToNanos;
        UInt64		thePeriodNanos;
        
        samplesToNanos  = (1000.0f * 1000.0f * 1000.0f) / mACM->_AudioDeviceASBD->mSampleRate;
        thePeriodNanos = ((float)mACM->_BufferSizeInFrames) * samplesToNanos;
        setThreadToPriority (mACM->_feederThread, 96, FALSE, thePeriodNanos);
    }
    
	[self logStateData];
}

- (IBAction)iaSeparateThreadSelectorChanged:(id)sender
{
	bool shouldEnableWidgets = true;
	NSColor *theColor = [NSColor blackColor];
	
	if ([uiSeparateThreadSelector selectedRow] == 0) {
		shouldEnableWidgets = false;
		mACM->_SimulatedWorkShouldBeDoneInSeparateThread = false;
		theColor = [NSColor grayColor];
	} else {
		mACM->_SimulatedWorkShouldBeDoneInSeparateThread = true;
		theColor = [NSColor blackColor];
	}
	
	[uiFeederThreadRetrievedPriority setTextColor:theColor];
	[uiFTPolicySetTitle setTextColor:theColor];
	[uiFTPrioritySetTitle setTextColor:theColor];
	[uiFTPriorityGetTitle setTextColor:theColor];
    [uiFTPolicy setEnabled:shouldEnableWidgets];
    if ((shouldEnableWidgets == true) && ([uiFTPolicy selectedRow] < 2)) {
        [uiFTPriority setEnabled:TRUE];
    } else {
        [uiFTPriority setEnabled:FALSE];
    }
}

- (IBAction)iaFeederThreadCPULoadingChanged:(id)sender
{
	mACM->_feederThreadLoadingValue = [uiFeederThreadLoadingSlider intValue];
	[uiFeederThreadLoading setIntValue:mACM->_feederThreadLoadingValue];
}

- (IBAction)iaFeederThreadCPUWorkCyclesChanged:(id)sender
{
	mACM->_feederThreadCycles = [uiFeederThreadCPUWorkCycles intValue];
}

- (IBAction)iaTraceSelectionChanged:(id)sender
{
	if ( ([uiLatencyTrace state] == NSOnState) || ([uiWorkLoopTrace state] == NSOnState) ) {
		[uiDataUpdateFrequency setEditable:FALSE];
		[uiDataUpdateFrequency setIntValue:1];
	} else {
		[uiDataUpdateFrequency setEditable:TRUE];
	}
	
	if ([uiLatencyTrace state] == NSOnState) {
		mACM->_logCPUExecutionTraceBoxChecked = TRUE;
	} else {
		mACM->_logCPUExecutionTraceBoxChecked = FALSE;
	}
	
	if ([uiWorkLoopTrace state] == NSOnState) {
		mACM->_logWorkLoopTraceBoxChecked = TRUE;
	} else {
		mACM->_logWorkLoopTraceBoxChecked = FALSE;
	}
	
	[uiDataCollectionView resetData];
}

- (IBAction)iaDisplayUpdateFrequencyChanged:(id)sender
{
    mDCM->_dataCollectionsPerDisplay = [uiDisplayUpdateFrequency intValue];
}

- (IBAction) iaScalarChanged:(id)sender
{
    float scalarValue;
    
	if (sender == uiHALScalar) {
        scalarValue = [uiHALScalar floatValue] * 2;
		[uiDataCollectionView setHALScalar:scalarValue];
		if ([uiDisplayUnitsSelector selectedRow] == 0) {
			[uiUnitsView setScaleFactor:scalarValue forDataSet:HAL_LATENCY_TRACE];
		}
    }
	if (sender == uiFTScalar) {
        scalarValue = [uiFTScalar floatValue] * 2;
		[uiDataCollectionView setFTScalar:scalarValue];
		if ([uiDisplayUnitsSelector selectedRow] == 1) {
			[uiUnitsView setScaleFactor:scalarValue forDataSet:FT_LATENCY_TRACE];
		}
	}
	if (sender == uiWorkScalar) {
        scalarValue = [uiWorkScalar floatValue] * 0.005;
		[uiDataCollectionView setWorkScalar:scalarValue];
		if ([uiDisplayUnitsSelector selectedRow] == 2) {
			[uiUnitsView setScaleFactor:scalarValue forDataSet:WORK_LOOP];
		}
	}
}

- (IBAction)iaDisplayUnitsSelectorChanged:(id)sender
{
    switch ( [uiDisplayUnitsSelector selectedRow] ) {
    case 0:
        [self iaScalarChanged:uiHALScalar];
        break;
    case 1:
        [self iaScalarChanged:uiFTScalar];
        break;
    case 2:
        [self iaScalarChanged:uiWorkScalar];
        break;
    }
}

- (IBAction) iaItemDisplayListChanged:(id)inSender
{
    if (inSender == uiItemDisplayList) {
        if ([[uiItemDisplayList selectedCell] state] == NSOnState) {
            _displayMask [[uiItemDisplayList selectedRow] * 3 + [uiItemDisplayList selectedColumn]] = TRUE;
        } else {
            _displayMask [[uiItemDisplayList selectedRow] * 3 + [uiItemDisplayList selectedColumn]] = FALSE;
        }
    } else {
        // otherwise it's the overload switch
        if ([uiOverloadSwitch state] == NSOnState) {
            _displayOverload = TRUE;
        } else {
            _displayOverload = FALSE;
        }
    }
    
    [uiDataCollectionView setDisplayMask:_displayMask overload:_displayOverload];
    [uiDataCollectionView setNeedsDisplay:TRUE];
}

- (IBAction)iaPanelTraceTypeSelectionChanged:(id)sender
{
    [self _showTrace];
}

- (IBAction)iaPanelPrevTrace:(id)sender
{
	if (_traceDisplayIndex > 0) {
		[self _populateTraceInfo:_traceDisplayIndex - 1];
	} else {
		[self _populateTraceInfo:[uiDataCollectionView getNumberOfDisplayablePoints] - 1];
	}
}

- (IBAction)iaPanelNextTrace:(id)sender
{
	if (_traceDisplayIndex + 1 >= [uiDataCollectionView getNumberOfDisplayablePoints]) {
		[self _populateTraceInfo:0];
	} else {
		[self _populateTraceInfo:_traceDisplayIndex + 1];
	}
}

- (void)showTrace:(NSNotification *)inNotification
{
    if ([self _populateTraceInfo:[[inNotification object] intValue]] == YES) {
		[uiConsoleController showWindow:self];
	}
}

- (BOOL)_populateTraceInfo:(int)traceIndexIn
{
	CycleData*			currentCycleData;
    DisplayPoint*		DP;
	
    // make sure we've got data before we try to access it
	if ( (traceIndexIn >= [uiDataCollectionView getDisplayIndex])  && ([uiDataCollectionView didWrap] == FALSE) ) {
		NSBeep();
		return NO;
	}
	
    _traceDisplayIndex = traceIndexIn;
	
	// get data point & update cursor update view's selection cursor
	DP = [uiDataCollectionView getDisplayPointAtIndex:_traceDisplayIndex];
    [uiDataCollectionView setDisplayIndex:_traceDisplayIndex];
	
	currentCycleData = DataBinGetCycleAtIndex (mDCM->_displayDataBin, _traceDisplayIndex);
	
	// set traceMatrix state based on the data we have
	if (currentCycleData->mHALIOProcTraceStorage == NULL) {
		[[uiTraceSelectionMatrix cellAtRow:0 column:0] setEnabled:NO];
	} else {
		[[uiTraceSelectionMatrix cellAtRow:0 column:0] setEnabled:YES];
	}
	if (currentCycleData->mFeederThreadTraceStorage == NULL) {
		[[uiTraceSelectionMatrix cellAtRow:1 column:0] setEnabled:NO];
	} else {
		[[uiTraceSelectionMatrix cellAtRow:1 column:0] setEnabled:YES];
	}
	if (currentCycleData->mWorkLoopTrace == NULL) {
		[[uiTraceSelectionMatrix cellAtRow:2 column:0] setEnabled:NO];
	} else {
		[[uiTraceSelectionMatrix cellAtRow:2 column:0] setEnabled:YES];
	}
	
    // display slice data
    [uiTraceIndex setIntValue:_traceDisplayIndex];
	[uiPanelWorkDone setIntValue:DP->WorkDoneMean];
    [uiPanelWakeupOffset setIntValue:DP->HALLatencyMean];
	[uiPanelFTLatency setStringValue:[NSString stringWithFormat:@"%8.1llf", DP->FTLatencyMean, nil]];
	
	// display overload & trace interference data
	[uiPanelWorkTraceOVL selectText:self];
	if (DP->WorkTraceOverload) {
		[uiPanelWorkTraceOVL setTextColor:mWorkOVLTextColor];
	} else {
		[uiPanelWorkTraceOVL setTextColor:mGreyedOutTextColor];
	}
    
	[uiPanelHALTraceOVL selectText:self];
	if (DP->HALTraceOverload) {
		[uiPanelHALTraceOVL setTextColor:mHALOVLTextColor];
	} else {
		[uiPanelHALTraceOVL setTextColor:mGreyedOutTextColor];
	}
	
	[uiPanelFTTraceOVL selectText:self];
	if (DP->FeederThreadTraceOverload) {
		[uiPanelFTTraceOVL setTextColor:mFTOVLTextColor];
	} else {
		[uiPanelFTTraceOVL setTextColor:mGreyedOutTextColor];
	}
    
	[uiPanelFTBlocked selectText:self];
	if (DP->FeederThreadNeverRan) {
		[uiPanelFTBlocked setTextColor:mFTBlockedTextColor];
	} else {
		[uiPanelFTBlocked setTextColor:mGreyedOutTextColor];
	}
    	
	[uiPanelHALOVL selectText:self];
	if (DP->HALOverload) {
		[uiPanelHALOVL setTextColor:mHALOVLTextColor];
	} else {
		[uiPanelHALOVL setTextColor:mGreyedOutTextColor];
	}
	
	[uiPanelFTOVL selectText:self];
	if (DP->FeederThreadOverload) {
		[uiPanelFTOVL setTextColor:mFTOVLTextColor];
	} else {
		[uiPanelFTOVL setTextColor:mGreyedOutTextColor];
	}
	
    [self _showTrace];
	
	return YES;
}

- (void)_showTrace
{
	CycleData*		currentCycleData;
    TraceBin*		currentTraceBin;
	UInt32			timeInUs;
	
	currentTraceBin = NULL;
	currentCycleData = DataBinGetCycleAtIndex(mDCM->_displayDataBin, _traceDisplayIndex);
	
	timeInUs = (UInt32)( ((Float64)AudioConvertHostTimeToNanos (currentCycleData->mHostTimeStamp - mACM->_testStartTime)) * 0.001);
	[uiTimeSlice setIntValue: timeInUs];	// in us of upTime
	
	switch ([uiTraceSelectionMatrix selectedRow]) {
		case 0:
			currentTraceBin = currentCycleData->mHALIOProcTraceStorage;
			break;
		case 1:
			currentTraceBin = currentCycleData->mFeederThreadTraceStorage;
			break;
		case 2:
			currentTraceBin = currentCycleData->mWorkLoopTrace;
			break;
	}
    [uiPanelTraceView setFont:[NSFont userFixedPitchFontOfSize:11.0]];
    if (currentTraceBin != NULL) {
		if (currentTraceBin->mHostTimeStamp == currentCycleData->mHostTimeStamp) {
            [uiPanelTraceView setString:[NSString stringWithCString:currentTraceBin->mTrace]];
		} else {
			[uiPanelTraceView setString:@"Sorry, this trace was overwritten in the pool."];
			printf ("UNALIGNED TRACES:\n");
			printf ("\tSlice timestamp: %llu\n", currentTraceBin->mHostTimeStamp);
			printf ("\tTrace timestamp: %llu\n", currentCycleData->mHostTimeStamp);
		}
	} else {
		[uiPanelTraceView setString:@" "];
	}
}

- (void)_populateDeviceList
{
    UInt32		i, count;
    char*		cString;

    [uiDeviceList removeAllItems];
    
    count = AudioCycleManagerGetNumberOfDevices (mACM);
    for (i = 0; i < count; i++) {
        cString = AudioCycleManagerGetDeviceNameAtIndex(mACM, i);
		if (AudioCycleManagerGetDeviceAtIndexHasOutput (mACM, i)) {
			[uiDeviceList addItemWithTitle:[NSString stringWithFormat:@"[%d] %s", i, cString]];
		} else {
			[uiDeviceList addItemWithTitle:[NSString stringWithFormat:@"[%d] %s [INPUT ONLY]", i, cString]];
		}
        free (cString);
    }
}

- (void)_populateIOProcSize
{
	UInt32		frameVal = 16;
	UInt32		lo, hi;
	Boolean		firstWrite = FALSE;
	Boolean		writeComplete = FALSE;
	
	[uiIOProcSizePopUp removeAllItems];
	
	AudioCycleManagerGetIOProcFrameSizeRange (mACM, &lo, &hi);
	do {
		if (!firstWrite) {
			if (frameVal >= lo) {
				// first write
				if (lo != frameVal) {
					[uiIOProcSizePopUp addItemWithTitle:[NSString stringWithFormat:@"%lu frames", lo]];
				}
				[uiIOProcSizePopUp addItemWithTitle:[NSString stringWithFormat:@"%lu frames", frameVal]];
				firstWrite = true;
			}
		} else {
			if (frameVal >= hi) {
				// last write
				if (hi != frameVal) {
					[uiIOProcSizePopUp addItemWithTitle:[NSString stringWithFormat:@"%lu frames", hi]];
				} else {
				[uiIOProcSizePopUp addItemWithTitle:[NSString stringWithFormat:@"%lu frames", frameVal]];
				}
				writeComplete = true;
			} else {
				// middle writes
				[uiIOProcSizePopUp addItemWithTitle:[NSString stringWithFormat:@"%lu frames", frameVal]];
			}
		}
		// code to include 48 frame case 
		if (frameVal == 32) {
			[uiIOProcSizePopUp addItemWithTitle:@"48 frames"];
		}
		frameVal *= 2;
	} while (!writeComplete);
	
	frameVal = AudioCycleManagerGetBufferFrameSize (mACM);
	
	[uiIOProcSizePopUp selectItemWithTitle:[NSString stringWithFormat:@"%lu frames", frameVal]];
}

- (IBAction)iaShowTraceWindow:(id)sender
{
    [(NSNotificationCenter *)[NSNotificationCenter defaultCenter] postNotificationName:@"showTrace" object:[NSNumber numberWithFloat:_traceDisplayIndex]];
}

- (void) RunLoopDisplayUpdater
{
    Float64					testElapsedTime;
	
    // show all graphed data up to & including this point
    if (mDCM->_dataNeedsDisplayUpdate) {
		[uiDataCollectionView drawNextSlice];
		mDCM->_dataNeedsDisplayUpdate = FALSE;
	}
    
	// display elapsed time (if test is running)
	if ( [uiTestButton state] == NSOnState) {
		testElapsedTime = AudioConvertHostTimeToNanos(AudioGetCurrentHostTime() - mACM->_testStartTime) * .000000001;	// print out value in seconds 
		[uiTestTime setStringValue:[NSString stringWithFormat:@"%8.0lf", testElapsedTime, nil]];
	}
	
	// display scheduled priority
	[uiFeederThreadRetrievedPriority setIntValue:getThreadScheduledPriority (mACM->_feederThread) ];
		
	// stop test if flagged
	if (mACM->_testNeedsToStop == TRUE) {
		[uiTestButton setState:NSOffState];
		[self stopTest];
		mACM->_testNeedsToStop = FALSE;
	}
	
	// display data buffer overrun alert panel if flagged
	if ( AudioCycleManagerDataCollectionBufferOverrun(mACM) ) {
		NSRunInformationalAlertPanel(@"Data Collection Buffer Overrun", @"The data collection thread was starved.  You were probably hitting the system pretty hard for a while.  Unable to process data produced: test stopped.", @"OK", nil, nil);
	}
	
}

#pragma mark ____UTILITY ROUTINES
/* -------------------- */
/* - UTILITY ROUTINES - */
/* -------------------- */

- (void)startTest
{
	mDCM->_dataCollectionThreadShouldPause = TRUE;
	while (mDCM->_dataCollectionThreadIsRunning) usleep (1000);
	
	// destroy & reallocate displayDataBin 
	if (mDCM->_displayDataBin != NULL) {
		DataBinDealloc (mDCM->_displayDataBin);
		mDCM->_displayDataBin = NULL;
	} 
	
	mDCM->_displayDataBin = DataBinNew ([uiDataCollectionView getNumberOfDisplayablePoints], mACM->_logCPUExecutionTraceBoxChecked);
	
	// destroy DataBinCollection if it exists
	if (mDCM->_DataBinQueue != NULL) {
		DataBinCollectionDealloc (mDCM->_DataBinQueue);
		mDCM->_DataBinQueue = NULL;
	}
	
	// setup bin-writing based on granularity
	mDCM->_binGranularity = [uiDataUpdateFrequency intValue];
	if (mDCM->_binGranularity > 1) {
		// create DataBinCollection
		mDCM->_DataBinQueue = DataBinCollectionNew (50, mDCM->_binGranularity, mACM->_logCPUExecutionTraceBoxChecked);
		
		mDCM->_currentDataBin = DataBinCollectionGetNextWriteBin (mDCM->_DataBinQueue);
	} else {
		mDCM->_currentDataBin = mDCM->_displayDataBin;
	}
	
	mDCM->_dataCollectionThreadShouldPause = FALSE;
	while (!mDCM->_dataCollectionThreadIsRunning) usleep (1000);
	
	// reset data in view
	[uiDataCollectionView resetData];
	
	// set latency threshold
	mACM->_ThreadLatencyThreshold = [uiThreadLatencyThreshold intValue];
    mACM->_WorkLoopTraceThreshold = [uiWorkLoopTraceThreshold floatValue] * 0.01;	// convert from 0-100 to 0.0-1.0 here
	
	// change UI state (disabling several controls while test is running)
	[uiTestButton setTitle:@"Stop Test"];
	[self setTestParameterState:FALSE];
	
	AudioCycleManagerStart (mACM, [uiWaveformPopUp indexOfSelectedItem], [uiFrequencyBox floatValue], [uiAmplitudeSlider floatValue]);
}

- (void)stopTest
{
	AudioCycleManagerStop (mACM);
	
	[uiTestButton setTitle:@"Run Test"];
	[self setTestParameterState:TRUE];
}

- (void)setTestParameterState:(bool)inState
{
	if ((inState == TRUE) && (AudioCycleManagerGetTracingEnabled(mACM))) {
		[uiLatencyTrace setEnabled:TRUE];
		[uiWorkLoopTrace setEnabled:TRUE];
		[uiThreadLatencyThreshold setEnabled:TRUE];
		[uiWorkLoopTraceThreshold setEnabled:TRUE];
	} else {
		if (inState == FALSE) {
			[uiLatencyTrace setEnabled:FALSE];
			[uiWorkLoopTrace setEnabled:FALSE];
            [uiThreadLatencyThreshold setEnabled:FALSE];
            [uiWorkLoopTraceThreshold setEnabled:FALSE];
		}
	}
	
    [uiDeviceList setEnabled:inState];
	[uiIOProcSizePopUp setEnabled:inState];
    
    [uiWaveformPopUp setEnabled:inState];
    [uiFrequencyBox setEnabled:inState];
    [uiAmplitudeSlider setEnabled:inState];
    
	[uiSeparateThreadSelector setEnabled:inState];
	
    [uiLogToFile setEnabled:inState];
	[uiLongTermTest setEnabled:inState];

	[uiDataUpdateFrequency setEnabled:inState];
}

- (AudioCycleManager *) getACM
{
	return mACM;
}

- (DataCollectionManager *) getDCM
{
	return mDCM;
}

- (void)didOverload
{
    // need to setup our own autorelease pool here since we're creating Cocoa objects outside of the main thread
    // (in NSRunInformationalAlertPanel)
    if ([uiLongTermTest state] == NSOnState) {   
        Float64					testElapsedTime;
        NSAutoreleasePool*		myPool = [[NSAutoreleasePool alloc] init];
        
		AudioCycleManagerStop (mACM);
        mACM->_testNeedsToStop = TRUE;
        testElapsedTime = AudioConvertHostTimeToNanos(AudioGetCurrentHostTime() - mACM->_testStartTime) * .000000001;	// print out value in seconds 
        NSRunInformationalAlertPanel(@"Audio Fault", [NSString stringWithFormat:@"Occurred after %8.1lf seconds.", testElapsedTime, nil], @"OK", nil, nil);
		[myPool release];
	}
}

- (UInt32) getOwnedDCVIndex
{
	return [uiDataCollectionView getDisplayIndex];
}

- (void) aggregateNextDataBin:(DataBin *)binIn
{
	// This function should only be called for bins with AT LEAST 2 CYCLES (more than 2 is fine)
    UInt32				i;
	UInt32				valuesToAverage;
	Float64				WD, HAL, FT;
	CycleData*			cycleBeingProcessed;
	DisplayPoint*		newDP;
	
	// create new point & reset its data
	newDP = DisplayPointNew ();
    valuesToAverage = 0;
    
	if (DataBinNumCycles(binIn) > 0) {
		// prime data
		cycleBeingProcessed = DataBinGetCycleAtIndex (binIn, 0);
		if ((cycleBeingProcessed->mWorkDoneInCycle) > 0) {
			newDP->WorkDoneMin = newDP->WorkDoneMean = newDP->WorkDoneMax = cycleBeingProcessed->mWorkDoneInCycle;
			++valuesToAverage;
		} else {
			// Multiple HAL Overload case
			newDP->HALOverload = TRUE;
		}
        
		newDP->FeederThreadOverload = cycleBeingProcessed->mFeederThreadDidOverloadLastCycle;
		newDP->HALOverload = cycleBeingProcessed->mHALThreadDidOverload;
		newDP->HALLatencyMin = newDP->HALLatencyMean = newDP->HALLatencyMax = cycleBeingProcessed->mWakeUpLatencyForHAL;
		newDP->FTLatencyMin = newDP->FTLatencyMean = newDP->FTLatencyMax = cycleBeingProcessed->mWakeUpLatencyForFeederThread;
		
		// process remaining cycles
		for (i = 1; i < DataBinNumCycles (binIn); i++) {
			cycleBeingProcessed = DataBinGetCycleAtIndex (binIn, i);
			WD = cycleBeingProcessed->mWorkDoneInCycle;
			HAL = cycleBeingProcessed->mWakeUpLatencyForHAL;
			FT = cycleBeingProcessed->mWakeUpLatencyForFeederThread;
			if ((WD) > 0) {
				newDP->WorkDoneMin	=	minOf(newDP->WorkDoneMin, WD);
				newDP->WorkDoneMax	=	maxOf(newDP->WorkDoneMax, WD);
				newDP->WorkDoneMean	+=	WD;
				++valuesToAverage;
			} else {
				newDP->FeederThreadOverload = TRUE;
			}
			
			newDP->HALLatencyMin	=	minOf(newDP->HALLatencyMin, HAL);
			newDP->HALLatencyMax	=	maxOf(newDP->HALLatencyMax, HAL);
			newDP->HALLatencyMean	+=	HAL;

			newDP->FTLatencyMin		=	minOf(newDP->FTLatencyMin, FT);
			newDP->FTLatencyMax		=	maxOf(newDP->FTLatencyMax, FT);
			newDP->FTLatencyMean	+=	FT;
			
			if (cycleBeingProcessed->mFeederThreadDidOverloadLastCycle) {
				newDP->FeederThreadOverload = TRUE;
			}
			if (cycleBeingProcessed->mHALThreadDidOverload) {
				newDP->HALOverload = TRUE;
			}
		}
		// calculate means
		newDP->WorkDoneMean /= valuesToAverage;
		newDP->HALLatencyMean /= DataBinNumCycles (binIn);
		newDP->FTLatencyMean /= DataBinNumCycles (binIn);
	}
	
    // write values to file
    if ([uiLogToFile state] == NSOnState) {
        fprintf (	_logFile, "Processing Time:\t%lf\t%lf\t%lf\tFT overload:\t%d; HAL Wakeup Data:\t%lf\t%lf\t%lf, overload:\t%d; FT Wakeup Data:\t%lf\t%lf\t%lf\n",
					newDP->WorkDoneMin, newDP->WorkDoneMean, newDP->WorkDoneMax, newDP->FeederThreadOverload,
					newDP->HALLatencyMin, newDP->HALLatencyMean, newDP->HALLatencyMax, newDP->HALOverload,
					newDP->FTLatencyMin, newDP->FTLatencyMean, newDP->FTLatencyMax);
    }
    
    // add data points to path
    [uiDataCollectionView addDisplayPoint:newDP];
	
	// handle long-term test trip
	if (newDP->FeederThreadOverload && (mACM->_HALCyclesRun > 15)) {
		[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] postNotificationName:@"didOverload" object:nil];
	}
}

- (void)_directDataWriteFromBin:(DataBin *)binIn startPoint:(UInt32)startPoint numPoints:(UInt32)delta
{
	UInt32				i;
	CycleData*			CDProcessing;
	DisplayPoint*		newDP;
	
	for (i = startPoint; i < (startPoint + delta); i++) {
		newDP = DisplayPointNew ();
		newDP->HALOverload = newDP->FeederThreadOverload = FALSE;
		CDProcessing = DataBinGetCycleAtIndex (binIn, i);
		
		if (CDProcessing->mHALThreadDidOverload == TRUE) {
			newDP->HALOverload = TRUE;
		}
		if ( CDProcessing->mHALThreadTraceDidOverload == TRUE ) {
			newDP->HALTraceOverload = TRUE;
		}
		
		if ( (CDProcessing->mFeederThreadDidOverloadLastCycle == TRUE) || (CDProcessing->mWorkDoneInCycle < 0) ) {
			newDP->FeederThreadOverload = TRUE;
			newDP->WorkDoneMin = newDP->WorkDoneMean = newDP->WorkDoneMax = 0;
		} else {
			newDP->WorkDoneMin = newDP->WorkDoneMean = newDP->WorkDoneMax = CDProcessing->mWorkDoneInCycle;
		}
		
		if ( CDProcessing->mFeederThreadNeverRan == TRUE ) {
			newDP->FeederThreadNeverRan = TRUE;
		}
		
		if ( CDProcessing->mFeederThreadTraceDidOverload == TRUE ) {
			newDP->FeederThreadTraceOverload = TRUE;
		}
		
		if ( CDProcessing->mWorkLoopTraceDidOverload == TRUE ) {
			newDP->WorkTraceOverload = TRUE;
		}
		
		newDP->HALLatencyMin = newDP->HALLatencyMean = newDP->HALLatencyMax = CDProcessing->mWakeUpLatencyForHAL;
		newDP->FTLatencyMin = newDP->FTLatencyMean = newDP->FTLatencyMax = CDProcessing->mWakeUpLatencyForFeederThread;
		// write values to file
		if ([uiLogToFile state] == NSOnState) {
			fprintf (_logFile, "Processing Time:\t%lf;\toverload:\t%d; HAL Wakeup Data:\t%lf;\toverload:\t%d; FT Wakeup Data:\t%lf\n",
			newDP->WorkDoneMean, newDP->FeederThreadOverload,
			newDP->HALLatencyMean, newDP->HALOverload,
			newDP->FTLatencyMean);
		}
				
		// add data points to path
		[uiDataCollectionView addDisplayPoint:newDP];
	
            // handle long-term test trip
            if (newDP->FeederThreadOverload && (mACM->_HALCyclesRun > 15)) {
                    [(NSNotificationCenter *)[NSNotificationCenter defaultCenter] postNotificationName:@"didOverload" object:nil];
            }
	}

}

- (void)logStateData
{
	fprintf (_logFile, "/*---------------------\n");
	if (mACM->_feederThreadLoadingStyle == TIME_LOADING) {
		fprintf (_logFile, "Test type: TIME loading\n");
	} else {
		fprintf (_logFile, "Test type: WORK loading\n");
	}
	
	fprintf (_logFile, "IOProc size: %lu frames\n", mACM->_BufferSizeInFrames);
	fprintf (_logFile, "---------------------*/\n");\
}

- (void) windowWillClose:(NSNotification *)inNotification
{
	if (_logFile) {
		fclose (_logFile);
	}
	[NSApp terminate:self];
}

@end

Float64 minOf (Float64 a, Float64 b)
{
	if (a < b)
		return a;
	return b;
}

Float64 maxOf (Float64 a, Float64 b)
{
	if (a > b)
		return a;
	return b;
}


/*
 */
