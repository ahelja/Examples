/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
 *  AUPresetManager.m
 *  PlayPen
 *-----------------------------------------------------------------------------
 *
 *
 * Revision 1.8  2005/03/09 22:42:22  dwyatt
 * don't leak presetToSet.presetName in itemWasActivated
 *
 * Revision 1.7  2005/03/09 01:38:13  bills
 * fix setting preset calls
 *
 * Revision 1.6  2005/03/08 02:19:27  dwyatt
 * don't leak CFPropertyListRefs
 *
 * Revision 1.5  2004/11/16 17:35:35  luke
 * clean up code
 *
 * Revision 1.4  2004/11/13 17:46:17  luke
 * removed NSAsserts & better handling errors
 *
 * Revision 1.3  2004/11/10 03:34:24  bills
 * delete on NULL is ok
 *
 * Revision 1.2  2004/11/10 03:32:52  bills
 * fix dealloc method
 *
 * Revision 1.1  2004/07/29 00:21:15  luke
 * factored out from PlayPen
 *
 * Revision 1.12  2004/07/29 00:08:51  luke
 * tweaks
 *
 * Revision 1.11  2004/07/28 23:57:20  luke
 * pass 3: final
 *
 * Revision 1.9  2004/07/28 20:49:24  luke
 * align with changes to CAFileBrowser
 *
 * Revision 1.8  2004/07/07 00:24:16  luke
 * [3482090] make AUInspector localizable
 *
 * Revision 1.7  2004/06/24 23:20:22  mhopkins
 * added method to return name and data key strings
 *
 * Revision 1.6  2004/05/27 22:56:08  luke
 * don't allow Factory presets to be renamed
 *
 * Revision 1.5  2004/05/26 19:42:47  luke
 * don't show factory preset option if unit doesn't have any factory presets
 *
 * Revision 1.4  2004/05/26 17:13:01  luke
 * [3656248] only search Network for presets when preferences say to do so
 *
 * Revision 1.3  2004/05/25 22:23:36  luke
 * sync. with changes to superclass & deallocate correctly
 *
 * Revision 1.2  2004/04/29 23:08:39  luke
 * fix sync w/ interface changes to AUInspector
 *
 * Revision 1.1  2004/04/17 20:05:20  luke
 * source reorg.
 *
 * Revision 1.22  2003/12/18 19:48:29  luke
 * push directory-creation code into CAFileBrowser superclass
 *
 * Revision 1.21  2003/12/02 01:17:19  luke
 * CAFileHandling factored
 *
 * Revision 1.20  2003/11/20 00:58:31  crogers
 * check factoryPresets for NULL
 *
 * Revision 1.19  2003/11/12 23:58:02  luke
 * tweak includes
 *
 * Revision 1.18  2003/11/06 19:25:04  luke
 * change title: 'Presets' -> 'AudioUnit Presets
 *
 * Revision 1.17  2003/10/30 19:08:08  luke
 * fix for NetInfo user dirs
 *
 * Revision 1.16  2003/10/22 20:21:39  luke
 * don't notify on presetChange: use propListeners
 *
 * Revision 1.15  2003/10/21 23:28:26  luke
 * added features that future subclasses may need
 *
 * Revision 1.14  2003/10/21 22:03:57  luke
 * change some behavior per discussions with Bill
 *
 * Revision 1.13  2003/10/21 18:53:52  luke
 * call self, not super
 *
 * Revision 1.12  2003/10/17 22:39:14  luke
 * implement preset saving
 *
 * Revision 1.11  2003/10/15 16:56:43  luke
 * tighten up code.
 *
 * Revision 1.10  2003/10/15 00:46:04  luke
 * tighten up code
 *
 * Revision 1.9  2003/10/15 00:12:50  luke
 * factor general support into CAFileBrowser class
 *
 * Revision 1.8  2003/10/14 19:18:32  luke
 * sync. with changes to CAFileHandling
 *
 * Revision 1.7  2003/10/14 18:18:19  luke
 * sync. with requested changes to CAFileHandling
 *
 * Revision 1.6  2003/10/14 17:25:35  luke
 * fix warnings
 *
 * Revision 1.5  2003/10/14 17:21:59  luke
 * now using CAFileHandling class
 *
 * Revision 1.4  2003/09/24 21:07:28  luke
 * alert AUParameterListeners when we load a preset
 *
 * Revision 1.3  2003/09/24 06:39:37  luke
 * finish implementation of preset browser
 *
 * Revision 1.2  2003/09/23 20:46:44  luke
 * updates
 *
 * Revision 1.1  2003/09/23 17:22:01  luke
 * AUPresetManager.m -> AUPresetManager.mm
 *
 * Revision 1.1  2003/09/23 00:58:22  luke
 * new file
 *
 *
 *-----------------------------------------------------------------------------
 *  Created by Luke Bellandi on Mon Sep 22 2003.
 *  Copyright (c) 2003 Apple Computer Inc. All rights reserved.
 *=============================================================================*/

