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
/* MillionMonkeys.h */
#import <Cocoa/Cocoa.h>

#import "DataCollectionView.h"
#import "DataBinCollection.h"

#import "AudioCycleManager.h"
#import "DataCollectionManager.h"
#import "UnitsView.h"

@class MillionMonkeys;
@class MMInfoView;

@interface MillionMonkeys : NSObject
{
 /* GUI members */
	IBOutlet NSWindow					*window;
    
    IBOutlet NSPopUpButton				*uiDeviceList;
	IBOutlet NSPopUpButton				*uiIOProcSizePopUp;
    IBOutlet NSTextField				*uiSafetyOffset;
    
    IBOutlet NSPopUpButton				*uiWaveformPopUp;
    IBOutlet NSComboBox					*uiFrequencyBox;
    IBOutlet NSSlider					*uiAmplitudeSlider;
    
    IBOutlet NSMatrix					*uiSelectedTest;
	IBOutlet NSButton					*uiTestButton;
	IBOutlet NSTextField				*uiTestTime;
    
	IBOutlet NSBox						*uiTestTypeContainer;
	IBOutlet NSBox						*uiTestTypeTimeConsole;
	IBOutlet NSBox						*uiTestTypeWorkConsole;
	IBOutlet NSSlider					*uiFeederThreadLoadingSlider;
	IBOutlet NSTextField				*uiFeederThreadLoading;
	IBOutlet NSTextField				*uiFeederThreadCPUWorkCycles;
	
	IBOutlet NSMatrix					*uiSeparateThreadSelector;
	IBOutlet NSMatrix					*uiFTPolicy;
    IBOutlet NSTextField				*uiFTPolicySetTitle;
    IBOutlet NSPopUpButton				*uiFTPriority;
	IBOutlet NSTextField				*uiFTPriorityGetTitle;
	IBOutlet NSTextField				*uiFTPrioritySetTitle;
    IBOutlet NSTextField				*uiFeederThreadRetrievedPriority;
	
	IBOutlet NSButton					*uiLatencyTrace;
	IBOutlet NSButton					*uiWorkLoopTrace;
	IBOutlet NSButton					*uiLongTermTest;
	IBOutlet NSButton					*uiLogToFile;
	IBOutlet NSTextField				*uiDataUpdateFrequency;
	IBOutlet NSTextField				*uiDisplayUpdateFrequency;
    IBOutlet NSTextField				*uiThreadLatencyThreshold;
    IBOutlet NSTextField				*uiWorkLoopTraceThreshold;
        
	IBOutlet DataCollectionView			*uiDataCollectionView;
    IBOutlet UnitsView					*uiUnitsView;
    
	IBOutlet NSMatrix					*uiItemDisplayList;
	IBOutlet NSButton					*uiOverloadSwitch;
	IBOutlet NSSlider					*uiHALScalar;
	IBOutlet NSSlider					*uiFTScalar;
	IBOutlet NSSlider					*uiWorkScalar;
    IBOutlet NSMatrix					*uiDisplayUnitsSelector;
                        
	/* Trace panel gui members  */
			 NSWindowController			*uiConsoleController;
	IBOutlet NSPanel					*uiConsolePanel;
    IBOutlet NSTextField				*uiTimeSlice;
	IBOutlet NSTextField				*uiPanelWorkTraceOVL;
	IBOutlet NSTextField				*uiPanelHALTraceOVL;
	IBOutlet NSTextField				*uiPanelFTTraceOVL;
    IBOutlet NSTextField				*uiPanelFTBlocked;
	IBOutlet NSTextField				*uiPanelHALOVL;
	IBOutlet NSTextField				*uiPanelFTOVL;
	IBOutlet NSTextField				*uiTraceIndex;
	IBOutlet NSButton					*uiTracePrevButton;
	IBOutlet NSButton					*uiTraceNextButton;
	IBOutlet NSTextField				*uiPanelWorkloadTitle;
	IBOutlet NSTextField				*uiPanelWorkDone;
	IBOutlet NSTextField				*uiPanelWorkloadUnits;
	IBOutlet NSTextField				*uiPanelWakeupOffset;
	IBOutlet NSTextField				*uiPanelFTLatency;
    IBOutlet NSMatrix					*uiTraceSelectionMatrix;
	IBOutlet NSTextView					*uiPanelTraceView;
	
	/* MMInfoDrawer */
	IBOutlet NSDrawer					*uiMMInfoDrawer;
	IBOutlet MMInfoView					*uiMMInfoView;
	
	/* other members */
	AudioCycleManager					*mACM;
	DataCollectionManager				*mDCM;

	NSColor								*mHALOVLTextColor;
	NSColor								*mFTOVLTextColor;
    NSColor								*mFTBlockedTextColor;
	NSColor								*mWorkOVLTextColor;
	NSColor								*mGreyedOutTextColor;
	
	int									_traceDisplayIndex;
	UInt32								_dataCollectionCounter;
	Boolean								_displayMask[9];
	Boolean								_displayOverload;
}

/* METHOD DECLARATIONS */
- (id)init;
- (void)dealloc;


- (IBAction)iaTestButtonPressed:(id)sender;
- (IBAction)iaSelectedDeviceChanged:(id)sender;
- (IBAction)iaIOProcSizeChanged:(id)sender;
- (IBAction)iaSeparateThreadSelectorChanged:(id)sender;
- (IBAction)iaFeederThreadCPULoadingChanged:(id)sender;
- (IBAction)iaTraceSelectionChanged:(id)sender;
- (IBAction)iaDisplayUpdateFrequencyChanged:(id)sender;
- (IBAction)iaFTPolicyChanged:(id)sender;
- (IBAction)iaFTPriorityChanged:(id)sender;
- (void)_setFTBasedOnUISettings;

- (AudioCycleManager *) getACM;
- (DataCollectionManager *) getDCM;

- (UInt32) getOwnedDCVIndex;
- (void) aggregateNextDataBin:(DataBin *)binIn;
- (IBAction)iaItemDisplayListChanged:(id)sender;
- (IBAction)iaScalarChanged:(id)sender;
- (IBAction)iaDisplayUnitsSelectorChanged:(id)sender;

- (IBAction)iaPanelTraceTypeSelectionChanged:(id)sender;
- (IBAction)iaPanelPrevTrace:(id)sender;
- (IBAction)iaPanelNextTrace:(id)sender;
- (void)_populateDeviceList;
- (void)_populateIOProcSize;

- (IBAction)iaShowTraceWindow:(id)sender;

- (void)didOverload;

- (void)startTest;
- (void)stopTest;
- (void)setTestParameterState:(bool)inState;
- (void)logStateData;
- (void)showTrace:(NSNotification *)inNotification;
- (BOOL)_populateTraceInfo:(int)traceIndexIn;
- (void)_showTrace;
- (void)windowWillClose:(NSNotification *)inNotification;
- (void) RunLoopDisplayUpdater;
- (void)_directDataWriteFromBin:(DataBin *)binIn startPoint:(UInt32)startPoint numPoints:(UInt32)delta;

@end

/* C function decl */
Boolean notifyOnError (int fctnResult, char* errMsg);
Float64 minOf (Float64 a, Float64 b);
Float64 maxOf (Float64 a, Float64 b);

/*
 */
