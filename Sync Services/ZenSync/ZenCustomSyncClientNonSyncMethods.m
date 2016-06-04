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
//  ZenCustomSyncClientNonSyncMethods.m
//  ZenSync
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import "ZenCustomSyncClient.h"
#import "strings.h"
#import "ZenSchemaStrings.h"
#import <SyncServices/SyncServices.h>
// All this overhead just to play with images
#import <AppKit/AppKit.h>

@implementation ZenCustomSyncClient (NonSyncMethods)

// Given an array of changes expected in the changes parameter of an ISyncChange, make a new dictionary,
// possibly using the values of an original dictionary as starting points (modified values will overwrite them)
- (NSMutableDictionary *)_mutDictFromChanges:(NSArray *)changes startingDict:(NSMutableDictionary *)startingDict {
	int i;
	NSMutableDictionary *retDict = [[NSMutableDictionary alloc] init];
	
	if (startingDict) {
		// Use the original values as a baseline
		[retDict setDictionary:startingDict];
	}
	
	// overwrite any baseline values, or fill in a blank dictionary with our new values
	for (i = 0; i < [changes count]; i++) {
		NSDictionary *curChange = [changes objectAtIndex:i];
		[retDict setObject:[curChange objectForKey:ISyncChangePropertyValueKey]
					forKey:[curChange objectForKey:ISyncChangePropertyNameKey]];
	}
	return retDict;
}

// Returns whether or not is was an acceptable parsing
- (BOOL)_readDataFromFile {
	NSString *fileData = [NSString stringWithContentsOfFile:_fileLoc];
	NSArray *lines = [fileData componentsSeparatedByString:@"\n"];
	NSArray *valuesArray = NULL;
	NSMutableDictionary *newDict = NULL;
	NSMutableArray *changeLines = [[NSMutableArray alloc] init];
	
	int i, j, numAdds = 0;
	
	// Loop through lines, but save the change lines to be parsed after everything else
	// (so we can check against records)
	for (i = 0; i < [lines count]; i++) {
		NSString *curLine = [lines objectAtIndex:i];
		
		if (([curLine length] == 0) || ([curLine characterAtIndex:0] == '#'))
			continue; // skip all comments and blank lines
		
		if ([curLine hasPrefix:@"T: "]) {
			if (_lastSyncNumber != -1)
				return NO;
			_lastSyncNumber = [[curLine substringFromIndex:3] intValue];
		} else if ([curLine hasPrefix:@"R: "]) {
			valuesArray = [[curLine substringFromIndex:3] componentsSeparatedByString:@","];
			newDict = [[NSMutableDictionary alloc] init];
			
			for (j = 1; j < [_fields count]; j++) {
				[newDict setObject:[valuesArray objectAtIndex:j] forKey:[_fields objectAtIndex:j]];
			}
			
			int recIdAsInt = [(NSString *)[valuesArray objectAtIndex:0] intValue];
			
			_highestLocalId = (_highestLocalId > recIdAsInt) ? _highestLocalId : recIdAsInt;
			
			[_records setObject:newDict forKey:[valuesArray objectAtIndex:0]];
			[newDict release];

		// Store the changes for later parsing
		} else if ([curLine hasPrefix:@"C: "]) {
			[changeLines addObject:curLine];
		} else
			return NO;
	}
	
	// Parse the change lines
	for (i = 0; i < [changeLines count]; i++) {
		NSString *curLine = [changeLines objectAtIndex:i];
		
		if (!_fields)
			return NO;
		if (!_changes)
			_changes = [[NSMutableDictionary alloc] init];
		
		if ([curLine length] == 3)
			continue; // Empty Change Line
		
		// Translate change lines into SyncServer friendly dictionaries
		NSString *curRecId;
		NSMutableArray *changeLine = [[NSMutableArray alloc] init];
		[changeLine setArray:[[curLine substringFromIndex:3] componentsSeparatedByString:@" "]];
		
		NSString *directive = [changeLine objectAtIndex:0];
		NSMutableDictionary *changeRecord = [[NSMutableDictionary alloc] init];
		
		[changeLine removeObjectAtIndex:0];
		
		if ( [directive isEqualToString:@"add"] ) {
			NSString *args = [changeLine componentsJoinedByString:@" "];
			valuesArray = [args componentsSeparatedByString:@","];
			NSMutableArray *changeArray = [[NSMutableArray alloc] init];
			
			for (j = 1; j < [_fields count]; j++) {
				NSMutableDictionary *curChange = [[NSMutableDictionary alloc] init];
				
				[curChange setObject:ISyncChangePropertySet forKey:ISyncChangePropertyActionKey];
				[curChange setObject:[_fields objectAtIndex:j] forKey:ISyncChangePropertyNameKey];
				[curChange setObject:[valuesArray objectAtIndex:j - 1] forKey:ISyncChangePropertyValueKey];
				
				[changeArray addObject:curChange];
			}
			
			NSMutableDictionary *entityNameDict = [[NSMutableDictionary alloc] init];
			[entityNameDict setObject:ISyncChangePropertySet forKey:ISyncChangePropertyActionKey];
			[entityNameDict setObject:ISyncRecordEntityNameKey forKey:ISyncChangePropertyNameKey];
			[entityNameDict setObject:ISyncEntity_Com_MyCompany_SyncExamples_Zensync_Customentity
							   forKey:ISyncChangePropertyValueKey];
			[changeArray addObject:entityNameDict];
			
			// Because we haven't talked to the sync server yet, we don't know what the stored highest id is.
			// Therefore, we place in a marker, which we will overwrite when we sync
			curRecId = [NSString stringWithFormat:@"Add%d", numAdds++];
			[changeRecord setObject:changeArray forKey:@"ChangeSet"];
			[changeRecord setObject:@"Add" forKey:@"ChangeType"];
		} else if ( [directive isEqualToString:@"delete"] ) {
			curRecId = [changeLine componentsJoinedByString:@" "];
			[changeRecord setObject:@"Delete" forKey:@"ChangeType"];
		} else if ( [directive isEqualToString:@"modify"] ) {
			curRecId = [changeLine objectAtIndex:0];
			[changeLine removeObjectAtIndex:0];
			NSString *args = [changeLine componentsJoinedByString:@" "];
			valuesArray = [args componentsSeparatedByString:@","];
			NSMutableArray *changeArray = [[NSMutableArray alloc] init];
			
			for (j = 0; j < [valuesArray count]; j+=2) {
				NSMutableDictionary *curChange = [[NSMutableDictionary alloc] init];
				
				[curChange setObject:ISyncChangePropertySet forKey:ISyncChangePropertyActionKey];
				[curChange setObject:[valuesArray objectAtIndex:j] forKey:ISyncChangePropertyNameKey];
				[curChange setObject:[valuesArray objectAtIndex:j + 1] forKey:ISyncChangePropertyValueKey];
				
				[changeArray addObject:curChange];
			}
			
			[changeRecord setObject:@"Modify" forKey:@"ChangeType"];
			[changeRecord setObject:changeArray forKey:@"ChangeSet"];
		} else {
			NSMutableDictionary *changeDict = [[NSMutableDictionary alloc] init];
			valuesArray = [[changeLine componentsJoinedByString:@" "] componentsSeparatedByString:@","];
			
			for ( j = 1; j < [_fields count]; j++) {
				[changeDict setObject:[valuesArray objectAtIndex:j - 1] forKey:[_fields objectAtIndex:j]];
			}
			
			[changeDict setObject:ISyncEntity_Com_MyCompany_SyncExamples_Zensync_Customentity
						   forKey:ISyncRecordEntityNameKey];
			
			[changeRecord setObject:changeDict forKey:@"ChangeSet"];
			curRecId = directive;
		}
		if ([_changes objectForKey:curRecId])
			return NO; // enforcing 1 change per record per session
		if ((![[changeRecord objectForKey:@"ChangeType"] isEqualToString:@"Add"]) && (![_records objectForKey:curRecId]))
			return NO; // Ensure that deletes and modifies represent a real record
		[_changes setObject:changeRecord forKey:curRecId];
	}
	
	// If we got here, it was a good parsing
	return YES;
}

