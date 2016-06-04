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
#import "PictureBrowserProtocol.h"
#import "Document.h"

#import <WebKit/WebKit.h>

NSString *loadingStatus = @"Loading";
NSString *scanningStatus = @"Scanning";

NSString *fileSeparator = @"|";
NSString *directoryListSeparator = @";";

@implementation Document

- (void)dealloc
{
    [fileDataSource release];
    
    [super dealloc];
}

- (void)close
{
    [fileDataSource stopScanning];
    
    [super close];
}

- (NSString *)windowNibName {
    return @"Document";
}

- (BOOL)readFromFile:(NSString *)fileName ofType:(NSString *)type 
{
    static BOOL registeredDataProviderClass = NO;
    
    if (!registeredDataProviderClass){
        registeredDataProviderClass = YES;
        [PictureBrowserProtocol setDataProvider:[Document class]];
    }
    
    [self makeWindowControllers];
    [self showWindows];

    [[webView backForwardList] setPageCacheSize: 8];
    [webView setMaintainsBackForwardList:YES];
    [webView setUIDelegate:self];
    [webView setFrameLoadDelegate:self];

    [statusText setStringValue:@""];
    [progressIndicator setStyle:NSProgressIndicatorSpinningStyle];
    [progressIndicator setDisplayedWhenStopped:NO];
            
    fileDataSource = [[FileDataSource alloc] initWithPath:[self fileName] delegate: self];
    [outlineView setDataSource:fileDataSource];
    [outlineView setDelegate:self];
    [outlineView setVerticalMotionCanBeginDrag:YES];
    [outlineView setHighlightedTableColumn:[outlineView tableColumnWithIdentifier:@"NAME"]];
    [outlineView setIndicatorImage:[NSImage imageNamed:@"NSAscendingSortIndicator"] inTableColumn:[outlineView tableColumnWithIdentifier:@"NAME"]];
    [outlineView setAutoresizesOutlineColumn:YES];

    FileSystemItem *item = [fileDataSource rootItem];
    [outlineView expandItem: item];
    [outlineView sizeToFit];
    
    return YES;
}

#define MAX_SIZE_PER_PAGE	4096*1024

+ (BOOL)isImageExtension:(NSString *)path
{
    NSArray *imageTypes = [NSImage imageFileTypes];
    int i, count = [imageTypes count];
    
    for (i = 0; i < count; i++){
        if ([[path pathExtension] caseInsensitiveCompare: [imageTypes objectAtIndex: i]] == NSOrderedSame)
            return YES;
    }
    return NO;
}

- (void)_updateViewForSelection
{
    NSMutableArray *items;
    int i;
    
    // If a directory is selected show thumbnails for all the images
    // in the directory.
    NSEnumerator *selectedRows = [outlineView selectedRowEnumerator];
    NSNumber *row;
    NSMutableSet *directories = nil; [[NSMutableSet alloc] init];
    
    items = [[[NSMutableArray alloc] init] autorelease];
    while (row = [selectedRows nextObject]){
        FileSystemItem *item = [outlineView itemAtRow: [row intValue]];
        int numChildren = [item numberOfChildren];
        if (numChildren >= 0){
            if (!directories)
                directories = [[NSMutableSet alloc] init];
            [directories addObject:item];
            for (i = 0; i < numChildren; i++){
                // Use the data source method to ensure that we get the correct
                // sort order.
                FileSystemItem *child = [fileDataSource outlineView:outlineView child:i ofItem:item];
                if ([child numberOfChildren] < 0){
                    [items addObject:child];
                }
            }
        }
        else {
            if (![directories containsObject:[item parent]])
                [items addObject: item];
        }
    }
    [directories release];

    [self loadPageForItems:items];
}

