/*

File: GMMainWindowController.m

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

Copyright 2004 Apple Computer, Inc., All Rights Reserved

*/ 

#import "GMMainWindowController.h"
#import "GMApplicationDelegate.h"
#import "GMRectangleDraggerView.h"
#import "GMServiceBrowser.h"
#import "GMConnectionController.h"
#import <libkern/OSByteOrder.h>

#define MAX_ITERATIONS	8192

static const int GMMainWindowOKReturnCode = 0;
static const int GMMainWindowCancelReturnCode = 1;
static const int GMMainWindowAuthenticationNeededReturnCode = 2;

static const int GMMainWindowAuthenticationMethodSSOTag = 0;
static const int GMMainWindowAuthenticationMethodPasswordTag = 1;

@implementation GMMainWindowController

- (id)init;
{
	self = [super initWithWindowNibName:@"MainWindow"];
	
	if (self != nil) {
	
		_pixelsWide = 480;
		_pixelsHigh = _pixelsWide;

		_patchPixelsWide = _pixelsWide / 3;
		_patchPixelsHigh = _pixelsHigh / 3;

		_currentImageX = -2.1;
		_currentImageY = -1.4;
		_currentImageWidth = 2.8;
		
		_nextImageX = _currentImageX;
		_nextImageY = _currentImageY;
		_nextImageWidth = _currentImageWidth;
		
		[self setValue:@"Start" forKey:@"toggleAutomaticTourTitle"];
		[self setValue:[NSNumber numberWithBool:YES] forKey:@"useAltivec"];
		[self setValue:[NSNumber numberWithBool:YES] forKey:@"canToggleAutomaticTour"];
		[self setValue:[NSNumber numberWithBool:YES] forKey:@"canShowImage"];
		
		NSString *tourPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"tour" ofType:@"txt"];
		
		_tourArray = [[NSArray alloc] initWithContentsOfFile:tourPath];
		
		_settingsArray = [[NSMutableArray alloc] init];
		
		[_settingsArray addObject:[self currentImageSettings]];
		
		_imageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
										 				    pixelsWide:_pixelsWide
										 				    pixelsHigh:_pixelsHigh
														 bitsPerSample:8
													   samplesPerPixel:3
															  hasAlpha:NO
															  isPlanar:NO
													    colorSpaceName:NSDeviceRGBColorSpace
														   bytesPerRow:_pixelsWide * 3
														  bitsPerPixel:24];

		NSSize imageSize = NSMakeSize(_pixelsWide, _pixelsHigh);

		[_imageRep setSize:imageSize];
		
		_image = [[NSImage alloc] initWithSize:imageSize];
		
		[_image addRepresentation:_imageRep];
		
		[self clearImageRep];
		
		[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(updateSettingsFromUserDrag:)
                                                     name:NewDraggedRecNotification
                                                   object:_imageView];
		
		_submissionMonitors = [[NSMutableSet alloc] init];
		_jobIdentifiers = [[NSMutableSet alloc] init];
		_jobs = [[NSMutableSet alloc] init];
	}
	
	return self;
}

- (void)dealloc;
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

	[_grid release];
	[_tourArray release];
	[_settingsArray release];
	[_imageRep release];
	[_image release];
	[_submissionMonitors release];
	[_jobIdentifiers release];
	[_jobs release];
	
	[super dealloc];
}

- (void)awakeFromNib;
{
	[_imageView setImageFrameStyle:NSImageFrameGrayBezel];
	
	[self refreshImage];
}

#pragma mark *** Convenience methods ***

- (void)connectionDidOpen;
{
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isConnecting"];
	[self setValue:@"" forKey:@"connectMessage"];

	[NSApp endSheet:_connectSheet returnCode:GMMainWindowOKReturnCode];

	[self performSelector:@selector(showInitialImage) withObject:nil afterDelay:1.0];
}

- (void)connectionDidNotOpen;
{
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isConnecting"];
	[self setValue:@"Connection failed" forKey:@"connectMessage"];
}

