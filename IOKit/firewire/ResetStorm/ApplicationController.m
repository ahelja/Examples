/*
	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.

	Written by: cpieper
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	$Log: ApplicationController.m,v $
	Revision 1.3  2005/02/28 23:02:47  firewire
	*** empty log message ***
	
	Revision 1.2  2005/02/04 19:33:23  firewire
	*** empty log message ***
	
	Revision 1.1.1.1  2003/05/27 18:07:40  firewire
	first check-in
	
*/

#import "ApplicationController.h"
#import <mach/mach.h>

typedef struct
{
	float	min;
	float	max;
} FloatRange;

typedef struct
{
	UInt32	min;
	UInt32	max;
} UInt32Range;

typedef struct
{
	UInt32Range	count;
	FloatRange	interval;
} StormState;

typedef struct
{
	StormState	calm;
	StormState	gale;
} StormRange;

static StormRange sStormRanges[5] = { 
										{ 
											{
												{5, 10}, 			// calm count
												{5.0, 60.0}			// calm interval
											},
											{
												{1, 2},				// gale count
												{0.000001, 0.25}	// gale interval
											}
										},
									  	{ 
											{
												{5, 10}, 			// calm count
												{5.0, 15.0}			// calm interval
											},
											{
												{2,10},				// gale count
												{0.000001, 0.25}	// gale interval
											}
										},
										{ 
											{
												{5, 10}, 			// calm count
												{1.0, 5.0}			// calm interval
											},
											{
												{5, 50},			// gale count
												{0.000001, 0.25}	// gale interval
											}
										},
										{ 
											{
												{1, 5}, 			// calm count
												{1.0, 4.0}			// calm interval
											},
											{
												{30, 100},			// gale count
												{0.000001, 0.25}	// gale interval
											}
										},
										{ 
											{
												{1,5}, 			// calm count
												{1.0, 2.0}			// calm interval
											},
											{
												{30, 100},			// gale count
												{0.000001, 0.25}	// gale interval
											}
										}
									};

									
@implementation ApplicationController

// init
//
//

- (id)init
{
	IOReturn status = kIOReturnSuccess;

	[super init];
	
	fTimer = NULL;
	fStormState = kResetStormIdle;
	fStormSetting = 2;
	fLowLevelResets = true;
	
	if( status == kIOReturnSuccess )
	{
		fFWDeviceInterface = [self createFireWireInterfaceForLocalNode];
		if( fFWDeviceInterface == NULL )
			status = kIOReturnError;
	}
	
	if( status == kIOReturnSuccess )
	{
		fFireWireUserClientService = [self findFireWireUserClient:fFWDeviceInterface];
		if( fFireWireUserClientService == 0 )
			status = kIOReturnError;
	}
	
	if( status == kIOReturnSuccess )
	{
		[self setLowLevelResets:fLowLevelResets];
	}
	
	if( status == kIOReturnSuccess )
	{
		srandom((unsigned)[[NSDate date] timeIntervalSinceReferenceDate]);
	}
	
	return self;
}

// applicationWillTerminate
//
//

- (void)applicationWillTerminate:(NSNotification *)notification 
{
	if( fFWDeviceInterface != NULL )
	{
		[self stopTimer];
		[self destroyFireWireInterfaceForLocalNode:fFWDeviceInterface];
		fFWDeviceInterface = NULL;
	}
}

// createFireWireInterfaceForLocalNode
//
//

- (IOFireWireDeviceInterface **)createFireWireInterfaceForLocalNode
{
	IOReturn status = kIOReturnSuccess;
	
	CFDictionaryRef			matchDictionary = NULL;
	mach_port_t				masterPort = 0;
	io_iterator_t			iterator = 0;
	io_object_t				service = 0;
	
	IOCFPlugInInterface **			cfPlugInInterface = NULL;
	IOFireWireDeviceInterface **	fwDeviceInterface = NULL;
	
	// allocate an IOKit master mach port
	status = IOMasterPort( bootstrap_port, &masterPort );

	// create a matching dictionary that matches on the local node
	if( status == kIOReturnSuccess )
	{
		matchDictionary = IOServiceMatching( "IOFireWireLocalNode" );
		if( matchDictionary == NULL )
			status = kIOReturnError;
	}
	
	// get an iterator of matching service objects
	if( status == kIOReturnSuccess )
	{
		status = IOServiceGetMatchingServices( masterPort, matchDictionary, &iterator );
	}
	
	matchDictionary = NULL;
	
	// get the first service
	if( status == kIOReturnSuccess )
	{
		IOIteratorReset( iterator );
		service = IOIteratorNext( iterator );
	}
	
	// release the iterator
	
	if( iterator )
	{
		IOObjectRelease( iterator );
		iterator = 0 ;
	}
	
	// deallocate the master port
	if( masterPort )
	{
		mach_port_deallocate( mach_task_self(), masterPort );
		masterPort = 0;
	}
	
	// load the FireWireLib plugin on our service
	if( status == kIOReturnSuccess )
	{
		SInt32 score = 0;
		status = IOCreatePlugInInterfaceForService( service, 
													kIOFireWireLibTypeID, 
													kIOCFPlugInInterfaceID,
													&cfPlugInInterface,
													&score );
	}
	
	// get the firewire device interface from the plugin
	if( status == kIOReturnSuccess )
	{
		HRESULT res;
		res = (*cfPlugInInterface)->QueryInterface( cfPlugInInterface,
													CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID),
													(void**)&fwDeviceInterface );
		if( res != S_OK )
			status = kIOReturnError;
	}
	
	if( status == kIOReturnSuccess )
	{
		(*cfPlugInInterface)->Release( cfPlugInInterface );
		cfPlugInInterface = NULL;
	}

	// open the device interface
	if( status == kIOReturnSuccess )
	{
		status = (*fwDeviceInterface)->Open( fwDeviceInterface );
	}

	return fwDeviceInterface;
}

