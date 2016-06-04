/*

File: TilePuzzleAppDelegate.m

Abstract: Application delegate for TilePuzzle

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

#import "TilePuzzleAppDelegate.h"
#import "TilesView.h"

NSString * const kPuzzleImageName = @"TigerPuzzle.png";

@implementation TilePuzzleAppDelegate

- (id) init {
    self = [super init];
    if (self) {
        isShowingSolution = NO;
        
        NSString *imagePath = [[NSBundle mainBundle] pathForImageResource:kPuzzleImageName];
        if (imagePath == nil) {
            NSRunAlertPanel(@"Alert", [NSString stringWithFormat:@"Image file missing from main bundle: %@", kPuzzleImageName], @"Quit", nil, nil);
            [[NSApplication sharedApplication] terminate:self];
        }
        puzzleImage = [[NSImage alloc] initWithContentsOfFile:imagePath];
        if (puzzleImage == nil) {
            NSRunAlertPanel(@"Alert", [NSString stringWithFormat:@"Can't open image file: %@", kPuzzleImageName], @"Quit", nil, nil);
            [[NSApplication sharedApplication] terminate:self];
        }
    }
    return self;
}

// Returns the managed object model, constructing it from all the managed object model files
// (compiled from xcdatamodel files) if it has not yet been constructed.
- (NSManagedObjectModel *) managedObjectModel {
    if (managedObjectModel) return managedObjectModel;
	
	NSMutableSet *allBundles = [[NSMutableSet alloc] init];
	[allBundles addObject: [NSBundle mainBundle]];
	[allBundles addObjectsFromArray: [NSBundle allFrameworks]];
    
    managedObjectModel = [[NSManagedObjectModel mergedModelFromBundles: [allBundles allObjects]] retain];
    [allBundles release];
    
    return managedObjectModel;
}

// Select the application support folder where we store saved data
- (NSString *) applicationSupportFolder {
    NSString *applicationSupportFolder = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    if ( [paths count] == 0 ) {
        NSRunAlertPanel(@"Alert", @"Can't find application support folder", @"Quit", nil, nil);
        [[NSApplication sharedApplication] terminate:self];
    } else {
        applicationSupportFolder = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"TilePuzzle"];
    }
    return applicationSupportFolder;
}

// Returns the managed object context, creating it and the persistent store coordinator if it has not already been created
- (NSManagedObjectContext *) managedObjectContext {
    NSError *error;
    NSString *applicationSupportFolder = nil;
    NSURL *url;
    NSFileManager *fileManager;
    NSPersistentStoreCoordinator *coordinator;
    BOOL didCreateNewStoreFile = YES;
    
    if (managedObjectContext) {
        return managedObjectContext;
    }
    
    fileManager = [NSFileManager defaultManager];
    applicationSupportFolder = [self applicationSupportFolder];
    if ( ![fileManager fileExistsAtPath:applicationSupportFolder isDirectory:NULL] ) {
        [fileManager createDirectoryAtPath:applicationSupportFolder attributes:nil];
    }
    
    NSString *storeFilePath = [applicationSupportFolder stringByAppendingPathComponent: @"TilePuzzle.xml"];
    if ([fileManager fileExistsAtPath: storeFilePath]) {
        didCreateNewStoreFile = NO;
    }
    url = [NSURL fileURLWithPath: storeFilePath];
    coordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: [self managedObjectModel]];
    if ([coordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:url options:nil error:&error]){
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator: coordinator];
    } else {
        [[NSApplication sharedApplication] presentError:error];
    }    
    [coordinator release];
    
    // If we are creating a new store instead of loading a pre-existing one, we create the
    // managed objects represeting the tiles.
    if (didCreateNewStoreFile) {
        [self createTiles: managedObjectContext];
    }
    
    return managedObjectContext;
}

- (IBAction) saveAction:(id)sender {
    NSError *error = nil;
    if (![[self managedObjectContext] save:&error]) {
        [[NSApplication sharedApplication] presentError:error];
    }
}

- (NSUndoManager *) windowWillReturnUndoManager:(NSWindow *)sender {
	return [[self managedObjectContext] undoManager];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    NSError *error;
    NSManagedObjectContext *context;
    int reply = NSTerminateNow;
    
    context = [self managedObjectContext];
    if (context != nil) {
        if ([context commitEditing]) {
            if (![context save:&error]) {
				
                // In this example we are using the Core Data Application template's default error handling.
                // A real application should replace this behavior with something more robust and user-friendly.
                // Display two panels: one with the error, and one asking if the user wishes to quit without saving.
                BOOL errorResult = [[NSApplication sharedApplication] presentError:error];
				
				if (errorResult == YES) { // Then the error was handled
					reply = NSTerminateCancel;
				} else {
					
					// Fall back to displaying a "quit anyway" panel.
					int alertReturn = NSRunAlertPanel(nil, @"Could not save changes while quitting. Quit anyway?" , @"Quit anyway", @"Cancel", nil);
					if (alertReturn == NSAlertAlternateReturn) {
						reply = NSTerminateCancel;	
					}
				}
            }
        } else {
            reply = NSTerminateCancel;
        }
    }
    return reply;
}

// Add managed objects representing the puzzle tiles to the managed object context.
- (void) createTiles:(NSManagedObjectContext *)context {
	
	NSSize imageSize = [puzzleImage size];
	float pieceWidth = imageSize.width / kTileGridSize;
	float pieceHeight = imageSize.height / kTileGridSize;

    // Place the 15 image tiles in positions 0,0 - 3,2
    int tileX, tileY;
    int i = 1;
    for (tileX = 0; tileX < kTileGridSize; ++tileX) {
        for ( tileY = 0; tileY < kTileGridSize && i < kNumTiles; ++tileY && ++i) {
            // This rect represents the part of the puzzleImage that this tile will use when drawing this tile
			NSRect imageRect = NSMakeRect(tileX * pieceWidth, tileY * pieceHeight, pieceWidth, pieceHeight);
            
			NSManagedObject *newTile;
            newTile = [NSEntityDescription insertNewObjectForEntityForName:@"ImageTile" inManagedObjectContext:context];
			[newTile setValue:[NSNumber numberWithInt:tileX] forKey:@"xPosition"];
			[newTile setValue:[NSNumber numberWithInt:tileX] forKey:@"correctXPosition"];
			[newTile setValue:[NSNumber numberWithInt:tileY] forKey:@"yPosition"];
			[newTile setValue:[NSNumber numberWithInt:tileY] forKey:@"correctYPosition"];
            // We represent the tile's rect as a string using NSStringFromRect (and later NSRectFromString).
            [newTile setValue:NSStringFromRect(imageRect) forKey:@"imageRectString"];
        }
    }
    // Place the blank tile at 3,3. 
    NSManagedObject *blankTile;
    blankTile = [NSEntityDescription insertNewObjectForEntityForName:@"BlankTile" inManagedObjectContext:context];
    [blankTile setValue:[NSNumber numberWithInt:(kTileGridSize - 1)] forKey:@"xPosition"];
    [blankTile setValue:[NSNumber numberWithInt:(kTileGridSize - 1)] forKey:@"correctXPosition"];
    [blankTile setValue:[NSNumber numberWithInt:(kTileGridSize - 1)] forKey:@"yPosition"];
    [blankTile setValue:[NSNumber numberWithInt:(kTileGridSize - 1)] forKey:@"correctYPosition"];
    
    // To avoid undo registration for this setup we removeAllActions on the undoManager. We first call processPendingChanges
    // on the managed object context to force the undo registration for the inserted objects, then call removeAllActions.
    [context processPendingChanges];
    [[context undoManager] removeAllActions];
}

// Shuffle the puzzle piece positions
- (IBAction) shuffle:(id)sender {

    // Fetch the set of tiles
    NSFetchRequest *request = [[self managedObjectModel] fetchRequestTemplateForName:@"allTiles"];
    NSError *fetchError;
	NSArray *fetchedTiles = [[self managedObjectContext] executeFetchRequest:request error:&fetchError];
    if (fetchedTiles == nil) {
        [[NSApplication sharedApplication] presentError:fetchError];
        [[NSApplication sharedApplication] terminate:self];
    }
    NSAssert(([fetchedTiles count] == kNumTiles), ([NSString stringWithFormat:@"There should be %d tiles, but there are only %u", kNumTiles, [fetchedTiles count]]));
    
    // Shuffle the tiles by swapping each tile with another random tile
    [[[self managedObjectContext] undoManager] beginUndoGrouping];
	int i;
	for (i = 0; i < kNumTiles; ++i) {
        [self swapTile:[fetchedTiles objectAtIndex:i] withTile:[fetchedTiles objectAtIndex:(random() % kNumTiles)]];
    }
    [[[self managedObjectContext] undoManager] endUndoGrouping];
    
    [window display];
}

- (IBAction) showSolution:(id)sender {

    isShowingSolution = YES;
    [window display];
}

// Swap the position of the two given tiles
- (void) swapTile:(NSManagedObject *)firstTile withTile:(NSManagedObject *)secondTile {
    
    NSNumber *firstX = [firstTile valueForKey:@"xPosition"];
    NSNumber * firstY = [firstTile valueForKey:@"yPosition"];
    
    NSManagedObjectContext *context = [self managedObjectContext];
    NSAssert(context != nil, @"managedObjectContext is nil!.");
    
    [[context undoManager] beginUndoGrouping];
    [firstTile setValue:[secondTile valueForKey:@"xPosition"] forKey:@"xPosition"];
    [firstTile setValue:[secondTile valueForKey:@"yPosition"] forKey:@"yPosition"];
    [secondTile setValue:firstX forKey:@"xPosition"];
    [secondTile setValue:firstY forKey:@"yPosition"];
    [[context undoManager] endUndoGrouping];
}

- (void) dealloc {
    [puzzleImage release];
    [super dealloc];
}

@end