- (void)connectionWasCanceled;
{
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isConnecting"];
	[self setValue:@"Connection canceled" forKey:@"connectMessage"];
}

- (void)connectionDidClose;
{
	_doingTour = NO;
	
	// remove all jobs
	[self removeAllJobs];
	
	// reset button states and titles
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"animate"];
	[self setValue:@"Start" forKey:@"toggleAutomaticTourTitle"];
	[self setValue:[NSNumber numberWithBool:YES] forKey:@"canShowImage"];
	[self setValue:[NSNumber numberWithBool:YES] forKey:@"canToggleAutomaticTour"];
}

- (void)connectionAuthenticationNeeded;
{
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isConnecting"];
	[self setValue:@"" forKey:@"connectMessage"];

	[NSApp endSheet:_connectSheet returnCode:GMMainWindowAuthenticationNeededReturnCode];
}

- (XGGrid *)grid;
{
	if (_grid == nil) {
		
		_grid = [[[[[NSApp delegate] connectionController] controller] defaultGrid] retain];
	}
	
	return _grid;
}

- (NSDictionary *)settingsDictionaryWithX:(double)x y:(double)y width:(double)width;
{
    return [NSDictionary dictionaryWithObjectsAndKeys:
        [NSString stringWithFormat:@"%.20e", x], @"x",
        [NSString stringWithFormat:@"%.20e", y], @"y",
        [NSString stringWithFormat:@"%.20e", width], @"w",
        nil];
}

- (NSDictionary *)currentImageSettings;
{
	return [self settingsDictionaryWithX:_currentImageX
									   y:_currentImageY
								   width:_currentImageWidth];
}

- (NSDictionary *)nextImageSettings;
{
	return [self settingsDictionaryWithX:_nextImageX
									   y:_nextImageY
								   width:_nextImageWidth];
}

- (NSDictionary *)currentSettings;
{
	return [[[_settingsArray objectAtIndex:_currentSettingsIndex] retain] autorelease];
}

- (void)incrementTourIndex;
{
	int newTourIndex;
	
	if (_currentTourIndex >= ([_tourArray count] - 1)) newTourIndex = 0;
	else newTourIndex = _currentTourIndex + 1;
	
	[self setValue:[NSNumber numberWithInt:newTourIndex] forKey:@"currentTourIndex"];
}

- (NSDictionary *)currentTourSettings;
{
	return [[[_tourArray objectAtIndex:_currentTourIndex] retain] autorelease];
}

- (void)clearImageRep;
{
    unsigned char 	*pixels;
    int				i;
    
    pixels = [_imageRep bitmapData];

    int length = [_imageRep pixelsHigh] * [_imageRep pixelsWide];
    
    for (i=0; i<length; i++) {
        pixels[3*i+0] = 128;
        pixels[3*i+1] = 128;
        pixels[3*i+2] = 128;
    }
}

- (void)fadeImageRep;
{
    unsigned char 	*pixels;
    int				i;

    pixels = [_imageRep bitmapData];

    int length = [_imageRep pixelsHigh] * [_imageRep pixelsWide];
    
    for (i=0; i<length; i++) {
        int red = pixels[3*i+0] + 50;
        int green = pixels[3*i+1] + 50;
        int blue = pixels[3*i+2] + 50;

        if (red > 255) red = 255;
        if (green > 255) green = 255;
        if (blue > 255) blue = 255;
        
        pixels[3*i+0] = red;
        pixels[3*i+1] = green;
        pixels[3*i+2] = blue;
    }
}

- (void)refreshImage;
{
    [_imageView setImage:nil];
    [_imageView setImage:_image];

    [_imageView setNeedsDisplay:YES];
}

- (void)setCurrentImageSettings:(NSDictionary *)settings;
{
	double newX = [[settings objectForKey:@"x"] doubleValue];
	double newY = [[settings objectForKey:@"y"] doubleValue];
	double newWidth = [[settings objectForKey:@"w"] doubleValue];
	
	[self setValue:[NSNumber numberWithDouble:newX] forKey:@"currentImageX"];
	[self setValue:[NSNumber numberWithDouble:newY] forKey:@"currentImageY"];
	[self setValue:[NSNumber numberWithDouble:newWidth] forKey:@"currentImageWidth"];
}

