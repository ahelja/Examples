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
#import <CoreAudioKit/CoreAudioKit.h>
#import <AudioUnit/AUCocoaUIView.h>

#include "CAComponent.h"
#include "CAComponentDescription.h"
#include "AudioFilePlay.h"

#import "CAUHWindowController.h"

#import "AudioFileListView.h"


void AudioFileNotificationHandler (void *inRefCon, OSStatus inStatus)
{
    HostingWindowController *SELF = (HostingWindowController *)inRefCon;
    [SELF performSelectorOnMainThread:@selector(iaPlayStopButtonPressed:) withObject:SELF waitUntilDone:NO];
}

int componentCountForAUType(OSType inAUType)
{
	CAComponentDescription desc = CAComponentDescription(inAUType);
	return desc.Count();
}

void getComponentsForAUType(OSType inAUType, CAComponent *ioCompBuffer, int count)
{
	CAComponentDescription desc = CAComponentDescription(inAUType);
	CAComponent *last = NULL;
	
	for (int i = 0; i < count; ++i) {
		ioCompBuffer[i] = CAComponent(desc, last);
		last = &(ioCompBuffer[i]);
	}
}

@implementation HostingWindowController
+ (BOOL)plugInClassIsValid:(Class) pluginClass
{
	if ([pluginClass conformsToProtocol:@protocol(AUCocoaUIBase)]) {
		if ([pluginClass instancesRespondToSelector:@selector(interfaceVersion)] &&
			[pluginClass instancesRespondToSelector:@selector(uiViewForAudioUnit:withSize:)]) {
			return YES;
		}
	}
	
    return NO;
}

- (void)showCocoaViewForAU:(AudioUnit)inAU
{
	// get AU's Cocoa view property
    UInt32 						dataSize;
    Boolean 					isWritable;
    AudioUnitCocoaViewInfo *	cocoaViewInfo = NULL;
    UInt32						numberOfClasses;
    
    OSStatus result = AudioUnitGetPropertyInfo(	inAU,
                                                kAudioUnitProperty_CocoaUI,
                                                kAudioUnitScope_Global, 
                                                0,
                                                &dataSize,
                                                &isWritable );
    
    numberOfClasses = (dataSize - sizeof(CFURLRef)) / sizeof(CFStringRef);
    
    NSURL 	 *	CocoaViewBundlePath = nil;
    NSString *	factoryClassName = nil;
    
	// Does view have custom Cocoa UI?
    if ((result == noErr) && (numberOfClasses > 0) ) {
        cocoaViewInfo = (AudioUnitCocoaViewInfo *)malloc(dataSize);
        if(AudioUnitGetProperty(		inAU,
                                        kAudioUnitProperty_CocoaUI,
                                        kAudioUnitScope_Global,
                                        0,
                                        cocoaViewInfo,
                                        &dataSize) == noErr) {
            CocoaViewBundlePath	= (NSURL *)cocoaViewInfo->mCocoaAUViewBundleLocation;
			
			// we only take the first view in this example.
            factoryClassName	= (NSString *)cocoaViewInfo->mCocoaAUViewClass[0];
        } else {
            if (cocoaViewInfo != NULL) {
				free (cocoaViewInfo);
				cocoaViewInfo = NULL;
			}
        }
    }
	
	NSView *AUView = nil;
	BOOL wasAbleToLoadCustomView = NO;
	
	// [A] Show custom UI if view has it
	if (CocoaViewBundlePath && factoryClassName) {
		NSBundle *viewBundle  	= [NSBundle bundleWithPath:[CocoaViewBundlePath path]];
		if (viewBundle == nil) {
			NSLog (@"Error loading AU view's bundle");
		} else {
			Class factoryClass = [viewBundle classNamed:factoryClassName];
			NSAssert (factoryClass != nil, @"Error getting AU view's factory class from bundle");
			
			// make sure 'factoryClass' implements the AUCocoaUIBase protocol
			NSAssert(	[HostingWindowController plugInClassIsValid:factoryClass],
						@"AU view's factory class does not properly implement the AUCocoaUIBase protocol");
			
			// make a factory
			id factoryInstance = [[[factoryClass alloc] init] autorelease];
			NSAssert (factoryInstance != nil, @"Could not create an instance of the AU view factory");
			// make a view
			AUView = [factoryInstance	uiViewForAudioUnit:inAU
										withSize:[[mScrollView contentView] bounds].size];
			
			// cleanup
			[CocoaViewBundlePath release];
			if (cocoaViewInfo) {
				UInt32 i;
				for (i = 0; i < numberOfClasses; i++)
					CFRelease(cocoaViewInfo->mCocoaAUViewClass[i]);
				
				free (cocoaViewInfo);
			}
			wasAbleToLoadCustomView = YES;
		}
	}
	
	if (!wasAbleToLoadCustomView) {
		// [B] Otherwise show generic Cocoa view
		AUView = [[AUGenericView alloc] initWithAudioUnit:inAU];
		[(AUGenericView *)AUView setShowsExpertParameters:YES];
    }
	
	// Display view
	NSRect viewFrame = [AUView frame];
	NSSize frameSize = [NSScrollView	frameSizeForContentSize:viewFrame.size
										hasHorizontalScroller:[mScrollView hasHorizontalScroller]
										hasVerticalScroller:[mScrollView hasVerticalScroller]
										borderType:[mScrollView borderType]];
	
	NSRect newFrame;
	newFrame.origin = [mScrollView frame].origin;
	newFrame.size = frameSize;
	
	NSRect currentFrame = [mScrollView frame];
	[mScrollView setFrame:newFrame];
	[mScrollView setDocumentView:AUView];
	
	NSSize oldContentSize = [[[self window] contentView] frame].size;
	NSSize newContentSize = oldContentSize;
	newContentSize.width += (newFrame.size.width - currentFrame.size.width);
	newContentSize.height += (newFrame.size.height - currentFrame.size.height);
	
	[[self window] setContentSize:newContentSize];
}

