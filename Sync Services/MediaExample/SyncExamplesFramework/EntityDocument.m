/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
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
 OF SUCH DAMAGE.
 */
//
//  EntityDocument.m
//  SyncExamples
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import "EntityDocument.h"
#import <SyncServices/SyncServices.h>
#import "EntityModel.h"
#import "DataSource.h"
#import "RecordTransformer.h"

static NSString *EntityDocumentType = @"Plist";
static BOOL ForceRefreshSync = NO; // Set to YES if we have no data file on startup

typedef enum {
	NoSyncType,
	AnySyncType,
	SlowSyncType,
	RefreshSyncType
} SyncType;

#define kDefaultsTrickleSync @"TrickleSync"

@interface EntityDocument(PrivateAPI)
// Registering the client
- (id)_getSyncEntities;

// Trickle syncing
- (void)_setTrickleSync:(BOOL)trickleSync;
- (void)_handleSyncRequestNotification:(NSNotification*)notification;

// Core syncing methods
- (BOOL)_syncDocumentWithSyncType:(SyncType)syncType;
- (BOOL)_syncDocumentWithClient:(ISyncClient *)client andSession:(ISyncSession *)session mustSlowSync:(BOOL)mustSlowSync;
- (void)_client:(ISyncClient *)client mightWantToSyncEntityNames:(NSArray *)entityNames;
- (BOOL)_pushChangesToSyncSession:(ISyncSession *)session usingDataSource:(id)dataSource andRecordTransformer:(id)recordTransformer mustSlowSync:(BOOL)mustSlowSync;
- (BOOL)_pullChangesFromSyncSession:(ISyncSession *)session usingDataSource:(id)dataSource andRecordTransformer:(id)recordTransformer;
@end

@implementation EntityDocument

// Initialization and deallocation

// Loads the entity model for the first time. Used by -init method.
- (id)_loadEntityModelFromFile:(NSString *)fileName
{
	id resourcePath = [[NSBundle mainBundle] resourcePath];
	return [[EntityModel alloc] initWithContentsOfFile:[resourcePath stringByAppendingPathComponent:fileName]];
}

// Creates the data source with an entity model loaded from the bundle. The tables in
// the data source are initially empty.

static SyncType sSavedSyncType = NoSyncType;

- (void) _handleSyncAvailabilityChangedNotification:(NSNotification*)notification {
	NSLog(@"Notified that syncing is reenabled");
	if (sSavedSyncType != NoSyncType) {
		NSLog(@"Saved sync type of %d, syncing", sSavedSyncType);
		sSavedSyncType = NoSyncType;
		[self _syncDocumentWithSyncType:sSavedSyncType];
	}
}

- (id)init
{
    self = [super init];
    if (self) {
		_trickleSync = [[NSUserDefaults standardUserDefaults] boolForKey:kDefaultsTrickleSync];
		_fastSync = NO; // Assume we must slow sync unless we read a data file that tells otherwise
		[[NSApplication sharedApplication] setDelegate:self];
		// Register the schema
		[self registerSchemaIfNecessary];
		
		// Creates the initial data source with the given entity model (will have tables but no records)
		NSLog(@"Creating data source and registering for notifications for data changed");
		_myDataSource = [[DataSource alloc] initWithModel:[[self _loadEntityModelFromFile:[DataSource defaultEntityModelFileName]] autorelease]];
		[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_handleSyncRequestNotification:) name:DataSourceChangedNotification object:_myDataSource];

		// Load the data source content (tables and records) from the backup file, if it exists
		NSLog(@"Initializing with data from file %@", [self backupFileName]);
		if ([self readFromFile:[self backupFileName] ofType:EntityDocumentType] == NO) {
			NSLog(@"No data from backup file on init, creating empty new data source");
			if (_trickleSync) ForceRefreshSync = YES; // On first sync, we'll refresh state from Truth
		}
		else {
			NSLog(@"Read in some data on init from backup file");
		}
		[(NSDistributedNotificationCenter *)[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(_handleSyncAvailabilityChangedNotification:) name:ISyncAvailabilityChangedNotification object:@"YES"];
	}
	return self;
}

