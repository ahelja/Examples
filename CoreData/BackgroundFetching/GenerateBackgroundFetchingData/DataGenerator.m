/*

File: DataGenerator.m

Abstract: Creates and populates a store for the BackgroundFetching example

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

#import "DataGenerator.h"
#import "SharedConstants.h"

@implementation DataGenerator

// Generates the Letter managed objects, inserts them into the context, and returns an NSDictionary
// mapping characters to the letter managed object.
- (NSDictionary *)generateLetters
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    NSMutableDictionary *letterObjects = [[[NSMutableDictionary alloc] init] autorelease];
    NSString *lettersAsString = @"abcdefghijklmnopqrstuvwxyz";
    NSRange letterIndex = {0, 1};
    for (letterIndex.location = 0; letterIndex.location < [lettersAsString length]; ++letterIndex.location) {
        NSString *character = [lettersAsString substringWithRange:letterIndex];
        NSManagedObject *newLetter = [NSEntityDescription insertNewObjectForEntityForName:@"Letter" inManagedObjectContext:context];
        [newLetter setValue:character forKey:@"character"];
        
        // Add the Letter to our dictionary for quick access when generating Words
        [letterObjects setValue:newLetter forKey:character];
    }
    
    return letterObjects;
}

// Generates the Word managed objects and inserts them into the context
// Also sets the relationship between a Word and it's firstLetter using the supplied Letters dictionary
- (void)generateWordsWithFirstLetters:(NSDictionary *)letters
{
    NSManagedObjectContext *context = [self managedObjectContext];

    char *wordsFilePath = "/usr/share/dict/words";
    FILE *wordsFile = fopen(wordsFilePath, "r");
    if (wordsFile == NULL) {
        [NSException raise:@"IOException" format:@"Error opening words file: %s", strerror(errno)];
    }
    @try {
        const int kMaxWordLength = 1024;
        char wordBuffer[kMaxWordLength + 1];
        int numCharsScanned;
        // A format string for fscanf specifying that we want up to kMaxWordLength non-whitespace characters
        // This constructs the string @"%1024s"
        const char *fscanfFormatString = [[NSString stringWithFormat: @"%%%is", kMaxWordLength] cStringUsingEncoding:NSASCIIStringEncoding];
        int insertCount = 0;
        for (;;) {
            NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
            @try {
                // Read in another word from the words file
                numCharsScanned = fscanf(wordsFile, fscanfFormatString, wordBuffer);
                if (numCharsScanned >= kMaxWordLength) {
                    // If we get the maximum possible number of characters back, we don't know if we actually
                    // got the whole word; it could be even longer, so we throw.
                    [NSException raise:@"IOException" format:@"Word in words file exceeded word buffer size"];
                }
                else if (numCharsScanned == EOF || numCharsScanned <= 0) {
                    break;
                }
                
                // Create the managed object for this word
                NSManagedObject *newWord = [NSEntityDescription insertNewObjectForEntityForName:@"Word" inManagedObjectContext:context];
                NSString *wordText = [NSString stringWithCString:wordBuffer encoding:NSASCIIStringEncoding];
                [newWord setValue:wordText forKey:@"text"];
                [newWord setValue:[NSNumber numberWithInt:[wordText length]] forKey:@"length"];
                
                NSRange firstLetterRange = {0, 1};
                NSManagedObject *firstLetter = [letters valueForKey:[[wordText substringWithRange:firstLetterRange] lowercaseString]];
                if (nil == firstLetter) {
                    [NSException raise:@"InvalidDataException" format:@"Word from words file has unrecognized first letter: %@", [[wordText substringWithRange:firstLetterRange] lowercaseString]];
                }
                // As Word's firstLetter and Letter's words are inverse relationships, setting firstLetter will also automatically
                // add this Word to the Letter's words.
                [newWord setValue:firstLetter forKey:@"firstLetter"];
                
                if (insertCount >= 10000) {  // Save after every 1000 inserts
                    NSError *saveError;
                    if (![[self managedObjectContext] save:&saveError]) {
                        [NSException raise:@"SaveException" format:@"Error while saving: %@", ([saveError description] != nil) ? [saveError description] : @"Unknown Error"];
                    }
                    insertCount = 0;
                } else {
                    ++insertCount;
                }
            }
            @catch (id exception) {
                [exception retain];
                [pool release];
                [exception autorelease];
                @throw exception;
            }
            [pool release];
        }
    }
    @finally {
        fclose(wordsFile);
    }

}

// Populate the persistent store with Word managed objects generated from the system words file.
- (void)generateData
{
    [[self managedObjectContext] setUndoManager:nil];

    NSDictionary *letters = [self generateLetters];
    [self generateWordsWithFirstLetters:letters];
        
    NSError *saveError;
    if (![[self managedObjectContext] save:&saveError]) {
        [NSException raise:@"SaveException" format:@"Error while saving", ([saveError description] != nil) ? [saveError description] : @"Unknown Error"];
    }
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

- (NSString *)applicationSupportFolder {

    NSString *applicationSupportFolder = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    if ([paths count] == 0) {
        [NSException raise:@"ApplicationSupportNotFoundException" format:@"Can't find application support folder"];
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
    
    if (managedObjectContext) {
        return managedObjectContext;
    }
    
    fileManager = [NSFileManager defaultManager];
    applicationSupportFolder = [self applicationSupportFolder];
    if ( ![fileManager fileExistsAtPath:applicationSupportFolder isDirectory:NULL] ) {
        [fileManager createDirectoryAtPath:applicationSupportFolder attributes:nil];
    }
    
    NSString *storeFilePath = [applicationSupportFolder stringByAppendingPathComponent:kStoreFileName];
    if ([fileManager fileExistsAtPath: storeFilePath]) {
        // Delete the current store file if it exists
        if (NO == [fileManager removeFileAtPath:storeFilePath handler:nil]) {
            [NSException raise:@"IOException" format:@"Could not remove existing persistent store file."];
        }
    }
    url = [NSURL fileURLWithPath: storeFilePath];
    coordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: [self managedObjectModel]];
    if ([coordinator addPersistentStoreWithType:storeType() configuration:nil URL:url options:nil error:&error]){
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator: coordinator];
    } else {
        [[NSApplication sharedApplication] presentError:error];
    }    
    [coordinator release];
    
    return managedObjectContext;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    NSError *error;
    NSManagedObjectContext *context;
    int reply = NSTerminateNow;
    
    context = [self managedObjectContext];
    if (context != nil) {
        if ([context commitEditing]) {
            if (![context save:&error]) {
				
				// This default error handling implementation should be changed to make sure the error presented includes application specific error recovery. For now, simply display 2 panels.
                BOOL errorResult = [[NSApplication sharedApplication] presentError:error];
				
				if (errorResult == YES) { // Then the error was handled
					reply = NSTerminateCancel;
				} else {
					
					// Error handling wasn't implemented. Fall back to displaying a "quit anyway" panel.
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

@end
