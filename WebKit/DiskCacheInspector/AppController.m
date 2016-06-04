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
/* AppController.m */

#import "AppController.h"
#import "ContentViewerController.h"
#import "HTTPInfoController.h"
#import "ValidityInfoController.h"

#import <Foundation/NSURLCache.h>
#import <Foundation/NSURLResponse.h>

static AppController *sharedAppController;
static CFRunLoopRef mainRunLoop;
static CFRunLoopSourceRef fileLoadProgressRunLoopSource;
static CFRunLoopSourceRef fileLoadDoneRunLoopSource;

#define DefaultDiskCacheDirectory ([NSString stringWithFormat:@"%@/%@", NSHomeDirectory(), @"Library/Caches/Safari"])

// menu item tags
#define ViewMenuTag               (400)
#define ContentViewerMenuItemTag  (401)
#define HTTPInfoMenuItemTag       (402)
#define ValidityInfoMenuItemTag   (403)

@implementation AppController

+(AppController *)sharedAppController
{
    return sharedAppController;
}

-(id)init
{
    self = [super init];
    
    sharedAppController = self;
	NSNotificationCenter *defaultCenter = [NSNotificationCenter defaultCenter];
    [defaultCenter addObserver:self selector:@selector(diskCacheObjectDidChange:) name:DiskCacheObjectChangedNotification object:nil];

    return self;
}

-(DiskCacheObject *)currentDiskCacheObject
{
    return currentDiskCacheObject;
}

-(id <NSMenuItem>)menuItem:(int)menuItemTag inMenu:(int)menuTag
{
    NSMenuItem *mainMenuItem = [[NSApp mainMenu] itemWithTag:menuTag];
    NSMenu *menu = [mainMenuItem submenu];
    return [menu itemWithTag:menuItemTag];
}

-(void)toggleViewingValidityInfoWindow:(id)sender
{
    ValidityInfoController *controller = [ValidityInfoController sharedController];
    [controller toggleHideShowWindow];
    
    id <NSMenuItem> menuItem = [self menuItem:ValidityInfoMenuItemTag inMenu:ViewMenuTag];
    if ([controller showingWindow]) {
        [menuItem setState:NSOnState];
    }
    else {
        [menuItem setState:NSOffState];
    }
}

-(void)toggleViewingContentViewerWindow:(id)sender
{
    ContentViewerController *controller = [ContentViewerController sharedController];
    [controller toggleHideShowWindow];
    
    id <NSMenuItem> menuItem = [self menuItem:ContentViewerMenuItemTag inMenu:ViewMenuTag];
    if ([controller showingWindow]) {
        [menuItem setState:NSOnState];
    }
    else {
        [menuItem setState:NSOffState];
    }
}

-(void)toggleViewingHTTPInfoWindow:(id)sender
{
    HTTPInfoController *controller = [HTTPInfoController sharedController];
    [controller toggleHideShowWindow];
    
    id <NSMenuItem> menuItem = [self menuItem:HTTPInfoMenuItemTag inMenu:ViewMenuTag];
    if ([controller showingWindow]) {
        [menuItem setState:NSOnState];
    }
    else {
        [menuItem setState:NSOffState];
    }
}

-(NSCachedURLResponse *)cachedResponseForPath:(NSString *)filePath
{
    NSCachedURLResponse *cachedResponse = nil;
    NSData *data = [NSData dataWithContentsOfFile:filePath];
    NSUnarchiver *unarchiver = nil;

    NS_DURING
        unarchiver = [[NSUnarchiver alloc] initForReadingWithData:data];
        if (unarchiver) {
            // there is an NSURLRequest first in line
            // we don't care about that
            [unarchiver decodeObject];
            cachedResponse = [[unarchiver decodeObject] retain];
        }
    NS_HANDLER
        cachedResponse = nil;
    NS_ENDHANDLER

    [unarchiver release];

    return cachedResponse;
}

-(void)setDiskCacheDirectory:(NSString *)path
{
    if (path != diskCacheDirectory) {
        [diskCacheDirectory release];
        diskCacheDirectory = [path retain];
    }
    [diskCacheDirectoryLabel setStringValue:diskCacheDirectory];
}