- (void)synchronizePlayStopButton
{
    if (mComponentHostType == kAudioUnitType_Effect) {
        [uiPlayStopButton setEnabled:[mAudioFileList count] > 0];
    } else {
        [uiPlayStopButton setEnabled:YES];
    }
}

- (void)synchronizeForNewAUType
{
    // [A] what is new AUType?
    int selectedRow = [uiAUTypeMatrix selectedRow];
    mComponentHostType = (selectedRow == 0) ? kAudioUnitType_Generator : kAudioUnitType_Effect;
    
    // [B] sync with new AUType
    //   [1] get new AUList
	if (mAUList != NULL) {
		free (mAUList);
		mAUList = NULL;
	}
	
	int componentCount = componentCountForAUType(mComponentHostType);
	UInt32 dataByteSize = componentCount * sizeof(CAComponent);
	mAUList = static_cast<CAComponent *>(malloc(dataByteSize));
	memset (mAUList, 0, dataByteSize);
	getComponentsForAUType(mComponentHostType, mAUList, componentCount);
	
	//   [2] populate AUPopUp with new list
    [uiAUPopUpButton removeAllItems];
	
	for (int i = 0; i < componentCount; ++i) {
		[uiAUPopUpButton addItemWithTitle:(NSString *)(mAUList[i].GetAUName())];
	}
    
    //   [3] enable AudioFileDrawerToggle button for effects
    if (mComponentHostType == kAudioUnitType_Effect) {
        [uiAudioFileButton setEnabled:YES];
    } else {
        [uiAudioFileButton setEnabled:NO];
        [(NSDrawer *)[[[self window] drawers] objectAtIndex:0] close];
    }
    
    //   [4] other UI
    [self synchronizePlayStopButton];
    
    //   [5] select top-of-list AU & show its UI
    [self iaAUPopUpButtonPressed:self];  
}

- (void)addLinkToFiles:(NSArray *)inFiles
{
    [mAudioFileList addObjectsFromArray:inFiles];
    [self synchronizePlayStopButton];
    [uiAudioFileTableView reloadData];
}

- (void)createGraph
{
	verify_noerr (NewAUGraph(&mGraph));
	
	CAComponentDescription desc = CAComponentDescription (	kAudioUnitType_Output,
															kAudioUnitSubType_DefaultOutput,
															kAudioUnitManufacturer_Apple	);
    
	verify_noerr (AUGraphNewNode(mGraph, &desc, 0, NULL, &mOutputNode));
	verify_noerr (AUGraphOpen(mGraph));
    verify_noerr (AUGraphGetNodeInfo(mGraph, mOutputNode, NULL, NULL, NULL, &mOutputUnit));
}

- (void)startGraph
{
    verify_noerr (AUGraphConnectNodeInput (mGraph, mTargetNode, 0, mOutputNode, 0));
    verify_noerr (AUGraphUpdate (mGraph, NULL) == noErr);
    verify_noerr (AUGraphInitialize(mGraph) == noErr);
    verify_noerr (AUGraphStart(mGraph) == noErr);
}

- (void)stopGraph
{
	verify_noerr (AUGraphStop(mGraph));
	verify_noerr (AUGraphUninitialize(mGraph));
	verify_noerr (AUGraphDisconnectNodeInput (mGraph, mOutputNode, 0));
	verify_noerr (AUGraphUpdate (mGraph, NULL));
	if (mAFPID) {
		verify_noerr (AFP_Disconnect(mAFPID));
		verify_noerr (DisposeAudioFilePlayID(mAFPID));
		[uiAudioFileNowPlayingName setStringValue:@""];
		mAFPID = NULL;
	}
}

