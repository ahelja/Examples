/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
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
/*
    SnippetController.m
    Copyright (C) 2003 Apple Computer, Inc. All rights reserved.
    
    Public source file.
*/

#import "SnippetController.h"

#import <unistd.h>

#import <WebKit/WebKit.h>

#import <Foundation/NSError.h>


@interface AttributedStringView : NSView
{
    NSAttributedString *attributedString;
}
- (void)setAttributedString: (NSAttributedString *)string;
@end


@implementation SnippetController

- (void)dealloc
{
    [textStorage release];
    [attributedStringScrollView release];
    [webView release];
    [super dealloc];
}

- (void)windowWillClose:(NSNotification *)notification
{
    [textStorage release];
    textStorage = nil;
    [attributedStringScrollView release];
    attributedStringScrollView = nil;
    
    [self release];
}

- (NSString *)windowNibName
{
    return @"Snippet";
}

- (void)awakeFromNib
{
    [attributedStringScrollView retain];
    [attributedStringScrollView removeFromSuperview];
    [[self window] setDelegate:self];
    
    // Keep a reference to WebView.  The splitview has the only reference
    // to it, and we don't want it to be dealloced when we remove it from
    // the splitview.
    [webView retain];
}

- (void)_resetViews
{
    if ([webView superview] != splitView) {
        [attributedStringScrollView removeFromSuperview];
        [splitView addSubview:webView];
    }

    [[self window] orderFront:nil];
}

- (void)_updateTitleWithURL
{
    NSString *title = [[[[[webView mainFrame] dataSource] request] URL] absoluteString];
    
    [[self window] setTitle:title];
}

// Open the URL specified in the textfield.
- (IBAction)updateFromURL:(id)sender
{
    NSURL *URL = [NSURL URLWithString: [urlField stringValue]];
    
    if (!URL){
        [[self window] setTitle: @"Invalid URL"];
        return;
    }
    
    [self _resetViews];
    
    // Make sure we're set at the location change delegate.
    [webView setFrameLoadDelegate: self];
    
    WebFrame *mainFrame = [webView mainFrame];
    if (documentURL != URL){
        [documentURL release];
        documentURL = [URL retain];
    }
    [mainFrame loadRequest:[NSURLRequest requestWithURL:URL]];
}

// Create a DataResourceRequest from the string in the
// text area.
- (IBAction)updateFromText:(id)sender
{
    NSString *htmlString = [textView string];
    
    if ([htmlString length] == 0){
        [[self window] setTitle: @"Invalid URL"];
        return;
    }
        
    [self _resetViews];
    
    WebFrame *mainFrame = [webView mainFrame];
    [mainFrame loadHTMLString:htmlString baseURL:documentURL];
}

- (void)receivedError: (NSError *)error
{
    [[self window] setTitle: [error localizedDescription]];
}

- (void)loadComplete
{
    WebFrame *mainFrame = [webView mainFrame];
    id documentRepresentation = [[mainFrame dataSource] representation];

    if ([documentRepresentation canProvideDocumentSource]){
        [textView setString: [documentRepresentation documentSource]];
    }

    id viewRepresentation = [[mainFrame frameView] documentView];

    if ([(NSCell *)showUsingNSTextSwitch state] && [viewRepresentation conformsToProtocol: @protocol(WebDocumentText)]){
        id<WebDocumentText> viewTextRepresentation = viewRepresentation;
        
        NSAttributedString *attributedString = [viewTextRepresentation attributedString];
        NSTextStorage *oldStorage = textStorage;
        textStorage = [[NSTextStorage alloc] initWithAttributedString:attributedString];
        [[attributedStringView layoutManager] replaceTextStorage:textStorage];
        [oldStorage release];

        [webView removeFromSuperview];
        [webView setFrameOrigin: NSMakePoint (300,0)];
        [splitView addSubview:attributedStringScrollView];
    }        

    [splitView adjustSubviews];

    [self _updateTitleWithURL];
}

- (void)setTitle:(NSString *)t
{
    [[self window] setTitle:t];
}

- (WebDataSource *)mainDataSource
{
    return [[webView mainFrame] dataSource];
}

//------------------------------------------
// LocationChangeDelegate
//------------------------------------------

- (void)webView:(WebView *)wv locationChangeDone:(NSError *)error forDataSource:(WebDataSource *)dataSource
{
    if (error != nil){
        [self receivedError: error];
    }
    else if (dataSource == [self mainDataSource]) {
        [self loadComplete];
    }
}

- (void)webView:(WebView *)sender didReceiveTitle:(NSString *)title forFrame:(WebFrame *)frame
{
    if ([frame dataSource] == [self mainDataSource]) {
        [self setTitle:title];
    }
}

- (void)webView:(WebView *)sender didFailProvisionalLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
    [self webView:sender locationChangeDone:error forDataSource:[frame provisionalDataSource]];
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    [self webView:sender locationChangeDone:nil forDataSource:[frame dataSource]];
}

- (void)webView:(WebView *)sender didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
    [self webView:sender locationChangeDone:error forDataSource:[frame dataSource]];
}

@end

@implementation AttributedStringView

- (void)dealloc
{
    [attributedString release];
    [super dealloc];
}

- (void)setAttributedString:(NSAttributedString *)string
{
    NSAttributedString *copy = [string copy];
    [attributedString release];
    attributedString = copy;
}

- (void)drawRect:(NSRect)rect
{
    [[NSColor whiteColor] set];
    NSRectFill(rect);
    [attributedString drawInRect:rect];
}

@end