- (void)dealloc {
	[(NSDistributedNotificationCenter *)[NSDistributedNotificationCenter defaultCenter] removeObserver:self name:ISyncAvailabilityChangedNotification object:nil];
    [_myDataSource release];
	[_oneShotTimer invalidate];
	[_oneShotTimer release];
    [super dealloc];
}


// Overriding typical NSDocument methods

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	if (_trickleSync == YES) {
		[[self trickleButton] setState:NSOnState];
		[self _setTrickleSync:YES];
	}	
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
	// Enhancement: just sync with a push, don't bother pulling, or
	// only pull if can be done quickly...
	if ([_myDataSource isChanged] == YES) {
		if (_trickleSync == YES) [self syncDocument:self];
		else [self writeToFile:[self backupFileName] ofType:EntityDocumentType];
	}
	[(NSDistributedNotificationCenter *)[NSDistributedNotificationCenter defaultCenter] removeObserver:self name:ISyncAvailabilityChangedNotification object:nil];
}

- (BOOL)isDocumentEdited {
	return NO;
}


// Loading and saving documents

// Enhancement: Instead of simply _fastSync, use anchor and match to metaclient value in the truth
- (NSData *)dataRepresentationOfType:(NSString *)aType
{
	// Convert the DataSource object dictionary to an NSData for archiving because it can contain
	// unsupported property list types, like instances of NSCalendarDate and custom entities.
	NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
		[NSArchiver archivedDataWithRootObject:[_myDataSource representation]], @"data source",
		[NSNumber numberWithBool:_fastSync], @"fast sync", nil];
	
	NSAssert([aType isEqualToString:EntityDocumentType], @"Unknown type");
	return (NSData *)CFPropertyListCreateXMLData(kCFAllocatorDefault, (CFPropertyListRef)dict);
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)docType {
	NSString *error;
	NSDictionary *dict = (NSDictionary *)CFPropertyListCreateFromXMLData(kCFAllocatorDefault, (CFDataRef)data, 
																		 kCFPropertyListMutableContainersAndLeaves, 
																		 (CFStringRef *)&error);
	// Converts the NSData back into a DataSource representation that may contain non-property list types
	[_myDataSource setRepresentation:[NSUnarchiver unarchiveObjectWithData:[dict objectForKey:@"data source"]]];
	_fastSync = [[dict objectForKey:@"fast sync"] boolValue];
    return YES;
}


// Getting EntityDocument properties

- (id)dataSource
{
	return _myDataSource;
}

// Returns a default backup file name for the sync operation to use
- (NSString *)backupFileName
{
	NSString *fname = [self fileName];
	if (fname == nil){
		NSString *dname;
		fname = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/Application Support/SyncExamples"];
		[[NSFileManager defaultManager] createDirectoryAtPath:fname attributes:nil]; // In case it doesn't exist
		dname = [self clientIdentifier];
		fname = [fname stringByAppendingPathComponent:[dname stringByAppendingPathExtension:@"plist"]];
	}
	return fname;
}


// Registering the schema

- (void) registerSchemaIfNecessary
{
	NS_DURING
		NSBundle *bundle = [NSBundle bundleForClass:[EntityDocument class]];
		[[ISyncManager sharedManager] registerSchemaWithBundlePath:[[bundle resourcePath] stringByAppendingPathComponent:@"SyncExamples.syncschema"]];
	NS_HANDLER
		NSLog(@"Exception occured trying to register schema for SyncExamples:\n%@", localException);
	NS_ENDHANDLER	
}


// Registering the client

