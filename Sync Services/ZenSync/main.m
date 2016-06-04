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
//  main.m
//  ZenSync
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <SyncServices/SyncServices.h>
#import "ZenCustomSyncClient.h"
#import "ZenStockSyncClient.h"
#import "strings.h"

/* This file only does basic app functionality things.  No sync specific code here.
* go check out ZenCustomSyncClient.[h|m] and ZenStockSyncClient.[h|m]
*/

typedef enum {
	notSet,
	failure,
	noSuchFile,
	help,
	dumpData,
	sync,
	refresh
} parseResult;

void printHelp() {
	NSLog(HELP_TEXT);
}

/* Dump out two text files, set up so that the first is ready for a slow sync push,
and the second for a fast sync pull */
void fDumpCustomData(NSString *fileLoc1, NSString *fileLoc2) {
	[[NSString stringWithFormat:@"%@\n%@0\n#%@\n%@\n", 
		DEFAULT_CUSTOM_COMMENT_LINE,
		DEFAULT_CUSTOM_TAG_LINE,
		DEFAULT_CUSTOM_CHANGES,
		DEFAULT_CUSTOM_RECORDS] writeToFile:fileLoc1 atomically:YES];
	[[NSString stringWithFormat:@"%@\n%@0\n%@\n", 
		DEFAULT_CUSTOM_COMMENT_LINE,
		DEFAULT_CUSTOM_TAG_LINE,
		DEFAULT_CUSTOM_CHANGES] writeToFile:fileLoc2 atomically:YES];
}

/* Dump out two text files, set up so that the first is ready for a slow sync push,
and the second for a fast sync pull */
void fDumpStockData(NSString *fileLoc1, NSString *fileLoc2) {
	[[NSString stringWithFormat:@"%@\n%@0\n#%@\n#%@\n%@\n", 
		DEFAULT_STOCK_COMMENT_LINE,
		DEFAULT_STOCK_TAG_LINE,
		DEFAULT_STOCK_CHANGES_1,
		DEFAULT_STOCK_CHANGES_2,
		DEFAULT_STOCK_RECORDS] writeToFile:fileLoc1 atomically:YES];
	[[NSString stringWithFormat:@"%@\n%@0\n%@\n%@\n", 
		DEFAULT_STOCK_COMMENT_LINE,
		DEFAULT_STOCK_TAG_LINE,
		DEFAULT_STOCK_CHANGES_1,
		DEFAULT_STOCK_CHANGES_2] writeToFile:fileLoc2 atomically:YES];
}

