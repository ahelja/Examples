/*
 
 File: GridMPINewJobWindowController.m
 
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

#import "GridMPINewJobWindowController.h"

@implementation GridMPINewJobWindowController

- (void)awakeFromNib;
{
	[self setValue:@"1" forKey:@"numberOfProcesses"];
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

- (NSDictionary *)jobSpecification;
{
	NSString *name = _jobName;
	NSString *command = _command;
	NSString *numberOfProcesses = _numberOfProcesses;

	if (name == nil || [name isEqualToString:@""] == YES) {
		
		name = command;
	}
	
	NSString *executablePath = [command stringByExpandingTildeInPath];
	
    if ([[self window] makeFirstResponder:nil] == NO || executablePath == nil || [executablePath length] == 0) {
		
        NSBeep();
		
        NSBeginAlertSheet(@"Executable path error",
                          @"OK",
                          nil,
                          nil,
                          [self window],
                          nil,
                          NULL,
                          NULL,
                          NULL,
                          @"No executable has been specified.");
		
        return nil;
    }
	
	NSString *bootstrapCommand = @"xgridmpiboot";
	
    NSString *bootstrapPath = [[NSBundle bundleForClass:[self class]] pathForAuxiliaryExecutable:bootstrapCommand];
	
    NSData *bootstrapData = [NSData dataWithContentsOfFile:bootstrapPath];
	
    NSData *executableData = [NSData dataWithContentsOfFile:executablePath];

    if (executableData != nil && bootstrapData != nil) {
		
		NSMutableDictionary *bootstrapFile = [NSMutableDictionary dictionary];

		[bootstrapFile setObject:bootstrapData forKey:XGJobSpecificationFileDataKey];
		
		NSMutableDictionary *executableFile = [NSMutableDictionary dictionary];
		
		[executableFile setObject:executableData forKey:XGJobSpecificationFileDataKey];
		[executableFile setObject:@"YES" forKey:XGJobSpecificationIsExecutableKey];

		NSMutableDictionary *inputFiles = [NSMutableDictionary dictionary];

		[inputFiles setObject:bootstrapFile forKey:bootstrapCommand];
		[inputFiles setObject:executableFile forKey:[executablePath lastPathComponent]];
		
		int numberOfNodes = [numberOfProcesses intValue];

		CFUUIDRef myUUID = CFUUIDCreate(kCFAllocatorDefault);
        
        CFStringRef myUUIDString = CFUUIDCreateString(kCFAllocatorDefault, myUUID);
		
        NSString *masterName = [NSString stringWithString:(NSString *)myUUIDString];
		
        if (myUUIDString != NULL) CFRelease(myUUIDString);
        
        if (myUUID != NULL) CFRelease(myUUID);
        
        if (masterName == nil) masterName = @"MPIMaster";
        
        NSMutableDictionary *taskSpecifications = [NSMutableDictionary dictionary];
        
		NSMutableDictionary *masterSpecification = [NSMutableDictionary dictionary];

        NSArray *masterArguments = [NSArray arrayWithObjects:
            @"-IsMaster", @"YES",
            @"-MasterName", masterName,
            @"-MPIType", @"MacMPI",
            @"-NumberOfProcesses", [NSString stringWithFormat:@"%d", numberOfNodes],
            @"-ExecutablePath", [executablePath lastPathComponent],
            nil];
        
		[masterSpecification setObject:bootstrapCommand forKey:XGJobSpecificationCommandKey];
		[masterSpecification setObject:masterArguments forKey:XGJobSpecificationArgumentsKey];
        
        [taskSpecifications setObject:masterSpecification forKey:@"0"];
        
		NSMutableDictionary *slaveSpecification = [NSMutableDictionary dictionary];

        NSArray *slaveArguments = [NSArray arrayWithObjects:
            @"-IsMaster", @"NO",
            @"-MasterName", masterName,
            @"-MPIType", @"MacMPI",
            @"-NumberOfProcesses", [NSString stringWithFormat:@"%d", numberOfNodes],
            @"-ExecutablePath", [executablePath lastPathComponent],
            nil];
        
		[slaveSpecification setObject:bootstrapCommand forKey:XGJobSpecificationCommandKey];
		[slaveSpecification setObject:slaveArguments forKey:XGJobSpecificationArgumentsKey];
        
        int i = 0;

        for (i = 1; i < numberOfNodes; i++) {
            
			[taskSpecifications setObject:slaveSpecification forKey:[NSString stringWithFormat:@"%d", i]];
        }
        
		NSString *applicationIdentifier = @"com.apple.xgrid.sample.mpi";

		NSMutableDictionary *jobSpecification = [NSMutableDictionary dictionary];
		
		[jobSpecification setObject:name forKey:XGJobSpecificationNameKey];
		[jobSpecification setObject:applicationIdentifier forKey:XGJobSpecificationApplicationIdentifierKey];
		[jobSpecification setObject:taskSpecifications forKey:XGJobSpecificationTaskSpecificationsKey];
		[jobSpecification setObject:inputFiles forKey:XGJobSpecificationInputFilesKey];

		return jobSpecification;
    }
    else {
		
        NSBeep();
		return nil;
    }

}


@end
