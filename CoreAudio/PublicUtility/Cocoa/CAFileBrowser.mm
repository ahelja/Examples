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
 *  CAFileBrowser.mm
 *  PlayPen
 *-----------------------------------------------------------------------------
 *
 *
 * Revision 1.12  2005/05/17 18:24:48  luke
 * change file-system timeouts from 5sec to 90sec (rdar://problem/4110219)
 *
 * Revision 1.11  2005/05/02 17:47:44  luke
 * don't invoke renaming bahvior if user is pressing keys
 *
 * Revision 1.10  2004/12/19 20:27:18  luke
 * add 'presetName:ExistsAsChildOf:' method
 *
 * Revision 1.9  2004/12/19 18:11:45  luke
 * only expand 1st level user/local/network root items
 *
 * Revision 1.8  2004/11/16 17:36:17  luke
 * last part of fix for [3747338]
 *
 * Revision 1.7  2004/11/13 17:51:12  luke
 * fix warning
 *
 * Revision 1.6  2004/11/13 17:46:17  luke
 * removed NSAsserts & better handling errors
 *
 * Revision 1.5  2004/10/29 01:17:18  luke
 * fix warnings in Panther
 *
 * Revision 1.4  2004/09/20 17:10:23  luke
 * conditionalize some NSTableView API usage based on OSX version (due to AppKit API deprecation)
 *
 * Revision 1.3  2004/08/03 18:34:16  luke
 * tweaks
 *
 * Revision 1.2  2004/08/03 18:31:17  luke
 * tighten up setName code
 *
 * Revision 1.1  2004/07/29 00:21:15  luke
 * factored out from PlayPen
 *
 * Revision 1.17  2004/07/29 00:08:51  luke
 * tweaks
 *
 * Revision 1.16  2004/07/28 23:57:20  luke
 * pass 3: final
 *
 * Revision 1.15  2004/07/28 21:43:26  luke
 * 2nd pass: CAFileBrowser API breakup
 *
 * Revision 1.14  2004/07/28 20:50:15  luke
 * first pass: transition into approved API
 *
 * Revision 1.13  2004/07/22 19:44:35  luke
 * Panther fix...
 *
 * Revision 1.12  2004/07/21 23:54:42  luke
 * fix warnings on Tiger
 *
 * Revision 1.11  2004/07/07 00:24:16  luke
 * [3482090] make AUInspector localizable
 *
 * Revision 1.10  2004/07/02 01:02:31  mhopkins
 * Fixed hang renaming files
 *
 * Revision 1.9  2004/06/24 23:21:58  mhopkins
 * added methods to return name and data key strings
 *
 * Revision 1.8  2004/05/27 22:55:50  luke
 * fix last network-access dangler.  allow preset renaming to be blocked by subclasses
 *
 * Revision 1.7  2004/05/26 21:54:27  luke
 * remove NSLog()
 *
 * Revision 1.6  2004/05/26 21:09:54  luke
 * add hasPresets method
 *
 * Revision 1.5  2004/05/26 19:41:23  luke
 * fix font clipping
 *
 * Revision 1.4  2004/05/26 18:20:38  luke
 * [3665153] fix MIDIThru preset dictionary embedding
 *
 * Revision 1.3  2004/05/26 17:13:01  luke
 * [3656248] only search Network for presets when preferences say to do so
 *
 * Revision 1.2  2004/05/25 22:25:19  luke
 * allow preset renaming (per [3616417])
 *
 * Revision 1.1  2004/04/17 20:05:20  luke
 * source reorg.
 *
 * Revision 1.15  2003/12/18 19:48:29  luke
 * push directory-creation code into CAFileBrowser superclass
 *
 * Revision 1.14  2003/11/06 19:28:23  luke
 * pull 'savePresetWithName:asLocal: into superclass
 *
 * Revision 1.13  2003/10/28 19:02:37  luke
 * tweaks
 *
 * Revision 1.12  2003/10/27 23:57:29  luke
 * treat enter/return press as a double-click
 *
 * Revision 1.11  2003/10/22 22:17:32  luke
 * add path accessor
 *
 * Revision 1.10  2003/10/22 00:08:24  mhopkins
 * Added initWithCoder method so browser can be instantiated via a nib file
 *
 * Revision 1.9  2003/10/21 23:28:26  luke
 * added features that future subclasses may need
 *
 * Revision 1.8  2003/10/21 22:03:31  luke
 * inherit from NSOutlineView & change some behavior per discussion with Bill
 *
 * Revision 1.7  2003/10/21 20:27:30  luke
 * fix edge null-deref error
 *
 * Revision 1.6  2003/10/20 18:29:14  luke
 * improve preset mgr. behavior (still not correct on scrolling though)
 *
 * Revision 1.5  2003/10/16 22:38:39  luke
 * improved behavior
 *
 * Revision 1.4  2003/10/15 16:56:43  luke
 * tighten up code.
 *
 * Revision 1.3  2003/10/15 00:46:03  luke
 * tighten up code
 *
 * Revision 1.2  2003/10/15 00:26:28  luke
 * label preset groups
 *
 * Revision 1.1  2003/10/15 00:12:30  luke
 * new file
 *
 *
 *-----------------------------------------------------------------------------
 *  Created by Luke Bellandi on Tue Oct 14 2003.
 *  Copyright (c) 2003 Apple Computer Inc. All rights reserved.
 *=============================================================================*/

#import "CAFileHandling.h"

#import "CAFileBrowser.h"
#import "CAFileBrowser_Protected.h"

NSString *CARootUserDirectory		= @"CARootUserDirectory";
NSString *CARootLocalDirectory		= @"CARootLocalDirectory";
NSString *CARootNetworkDirectory	= @"CARootNetworkDirectory";

static NSDictionary *sContextMenuFontAttrs = nil;

@interface CAFileBrowser(Internal)
- (float)fontHeight;
- (void)dispatchedObjectSetup;
- (CFTreeRef)privGetTreeFromItem:(id)item;
- (void)handleBrowserClick:(id)inSender;
- (void)handleBrowserClickOffloaded:(NSValue *)inWrappedMouseLocation;
- (void)handleBrowserDoubleClick:(id)inSender;
- (void)itemWasActivatedDispatcher:(id)sender;
@end

#pragma mark -

@implementation CAFileBrowser
#pragma mark ____ INIT / DEALLOC ____
- (id)init {
    [self release];
    return nil;
}

- (id)initWithFrame:(NSRect)frame {
	return [self initWithFrame:frame shouldScanNetworkForFiles:NO];
}

- (id)initWithFrame:(NSRect)frame shouldScanNetworkForFiles:(BOOL)shouldScanNetworkForFiles {
    self = [super initWithFrame:frame];
    
	mScanNetworkForFiles = shouldScanNetworkForFiles;
    [self dispatchedObjectSetup];
    
    return self;
}

- (id)initWithCoder:(NSCoder *)decoder {
	self = [super initWithCoder:decoder];
    
	// N.B.: Nothing currently encodes the 'kScanNetworkForFilesKey'
	mScanNetworkForFiles = [decoder decodeBoolForKey:@"kScanNetworkForFilesKey"];
	
    [self dispatchedObjectSetup];

	return self;
}

- (void)dealloc {
    [mOutlineViewDataRetentionPool release];
    [mBrowserTreeList release];
    
    [mCell release];
	[mCachedRenameValue release];
	
	[super dealloc];
}

#pragma mark ____ PUBLIC FUNCTIONS ____
- (BOOL)scansNetworkForFiles {
	return mScanNetworkForFiles;
}

- (NSURL *)URLForItem:(id)item {
    if ([item isKindOfClass:[NSValue class]]) {
        CFTreeRef tree = (CFTreeRef)[(NSValue *)item pointerValue];
		if (tree == NULL)
			return nil;
        if (mFileHandler->IsItem(tree)) {
            tree = CFTreeGetParent(tree);
        }
        CFURLRef url = NULL;
        mFileHandler->GetPathCopy(tree, url);
        return [(NSURL *)url autorelease];
    }
    
    return nil;
}

- (CFPropertyListRef)readPropertyListForItem:(id)item {
    if ([item isKindOfClass:[NSValue class]]) {
        CFTreeRef tree = (CFTreeRef)[(NSValue *)item pointerValue];
        CFPropertyListRef retVal = NULL;
         if (mFileHandler->ReadFromTreeLeaf(tree, retVal) == noErr) {
			return retVal;
		 }
	}
	
	return NULL;
}

- (BOOL)presetName:(NSString *)inPresetName existsAsChildOf:(id)item {
	if (![item isKindOfClass:[NSValue class]]) return NO;
	
	NSURL *baseURL = [self URLForItem:item];
	NSString *potentialPath = [NSString stringWithFormat:@"%@/%@.%@", [baseURL path], inPresetName, [self fileExtension]];
	
	return [[NSFileManager defaultManager] fileExistsAtPath:potentialPath];
}

- (BOOL)savePresetWithName:(NSString *)inPresetName asChildOf:(id)item {
    // subclasses should implement this method to save appropriate
	// data at the location specified by the client.
	return NO;
}

- (BOOL)createDirectory:(NSString *)inName asChildOfItem:(id)parent {
	if (!inName) return NO;
	
	CFTreeRef parentTree = [self privGetTreeFromItem:parent];
	
	CFURLRef url = NULL;
	mFileHandler->GetPathCopy(parentTree, url);
	NSURL *baseURL = [(NSURL *)url autorelease];
	
	NSString *path = [[baseURL path] stringByAppendingPathComponent:inName];
	
	BOOL retVal = [[NSFileManager defaultManager] createDirectoryAtPath:path attributes:nil];
	
	[self rescanFiles];
	
	return retVal;
}

- (NSArray *)expandedItems {
	NSMutableArray *array = [NSMutableArray array];
	
	int count = [self numberOfRows];
	id currentItem;
	NSString *name;
	NSNumber *isExpanded;
	NSArray *itemArray;
	
	for (int i = 0; i < count; ++i)
	{
		currentItem = [self itemAtRow:i];
		if (currentItem == NULL) continue;
		
		isExpanded = [NSNumber numberWithBool:[self isItemExpanded:currentItem]];
		
		name = [[self outlineView:self objectValueForTableColumn:nil byItem:currentItem] stringValue];
		if (name == NULL) continue;
		
		itemArray = [NSArray arrayWithObjects:(NSString *)name, isExpanded, nil];
		[array addObject:itemArray];
	}
	
	return array;
}

- (void)setExpandedItems:(NSArray *)inOpenedItems
{
	int currentRowIndex = 0;
	int openedItemsCount = [inOpenedItems count];
	
	int j;
	int count;
	id currentItem;
	NSArray *currentRecord;
	NSString *currentName;
	BOOL shouldOpenCurrentItem;
	NSString *displayName;
	
	for (int i = 0; i < openedItemsCount; ++i)
	{
		count = [self numberOfRows];
		if (i >= count) return;
		
		currentRecord = (NSArray *)[inOpenedItems objectAtIndex:i];
		if ([currentRecord count] != 2) continue;
		
		currentName = (NSString *)[currentRecord objectAtIndex:0];
		shouldOpenCurrentItem = [(NSNumber *)[currentRecord objectAtIndex:1] boolValue];
		
		for (j = currentRowIndex; j < count; ++j)
		{
			currentItem = [self itemAtRow:j];
			
			displayName = [[self outlineView:self objectValueForTableColumn:nil byItem:currentItem] stringValue];
			if (displayName == nil) continue;
			
			// when we match, expand if necessary and break out of inner loop
			if ([currentName isEqualToString:displayName]) {
				if (shouldOpenCurrentItem) {
					[self expandItem:currentItem];
				}
				currentRowIndex = ++j;
				break;
			}
			
		}
	}
	
}

- (void)rescanFiles {
    // subclasses should implement this to re-create the CAFileHandlingObject & reset it
    // (to be called when directory structure changes.)
}

#pragma mark ____ NSOutlineView.DataSource implementation ____
- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item {
    // TOP LEVEL
    if (item == nil) {
        return [mBrowserTreeList objectAtIndex:index];
    }
    
    // SUB-LEVELS
    void *ptr = [(NSValue *)item pointerValue];
    
    // CFTreeRef
    if (CFGetTypeID(ptr) == CFTreeGetTypeID()) {
        NSValue *newValue = [NSValue valueWithPointer:CFTreeGetChildAtIndex ((CFTreeRef)ptr, index)];
        [mOutlineViewDataRetentionPool addObject:newValue];
        return newValue;
    }
    
    return nil;
}

- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item {
    // TOP LEVEL
    if (item == nil)
        return [mBrowserTreeList count];
    
    // SUB-LEVELS
    if ([item isKindOfClass:[NSValue class]]) {
        // CFTreeRef
        CFTreeRef tree = (CFTreeRef)[(NSValue *)item pointerValue];
        return CFTreeGetChildCount(tree);
    }
    
    return 0;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item {
    // TOP LEVEL
    if (item == nil) return NO;
    
    // SUB-LEVELS
    if ([item isKindOfClass:[NSValue class]]) {
        if ( mFileHandler->IsDirectory((CFTreeRef)[(NSValue *)item pointerValue]) )
            return YES;
    }
    
    return NO;
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item {
    if (item == nil) return nil;
	
    CFTreeRef tree = (CFTreeRef)[(NSValue *)item pointerValue];
    NSString *pName;
    
    // special case naming for tree roots
    if (tree == mFileHandler->GetUserTree()) {
        pName = NSLocalizedStringFromTable(@"User", @"AUInspector", @"Preset file browser group heading for 'User'");
    } else if (tree == mFileHandler->GetLocalTree()) {
        pName = NSLocalizedStringFromTable(@"Local", @"AUInspector", @"Preset file browser group heading for 'Local'");
    } else if (tree == mFileHandler->GetNetworkTree()) {
        pName = NSLocalizedStringFromTable(@"Network", @"AUInspector", @"Preset file browser group heading for 'Network'");
    } else {
        CFStringRef string = nil;
        mFileHandler->GetNameCopy(tree, string);
        pName = [(NSString *)string autorelease];
    }
    
    if ([self outlineView:outlineView isItemExpandable:item]) {
        [mCell setFont:[NSFont boldSystemFontOfSize:[self fontHeight]]];
    } else {
        [mCell setFont:[NSFont systemFontOfSize:[self fontHeight]]];
    }
	
    if (pName == nil) {
		if (!mFileIsBeingRenamed) {
			// if we get here, the file was probably erased... we should rescan our model.
			mFileIsBeingRenamed = YES;
			[self rescanFiles];
			[outlineView performSelectorOnMainThread: @selector(reloadData) withObject:nil waitUntilDone:NO];	
			mFileIsBeingRenamed = NO;
        }
		
        pName = @"";
    }
    
    [mCell setTitle:pName];
    
    return mCell;
}

- (void)outlineView:(NSOutlineView *)inOutlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)inTableColumn byItem:(id)item
{
	[inTableColumn setDataCell:mCell];
	
	if (mCachedRenameValue == nil) return;
	
	NSString *oldName = [(NSTextFieldCell *)[self outlineView:self objectValueForTableColumn:inTableColumn byItem:item] stringValue];
	
	[self selectRow:[self selectedRow] byExtendingSelection:NO];
	id theObject = [self itemAtRow:[self selectedRow]];
	
	if ([oldName isEqual:mCachedRenameValue]) return;
	if (![theObject isKindOfClass:[NSValue class]]) return;
	
	CFTreeRef tree = (CFTreeRef)[(NSValue *)theObject pointerValue];
	
	CFURLRef myRef = nil;
	mFileHandler->GetPathCopy (tree, myRef);
	NSURL *url = [(NSURL *)myRef autorelease];
	
	NSString *sourcePath = [url path];
	NSString *fileExtension = [self fileExtension];
	NSString *destPath = [NSString stringWithFormat:@"%@/%@.%@", [sourcePath stringByDeletingLastPathComponent], mCachedRenameValue, fileExtension];
	
	// rename preset in file
	NSDictionary *fileData = [NSDictionary dictionaryWithContentsOfFile:sourcePath];
	NSFileManager *fileMgr = [NSFileManager defaultManager];
	
	if (fileData != nil)
	{
		NSMutableDictionary *mutableFileData = [NSMutableDictionary dictionaryWithDictionary:fileData];
		[mutableFileData setObject:mCachedRenameValue forKey: [self nameKeyString]];
		
		[mutableFileData writeToFile:sourcePath atomically:YES];
		
		// rename file
		[fileMgr movePath:sourcePath toPath:destPath handler:nil];
	}
	
	// give file system a second to catch up with changes
	// wait until file manager's done its work
	mFileIsBeingRenamed = YES;
		int timeOutCount = 900;	// 90 seconds
		int count = 0;
		while (count < timeOutCount) {
			if ([fileMgr fileExistsAtPath:destPath]) break;
			
			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);
			++count;
		};
		
		[mCachedRenameValue release];
		mCachedRenameValue = nil;
		
		[self rescanFiles];
	mFileIsBeingRenamed = NO;
}

#pragma mark ____ NSOutlineView delegates ____
- (BOOL)outlineView:(NSOutlineView *)inOutlineView shouldEditTableColumn:(NSTableColumn *)inTableColumn item:(id)inItem
{
	return NO;
}

- (void)textDidEndEditing:(NSNotification *)notification {
    if([[[notification userInfo] valueForKey:@"NSTextMovement"] intValue] == NSReturnTextMovement)
	{
		mCachedRenameValue = [[mCell stringValue] retain];
		NSMutableDictionary *newUserInfo = [[NSMutableDictionary alloc] initWithDictionary:[notification userInfo]];
		[newUserInfo setObject:[NSNumber numberWithInt:NSIllegalTextMovement] forKey:@"NSTextMovement"];
		notification = [NSNotification	notificationWithName:[notification name]
										object:[notification object]
										userInfo:newUserInfo];
		[super textDidEndEditing:notification];
		[newUserInfo release];
		[[self window] makeFirstResponder:self];
	} else {
		[super textDidEndEditing:notification];
    }
}

#pragma mark ____ NSResponder/NSView overrides ____
- (void)keyDown:(NSEvent *)inEvent {
    mKeysWerePressed = YES;
	
    BOOL eventWasHandled = NO;
	
    UInt32		i, length;
    NSString *	keyInput = [inEvent charactersIgnoringModifiers];
    length = [keyInput length];
    for (i = 0; i < length; i++) {
        if ( 	([keyInput characterAtIndex:i] == NSCarriageReturnCharacter) ||
                ([keyInput characterAtIndex:i] == NSEnterCharacter) )	{
            [self handleBrowserDoubleClick:self];
            eventWasHandled = YES;
        }
    }
    
    if (!eventWasHandled)
        [super keyDown:inEvent];
}

- (BOOL)acceptsFirstMouse:(NSEvent *)inEvent {
    return YES;
}

- (CFStringRef)filenameForNode:(CFTreeRef)inNode
{
	if (inNode == NULL)
		return NULL;
		
	// [4] if we are allowed to rename it, get the preset's filename
	CFURLRef presetURL = NULL;
	verify_noerr (mFileHandler->GetPathCopy (inNode, presetURL));
	if (presetURL == NULL) return NULL;
	
	CFURLRef presetNameURL = CFURLCreateCopyDeletingPathExtension(NULL, presetURL);
	CFRelease(presetURL);
	if (presetNameURL == NULL) return NULL;
	
	CFStringRef presetFileName = CFURLCopyLastPathComponent(presetNameURL);
	CFRelease(presetNameURL);
	return presetFileName;
}

- (CFStringRef)presetNameForNode:(CFTreeRef)inNode
{
	if (inNode == NULL)
		return NULL;
	
	CFStringRef presetName = NULL;
	verify_noerr (mFileHandler->GetNameCopy(inNode, presetName));
	
	return presetName;
}

- (NSMenu *)menuForEvent:(NSEvent *)inEvent
{
	// [1] find out which item we're over
	 NSPoint	mouseLocationInView = [self convertPoint:[inEvent locationInWindow] fromView:nil];
	 int row = [self rowAtPoint:mouseLocationInView];
	 if ((row < 0) || (row >= [self numberOfRows]))
		return nil;
	
	id item = [self itemAtRow:row];
	if (item == nil) return nil;
	
	// [2] find out if we're allowed to rename that item
	if (![self shouldAllowItemRenaming:item]) return nil;
	
	// [3] get the CFTree safely
	if (![item isKindOfClass:[NSValue class]]) return nil;
	
	CFTreeRef tree = (CFTreeRef)[(NSValue *)item pointerValue];
	if (CFGetTypeID(tree) != CFTreeGetTypeID()) return nil;
	mPresetToRename = tree;
	
	// [4] get preset name & preset filename
	CFStringRef presetFileName = [self filenameForNode:tree];
	CFStringRef presetName = [self presetNameForNode:tree];
	
	// [5] if preset name already == filename, and this is a leaf node, don't need to show a menu
	if (	(CFStringCompare (presetFileName, presetName, kCFCompareCaseInsensitive) == kCFCompareEqualTo) &&
			(mFileHandler->IsItem(mPresetToRename))	)
	{
		CFRelease(presetFileName);
		CFRelease(presetName);
		return nil;
	}
	CFRelease(presetName);
	
	CFRetain(mPresetToRename);
	
	mPresetFileName = [(NSString *)presetFileName retain];
	CFRelease(presetFileName);
	
	// [6] create and return menu with preset name option
	NSString *titleString;
	if (mFileHandler->IsItem(mPresetToRename)) {
		titleString = [NSString stringWithFormat:NSLocalizedStringFromTable(@"Fix Preset Name (%@)", @"AUInspector", @"Formatted string for presetView context menu"), (NSString *)mPresetFileName];
	} else {
		titleString = NSLocalizedStringFromTable(@"Fix preset names in this folder", @"AUInspector", @"string for presetView context menu");
	}
	//NSAttributedString *attributedTitle = [[[NSAttributedString alloc] initWithString:titleString attributes:sContextMenuFontAttrs] autorelease];
	
	NSMenuItem *menuItem = [[[NSMenuItem alloc] initWithTitle:titleString action:@selector(renamePresetAsFileName:) keyEquivalent:@""] autorelease];
	//[menuItem setAttributedTitle:attributedTitle];	// this doesn't want to work for some reason... gives a blank menu...
	NSMenu *menu = [[[NSMenu alloc] initWithTitle:@""] autorelease];
	[menu addItem:menuItem];
	return menu;
}


- (void)renamePreset:(CFTreeRef)inPreset withFileName:(NSString *)inFileName
{
	// find preset file
	CFURLRef presetURL = NULL;
	verify_noerr (mFileHandler->GetPathCopy(inPreset, presetURL));
	if (presetURL == NULL)
		return;
	
	// open preset file
	NSMutableDictionary *presetDict = [NSMutableDictionary dictionaryWithContentsOfURL:(NSURL *)presetURL];
	if (presetDict == NULL)
		return;
	
	// change preset name
	[presetDict setObject:inFileName forKey:@kAUPresetNameKey];
	
	// write data back to file
	[presetDict writeToURL:(NSURL *)presetURL atomically:YES];
	CFRelease(presetURL);
}

- (void)renamePresetAsFileName:(id)sender
{
	if (!mPresetToRename || !mPresetFileName)
		return;
	
	if (mFileHandler->IsItem(mPresetToRename)) {
		// rename a single item
		[self renamePreset:mPresetToRename withFileName:mPresetFileName];
	} else {
		// rename all children of the directory item
		CFIndex count = CFTreeGetChildCount(mPresetToRename);
		CFTreeRef *children = static_cast<CFTreeRef *>(malloc(count * sizeof(CFTreeRef)));
		CFTreeGetChildren(mPresetToRename, children);
		
		CFStringRef presetFileName;
		CFStringRef presetName;
		
		for (int i = 0; i < count; ++i) {
			if (children[i] == NULL) continue;
			
			presetFileName = [self filenameForNode:children[i]];
			presetName = [self presetNameForNode:children[i]];
			
			// only rename child if it's an item, and its filename != its preset name
			if (	(CFStringCompare (presetFileName, presetName, kCFCompareCaseInsensitive) != kCFCompareEqualTo) &&
					(mFileHandler->IsItem(children[i]))	)
			{
				[self renamePreset:children[i] withFileName:(NSString *)presetFileName];
			}
			
			if (presetFileName != NULL) CFRelease(presetFileName);
			if (presetName != NULL) CFRelease(presetName);
		}
		
		free(children);
	}
	
	// cleanup member-handoff objects
	CFRelease (mPresetToRename); mPresetToRename = NULL;
	[mPresetFileName release]; mPresetFileName = nil;
	
	// reload data
	[self rescanFiles];
}

- (void)mouseUp:(NSEvent *)inEvent
{
	// cleanup member-handoff objects
	// need to do this in the case that the user has right-clicked and gotten the menu to appear, but
	// has NOT selected the menu option -- need to release & nullify these ivars.
	if (mPresetToRename != NULL)
		CFRelease (mPresetToRename); mPresetToRename = NULL;
	if (mPresetFileName != nil)
		[mPresetFileName release]; mPresetFileName = nil;
}

@end

#pragma mark -

@implementation CAFileBrowser(Protected)
- (BOOL)writePropertyList:(CFPropertyListRef)inPropertyList withName:(NSString *)inName asChildOfItem:(id)parent {
	
	CFTreeRef tree = [self privGetTreeFromItem:parent];
	
	if (tree == NULL)
		return FALSE;
	
	// if this tree is a leaf, use its parent
	if (mFileHandler->IsItem(tree)) {
		tree = CFTreeGetParent(tree);
	}
	
    return (mFileHandler->SaveInDirectory (tree, CFStringRef(inName), inPropertyList) == noErr);
}

- (void)unsetCAFileHandlingObject
{
	// cache opened items state in outline view across the fileHandling object reload
	mCachedOpenItemsState = [self expandedItems];
	mCachedSelectedRow = [self selectedRow];
	mFileHandler = nil;
}

- (void)setCAFileHandlingObject:(CAFileHandling *)inCAFileHandlingObject
{
    mFileHandler = inCAFileHandlingObject;
	
    [mOutlineViewDataRetentionPool removeAllObjects];
    [mBrowserTreeList removeAllObjects];
	
	CFTreeRef networkTree = NULL;
    // Network (only include if user pref says to do so)
	if (mScanNetworkForFiles) {
		networkTree = mFileHandler->GetNetworkTree();
		if (networkTree)
			[mBrowserTreeList addObject:[NSValue valueWithPointer:networkTree]];
	}
    // Local
    CFTreeRef localTree = mFileHandler->GetLocalTree();
    if (localTree)
        [mBrowserTreeList addObject:[NSValue valueWithPointer:localTree]];
    // User
    CFTreeRef userTree = mFileHandler->GetUserTree();
    if (userTree)
        [mBrowserTreeList addObject:[NSValue valueWithPointer:userTree]];
    
    [self reloadData];
	
	if ([mCachedOpenItemsState count] > 0) {
		// if we have cached opened state, restore it
		[self setExpandedItems:mCachedOpenItemsState];
	} else {
		// otherwise open first-level items for User, Local, and Network
		int count = [self outlineView:self numberOfChildrenOfItem:nil];
		int i;
		id currentItem;
		CFTreeRef currentTree;
		
		for (i = 0; i < count; ++i) {
			currentItem = [self outlineView:self child:i ofItem:nil];
			if (![currentItem isKindOfClass:[NSValue class]]) continue;
			
			currentTree = CFTreeRef([(NSValue *)currentItem pointerValue]);
			if (	(currentTree == networkTree) ||
					(currentTree == localTree) ||
					(currentTree == userTree)       ) {
				[self expandItem:currentItem expandChildren:NO];
			}
		}
	}
	
	// if cached, restore selected row
	if (	(mCachedSelectedRow >= 0) &&
			(mCachedSelectedRow < [self numberOfRows]) ) {
		[self selectRow:mCachedSelectedRow byExtendingSelection:NO];
	}
}

- (CAFileHandling *)CAFileHandlingObject {
	return mFileHandler;
}

- (void)setTitle:(NSString *)inTitle {
    NSString *title = inTitle;
    if (inTitle == nil) title = @"";
    
    [(NSTableHeaderCell *)[[self outlineTableColumn] headerCell] setStringValue:title];
}

- (NSString *)title {
	return [(NSTableHeaderCell *)[[self outlineTableColumn] headerCell] stringValue];
}

- (BOOL)itemWasActivated:(id)object {
    // subclasses can override this to respond to outline view item selection
	// returns YES if action was successfully handled, NO on error condition
	return YES;
}

- (NSString *)fileExtension {
	// subclasses should override this to return the file extension (without the period)
	return nil;
}

- (NSString *)nameKeyString {
	// subclasses should override this to return the name key string
	return nil;	
}

- (BOOL)shouldAllowItemRenaming:(id)item {
	// default to allow all presets to be renamed.  Subclasses can override to disallow
	return YES;
}
@end

#pragma mark -

@implementation CAFileBrowser(Internal)
- (float)fontHeight {
	return [NSFont smallSystemFontSize] - 1.5;
}

- (void)dispatchedObjectSetup {
	mOutlineViewDataRetentionPool = [[NSMutableArray alloc] init];
    mBrowserTreeList = [[NSMutableArray alloc] init];
	
	if (sContextMenuFontAttrs == nil) {
		sContextMenuFontAttrs = [[NSDictionary	dictionaryWithObjects:[NSArray arrayWithObjects:[NSFont menuFontOfSize:[NSFont systemFontSize]], [NSColor redColor], nil]
												forKeys:[NSArray arrayWithObjects:NSFontAttributeName, NSForegroundColorAttributeName, nil]] retain];
	}
	
    [self setTarget:self];
	[self setAction:@selector(handleBrowserClick:)];
    [self setDoubleAction:@selector(handleBrowserDoubleClick:)];
    
    // setup cell prototype for outline view
    mCell = [[NSTextFieldCell alloc] init];
    [mCell setBordered:NO];
    [mCell setBezeled:NO];
	[mCell setEditable:YES];
	[mCell setFont:[NSFont systemFontOfSize:[self fontHeight]]];
    [mCell setAlignment:NSLeftTextAlignment];
	[self setCell:mCell];
	
    // setup tableColumn in outline view
    NSTableColumn *tc = [[[NSTableColumn alloc] init] autorelease];
    [tc setEditable:YES];
    [self setRowHeight:[NSFont smallSystemFontSize]];
    [self addTableColumn:tc];
    [self setOutlineTableColumn:tc];
	
    [self setAutoresizesOutlineColumn:YES];
    [self sizeLastColumnToFit];
    [self sizeToFit];
    
    // set data source after view is setup
    [self setDataSource:self];
    [self setDelegate:self];
}

- (CFTreeRef)privGetTreeFromItem:(id)item {
    CFTreeRef	parentTree = NULL;
    
	// Case 1: base directory
	if ([item isKindOfClass:[NSString class]]) {
		NSString *baseDir = (NSString *)item;
		
		// [1a] user
		if ([baseDir isEqualToString:CARootUserDirectory]) {
			parentTree = mFileHandler->GetUserTree();
			if (parentTree == NULL) {
				mFileHandler->CreateUserDirectories();
				int timeOutCount = 360;	// 90 seconds
				int count = 0;
				// allow some time for the directory to be created
				while ((parentTree == NULL) && (count < timeOutCount)) {
					CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, false);
					++count;
					parentTree = mFileHandler->GetUserTree();
				}
			}
		}
		// [1b] local
		if ([baseDir isEqualToString:CARootLocalDirectory]) {
			parentTree = mFileHandler->GetLocalTree();
			if (parentTree == NULL) {
				mFileHandler->CreateLocalDirectories();
				int timeOutCount = 360;	// 90 seconds
				int count = 0;
				// allow some time for the directory to be created
				while ((parentTree == NULL) && (count < timeOutCount)) {
					CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, false);
					++count;
					parentTree = mFileHandler->GetLocalTree();
				}
			}
		}
		// [1c] network
		if ([baseDir isEqualToString:CARootNetworkDirectory]) {
			parentTree = mFileHandler->GetNetworkTree();
			if (parentTree == NULL) {
				mFileHandler->CreateNetworkDirectories();
				int timeOutCount = 360;	// 90 seconds
				int count = 0;
				// allow some time for the directory to be created
				while ((parentTree == NULL) && (count < timeOutCount)) {
					CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, false);
					++count;
					parentTree = mFileHandler->GetNetworkTree();
				}
			}
		}
    }
	
	// Case 2: child directory
	if ([item isKindOfClass:[NSValue class]]) {
		parentTree = (CFTreeRef)[(NSValue *)item pointerValue];
	}
	
	return parentTree;
}