- (void)loadPageForItems:(NSArray *)items;
{
    FileSystemItem *item, *parentItem = nil;
    NSString *info = nil;

    int numItems = [items count];
    if (numItems > 0){
        NSEnumerator *objects = [items objectEnumerator];
        while(item = [objects nextObject]){
            if (!parentItem)
                parentItem = [item parent];
            if (!info){
                info = [parentItem fullPath];
            }
            if ([item parent] != parentItem){
                parentItem = [item parent];
                info = [info stringByAppendingString:directoryListSeparator];
                info = [info stringByAppendingString:[parentItem fullPath]];
            }
                
            info = [info stringByAppendingString:fileSeparator];
            info = [info stringByAppendingString:[item relativePath]];
        }
    }
    else {
        info = @"";
    }

    NSString *fixedInfo = (NSString *)CFURLCreateStringByAddingPercentEscapes(NULL, (CFStringRef)info, NULL, NULL, kCFStringEncodingUTF8);
    NSURL *fakeURL = [NSURL URLWithString: [NSString stringWithFormat:@"%@://%@", PictureBrowserScheme, fixedInfo]];
    [fixedInfo release];
    
    if (fakeURL){
        // Only update the page if the URL is different that what is currently displayed.
        if (![fakeURL isEqual: [[[[webView mainFrame] dataSource] request] URL]]){
            NSURLRequest *request = [[[NSURLRequest alloc] initWithURL: fakeURL] autorelease];
            [[webView mainFrame] loadRequest:request];
        }
    }
    else {
        NSLog (@"%s: Unable to create URL for %@", __FUNCTION__, info);
    }
}


static NSString *_thumbnailPageTemplate;
static NSString *_imagePageTemplate;
static NSString *_transparentImageURL;

+ (NSData *)generateDataForRequest:(NSURLRequest *)request;
{
    int i, j;
    NSString *HTMLString = nil;
    NSString *item;

    if (!_thumbnailPageTemplate){
        _thumbnailPageTemplate = [[NSString stringWithContentsOfFile: [[NSBundle mainBundle] pathForResource:@"thumbnail_template" ofType:@"html"]] retain];
        _imagePageTemplate = [[NSString stringWithContentsOfFile: [[NSBundle mainBundle] pathForResource:@"image_template" ofType:@"html"]] retain];
        _transparentImageURL = [[[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"nicetitle" ofType:@"png"]] absoluteString] retain];
    }

    NSString *fixedResource = (NSString *)CFURLCreateStringByReplacingPercentEscapes(NULL,(CFStringRef)[[request URL] resourceSpecifier],CFSTR(""));
    NSArray *directoryLists = [fixedResource componentsSeparatedByString:directoryListSeparator];
    [fixedResource release];
    NSArray *files = [[directoryLists objectAtIndex: 0] componentsSeparatedByString:fileSeparator];
    int numItems = [files count];
    int numDirectories = [directoryLists count];
    
    NSString *directory = nil;
    
    if (numDirectories == 1 && numItems == 2){
        NSRect screenFrame = [[NSScreen mainScreen] frame];
        directory = [files objectAtIndex:0];
        item = [files objectAtIndex: 1];
        NSString *fullPath = [NSString stringWithFormat:@"%@/%@", directory, item];
        NSString *fixedFullPath = (NSString *)CFURLCreateStringByAddingPercentEscapes(NULL, (CFStringRef)fullPath, NULL, (CFStringRef)@"?", kCFStringEncodingUTF8);
        HTMLString = [NSString stringWithFormat:_imagePageTemplate, (int)(screenFrame.size.height - 128), fixedFullPath, fixedFullPath];
        [fixedFullPath release];
    }
    else if (numDirectories >= 1){
        NSMutableString *imageURLs = nil;
        NSMutableString *imageLabels = nil;
        
        for (i = 0; i < numDirectories; i++){
            if (i != 0){
                files = [[directoryLists objectAtIndex: i] componentsSeparatedByString:fileSeparator];
                numItems = [files count];
            }
            directory = [files objectAtIndex:0];
            for (j = 1; j < numItems; j++){
                item = [files objectAtIndex: j];
                NSString *URLString = [NSString stringWithFormat:@"%@/%@", directory, item];
                NSString *imageLabel = [NSString stringWithFormat:@"\"%@\"", item];
                NSString *fixedURLString = (NSString *)CFURLCreateStringByAddingPercentEscapes(NULL, (CFStringRef)URLString, NULL, (CFStringRef)@"?", kCFStringEncodingUTF8);
                if (imageURLs){
                    [imageURLs appendString:@", "];
                    [imageURLs appendString: [NSString stringWithFormat:@"\"%@\"", fixedURLString]];
                    [imageLabels appendString:@", "];
                    [imageLabels appendString:imageLabel];
                }
                else {
                    imageURLs = [[NSMutableString alloc] init];
                    [imageURLs appendString:[NSString stringWithFormat:@"\"%@\"", fixedURLString]];
                    imageLabels = [[NSMutableString alloc] init];
                    [imageLabels appendString:imageLabel];
                }
                [fixedURLString release];
            }
        }
        HTMLString = [NSString stringWithFormat:_thumbnailPageTemplate, imageURLs, imageLabels, _transparentImageURL];

        [imageURLs release];
        [imageLabels release];
    }
    else {
        HTMLString = @"<HTML><BODY>No images</HTML></BODY>";
    }

    return [HTMLString dataUsingEncoding: NSUnicodeStringEncoding];
}

