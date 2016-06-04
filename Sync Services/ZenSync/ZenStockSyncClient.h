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
//  ZenStockSyncClient.h
//  ZenSync
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ZenStockSyncClient : NSObject {
	NSArray *_calFields; // The different fields used in the calendar entity of the ccalendars schema
	NSMutableDictionary *_calChanges, *_calRecords; // the parsed changes represented by CC: lines, and the parsed
											  // records represented by RC: lines
	
	NSArray *_evtFields; // The different fields used in the Event entity of the ccalendars schema
	NSMutableDictionary *_evtChanges, *_evtRecords; // the parsed changes represented by CE: lines, and the parsed
													// records represented by RE: lines
	
	int _lastSyncNumber, _highestLocalId; // the last sync number, a server stored datum, and the highest
										  // number we've used for a local record id

	BOOL _isRefresh; // Are we being asked to refresh our data from the truth?

	NSString *_fileLoc; // the absolute location of the file we're parsing / writing to
	NSString *_plistLoc; // the absolute location of the custom plist we create for this client
}

- (id)initWithFile:(NSString *)fileLoc isRefresh:(BOOL)isRefresh; // parses the given file
- (BOOL)sync; // Performs a slow or fast sync depending on the file

@end

@interface ZenStockSyncClient (NonSyncMethods)

- (BOOL)_readDataFromFile;
- (BOOL)_parseChangeLine:(NSString *)curLine fields:(NSArray *)fields intoDict:(NSMutableDictionary *)changeDictionary entityName:(NSString *)entityName numAddsP:(int *)numAddsP;
- (NSMutableDictionary *)_mutDictFromChanges:(NSArray *)changes startingDict:(NSMutableDictionary *)startingDict;
- (void)writeNewData;
- (NSString *)dumpIcon;
- (NSString *)createCustomPlist:(NSString *)basePlist;

@end

