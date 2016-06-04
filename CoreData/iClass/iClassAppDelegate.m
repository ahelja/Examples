/*

File: iClassAppDelegate.m

Abstract: Implementation for iClass' main Application Delegate

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

#import "iClassAppDelegate.h"
#import <CoreData/NSManagedObjectContext.h>

@implementation iClassAppDelegate

- (NSManagedObjectModel *)managedObjectModel {
    if (managedObjectModel) return managedObjectModel;
	
	NSMutableSet *allBundles = [[NSMutableSet alloc] init];
	[allBundles addObject: [NSBundle mainBundle]];
	[allBundles addObjectsFromArray: [NSBundle allFrameworks]];
    
    managedObjectModel = [[NSManagedObjectModel mergedModelFromBundles: [allBundles allObjects]] retain];
    [allBundles release];
    
    return managedObjectModel;
}

- (NSString *)applicationSupportFolder {
    return [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"iClass"];
}


- (id)createOrAddStoreWithConfiguration:(NSString *)configuration andURL:(NSURL *)url toPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator {
    NSError *error = nil;
    id store = [coordinator addPersistentStoreWithType:NSXMLStoreType configuration:configuration URL:url options:nil error:&error];
    if (!store) {
        [[NSApplication sharedApplication] presentError:error];
    }
    return store;
}

- (void)createOrAddPublicStoreToPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator {
    NSURL *publicURL;
    publicURL = [NSURL fileURLWithPath: [applicationSupportFolder stringByAppendingPathComponent: @"iClass_public.xml"]];
    publicStore = [self createOrAddStoreWithConfiguration: @"public" andURL: publicURL toPersistentStoreCoordinator: coordinator];
}

- (void)createOrAddPrivateStoreToPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator {
    NSURL *privateURL;
    privateURL = [NSURL fileURLWithPath: [applicationSupportFolder stringByAppendingPathComponent: @"iClass_private.xml"]];
    privateStore = [self createOrAddStoreWithConfiguration: @"private" andURL: privateURL toPersistentStoreCoordinator: coordinator];
}

- (IDGenerator *)idGenerator {
    return idGenerator;
}

- (void)getOrCreateIDGeneratorInStore: (id)store {
    NSFetchRequest *request = [[self managedObjectModel] fetchRequestTemplateForName: @"generators"];
    [request setAffectedStores: [NSArray arrayWithObject: store]];
    
    NSError *error = nil;
    
    NSArray *generators = [[self managedObjectContext] executeFetchRequest: request error: &error];
    
    if (nil == error) {
        if (0 == [generators count]) {
            idGenerator = [[IDGenerator alloc] initWithEntity: [[managedObjectModel entitiesByName] valueForKey: @"IDGenerator"] insertIntoManagedObjectContext: managedObjectContext];
                [managedObjectContext assignObject: idGenerator toPersistentStore: store];
        } else {
            idGenerator = [[generators objectAtIndex: 0] retain];
        }
    } else {
        [[NSApplication sharedApplication] presentError:error];
    }
    return;
}

- (NSManagedObjectContext *) managedObjectContext {

    NSFileManager *fileManager;
    NSPersistentStoreCoordinator *coordinator;
    
    if (managedObjectContext) {
        return managedObjectContext;
    }
    
    fileManager = [NSFileManager defaultManager];
    applicationSupportFolder = [self applicationSupportFolder];
    if ( ![fileManager fileExistsAtPath:applicationSupportFolder isDirectory:NULL] ) {
        [fileManager createDirectoryAtPath:applicationSupportFolder attributes:nil];
    }
    
    coordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: [self managedObjectModel]];
    
    [self createOrAddPublicStoreToPersistentStoreCoordinator: coordinator];
    [self createOrAddPrivateStoreToPersistentStoreCoordinator: coordinator];

    if (publicStore && privateStore) {
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator: coordinator];
        
        [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(handleManagedObjectContextUpdateNotification:) name: NSManagedObjectContextObjectsDidChangeNotification object: managedObjectContext];

        [self getOrCreateIDGeneratorInStore: privateStore];
        
    }

    [coordinator release];
    
    return managedObjectContext;
}

- (IBAction) saveAction:(id)sender {
    NSError *error = nil;
    if (![[self managedObjectContext] save:&error]) {
        [[NSApplication sharedApplication] presentError:error];
    }
}

- (IBAction) resetDatabaseAction:(id)sender {
    NSFetchRequest *request = [[NSFetchRequest alloc] init];
    NSArray *entities = [managedObjectModel entities];
    NSEnumerator *objectEnumerator = [entities objectEnumerator];
    NSEntityDescription *entityDescription = nil;
    
    while (entityDescription = (NSEntityDescription *)[objectEnumerator nextObject]) {
        if (nil == [entityDescription superentity]) {
            [request setEntity: entityDescription];
            [managedObjectContext executeFetchRequest: request error: nil];
        }
    }
    
    NSSet *registeredObjects = [managedObjectContext registeredObjects];
    objectEnumerator = [registeredObjects objectEnumerator];
    NSManagedObject *managedObject = nil;
    
    while (managedObject = (NSManagedObject *)[objectEnumerator nextObject]) {
        if (![managedObject isDeleted]) {
            [managedObjectContext deleteObject: managedObject];
        }
    }
    
    [self getOrCreateIDGeneratorInStore: privateStore];
    
    NSError *error = nil;
    
    [managedObjectContext save: &error];
    if (![[self managedObjectContext] save:&error]) {
        [[NSApplication sharedApplication] presentError:error];
    }
}

 - (void)handleManagedObjectContextUpdateNotification:(NSNotification *)notification {
    [self performSelector:@selector(saveAction:) withObject: nil afterDelay: 0];
} 

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    return [[self managedObjectContext] undoManager];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    NSError *error;
    NSManagedObjectContext *context;

    context = [self managedObjectContext];
    if ((context != nil) && (![context save:&error])) {
        [[NSApplication sharedApplication] presentError:error];
        return NSTerminateCancel;
    }
    return NSTerminateNow;
}

@end
