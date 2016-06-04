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
//  RelationshipTransformer.m
//  SyncExamples
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

// RelationshipTransformer transforms a sync to-one or to-many relationship to an entity object 
// or an array of entity objects. It also reverse transforms a to-one or to-many relationship in 
// entity objects, into a sync relationship (an array representation of record ids).

#import "RelationshipTransformer.h"
#import "DataSource.h"

@implementation RelationshipTransformer

+ (Class)transformedValueClass
{
    return [NSObject self]; // an object for a to-one relationship, otherwise an array
}

+ (BOOL)supportsReverseTransformation
{
    return YES;
}


// Creating a relationship transformer

// Convenience method for creating and registering instances with NSValueTransformer
+ (RelationshipTransformer *)transformerForDestinationEntity:(NSString *)aName andDataSource:(id)aSource
{
	id transformer = [[RelationshipTransformer alloc] init];
	[transformer setDataSource:aSource];
	[transformer setDestination:aName];
	[transformer setToMany:NO]; // default
	return [transformer autorelease];
}

- (void)dealloc {
	[dataSource release];
    [destination release];
    [super dealloc];
}


// Getting and setting properties

- (NSString *)destination
{
    return destination;
}

- (void)setDestination:(NSString *)aString
{
    [destination release];
    destination = [aString retain];
    return;
}

- (id)dataSource
{
	return dataSource;
}

- (void)setDataSource:(id)anObject
{
    [dataSource release];
    dataSource = [anObject retain];
    return;
}

- (BOOL)isToMany
{
	return isToMany;
}

- (void)setToMany:(BOOL)flag
{
	isToMany = flag;
}


// Transforming relationships

// The beforeObject is simply an array of record ids or primary keys. If the array contains 
// more than one object, then it's a to-many relationship, otherwise it is expected to be a 
// to-one relationship. Returns an entity object for a to-one relationship, and an array of
// entity objects for a to-many relationship.

- (id)transformedValue:(id)beforeObject
{
    //NSLog(@"RelationshipTransformer transformedValue:");
    //NSLog(@"beforeObject=%@", [beforeObject description]);

    if ((beforeObject == nil) || !([beforeObject isKindOfClass:[NSArray class]]) || (dataSource == nil) || (destination == nil))
		return nil;
	
	NSMutableArray *afterObject = [NSMutableArray array];
	NSEnumerator *primaryEnumerator = [beforeObject objectEnumerator];
	NSString *primaryKey;
	
	while (primaryKey = [primaryEnumerator nextObject]){
		id anObject = [dataSource recordWithPrimaryKey:primaryKey forEntityName:destination];
		if (anObject != nil)
			[afterObject addObject:anObject];
		else
			NSLog(@"Can't find record with primaryKey=%@ for destination=%@", primaryKey, destination);
	}
    //NSLog(@"afterObject=%@", [afterObject description]);
	if ([afterObject count] == 0)
		return nil;
	if ([self isToMany] == YES) return afterObject; // to-many
	else return [afterObject objectAtIndex:0]; // to-one
}

// This method does the reverse transformation. The beforeObject is expected to be a single enity object
// for a to-one relationship, and an array of entity objects for a to-many relationship. Returns an array
// containing the record ids or primary keys of the destination objects.

- (id)reverseTransformedValue:(id)beforeObject
{
    //NSLog(@"RelationshipTransformer reverseTransformedValue:");
    //NSLog(@"beforeObject=%@", [beforeObject description]);
	
	NSMutableArray *array = [NSMutableArray array];
	if (beforeObject != nil){
		// Handle to-one relationships
		if (!([beforeObject isKindOfClass:[NSArray class]])){ // should be better way to test for to-one
			[array addObject:[beforeObject valueForKey:@"primaryKey"]];
		}
		// Handle to-many relationships
		else {
			NSEnumerator *toManyEnumerator = [beforeObject objectEnumerator];
			id anObject;
			while (anObject = [toManyEnumerator nextObject]){
				[array addObject:[anObject valueForKey:@"primaryKey"]];
			}
		}
	}
	//NSLog(@"afterObject=%@", [array description]);
	if ([array count] > 0)
		return array;
	return nil;
}

@end
