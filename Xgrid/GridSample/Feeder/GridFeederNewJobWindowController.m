/*
 
 File: GridFeederNewJobWindowController.m
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright 2005 Apple Computer, Inc., All Rights Reserved
 
*/

#import "GridFeederNewJobWindowController.h"
#import "GridFeederArgumentContainerController.h"

NSMutableArray *allCombinations(NSArray *argValueLists)
{
    int numArgValueLists = [argValueLists count];
	
    int repeatNumber;
    int i, j, repeatingElt;
    int written;
	
    int numCombinations = 1;
	
    for (i = 0; i < numArgValueLists; i++) {
	
        numCombinations *= [[argValueLists objectAtIndex:i] count];
    }
	
    NSMutableArray *outList = [NSMutableArray arrayWithCapacity:numCombinations];
	
    int currentArgValueListIndex = 0;
	
    NSArray *currentArgValues = [argValueLists objectAtIndex:currentArgValueListIndex];
    int numValues = [currentArgValues count];
    int remainder = numCombinations / numValues;
	
    for (i = 0; i < remainder; i++) {
	
        for (j = 0; j < numValues; j++) {
		
            NSMutableArray *outItem = [NSMutableArray arrayWithCapacity:numArgValueLists];
            [outItem addObject:[currentArgValues objectAtIndex:j]];
            [outList addObject:outItem];
        }
    }
	
    repeatNumber = numValues;
    currentArgValueListIndex = 1;
	
    while (currentArgValueListIndex < numArgValueLists) {
		
        written = 0;
		
        currentArgValues = [argValueLists objectAtIndex:currentArgValueListIndex];
        numValues = [currentArgValues count];
        remainder = numCombinations / (repeatNumber * numValues);
		
        for (i = 0; i < remainder; i++) {
		
            for (repeatingElt = 0; repeatingElt < numValues; repeatingElt++) {
			
                for (j = 0; j < repeatNumber; j++) {
					
                    [[outList objectAtIndex:written] addObject:[currentArgValues objectAtIndex:repeatingElt]];
					
                    written++;
                }
            }
        }
		
        repeatNumber *= numValues;
        currentArgValueListIndex++;
    }
	
    return outList;
}

@implementation GridFeederNewJobWindowController

- (void)awakeFromNib;
{
	[self setValue:@"/usr/bin/cal" forKey:@"command"];
}