// Returns a sync client for this app
- (ISyncClient *)getSyncClient {
	// probably need process id in the client name to be unique?
	
	// Return an existing client?
	ISyncClient *client = [[ISyncManager sharedManager] clientWithIdentifier:[self clientIdentifier]];
	if (client != nil) {
		return client;
	}
	
	client = [[ISyncManager sharedManager] registerClientWithIdentifier:[self clientIdentifier] descriptionFilePath:
		[[NSBundle mainBundle] pathForResource:@"ClientDescription" ofType:@"plist"]];
	
    // #warning Workaround for client image, remove when fixed
	NSString *imageFile = [[client imagePath] lastPathComponent];
	[client setImagePath:[[NSBundle mainBundle] pathForResource:[imageFile stringByDeletingPathExtension]
														 ofType:[imageFile pathExtension]]];
	
	if (client == nil) {
		[NSException raise:[self clientIdentifier] format:@"can't register client."];
	}
	
	return client;
}

// Transforms the entity model into a description of entities and their properties that Sync Services expects
- (id)getSyncEntities
{
	// Create the dictionary object that SyncServices expects
	NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	
	NSEnumerator *entityEnumerator = [[[[_myDataSource entityModel] allEntities] valueForKey:@"entityName"] objectEnumerator];
	NSEnumerator *attributeEnumerator = [[[[_myDataSource entityModel] allEntities] valueForKey:@"attributeKeys"] objectEnumerator];
    id entityName, attributeKeys;
	
	while ((entityName = [entityEnumerator nextObject]) && (attributeKeys = [attributeEnumerator nextObject])) {
		[dict setValue:attributeKeys forKey:entityName];
    }
	//NSLog(@"getSyncEntities where dict=%@", [dict description]);
	return dict;
}


// Tricle Syncing Support

- (void) _setTrickleSync:(BOOL)trickleSync {
	ISyncClient* client = [self getSyncClient];
	_trickleSync = trickleSync;
	if (client != nil) {
		[client setShouldSynchronize:_trickleSync withClientsOfType:ISyncClientTypeApplication];
		[client setShouldSynchronize:_trickleSync withClientsOfType:ISyncClientTypeServer];
	}	
	if (_trickleSync) {
		[client setSyncAlertHandler:self selector:@selector(_client:mightWantToSyncEntityNames:)];
		[self syncDocument:self]; // Comment out and replace with next line so can simultaneously initial sync with two apps
		//[self syncDocumentWithDelay:self];
	}
}

// Handling document changes
- (void) _handleSyncRequestNotification:(NSNotification*)notification {
	if (_trickleSync == YES) 
		[[NSRunLoop currentRunLoop] performSelector:@selector(syncDocumentWithDelay:) 
											 target:self argument:self order:1000 modes:[NSArray arrayWithObject:NSDefaultRunLoopMode]];
}

// Keep track if syncs fail and we reschedule trickles so we don't
// keep doing this forever and ever....
static int sRescheduleTrickleSyncCount = 0;
- (void) _syncDocumentOnOneShotTimer:(NSTimer*)timer
{
	if ([self _syncDocumentWithSyncType:AnySyncType] == NO) {
		if (sRescheduleTrickleSyncCount++ < 10) {
			NSLog(@"Could not trickle sync after %d %s, rescheduling timer...", sRescheduleTrickleSyncCount, (sRescheduleTrickleSyncCount == 1) ? "try" : "tries");
			[self syncDocumentWithDelay:self];
		}
		else {
			NSLog(@"Could not trickle sync after %d tries, giving up", sRescheduleTrickleSyncCount);
			sRescheduleTrickleSyncCount = 0;
		}
	}
	else {
		sRescheduleTrickleSyncCount = 0;
	}
}

- (void)_syncDocumentOnTimer:(NSTimer*)timer
{
	[self syncDocument:self];
}

- (void)_client:(ISyncClient *)client mightWantToSyncEntityNames:(NSArray *)entityNames
{
	[self syncDocument:self];
}


// Sync Action Methods