// destroyFireWireInterfaceForLocalNode
//
//

- (void)destroyFireWireInterfaceForLocalNode:(IOFireWireDeviceInterface **)interface
{
	if( interface != NULL )
	{
		(*interface)->Close( interface );
		(*interface)->Release( interface );
	}
}

// findFireWireUserClient
//
//

- (io_service_t)findFireWireUserClient:(IOFireWireDeviceInterface **)interface
{
	IOReturn status = kIOReturnSuccess;
	
	io_iterator_t		iterator = 0;
	io_service_t 		userClientService = 0;
	
	IORegistryEntryGetChildIterator( (*interface)->GetDevice( interface ), kIOServicePlane, &iterator );
	if( iterator == 0 )
		status = kIOReturnError;
	
	if( status == kIOReturnSuccess )
	{
		io_service_t service = IOIteratorNext( iterator );
		
		while( service )
		{
			if( IOObjectConformsTo( service, "IOFireWireUserClient" ) )
			{
				CFNumberRef	cfPID = (CFNumberRef)IORegistryEntryCreateCFProperty
											( service, CFSTR("Owning PID"), kCFAllocatorDefault, 0 );
											
				if( cfPID && CFGetTypeID(cfPID) == CFNumberGetTypeID() )
				{
					int pid;
					Boolean result;
					
					result = CFNumberGetValue( cfPID, kCFNumberSInt32Type, &pid );
					CFRelease(cfPID);
					if( !result )
					{
						continue;
					}
					
					NSLog( @"ApplicationController : findOurFireWireUserClient - pid=%x, getpid()=%x\n", pid, [[NSProcessInfo processInfo] processIdentifier] );
					
					if( pid == [[NSProcessInfo processInfo] processIdentifier] )
					{
						userClientService = service;
						break;
					}
				}
			}
	
			service = IOIteratorNext( iterator );
		}
	}
	
	return userClientService;
}

// validateMenuItem
//
//

- (BOOL)validateMenuItem:(NSMenuItem *)aCell 
{
    SEL action;
    
    action = [aCell action];
    if (action == @selector(toggleLowLevelResets:))
    {
		if( fLowLevelResets )
		{
			[aCell setState:NSOnState];
		}
		else
		{
			[aCell setState:NSOffState];
		}
		
        return YES;
    } 

    return NO;
}

// toggleLowLevelResets
//
//

- (IBAction)toggleLowLevelResets:(id)sender
{	
	fLowLevelResets = !fLowLevelResets;
		
	[self setLowLevelResets:fLowLevelResets];
}

// setLowLevelResets
//
//

- (void)setLowLevelResets:(UInt8)state
{
	IOReturn status = kIOReturnSuccess;
	
	CFNumberRef 			number = NULL;
	CFMutableDictionaryRef 	dictionary = NULL;

	dictionary = CFDictionaryCreateMutable( kCFAllocatorDefault, 1,
											&kCFTypeDictionaryKeyCallBacks, 
											&kCFTypeDictionaryValueCallBacks );
	if( dictionary == NULL )
		status = kIOReturnError;
	
	if( status == kIOReturnSuccess )
	{
		number = CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt8Type, &state );
		if( number == NULL )
			status = kIOReturnError;
	}
	
	if( status == kIOReturnSuccess )
	{
		CFDictionaryAddValue( dictionary, CFSTR("unsafe bus resets"), number );
		status = IORegistryEntrySetCFProperties( fFireWireUserClientService, dictionary );
	}
	
	if( dictionary != NULL )
	{
		CFRelease( dictionary );
		dictionary = NULL;
	}
	
	if( number != NULL )
	{
		CFRelease( number );
		number = NULL;
	}
}

