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
	CAMIDIEndpointMenu.mm
	
=============================================================================*/

#import "CAMIDIEndpointMenu.h"
#include "CAMIDIEndpoints.h"

static CAMIDIEndpoints *gMIDIEndpoints = NULL;
static NSMutableSet *   gInstances = NULL;
static MIDIClientRef	gClient = NULL;

#define mEndpointInfo ((CAMIDIEndpoints::EndpointInfoList *)_endpointInfo)

// CoreMIDI callback for when endpoints change -- rebuilds all menu instances
static void NotifyProc(const MIDINotification *message, void *refCon)
{
	if (message->messageID == kMIDIMsgSetupChanged) {
		gMIDIEndpoints->UpdateFromCurrentState();
		
		NSEnumerator *e = [gInstances objectEnumerator];
		CAMIDIEndpointMenu *menu;
		while ((menu = [e nextObject]) != nil)
			[menu rebuildMenu];
	}
}

@implementation CAMIDIEndpointMenu

- (void)_init
{
	mEndpointInfo = NULL;
	if (gInstances == NULL)
		gInstances = [[NSMutableSet alloc] init];
	[gInstances addObject: self];
	mInited = YES;
	mType = -1;
	mOptions = 0;
	mSelection = 0;
	mSelectionName = nil;
}

- (id)initWithFrame: (NSRect)frame
{
    self = [super initWithFrame: frame];
    if (self)
		[self _init];
    return self;
}

- (void)dealloc
{
	delete mEndpointInfo;
	[gInstances removeObject: self];
	[mSelectionName release];
	[super dealloc];
}

- (void)buildMenu: (int)type opts: (int)opts
{
	if (!mInited)
		[self _init];
	
	if (gClient == NULL)
		MIDIClientCreate(CFSTR(""), NotifyProc, NULL, &gClient);

	mType = type;
	mOptions = opts;
	[self rebuildMenu];
}

- (void)syncSelectedName
{
	[mSelectionName release];
	[(mSelectionName = [self titleOfSelectedItem]) retain];
}

static NSString *UniqueTitle(NSString *name, NSMutableDictionary *previousTitles)
{
	NSString *newItemTitle = name;
	int suffix = 0;
	while (true) {
		if ([previousTitles objectForKey: newItemTitle] == nil)
			break;
		if (suffix == 0) suffix = 2; else ++suffix;
		newItemTitle = [NSString stringWithFormat: @"%@ #%d", name, suffix];
	}
	[previousTitles setObject: newItemTitle forKey: newItemTitle];
	return newItemTitle;
}

- (void)rebuildMenu
{
	int itemsToKeep = (mOptions & kMIDIEndpointMenuOpt_CanSelectNone) ? 1 : 0;
	
	while ([self numberOfItems] > itemsToKeep)
		[self removeItemAtIndex: itemsToKeep];
	delete mEndpointInfo;
	mEndpointInfo = NULL;
	
	if (gMIDIEndpoints == NULL)
		gMIDIEndpoints = new CAMIDIEndpoints;
	
	switch (mType) {
	case kMIDIEndpointMenuSources:
		mEndpointInfo = gMIDIEndpoints->GetSources(mOptions);
		break;
	case kMIDIEndpointMenuDestinations:
	default:	// better than failing I suppose
		mEndpointInfo = gMIDIEndpoints->GetDestinations(mOptions);
		break;
	case kMIDIEndpointMenuPairs:
		mEndpointInfo = gMIDIEndpoints->GetEndpointPairs(mOptions);
		break;
	}
	
	NSMutableDictionary *previousTitles = [[NSMutableDictionary alloc] init];
	int n = mEndpointInfo->size();
	bool foundSelection = false;
	for (int i = 0; i < n; ++i) {
		CAMIDIEndpoints::EndpointInfo *ei = (*mEndpointInfo)[i];
		NSString *name = (NSString *)ei->mDisplayName;
		NSString *newItemTitle = UniqueTitle(name, previousTitles);
		// see if that collides with any previous item -- base class requires unique titles
		
		[self addItemWithTitle: newItemTitle]; // cast from CFString
		if (ei->mUniqueID == mSelection) {
			[self selectItemAtIndex: itemsToKeep + i];
			[self syncSelectedName];
			foundSelection = true;
		}
	}
	if (!foundSelection) {
		/*if (mSelectionName != nil) {
#warning "all the code following indexOfSelectedItem calls will crash in the presence of this!"
			// add the previously selected item, disabled, at the bottom
			NSString *newItemTitle = UniqueTitle(mSelectionName, previousTitles);
			[self addItemWithTitle: newItemTitle];
			id <NSMenuItem> item = [self itemAtIndex: itemsToKeep + n];
			[item setEnabled: NO];
			[self selectItemAtIndex: itemsToKeep + n];
		} else*/ {
			[self selectItemAtIndex: 0];
		}
	}
	[previousTitles release];
}

