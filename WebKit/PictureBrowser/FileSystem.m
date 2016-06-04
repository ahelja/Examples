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
//
//  FileSystem.m
//  PictureBrowser
//
//  Created by Richard Williamson on Fri Apr 18 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import "FileSystem.h"

// Internal classes
@interface FileSystemMonitor : NSObject
{
    id delegate;
    NSConditionLock *initializationLock;
    CFRunLoopSourceRef fileSystemMonitorSource;
    CFRunLoopSourceRef delegateSource;
    CFRunLoopSourceRef terminateSource;
    CFRunLoopRef callerRunLoop;
    CFRunLoopRef monitorRunLoop;

    NSMutableArray *priorityItemsToScan;
    NSMutableArray *itemsToScan;
    NSLock *itemsToScanLock;
    NSMutableArray *results;
    NSLock *resultsLock;
    
    int state;
    
    BOOL stopped;
}

- initWithDelegate:(id)delegate;
- (void)startMonitoring;
- (void)fileSystemMonitorThread:(id)arg;
- (void)stop;
- (void)pingScanningThread;
- (void)pingDelegateThread;
- (void)addItemToScan:(FileSystemItem *)item priority:(BOOL)priority;
- (FileSystemItem *)getItemToScan;
- (void)scanItem:(FileSystemItem *)item;
- (void)addResults:(NSArray *)children forItem:(FileSystemItem *)item;
- (void)directoryScansAvailable;
- (NSArray *)_scanItem:(FileSystemItem *)item;
- (BOOL)isStopped;
@end

#define NO_SCANNING_STATE 0
#define SCANNING_STATE 1

// Private methods
@interface FileSystemItem (Private)
- (id)_initWithPath:(NSString *)path parent:(FileSystemItem *)obj monitor:(FileSystemMonitor *)doc;
- (void)_createChildren:(NSArray *)c;
- (BOOL)_loadPending;
- (void)_setModificationDate:(NSTimeInterval)m;
- (NSArray *)_dateSortedChildren;
- (NSArray *)_children;
@end

@implementation FileDataSource

- initWithPath:(NSString *)path delegate:(id)d
{
    [super init];

    monitor = [[FileSystemMonitor alloc] initWithDelegate: d];
    rootItem = [[FileSystemItem alloc] _initWithPath:path parent:nil monitor:monitor];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSDictionary *attributes =[fileManager fileAttributesAtPath:path traverseLink:YES];
    [rootItem _setModificationDate: [[attributes objectForKey:NSFileModificationDate] timeIntervalSinceReferenceDate]];

    delegate = d;

    [monitor startMonitoring];

    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(outlineViewItemWillExpand:) name:NSOutlineViewItemWillExpandNotification object:nil];
    [center addObserver:self selector:@selector(outlineViewSelectionDidChange:) name:NSOutlineViewSelectionDidChangeNotification object:nil];

    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    [rootItem release];
    [monitor stop];
    [monitor release];
    
    [super dealloc];
}

- (void)stopScanning
{
    [monitor stop];
}

- (void)setDelegate:(id)d
{
    delegate = d;
}

- (id)delegate
{
    return delegate;
}

- (FileSystemItem *)rootItem {
   if (rootItem == nil) rootItem = [[FileSystemItem alloc] _initWithPath:@"/" parent:nil monitor:monitor];
   return rootItem;       
}

// ---------------- NSOutlineViewDataSource methods

- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item {
    return (item == nil) ? 1 : [item numberOfChildren];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item {
    return (item == nil) ? YES : ([item numberOfChildren] != -1);
}

- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item {
    NSTableColumn *highlightedTableColumn = [outlineView highlightedTableColumn];
    id identifier = [highlightedTableColumn identifier];
    int sortOrder = [[highlightedTableColumn headerCell] tag];
    if ([identifier isEqual:@"NAME"]){
        if (sortOrder == SORT_ASCENDING)
            return (item == nil) ? [self rootItem] : [(FileSystemItem *)item childAtIndex:index];
        else
            return (item == nil) ? [self rootItem] : [(FileSystemItem *)item childAtIndex:[item numberOfChildren] - index - 1];
    }
    else if ([identifier isEqual:@"DATE_MODIFIED"]){
        NSArray *dateSortedChildren = [item _dateSortedChildren];
        if (sortOrder == SORT_ASCENDING)
            return (item == nil) ? [self rootItem] : [dateSortedChildren objectAtIndex:index];
        else
            return (item == nil) ? [self rootItem] : [dateSortedChildren objectAtIndex:[dateSortedChildren count] - index - 1];
    }
    return (item == nil) ? [self rootItem] : [(FileSystemItem *)item childAtIndex:index];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item {
    if ([[tableColumn identifier] isEqual: @"NAME"]){
        int numberOfLeafChildren = [item numberOfLeafChildren];
        if (numberOfLeafChildren > 0)
            return [NSString stringWithFormat:@"%@: (%d)", [item relativePath], numberOfLeafChildren];
        return (item == nil) ? @"/" : (id)[item relativePath];
    }
    else if ([[tableColumn identifier] isEqual: @"DATE_MODIFIED"]){
        NSDate *date = [NSDate dateWithTimeIntervalSinceReferenceDate: [item modificationDate]];
        return [date descriptionWithCalendarFormat:@"%m/%d/%y, %I:%M %p" timeZone:nil locale:nil];
    }
    return @"";
}

- (BOOL)outlineView:(NSOutlineView *)outlineView writeItems:(NSArray *)rows toPasteboard:(NSPasteboard *)pasteboard
{
    int i, count = [rows count];
    NSMutableArray *fileNames = [[NSMutableArray alloc] init];

    [pasteboard declareTypes:[NSArray arrayWithObject:NSFilenamesPboardType] owner:nil];
    for (i = 0; i < count; i++){
        FileSystemItem *item = [rows objectAtIndex:i];
        [fileNames addObject:[item fullPath]];
    }
    [pasteboard setPropertyList:fileNames forType:NSFilenamesPboardType];
    [fileNames release];
    
    return YES;
}

- (void)outlineViewItemWillExpand:(NSNotification *)notification
{
    NSOutlineView *outlineView = [notification object];
    if ([outlineView dataSource] == self){
        FileSystemItem *item = [[notification userInfo] objectForKey:@"NSObject"];
        if ([item _loadPending] && item != rootItem){
            [monitor addItemToScan:item priority:YES];
        }
    }
}

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    NSOutlineView *outlineView = [notification object];
    if ([outlineView dataSource] == self){
        NSEnumerator *selectedRows = [outlineView selectedRowEnumerator];
        NSNumber *row;
        FileSystemItem *item;
        while (row = [selectedRows nextObject]){
            item = [outlineView itemAtRow: [row intValue]];
            if ([item _loadPending] && item != rootItem){
                [monitor addItemToScan:item priority:YES];
            }
        }
    }
}

@end


@implementation FileSystemItem

#define IsALeafNode ((id)-1)

- (id)_initWithPath:(NSString *)path parent:(FileSystemItem *)obj monitor:(FileSystemMonitor *)m{
    if (self = [super init]) {
        if (parent == nil)
            relativePath = [path retain];
        else
            relativePath = [[path lastPathComponent] copy];
        parent = obj;
    }
    monitor = m;
    return self;
}

- (void)dealloc {
    if (children != IsALeafNode) [children release];
    [dateSortedChildren release];
    [relativePath release];
    [super dealloc];
}

- (void)validate
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSTimeInterval currentModificationDate;
    NSString *fullPath = [self fullPath];
    
    NSDictionary *attributes =[fileManager fileAttributesAtPath:fullPath traverseLink:YES];
    currentModificationDate = [[attributes objectForKey:NSFileModificationDate] timeIntervalSinceReferenceDate];
    if (modificationDate && currentModificationDate != modificationDate){
        [monitor addItemToScan:self priority:YES];
        modificationDate = currentModificationDate;
    }
}