- (void)setNextImageSettings:(NSDictionary *)settings;
{
	double newX = [[settings objectForKey:@"x"] doubleValue];
	double newY = [[settings objectForKey:@"y"] doubleValue];
	double newWidth = [[settings objectForKey:@"w"] doubleValue];
	
	[self setValue:[NSNumber numberWithDouble:newX] forKey:@"nextImageX"];
	[self setValue:[NSNumber numberWithDouble:newY] forKey:@"nextImageY"];
	[self setValue:[NSNumber numberWithDouble:newWidth] forKey:@"nextImageWidth"];
}

- (void)showInitialImage;
{
	if (_doingTour == NO && _canShowImage == YES) {
	
		_doingTour = YES;
	
		[self setValue:@"Stop" forKey:@"toggleAutomaticTourTitle"];
		[self setValue:[NSNumber numberWithBool:NO] forKey:@"canShowImage"];
		
		[self performTour];
	}
/*
	if (_canShowImage == YES) {
	
		[self setValue:[NSNumber numberWithBool:NO] forKey:@"canShowImage"];
		[self setValue:[NSNumber numberWithBool:NO] forKey:@"canToggleAutomaticTour"];

		[self showImageWithSettings:[self currentSettings]];
	}
*/
}

- (void)showImageWithSettings:(NSDictionary *)settings;
{
	[self setValue:[NSNumber numberWithBool:YES] forKey:@"animate"];
	
	[self setCurrentImageSettings:settings];
	[_imageView setDraggedRectWithX:0 y:0 w:0 h:0];

	[self fadeImageRep];
	[self refreshImage];
	
	// do the work
	[self submitJobs];
}

- (void)submitJobs;
{
	int oneJobWidth = _patchPixelsWide;
	int oneJobHeight = _patchPixelsHigh;

    int jobsHorizontal = _pixelsWide/_patchPixelsWide;
    int jobsVertical   = _pixelsHigh/_patchPixelsHigh;

    double oneJobWidthInComplexPlane  = _currentImageWidth / jobsHorizontal;
    double oneJobHeightInComplexPlane = oneJobWidthInComplexPlane * (_pixelsHigh/_pixelsWide);

    int i,j;

	for (i = 0; i < jobsVertical; i++) {

		for (j = 0; j < jobsHorizontal; j++) {
		
			NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

			NSString *commandPath = [[NSBundle bundleForClass:[self class]] pathForAuxiliaryExecutable:@"MandelTool"];

			NSString *taskCommand = @"MandelTool";

			NSString *vectorizationFlag = _useAltivec ? @"1" : @"0";
			
			if (_doingTour == YES) vectorizationFlag = @"0";
			
			NSArray *argumentsArray = [NSArray arrayWithObjects:
				[NSString stringWithFormat:@"%d", j * oneJobWidth],
				[NSString stringWithFormat:@"%d", i * oneJobHeight],
				[NSString stringWithFormat:@"%d", oneJobWidth],
				[NSString stringWithFormat:@"%d", oneJobHeight],
				[NSString stringWithFormat:@"%.20e", oneJobWidthInComplexPlane],
				[NSString stringWithFormat:@"%.20e", oneJobHeightInComplexPlane ],
				[NSString stringWithFormat:@"%.20e", _currentImageX + j * oneJobWidthInComplexPlane],
				[NSString stringWithFormat:@"%.20e", _currentImageY + i * oneJobHeightInComplexPlane],
				[NSString stringWithFormat:@"%d", MAX_ITERATIONS],                    
				vectorizationFlag,
				nil];

			NSData *commandFileData = [NSData dataWithContentsOfFile:commandPath];
	
			NSMutableDictionary *commandFile = [NSMutableDictionary dictionary];
			[commandFile setObject:commandFileData forKey:XGJobSpecificationFileDataKey];

			NSMutableDictionary *inputFiles = [NSMutableDictionary dictionary];
			[inputFiles setObject:commandFile forKey:@"MandelTool"];

			// task specification
			NSMutableDictionary *taskSpecification = [NSMutableDictionary dictionary];
			[taskSpecification setObject:taskCommand forKey:XGJobSpecificationCommandKey];
			[taskSpecification setObject:argumentsArray forKey:XGJobSpecificationArgumentsKey];
			
			// task specifications
			NSString *taskIdentifier = @"0";
			NSMutableDictionary *taskSpecifications = [NSMutableDictionary dictionary];
			[taskSpecifications setObject:taskSpecification forKey:taskIdentifier];
	
			// job specification
			NSMutableDictionary *jobSpecification = [NSMutableDictionary dictionary];
			// [jobSpecification setObject:XGJobSpecificationTypeTaskListValue forKey:XGJobSpecificationTypeKey];
			[jobSpecification setObject:@"MandelTool" forKey:XGJobSpecificationNameKey];
			// [jobSpecification setObject:@"abc" forKey:XGJobSpecificationSubmissionIdentifierKey];
			[jobSpecification setObject:GMApplicationIdentifier forKey:XGJobSpecificationApplicationIdentifierKey];
			[jobSpecification setObject:taskSpecifications forKey:XGJobSpecificationTaskSpecificationsKey];
			[jobSpecification setObject:inputFiles forKey:XGJobSpecificationInputFilesKey];

			XGController *controller = [[[NSApp delegate] connectionController] controller];
			
			NSString *gridIdentifier = [[self grid] identifier];
			
			XGActionMonitor *actionMonitor = [controller performSubmitJobActionWithJobSpecification:jobSpecification gridIdentifier:gridIdentifier];

			[self addSubmissionMonitor:actionMonitor];
			
			[pool release];
		}
	}
}