- (void)addStatusItem:(NSString *)string
{
    if (!statusItems)
        statusItems = [[NSMutableArray alloc] init];
    [statusItems addObject:string];
    [statusText setStringValue:[statusItems componentsJoinedByString:@", "]];
}

- (void)removeStatusItem:(NSString *)string
{
    [statusItems removeObject:string];
    [statusText setStringValue:[statusItems componentsJoinedByString:@", "]];
}

- (void)startProgressIndicator
{
    if (progressIndicatorCount == 0)
        [progressIndicator startAnimation:self];
    progressIndicatorCount++;
}

- (void)stopProgressIndicator
{
    progressIndicatorCount--;
    if (progressIndicatorCount == 0)
        [progressIndicator stopAnimation:self];
}


// ----------------------- FileSystemMonitorDelegate

- (BOOL)shouldIncludePath:(NSString *)path inDirectory:(NSString *)directory
{
    return [Document isImageExtension:path];
}

- (void)scanCompletedForItem:(FileSystemItem *)item
{
    [outlineView reloadItem:item reloadChildren:YES];
    
    if ([outlineView isRowSelected:[outlineView rowForItem:item]]){
        [self _updateViewForSelection];
    }
}

- (void)scanningStarted
{
    [self addStatusItem:scanningStatus];
    [self startProgressIndicator];
}

- (void)scanningFinished
{
    [self removeStatusItem:scanningStatus];
    [self stopProgressIndicator];
}


// ----------------------- IBAction methods.
- (IBAction)goBack:(id)sender
{
    [webView goBack:sender];
}

- (IBAction)goForward:(id)sender
{
    [webView goForward:sender];
}

- (IBAction)outlineViewItemClicked:sender
{
    //[webView stringByEvaluatingJavaScriptFromString:@"document.mumble();"];
    [self _updateViewForSelection];
}


// ----------------------- WebLocationChangeDelegate methods
- (void)webView:(WebView *)sender didStartProvisionalLoadForFrame:(WebFrame *)frame
{
    [self addStatusItem:loadingStatus];
    [self startProgressIndicator];
}

