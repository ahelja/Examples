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
//  VCalRecord.m
//  SyncExamples
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import "VCalRecord.h"

static NSString* sEventPreamble = @"BEGIN:VEVENT";
static NSString* sEventPostamble = @"END:VEVENT";
static NSString* sCalendarNewline = @"\r\n";

@interface VCalRecord(PrivateAPI)
+ (NSArray*) fieldsForVCalRecord:(NSString*)string useSpaceInSplitLines:(BOOL)useSpaceInSplitLines;
+ (NSString*) splitLine:(NSString*)string;
@end

@implementation VCalRecord

+ (VCalRecord*) recordWithString:(NSString*)string useSpaceInSplitLines:(BOOL)useSpaceInSplitLines
{
	int ii;
	int count;
	NSRange range;
	int valueIndex;
	NSString* key;
	NSString* value;
	NSMutableDictionary* stringDict = [NSMutableDictionary dictionary];
	NSMutableDictionary* attrDict = [NSMutableDictionary dictionary];
	NSCharacterSet* separatorCharacterSet = [NSCharacterSet characterSetWithCharactersInString:[NSString stringWithCString:":;"]];
	NSArray* lines = [self fieldsForVCalRecord:string useSpaceInSplitLines:useSpaceInSplitLines];
	count = [lines count];
	for (ii = 0; ii < count; ii++) {
		NSString* s = (NSString*)[lines objectAtIndex:ii];
		int length = [s length];
		//("Line %d is \"%s\"\n", ii, [s lossyCString]);
		// Look for separator
		range = [s rangeOfCharacterFromSet:separatorCharacterSet];
		if (range.location == NSNotFound) continue;
		// Snarf the key substring
		key = [s substringToIndex:range.location];
		// See if : (simple key/value) or ; (attributes before value)
		if ([s characterAtIndex:range.location] == ':') {
			// We have simple key/value
			valueIndex = range.location + 1; // Save start of value substring
			// Look for end of line or end of string
			value = [s substringFromIndex:range.location + 1];
			[stringDict setObject:value forKey:key];
		}
		else {
			NSString* subKey;
			NSString* subValue;
			int subValueIndex;
			int attrIndex = range.location + 1; // Start of attrs
			NSMutableDictionary* subdict = [NSMutableDictionary dictionary];
			// Look for end of attrs
			range = [s rangeOfString:@":" options:0 range:(NSRange){range.location, length - range.location}];
			if (range.location == NSNotFound) continue;
			valueIndex = range.location + 1; // Save start of value substring
			while (attrIndex != NSNotFound) {
				// Look for attr/value separator
				range = [s rangeOfString:@"=" options:0 range:(NSRange){attrIndex, length - attrIndex}];
				if (range.location == NSNotFound) break; // Nada, done with attrs
				subValueIndex = range.location + 1; // Save value
				// Pull out the key, than look for end of value
				subKey = [s substringWithRange:(NSRange){attrIndex, range.location - attrIndex}];
				range = [s rangeOfString:@";" options:0 range:(NSRange){subValueIndex, length - subValueIndex}];
				// If past attrs, grab last attr, set in dict and break attr subloop
				if ((range.location == NSNotFound) || (range.location >= valueIndex)) {
					subValue = [s substringWithRange:(NSRange){subValueIndex, valueIndex - subValueIndex - 1}];
					[subdict setObject:subValue forKey:subKey];
					break; // last attr so stop looking
				}
				else { // more attrs, grab this one and continue subloop
					subValue = [s substringWithRange:(NSRange){subValueIndex, range.location - subValueIndex}];
					[subdict setObject:subValue forKey:subKey];
					attrIndex = range.location + 1;
				}
			}
			// Get value after attrs
			value = [s substringFromIndex:valueIndex];
			// Add main value to string dict, add subdict to attr dict for key
			[stringDict setObject:value forKey:key];
			[attrDict setObject:subdict forKey:key];
		}
	}
	return [[[VCalRecord alloc] initWithStringDictionary:stringDict andAttrDict:attrDict] autorelease];
}

- (id) mutableCopy
{
	NSString* key;
	NSEnumerator* keyEnumerator;
	NSMutableDictionary* stringDict = [[_stringDict mutableCopy] autorelease];
	NSMutableDictionary* attrDict = [NSMutableDictionary dictionary];
	keyEnumerator = [_attrDict keyEnumerator];
	while (key = (NSString*)[keyEnumerator nextObject]) {
		[attrDict setObject:[[[_attrDict objectForKey:key] mutableCopy] autorelease] forKey:key];
	}
	return [[VCalRecord alloc] initWithStringDictionary:stringDict andAttrDict:attrDict];
}

- (NSString*) description:(BOOL)useLineBreaks
{
	NSString* key;
	NSString* string;
	NSString* line;
	NSEnumerator* keyEnumerator = [_stringDict keyEnumerator];
	string = [NSString stringWithFormat:@"%@%@", sEventPreamble, sCalendarNewline];
	while ((key = (NSString*)[keyEnumerator nextObject]) != nil) {
		if ([_attrDict objectForKey:key] == nil) {
			line = [NSString stringWithFormat:@"%@:%@", key, [_stringDict objectForKey:key]];
			if (useLineBreaks == YES) line = [VCalRecord splitLine:line];
			string = [NSString stringWithFormat:@"%@%@%@", string, line, sCalendarNewline];
		}
	}
	keyEnumerator = [_attrDict keyEnumerator];
	while ((key = [keyEnumerator nextObject]) != nil) {
		NSString* subKey;
		NSDictionary* subDict = (NSDictionary*)[_attrDict objectForKey:key];
		NSEnumerator* subKeyEnumerator = [subDict keyEnumerator];
		line = key;
		while ((subKey = [subKeyEnumerator nextObject]) != nil) {
			line = [NSString stringWithFormat:@"%@;%@=%@", line, subKey, (NSString*)[subDict objectForKey:subKey]];
		}
		line = [NSString stringWithFormat:@"%@:%@", line, (NSString*)[_stringDict objectForKey:key]];
		if (useLineBreaks == YES) line = [VCalRecord splitLine:line];
		string = [NSString stringWithFormat:@"%@%@%@", string, line, sCalendarNewline];
	}
	string = [NSString stringWithFormat:@"%@%@%@", string, sEventPostamble, sCalendarNewline];
	return string;
}