-(void)setItemCountLabel
{
    NSString *itemCountString;
    if ([filteredDiskCacheObjectArray count] != [diskCacheObjectArray count]) {
        itemCountString = [NSString stringWithFormat:@"Displaying %d of %d items", [filteredDiskCacheObjectArray count], [diskCacheObjectArray count]];
    } 
    else {
        if ([diskCacheObjectArray count] == 1) {
            itemCountString = @"1 item";
        }
        else {
            itemCountString = [NSString stringWithFormat:@"%d items", [diskCacheObjectArray count]];
        }
    }
    [itemCountLabel setStringValue:itemCountString];
}

-(IBAction)filterDiskCacheObjectArrayUsingSearchString:(id)sender
{
    NSString *searchString;
    
    searchString = [searchField stringValue];
    if (searchString && [searchString length] > 0) {
        NSMutableArray *array = [NSMutableArray array];
        int i;
        int count = [diskCacheObjectArray count];
        for (i = 0; i < count; i++) {
            DiskCacheObject *diskCacheObject = [diskCacheObjectArray objectAtIndex:i];
            NSCachedURLResponse *cachedResponse = [diskCacheObject cachedResponse];
            NSString *check = [[[cachedResponse response] URL] absoluteString];
            NSRange range = [check rangeOfString:searchString options:(NSLiteralSearch | NSCaseInsensitiveSearch)];
            if (range.location != NSNotFound) {
                [array addObject:diskCacheObject];
            }
        }
        [filteredDiskCacheObjectArray release];
        filteredDiskCacheObjectArray = [array sortedArrayUsingSelector:@selector(compare:)];
        [filteredDiskCacheObjectArray retain];
    }
    else {
        [filteredDiskCacheObjectArray release];
        filteredDiskCacheObjectArray = [diskCacheObjectArray retain];
    }

    [fileList reloadData];
    
    if ([filteredDiskCacheObjectArray count] > 0 && [fileList selectedRow] == -1) {
        [fileList selectRow:0 byExtendingSelection:NO];
    }
    else {
        [self sendDiskCacheObjectDidChangeNotification];
    }
    
    [self setItemCountLabel];
    [mainWindow makeFirstResponder:fileList];
} 

-(void)performLoadDiskCacheFiles
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSFileManager *defaultManager = [NSFileManager defaultManager];
    NSMutableArray *fileListArray = [NSMutableArray array];
    NSMutableArray *objectArray = [NSMutableArray array];
    NSString *filePath;
    NSString *fullFilePath;
    BOOL isDir;

    [defaultManager changeCurrentDirectoryPath:diskCacheDirectory];

    NSDirectoryEnumerator *directoryEnumerator = [defaultManager enumeratorAtPath:diskCacheDirectory];
    while (filePath = [directoryEnumerator nextObject]) {
        if ([defaultManager fileExistsAtPath:filePath isDirectory:&isDir]) {
            fullFilePath = [NSString stringWithFormat:@"%@/%@", diskCacheDirectory, filePath];
            if (!isDir) {
                [fileListArray addObject:filePath];
            }
        }
    }

    fileLoadProgressTotalFiles = [fileListArray count];
    fileLoadProgressCounter = 0;

    NSEnumerator *enumerator = [fileListArray objectEnumerator];
    while (filePath = [enumerator nextObject]) {
        fullFilePath = [NSString stringWithFormat:@"%@/%@", diskCacheDirectory, filePath];
        NSCachedURLResponse *cachedResponse = [self cachedResponseForPath:fullFilePath];
        if (cachedResponse) {
            NSDictionary *attributes = [defaultManager fileAttributesAtPath:fullFilePath traverseLink:NO];
            NSNumber *size = nil;
            if (attributes) {
                size = [attributes objectForKey:NSFileSize];
            }
            [objectArray addObject:[DiskCacheObject diskCacheObjectWithCachedResponse:cachedResponse size:size path:filePath]];
        }

        // update file load progress indicator
        fileLoadProgressCounter++;
        CFRunLoopSourceSignal(fileLoadProgressRunLoopSource);
        CFRunLoopWakeUp(mainRunLoop);
    }

    NSArray *sortedArray = [objectArray sortedArrayUsingSelector:@selector(compare:)];
    [diskCacheObjectArray release];
    diskCacheObjectArray = [sortedArray retain];

    CFRunLoopSourceSignal(fileLoadDoneRunLoopSource);
    CFRunLoopWakeUp(mainRunLoop);
    
    [pool release];
}

