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
//  VCalTransformer.m
//  Events
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

// VCalTransformer is a one-way transformer that takes a VCalRecord and converts 
// it into an Event object.

#import "VCalTransformer.h"
#import "VCalRecord.h"

static NSString* VCalTitleKey = @"SUMMARY";
static NSString* VCalStartDateKey = @"DTSTART";
static NSString* VCalEndDateKey = @"DTEND";
static NSString* EventTitleKey = @"title";
static NSString* EventStartDateKey = @"startDate";
static NSString* EventEndDateKey = @"endDate";

@implementation VCalTransformer

+ (Class)transformedValueClass
{
    return [VCalRecord self];
}

+ (BOOL)allowsReverseTransformation
{
    return NO;
}

- (id)transformedValue:(id)beforeObject
{
    // Takes a VCalRecord and transforms it into an Event record
	id afterObject = [NSMutableDictionary dictionary];
	NSDictionary *stringDict = [beforeObject stringDict];
	NSCalendarDate *start = nil, *end = nil;
	NSCalendarDate *today = [NSCalendarDate calendarDate];
	[today setCalendarFormat:@"%a %b %d %H:%M:%S %Z %Y"];
	
	if ([stringDict valueForKey:VCalStartDateKey]){
		start = [NSCalendarDate dateWithString:[stringDict valueForKey:VCalStartDateKey] 
												calendarFormat:@"%Y%m%d"];
		// Need to set format to match sync engine so comparisons work. Mon Feb 23 08:01:20 US/Pacific 2004
		[start setCalendarFormat:@"%a %b %d %H:%M:%S %Z %Y"];
	}
	if ([stringDict valueForKey:VCalEndDateKey]){
		end = [NSCalendarDate dateWithString:[stringDict valueForKey:VCalEndDateKey] 
										calendarFormat:@"%Y%m%d"];
		// Need to set format to match sync engine so comparisons work.
		[end setCalendarFormat:@"%a %b %d %H:%M:%S %Z %Y"];
	}
	
	// Can't do anything in this app with an event that has no title or start date
	if (([stringDict valueForKey:VCalTitleKey] == nil) || (start == nil)) return nil; 
	
	// Set the key-value pairs
	[afterObject setValue:[stringDict valueForKey:VCalTitleKey] forKey:EventTitleKey];
	[afterObject setValue:start forKey:EventStartDateKey];
    [afterObject setValue:end forKey:EventEndDateKey];
	//NSLog(@"afterObject=%@", [afterObject description]);
	
	return afterObject;
}

@end
