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
//  EventDocument.m
//  Events
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import "EventDocument.h"
#import <SyncExamples/EntityModel.h>
#import <SyncExamples/DataSource.h>
#import <SyncUtilities.h>
#import <SyncExamples/RelationshipTransformer.h>
#import "VCalTransformer.h"
#import "CalendarController.h"
#import "CalendarFilter.h"

@implementation EventDocument

- (id)init
{
    self = [super init];
    if (self) {    
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
		id transformer = [[[VCalTransformer alloc] init] autorelease];
		[NSValueTransformer setValueTransformer:transformer forName:@"VCalTransformer"];
		
		// Create a transformer to handle the to-one relationship from Media to Event
		transformer = [RelationshipTransformer transformerForDestinationEntity:@"com.mycompany.syncexamples.Event"
															   andDataSource:[self dataSource]];
		[transformer setToMany:NO];
		[NSValueTransformer setValueTransformer:transformer forName:@"EventRelationshipTransformer"];
		
		// Create a transformer to handle the to-many relationship from Event to Media
		transformer = [RelationshipTransformer transformerForDestinationEntity:@"com.mycompany.syncexamples.Media"
															   andDataSource:[self dataSource]];
		[transformer setToMany:YES];
		[NSValueTransformer setValueTransformer:transformer forName:@"MediaRelationshipTransformer"];
		
		// Create a transformer to handle the to-one relationship from Event to Calendar
		transformer = [RelationshipTransformer transformerForDestinationEntity:@"com.mycompany.syncexamples.Calendar"
																 andDataSource:[self dataSource]];
		[transformer setToMany:NO];
		[NSValueTransformer setValueTransformer:transformer forName:@"CalendarRelationshipTransformer"];
		
		// Create a transformer to handle the to-many relationship from Calendar to Event
		transformer = [RelationshipTransformer transformerForDestinationEntity:@"com.mycompany.syncexamples.Event"
																 andDataSource:[self dataSource]];
		[transformer setToMany:YES];
		[NSValueTransformer setValueTransformer:transformer forName:@"EventsRelationshipTransformer"];

		// Check first to see if a calendar was loaded from a saved filie
		id calendars = [_myDataSource valueForKey:@"calendarRecords"];
		if ((calendars != nil) && ([calendars count] > 0)){
			calendar = [calendars objectAtIndex:0];
		}
		// Otherwise, reate a calendar object with primary key set (early) so it can be used by the filter below
		else {
			calendar = [[NSMutableDictionary dictionaryWithObjectsAndKeys:@"Untitled", @"title", [NSMutableArray array], @"events", nil] retain];
			CFUUIDRef uuid = CFUUIDCreate(kCFAllocatorDefault);
			NSString *newKey = (NSString *)CFUUIDCreateString(kCFAllocatorDefault, uuid);
			CFRelease(uuid);
			[calendar setValue:newKey forKey:@"primaryKey"];
			[_myDataSource addRecord:calendar forEntityName:@"com.mycompany.syncexamples.Calendar"];
		}
		
		// create the sync filter  too
		id filter = [[CalendarFilter alloc] init];
		[filter setCalendarIdentifier:[calendar valueForKey:@"primaryKey"]];
		[self setFilters:[NSArray arrayWithObject:filter]];
    }
    return self;
}

- (void)dealloc {
    //[calendar release]; // was never retained above
	[calendarController release];
    [super dealloc];
}

- (void) registerSchemaIfNecessary
{
	[super registerSchemaIfNecessary];
	
	// Register the extension used by this app only
	NS_DURING
		NSLog(@"Registering schema: SyncExamplesExtension.syncschema");
		NSBundle *bundle = [NSBundle bundleForClass:[EventDocument class]];
		[[ISyncManager sharedManager] registerSchemaWithBundlePath:[[bundle resourcePath] stringByAppendingPathComponent:@"SyncExamplesExtension.syncschema"]];
	NS_HANDLER
		NSLog(@"Exception occured trying to register schema for SyncExamplesExtension:\n%@", localException);
	NS_ENDHANDLER	
}