parseResult parseArgs(int argc, const char *argv[], NSString **fileLocP, BOOL *useCustom) {
	int i;
	*useCustom = YES;
	parseResult retVal = notSet;
	
	if (argc == 1) return failure;
	for (i = 1; i < argc; i++) {
		if ((!strcmp(argv[i], "-h")) || (!strcmp(argv[i], "--help")))
			return help;
		if (!strcmp(argv[i], "--custom")) {
			*useCustom = YES;
		} else if (!strcmp(argv[i], "--stock")) {
			*useCustom = NO;
		} else if (!strcmp(argv[i], "--dump-data")) {
			if (retVal != notSet)
				return failure;
			retVal = dumpData;
		} else if (!strcmp(argv[i], "--sync")) {
			if (retVal != notSet)
				return failure;
			retVal = sync;
			
			if (i == argc - 1) // no more args
				return failure;

			NSString *fileLoc = [[NSString stringWithFormat:@"%@/%s", DEFAULT_WORKING_DIR, argv[++i]] stringByExpandingTildeInPath];
			*fileLocP = fileLoc;
			if (![[NSFileManager defaultManager] fileExistsAtPath:fileLoc])
				return noSuchFile;
		} else if (!strcmp(argv[i], "--refresh")) {
			if (retVal != notSet)
				return failure;
			retVal = refresh;
			
			if (i == argc - 1) // no more args
				return failure;
			NSString *fileLoc = [[NSString stringWithFormat:@"%@/%s", DEFAULT_WORKING_DIR, argv[++i]] stringByExpandingTildeInPath];
			*fileLocP = fileLoc;
			if (![[NSFileManager defaultManager] fileExistsAtPath:fileLoc])
				return noSuchFile;
		} else {
			return failure;
		}
	}
	if (retVal == notSet)
		return failure;
	else
		return retVal;
}

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	NSString *fileLoc = NULL;
	BOOL useCustom, isRefresh = NO;

	// Ensure that ~/Library/Application Support/SyncExamples/ZenSync/ exists
	{
		NSArray *pathComps = [[DEFAULT_WORKING_DIR stringByExpandingTildeInPath] pathComponents];
		NSString *tempPath = @"";
		int i;
		BOOL isDir;
				
		for (i = 0; i < [pathComps count]; i++) {
			tempPath = [tempPath stringByAppendingPathComponent:(NSString *)[pathComps objectAtIndex:i]];
			if (!([[NSFileManager defaultManager] fileExistsAtPath:tempPath isDirectory:&isDir] && isDir))
				[[NSFileManager defaultManager] createDirectoryAtPath:tempPath attributes:nil];
		}
	}
	
	if (argc == 1) {
		NSLog(@"An error occurred in parsing the command line arguments to this tool.  Run this tool with -h for a help message");
		return -1;
	}
	
	switch(parseArgs(argc, argv, &fileLoc, &useCustom)) {
		case help:
			printHelp();
			break;
		case dumpData:
			if (useCustom) {
				NSString *fileLoc1 = [DEFAULT_CUSTOM_DATA_1 stringByExpandingTildeInPath];
				[[NSFileManager defaultManager] createFileAtPath:fileLoc1 contents:NULL attributes:NULL];
				NSString *fileLoc2 = [DEFAULT_CUSTOM_DATA_2 stringByExpandingTildeInPath];
				[[NSFileManager defaultManager] createFileAtPath:fileLoc2 contents:NULL attributes:NULL];
				if (fileLoc1 && fileLoc2) {
					fDumpCustomData(fileLoc1, fileLoc2);
					break;
				} else {
					NSLog(@"Can't create files to dump custom data");
					break;
				}
			} else {
				NSString *fileLoc1 = [DEFAULT_STOCK_DATA_1 stringByExpandingTildeInPath];
				[[NSFileManager defaultManager] createFileAtPath:fileLoc1 contents:NULL attributes:NULL];
				NSString *fileLoc2 = [DEFAULT_STOCK_DATA_2 stringByExpandingTildeInPath];
				[[NSFileManager defaultManager] createFileAtPath:fileLoc2 contents:NULL attributes:NULL];
				if (fileLoc1 && fileLoc2) {
					fDumpStockData(fileLoc1, fileLoc2);
					break;
				} else {
					NSLog(@"Can't create files to dump stock data");
					break;
				}
			}
			break;
		case refresh:
			isRefresh = YES;
		case sync:
		{
			id curClient;
			if (useCustom) {
				BOOL isDirectory = NO;
				if (!([[NSFileManager defaultManager] fileExistsAtPath:[DEFAULT_CUSTOM_SCHEMA stringByExpandingTildeInPath]
														 isDirectory:&isDirectory]
					  && isDirectory)) {
					NSLog(@"The custom schema is not present at %@, please rebuild this tool",
						  [DEFAULT_CUSTOM_SCHEMA stringByExpandingTildeInPath]);
					return -1;
				}
				curClient = [[ZenCustomSyncClient alloc] initWithFile:fileLoc isRefresh:isRefresh];
			} else {
				curClient = [[ZenStockSyncClient alloc] initWithFile:fileLoc isRefresh:isRefresh];
			}
			
			BOOL goodSync = [curClient sync];
			
			NSLog(@"The sync was %@", goodSync ? @"successful" : @"unsuccessful");
			
			if (goodSync)
				[curClient writeNewData];
			break;
		}
		case noSuchFile:
			NSLog(@"The file %@ doesn't exist", fileLoc);
			break;
		case failure:
		default:
			NSLog(@"An error occurred in parsing the command line arguments to this tool.  Run this tool with -h for a help message");
			break;
	}
    [pool release];
    return 0;
}
