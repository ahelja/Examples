/* CGPDFViewer - MyDocument.m
 *
 * Author: Nick Kocharhook
 * Created 11 July 2003
 *
 * Copyright (c) 2003-2004 Apple Computer, Inc.
 * All rights reserved.
 */

/* IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.

 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE. */

#import "MyDocument.h"

// String constants for the toolbar

static NSString *toolbarIdentifier     = @"My Document Toolbar Identifier";
static NSString *rotateLeftIdentifier  = @"Rotate Left Toolbar Item Identifier";
static NSString *rotateRightIdentifier = @"Rotate Right Toolbar Item Identifier";
static NSString *pageUpIdentifier      = @"Page Up Toolbar Item Identifier";
static NSString *pageDownIdentifier    = @"Page Down Toolbar Item Identifier";

@interface MyDocument (private)
- (void)setupToolbar;
@end

@implementation MyDocument

- (id)init
{
    self = [super init];
    if (self == nil)
	return nil;

    return self;
}

- (void)dealloc
{
    [toolbar release];
    [window release];
    [fileName release];
    [super dealloc];
}

- (NSString *)windowNibName
{
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)controller
{
    // Creates a new CGPDFDocumentRef with a reference count of 1.
    CGPDFDocumentRef doc = CGPDFDocumentCreateWithURL((CFURLRef)documentURL);
    
    [super windowControllerDidLoadNib:controller];
    
    window = [[controller window] retain]; // Hang on to the window for convenience
    [self setupToolbar];
    [pdfView setPDFDocument:doc]; // Increments doc's reference count
    [pdfView setNeedsDisplay:YES];
    
    // Now we have to remove our reference, leaving the reference count at 1 again.
    // That way the document really goes away when it is released in PDFView's
    // -dealloc method.
    CGPDFDocumentRelease(doc);
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    // We aren't saving anything, so we can leave this method alone.
    return nil;
}

- (BOOL)readFromFile:(NSString *)name ofType:(NSString *)docType
{
    // We save the URL for now because this method gets called before
    // the PDFView has been created.
    [self setDocumentURL:[[NSURL alloc] initFileURLWithPath:name]];
    
    // Hold onto the file name for use in the window's displayName.
    fileName = [[[name componentsSeparatedByString:@"/"] lastObject] retain];

    return YES;
}

// Changes the display name so we can see the document's page count in the
// window title.
- (NSString *)displayName
{
    int cnt = [pdfView pageCount];
    NSString *name;
    
    if (cnt == 1) {
        NSString *format = NSLocalizedString(@"%@ (1 page)",
            @"Window title for a document with a single page");
        name = [NSString stringWithFormat:format, fileName];
    } else {
        NSString *format = NSLocalizedString(@"%@ (%d pages)",
            @"Window title for a document with multiple pages");
        name = [NSString stringWithFormat:format, fileName, cnt];
    }
    
    return name;
}

- (void)setDocumentURL:(NSURL *)newURL
{
    [newURL retain];
    [documentURL release];
    documentURL = newURL;
}

- (PDFView *)pdfView
{
    return pdfView;
}

//
// Toolbar methods
//

- (void) setupToolbar {
    if (!toolbar) {
        // Create a new toolbar instance, and attach it to our document window.
        toolbar = [(NSToolbar *)[NSToolbar alloc] initWithIdentifier:toolbarIdentifier];
    }
    
    // Set up toolbar properties: Allow customization, give a default display mode
    // and remember state in user defaults.
    [toolbar setAllowsUserCustomization:YES];
    [toolbar setAutosavesConfiguration:YES];
    [toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
    
    // We are the delegate.
    [toolbar setDelegate:self];
    
    // Attach the toolbar to the document window.
    [window setToolbar:toolbar];
}

@end

@implementation MyDocument (NSToolbarDelegation)

- (NSArray *) toolbarDefaultItemIdentifiers:(NSToolbar *)t
{
    return [NSArray arrayWithObjects: pageUpIdentifier,
        pageDownIdentifier,
        NSToolbarSeparatorItemIdentifier,
        rotateLeftIdentifier,
        rotateRightIdentifier, nil];
}

- (NSArray *) toolbarAllowedItemIdentifiers:(NSToolbar *)t
{
    return [NSArray arrayWithObjects: NSToolbarPrintItemIdentifier,
        NSToolbarCustomizeToolbarItemIdentifier,
        NSToolbarFlexibleSpaceItemIdentifier,
        NSToolbarSpaceItemIdentifier,
        NSToolbarSeparatorItemIdentifier,
        rotateLeftIdentifier,
        rotateRightIdentifier,
        pageUpIdentifier,
        pageDownIdentifier, nil];
}

- (BOOL) validateToolbarItem:(NSToolbarItem *)item {
    if ([[item itemIdentifier] isEqual:pageDownIdentifier]) {
        if ([pdfView pageCount] == [pdfView pageNumber])
            return NO;
    } else if ([[item itemIdentifier] isEqual:pageUpIdentifier]) {
        if ([pdfView pageNumber] == 1)
            return NO;
    }
    return YES;
}

- (NSToolbarItem *) toolbar:(NSToolbar *)t
      itemForItemIdentifier:(NSString *)identifier
  willBeInsertedIntoToolbar:(BOOL)flag
{
    SEL action;
    NSToolbarItem *toolbarItem;
    NSString *label, *labelComment, *tooltip, *tooltipComment, *imageName;
    
    toolbarItem = [[NSToolbarItem alloc] initWithItemIdentifier:identifier];

    if ([identifier isEqualToString:rotateLeftIdentifier]) {
        label = @"Rotate Left";
        labelComment = @"Rotate Left toolbar item";
        
        tooltip = @"Rotate every page 90 degrees to the left";
        tooltipComment = @"Tooltip for the Rotate Left toolbar item";
        
        imageName = @"RotateLeftToolbarImage";
        action = @selector(rotateLeft:);
    } else if ([identifier isEqualToString:rotateRightIdentifier]) {
        label = @"Rotate Right";
        labelComment = @"Rotate Right toolbar item";
        
        tooltip = @"Rotate every page 90 degrees to the right";
        tooltipComment = @"Tooltip for the Rotate Right toolbar item";
        
        imageName = @"RotateRightToolbarImage";
        action = @selector(rotateRight:);
    } else if ([identifier isEqualToString:pageUpIdentifier]) {
        label = @"Page Up";
        labelComment = @"Page Up toolbar item";
        
        tooltip = @"Go to the previous page";
        tooltipComment = @"Tooltip for the Page Up toolbar item";
        
        imageName = @"PageUpToolbarImage";
        action = @selector(decrementPageNumber:);
    } else if ([identifier isEqualToString:pageDownIdentifier]) {
        label = @"Page Down";
        labelComment = @"Page Down toolbar item";
        
        tooltip = @"Go to the next page";
        tooltipComment = @"Tooltip for the Page Down toolbar item";
        
        imageName = @"PageDownToolbarImage";
        action = @selector(incrementPageNumber:);
    } else {
	return nil;
    }
    
    [toolbarItem setLabel:NSLocalizedString(label, labelComment)];
    [toolbarItem setPaletteLabel:NSLocalizedString(label, labelComment)];

    [toolbarItem setToolTip:NSLocalizedString(tooltip, tooltipComment)];
    [toolbarItem setImage:[NSImage imageNamed:imageName]];
    
    [toolbarItem setTarget:[self pdfView]];
    [toolbarItem setAction:action];
    
    return [toolbarItem autorelease];
}

@end
