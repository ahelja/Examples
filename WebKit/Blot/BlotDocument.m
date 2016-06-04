//
//  BlotDocument.m
//  Blot
//
//  Created by Ken Kocienda on Mon Jan 12 2004.
//  Copyright (c) 2004 Apple Computer. All rights reserved.
//

#import "BlotDocument.h"
#import "BlotPreferences.h"
#import "WebViewExtras.h"

#define LocationFieldStringKey @"LocationFieldString"
#define WebArchiveDocumentType @"Web archive"

@implementation BlotDocument

- (void)dealloc
{
    [webView setFrameLoadDelegate:nil];
    [webView setPolicyDelegate:nil];
    [archive release];
    [source release];
    [super dealloc];
}

- (NSString *)windowNibName
{
    return @"BlotDocument";
}

-(void)_setSource:(NSString *)_source
{
    NSString *oldSource = source;
    source = [_source copy];
    [oldSource release];
}

- (void)_setArchive:(WebArchive *)theArchive
{
    [theArchive retain];
    [archive release];
    archive = theArchive;
}

- (void)_replaceEncodingTag
{
    DOMDocument *doc = [[webView mainFrame] DOMDocument]; 

    DOMHTMLMetaElement *metaTag;
    
    // Remove any meta tags with http-equiv Content-Type.    
    DOMNodeList *metaTags = [doc getElementsByTagName:@"meta"];
    unsigned long length = [metaTags length];
    unsigned long i;
    for (i = 0; i != length; ++i) {
        metaTag = (DOMHTMLMetaElement *)[metaTags item:i];
        NSString *httpEquiv = [metaTag httpEquiv];
        if ([httpEquiv caseInsensitiveCompare:@"content-type"] == NSOrderedSame) {
            [[metaTag parentNode] removeChild:metaTag];
        }
    }
    
    // Find or create head element.
    DOMNodeList *heads = [doc getElementsByTagName:@"head"];
    DOMNode *head;
    if ([heads length] > 0) {
        head = [heads item:0];
    } else {
        head = [doc createElement:@"head"];
        DOMNode *docElem = [doc documentElement];
        if ([docElem hasChildNodes]) {
            [docElem insertBefore:head :[docElem firstChild]];
        } else {
            [docElem appendChild:head];
        }
    }
    
    // Create a new meta tag and put it inside head element.
    metaTag = (DOMHTMLMetaElement *)[doc createElement:@"meta"];
    [metaTag setHttpEquiv:@"Content-Type"];
    [metaTag setContent:@"text/html; charset=UTF-8"];
    if ([head hasChildNodes]) {
        [head insertBefore:metaTag :[head firstChild]];
    } else {
        [head appendChild:metaTag];
    }
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];

    [webView setEditable:YES];

    WebFrame *frame = [webView mainFrame];
    if (source != nil) {
        [frame loadHTMLString:source baseURL:nil];
        [self _setSource:nil];
    } else if (archive != nil) {
        [frame loadArchive:archive];
        [self _setArchive:nil];
    } else {
        [frame loadHTMLString:[webView defaultEditingSource] baseURL:nil];
    }
    
    [aController setWindowFrameAutosaveName:@"Blot"];

    NSString *string = [[NSUserDefaults standardUserDefaults] stringForKey:LocationFieldStringKey];
    if (string != nil) {
        [locationField setStringValue:string];
    }
}

- (NSData *)dataRepresentationOfType:(NSString *)type
{
    if ([type isEqualToString:HTMLDocumentType]) {
        NSString *s = [(DOMHTMLElement *)[[[webView mainFrame] DOMDocument] documentElement] outerHTML];
        return [s dataUsingEncoding:NSUTF8StringEncoding];
    } else if ([type isEqualToString:WebArchiveDocumentType]) {
        return [[(DOMHTMLElement *)[[[webView mainFrame] DOMDocument] documentElement] webArchive] data];
    }
    return nil;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)type
{
    if ([type isEqualToString:HTMLDocumentType]) {
        NSString *theSource = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        [self _setSource:theSource];
        [theSource release];
        return YES;
    } else if ([type isEqualToString:WebArchiveDocumentType]) {
        WebArchive *theArchive = [[WebArchive alloc] initWithData:data];
        [self _setArchive:theArchive];
        [theArchive release];
        return YES;
    }
    return NO;
}

#pragma mark IBACTIONS

- (IBAction)openLocation:(id)sender
{
    [NSApp beginSheet:locationSheet modalForWindow:[self windowForSheet] modalDelegate:nil didEndSelector:nil contextInfo:NULL];
}

- (IBAction)hideLocationSheet:(id)sender
{
    [[NSUserDefaults standardUserDefaults] setObject:[locationField stringValue] forKey:LocationFieldStringKey];
    [NSApp endSheet:locationSheet];
    [locationSheet close];
}

- (IBAction)goToLocationFieldAddress:(id)sender
{
    [self hideLocationSheet:self];
    
    NSURL *URL = [NSURL URLWithString:[locationField stringValue]];
    if (URL) {
        [self goToURL:URL];
    }
}

#pragma mark LOADING

- (void)goToURL:(NSURL *)URL
{
    NSURLRequest *request = [[NSURLRequest alloc] initWithURL:URL];
    [[webView mainFrame] loadRequest:request];
    [request release];
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    [self _replaceEncodingTag];

    // The following code tests "Mail Link to This Page". It puts a link in the HTML page just as Mail would.
    if (_linkURLString) {
        DOMDocument *document = [[webView mainFrame] DOMDocument];
        DOMElement *anchor = [document createElement:@"a"];
        [anchor setAttribute:@"href" :_linkURLString];
        [anchor appendChild:[document createTextNode:_linkURLString]];
        
        // FIXME: replaceSelectionWithNode doesn't work here because of: 
        // <rdar://problem/3752755> replaceSelectionWithNode: does nothing inside of webView:didFinishLoadForFrame:
        [webView replaceSelectionWithNode:anchor];
        [_linkURLString release];
        _linkURLString = nil;
    }
    
    // Make the document view of the main frame first responder if the main frame
    // is the one that just finished loading.
    if ([[webView mainFrame] frameView] == [frame frameView])
        [[webView window] makeFirstResponder:[[frame frameView] documentView]];
}

- (void)insertLinkWithURLString:(NSString *)URLString
{
    [URLString retain];
    [_linkURLString release];
    _linkURLString = URLString;
}

- (void)loadArchive:(WebArchive *)theArchive
{
    [[webView mainFrame] loadArchive:theArchive];
}

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener
{
    int navigationType = [[actionInformation objectForKey:WebActionNavigationTypeKey] intValue];
    if (navigationType == WebNavigationTypeOther) {
        [listener use];
    } else if (navigationType == WebNavigationTypeLinkClicked) {
        [[NSWorkspace sharedWorkspace] openURL:[actionInformation objectForKey:WebActionOriginalURLKey]];
        [listener ignore];
    } else {
        // Ignore WebNavigationTypeFormSubmitted, WebNavigationTypeBackForward, 
        // WebNavigationTypeReload and WebNavigationTypeFormResubmitted.
        [listener ignore];
    }
}

# pragma mark DEBUG

- (IBAction)pasteTextAsMarkup:(id)sender
{
    NSString *markup = [[NSPasteboard generalPasteboard] stringForType:NSStringPboardType];
    [webView replaceSelectionWithMarkupString:markup ? markup : @""];
}

@end
