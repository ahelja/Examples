#import "XMethodsServiceList.h"
#include "XMethodsStubs.h"

static int _compareFunc(NSDictionary* a, NSDictionary* b, void* c)
{
	NSString* sa = [a objectForKey: @"name"];
	NSString* sb = [b objectForKey: @"name"];
	if (sa == NULL || sb == NULL)
		return 0;
	return [sa caseInsensitiveCompare: sb];
}

@implementation XMethodsServiceList

-(void) awakeFromNib
{
	// A synchronous call
	NSArray* unsorted = (NSArray*) [XMethodsQueryService getAllServiceNames];
	fMethods = [[[unsorted autorelease] sortedArrayUsingFunction:_compareFunc context:NULL] retain];
	
	[fList setDataSource: self];
}

- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	return [fMethods count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	return [(NSDictionary*) [fMethods objectAtIndex: row] objectForKey: @"name"];
}

-(NSString*) idForItem:(int) row
{
	return [(NSDictionary*) [fMethods objectAtIndex: row] objectForKey: @"id"];
}

@end