// Write out our records (possibly changed by a sync) to the file we started with
- (void)writeNewData {
	int i;
	NSEnumerator *recordsEnumerator = [_records keyEnumerator];
	NSString *recordId;
	NSMutableString *writeString;
	
	writeString = [NSMutableString stringWithFormat:@"%@\n%@%d\n%@\n",
		DEFAULT_CUSTOM_COMMENT_LINE, DEFAULT_CUSTOM_TAG_LINE, _lastSyncNumber, DEFAULT_CUSTOM_CHANGES];
	
	while (( recordId = [recordsEnumerator nextObject] )) {
		NSDictionary *recordValues = [_records objectForKey:recordId];
		
		[writeString appendString:[NSString stringWithFormat:@"R: %@,", recordId]];
		
		for ( i = 1; i < [_fields count]; i++ ) {
			NSString *curValue = [recordValues objectForKey:[_fields objectAtIndex:i]];
			
			[writeString appendString:curValue ? curValue : @""];
			[writeString appendString:(i == [_fields count] - 1) ? @"\n" : @","];
		}
	}

	[writeString writeToFile:_fileLoc atomically:YES];
}

- (NSString *)dumpIcon {
	// If the file name you're using isn't Custom[somenumber].txt,
	// you don't get a neatorific icon
	int offset, clientNum = -1;
	NSString *fileName = [_fileLoc lastPathComponent];
	
	if ([fileName hasPrefix:@"Custom"]) {
		// Since intValue returns 0 on failure, let's make sure the first digit really is a number
		NSString *possibleNum = [fileName substringWithRange:NSMakeRange(6, [fileName length] - 10)];
		char firstPossibleChar = [possibleNum characterAtIndex:0];
		if (firstPossibleChar >= '0' && firstPossibleChar <= '9')
			clientNum = [possibleNum intValue];
	}
	
	// Why is this line here, you may ask.  The answer is that we need to get the system to accept that
	// this tool really wants to create an offscreen view to draw into, something you don't get or want
	// with a CLI tool
	[NSApplication sharedApplication];
	
	// Make sure the pre-badged icon exists
	if (![[NSFileManager defaultManager] fileExistsAtPath:[DEFAULT_CLIENT_IMAGE stringByExpandingTildeInPath]]) {
		NSLog(@"The icon %@ doesn't exist.  Please rebuild this tool", [DEFAULT_CLIENT_IMAGE stringByExpandingTildeInPath]);
		return nil;
	}
	
	// Grab our pre-badged icon from disk
	NSImage *clientImage = [[NSImage alloc] initByReferencingFile:
		[DEFAULT_CLIENT_IMAGE stringByExpandingTildeInPath]];
	// We know where our red badge is, so let's create an inscribed square to draw into.
	NSRect drawRect = NSMakeRect(79.0, 9.0, 40.0, 40.0);

	NSString *clientAbbr;
	if (clientNum == -1)
		clientAbbr = @"C";
	else
		clientAbbr = [NSString stringWithFormat:@"C%d", clientNum];
	
	// Try to fill this square height-wise, with the string we want
	NSAttributedString *attrString = [[NSAttributedString alloc] initWithString:clientAbbr
																	 attributes:
		[NSDictionary dictionaryWithObjectsAndKeys:
			[NSColor whiteColor],
			NSForegroundColorAttributeName,
			[NSFont boldSystemFontOfSize:drawRect.size.height],
			NSFontAttributeName, nil]];
	NSSize stringSize = [attrString size];
	
	// If we made it too high, so that we're too wide to fit in the square, scale everything down
	if (stringSize.width > drawRect.size.width) {
		[attrString release];
		// This new string should be exactly as wide as our square
		attrString = [[NSAttributedString alloc] initWithString:clientAbbr
													 attributes:
			[NSDictionary dictionaryWithObjectsAndKeys:
				[NSColor whiteColor],
				NSForegroundColorAttributeName,
				[NSFont boldSystemFontOfSize:(drawRect.size.width / stringSize.width) * drawRect.size.height],
				NSFontAttributeName, nil]];
		// Shrink the square (now technically a rect) heightwise to bound the string
		offset = (drawRect.size.height - [attrString size].height) / 2;
		drawRect.origin.y += offset;
		drawRect.size.height = [attrString size].height;
		// This shouldn't be necessary, but it can't hurt
		offset = (drawRect.size.width - [attrString size].width) / 2;
		drawRect.origin.x += offset;
		drawRect.size.width = [attrString size].width;
	} else {
		// If we get here, that means it was okay to set the height of our string to the height of the
		// draw rect.  Now, we shrink the rect widthwise to bound the string
		offset = (drawRect.size.width - stringSize.width) / 2;
		drawRect.origin.x += offset;
		drawRect.size.width = stringSize.width;
		// This shouldn't be necessary, but it can't hurt
		offset = (drawRect.size.height - stringSize.height) / 2;
		drawRect.origin.y += offset;
		drawRect.size.height = stringSize.height;
	}
	
	// Draw the string onto our image
	[clientImage lockFocus];
	[attrString drawInRect:drawRect];
	[clientImage unlockFocus];

	// And save the resulting badged icon to disk
	NSString *imagePath = [[NSString stringWithFormat:@"%@/%@.png", DEFAULT_WORKING_DIR, fileName]
		stringByExpandingTildeInPath];
	NSBitmapImageRep *imageRep = [NSBitmapImageRep imageRepWithData:[clientImage TIFFRepresentation]];
	[[imageRep representationUsingType:NSPNGFileType properties:nil] writeToFile:imagePath atomically:YES];

	return imagePath;
}