- (void)addSubmissionMonitor:(XGActionMonitor *)submissionMonitor;
{
	[_submissionMonitors addObject:submissionMonitor];

	[submissionMonitor addObserver:self forKeyPath:@"outcome" options:0 context:NULL];
}

- (void)removeSubmissionMonitor:(XGActionMonitor *)submissionMonitor;
{
	[submissionMonitor removeObserver:self forKeyPath:@"outcome"];
	
	[_submissionMonitors removeObject:submissionMonitor];
}

- (void)addJobIdentifier:(NSString *)jobIdentifier;
{
	if ([_jobIdentifiers count] == 0) {
		
		[[self grid] addObserver:self forKeyPath:@"jobs" options:0 context:NULL];
	}
		
	[_jobIdentifiers addObject:jobIdentifier];
}

- (void)removeJobIdentifier:(NSString *)jobIdentifier;
{
	[_jobIdentifiers removeObject:jobIdentifier];
	
	if ([_jobIdentifiers count] == 0) {
		
		[[self grid] removeObserver:self forKeyPath:@"jobs"];
	}
	
}

- (void)addJob:(XGJob *)job;
{
	[job addObserver:self forKeyPath:@"state" options:0 context:NULL];
	
	[_jobs addObject:job];
}

- (void)removeJob:(XGJob *)job;
{
	[job removeObserver:self forKeyPath:@"state"];
	
	[job performDeleteAction];
	
	[_jobs removeObject:job];
	
	if ([self outstandingJobCount] == 0) {
	
		[self setValue:[NSNumber numberWithBool:NO] forKey:@"animate"];
		
		if (_doingTour == YES) {
		
			[self performSelector:@selector(performTour) withObject:nil afterDelay:0.0];
		}
		else {
		
			[self setValue:[NSNumber numberWithBool:YES] forKey:@"canShowImage"];
			[self setValue:[NSNumber numberWithBool:YES] forKey:@"canToggleAutomaticTour"];
		}
	}
}

- (void)removeAllJobs;
{
	NSEnumerator *jobEnumerator = [_jobs objectEnumerator];
	
	XGJob *job;
	
	while (job = [jobEnumerator nextObject]) {
	
		[job removeObserver:self forKeyPath:@"state"];
		
		[job performDeleteAction];
		
		[_jobs removeObject:job];
	}
}