#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>

#import "CAComponent.h"
#import "CAAUPresetFile.h"

#import "AUPresetManager.h"
#import "CAFileBrowser_Protected.h"

@implementation AUPresetManager

#pragma mark ____ PRIVATE FUNCTIONS ____
- (void)privGetFactoryPresets {
    // dump old presets
    [mFactoryPresets removeAllObjects];
    
	// can't do anything if no AU selected
 	if (mAudioUnit == NULL) return;
	
    NSMutableArray *presetArray = [NSMutableArray array];
	[mFactoryPresets setObject:presetArray forKey:NSLocalizedStringFromTable(@"Factory", @"AUInspector", @"Factory Preset group heading")];
    
    // get new presets    
    CFArrayRef factoryPresets = NULL;
    UInt32 dataSize = sizeof(factoryPresets);
    
	ComponentResult result = AudioUnitGetProperty (	mAudioUnit,
													kAudioUnitProperty_FactoryPresets,
													kAudioUnitScope_Global,
													0,
													&factoryPresets,
													&dataSize	);
	
    if ((result == noErr) && (factoryPresets != NULL)) {
        int count = CFArrayGetCount(factoryPresets);
        AUPreset *		currentPreset;
        
        for (int i = 0; i < count; ++i) {
            currentPreset = (AUPreset*) CFArrayGetValueAtIndex (factoryPresets, i);
            [presetArray addObject:[NSDictionary dictionaryWithObject:[NSNumber numberWithLong:currentPreset->presetNumber] forKey:(NSString *)(currentPreset->presetName)]];
        }
        
        CFRelease(factoryPresets);
    }
}

- (BOOL)privHasFactoryPresets {
	NSArray *valueArray = [mFactoryPresets allValues];
	if ([valueArray count] <= 0) return NO;
	NSArray *factoryPresetArray = (NSArray *)[valueArray objectAtIndex:0];
	return [factoryPresetArray count] > 0;
}

#pragma mark ____ INIT / DEALLOC ____
- (id)initWithFrame:(NSRect)frame shouldScanNetworkForFiles:(BOOL)shouldScanNetworkForFiles {
	[super initWithFrame:frame shouldScanNetworkForFiles:shouldScanNetworkForFiles];
	
	[self setTitle:NSLocalizedStringFromTable(@"AudioUnit Presets", @"AUInspector", @"Audio Unit Preset manager title")];
    
    mFactoryPresets = [[NSMutableDictionary alloc] init];
        
    return self;
}

- (id)initWithFrame:(NSRect)frame {
    return [self initWithFrame:frame shouldScanNetworkForFiles:NO];
}

- (void)dealloc {
    [mFactoryPresets release];
	delete mPresetFile; // can call delete on NULL
	[super dealloc];
}

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU {
    mAudioUnit = inAU;
    
	[self unsetCAFileHandlingObject];
    if (mPresetFile) delete mPresetFile;
    mPresetFile = new CAAUPresetFile (CAComponent(mAudioUnit), [self scansNetworkForFiles]);
    
    [self privGetFactoryPresets];
	[self setCAFileHandlingObject:mPresetFile];
	[self reloadData];
}

