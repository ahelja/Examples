/*
 
 File: CanvasToolbarController.m
 
 Abstract: Document window toolbar controller
 
 Version: 1.0
 
 © Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to 
 you by Apple Computer, Inc. ("Apple") in 
 consideration of your agreement to the following 
 terms, and your use, installation, modification 
 or redistribution of this Apple software 
 constitutes acceptance of these terms.  If you do 
 not agree with these terms, please do not use, 
 install, modify or redistribute this Apple 
 software.
 
 In consideration of your agreement to abide by 
 the following terms, and subject to these terms, 
 Apple grants you a personal, non-exclusive 
 license, under Apple's copyrights in this 
 original Apple software (the "Apple Software"), 
 to use, reproduce, modify and redistribute the 
 Apple Software, with or without modifications, in 
 source and/or binary forms; provided that if you 
 redistribute the Apple Software in its entirety 
 and without modifications, you must retain this 
 notice and the following text and disclaimers in 
 all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or 
 logos of Apple Computer, Inc. may be used to 
 endorse or promote products derived from the 
 Apple Software without specific prior written 
 permission from Apple.  Except as expressly 
 stated in this notice, no other rights or 
 licenses, express or implied, are granted by 
 Apple herein, including but not limited to any 
 patent rights that may be infringed by your 
 derivative works or by other works in which the 
 Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS 
 IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
 THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
 OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
 SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
 THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
 UNDER THEORY OF CONTRACT, TORT (INCLUDING 
 NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
 IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.
 
 */ 


#import "CanvasToolbarController.h"
#import "CanvasInspectorController.h"

// Toolbar item identifiers
static NSString *CanvasDocToolbarIdentifier               = @"CanvasDocToolbarIdentifier";
static NSString *CanvasAddObjectToolbarItemIdentifier     = @"CanvasAddObjectToolbarItemIdentifier";
static NSString *CanvasRemoveObjectToolbarItemIdentifier  = @"CanvasRemoveObjectToolbarItemIdentifier";
static NSString *CanvasShowInspectorToolbarItemIdentifier = @"CanvasShowInspectorToolbarItemIdentifier";
static NSString *CanvasBringForwardToolbarItemIdentifer   = @"CanvasBringForwardToolbarItemIdentifer";
static NSString *CanvasSendBackwardToolbarItemIdentifier  = @"CanvasSendBackwardToolbarItemIdentifier";

@implementation CanvasToolbarController
- (void)awakeFromNib {
	[self setupToolbar];
}