- (int)outstandingJobCount;
{
	return ([_jobs count] + [_jobIdentifiers count] + [_submissionMonitors count]);
}

- (void)addGetOutputStreamsMonitor:(XGActionMonitor *)getOutputStreamsMonitor;
{
	[getOutputStreamsMonitor addObserver:self forKeyPath:@"outcome" options:0 context:NULL];
}

- (void)getStandardOutput:(XGFile *)standardOutput;
{
    [[XGFileDownload alloc] initWithFile:standardOutput delegate:self];
}

- (void)performTour;
{
	if (_doingTour == YES) {
	
		[self showImageWithSettings:[self currentTourSettings]];
	
		[self incrementTourIndex];
	}
}

- (void)writeImageFromData:(NSData *)data;
{
    // data is a list of dataWidth * dataHeight longs, each of which is the escape
    // value of that point from the Mandelbrot set
	// data also contains the coordinates and scale

    unsigned long *dataPtr = (unsigned long *)[data bytes];
    int dataCursor = 0;
    
    if (dataPtr == 0) return;

    unsigned char *pixels = [_imageRep bitmapData];

    int i, j;
    int index;
    int datum;
    
    // Parse the header of the results
    int x, y;
    int dataWidth, dataHeight;

    x = OSReadBigInt32(dataPtr, dataCursor);
    dataCursor += sizeof(uint32_t);
    
    y = OSReadBigInt32(dataPtr, dataCursor);
    dataCursor += sizeof(uint32_t);
    
    dataWidth = OSReadBigInt32(dataPtr, dataCursor);
    dataCursor += sizeof(uint32_t);
    
    dataHeight = OSReadBigInt32(dataPtr, dataCursor);
    dataCursor += sizeof(uint32_t);

    for (i = 0; i < dataHeight; i++) {
	
        for (j = 0; j < dataWidth; j++) {
		
            index = 3 * ((y + i)*_pixelsWide + x + j);
            
            datum = OSReadBigInt32(dataPtr, dataCursor);
            dataCursor += sizeof(uint32_t);

            pixels[index]     =  0 + 7 * datum;
            pixels[index + 1] =  80 + 23 * (datum / 4);
            pixels[index + 2] =  40 + 41 * (datum / 32);

            // make all members of the set black
            if (datum == MAX_ITERATIONS) {
			
                pixels[index] = pixels[index + 1] = pixels[index + 2]  = 0;
            }
        }
    }

	[self refreshImage];
}

#pragma mark *** Sheet methods ***

- (void)showConnectSheet;
{
	NSArray *netServiceNames = [[[NSApp delegate] serviceBrowser] valueForKeyPath:@"netServices.name"];
	
	if ([netServiceNames count] > 0) {
		
		[self setValue:[netServiceNames objectAtIndex:0] forKey:@"connectName"];
	}
	
	[self setValue:@"" forKey:@"connectMessage"];

	[NSApp beginSheet:_connectSheet modalForWindow:[self window] modalDelegate:self didEndSelector:@selector(connectSheetDidEnd:returnCode:contextInfo:) contextInfo:NULL];
}

- (void)showAuthenticationNeededSheetForConnectionController:(GMConnectionController *)connectionController;
{
	[self setValue:@"" forKey:@"authenticationPassword"];

	[NSApp beginSheet:_authenticationNeededSheet modalForWindow:[self window] modalDelegate:self didEndSelector:@selector(authenticationNeededSheetDidEnd:returnCode:contextInfo:) contextInfo:connectionController];
}