- (void)destroyGraph
{
	// stop graph if necessary
    Boolean isRunning = FALSE;
	verify_noerr (AUGraphIsRunning(mGraph, &isRunning));
	if (isRunning)
		[self stopGraph];
	
	// close and destroy
	verify_noerr (AUGraphClose(mGraph));
	verify_noerr (DisposeAUGraph(mGraph));
}

- (void)loadAudioFile:(NSString *)inAudioFileName
{
	FSRef destFSRef;
	UInt8 *pathName = (UInt8 *)[inAudioFileName cString];
	verify_noerr (FSPathMakeRef(pathName, &destFSRef, NULL));
	
	verify_noerr (NewAudioFilePlayID(&destFSRef, &mAFPID));
	verify_noerr (AFP_SetNotifier(mAFPID, AudioFileNotificationHandler, self));
	
	verify_noerr (AFP_SetDestination(mAFPID, mTargetUnit, 0));
	verify_noerr (AFP_Connect(mAFPID));
}

- (void)awakeFromNib
{
    mAudioFileList = [[NSMutableArray alloc] init];
    
    // create scroll-view
    NSRect frameRect = [[uiAUViewContainer contentView] frame];
    mScrollView = [[[NSScrollView alloc] initWithFrame:frameRect] autorelease];
    [mScrollView setDrawsBackground:NO];
    [mScrollView setHasHorizontalScroller:YES];
    [mScrollView setHasVerticalScroller:YES];
    [uiAUViewContainer setContentView:mScrollView];
    
    // dispatched setup
    [self createGraph];
    [self synchronizeForNewAUType];
    
	// make this the app. delegate
	[NSApp setDelegate:self];
}

-(void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
	if (mAUList != NULL) {
		free(mAUList);
		mAUList = NULL;
	}
	
    [mAudioFileList release];
    
    if (mAFPID)
		verify_noerr(DisposeAudioFilePlayID(mAFPID));
    
    [self destroyGraph];
	
	[super dealloc];
}

- (IBAction)iaAUTypeChanged:(id)sender
{
    [self synchronizeForNewAUType];
}

- (IBAction)iaAUPopUpButtonPressed:(id)sender
{
    // replace effect AU in chain
	int index = [uiAUPopUpButton indexOfSelectedItem];
	ComponentDescription desc = mAUList[index].Desc();//[[mAUList objectAtIndex:index] componentDescription];
	
	if (mTargetNode) {
			// remove the old view first before closing the AU
		[[mScrollView documentView] removeFromSuperview];
		verify_noerr (AUGraphRemoveNode(mGraph, mTargetNode));
    }
	
    verify_noerr (AUGraphNewNode(mGraph, &desc, 0, NULL, &mTargetNode));
	verify_noerr (AUGraphGetNodeInfo(mGraph, mTargetNode, NULL, NULL, NULL, &mTargetUnit));
    verify_noerr (AUGraphUpdate (mGraph, NULL));
    
	[self showCocoaViewForAU:mTargetUnit];
}

- (IBAction)iaPlayStopButtonPressed:(id)sender
{
    if (sender == self) {
        // change button icon manually if this function is called internally
        [uiPlayStopButton setState:([uiPlayStopButton state] == NSOffState) ? NSOnState : NSOffState];
    }
    
    Boolean isRunning = FALSE;
	verify_noerr (AUGraphIsRunning(mGraph, &isRunning));
	
	// [1] if the AUGraph is running, stop it
    if (isRunning) {
        // stop graph, update UI & return
		[self stopGraph];
		
		[uiAUTypeMatrix setEnabled:YES];
        [uiAUPopUpButton setEnabled:YES];
        return;
    }
    
	// [2] otherwise start the AUGraph
    // load file
    if (mComponentHostType == kAudioUnitType_Effect) {
		int selectedRow = [uiAudioFileTableView selectedRow];
		if ( (selectedRow < 0) || ([mAudioFileList count] == 0) ) return;	// no file selected
		
		NSString *audioFileName = (NSString *)[mAudioFileList objectAtIndex:selectedRow];
		[self loadAudioFile:audioFileName];
		
        // set filename in UI
        [uiAudioFileNowPlayingName setStringValue:[audioFileName lastPathComponent]];
    }
	
    [uiAUTypeMatrix setEnabled:NO];
    [uiAUPopUpButton setEnabled:NO];
    
	[self startGraph];
}

- (int)numberOfRowsInTableView:(NSTableView *)inTableView
{
    int count = [mAudioFileList count];
    return (count > 0) ? count : 1;
}

- (id)tableView:(NSTableView *)inTableView objectValueForTableColumn:(NSTableColumn *)inTableColumn row:(int)inRow
{
    int count = [mAudioFileList count];
    return (count > 0) ?	[(NSString *)[mAudioFileList objectAtIndex:inRow] lastPathComponent] :
                            @"< drag audio files here >";
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)inSender
{
	return YES;
}

@end
