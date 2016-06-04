/*

File: MyDocument.m

Abstract: Implementation file for our NSPersistentDocument subclass

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

Copyright © 2004-2005 Apple Computer, Inc., All Rights Reserved

*/

#import "MyDocument.h"

@implementation MyDocument

- (id)initWithType:(NSString *)type error:(NSError **)error {
    
    self = [super initWithType:type error:error];
    if (self != nil) {
        // Insert an Occasion entity into the document
        // We only want one Occasion ever inserted into a document. initWithType:error: is only called once per document,
        // unlike init (which is called when opening saved documents as well as creating them for the first time). If we
        // created the Occasion in init, we'd have to check to see if there was already an Occasion (e.g. with a Fetch Request)
        // before inserting one. With initWithType:error: we do not need to check.
		NSManagedObjectContext *managedObjectContext = [self managedObjectContext];
        [NSEntityDescription insertNewObjectForEntityForName:@"Occasion" inManagedObjectContext:managedObjectContext];
        // To avoid undo registration for this insertion we removeAllActions on the undoManager. We first call processPendingChanges
        // on the managed object context to force the undo registration for this insertion, then call removeAllActions.
        [managedObjectContext processPendingChanges];
        [[managedObjectContext undoManager] removeAllActions];
        [self updateChangeCount:NSChangeCleared];
    }
    return self;
}

- (NSString *)windowNibName  {
    return @"MyDocument";
}

// Intercept validation errors with willPresentError: so that we can handle their display
- (NSError *)willPresentError:(NSError *)inError 
{
	// The error is a Core Data validation error if its domain is NSCocoaErrorDomain and it is between
	// the minimum and maximum for Core Data validation error codes.
	if ([[inError domain] isEqualToString:NSCocoaErrorDomain]) {
		int errorCode = [inError code];
		if ( errorCode >= NSValidationErrorMinimum && errorCode <= NSValidationErrorMaximum) {

			// If there are multiple validation errors, inError will be a NSValidationMultipleErrorsError
			// and all the validation errors will be in an array in the userInfo dictionary for key NSDetailedErrorsKey
			id detailedErrors = [[inError userInfo] objectForKey:NSDetailedErrorsKey];
			if (detailedErrors != nil) {
			
				// For this example we are only presenting the error messages for up to 3 validation errors at a time.
				// We are simply passing the NSLocalizedDescription for each error to the user, but one could instead
				// construct a customized, user-friendly error here. The error codes and userInfo dictionary
				// keys for validation errors are listed in <CoreData/CoreDataErrors.h>.
				
				unsigned numErrors = [detailedErrors count];							
				NSMutableString *errorString = [NSMutableString stringWithFormat:@"%u validation errors have occurred", numErrors];
				if (numErrors > 3)
					[errorString appendFormat:@".\nThe first 3 are:\n"];
				else
					[errorString appendFormat:@":\n"];
				
				unsigned i;
				for (i = 0; i < (numErrors > 3 ? 3 : numErrors); ++i) {
					[errorString appendFormat:@"%@\n", [[detailedErrors objectAtIndex:i] localizedDescription]];
				}
				
				NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithDictionary:[inError userInfo]];
				[userInfo setObject:errorString forKey:NSLocalizedDescriptionKey];
				
				return [NSError errorWithDomain:[inError domain] code:[inError code] userInfo:userInfo];
				
			} else {
				// As there is only one validation error, we are returning it verbatim to the user.
				return inError;
			}
		}
	}
	return inError;
}

@end
