/* CGPDFViewer - MainController.m
 *
 * A simple multi-document CoreGraphics PDF example program which:
 * 1. Opens and pages through PDF documents and
 * 2. Rotates said PDF documents.
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

#import "MainController.h"
#import "MyDocument.h"
#import "PDFView.h"

@interface MainController (Private)

- (PDFView *)currentPDFView;

@end

@implementation MainController

// These six methods pass the appropriate message to the current
// (frontmost) PDFView.  They are called by the menu items and enable
// keyboard shortcuts.

- (IBAction)firstPage:(id)sender
{
    [[self currentPDFView] setPageNumber:1];
}

- (IBAction)lastPage:(id)sender
{
    [[self currentPDFView] setPageNumber:[[self currentPDFView] pageCount]];
}

- (IBAction)pageDown:(id)sender
{
    [[self currentPDFView] incrementPageNumber:self];
}

- (IBAction)pageUp:(id)sender
{
    [[self currentPDFView] decrementPageNumber:self];
}

- (IBAction)rotateLeft:(id)sender
{
    [[self currentPDFView] rotateLeft:self];
}

- (IBAction)rotateRight:(id)sender
{
    [[self currentPDFView] rotateRight:self];
}

// Retrieve the PDF View of the main window.

- (PDFView *)currentPDFView
{
    return [[[[NSApp mainWindow] windowController] document] pdfView];
}

// Disable inappropriate menu items.

- (BOOL)validateMenuItem:(id <NSMenuItem>)item
{
    if ([item action] == @selector(firstPage:)
        ||[item action] == @selector(pageUp:)) {
        if (![self currentPDFView]
            || [[self currentPDFView] pageNumber] == 1) {
            return NO;
        }
    } else if ([item action] == @selector(lastPage:)
               || [item action] == @selector(pageDown:)) {
        if (![self currentPDFView]
            || [[self currentPDFView] pageNumber] == [[self currentPDFView] pageCount]) {
            return NO;
        }
    } else if ([item action] == @selector(rotateLeft:)
               || [item action] == @selector(rotateRight:)) {
        if (![self currentPDFView]) {
            return NO;
        }
    }
    
    return YES;
}

@end
