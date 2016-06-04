/*

File: TilesView.m

Abstract: Custom NSView subclass for the collection of puzzle tiles

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/

#import "TilesView.h"
#import "TilePuzzleAppDelegate.h"

#define TILE_GRID_SIZE 4
// The number of tiles across or down in the Tileview
const int kTileGridSize = TILE_GRID_SIZE;
// The number of tiles in the TilesView
const int kNumTiles = TILE_GRID_SIZE * TILE_GRID_SIZE;
#undef TILE_GRID_SIZE

@implementation TilesView

- (void)awakeFromNib {
    [[NSNotificationCenter defaultCenter] addObserver:self
                                          selector:@selector(didUndo:)
                                          name:nil
                                          object:[[delegate managedObjectContext] undoManager]];
}

- (void)drawRect:(NSRect)rect {

    // Fill the view with a background color
    [[NSColor whiteColor] set];
    [NSBezierPath fillRect:rect];
	
    NSSize tileSize;
    tileSize.height = rect.size.height / kTileGridSize;
    tileSize.width  = rect.size.width / kTileGridSize;

    // Draw each tile in the appropriate rect
    NSPoint tileOrigin = rect.origin;
    int x, y;
    for (x = 0; x < kTileGridSize; ++x) {
        for (y = 0; y < kTileGridSize; ++y) {
            NSRect gridRect = NSMakeRect(tileOrigin.x, tileOrigin.y, tileSize.width, tileSize.height);
            NSRect tileRect = NSInsetRect(gridRect, 1, 1);
            [self drawTileAtX:x andY:y inRect:tileRect];
            
            tileOrigin.y += tileSize.height;
        }
        tileOrigin.x += tileSize.width;
        tileOrigin.y = rect.origin.y;
    }
}

- (void)drawTileAtX:(int)xPosition andY:(int)yPosition inRect:(NSRect)rect {

    NSManagedObject *fetchedTile;
    if ([[delegate valueForKey:@"isShowingSolution"] boolValue]) {
        fetchedTile = [self fetchTileAtCorrectX:xPosition andY:yPosition];
    } else {
        fetchedTile = [self fetchTileAtX:xPosition andY:yPosition];
    }

    NSAssert(fetchedTile != nil, ([NSString stringWithFormat:@"Tile fetch at %d, %d returned nil!", xPosition, yPosition]));
	
	if ([[[fetchedTile entity] name] isEqual:@"BlankTile"]) {
        // No drawing is needed for a blank tile
		return;
	}
    
    NSString *rectString = [fetchedTile valueForKey:@"imageRectString"];
    NSAssert(rectString != nil, @"[fetchedTile valueForKey:@\"imageRectString\"] returned nil!");
    NSRect imageRect = NSRectFromString(rectString);
    
    NSImage *puzzleImage = [delegate valueForKey:@"puzzleImage"];
    NSAssert(puzzleImage != nil, @"[delegate valueForKey:@\"puzzleImage\"] returned nil!");
    
    // We simply composite the appropriate piece of the source image (puzzleImage) onto the view.
    // The "appropriate piece" is determined by the rect stored in the tile.
    [puzzleImage compositeToPoint:rect.origin fromRect:imageRect operation:NSCompositeCopy];
}

- (id)fetchTileAtCorrectX:(int)xPosition andY:(int)yPosition {
    NSDictionary *substitutionVars = [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithInt:xPosition], @"x",
                                                                                 [NSNumber numberWithInt:yPosition], @"y", nil];
    NSFetchRequest *request = [[delegate managedObjectModel] fetchRequestFromTemplateWithName:@"tileAtCorrectXAndY"
                                                             substitutionVariables:substitutionVars];
    NSAssert(request != nil, @"fetchRequestFromTemplateWithName:\"tileAtCorrectXAndY\" returned nil!");
    
    return [self executeTileFetch:request];
}

- (id)fetchTileAtX:(int)xPosition andY:(int)yPosition {
	
    NSDictionary *substitutionVars = [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithInt:xPosition], @"x",
                                                                                 [NSNumber numberWithInt:yPosition], @"y", nil];
    NSFetchRequest *request = [[delegate managedObjectModel] fetchRequestFromTemplateWithName:@"tileAtXAndY"
                                                             substitutionVariables:substitutionVars];
    NSAssert(request != nil, @"fetchRequestFromTemplateWithName:\"tileAtXAndY\" returned nil!");
    
    return [self executeTileFetch:request];
}

- (id)executeTileFetch:(NSFetchRequest *)request {
    // Fetch the tile at xPosition, yPosition using an NSFetchRequest
    // Retrieve the fetch request from the model. This fetch request was defined using the XCode Data Modeler
    // and can be viewed in the "Fetch Requests" list for the "Tile" entity.
	
    NSManagedObjectContext *context = [delegate managedObjectContext];
    NSAssert(context != nil, @"The TilesView's delegate's managedObjectContext is nil!.");
    NSError *fetchError;
	NSArray *fetchedTiles = [context executeFetchRequest:request error:&fetchError];
    if (fetchedTiles == nil) {
        [[NSApplication sharedApplication] presentError:fetchError];
        [[NSApplication sharedApplication] terminate:self];
    }
    NSAssert([fetchedTiles count] == 1, ([NSString stringWithFormat:@"Tile fetch returned %d results!", [fetchedTiles count]]));
	return [fetchedTiles objectAtIndex:0];
}

// Return the blank tile
- (NSManagedObject *)blankTile
{   
    // This fetch request could easily be specified in the Managed Object Model using XCode's Data Modeller, but
    // we're constructing it programmatically here as an example
	NSFetchRequest *request = [[[NSFetchRequest alloc] init] autorelease];
	NSEntityDescription *entity = [[[delegate managedObjectModel] entitiesByName] objectForKey:@"BlankTile"];
	[request setEntity:entity];
	
    NSManagedObjectContext *context = [delegate managedObjectContext];
    NSAssert(context != nil, @"The TilesView's delegate's managedObjectContext is nil!.");
	NSArray *fetchedTiles = [context executeFetchRequest:request error:nil];
    NSAssert([fetchedTiles count] == 1, ([NSString stringWithFormat:@"There should have been 1 blank tile, but there were %d!",
        [fetchedTiles count]]));
    
    return [fetchedTiles objectAtIndex:0];
}

- (void)mouseDown:(NSEvent *)event {

    // If we're showing the solution, a mouse down switches back into puzzle mode
    if ([[delegate valueForKey:@"isShowingSolution"] boolValue]) {
        [delegate setValue:[NSNumber numberWithBool:NO] forKey:@"isShowingSolution"];
    } else {
        // Figure out which tile the user clicked on
        float viewWidth = NSWidth([self bounds]);
        float viewHeight = NSHeight([self bounds]);
        NSPoint locationInView = [event locationInWindow];
        locationInView = [self convertPoint:locationInView fromView:nil];
        
        int tileX = (int)(locationInView.x / viewWidth  * kTileGridSize);
        tileX = tileX > (kTileGridSize - 1) ? (kTileGridSize - 1) : tileX;
        int tileY = (int)(locationInView.y / viewHeight * kTileGridSize);
        tileY = tileY > (kTileGridSize - 1) ? (kTileGridSize - 1) : tileY;
        
        NSManagedObject *blankTile = [self blankTile];
        int blankX = [[blankTile valueForKey:@"xPosition"] intValue];
        int blankY = [[blankTile valueForKey:@"yPosition"] intValue];
        
        // Check to make sure the clicked tile is adjacent to the blank tile
        if (tileX == blankX) {
            int distance = (blankY - tileY);
            if (distance != -1 && distance != 1) {
                NSBeep();
                return;
            }
        } else if (tileY == blankY) {
            int distance = (blankX - tileX);
            if (distance != -1 && distance != 1) {
                NSBeep();
                return;
            }
        } else {
            NSBeep();
            return;
        }
        
        NSManagedObject *clickedTile = [self fetchTileAtX:tileX andY:tileY];
        [delegate swapTile:clickedTile withTile:blankTile];
    }
    
    // Refresh the view
    [self setNeedsDisplay:YES];
}

- (void)didUndo:(NSNotification *)notification {
	[self setNeedsDisplay:YES];
}

- (BOOL) isOpaque {
    return YES;
}

@end