- (NSString *)clientIdentifier
{
	return @"com.mycompany.syncexamples.calendar";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	[calendarController retain];
	[calendarController setDataSource:[self dataSource]];
	[[calendarController window] setTitle:@"Calendar of Events"];
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"EventDocument";
}

- (NSString *)displayName {
	return @"Calendar";
}

// Imports a "calendar" file from the ~/Library/Calendars folder by creating an Event object for each VCalRecord.
- (IBAction)importCalendar:(id)sender
{	
    int result;
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];		

	[openPanel setCanChooseDirectories:NO];
	[openPanel setCanChooseFiles:YES];
    [openPanel setAllowsMultipleSelection:NO];  // can handle only one calendar at a time	
	NSString *libraryPath = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/Calendars"];	
    result = [openPanel runModalForDirectory:libraryPath file:nil types:nil];
	
    if (result == NSOKButton) {
        NSArray *filesToOpen = [openPanel filenames];

		// Assumes there is one file being imported
		if ([filesToOpen count] > 0){
            NSString *file = [filesToOpen objectAtIndex:0];
			[self importCalendarWithContentsOfFile:file];
        }
    }
	
	//NSLog(@"eventRecords=%@", [[[self dataSource] valueForKey:@"eventRecords"] description]);
	
	return;
}

- (void)importCalendarWithContentsOfFile:(NSString *)file
{
	NSLog(@"Importing calendar with contents of:%@", file);
	// Take an array of VCalRecords and transforms it into an array of Event record
	NSMutableDictionary *beforeObjects = [SyncUtilities calendarWithContentsOfFile:file];
	//NSLog (@"allValues =%@", [beforeObject allValues]);
	id calendarEnumerator = [[beforeObjects allValues] objectEnumerator];
	id record;
	
	// Set the calendar title to match the imported calendar name
	[calendar setValue:[[file lastPathComponent] stringByDeletingPathExtension] forKey:@"title"];
	id calendarSyncValue = [[NSValueTransformer valueTransformerForName:@"CalendarRelationshipTransformer"] reverseTransformedValue:calendar];
	
	[_myDataSource beginBatching];
	// Transform all the VCalRecords to Event objects using custom value transformer
	while (record = [calendarEnumerator nextObject]){
		NSMutableDictionary *event = [[NSValueTransformer valueTransformerForName:@"VCalTransformer"] transformedValue:record];
		//NSLog(@"adding event=%@", [event description]);
		if (event != nil){
			[_myDataSource addRecord:event forEntityName:@"com.mycompany.syncexamples.Event"];
			[event setValue:calendarSyncValue forKey:@"calendar"];
			[[calendar mutableArrayValueForKey:@"events"] addObject:[event valueForKey:@"primaryKey"]];
			//[[calendar valueForKey:@"events"] addObject:[event valueForKey:@"primaryKey"]];
		}
	}
	[_myDataSource endBatching];
}

- (IBAction)syncDocument:(id)sender {
	[super syncDocument:sender];
	[calendarController nextMonth:self];
	[calendarController previousMonth:self];
	return;
}

- (IBAction)openCalendar:(id)sender
{
	// Pass along the open command to the current document
	[calendarController setYearObject:[calendarController firstYear]]; // should start on the first year
	[[calendarController window] makeKeyAndOrderFront:sender];
	return;
}

- (NSProgressIndicator*) progressIndicator {
	return _progressIndicator;
}

- (NSButton*) trickleButton {
	return _trickleButton;
}

// Filtering Methods

// Setting Filters

- (void)setFilters:(NSArray *)filters
{
	// Will create a sync client if it doesn't exist yet
	[[self getSyncClient] setFilters:filters];
}


@end