- (IBAction)chooseCommand:(id)sender;
{
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];
    
    [oPanel setCanChooseDirectories:NO];
    [oPanel setCanChooseFiles:YES];
    [oPanel setAllowsMultipleSelection:NO];
    [oPanel setPrompt:@"Choose"];
    
    [oPanel beginSheetForDirectory:nil file:nil types:nil modalForWindow:[self window] modalDelegate:self
                    didEndSelector:@selector(chooseCommandOpenPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

- (void)chooseCommandOpenPanelDidEnd:(NSOpenPanel *)oPanel returnCode:(int)returnCode contextInfo:(void *)contextInfo;
{
    if (returnCode == NSFileHandlingPanelOKButton) {
        
        NSString *dirname = [oPanel filename];
        
		[self setValue:[[dirname stringByStandardizingPath] stringByAbbreviatingWithTildeInPath] forKey:@"command"];
    }
}

- (IBAction)chooseSourceDirectory:(id)sender;
{
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];
    
    [oPanel setCanChooseDirectories:YES];
    [oPanel setCanChooseFiles:NO];
    [oPanel setAllowsMultipleSelection:NO];
    [oPanel setPrompt:@"Choose"];
    
    [oPanel beginSheetForDirectory:nil file:nil types:nil modalForWindow:[self window] modalDelegate:self
                    didEndSelector:@selector(chooseSourceDirectoryOpenPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

- (void)chooseSourceDirectoryOpenPanelDidEnd:(NSOpenPanel *)oPanel returnCode:(int)returnCode contextInfo:(void *)contextInfo;
{
    if (returnCode == NSFileHandlingPanelOKButton) {
        
        NSString *dirname = [oPanel filename];
        
		[self setValue:[[dirname stringByStandardizingPath] stringByAbbreviatingWithTildeInPath] forKey:@"sourceDirectoryPath"];
    }
}

- (NSDictionary *)jobSpecification;
{
	// verify job parameters
	
	BOOL commandPathIsRelative = NO;
	
	NSString *fullSourceDirectoryPath = [_sourceDirectoryPath stringByExpandingTildeInPath];
	
	if ([fullSourceDirectoryPath isEqualToString:@""] == YES) fullSourceDirectoryPath = nil;
	
	if (fullSourceDirectoryPath != nil && [[NSFileManager defaultManager] isReadableFileAtPath:fullSourceDirectoryPath] == NO) {
		
		NSBeginAlertSheet(@"Source directory error", @"OK", nil, nil, [self window], nil, NULL, NULL, NULL, @"The source directory path you have entered does not exist or is unreadable.");
		
		return nil;
	}
	
	NSString *commandPath = [_command stringByExpandingTildeInPath];
	
	NSString *fullCommandPath = commandPath;
	
	if (commandPath == nil || [commandPath isEqualToString:@""] == YES || [commandPath isEqualToString:@"/"] == YES) {
		
		NSBeginAlertSheet(@"Command error", @"OK", nil, nil, [self window], nil, NULL, NULL, NULL, @"You must enter the path of the command you wish to run.");
		
		return nil;
	}
	
	if ([commandPath isAbsolutePath] == NO) {
		
		if (fullSourceDirectoryPath != nil) {
			
			commandPathIsRelative = YES;
			
			fullCommandPath = [fullSourceDirectoryPath stringByAppendingPathComponent:fullCommandPath];
		}
		else {
			
			NSBeginAlertSheet(@"Command error", @"OK", nil, nil, [self window], nil, NULL, NULL, NULL, @"Relative command paths are only supported if a source directory is specified.");
			
			return nil;
		}
	}
	
	if ([[NSFileManager defaultManager] isExecutableFileAtPath:fullCommandPath] == NO) {
		
		NSBeginAlertSheet(@"Command error", @"OK", nil, nil, [self window], nil, NULL, NULL, NULL, @"The command path you have entered does not exist on this computer or is not executable.");
		
		return nil;
	}
	
	NSArray *arrayOfArgumentStringArrays = [_argumentContainerController arrayOfArgumentStringArrays];
	
	NSArray *arrayOfCombinedArgumentArrays = nil;
	
	if ([arrayOfArgumentStringArrays count] == 1 && [[arrayOfArgumentStringArrays objectAtIndex:0] count] < 1) {
		
		// special case: only one argument and it is empty
		arrayOfCombinedArgumentArrays = [NSArray arrayWithObject:[NSArray array]];
	}
	else {
		
		NSEnumerator *argumentStringArrayEnumerator = [arrayOfArgumentStringArrays objectEnumerator];
		NSArray *argumentStringArray = nil;
				
		while (argumentStringArray = [argumentStringArrayEnumerator nextObject]) {
			
			if ([argumentStringArray count] < 1) {
				
				NSBeginAlertSheet(@"Argument error", @"OK", nil, nil, [self window], nil, NULL, NULL, NULL, @"You must fill in all of the argument fields or remove the blank ones.");
				
				return nil;
			}
		}
		
		arrayOfCombinedArgumentArrays = allCombinations(arrayOfArgumentStringArrays);
	}
	
	// generate the job name
	
	NSString *jobName = _jobName;
	
	if (jobName == nil || [jobName isEqualToString:@""] == YES) {
		
		jobName = _command;
	}
	
	// generate the application identifier
	
	NSString *applicationIdentifier = @"com.apple.xgrid.sample.feeder";
	
	// generate the input files
	
	NSMutableDictionary *inputFiles = [NSMutableDictionary dictionary];
	
	NSDirectoryEnumerator *sourceDirectoryFilePathEnumerator = [[NSFileManager defaultManager] enumeratorAtPath:fullSourceDirectoryPath];
	
	NSString *sourceDirectoryFilePath = nil;
	
	while (sourceDirectoryFilePath = [sourceDirectoryFilePathEnumerator nextObject]) {
		
		NSDictionary *sourceDirectoryFileAttributes = [sourceDirectoryFilePathEnumerator fileAttributes];
		
		NSString *sourceDirectoryFileType = [sourceDirectoryFileAttributes objectForKey:NSFileType];
		
		if ([sourceDirectoryFileType isEqualToString:NSFileTypeRegular] == YES) {
			
			NSString *fullSourceDirectoryFilePath = [fullSourceDirectoryPath stringByAppendingPathComponent:sourceDirectoryFilePath];
			
			NSData *sourceDirectoryFileData = [NSData dataWithContentsOfFile:fullSourceDirectoryFilePath];
			
			NSMutableDictionary *sourceDirectoryFile = [NSMutableDictionary dictionary];
			
			[sourceDirectoryFile setObject:sourceDirectoryFileData forKey:XGJobSpecificationFileDataKey];
			
			if ([[NSFileManager defaultManager] isExecutableFileAtPath:fullSourceDirectoryFilePath] == YES) {
				
				[sourceDirectoryFile setObject:@"YES" forKey:XGJobSpecificationIsExecutableKey];
			}
			
			[inputFiles setObject:sourceDirectoryFile forKey:sourceDirectoryFilePath];
		}
	}
	
	// generate the task specifications
	
	NSMutableDictionary *taskSpecifications = [NSMutableDictionary dictionary];

	NSEnumerator *combinedArgumentArrayEnumerator = [arrayOfCombinedArgumentArrays objectEnumerator];
	NSArray *combinedArgumentArray = nil;
	
	int taskIdentifierInt = 1;
	
	while (combinedArgumentArray = [combinedArgumentArrayEnumerator nextObject]) {
		
		NSMutableDictionary *taskSpecification = [NSMutableDictionary dictionary];
					
		[taskSpecification setObject:commandPath forKey:XGJobSpecificationCommandKey];
		[taskSpecification setObject:combinedArgumentArray forKey:XGJobSpecificationArgumentsKey];
	
		NSString *taskIdentifier = [NSString stringWithFormat:@"%d", taskIdentifierInt++];
		
		[taskSpecifications setObject:taskSpecification forKey:taskIdentifier];
	}
	
	// generate the job specification
	
	NSMutableDictionary *jobSpecification = [NSMutableDictionary dictionary];
	
	[jobSpecification setObject:jobName forKey:XGJobSpecificationNameKey];
	[jobSpecification setObject:applicationIdentifier forKey:XGJobSpecificationApplicationIdentifierKey];
	[jobSpecification setObject:inputFiles forKey:XGJobSpecificationInputFilesKey];
	[jobSpecification setObject:taskSpecifications forKey:XGJobSpecificationTaskSpecificationsKey];
	
	return jobSpecification;
}

@end