- (void)showConnectionDidCloseAlertForConnectionController:(GMConnectionController *)connectionController;
{
    NSString *informativeText = NSLocalizedString( @"A connection to the service \"%1$@\" could not be opened: %2$@", @"connection error %1=service name, %2=error text" );
    NSString *connectionName = [[connectionController connection] name];
    NSError *connectionError = [[connectionController connection] error];
	
	NSString *recoverySuggestion = [NSString localizedStringWithFormat:informativeText, connectionName, connectionError];
    
	NSDictionary *userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
		NSLocalizedString(@"Connection Failure", @"connection failure error"), NSLocalizedDescriptionKey,
		recoverySuggestion, NSLocalizedRecoverySuggestionErrorKey,
		nil];
	
	NSError *error = [NSError errorWithDomain:@"" code:0 userInfo:userInfo];
	
	NSAlert *alert = [NSAlert alertWithError:error];
	
	if ([self window] != nil) {
		
		[alert beginSheetModalForWindow:[self window] modalDelegate:self didEndSelector:@selector(connectionDidCloseAlertDidEnd:returnCode:contextInfo:) contextInfo:NULL];
	}
	else {
		
		[self connectionDidCloseAlertDidEnd:alert returnCode:[alert runModal] contextInfo:NULL];
	}
}

- (void)connectSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
{
	[sheet orderOut:self];
	
	if (returnCode == GMMainWindowOKReturnCode) {
		
		// do nothing
	}
	else if (returnCode == GMMainWindowAuthenticationNeededReturnCode) {

		// do nothing
	}
	else {
	
		[NSApp terminate:self];
	}
}

- (void)authenticationNeededSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
{
	[sheet orderOut:self];

	if (returnCode == GMMainWindowOKReturnCode) {

		if (_authenticationMethodTag == GMMainWindowAuthenticationMethodSSOTag) {
			
			XGGSSAuthenticator *authenticator = [[[XGGSSAuthenticator alloc] init] autorelease];
			
			GMConnectionController *connectionController = contextInfo;

			[authenticator setServicePrincipal:[connectionController servicePrincipal]];

			[[connectionController connection] setAuthenticator:authenticator];
			
			[[connectionController connection] open];
		}
		else if (_authenticationMethodTag == GMMainWindowAuthenticationMethodPasswordTag) {
			
			XGTwoWayRandomAuthenticator *authenticator = [[[XGTwoWayRandomAuthenticator alloc] init] autorelease];
			
			[authenticator setUsername:@"one-xgrid-client"];
			[authenticator setPassword:_authenticationPassword];
			
			GMConnectionController *connectionController = contextInfo;
			
			[[connectionController connection] setAuthenticator:authenticator];
			
			[[connectionController connection] open];
		}
	}
}

- (void)connectionDidCloseAlertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo;
{
	[self performSelector:@selector(showConnectSheet) withObject:nil afterDelay:0.0];
}

#pragma mark *** Action methods ***

- (IBAction)toggleAutomaticTour:(id)sender;
{
	if (_doingTour == NO) {
	
		_doingTour = YES;
	
		[self setValue:@"Stop" forKey:@"toggleAutomaticTourTitle"];
		[self setValue:[NSNumber numberWithBool:NO] forKey:@"canShowImage"];
		
		[self performTour];
	}
	else {
	
		_doingTour = NO;
	
		[self setValue:@"Start" forKey:@"toggleAutomaticTourTitle"];
		
		if ([self outstandingJobCount] > 0) {
		
			[self setValue:[NSNumber numberWithBool:NO] forKey:@"canToggleAutomaticTour"];
		}
	}
}

- (IBAction)showImage:(id)sender;
{
    if ([[(NSView *)sender window] makeFirstResponder:nil] == NO) {
	
        NSBeep();
        return;
    } 
    
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"canShowImage"];
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"canToggleAutomaticTour"];

	while (([_settingsArray count] - 1) > _currentSettingsIndex) {
	
		[_settingsArray removeLastObject];
	}
	
	[_settingsArray addObject:[self nextImageSettings]];
	
	NSNumber *newSettingsIndex = [NSNumber numberWithInt:_currentSettingsIndex + 1];

	[self setValue:newSettingsIndex forKey:@"currentSettingsIndex"];
	
	NSDictionary *currentSettings = [self currentSettings];
	
	[self showImageWithSettings:currentSettings];
	
	[self setNextImageSettings:currentSettings];
}