- (void)handleBrowserClick:(id)inSender {
	NSValue *wrappedMouseLocation = [NSValue valueWithPoint:[NSEvent mouseLocation]];
	mKeysWerePressed = NO;
	[self performSelector:@selector(handleBrowserClickOffloaded:) withObject:wrappedMouseLocation afterDelay: 1];
}

- (void)handleBrowserClickOffloaded:(NSValue *)inWrappedMouseLocation {
	// UI: mouse must not have ben moved since first click, and must not have been double-clicked
	if (	(!mBrowserWasDoubleClicked) && !mKeysWerePressed &&
			(NSEqualPoints([inWrappedMouseLocation pointValue], [NSEvent mouseLocation]))   )
	{
		int selectedRow = [self selectedRow];
		// must have valid row selected
		if (selectedRow > 0)
		{
			id itemAtRow = [self itemAtRow:selectedRow];
			// must be a leaf node (preset file, not directory)
			if (![self outlineView:self isItemExpandable:itemAtRow]) {
				// one last browser-specific check
				if ([self shouldAllowItemRenaming:itemAtRow])
					[self editColumn:0 row:[self selectedRow] withEvent:nil select:YES];
			}
		}
	}
	
	mBrowserWasDoubleClicked = NO;
}

- (void)handleBrowserDoubleClick:(id)inSender {
	mBrowserWasDoubleClicked = YES;
	
    [self itemWasActivatedDispatcher:inSender];
}

- (void)itemWasActivatedDispatcher:(id)sender {
	id object = [self itemAtRow:[self selectedRow]];
    if (object == nil) return;
    
    // expand/collapse preset groups
    if ([self outlineView:self isItemExpandable:object]) {
        if ([self isItemExpanded:object])
            [self collapseItem:object];
        else
            [self expandItem:object];
    }
    
    [self itemWasActivated:object];
}

@end
