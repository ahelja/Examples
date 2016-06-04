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
//  MediaDocument.m
//  MediaAssets
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import "MediaDocument.h"
#import <SyncServices/SyncServices.h>
#import <SyncExamples/EntityModel.h>
#import <SyncExamples/DataSource.h>
#import <SyncExamples/RelationshipTransformer.h>

@implementation MediaDocument

- (id)init
{
    self = [super init];
    if (self) {
		// Create relationship transformers
		RelationshipTransformer *transformer = [RelationshipTransformer transformerForDestinationEntity:@"com.mycompany.syncexamples.Event"
																 andDataSource:[self dataSource]];
		[transformer setToMany:NO];
		[NSValueTransformer setValueTransformer:transformer forName:@"EventRelationshipTransformer"];
    }
	
    return self;
}

- (NSString *)clientIdentifier
{
	return @"com.mycompany.syncexamples.media";
}

// Typical NSDocument Methods

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	
	// Create the sort descriptors needed for the events array controller
	if (eventController == nil)
		NSLog(@"eventController=nil");
	id titleDescriptor = [[[NSSortDescriptor alloc] initWithKey:@"title" ascending:YES] autorelease];
	id dateDescriptor = [[[NSSortDescriptor alloc] initWithKey:@"startDate" ascending:YES] autorelease];
	[eventController setSortDescriptors:[NSArray arrayWithObjects:titleDescriptor, dateDescriptor, nil]];
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MediaDocument";
}

- (NSString *)displayName {
	return @"Media Assets";
}


// Other Action Methods

// Used by smartEvents: to test if a media object has the same date as an event. Returns YES if the media's date
// matches the event, NO otherwise.
- (BOOL)_media:(id)media matchesEvent:(id)event
{
	NSCalendarDate *startDate = [event valueForKey:@"startDate"];
	NSCalendarDate *endDate = [event valueForKey:@"endDate"];
	NSCalendarDate *mediaDate = [media valueForKey:@"date"];
	
	//NSLog(@"event=%@ media=%@", [event valueForKey:@"title"], [media valueForKey:@"title"]);
	//NSLog(@"startDate=%@ endDate=%@ mediaDate=%@", [startDate description], [endDate description], [mediaDate description]);
	// Tricky to test because either or both start and end date can be nil or null.				
	if ((startDate == nil) && ![startDate isEqual:[NSNull null]])
		return NO;
	
	// If the endDate is nil or null then just compare with the start date
	if ((endDate == nil) || [endDate isEqual:[NSNull null]]){
		if ([startDate dayOfCommonEra] == [mediaDate dayOfCommonEra])
			return YES;
		else
			return NO;
	}
	
	// Otherwise, check if it's within the range of a multi-day event
	int startMinutes = [startDate dayOfCommonEra]*1440 + [startDate hourOfDay]*60 + [startDate minuteOfHour];
	int mediaMinutes = [mediaDate dayOfCommonEra]*1440 + [mediaDate hourOfDay]*60 + [mediaDate minuteOfHour];
	int endMinutes = [endDate dayOfCommonEra]*1440 + [endDate hourOfDay]*60 + [endDate minuteOfHour];
	if ((startMinutes == endMinutes) && (startMinutes == mediaMinutes)) // odd case that can happen
		return YES;
	if ((startMinutes <= mediaMinutes) && (mediaMinutes < (endMinutes-1)))  // covers an all-day event
		return YES;
	
	return NO;
}

// Matches all the Media objects, that have no assigned event, to the most logical event--the event with matching dates.
// Use this method to quickly set all the Media to-one relationships to Event, and inverse to-many relationships.
- (IBAction)smartEvents:(id)sender {
	// Might be more efficient if you sort the events and media by date first
	id eventRecords = [[[self dataSource] valueForKey:@"eventRecords"] sortedArrayUsingDescriptors:
		[NSArray arrayWithObject:[[[NSSortDescriptor alloc] initWithKey:@"startDate" ascending:YES] autorelease]]];
	id mediaRecords =  [[[self dataSource] valueForKey:@"mediaRecords"] sortedArrayUsingDescriptors:
		[NSArray arrayWithObject:[[[NSSortDescriptor alloc] initWithKey:@"date" ascending:YES] autorelease]]];
	id transformer = [NSValueTransformer valueTransformerForName:@"EventRelationshipTransformer"];
	
	NSEnumerator *mediaEnumerator = [mediaRecords objectEnumerator];
	id media;
	BOOL changed = NO;
	[_myDataSource beginBatching];
	while (media = [mediaEnumerator nextObject]){
		// Try to find a matching event for media objects that have no assigned event
		if ([transformer transformedValue:[media valueForKey:@"event"]] == nil){
			NSEnumerator *eventEnumerator = [eventRecords objectEnumerator];
			id event;
						
			// Find the first matching event
			for (event = [eventEnumerator nextObject]; (event != nil) && ![self _media:media matchesEvent:event]; ){
				event = [eventEnumerator nextObject];
			}
			if (event != nil){ // found matching event
				id syncValue = [transformer reverseTransformedValue:event];
				[media setValue:syncValue forKey:@"event"];
				changed = YES;
			}
		}
	}
	[_myDataSource endBatching];
	
	return;
}

