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
//  Media.m
//  MediaAssets and Events
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

// This is a custom entity class that is needed to enforce the
// the inverse to-many relationship between Event and Media. Whenever a Media
// object's event property (a to-one relationship) is set, the Media object is 
// added to the Event's media property (a to-many relationship).

#import "Media.h"

@implementation Media

- (id)init
{
    self = [super init];
    if (self) {
		_representation = [[NSMutableDictionary dictionary] retain];
    }
	
    return self;
}

- (void)dealloc {
    [_representation release];
    [super dealloc];
}


- (NSString *)description
{
	NSString *myDescription = [NSString stringWithFormat:@"[\n%@ = %@\n]", 
		@"_representation", [_representation description]];
	return myDescription;
}

- (id)date
{
	return [_representation valueForKey:@"date"];
}

- (void)_setDate:(id)value
{
	[_representation setValue:value forKey:@"date"];
}

- (id)event
{
	return [_representation valueForKey:@"event"];
}

- (void)_setEvent:(id)value
{
	[_representation setValue:value forKey:@"event"];
}

// Event is expected to be an array of primary keys, containing a single primary key if a to-one relationship,
// and multiple primary keys if a to-many relationship. This implementation is complicated because all 
// relationships are represented by an array of primary keys, not arrays of or a single entity object.
- (void)setEvent:(id)event
{
	//NSLog(@"Media setEvent:");
	id transformer = [NSValueTransformer valueTransformerForName:@"EventRelationshipTransformer"];
	
	// remove the receiver from the previous event
	id oldEvent = [_representation valueForKey:@"event"]; // returns an array of primary keys
	id oldMedia = [[transformer transformedValue:oldEvent] mutableArrayValueForKey:@"media"]; // returns an array of primary keys
	[oldMedia removeObject:[self valueForKey:@"primaryKey"]]; // remove receiver's primary key

	// set the receiver's event property
	[self _setEvent:event];
	
	// add the receiver to the new event
	id eventObject = [transformer transformedValue:event];
	if ([eventObject valueForKey:@"media"] == nil) // add the key-value pair if it doesn't exist yet
		[eventObject setValue:[NSMutableArray array] forKey:@"media"];
	NSMutableArray *media = [eventObject mutableArrayValueForKey:@"media"];
	if (![media containsObject:[self valueForKey:@"primaryKey"]])
		[media addObject:[self valueForKey:@"primaryKey"]];
}

- (id)imageURL
{
	return [_representation valueForKey:@"imageURL"];
}

- (void)_setImageURL:(id)value
{
	[_representation setValue:value forKey:@"imageURL"];
}

- (id)title
{
	return [_representation valueForKey:@"title"];
}

- (void)_setTitle:(id)value
{
	[_representation setValue:value forKey:@"title"];
}

- (id)primaryKey
{
	return [_representation valueForKey:@"primaryKey"];
}

- (void)_setPrimaryKey:(id)value
{
	[_representation setValue:value forKey:@"primaryKey"];
}

// Override the KVC "undefinedKey" methods so that other properties can be accessed.

- (id)valueForUndefinedKey:(NSString *)key
{
	return [_representation valueForKey:key];
}

- (void)setValue:(id)value forUndefinedKey:(NSString *)key
{
	[_representation setValue:value forKey:key];
}

- (NSDictionary*)representation
{
	return [[_representation retain] autorelease];
}

// Archiving Support

- (id)initWithCoder:(NSCoder *)coder
{
    //self = [super initWithCoder:coder];
	self = [super init];
    if ( [coder allowsKeyedCoding] ) {
        _representation = [[coder decodeObjectForKey:@"representation"] retain];
    } else {
        _representation = [[coder decodeObject] retain];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    //[super encodeWithCoder:coder];
    if ( [coder allowsKeyedCoding] ) {
        [coder encodeObject:_representation forKey:@"representation"];
    } else {
        [coder encodeObject:_representation];
    }
    return;
}

@end
