/*

File: BackgroundFetchingAppDelegate.h

Abstract: Header file for the BackgroundFetching main application delegate

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright � 2005 Apple Computer, Inc., All Rights Reserved

*/

#import "BackgroundFetchingAppDelegate.h"
#import "SharedConstants.h"

@implementation BackgroundFetchingAppDelegate

- (id)init {
    if (self = [super init]) {
        NSSortDescriptor *sortDescriptor = [[NSSortDescriptor alloc] initWithKey:@"character" ascending:YES];
        letterSortDescriptors = [[NSArray alloc] initWithObjects:sortDescriptor, nil];
        [sortDescriptor release];
        retainedWords = [[NSMutableArray alloc] initWithCapacity:200000];
    }
    return self;
}

- (void)runBackgroundFetcher:(NSArray *)backgroundFetcherArguments
{
    NSAssert(nil == fetcher, @"-[BackgroundFetchingAppDelegate runBackgroundFetcher:] called when fetcher != nil!");
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    @try {
        // Unpack the arguments to this method
        NSPersistentStoreCoordinator *coordinator = [backgroundFetcherArguments objectAtIndex:0];
        NSMutableArray *objectIDArray = [backgroundFetcherArguments objectAtIndex:1];
        // Create and run the BackgroundFetcher
        fetcher = [[BackgroundFetcher alloc] initWithCoordinator:coordinator objectIDArray:objectIDArray];
        [fetcher fetchObjects];
    }
    @catch (id exception) {
        [exception retain];
        [pool release];
        [exception autorelease];
        @throw exception;
    }
    [pool release];
}

- (void)checkForNewObjects:(NSMutableArray *)objectIDArray
{
	NSArray *newObjects = nil;
    // Check for new objects in the objectIDArray
    @synchronized(objectIDArray) {
        if ([objectIDArray count] > 0) {
			newObjects = [NSArray arrayWithArray:objectIDArray];
			[objectIDArray removeAllObjects];
        }
        if (objectIDArray == nil) {
            backgroundFetchComplete = YES;
        }
    }
	
	if (nil != newObjects) {
		// Bring the managed objects whose IDs were in objectIDArray into this context
		NSManagedObjectContext *context = [self managedObjectContext];
		unsigned objectIndex;
		unsigned objectCount = [newObjects count];
		for (objectIndex = 0; objectIndex < objectCount; ++objectIndex) {
			// Bring the object with the current objectID into this context
			// We don't have to manually retain the object to keep it into this context,
			// as the letters we retain in the app delegate's letters array will retain
			// the words for us, due to the Letter words relationship.
			NSManagedObject* nextWord = [context objectWithID:[newObjects objectAtIndex:objectIndex]];
            [nextWord valueForKey:@"length"];  // this fires the fault in this thread's NSManagedObjectContext
            [retainedWords addObject:nextWord];
		}
	}

    if (backgroundFetchComplete) {
        [fetcher release];
    }
    else {
        [self performSelector:@selector(checkForNewObjects:) withObject:objectIDArray afterDelay:0.3];
    }
}

// Provide sort descriptors for the letters NSTableColumn
- (NSArray *)sortDescriptors {
    return letterSortDescriptors;
}

- (void)setSortDescriptors:(NSArray *)newDescriptors {
    [newDescriptors retain];
    [letterSortDescriptors release];
    letterSortDescriptors = newDescriptors;
}

- (NSManagedObjectModel *)managedObjectModel {
    if (managedObjectModel) return managedObjectModel;
	
	NSMutableSet *allBundles = [[NSMutableSet alloc] init];
	[allBundles addObject: [NSBundle mainBundle]];
	[allBundles addObjectsFromArray: [NSBundle allFrameworks]];
    
    managedObjectModel = [[NSManagedObjectModel mergedModelFromBundles: [allBundles allObjects]] retain];
    [allBundles release];
    
    return managedObjectModel;
}

/* Change this path/code to point to your App's data store. */
- (NSString *)applicationSupportFolder {

    NSString *applicationSupportFolder = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    if ( [paths count] == 0 ) {
        NSRunAlertPanel(@"Alert", @"Can't find application support folder", @"Quit", nil, nil);
        [[NSApplication sharedApplication] terminate:self];
    } else {
        applicationSupportFolder = [[paths objectAtIndex:0] stringByAppendingPathComponent:kAppSupportFolderName];
    }
    return applicationSupportFolder;
}

- (NSManagedObjectContext *) managedObjectContext {
    NSError *error;
    NSString *applicationSupportFolder = nil;
    NSURL *url;
    NSFileManager *fileManager;
    NSPersistentStoreCoordinator *coordinator;
    static BOOL isTerminating = NO;
    
    if (managedObjectContext || isTerminating) {
        return managedObjectContext;
    }
    
    fileManager = [NSFileManager defaultManager];
    applicationSupportFolder = [self applicationSupportFolder];
    if ( ![fileManager fileExistsAtPath:applicationSupportFolder isDirectory:NULL] ) {
        NSRunAlertPanel(@"Alert", @"Can't find BackgroundFetching application support folder. Please run GenerateBackgroundFetchingData.", @"Quit", nil, nil);
        isTerminating = YES;
        [[NSApplication sharedApplication] terminate:self];
    }
    
    NSString *storeFilePath = [applicationSupportFolder stringByAppendingPathComponent:kStoreFileName];
    if (! [fileManager fileExistsAtPath: storeFilePath]) {
        NSRunAlertPanel(@"Alert", @"Can't find BackgroundFetching store. Please run GenerateBackgroundFetchingData.", @"Quit", nil, nil);
        isTerminating = YES;
        [[NSApplication sharedApplication] terminate:self];
    }
    url = [NSURL fileURLWithPath: storeFilePath];
    coordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: [self managedObjectModel]];
    if ([coordinator addPersistentStoreWithType:storeType() configuration:nil URL:url options:nil error:&error]){
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator:coordinator];
		
		// This ensures that when we pull objects into this context that we've fetched in the background
		// thread's context, we don't need to manually retain the objects to keep them in memory (and thus
		// prevent accessing those objects from triggering a fetch from the database).
		[managedObjectContext setRetainsRegisteredObjects:YES];

        backgroundFetchComplete = NO;
        NSMutableArray *objectIDArray = [[NSMutableArray alloc] init];
        // Pack an array with the two arguments to the runBackgroundFetcher: method
        NSArray *backgroundFetcherArguments = [[NSArray alloc] initWithObjects:coordinator, objectIDArray, nil];
        // Run the background fetcher in a seperate thread
        [NSThread detachNewThreadSelector:@selector(runBackgroundFetcher:) toTarget:self withObject:backgroundFetcherArguments];
        [backgroundFetcherArguments release];
        // Schedule ourselves to check for objects that have been fetched into the background context
        [self performSelector:@selector(checkForNewObjects:) withObject:objectIDArray afterDelay:0.3];
    } else {
        [[NSApplication sharedApplication] presentError:error];
    }    
    [coordinator release];
    
    return managedObjectContext;
}

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    return [[self managedObjectContext] undoManager];
}

- (IBAction) saveAction:(id)sender {
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    return NSTerminateNow;
}

- (void)dealloc {
    [managedObjectContext release];
    [managedObjectModel release];
    [letterSortDescriptors release];
    [super dealloc];
}

@end