- (IBAction)backImage:(id)sender;
{
	if (_currentSettingsIndex > 0) {
		
		[self setValue:[NSNumber numberWithBool:NO] forKey:@"canShowImage"];
		[self setValue:[NSNumber numberWithBool:NO] forKey:@"canToggleAutomaticTour"];

		NSDictionary *previousSettings = [self currentSettings];
		
		NSNumber *newSettingsIndex = [NSNumber numberWithInt:_currentSettingsIndex - 1];
		
		[self setValue:newSettingsIndex forKey:@"currentSettingsIndex"];
		
		[self showImageWithSettings:[self currentSettings]];
		
		[self setNextImageSettings:previousSettings];
	}
}

- (IBAction)connectSheetOK:(id)sender;
{
	if ([_connectSheet makeFirstResponder:nil] == NO) {
		
		NSBeep();
		return;
	}
	
	[self setValue:[NSNumber numberWithBool:YES] forKey:@"isConnecting"];
	[self setValue:@"Connecting..." forKey:@"connectMessage"];
		
	NSNetService *netService = [[[NSApp delegate] serviceBrowser] netServiceWithName:_connectName];
	
	GMConnectionController *connectionController = nil;
	
	if (netService != nil) {
		
		connectionController = [[[GMConnectionController alloc] initWithNetServiceDomain:[netService domain] type:[netService type] name:[netService name] authenticator:nil] autorelease];
	}
	else {
		
		connectionController = [[[GMConnectionController alloc] initWithHostname:_connectName portnumber:0 authenticator:nil] autorelease];
	}
	
	if (connectionController != nil) {
		
		[[NSApp delegate] setConnectionController:connectionController];
		
		[[connectionController connection] open];
	}
}

- (IBAction)connectSheetCancel:(id)sender;
{
	if (_isConnecting == YES) {
	
		[[[[NSApp delegate] connectionController] connection] close];
		[[NSApp delegate] setConnectionController:nil];
		
		[self connectionWasCanceled];
	}
	else {
	
		[NSApp endSheet:_connectSheet returnCode:GMMainWindowCancelReturnCode];
	}
}

- (IBAction)authenticationNeededSheetOK:(id)sender;
{
	[_authenticationNeededSheet makeFirstResponder:nil];
	
	[NSApp endSheet:_authenticationNeededSheet returnCode:GMMainWindowOKReturnCode];
}

- (IBAction)authenticationNeededSheetCancel:(id)sender;
{
	[NSApp endSheet:_authenticationNeededSheet returnCode:GMMainWindowCancelReturnCode];
}

#pragma mark *** Notification handlers ***

- (void)updateSettingsFromUserDrag:(NSNotification *)aNotification;
{
	NSRect draggedRect = [_imageView draggedRect];
	
    double newX = draggedRect.origin.x;
    double newY = draggedRect.origin.y;
    double newWidth = draggedRect.size.width;
	
	// Assume the selection (and image) is square
	double newHeight = newWidth;
	double currentImageHeight = _currentImageWidth;
	
	double newXRatio = newX / _pixelsWide;
	double newYRatio = 1 - ((newY + newHeight) / _pixelsHigh);
    double newWidthRatio = newWidth / _pixelsWide;
	
	NSNumber *nextImageX;
	NSNumber *nextImageY;
	NSNumber *nextImageWidth;

	if (newWidthRatio != 0) {
	
		nextImageX = [NSNumber numberWithDouble:(_currentImageX + newXRatio * _currentImageWidth)];
		nextImageY = [NSNumber numberWithDouble:(_currentImageY + newYRatio * currentImageHeight)];
		nextImageWidth = [NSNumber numberWithDouble:(newWidthRatio * _currentImageWidth)];
	}
	else {
	
		nextImageX = [self valueForKey:@"currentImageX"];
		nextImageY = [self valueForKey:@"currentImageY"];
		nextImageWidth = [self valueForKey:@"currentImageWidth"];
	}

	[self setValue:nextImageX forKey:@"nextImageX"];
	[self setValue:nextImageY forKey:@"nextImageY"];
	[self setValue:nextImageWidth forKey:@"nextImageWidth"];
}