// Trickle sync action method, turns it on and off
- (IBAction)setTrickleSync:(id)sender {
	BOOL trickleSync = [sender intValue];
	[self _setTrickleSync:trickleSync];
	[[NSUserDefaults standardUserDefaults] setBool:trickleSync forKey:kDefaultsTrickleSync];
}

- (IBAction)syncDocument:(id)sender {
	NSLog(@"\n\nBEGIN SYNCING...");
	[self _syncDocumentWithSyncType:AnySyncType];
	NSLog(@"\nEND SYNCING\n\n");
}

- (IBAction)syncDocumentWithDelay:(id)sender {
	if (_oneShotTimer == nil) {
		_oneShotTimer = [[NSTimer scheduledTimerWithTimeInterval:5 target:self selector:@selector(_syncDocumentOnOneShotTimer:) userInfo:self repeats:NO] retain];
	}
	else {
		[_oneShotTimer setFireDate:[NSDate dateWithTimeIntervalSinceNow:5]];
	}
}

// Same as syncDocument: but requests a slow sync. Use this method if the client can't tell the sync engine
// which records have been added, modified, or deleted.
- (IBAction)slowSyncDocument:(id)sender {
	NSLog(@"\n\nBEGIN SLOW SYNCING...");
	[self _syncDocumentWithSyncType:SlowSyncType];
	NSLog(@"\nEND SLOW SYNCING\n\n");
}

// Same as syncDocument: but requests a refresh sync mode. Use this method if the client doesn't have a local database
// or simply wants to revert its document to the state of truth.
- (IBAction)refreshSync:(id)sender {
	// Should open panel to warn user that all the records will be deleted by the action	
	NSAlert *alert = [[NSAlert alloc] init];	
	[alert addButtonWithTitle:@"OK"];	
	[alert addButtonWithTitle:@"Cancel"];	
	[alert setMessageText:@"Replace your document with the objects in the sync engine?"];	
	[alert setInformativeText:@"Pulling the truth can not be undone (yet)."];
	[alert setAlertStyle:NSWarningAlertStyle];
	if ([alert runModal] == NSAlertFirstButtonReturn){
		NSLog(@"\n\nBEGIN RESET SYNCING...");
		[self _syncDocumentWithSyncType:RefreshSyncType];
		NSLog(@"\nEND RESET SYNCING\n\n");	
	}
	[alert release];
	
	return;
}


// Core Syncing Methods

