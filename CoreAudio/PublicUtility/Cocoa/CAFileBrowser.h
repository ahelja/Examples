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
/*=============================================================================
 *  CAFileBrowser.h
 *  PlayPen
 *-----------------------------------------------------------------------------
 *
 *=============================================================================*/

#import <Cocoa/Cocoa.h>

extern NSString *CARootUserDirectory;
extern NSString *CARootLocalDirectory;
extern NSString *CARootNetworkDirectory;

class CAFileHandling;

@interface CAFileBrowser : NSOutlineView {
@protected
    NSTextFieldCell *		mCell;
	NSString *				mCachedRenameValue;
	
    CAFileHandling *		mFileHandler;
    
    NSMutableArray *		mBrowserTreeList;
    NSMutableArray *		mOutlineViewDataRetentionPool;
	
	BOOL					mBrowserWasDoubleClicked;
	BOOL					mKeysWerePressed;
	BOOL					mFileIsBeingRenamed;
	
	BOOL					mScanNetworkForFiles;
	
	// preset name vs. file name member holders
	CFTreeRef				mPresetToRename;
	NSString *				mPresetFileName;
	
	// cached-across-reload state
	NSArray *				mCachedOpenItemsState;
	int						mCachedSelectedRow;
}

// public functions
- (id)initWithFrame:(NSRect)inRect shouldScanNetworkForFiles:(BOOL)shouldScanNetworkForFiles;
	// scanning the network can prove time-consuming for some users, so we leave it
	// up to the client whether or not to scan it.

- (BOOL)scansNetworkForFiles;
	// self explanatory

- (NSURL *)URLForItem:(id)item;
	// returns autoreleased URL

- (CFPropertyListRef)readPropertyListForItem:(id)item;
	// client is responsible for releasing returned CFPropertyListRef
	// returns NULL if property list could not be retrieved
	
- (BOOL)presetName:(NSString *)inPresetName existsAsChildOf:(id)item;
	// use before calling savePresetWithName:asChildOf: to detect a potential overwrite
	
- (BOOL)savePresetWithName:(NSString *)inPresetName asChildOf:(id)item;
    // subclasses should implement this method to save appropriate
	// data at the location specified by the client.
	// returns: YES if operation was successful
	
- (NSArray *)expandedItems;
	// returns a storable array of the expanded items in the CAFileBrowser.
	// This can be saved to disk or cached to restore the items-expanded state
	// of the CAFileBrowser across calls to rescanFiles -- which loses the
	// expanded-items state of the CAFileBrowser -- or across app launches.
- (void)setExpandedItems:(NSArray *)inOpenedItems;
	// restores the CAFileBrowser's items-expanded state from an array retrieved
	// from an earlier call to expandedItems.
	
- (BOOL)createDirectory:(NSString *)inName asChildOfItem:(id)parent;
	// creates an empty directory as a child of the specified item
	// (specified item must be a directory.)
	// returns: YES if operation was successful

- (void)rescanFiles;
	// implemented by subclasses to rescan file system to pick up any
	// changes made on disk and integrate those into the hierarchy.
	// Subclasses should delete their CAFileHandlingObject here,
	// create a new one, and reset it using the protected
	// setCAFileHandlingObject method.
	
@end