- (FileSystemItem *)parent
{
    return parent;
}

- (void)_createChildren:(NSArray *)c
{
    int i = 0, numChildren = [c count];
    
    if (children && children != IsALeafNode){
        [children release];
        [dateSortedChildren release];
        dateSortedChildren = NULL;
    }
    children = [[NSMutableArray alloc] initWithCapacity:numChildren];
    while(i < numChildren) {
        NSString *path = [c objectAtIndex:i++];
        NSDictionary *attributes = [c objectAtIndex:i++];
        FileSystemItem *child = [[FileSystemItem alloc] _initWithPath:path parent:self monitor:monitor];
        child->modificationDate = [[attributes objectForKey:NSFileModificationDate] timeIntervalSinceReferenceDate];
        [children addObject:child];
        [child release];
    }

    pendingLoad = NO;
}

- (BOOL)_loadPending
{
    return pendingLoad;
}

- (NSArray *)children {
    if (children == NULL) {
        if (!pendingLoad){
            NSString *fullPath = [self fullPath];
            NSFileManager *fileManager = [NSFileManager defaultManager];
            BOOL isDir, valid = [fileManager fileExistsAtPath:fullPath isDirectory:&isDir];
            if (valid && isDir) {
                pendingLoad = YES;
                [monitor addItemToScan:self priority:NO];
            } else {
                children = IsALeafNode;
            }
        }
    }
    return children;
}

- (NSArray *)_children
{
    return children;
}

- (NSComparisonResult)compareModificationDate:(id)to
{
    if ([self modificationDate] > [to modificationDate])
        return NSOrderedDescending;
    else if ([self modificationDate] < [to modificationDate])
        return NSOrderedAscending;
    return NSOrderedSame;
}

- (NSArray *)_dateSortedChildren
{
    if (dateSortedChildren == NULL && children != IsALeafNode && children != NULL){
        dateSortedChildren = [[children sortedArrayUsingSelector:@selector(compareModificationDate:)] retain];
    }
    return dateSortedChildren;
}

- (NSString *)relativePath {
    return parent ? relativePath : [relativePath lastPathComponent];
}

- (NSString *)fullPath {
    return parent ? [[parent fullPath] stringByAppendingPathComponent:relativePath] : relativePath;
}

- (NSString *)URLString
{
    return [[NSURL fileURLWithPath:[self fullPath]] absoluteString];
}

- (FileSystemItem *)childAtIndex:(int)n {
    return [[self children] objectAtIndex:n];
}

- (int)numberOfChildren {
    id tmp = [self children];
    return (tmp == IsALeafNode) ? (-1) : [tmp count];
}

- (int)numberOfLeafChildren
{
    int i, numberOfChildren = [self numberOfChildren];
    int numberOfLeafChildren = 0;
    if (numberOfChildren > 0){
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSString *parentPath = [self fullPath];
        FileSystemItem *child;
        BOOL isDir;
        for (i = 0; i < numberOfChildren; i++){
            child = [self childAtIndex: i];
            [fileManager fileExistsAtPath:[parentPath stringByAppendingPathComponent:[child relativePath]] isDirectory:&isDir];
            if (!isDir)
                numberOfLeafChildren++;
        }
    }
    return numberOfLeafChildren;
}

- (NSTimeInterval)modificationDate
{
    return modificationDate;
}

- (void)_setModificationDate:(NSTimeInterval)m
{
    modificationDate = m;
}

@end


@implementation FileSystemMonitor

#define FILE_MONITOR_STARTUP 1
#define FILE_MONITOR_STARTUP_COMPLETE 2
// Runs in the monitor thread
static void monitorFileSystem(void *info);
static void monitorFileSystem(void *info)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    FileSystemMonitor *monitor = (FileSystemMonitor *)info;
    FileSystemItem *item;
    
    do {
        item = [monitor getItemToScan];
        if (item) {
            [monitor scanItem:item];
            [monitor pingDelegateThread];
        }
    } while (item && ![monitor isStopped]);
    
    [pool release];
}