- (BOOL)savePresetWithName:(NSString *)inPresetName asChildOf:(id)item {
    AUPreset presetToSet;
    
    presetToSet.presetName = (CFStringRef)inPresetName;
    presetToSet.presetNumber = -1;
    
    ComponentResult result = AudioUnitSetProperty (	mAudioUnit,
                                                    kAudioUnitProperty_PresentPreset,
                                                    kAudioUnitScope_Global,
                                                    0,
                                                    &presetToSet,
                                                    sizeof(AUPreset)	);
    if (result != noErr) {
        result = AudioUnitSetProperty (	mAudioUnit,
                                        kAudioUnitProperty_CurrentPreset,
                                        kAudioUnitScope_Global,
                                        0,
                                        &presetToSet,
                                        sizeof(AUPreset)	);
        if (result != noErr)
			return NO;
    }
    
    // now get classInfo & write to file
    CFPropertyListRef classInfo = NULL;
    UInt32 dataSize = sizeof(CFPropertyListRef);
    result = AudioUnitGetProperty (	mAudioUnit,
                                    kAudioUnitProperty_ClassInfo,
                                    kAudioUnitScope_Global,
                                    0,
                                    &classInfo,
                                    &dataSize);
    if (result != noErr)
		return NO;
    
    BOOL retVal = [self writePropertyList:classInfo withName:inPresetName asChildOfItem:item];
	
    if (classInfo) CFRelease (classInfo);
	
	[self rescanFiles];
	
	return retVal;
}

- (void)rescanFiles {
    [self setAU:mAudioUnit];
}

#pragma mark ____ PROTECTED FUNCTIONS ____
- (BOOL)itemWasActivated:(id)object 
{
    if ([object isKindOfClass:[NSDictionary class]])
	{
		// FACTORY PRESETS
        id obj = [[(NSDictionary *)object allValues] objectAtIndex:0];
        if (![obj isKindOfClass:[NSNumber class]]) return NO;
		
		NSString *presetName = (NSString *)[[(NSDictionary *)object allKeys] objectAtIndex:0];
		UInt32 presetNumber = [(NSNumber *)obj longValue];
		
		AUPreset presetToSet;
		presetToSet.presetName = (CFStringRef)presetName;
		presetToSet.presetNumber = presetNumber;
		
		ComponentResult result = AudioUnitSetProperty (	mAudioUnit,
														kAudioUnitProperty_PresentPreset,
														kAudioUnitScope_Global,
														0,
														&presetToSet,
														sizeof(AUPreset)	);
		if (result != noErr) {
			// try old 'currentPreset' property if presentPreset doesn't work
			result = AudioUnitSetProperty (	mAudioUnit,
											kAudioUnitProperty_CurrentPreset,
											kAudioUnitScope_Global,
											0,
											&presetToSet,
											sizeof(AUPreset)	);
			if (result != noErr)
				return NO;
		}
    }
	else
	{
		// LOCAL/USER/NETWORK PRESETS
		CFPropertyListRef preset = [self readPropertyListForItem:object];
		if (preset == NULL) return NO;
		
		// set preset in AU
		AUPreset presetToSet;
		
		CFTreeRef tree = (CFTreeRef)[(NSValue *)object pointerValue];
		mPresetFile->GetNameCopy(tree, presetToSet.presetName);
		presetToSet.presetNumber = -1;
		
		ComponentResult result = AudioUnitSetProperty (	mAudioUnit,
														kAudioUnitProperty_ClassInfo,
														kAudioUnitScope_Global,
														0,
														&preset,
														sizeof(CFPropertyListRef));
		CFRelease(preset);
		CFRelease(presetToSet.presetName);
		if (result != noErr)
			return NO;
	}
	
	// notify parameter listeners that they should re-scan unit
	AudioUnitParameter changedUnit;
	changedUnit.mAudioUnit = mAudioUnit;
	changedUnit.mParameterID = kAUParameterListener_AnyParameter;
	if (AUParameterListenerNotify (NULL, NULL, &changedUnit) != noErr)
		return NO;
	
	return YES;
}