#pragma mark Toolbar
- (void)setupToolbar {
	// Creates the document window's toolbar.
	NSToolbar *toolbar = [[(NSToolbar *)[NSToolbar alloc] initWithIdentifier: CanvasDocToolbarIdentifier] autorelease];
	[toolbar setAllowsUserCustomization:YES];
	[toolbar setAutosavesConfiguration:YES];
		
	[toolbar setDelegate:self];
	[window setToolbar:toolbar];
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)identifier willBeInsertedIntoToolbar:(BOOL)willBeInserted {
	NSToolbarItem *toolbarItem = [[[NSToolbarItem alloc] initWithItemIdentifier:identifier] autorelease];
	if([identifier isEqualToString:CanvasAddObjectToolbarItemIdentifier]) {
		[toolbarItem setLabel:NSLocalizedStringFromTable(@"Add", @"ToolbarItems", @"Add Object toolbar item label")];
		[toolbarItem setPaletteLabel:NSLocalizedStringFromTable(@"Add", @"ToolbarItems", @"Add Object toolbar item palette label")];
		[toolbarItem setToolTip:NSLocalizedStringFromTable(@"Adds a new canvas object.", @"ToolbarItems", @"Add Object toolbar item tooltip")];
		[toolbarItem setImage:[NSImage imageNamed:@"add"]];
		[toolbarItem setTarget:canvas];
		[toolbarItem setAction:@selector(addNewObject:)];
	} else if([identifier isEqualToString:CanvasRemoveObjectToolbarItemIdentifier]) {
		[toolbarItem setLabel:NSLocalizedStringFromTable(@"Remove", @"ToolbarItems", @"Remove Object toolbar item label")];
		[toolbarItem setPaletteLabel:NSLocalizedStringFromTable(@"Remove", @"ToolbarItems", @"Remove Object toolbar item palette label")];
		[toolbarItem setToolTip:NSLocalizedStringFromTable(@"Removes the selected object(s).", @"ToolbarItems", @"Remove Object toolbar item tooltip")];
		[toolbarItem setImage:[NSImage imageNamed:@"remove"]];
		[toolbarItem setTarget:canvas];
		[toolbarItem setAction:@selector(delete:)];
	} else if([identifier isEqualToString:CanvasShowInspectorToolbarItemIdentifier]) {
		[toolbarItem setLabel:NSLocalizedStringFromTable(@"Show Inspector", @"ToolbarItems", @"Show Inspector toolbar item label")];
		[toolbarItem setPaletteLabel:NSLocalizedStringFromTable(@"Show Inspector", @"ToolbarItems", @"Show Inspector toolbar item palette label")];
		[toolbarItem setToolTip:NSLocalizedStringFromTable(@"Displays the Inspector.", @"ToolbarItems", @"Show Inspector toolbar item tooltip")];
		[toolbarItem setImage:[NSImage imageNamed:@"inspector"]];
		[toolbarItem setTarget:[CanvasInspectorController sharedController]];
		[toolbarItem setAction:@selector(showWindow:)];
	} else if([identifier isEqualToString:CanvasBringForwardToolbarItemIdentifer]) {
		[toolbarItem setLabel:NSLocalizedStringFromTable(@"Bring Forward", @"ToolbarItems", @"Bring Forward toolbar item label")];
		[toolbarItem setPaletteLabel:NSLocalizedStringFromTable(@"Bring Forward", @"ToolbarItems", @"Bring Forward toolbar item palette label")];
		[toolbarItem setToolTip:NSLocalizedStringFromTable(@"Brings the selected object(s) forward.", @"ToolbarItems", @"Bring Forward toolbar item tooltip")];
		[toolbarItem setImage:[NSImage imageNamed:@"move_forward"]];
		[toolbarItem setTarget:canvas];
		[toolbarItem setAction:@selector(moveForward:)];
	} else if([identifier isEqualToString:CanvasSendBackwardToolbarItemIdentifier]) {
		[toolbarItem setLabel:NSLocalizedStringFromTable(@"Send Backward", @"ToolbarItems", @"Send Backward toolbar item label")];
		[toolbarItem setPaletteLabel:NSLocalizedStringFromTable(@"Send Backward", @"ToolbarItems", @"Send Backward toolbar item palette label")];
		[toolbarItem setToolTip:NSLocalizedStringFromTable(@"Sends the selected object(s) backward.", @"ToolbarItems", @"Send Backward toolbar item tooltip")];
		[toolbarItem setImage:[NSImage imageNamed:@"move_backward"]];
		[toolbarItem setTarget:canvas];
		[toolbarItem setAction:@selector(moveBackward:)];
		
	} else {
		toolbarItem = nil;
	}
	
	return toolbarItem;
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar {
	return [NSArray arrayWithObjects:CanvasAddObjectToolbarItemIdentifier, CanvasRemoveObjectToolbarItemIdentifier, CanvasBringForwardToolbarItemIdentifer, CanvasSendBackwardToolbarItemIdentifier, CanvasShowInspectorToolbarItemIdentifier, NSToolbarShowColorsItemIdentifier, NSToolbarShowFontsItemIdentifier, NSToolbarCustomizeToolbarItemIdentifier, NSToolbarFlexibleSpaceItemIdentifier, NSToolbarSpaceItemIdentifier, NSToolbarSeparatorItemIdentifier, nil];
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar {
	return [NSArray arrayWithObjects:CanvasAddObjectToolbarItemIdentifier, CanvasRemoveObjectToolbarItemIdentifier,  NSToolbarSeparatorItemIdentifier, CanvasBringForwardToolbarItemIdentifer, CanvasSendBackwardToolbarItemIdentifier, NSToolbarFlexibleSpaceItemIdentifier, CanvasShowInspectorToolbarItemIdentifier, nil];
}

@end
