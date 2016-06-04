/*
	AppController.m
	Copyright (c) 2001-2004, Apple Computer, Inc., all rights reserved.
	Author: Chuck Pisula

	Milestones:
	Initially created 3/1/01

	Application Controller Object, and fsBrowser delegate.
*/

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "AppController.h"
#import "FSNodeInfo.h"
#import "FSBrowserCell.h"

#define MAX_VISIBLE_COLUMNS 4

@interface AppController (PrivateUtilities)
- (NSString*)fsPathToColumn:(int)column;
- (NSDictionary*)normalFontAttributes;
- (NSDictionary*)boldFontAttributes;
- (NSAttributedString*)attributedInspectorStringForFSNode:(FSNodeInfo*)fsnode;
@end

@implementation AppController

- (void)awakeFromNib {
    // Make the browser user our custom browser cell.
    [fsBrowser setCellClass: [FSBrowserCell class]];

    // Tell the browser to send us messages when it is clicked.
    [fsBrowser setTarget: self];
    [fsBrowser setAction: @selector(browserSingleClick:)];
    [fsBrowser setDoubleAction: @selector(browserDoubleClick:)];
    
    // Configure the number of visible columns (default max visible columns is 1).
    [fsBrowser setMaxVisibleColumns: MAX_VISIBLE_COLUMNS];
    [fsBrowser setMinColumnWidth: NSWidth([fsBrowser bounds])/(float)MAX_VISIBLE_COLUMNS];

    // Prime the browser with an initial load of data.
    [self reloadData: nil];
}

- (IBAction)reloadData:(id)sender {
    [fsBrowser loadColumnZero];
}

// ==========================================================
// Browser Delegate Methods.
// ==========================================================

// Use lazy initialization, since we don't want to touch the file system too much.
- (int)browser:(NSBrowser *)sender numberOfRowsInColumn:(int)column {
    NSString   *fsNodePath = nil;
    FSNodeInfo *fsNodeInfo = nil;
    
    // Get the absolute path represented by the browser selection, and create a fsnode for the path.
    // Since column represents the column being (lazily) loaded fsNodePath is the path for the last selected cell.
    fsNodePath = [self fsPathToColumn: column];
    fsNodeInfo = [FSNodeInfo nodeWithParent: nil atRelativePath: fsNodePath];
    
    return [[fsNodeInfo visibleSubNodes] count];
}

- (void)browser:(NSBrowser *)sender willDisplayCell:(id)cell atRow:(int)row column:(int)column {
    NSString   *containingDirPath = nil;
    FSNodeInfo *containingDirNode = nil;
    FSNodeInfo *displayedCellNode = nil;
    NSArray    *directoryContents = nil;
    
    // Get the absolute path represented by the browser selection, and create a fsnode for the path.
    // Since (row,column) represents the cell being displayed, containingDirPath is the path to it's containing directory.
    containingDirPath = [self fsPathToColumn: column];
    containingDirNode = [FSNodeInfo nodeWithParent: nil atRelativePath: containingDirPath];
    
    // Ask the parent for a list of visible nodes so we can get at a FSNodeInfo for the cell being displayed.
    // Then give the FSNodeInfo to the cell so it can determine how to display itself.
    directoryContents = [containingDirNode visibleSubNodes];
    displayedCellNode = [directoryContents objectAtIndex: row];
    
    [cell setAttributedStringValueFromFSNodeInfo: displayedCellNode];
}

// ==========================================================
// Browser Target / Action Methods.
// ==========================================================

- (IBAction)browserSingleClick:(id)browser {
    // Determine the selection and display it's icon and inspector information on the right side of the UI.
    NSImage            *inspectorImage = nil;
    NSAttributedString *attributedString = nil;
    
    if ([[browser selectedCells] count]==1) {
        NSString *nodePath = [browser path];
        FSNodeInfo *fsNode = [FSNodeInfo nodeWithParent: nil atRelativePath: nodePath];
        
        attributedString = [self attributedInspectorStringForFSNode: fsNode];
        inspectorImage = [fsNode iconImageOfSize: NSMakeSize(128,128)];
    }
    else if ([[browser selectedCells] count]>1) {
        attributedString = [[NSAttributedString alloc] initWithString: @"Multiple Selection"];
    }
    else {
	attributedString = [[NSAttributedString alloc] initWithString: @"No Selection"];
    }
    
    [nodeInspector setAttributedStringValue: attributedString];
    [nodeIconWell setImage: inspectorImage];
}

- (IBAction)browserDoubleClick:(id)browser {
    // Open the file and display it information by calling the single click routine.
    NSString *nodePath = [browser path];
    [self browserSingleClick: browser];
    [[NSWorkspace sharedWorkspace] openFile: nodePath];
}

@end

@implementation AppController (PrivateUtilities)

- (NSString*)fsPathToColumn:(int)column {
    NSString *path = nil;
    if(column==0) path = [NSString stringWithFormat: @"/"];
    else path = [fsBrowser pathToColumn: column];
    return path;
}

- (NSDictionary*)normalFontAttributes {
    return [NSDictionary dictionaryWithObject: [NSFont systemFontOfSize:[NSFont systemFontSize]] forKey:NSFontAttributeName];
}

- (NSDictionary*)boldFontAttributes {
    return [NSDictionary dictionaryWithObject: [NSFont boldSystemFontOfSize:[NSFont systemFontSize]] forKey:NSFontAttributeName];
}

- (NSAttributedString*)attributedInspectorStringForFSNode:(FSNodeInfo*)fsnode {
    NSMutableAttributedString *attrString = [[[NSMutableAttributedString alloc] initWithString:@"Name: " attributes:[self boldFontAttributes]] autorelease];
    [attrString appendAttributedString: [[[NSAttributedString alloc] initWithString:[NSString stringWithFormat: @"%@\n", [fsnode lastPathComponent]] attributes:[self normalFontAttributes]] autorelease]];
    [attrString appendAttributedString: [[[NSAttributedString alloc] initWithString:@"Type: " attributes:[self boldFontAttributes]] autorelease]];
    [attrString appendAttributedString: [[[NSAttributedString alloc] initWithString:[NSString stringWithFormat: @"%@\n", [fsnode fsType]] attributes:[self normalFontAttributes]] autorelease]];
    return attrString;
}

@end