- (MIDIEndpointRef)selectedEndpoint
{
	int itemsToIgnore = (mOptions & kMIDIEndpointMenuOpt_CanSelectNone) ? 1 : 0;
	int i = [self indexOfSelectedItem];
	if (i >= itemsToIgnore) {
		CAMIDIEndpoints::EndpointInfo *ei = (*mEndpointInfo)[i - itemsToIgnore];
		mSelection = ei->mUniqueID;
		[self syncSelectedName];
		if (mType == kMIDIEndpointMenuSources)
			return ei->mSourceEndpoint;
		else
			return ei->mDestinationEndpoint;
	}
	return NULL;
}

- (void)selectedEndpointPair: (MIDIEndpointRef *)outSource dest: (MIDIEndpointRef *)outDest
{
	int itemsToIgnore = (mOptions & kMIDIEndpointMenuOpt_CanSelectNone) ? 1 : 0;
	int i = [self indexOfSelectedItem];
	if (i >= itemsToIgnore) {
		CAMIDIEndpoints::EndpointInfo *ei = (*mEndpointInfo)[i - itemsToIgnore];
		mSelection = ei->mUniqueID;
		[self syncSelectedName];
		*outSource = ei->mSourceEndpoint;
		*outDest = ei->mDestinationEndpoint;
		return;
	}
	*outSource = NULL;
	*outDest = NULL;
}

- (MIDIUniqueID)selectedUniqueID
{
	[self syncSelectedName];
	int itemsToIgnore = (mOptions & kMIDIEndpointMenuOpt_CanSelectNone) ? 1 : 0;
	int i = [self indexOfSelectedItem];
	MIDIUniqueID uid = (i >= itemsToIgnore) ? (*mEndpointInfo)[i - itemsToIgnore]->mUniqueID : kMIDIInvalidUniqueID;
	mSelection = uid;
	return uid;
}

- (BOOL)selectUniqueID: (MIDIUniqueID)uniqueID
{
	mSelection = uniqueID;
	int itemsToIgnore = (mOptions & kMIDIEndpointMenuOpt_CanSelectNone) ? 1 : 0;
	if (uniqueID == kMIDIInvalidUniqueID && itemsToIgnore == 1) {
		[self selectItemAtIndex: 0];
		[self syncSelectedName];
		return YES;
	}
	int n = mEndpointInfo->size();
	for (int i = 0; i < n; ++i) {
		CAMIDIEndpoints::EndpointInfo *ei = (*mEndpointInfo)[i];
		if (ei->mUniqueID == uniqueID) {
			[self selectItemAtIndex: itemsToIgnore + i];
			[self syncSelectedName];
			return YES;
		}
	}
	return NO;
}

+ (CAMIDIEndpoints *)getEndpoints
{
	if (gMIDIEndpoints == NULL)
		gMIDIEndpoints = new CAMIDIEndpoints;
	return gMIDIEndpoints;
}

@end