- (NSString *)fileExtension
{
	return (NSString *)CAAUPresetFile::kAUPresetFileExtension;
}

- (NSString *)nameKeyString
{
	return (NSString *)CAAUPresetFile::kAUPresetNameKeyString;
}

- (BOOL)shouldAllowItemRenaming:(id)inItem
{
	return (![inItem isKindOfClass:[NSDictionary class]]);
}

#pragma mark ____NSOutlineView.DataSource overrides____
// override outlineView dataSource methods so we can integrate factory presets
- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item {
    // TOP LEVEL?
    if (item == nil) {
		if ([self privHasFactoryPresets]) {
			if (index == 0) return mFactoryPresets;
			return [super outlineView:outlineView child:(index - 1) ofItem:item];
		} else {
			return [super outlineView:outlineView child:index ofItem:item];
		}
    }
    
    // FACTORY PRESET?
    if ([item isKindOfClass:[NSDictionary class]]) {
		NSArray *array = [(NSDictionary*)item allValues];
		if ([array count] <= 0) return nil;
        id obj = [array objectAtIndex:0];
        
        if ([obj isKindOfClass:[NSArray class]])
            return [(NSArray *)obj objectAtIndex:index];
        else
            return obj;	// NSDictionary *
    }
    
    // ELSE SUPERCLASS HANDLING
    return [super outlineView:outlineView child:index ofItem:item];
}

- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item {
    // TOP LEVEL?
    if (item == nil) {
		int factoryOffset = ([self privHasFactoryPresets] > 0) ? 1 : 0;
        return [super outlineView:outlineView numberOfChildrenOfItem:item] + factoryOffset;
    }
    
    // FACTORY PRESET?
    if ([item isKindOfClass:[NSDictionary class]]) {
		NSArray *array = [(NSDictionary*)item allValues];
		if ([array count] <= 0) return 0;
        id obj = [array objectAtIndex:0];
        
        if ([obj isKindOfClass:[NSArray class]])
            return [(NSArray *)obj count];
        else 
            return 0;
    }
    
    // ELSE SUPERCLASS HANDLING
    return [super outlineView:outlineView numberOfChildrenOfItem:item];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item {
    // TOP LEVEL?
    if (item == nil) return NO;
    
    // FACTORY PRESET?
    if ([item isKindOfClass:[NSDictionary class]]) {
		NSArray *array = [(NSDictionary*)item allValues];
		if ([array count] <= 0) return NO;
        id obj = [array objectAtIndex:0];
        
        return [obj isKindOfClass:[NSArray class]];
    }
    
    // ELSE SUPERCLASS HANDLING
    return [super outlineView:outlineView isItemExpandable:item];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
	NSString *title = nil;
	if ([item isKindOfClass:[NSArray class]]) {
		title = NSLocalizedStringFromTable(@"Factory", @"AUInspector", @"Factory Preset group heading");
	} else if ([item isKindOfClass:[NSDictionary class]]) {
        title = (NSString *)[[(NSDictionary *)item allKeys] objectAtIndex:0];
	}
	
	if (title != nil) {
        NSCell *cell = [self cell];
		float fontSize = [[cell font] pointSize];
		
        if ([self outlineView:outlineView isItemExpandable:item]) {
            [cell setFont:[NSFont boldSystemFontOfSize:fontSize]];
        } else {
            [cell setFont:[NSFont systemFontOfSize:fontSize]];
        }
        
        [cell setTitle:title];
        
        return cell;
    }
    
	// ELSE SUPERCLASS HANDLING
    return [super outlineView:outlineView objectValueForTableColumn:tableColumn byItem:item];
}


@end