// Runs in the thread that initiated the monitoring.
static void notifyDelegate(void *info);
static void notifyDelegate(void *info)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    FileSystemMonitor *monitor = (FileSystemMonitor *)info;
    
    [monitor directoryScansAvailable];
    
    [pool release];
}


// Runs in the monitor thread
static void terminateMonitor(void *info);
static void terminateMonitor(void *info)
{
    [NSThread exit];
}

- initWithDelegate:(id)d
{
    [super init];
    initializationLock = [[NSConditionLock alloc] initWithCondition:FILE_MONITOR_STARTUP];
    delegate = d;

    itemsToScan = [[NSMutableArray alloc] init];
    priorityItemsToScan = [[NSMutableArray alloc] init];
    itemsToScanLock = [[NSLock alloc] init];
    results = [[NSMutableArray alloc] init];
    resultsLock = [[NSLock alloc] init];

    return self;
}

- (void)startMonitoring
{
    callerRunLoop = CFRunLoopGetCurrent();
    CFRunLoopSourceContext delegateSourceContext = {0, self, NULL, NULL, NULL, NULL, NULL, NULL, NULL, notifyDelegate};
    delegateSource = CFRunLoopSourceCreate(NULL, 0, &delegateSourceContext);
    CFRunLoopAddSource(callerRunLoop, delegateSource, kCFRunLoopDefaultMode);
    
    [NSThread detachNewThreadSelector:@selector(fileSystemMonitorThread:) toTarget:self withObject:nil];

    // Wait for the fileSystemMonitorThread: to finish initializing.
    [initializationLock lockWhenCondition:FILE_MONITOR_STARTUP_COMPLETE];
    NSConditionLock *l = initializationLock;
    initializationLock = nil;
    [initializationLock unlock];
    [l release];
}


// Runs in the monitor thread
-(void)fileSystemMonitorThread:(id)arg
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    [initializationLock lockWhenCondition:FILE_MONITOR_STARTUP];
    monitorRunLoop = CFRunLoopGetCurrent();
    CFRunLoopSourceContext fileSystemMonitorContext = {0, self, NULL, NULL, NULL, NULL, NULL, NULL, NULL, monitorFileSystem};
    fileSystemMonitorSource = CFRunLoopSourceCreate(NULL, 0, &fileSystemMonitorContext);
    CFRunLoopAddSource(monitorRunLoop, fileSystemMonitorSource, kCFRunLoopDefaultMode);

    CFRunLoopSourceContext terminateContext = {0, self, NULL, NULL, NULL, NULL, NULL, NULL, NULL, terminateMonitor};
    terminateSource = CFRunLoopSourceCreate(NULL, 0, &terminateContext);
    CFRunLoopAddSource(monitorRunLoop, terminateSource, kCFRunLoopDefaultMode);

    [initializationLock unlockWithCondition:FILE_MONITOR_STARTUP_COMPLETE];

    while (YES) {
        [[NSRunLoop currentRunLoop] run];
    }

    [pool release];
}

// Normally called from the thread that initiated the monitoring.
- (void)pingScanningThread
{
    CFRunLoopSourceSignal(fileSystemMonitorSource);
    if (CFRunLoopIsWaiting(monitorRunLoop))
        CFRunLoopWakeUp(monitorRunLoop);
}

- (void)pingDelegateThread
{
    CFRunLoopSourceSignal(delegateSource);
    if (CFRunLoopIsWaiting(callerRunLoop))
        CFRunLoopWakeUp(callerRunLoop);
}

- (void)stop
{
    stopped = YES;
    CFRunLoopSourceSignal(terminateSource);
}

- (BOOL)isStopped
{
    return stopped;
}

