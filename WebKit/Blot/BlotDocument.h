//
//  BlotDocument.h
//  Blot
//
//  Created by Ken Kocienda on Mon Jan 12 2004.
//  Copyright (c) 2004 Apple Computer. All rights reserved.
//


#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

static NSString *HTMLDocumentType = @"HTML document";

@interface BlotDocument : NSDocument
{
    IBOutlet WebView *webView;
    IBOutlet NSPanel *locationSheet;
    IBOutlet NSTextField *locationField;

    NSString *source;
    NSString *_linkURLString;
    
    WebArchive *archive;
}

- (IBAction)openLocation:(id)sender;
- (IBAction)hideLocationSheet:(id)sender;
- (IBAction)goToLocationFieldAddress:(id)sender;

- (void)goToURL:(NSURL *)URL;
- (void)loadArchive:(WebArchive *)archive;
- (void)insertLinkWithURLString:(NSString *)URLString;

- (IBAction)pasteTextAsMarkup:(id)sender;

@end