- (void)openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    if (returnCode == NSOKButton) {
        NSArray *filenames = [openPanel filenames];
        NSString *directory = [filenames objectAtIndex:0];
        [self setDiskCacheDirectory:directory];
        
        [self performSelector:@selector(loadDiskCacheFiles:)
               withObject:self
               afterDelay:0];
    }
}

-(IBAction)runDiskCacheDirectoryOpenPanel:(id)sender
{
    [openPanel setResolvesAliases:YES];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setCanChooseFiles:NO];
    [openPanel setAllowsMultipleSelection:NO];
    NSString *directory = [NSString stringWithFormat:@"%@/%@", NSHomeDirectory(), @"Library/Caches"];
    [openPanel beginSheetForDirectory:directory file:nil types:nil modalForWindow:mainWindow modalDelegate:self didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

-(IBAction)loadDiskCacheFiles:(id)sender
{
    [itemCountLabel setStringValue:@""];
    [filePathLabel setStringValue:@""];
    [fileSizeLabel setStringValue:@""];
    [createdLabel setStringValue:@""];

    [fileLoadProgressLabel setStringValue:@""];
    [fileLoadProgressIndicator setIndeterminate:YES];
    [fileLoadProgressIndicator startAnimation:self];

    [NSApp beginSheet:fileLoadProgressSheet modalForWindow:mainWindow modalDelegate:self didEndSelector:nil contextInfo:nil];
    [NSThread detachNewThreadSelector:@selector(performLoadDiskCacheFiles) toTarget:self withObject:nil];
}

-(void)performDelayedLaunchOperations
{
    // reopen utility windows
    [[ContentViewerController sharedController] reopenWindowIfNeeded];
    [[HTTPInfoController sharedController] reopenWindowIfNeeded];
    [[ValidityInfoController sharedController] reopenWindowIfNeeded];
    
    // load cache files from default location
    [self loadDiskCacheFiles:self];
}

-(void)updateFileLoadProgress
{
    if ([fileLoadProgressIndicator isIndeterminate]) {
        [fileLoadProgressIndicator stopAnimation:self];
        [fileLoadProgressIndicator setIndeterminate:NO];
        [fileLoadProgressIndicator setMaxValue:fileLoadProgressTotalFiles];
        [fileLoadProgressIndicator setDoubleValue:0];
    }
    [fileLoadProgressIndicator setDoubleValue:fileLoadProgressCounter];

    NSMutableString *fileLoadProgressString = [NSMutableString string];
    int fileLoadProgressStringLength = [fileLoadProgressString length];
    if (fileLoadProgressStringLength) {
        [fileLoadProgressString deleteCharactersInRange:NSMakeRange(0, fileLoadProgressStringLength)];
    }
    [fileLoadProgressString appendFormat:@"%d", fileLoadProgressCounter];
    [fileLoadProgressString appendString:@" of "];
    [fileLoadProgressString appendFormat:@"%d", fileLoadProgressTotalFiles];
    [fileLoadProgressLabel setStringValue:fileLoadProgressString];
}

static void _updateFileLoadProgress(void *info)
{
    AppController *controller = (AppController *)info;
    [controller updateFileLoadProgress];
}

-(void)fileLoadDone
{
    [NSApp endSheet:fileLoadProgressSheet];
    [fileLoadProgressSheet close];
    [searchField setStringValue:@""];
    [self filterDiskCacheObjectArrayUsingSearchString:self];
    [mainWindow makeFirstResponder:fileList];
}

static void _fileLoadDone(void *info)
{
    AppController *controller = (AppController *)info;
    [controller fileLoadDone];
}

- (void)awakeFromNib
{
    [self setDiskCacheDirectory:DefaultDiskCacheDirectory];

    openPanel = [NSOpenPanel openPanel];

    // set up the run loop source we use to handle the file load progress indicator
    mainRunLoop = CFRunLoopGetCurrent();

    CFRunLoopSourceContext fileLoadProgressRunLoopContext = {0, self, NULL, NULL, NULL, NULL, NULL, NULL, NULL, _updateFileLoadProgress};
    fileLoadProgressRunLoopSource = CFRunLoopSourceCreate(NULL, 0, &fileLoadProgressRunLoopContext);
    CFRunLoopAddSource(mainRunLoop, fileLoadProgressRunLoopSource, kCFRunLoopCommonModes);

    CFRunLoopSourceContext fileLoadDoneRunLoopContext = {0, self, NULL, NULL, NULL, NULL, NULL, NULL, NULL, _fileLoadDone};
    fileLoadDoneRunLoopSource = CFRunLoopSourceCreate(NULL, 1, &fileLoadDoneRunLoopContext);
    CFRunLoopAddSource(mainRunLoop, fileLoadDoneRunLoopSource, kCFRunLoopCommonModes);

    [self performSelector:@selector(performDelayedLaunchOperations)
               withObject:nil
               afterDelay:0];
}

-(id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
    id result = nil;
    
    if (aTableView == fileList) {
        DiskCacheObject *diskCacheObject = [filteredDiskCacheObjectArray objectAtIndex:rowIndex];
        NSCachedURLResponse *cachedResponse = [diskCacheObject cachedResponse];
        NSString *identifier = [aTableColumn identifier];
        if ([identifier isEqualToString:@"URL"]) {
            result = [[cachedResponse response] URL];
            if (!result) { 
                result = @"NO URL"; 
            }
        } 
    }
    return result;
}

-(int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if (aTableView == fileList) {
        return [filteredDiskCacheObjectArray count];
    }
    else {
        return 0;
    }
}

#define OneGigabyte (1024 * 1024 * 1024)
#define OneMegabyte (1024 * 1024)
#define OneKilobyte (1024)

- (void)diskCacheObjectDidChange:(NSNotification *)aNotification
{
    [currentDiskCacheObject release];
    currentDiskCacheObject = [[aNotification object] retain];
    if (!currentDiskCacheObject) {
        return;
    }
    
    NSCachedURLResponse *cachedResponse = [currentDiskCacheObject cachedResponse];
    
    // set icon
    NSString *pathForIcon = [[[cachedResponse response] URL] absoluteString];
    NSImage *icon = [[NSWorkspace sharedWorkspace] iconForFileType:[pathForIcon pathExtension]];
    [iconView setImage:icon];

    // set file path
    [filePathLabel setStringValue:[currentDiskCacheObject path]];

    // set file size
    NSString *fileSizeString = nil;
    int fileSize = [[currentDiskCacheObject size] intValue];
    if (fileSize >= OneGigabyte) {
        fileSizeString = [NSString stringWithFormat:@"%.2f GB", (float)fileSize / OneGigabyte];
    }
    else if (fileSize >= OneMegabyte) {
        fileSizeString = [NSString stringWithFormat:@"%.2f MB", (float)fileSize / OneMegabyte];
    }
    else if (fileSize >= OneKilobyte) {
        fileSizeString = [NSString stringWithFormat:@"%.2f KB", (float)fileSize / OneKilobyte];
    }
    else if (fileSize == 1) {
        fileSizeString = @"1 byte";
    }
    else {
        fileSizeString = [NSString stringWithFormat:@"%d bytes", fileSize];
    }
    [fileSizeLabel setStringValue:fileSizeString];
    
    // set created date
    NSDate *created = nil;
    if ([cachedResponse isKindOfClass:[NSHTTPURLResponse class]]) {
        NSString *createdString = [[(NSHTTPURLResponse *)cachedResponse allHeaderFields] objectForKey:@"Date"];
        created = [[NSCalendarDate alloc] initWithString:createdString calendarFormat:@"%a, %d %b %Y %H:%M:%S %Z"];
    }
    if (!created) {
        NSString *fullPath = [NSString stringWithFormat:@"%@/%@", diskCacheDirectory, [currentDiskCacheObject path]];
        created = [[[NSFileManager defaultManager] fileAttributesAtPath:fullPath traverseLink:NO] objectForKey:NSFileCreationDate];
    }
    [createdLabel setObjectValue:created];
}

- (void)sendDiskCacheObjectDidChangeNotification
{
    DiskCacheObject *diskCacheObject = nil;
    if ([fileList selectedRow] >= 0) {  
        diskCacheObject = [filteredDiskCacheObjectArray objectAtIndex:[fileList selectedRow]];
    }
	
	NSNotificationCenter *defaultCenter = [NSNotificationCenter defaultCenter];
    [defaultCenter postNotificationName:DiskCacheObjectChangedNotification object:diskCacheObject];
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
    [self sendDiskCacheObjectDidChangeNotification];
}

@end

