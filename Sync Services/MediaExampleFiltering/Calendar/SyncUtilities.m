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
//  SyncUtilities.m
//  SyncExamples
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

// This class  contains a bunch of utility methods and functions used through
// out the examples.

#import "SyncUtilities.h"
#import "VCalRecord.h"
#import <SyncExamples/EntityModel.h>
#import <SyncExamples/DataSource.h>
#import <SyncExamples/RecordTransformer.h>

static const char* sCalendarPostamble = "END:VCALENDAR";
static const char* sEventPreamble = "BEGIN:VEVENT";
static NSString *sUIDKeyString = @"UID";

static const char* sGetStringWithFilePath (NSString *filePath)
{
	//NSString* filepath = [NSString stringWithFormat:@"%@/%s/%s.ics", NSHomeDirectory(), sCalendarPrefix, sCalendarName];
	const char* s = [filePath fileSystemRepresentation];
	//printf("Using calendar file %s\n", s);
	return s;
}

static NSMutableDictionary* sReadCalendar (NSString *filePath)
{
	char* p;
	char* q;
	char* buffer;
	int size;
	FILE* fp;
	NSMutableDictionary* dict;
	
	dict = [NSMutableDictionary dictionary];
	fp = fopen(sGetStringWithFilePath(filePath), "r");
	if (fp == NULL) return nil;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buffer = (char*)calloc(size, 1);
	fread(buffer, size, 1, fp);
	p = buffer;
	p = strstr(p, sEventPreamble);
	while (p != NULL) {
		NSString* uidString;
		NSString* eventString;
		if (p[0] == 'E') break;
		q = strstr(p+1, sEventPreamble);
		if (q == NULL) q = strstr(p+1, sCalendarPostamble);
		if (q == NULL) break;
		eventString = [NSString stringWithCString:p length:q-p];
		VCalRecord* record =  [VCalRecord recordWithString:eventString useSpaceInSplitLines:NO];
		uidString = [record stringValueForKey:sUIDKeyString];
		if (uidString != nil) {
			//printf("Event String for uid %s is\n%s\n", [uidString lossyCString], [[record description:YES] lossyCString]);
			[dict setValue:record forKey:uidString];
		}
		else printf("Skipping event with no uid:\n%s\n", [[record description:YES] lossyCString]);
		p = q;
	}
	free(buffer);
	fclose(fp);
	return dict;
}

@implementation SyncUtilities

+ (NSMutableDictionary *)calendarWithContentsOfFile:(NSString *)filePath
{
	return sReadCalendar(filePath);
}

@end