#pragma mark *** Key Value Observing ***

- (void)actionMonitorOutcomeDidChange:(XGActionMonitor *)actionMonitor;
{
	XGResourceAction action = [actionMonitor action];
	
	[self removeSubmissionMonitor:actionMonitor];

	if (action == XGResourceActionSubmitJob) {
	
		XGActionMonitorOutcome outcome = [actionMonitor outcome];
		
		if (outcome == XGActionMonitorOutcomeSuccess) {
		
			NSString *jobIdentifier = [[actionMonitor results] objectForKey:@"jobIdentifier"];
			
			XGGrid *grid = [self grid];
			
			if (grid == nil) {
			
				NSLog(@"grid unexpectedly nil");
				return;
			}
			
			XGJob *job = [grid jobForIdentifier:jobIdentifier];
			
			if (job != nil) {
				
				[self addJob:job];
			}
			else {
				
				// the job hasn't been added to the grid yet
				// we'll need to wait until the job is added
				
				[self addJobIdentifier:jobIdentifier];
			}
		}
		else {
		
			NSLog(@"error submitting job");
		}
	}
	else if (action == XGResourceActionGetOutputStreams) {
	
		NSArray *outputStreams = [[actionMonitor results] objectForKey:XGActionMonitorResultsOutputStreamsKey];
		
		if ([outputStreams count] > 0) {
		
			XGFile *standardOutput = [outputStreams objectAtIndex:0];
			
			[self getStandardOutput:standardOutput];
		}
	}
}

- (void)jobStateDidChange:(XGJob *)job;
{
	XGResourceState state = [job state];
	
	if (state == XGResourceStateFinished) {
	
		XGActionMonitor *getOutputStreamsMonitor = [job performGetOutputStreamsAction];
		
		[self addGetOutputStreamsMonitor:getOutputStreamsMonitor];
	}
	else if (state == XGResourceStateFailed) {
	
		NSLog(@"job failed");

		[self removeJob:job];
	}
	else if (state == XGResourceStateCanceled) {
	
		NSLog(@"job canceled");
	
		[self removeJob:job];
	}
	else return;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context;
{
	// we observe the outcome of action monitors and the state of jobs
	
	if ([object isKindOfClass:[XGActionMonitor self]] == YES) {
	
		XGActionMonitor *actionMonitor = object;
		
		if ([keyPath isEqualToString:@"outcome"] == YES) {
		
			[self actionMonitorOutcomeDidChange:actionMonitor];
		}
	}
	else if ([object isKindOfClass:[XGJob self]] == YES) {
	
		XGJob *job = object;
		
		if ([keyPath isEqualToString:@"state"] == YES) {

			[self jobStateDidChange:job];
		}
	} 
	else if ([object isKindOfClass:[XGGrid self]] == YES) {
		
		XGGrid *grid = object;
		
		if ([keyPath isEqualToString:@"jobs"] == YES) {
			
			NSEnumerator *jobIdentifierEnumerator = [_jobIdentifiers objectEnumerator];
			NSString *jobIdentifier;
			
			while (jobIdentifier = [jobIdentifierEnumerator nextObject]) {
								
				XGJob *job = [grid jobForIdentifier:jobIdentifier];
				
				if (job != nil) {
					
					[self addJob:job];
					
					[self removeJobIdentifier:jobIdentifier];
				}
			}
		}
	}
}

#pragma mark *** XGFileDownload delegate methods ***

- (void)fileDownload:(XGFileDownload *)fileDownload didReceiveData:(NSData *)data;
{
	[self writeImageFromData:data];
}

- (void)fileDownload:(XGFileDownload *)fileDownload didFailWithError:(NSError *)error;
{
	[fileDownload autorelease];
}

- (void)fileDownloadDidFinish:(XGFileDownload *)fileDownload;
{
	[self removeJob:[[fileDownload file] job]];
	
	[fileDownload autorelease];
}

@end