// The primary sync method that registers the client, creates a session, and syncs the doc. 
// Use the syncType argument to request a sync mode. Pass AnySyncType if you want to let the 
// sync engine decide to fast or slow sync the document.
- (BOOL) _syncDocumentWithSyncType:(SyncType)syncType {
	// If our data file doesn't exist, do a refresh sync so we get data saved in the truth
	if (ForceRefreshSync == YES) {
		syncType = RefreshSyncType;
	}
	// If we had failed before, or syncing was disabled, make sure we do the
	// most conservative sync operation here.
	if (sSavedSyncType > syncType) {
		NSLog(@"Rock, paper, scissors, stone - saved sync type %d overrides requested sync type %d", sSavedSyncType, syncType);
		syncType = sSavedSyncType;
	}
	else {
		sSavedSyncType = syncType; // In case we fail
	}
	if ([[ISyncManager sharedManager] isEnabled] == NO) {
		NSLog(@"Skipping save and sync as syncing is disabled");
		if (syncType > sSavedSyncType) {
			NSLog(@"Resetting sSavedSyncType to %d (was %d)", syncType, sSavedSyncType);
			sSavedSyncType = syncType;
		}
		return YES; // Return YES to satisfy everyone
	}
	BOOL syncSuccess = NO;
	BOOL mustSlowSync = (_fastSync == NO) || (syncType == SlowSyncType);

	NSProgressIndicator* progressIndicator = [self progressIndicator];
	[progressIndicator setUsesThreadedAnimation:YES];
	[progressIndicator setHidden:NO];
	[progressIndicator startAnimation:self];
	[progressIndicator display];
	
	if (_oneShotTimer) {
		[_oneShotTimer invalidate];
		[_oneShotTimer release];
		_oneShotTimer = nil;
	}
	NS_DURING
		// Save the data source in case an error occurs and you need to revert
		NSLog(@"Saving before sync into %@", [self backupFileName]);
		_fastSync = NO; // Force slow sync on next sync unless we succeed below
		[self writeToFile:[self backupFileName] ofType:EntityDocumentType];	 
				
		// Register the client if it's not already registered
		ISyncClient *client = [self getSyncClient];
		
		// Create an ISyncSession for this sync
		ISyncSession *session;
		session = [ISyncSession beginSessionWithClient:client entityNames:
			[[_myDataSource entityModel] entityNames] beforeDate:[NSDate dateWithTimeIntervalSinceNow:60]];
		if (session == nil) {
			NSLog(@"Timed out trying to create a Sync Session");
		}
		else {
			// If successful, negotiate a sync mode
			if (syncType == RefreshSyncType) {
				NSLog(@"Requesting reset sync from engine for entity names %@", [[_myDataSource entityModel] entityNames]);
				[session clientDidResetEntityNames:[[_myDataSource entityModel] entityNames]];
			}
			// Perform the actual pushing and pulling of records
			syncSuccess = [self _syncDocumentWithClient:client andSession:session mustSlowSync:mustSlowSync];
			
			// If successful, save the data source after syncing
			if (syncSuccess && ([session isCancelled] == NO)) {
				NSLog(@"Saving after sync into %@", [self backupFileName]);
				[self writeToFile:[self backupFileName] ofType:EntityDocumentType];
				[session clientCommittedAcceptedChanges]; // Commit all accepted changes
				ForceRefreshSync = NO; // Don't force a refresh again
                sSavedSyncType = NoSyncType; // Reset with success
			}
			else {
				NSLog(@"Error occurred syncing client, not saving or committing changes");
			}
			if ([session isCancelled] == NO) [session finishSyncing];
		}
	NS_HANDLER
		NSLog(@"Exception occured while syncing: %@", localException);
	NS_ENDHANDLER
	
	[progressIndicator stopAnimation:self];
	[progressIndicator setHidden:YES];
	return syncSuccess;
}

// Core method that actually pushes and pulls records.
- (BOOL)_syncDocumentWithClient:(ISyncClient *)client andSession:(ISyncSession *)session mustSlowSync:(BOOL)mustSlowSync
{
	RecordTransformer *transformer;
	BOOL pushedChanges = NO;
	BOOL pulledChanges = NO;
	
	if (session == nil) return NO;
	
	// Create the record transform that converts entity objects to sync records and back.
	transformer = [[RecordTransformer alloc] init];
	[transformer setDataSource:_myDataSource];

	// Push and pull records
	[_myDataSource beginBatching];
	NS_DURING
		pushedChanges = [self _pushChangesToSyncSession:session usingDataSource:_myDataSource andRecordTransformer:transformer mustSlowSync:mustSlowSync];
		pulledChanges = [self _pullChangesFromSyncSession:session usingDataSource:_myDataSource andRecordTransformer:transformer];
		_fastSync = YES; // We can fast sync next time since we did successful sync here
	NS_HANDLER
        NSLog(@"Exception syncing: %@, reason %@", [localException name], [localException reason]);
	NS_ENDHANDLER
	
	// If successful, clear all the change properties in the local entity objects and delete any records.
	if (pushedChanges == YES) { 
		[_myDataSource clearAllChanges];
	}
	[_myDataSource endBatching];
	[transformer release];
	return pulledChanges;
}