- (NSString*) description
{
	return [self description:NO];
}

- (void) setStringValue:(NSString*)value forKey:(NSString*)key
{
	if (value == nil) {
		[_stringDict removeObjectForKey:key];
	}
	else {
		[_stringDict setObject:value forKey:key];
	}
}

- (void) setAttributeValue:(NSString*)value forKey:(NSString*)key attributeKey:(NSString*)attributeKey
{
	NSMutableDictionary* dict = (NSMutableDictionary*)[_attrDict objectForKey:key];
	if (value == nil) {
		if (dict) {
			[dict removeObjectForKey:attributeKey];
			if ([dict count] == 0) {
				[_attrDict removeObjectForKey:key];
			}
		}
	}
	else {
		if (dict == nil) {
			dict = [NSMutableDictionary dictionary];
			[_attrDict setObject:dict forKey:key];
		}
		[dict setObject:value forKey:attributeKey];
	}
}

- (NSString*) stringValueForKey:(NSString*)key
{
	return [_stringDict objectForKey:key];
}

- (NSString*) attributeValueForKey:(NSString*)key attributeKey:(NSString*)attributeKey
{
	NSDictionary* dict = [_attrDict objectForKey:key];
	return [dict objectForKey:attributeKey];
}

- (VCalRecord*) initWithStringDictionary:(NSMutableDictionary*)stringDict andAttrDict:(NSMutableDictionary*)attrDict
{
	self = [super init];
	_stringDict = [stringDict retain];
	_attrDict = [attrDict retain];
	return self;
}

- (void) dealloc
{
	[_stringDict release];
	[_attrDict release];
	[super dealloc];
}

- (NSDictionary *)stringDict
{
return _stringDict;
}

- (NSDictionary *)attrDict
{
return _attrDict;
}

@end

@implementation VCalRecord(PrivateAPI)
+ (NSArray*) fieldsForVCalRecord:(NSString*)string useSpaceInSplitLines:(BOOL)useSpaceInSplitLines
{
	NSRange range;
	NSRange tempRange;
	int alphaLocation;
	int whitespaceLocation;
	int lineLocation;
	int length = [string length];
	NSMutableArray* array = [NSMutableArray array];
	NSCharacterSet* endLineCharacterSet = [NSCharacterSet characterSetWithCharactersInString:[NSString stringWithCString:"\n\f\r"]];
	NSCharacterSet* alphanumericCharacterSet = [NSCharacterSet alphanumericCharacterSet];
	NSCharacterSet* whitespaceCharacterSet = [NSCharacterSet whitespaceCharacterSet];
	// Skip past Begin
	range = [string rangeOfString:@"BEGIN:"];
	if (range.location == NSNotFound) return array;
	range = [string rangeOfCharacterFromSet:endLineCharacterSet options:0 range:(NSRange){range.location, length - range.location}];
	while (range.location != NSNotFound) {
		NSString* field = @"";
		// Look for start of next field
		range = [string rangeOfCharacterFromSet:alphanumericCharacterSet options:0 range:(NSRange){range.location, length - range.location}];
		if (range.location == NSNotFound) break;
		NSString* tempString = [string substringWithRange:(NSRange){range.location, 4}];
		if ([tempString isEqualToString:@"END:"]) break;
		while (range.location != NSNotFound) {
			lineLocation = range.location;
			range = [string rangeOfCharacterFromSet:endLineCharacterSet options:0 range:(NSRange){lineLocation, length - lineLocation}];
			if (range.location == NSNotFound) {
				field = [NSString stringWithFormat:@"%@%@", field, [string substringFromIndex:lineLocation]];
				break;
			}
			else {
				field = [NSString stringWithFormat:@"%@%@", field, [string substringWithRange:(NSRange){lineLocation, range.location - lineLocation}]];
				tempRange = [string rangeOfCharacterFromSet:alphanumericCharacterSet options:0 range:(NSRange){range.location, length - range.location}];
				alphaLocation = tempRange.location;
				tempRange = [string rangeOfCharacterFromSet:whitespaceCharacterSet options:0 range:(NSRange){range.location, length - range.location}];
				whitespaceLocation = tempRange.location;
				if ((alphaLocation != NSNotFound) && (whitespaceLocation != NSNotFound) && (whitespaceLocation < alphaLocation)) {
					range.location = whitespaceLocation + 1; // Continue with line extension for field
					if (useSpaceInSplitLines == YES) field = [NSString stringWithFormat:@"%@ ", field];
				}
				else break; // This field is complete
			}
		}
		[array addObject:field];
	}
	return array;
}

#define LINE_LENGTH 72
+ (NSString*) splitLine:(NSString*)string
{
	NSString* newString = @"";
	NSString* remainder = string;
	while (YES) {
		if ([remainder length] > LINE_LENGTH) {
			newString = [NSString stringWithFormat:@"%@%@%@ ", newString, [remainder substringToIndex:LINE_LENGTH], sCalendarNewline];
			remainder = [remainder substringFromIndex:LINE_LENGTH];
		}
		else {
			newString = [NSString stringWithFormat:@"%@%@", newString, remainder];
			break;
		}
	}
	return newString;
}

@end