// Imports a "year" folder in the iPhoto Library by creating a Media object for each photo.
// NOTE: this method currently doesn't work with album folders which contain a bunch of links.
// Also, the dates of the Media objects are derived from the year-month-day structure of the
// iPhoto "year" folder. If you import an album, your Media won't have any creation dates.
- (IBAction)importFiles:(id)sender
{	
    int result;
    //NSArray *fileTypes = [NSArray arrayWithObject:NSFileTypeDirectory];	
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];		

	[openPanel setCanChooseDirectories:YES];
	[openPanel setCanChooseFiles:NO];
    [openPanel setAllowsMultipleSelection:NO];  // can handle only one album at a time	
	NSString *libraryPath = [NSHomeDirectory() stringByAppendingPathComponent:@"Pictures/iPhoto Library"];	
    result = [openPanel runModalForDirectory:libraryPath file:nil types:nil];
	
    if (result == NSOKButton) {
        NSArray *filesToOpen = [openPanel filenames];

		// Assumes there is one folder
		if ([filesToOpen count] > 0){
			[_myDataSource beginBatching];
            NSString *file = [filesToOpen objectAtIndex:0];
			[self createEntitiesFromAlbumYear:file];
			[_myDataSource endBatching];
        }
    }
}

// Creates a Media object per photo in an iPhoto Library album (expected to be a "year" folder). 
// Attributes, title, imagePath and date, are set appropriately. Adds the media objects to the
// data source (uses KVO so the views will be updated automatically).
- (void)createEntitiesFromAlbumYear:(NSString *)yearPath
{
	NSLog(@"Importing media from yearPath=%@", yearPath);
	id media;
	NSString *libraryName;
	NSString *monthPath, *dayPath,*year, *month, *day, *image;
	NSCalendarDate *today = [NSCalendarDate calendarDate];
	NSURL *imageURL;
	BOOL isDirectory;
	int y;
	NSString *className = [[[[self dataSource] entityModel] entityWithEntityName:@"com.mycompany.syncexamples.Media"] className];

	// If the year can't be converted to a number then abort
	year = [yearPath lastPathComponent];
	y = [year intValue];
	if (y == 0) return; //abort
	
	// Otherwise create the path components
	libraryName = @"iPhoto Library";	
	//NSLog(@"yearPath=%@", yearPath);
	int m, d;
	
	for (m = 1; m < 13; m++){
		// Get the first day of this year
		month = [self integerToString:m];
		//NSLog(@"month=%@", month);
		
		monthPath = [NSString pathWithComponents:[NSArray arrayWithObjects: yearPath, month, nil]];
		//NSLog(@"monthPath=%@", monthPath);
		if ([[NSFileManager defaultManager]  fileExistsAtPath:monthPath isDirectory:&isDirectory] && isDirectory){
			for (d = 1; d < 32; d++){
				day = [self integerToString:d];
				//NSLog(@"day=%@", day);

				dayPath = [NSString pathWithComponents:[NSArray arrayWithObjects: monthPath, day, nil]];
			    //NSLog(@"dayPath=%@", dayPath);
				if ([[NSFileManager defaultManager]  fileExistsAtPath:dayPath isDirectory:&isDirectory] && isDirectory){
					NSArray *files = [[NSFileManager defaultManager] directoryContentsAtPath:dayPath];
					NSEnumerator *fileEnum = [files objectEnumerator];
					
					while (image = [fileEnum nextObject]){
						if ([[[image pathExtension] lowercaseString] isEqual:@"jpg"]){
							imageURL = [NSURL fileURLWithPath:
								[NSString pathWithComponents:[NSArray arrayWithObjects: yearPath, month, day, image, nil]]];
							media = [[[NSClassFromString(className) alloc] init] autorelease];
							[media setValue:imageURL forKey:@"imageURL"];
							[media setValue:image forKey:@"title"];
							NSCalendarDate *date = [NSCalendarDate dateWithYear:y month:m day:d hour:0 minute:0 second:0
													timeZone:[today timeZone]];							
							[media setValue:date forKey:@"date"];
							[_myDataSource addRecord:media forEntityName:@"com.mycompany.syncexamples.Media"];
						}
					}
				}
			}
		}
	}

	//NSLog (@"myDataSource = %@", [_myDataSource description]);
}

- (NSString *)integerToString:(int)i
{
	NSString *str;
	if (i < 10)
		str = [NSString stringWithFormat:@"%d%d", 0, i];
	else
		str = [NSString stringWithFormat:@"%d", i];
	return str;
}

- (NSProgressIndicator*) progressIndicator {
	return _progressIndicator;
}

- (NSButton*) trickleButton {
	return _trickleButton;
}
@end