// Pushes changes made to dataSource to session using  recordTransformer to transform KVC-compliant objects to sync records.
- (BOOL)_pushChangesToSyncSession:(ISyncSession *)session usingDataSource:(id)dataSource andRecordTransformer:(id)recordTransformer mustSlowSync:(BOOL)mustSlowSync
{
	id entityName;
    NSEnumerator *entityEnumerator;
    NSMutableDictionary *entitiesToPush = [NSMutableDictionary dictionary];
	// Checks if the sync engine wants to do a fast sync and denotes only the changed records, otherwise slow syncs.
	// Also handles a refresh sync mode if specified to replace all client records.
    NSLog(@"CHECKING changes to push to Sync Services session");
	entityEnumerator = [[dataSource entityNames] objectEnumerator];
	while (entityName = [entityEnumerator nextObject]) {
        NSArray *records;
		BOOL serverWantsAllRecordsForEntityName = [session shouldPushAllRecordsForEntityName:entityName];
		// Get the records that have changed since the last sync (or all of the records, if slow syncing). Sets
		// the changedRecords to nil if this is a refresh sync mode.
        
		if (![session shouldPushChangesForEntityName:entityName]) {
			NSLog(@"Server does not want us to push changes for entityName %@...", entityName);
        }
		// Should you push all records for this entity?
        else if (mustSlowSync || serverWantsAllRecordsForEntityName) {
			// Push all records (slow sync)
			NSLog(@"Slow syncing entityName %@...", entityName);
			if (serverWantsAllRecordsForEntityName == NO) {
				// Let server know that WE want to push all records (slow sync)
				NSLog(@"Letting the server know our honorable intention to push all records for entityName %@...", entityName);
				[session clientWantsToPushAllRecordsForEntityNames:[NSArray arrayWithObject:entityName]];
			}
            records = [dataSource valueForKey:[dataSource keyForEntityName:entityName]];
            if (records != nil) [entitiesToPush setObject:records forKey:entityName];
        }
		else {
			// Otherwise, push changes only (fast sync)
			NSLog(@"Fast syncing entityName %@...", entityName);
            records = [dataSource changedRecordsForEntityName:entityName];
            // If nothing changed, but some records deleted, make sure we won't skip below
            if ((records == nil) && [dataSource deletedRecordsForEntityName:entityName]) records = [NSArray array];
            if (records != nil) [entitiesToPush setObject:records forKey:entityName];
        }
    }
    // Now we actually push changes for any records that are changed and that the engine allows us to push
    NSLog(@"PUSHING changes to Sync Services session for entities %@", [entitiesToPush allKeys]);
   	entityEnumerator = [entitiesToPush keyEnumerator];
	while (entityName = [entityEnumerator nextObject]) { 
        NSArray *pushRecords = [entitiesToPush objectForKey:entityName];
        //NSLog(@"Pushing changedRecords=%@", [[pushRecords valueForKey:@"primaryKey"] description]);
        
        // Transform your entity object into a sync record and push the whole record that changed.
        // (Alternatively, you can push just the properties that changed but then you need to record them.)
        unsigned ii, count = [pushRecords count];
        for (ii = 0; ii < count; ii++) {
            id record = [pushRecords objectAtIndex:ii];
            [recordTransformer setEntityName:entityName];
            id dict = [recordTransformer transformedValue:record];
            NSLog(@"pushing sync record %@:\n%@", [record valueForKey:@"primaryKey"], [dict description]);
            [session pushChangesFromRecord:dict withIdentifier:[record valueForKey:@"primaryKey"]];
        }
        // Push deleted records if fast syncing only.
        if ([session shouldPushAllRecordsForEntityName:entityName] == NO) {
            NSEnumerator *keys;
            NSString *primaryKey;
            NSDictionary *deletedRecords = [dataSource deletedRecordsForEntityName:entityName];
            if (deletedRecords) NSLog(@"Deleting deletedRecords with primaryKeys=%@", [deletedRecords allKeys]);			
            keys = [deletedRecords keyEnumerator];
            while (primaryKey = [keys nextObject]) {
                NSLog(@"Deleting record with primary key %@", primaryKey);
                [session deleteRecordWithIdentifier:primaryKey];
            }
        }
    }
    return YES;
}