// resetNow
//
//

- (IBAction)resetNow:(id)sender
{
	IOReturn status = kIOReturnSuccess;
	
	status = (*fFWDeviceInterface)->BusReset( fFWDeviceInterface );

	//NSLog( @"ApplicationController::resetNow status = 0x%08lx\n", status );
}

// setFrequency
//
//

- (IBAction)setFrequency:(id)sender
{	
	//NSLog( @"ApplicationController::setFrequency\n" );
	
	fStormSetting = [fFrequencySlider floatValue];
	if( fStormState != kResetStormIdle )
	{
		[self stopTimer];
		[self enterCalmState];
	}
}

// startResetStorm
//
//

- (IBAction)startResetStorm:(id)sender
{	
	if( fFWDeviceInterface != NULL )
	{
		[fStartButton setEnabled:false];
		[fStopButton setEnabled:true];
	
		//NSLog( @"ApplicationController::startResetStorm\n" );
		
		[self enterCalmState];
	}

}

// stopResetStorm
//
//

- (IBAction)stopResetStorm:(id)sender
{
	if( fFWDeviceInterface != NULL )
	{
		[self stopTimer];
		fStormState = kResetStormIdle;
		[fStartButton setEnabled:true];
		[fStopButton setEnabled:false];
	}
}

- (void)enterCalmState
{
	UInt32Range	count;
	UInt32		range;
	float		randomFloat;
	
	count = sStormRanges[fStormSetting].calm.count;
	range = count.max - count.min;
	randomFloat = (float)random() / RAND_MAX;
	
	fResetCount = count.min + (UInt32)(randomFloat * range);
	fStormState = kResetStormStateCalm;
	
	[self scheduleReset];
}


- (void)enterGaleState
{
	UInt32Range	count;
	UInt32		range;
	float		randomFloat;
	
	count = sStormRanges[fStormSetting].gale.count;
	range = count.max - count.min;
	randomFloat = (float)random() / RAND_MAX;
	
	fResetCount = count.min + (UInt32)(randomFloat * range);
	fStormState = kResetStormStateGale;
	
	[self scheduleReset];
}

- (void)scheduleReset
{
	FloatRange	interval = { 0.0, 0.0 } ;
	float		range;
	float		randomFloat;
	float		duration;
	
	switch( fStormState )
	{
		case kResetStormStateCalm:
			interval = sStormRanges[fStormSetting].calm.interval;
			break;
		
		case kResetStormStateGale:
			interval = sStormRanges[fStormSetting].gale.interval;
			break;
			
		default:
			//NSLog( @"scheduleReset : in odd storm state = 0x%08lx\n", fStormState );
			break;
	}
	
	range = interval.max - interval.min;
	randomFloat = (float)random() / RAND_MAX;
	
	duration = interval.min + randomFloat * range;

#if 0
	switch( fStormState )
	{
		case kResetStormStateCalm:
			NSLog( @"scheduleReset : kResetStormStateCalm - duration = %f\n", duration );
			break;
		
		case kResetStormStateGale:
			NSLog( @"scheduleReset : kResetStormStateGale - duration = %f\n", duration );
			break;
		
		default:
			break;
	}
#endif

	[self startTimer:duration];
	
}

// timerIntervalExpired
//
//

- (IBAction)timerIntervalExpired:(id)sender
{	
	IOReturn status = kIOReturnSuccess;
		
#if 0
	switch( fStormState )
	{
		case kResetStormStateCalm:
			NSLog( @"timerIntervalExpired : kResetStormStateCalm - BUS RESET #%d\n", fResetCount );	
			break;
		
		case kResetStormStateGale:
			NSLog( @"timerIntervalExpired : kResetStormStateGale - BUS RESET #%d\n", fResetCount );
			break;
	
		default:
			break;
	}
#endif

	status = (*fFWDeviceInterface)->BusReset( fFWDeviceInterface );
	fResetCount--;
	
	if( fResetCount == 0 )
	{
		switch( fStormState )
		{
			case kResetStormStateCalm:
				[self enterGaleState];
				break;
			
			case kResetStormStateGale:
				[self enterCalmState];
				break;
				
			default:
				// NSLog( @"timerIntervalExpired : in odd storm state = 0x%08lx\n", fStormState );
				break;
		}
	}
	else
	{
		[self scheduleReset];
	}
}

// startTimer
//
//

- (void)startTimer:(NSTimeInterval)time
{
	fTimer = [NSTimer scheduledTimerWithTimeInterval:time target:self
				selector:@selector(timerIntervalExpired:) userInfo:nil repeats:NO];
}

// stopTimer
//
//

- (void)stopTimer
{
	//NSLog( @"ApplicationController::stopResetStorm\n" );

	if( fTimer )
	{
		[fTimer invalidate];
		fTimer = NULL;
	}
}

@end