// Given a base plist describing this client, create a custom one that has a unique display name.
// This display name incorporates the name of the data file we're using
- (NSString *)createCustomPlist:(NSString *)basePlist {
	NSString *newDisplayName = [[_fileLoc lastPathComponent] stringByDeletingPathExtension];	
	NSMutableString *newPlistLoc = [NSMutableString stringWithCapacity:128];
	[newPlistLoc setString:[basePlist stringByDeletingLastPathComponent]];
	[newPlistLoc appendString:@"/CustomClientDescription"];
	[newPlistLoc appendString:newDisplayName];
	[newPlistLoc appendString:@".plist"];

	if ([[NSFileManager defaultManager] fileExistsAtPath:newPlistLoc])
		return newPlistLoc;
	
	NSString *imagePath = [self dumpIcon];

	// If we fail creating the new image, continue the failure
	if (!imagePath)
		return nil;

	NSDictionary *basePlistDict = [NSDictionary dictionaryWithContentsOfFile:basePlist];
	NSMutableDictionary *basePlistMutDict = 
		[NSMutableDictionary dictionaryWithCapacity:[basePlistDict count]];
	
	[basePlistMutDict addEntriesFromDictionary:basePlistDict];
	
	[basePlistMutDict setObject:[@"Zen " stringByAppendingString:newDisplayName]
						 forKey:@"DisplayName"];
	[basePlistMutDict setObject:imagePath forKey:@"ImagePath"];
	
	
	[basePlistMutDict writeToFile:newPlistLoc atomically:YES];
	return newPlistLoc;
}


@end