// Pulls changes from session and applies them to dataSource. Uses recordTransformer to transform sync records to entity objects.
- (BOOL)_pullChangesFromSyncSession:(ISyncSession *)session usingDataSource:(id)dataSource andRecordTransformer:(id)recordTransformer
{
	NSEnumerator *entityEnumerator;
	id entityName;
	BOOL rc = NO;
    
	NSLog(@"PULLING changes from Sync Services session");
	
	// Filter the entity names based on what the sync engine thinks should be pulled
	entityEnumerator = [[[dataSource entityModel] entityNames] objectEnumerator];
	NSMutableArray *filteredEntityNames = [NSMutableArray array];
	while (entityName = [entityEnumerator nextObject]){
		if ([session shouldPullChangesForEntityName:entityName])
			[filteredEntityNames addObject:entityName];
	}

	// Prepare the sync engine for pulling records--enters the session mingling state.
    NSLog(@"invoking prepareToPullChangesForEntityNames: with entityNames=%@", [filteredEntityNames description]);
	if ([session prepareToPullChangesForEntityNames:filteredEntityNames beforeDate:[NSDate dateWithTimeIntervalSinceNow:60]]) {
		// For each entity, check to see if there are changes to pull.
		entityEnumerator = [filteredEntityNames objectEnumerator];
        while (entityName = [entityEnumerator nextObject]){
			// Should you replace all records? For example, if pulling the truth or refresh syncing.
			if ([session shouldReplaceAllRecordsOnClientForEntityName:entityName]) {
				// Remove the records from the tables
				[[dataSource mutableArrayValueForKey:[dataSource keyForEntityName:entityName]] removeAllObjects];
			}
			
			// Now apply all the pulled changes for this entity.
			NSEnumerator *enumerator = [session changeEnumeratorForEntityNames:[NSArray arrayWithObject:entityName]];
			ISyncChange *change; 
			while (change = [enumerator nextObject]) {
				BOOL success = NO;
				NSString *recordIdentifier = [change recordIdentifier];
				switch ([change type])
				{
					// Add and modfies are treated the same in this example.
					case ISyncChangeTypeAdd:
					case ISyncChangeTypeModify:
						// Use the supplied transformer to apply the change to the entity object.
						[recordTransformer setEntityName:entityName];
						id myRecord = [recordTransformer reverseTransformedValueWithChange:change];
						success = (myRecord == nil) ? NO : YES;
						if (success == YES) {
							// Accept the change.
							[session clientAcceptedChangesForRecordWithIdentifier:recordIdentifier formattedRecord:nil 
															  newRecordIdentifier:nil];
						} else
							NSLog(@"FAILED to add or modify recordIdentifier=%@", recordIdentifier);
						break;
					case ISyncChangeTypeDelete:
						// Delete the record from the local data source.
						NSLog(@"Deleting recordIdentifier=%@", recordIdentifier);
						success = [dataSource deleteRecordWithPrimaryKey:recordIdentifier forEntityName:entityName];
						if (success == YES)
							// Accept the change.
							[session clientAcceptedChangesForRecordWithIdentifier:recordIdentifier formattedRecord:nil 
															  newRecordIdentifier:nil];
						else
							NSLog(@"FAILED to delete recordIdentifier=%@", recordIdentifier);
						
						break;
				}
			}
        }
        rc = YES;
    }
    else {
        NSLog(@"Couldn't wait for the mingle. I have important things to do! We'll get changes next time.");
        rc = NO;
    }
    return rc;
}


// Methods intended to be overriden by subclasses

- (NSString *)clientIdentifier
{
	// NOTE: Should raise since MUST be subclassed, note in header
	return nil;
}

- (NSArray *)clientReadOnlyEntityNames
{
	return nil;
}

- (NSArray *)clientWriteOnlyEntityNames
{
	return nil;
}

// Subclass overrides these accessors for UI
- (NSProgressIndicator*) progressIndicator {
	return nil;
}

- (NSButton*) trickleButton {
	return nil;
}
@end
