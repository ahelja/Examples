/*

File: BackgroundFetcher.m

Abstract: Implementation file for the class that fetches managed objects into a context in a background thread

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

#import "BackgroundFetcher.h"


@implementation BackgroundFetcher

- (id)initWithCoordinator:(NSPersistentStoreCoordinator *)coordinator objectIDArray:(NSMutableArray *)objectIDArray
{
    if (self = [super init]) {
        if (nil == coordinator) {
            [NSException raise:@"InvalidArgumentException" format:@"-[BackgroundFetcher initWithCoordinator:] was passed a nil coordinator!"];
        }
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator:coordinator];

        // this retains all the objects we fetch
        [managedObjectContext setRetainsRegisteredObjects:YES];
        
        @synchronized(objectIDsFetched) {
            NSAssert([objectIDsFetched count] == 0, @"Object ID array passed to BackgroundFetcher is not empty!");
        }
        objectIDsFetched = objectIDArray;  // joint ownership, hence the synchronized blocks
    }
    return self;
}

- (void)fetchWordsForLetter:(NSManagedObject *)letter
{
    NSFetchRequest *wordFetch = [[[NSFetchRequest alloc] init] autorelease];
	[wordFetch setEntity:[NSEntityDescription entityForName:@"Word" inManagedObjectContext:managedObjectContext]];
    NSPredicate *wordPredicate = [NSPredicate predicateWithFormat:@"firstLetter = %@", letter];
    [wordFetch setPredicate:wordPredicate];
    
	NSError *wordFetchError;
    NSArray *words = [managedObjectContext executeFetchRequest:wordFetch error:&wordFetchError];

	if (nil == words) {
        NSLog([NSString stringWithFormat:@"Fetch error in background thread: %@", wordFetchError]);
        return;
    }
	
	// Fault the words relationship on the letter
    // The precise contents of the relationship are a separate fault-firing (fetch) from the actual words data
	[[letter valueForKey:@"words"] count];
	
	@synchronized(objectIDsFetched) {
        unsigned wordCount = [words count];
        unsigned i;
        for (i = 0; i < wordCount; ++i) {
            [objectIDsFetched addObject:[[words objectAtIndex:i] objectID]];
        }
	}
}

- (void)fetchObjects
{
    NSFetchRequest *letterFetch = [[[NSFetchRequest alloc] init] autorelease];
    [letterFetch setEntity:[NSEntityDescription entityForName:@"Letter" inManagedObjectContext:managedObjectContext]];
    NSAssert(nil == letters, @"-[BackgroundFetcher fetchObjects] called when letters array is non-nil!");
    
	NSSortDescriptor* sortOrdering = [[[NSSortDescriptor alloc] initWithKey:@"character" ascending:YES] autorelease];
	[letterFetch setSortDescriptors:[NSArray arrayWithObject:sortOrdering]];
	
    NSError *letterFetchError;
    letters = [managedObjectContext executeFetchRequest:letterFetch error:&letterFetchError];
	[letters retain];
	
    if (nil == letters) {
        NSLog([NSString stringWithFormat:@"Fetch error in background thread: %@", letterFetchError]);
        return;
    }

	int letterIndex;
    for (letterIndex = 0; letterIndex < [letters count]; ++letterIndex) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        @try {
            [self fetchWordsForLetter:[letters objectAtIndex:letterIndex]];
        }
        @catch (id exception) {
            [exception retain];
            [pool release];
            [exception autorelease];
            @throw exception;
        }
		[pool release];
    }
    
    // We need to wait until the consumer has brought all the words into its own NSManagedObjectContext
    // If we don't, then when we release our NSManagedObjectContext, all the row data
    // we fetched will go away because no own else is using it (yet)
    BOOL notDone = YES;
    while (notDone) {
        @synchronized(objectIDsFetched) {
            if ([objectIDsFetched count] == 0) {
                [objectIDsFetched release];
                objectIDsFetched = nil;
                notDone = NO;
            }
        }
        if (notDone) {
            [NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.0]];
        }
    }
}

- (void)dealloc
{
    [letters release];
	letters = nil;
    [managedObjectContext release];
	managedObjectContext = nil;
    [super dealloc];
}

@end