- (void)dealloc
{
    [initializationLock release];
    CFRelease(fileSystemMonitorSource);
    CFRelease(delegateSource);
    CFRelease(terminateSource);

    [itemsToScan release];
    [priorityItemsToScan release];
    [itemsToScanLock release];
    [results release];
    [resultsLock release];

    [super dealloc];
}

- (NSArray *)_scanItem:(FileSystemItem *)item
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *directory = [item fullPath];
    NSArray *array = [fileManager directoryContentsAtPath:directory];
    NSString *fullPath;
    NSMutableArray *children;
    NSDictionary *attributes;
    BOOL isDir;
        
    int cnt, numChildren = [array count];
    children = [[[NSMutableArray alloc] initWithCapacity:numChildren] autorelease];
    for (cnt = 0; cnt < numChildren; cnt++) {
        NSString *path = [array objectAtIndex:cnt];
        fullPath = [directory stringByAppendingPathComponent:path];
        attributes = [fileManager fileAttributesAtPath:fullPath traverseLink:YES];
        isDir = [[attributes objectForKey: NSFileType] isEqual:NSFileTypeDirectory];
        if ([delegate shouldIncludePath:path inDirectory:directory] || isDir){
            [children addObject:path];
            [children addObject:attributes];
        }
    }

    return children;
}

// Called from the monitor thread
- (void)scanItem:(FileSystemItem *)item
{
    NSArray *children = [self _scanItem:item];
    [self addResults:children forItem:item];
}

- (void)addItemToScan:(FileSystemItem *)item priority:(BOOL)priority
{
    if (stopped)
        return;
        
    if (state == NO_SCANNING_STATE){
        state = SCANNING_STATE;
        [delegate scanningStarted];
    }

    [itemsToScanLock lock];
    
    if (priority && ![priorityItemsToScan containsObject:item]){
        [priorityItemsToScan addObject:item];
        unsigned i = [itemsToScan indexOfObject:item];
        if (i != NSNotFound)
            [itemsToScan removeObjectAtIndex:i];
    }
        
    else if (![itemsToScan containsObject:item])
        [itemsToScan addObject:item];
    
    [itemsToScanLock unlock];

    [self pingScanningThread];
}

- (FileSystemItem *)getItemToScan
{
    FileSystemItem *item = nil;
    int count;
    
    [itemsToScanLock lock];

    count = [priorityItemsToScan count];
    if (count){
        item = [priorityItemsToScan objectAtIndex: count - 1];
        [[item retain] autorelease];
        [priorityItemsToScan removeLastObject];
    }
    else {
        count = [itemsToScan count];
        if ([itemsToScan count]){
            item = [itemsToScan objectAtIndex: [itemsToScan count] - 1];
            [[item retain] autorelease];
            [itemsToScan removeLastObject];
        }
    }
    [itemsToScanLock unlock];
    
    return item;
}

- (void)addResults:(NSArray *)children forItem:(FileSystemItem *)item
{
    [resultsLock lock];
    
    [results addObject:item];
    [results addObject:children];

    [resultsLock unlock];
}

- (void)directoryScansAvailable
{
    int i = 0, count;
    FileSystemItem *item;
    NSArray *localResults;
    NSArray *children;

    if (stopped)
        return;
        
    // Make copy to minimize lock contention.
    [resultsLock lock];
    localResults = [results copy];
    [results removeAllObjects];
    [resultsLock unlock];
    
    count = [localResults count];
    while (i < count){
        item = [localResults objectAtIndex:i++];
        children = [localResults objectAtIndex:i++];
        [item _createChildren:children];
        [delegate scanCompletedForItem:(FileSystemItem *)item];
    }
    [localResults release];

    BOOL moreItems = NO;
    [itemsToScanLock lock];
    if ([priorityItemsToScan count] || [itemsToScan count])
        moreItems = YES;
    [itemsToScanLock unlock];
    
    if (moreItems == NO && state == SCANNING_STATE){
        state = NO_SCANNING_STATE;
        [delegate scanningFinished];
    }
}


@end