- (void)webView:(WebView *)sender locationChangeDone:(NSError *)error forDataSource:(WebDataSource *)dataSource
{
    [self removeStatusItem:loadingStatus];
    [self stopProgressIndicator];
    if ([sender canGoBack])
        [backButton setEnabled: YES];
    else
        [backButton setEnabled: NO];
        
    if ([sender canGoForward])
        [forwardButton setEnabled: YES];
    else
        [forwardButton setEnabled: NO];
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

// ----------------------- NSOutlineView delegate methods
- (void)outlineViewItemDidExpand:(NSNotification *)notification
{
    FileSystemItem *item = [[notification userInfo] objectForKey: @"NSObject"];
    
    [item validate];
}

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    [self _updateViewForSelection];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item {
    return NO;
}

- (void)tableView:(NSTableView*)tableView didClickTableColumn:(NSTableColumn *)tableColumn
{
    int tag = [[tableColumn headerCell] tag];
    NSImage *indicatorImage;
    NSArray *columns = [outlineView tableColumns];;

    if (tag == SORT_ASCENDING){
        [[tableColumn headerCell] setTag:SORT_DESCENDING];
        indicatorImage = [NSImage imageNamed:@"NSDescendingSortIndicator"];
    }
    else {
        [[tableColumn headerCell] setTag:SORT_ASCENDING];
        indicatorImage = [NSImage imageNamed:@"NSAscendingSortIndicator"];
    }
    [tableView setHighlightedTableColumn: tableColumn];
    [outlineView setIndicatorImage:indicatorImage inTableColumn:tableColumn];
    int i, count = [columns count];
    for (i = 0; i < count; i++){
        NSTableColumn *aColumn = [columns objectAtIndex: i];
        if (aColumn != tableColumn){
            [outlineView setIndicatorImage:nil inTableColumn:aColumn];
        }
    }
    [((NSOutlineView *)tableView) reloadItem:[fileDataSource rootItem] reloadChildren:YES];
}


// ----------------------- WebUIDelegate delegate methods
- (void)webView: (WebView *)wv runJavaScriptAlertPanelWithMessage:(NSString *)message
{
}

- (NSArray *)webView: (WebView *)wv contextMenuItemsForElement: (NSDictionary *)theElement  defaultMenuItems: (NSArray *)defaultMenuItems
{
    NSURL *imageURL;

    imageURL = [theElement objectForKey:WebElementImageURLKey];
    if(imageURL){
        NSMenuItem *menuItem1 = [[NSMenuItem alloc] init];
        [menuItem1 setTarget:self];
        [menuItem1 setTitle:@"Zoom In"];
        [menuItem1 setAction:@selector(zoomImageIn:)];
        [menuItem1 setRepresentedObject:imageURL];
        NSMenuItem *menuItem2 = [[NSMenuItem alloc] init];
        [menuItem2 setTarget:self];
        [menuItem2 setTitle:@"Zoom Out"];
        [menuItem2 setAction:@selector(zoomImageOut:)];
        [menuItem2 setRepresentedObject:imageURL];
/*
        // Add a menu item to rotate the image.
        NSMenuItem *menuItem3 = [[NSMenuItem alloc] init];
        [menuItem3 setTarget:self];
        [menuItem3 setEnabled:NO];
        [menuItem3 setTitle:@"Rotate"];
        [menuItem3 setAction:@selector(rotateImage:)];
        [menuItem3 setRepresentedObject:imageURL];
*/
        return [NSArray arrayWithObjects:menuItem1, menuItem2, nil];
    }
    return nil;
}

// ----------------------- context menu methods
// These method pass the image URL 'under' the context menu to
// JavaScript.
- (void)zoomImageIn:(id)sender
{
    NSString *returnString;
    
    returnString = [webView stringByEvaluatingJavaScriptFromString:[NSString stringWithFormat:@"document.zoomImageIn(\"%@\");",[sender representedObject]]];
}

- (void)zoomImageOut:(id)sender
{
    NSString *returnString;
    
    returnString = [webView stringByEvaluatingJavaScriptFromString:[NSString stringWithFormat:@"document.zoomImageOut(\"%@\");",[sender representedObject]]];
}

// Does nothing.  No way to rotate an image with HTML/CSS.  The image could
// be rotated with CoreGraphics or Cocoa and then refreshed on the page by
// this method.
- (void)rotateImage:(id)sender
{
    NSString *returnString;
    
    returnString = [webView stringByEvaluatingJavaScriptFromString:[NSString stringWithFormat:@"document.rotateImage(\"%@\");",[sender representedObject]]];
}

@end
