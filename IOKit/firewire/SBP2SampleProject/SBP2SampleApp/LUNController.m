/*
	File:		LUNController.m

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.

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
*/

#import "LUNController.h"
#import <objc/objc-runtime.h>

void sendTURCompletionRoutine( void * refCon, IOReturn status )
{
	FWLOG(( "LUNController : sendTURCompletionRoutine\n" ));
	
	objc_msgSend( refCon, @selector(TURComplete:), status );
		
}

@implementation LUNController

//
// init / dealloc
//

- (id)initWithLUNReference:(io_object_t)lun
{	
	io_name_t 				className;
	char					cstr[100];
	IOReturn				status = kIOReturnSuccess;
	CFMutableDictionaryRef	serviceProps = NULL;
	UInt32					commandSet, commandSetSpecID;
	
	fDriverInterface = NULL;
	fCFPlugInInterface = NULL;
	
	fInstantiated = false;
	fLUNReference = lun;
	
	//
	// get some values from the name registry and generate a name string
	//
	
	status = IOObjectGetClass( lun, className );
	if( status == kIOReturnSuccess )
	{			
		status = IORegistryEntryCreateCFProperties( lun, &serviceProps,
													kCFAllocatorDefault, kNilOptions );
	}
	 
	if( status == kIOReturnSuccess )
	{
	    CFTypeRef val = CFDictionaryGetValue( serviceProps, CFSTR("Command_Set") );		
		if (val && CFGetTypeID(val) == CFNumberGetTypeID()) 
		{
			CFNumberGetValue( (CFNumberRef)val, kCFNumberSInt32Type, &commandSet );
        }
		printf( "Command_Set = %ld\n", commandSet );
	}
	
	if( status == kIOReturnSuccess )
	{
	    CFTypeRef val = CFDictionaryGetValue( serviceProps, CFSTR("Command_Set_Spec_ID") );		
		if (val && CFGetTypeID(val) == CFNumberGetTypeID()) 
		{
			CFNumberGetValue( (CFNumberRef)val, kCFNumberSInt32Type, &commandSetSpecID );
        }
		printf( "Command_Set_Spec_ID = %ld\n", commandSetSpecID );
	}
	
	if( serviceProps != NULL ) 	
		CFRelease( serviceProps );
	
	// create name string
	sprintf( cstr, "%s <0x%lx, 0x%lx>", className, commandSet, commandSetSpecID );
	fName = [NSString stringWithCString:cstr];

	return self;
}

- (void)dealloc
{
	FWLOG(( "LUNController : dealloc\n" ));
	
	[fName release];
	
	[super dealloc];
}

/////////////////////////////////////////////////////

//
// accessors
//

- (io_object_t)getLUNReference
{
	return fLUNReference;
}

- (NSString*)getName
{
	return fName;
}

/////////////////////////////////////////////////////

//
// instantiateLUNConnection
//
// load plugin and get important interfaces
//

- (void)instantiateLUNConnection
{

	IOReturn				status = kIOReturnSuccess;
	
	SInt32 					score;

	if( !fInstantiated )
	{
		fInstantiated = true;
		
		//
		// load GUI (opens a new window)
		//
				
		if( ![NSBundle loadNibNamed:@"LUNWindow" owner:self] )
		{
			FWLOG(( "LUNController : Failed to load LUNWindow.nib\n" ));
			status = kIOReturnError;
		}
		
		if( status == kIOReturnSuccess )
		{
			[fWindow setTitle:fName];
			fGlobalLock = [[NSLock alloc] init];
		}

		//
		// load the plugin and get the CFPlugin interface
		//
			
		if( status == kIOReturnSuccess )
		{
			status = IOCreatePlugInInterfaceForService( fLUNReference,
														kSBP2SampleDriverTypeID, 
														kIOCFPlugInInterfaceID,
														&fCFPlugInInterface,
														&score );	// calls Start method
			FWLOG(( "LUNController : IOCreatePlugInInterfaceForService status = 0x%08x\n", status ));
		}
		
		//
		// get the sample driver interface from the CFPlugin interface
		//
		
		if( status == kIOReturnSuccess )
		{
			HRESULT res;
			res = (*fCFPlugInInterface)->QueryInterface( fCFPlugInInterface, 
											CFUUIDGetUUIDBytes(kSBP2SampleDriverInterfaceID),
											(LPVOID) &fDriverInterface );
			
			if( res != S_OK )
				status = kIOReturnError;
		}

		//
		// attach callbacks to the current runloop
		//
		
		if( status == kIOReturnSuccess )
		{
			status = (*fDriverInterface)->setUpCallbacksWithRunLoop( fDriverInterface,
																	 CFRunLoopGetCurrent() );
		}
	}
	
	[fWindow makeKeyAndOrderFront:self];
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	FWLOG(( "LUNController : windowWillClose\n" ));
	fInstantiated = false;

	//
	// release driver interface
	//
	
	if( fDriverInterface != NULL )
	{

#if 1	
		//zzz the sample driver should do this on close.
		// when we do this here we send a logout to the device and then deallocate the driver
		// before it completes.  that's bad, although SBP2 can handle it.
		
		if( [fLogoutButton isEnabled] )
		{
			(*fDriverInterface)->logoutOfDevice(fDriverInterface);
		}
#endif
		
		(*fDriverInterface)->Release(fDriverInterface);
		fDriverInterface = NULL;
	}
	
	//	
	// release plugin interface
	//
	
	if( fCFPlugInInterface != NULL )
	{	
		IOReturn status = IODestroyPlugInInterface(fCFPlugInInterface); 		// calls Stop method
		FWLOG(( "LUNController : IODestroyPlugInInterface status = 0x%08x\n", status ));
		fCFPlugInInterface = NULL;
	}
	
	if( fGlobalLock != NULL )
	{
		[fGlobalLock release];
		fGlobalLock = NULL;
	}
}

/////////////////////////////////////////////////////

//
// login / logout of device
//
// called when user presses the appropriate button in the GUI
//

- (void)loginToDevice:(id)sender
{
	if( fDriverInterface != NULL )
	{
		(*fDriverInterface)->loginToDevice(fDriverInterface);
		[fLoginButton setEnabled:false];
		[fReadButton setEnabled:true];
		[fWriteButton setEnabled:true];
		[fWRCmpButton setEnabled:true];
		[fLogoutButton setEnabled:true];
	}
}

- (void)logoutOfDevice:(id)sender
{
	if( fDriverInterface != NULL )
	{
		(*fDriverInterface)->logoutOfDevice(fDriverInterface);
		[fLogoutButton setEnabled:false];
		[fReadButton setEnabled:false];
		[fWriteButton setEnabled:false];
		[fWRCmpButton setEnabled:false];
		[fLoginButton setEnabled:true];
	}
}

//
// performWRCompare
//
// Write / Read from device
//
- (void)wrcLoop:(id)sender
{
	FWLOG(( "LUNController : WRC Loop clicked \n"));
	if(fLoopTheWRC == true)
	{
		fLoopTheWRC = false;
	}
	else
	{
		fLoopTheWRC = true;
	}
}

- (void)performWRCmp:(id)sender
{
	if( fDriverInterface != NULL )
	{
		[fWRCmpButton setEnabled:false];
		[fLogoutButton setEnabled:false];
	
		[NSThread detachNewThreadSelector:@selector(runWRCmpTransaction:) toTarget:self withObject:nil];
	}
}

- (void)runWRCmpTransaction:(id)object
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	do
	{
		fWRCmpWorkingThreadCountLock = [[NSConditionLock alloc] initWithCondition:1];
		
		// run worker thread
		//zzz someday we will spawn multiple worker threads
		[NSThread detachNewThreadSelector:@selector(runWRCmpWorker:) toTarget:self withObject:nil];
		
		// block until condition == 0
		[fWRCmpWorkingThreadCountLock lockWhenCondition:0];
		[fWRCmpWorkingThreadCountLock unlock];
		[fWRCmpWorkingThreadCountLock release];
		if(fCancel == true)
		{
			break;
		}
	}while(fLoopTheWRC);
	
	// finish
	[fWRCmpButton setEnabled:true];
	[fLogoutButton setEnabled:true];

	[pool release];
}

- (void)runWRCmpWorker:(id)object
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSMutableString * readString;
	UInt8 wBlock[512];
	UInt8 rBlock[512];
	char  errString[108];
	UInt32 transactionID = 1; // planning to use this for cancelling
	UInt32 wrcStartBlocks = 0; UInt32 wrcEndBlocks = 0;
	UInt32 i = 0;
	IOReturn status;
	struct timeval t1, t2;	
	float t=0;
	
	wrcStartBlocks = [fWRCmpStartBlocks floatValue];
	wrcEndBlocks = [fWRCmpEndBlocks floatValue];
	
	if(wrcEndBlocks < wrcStartBlocks)
	{
		i = wrcStartBlocks;
		wrcStartBlocks = wrcEndBlocks;
		wrcEndBlocks = i;
	}
	
	FWLOG(( "LUNController : Total blocks to write = 0x%08lx\n", wrcStartBlocks - wrcEndBlocks));

	fCancel = false;
	[fWRCmpProgressIndicator setIndeterminate:FALSE];
	[fWRCmpProgressIndicator setMinValue:wrcStartBlocks];
	[fWRCmpProgressIndicator setMaxValue:wrcEndBlocks];
	[fWRCmpProgressIndicator setDoubleValue:wrcStartBlocks];
	[fWRCmpProgressIndicator displayIfNeeded];

	bzero(&wBlock, 512); 
	
	sprintf(errString, "WRC Block start ...");
	readString = [NSString stringWithCString:errString];
	[self logString:readString];

	gettimeofday(&t1, NULL);
	for(i=wrcStartBlocks;i<=wrcEndBlocks;i++)
	{
		// fill the buffer
		[self fillBuf:512:i:wBlock];
		
		// write block
		status = (*fDriverInterface)->writeBlock( fDriverInterface, transactionID, i, &wBlock );

		if(status != kIOReturnSuccess)
		{
			sprintf(errString, "Error: WRC to Block %ld\n", i);
			readString = [NSString stringWithCString:errString];
			[self logString:readString];
		}		
		
		bzero(&rBlock, 512);	
		// read block
		status = (*fDriverInterface)->readBlock( fDriverInterface, transactionID, i, &rBlock );
		if(status != kIOReturnSuccess)
		{
			sprintf(errString, "Error: WRC from Block %ld\n", i);
			readString = [NSString stringWithCString:errString];
			[self logString:readString];
		}			

		[fWRCmpProgressIndicator incrementBy: 1];
		[fWRCmpProgressIndicator displayIfNeeded];

		// compare, if error display and continue
		if(memcmp(wBlock, rBlock, 512) != 0)
		{
			sprintf(errString, "Error: WRC Block %ld\n", i);
			readString = [NSString stringWithCString:errString];
			[self logString:readString];
		}
		
		if(fCancel == true)
		{
			FWLOG(( "LUNController : Write Read Compare blocks cancelled \n"));
			break;
		}
	}
	
	gettimeofday(&t2, NULL);
	t = [self elapsed:&t1:&t2];
	sprintf(errString, " ... %f seconds to finish WRC\n", t);
	readString = [NSString stringWithCString:errString];
	[self logString:readString];

	[fWRCmpProgressIndicator setDoubleValue:wrcEndBlocks];
	[fWRCmpProgressIndicator displayIfNeeded];
		
	[fWRCmpWorkingThreadCountLock lock];
	[fWRCmpWorkingThreadCountLock unlockWithCondition:0];
	
	[pool release];
}

- (void)fillBuf:(UInt32)size:(UInt32)stuff:(UInt8*)buf;
{
	UInt32 slots = 0;
	UInt32 i;
	
	slots = size/4;
	
	for(i = 0; i <= slots-1; i++)
		*(UInt32*)&buf[i*4] = stuff;
}

//
// performWrite
//
// Write to device
//

- (void)writeLoop:(id)sender
{
	FWLOG(( "LUNController : Write Loop clicked \n"));
	if(fLoopTheWrite == TRUE)
	{
		fLoopTheWrite = FALSE;
	}
	else
	{
		fLoopTheWrite = TRUE;
	}
}

- (void)performWrite:(id)sender
{
	if( fDriverInterface != NULL )
	{
		[fWriteButton setEnabled:false];
		[fLogoutButton setEnabled:false];
	
		[NSThread detachNewThreadSelector:@selector(runWriteTransaction:) toTarget:self withObject:nil];
	}
}

- (void)runWriteTransaction:(id)object
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	do 
	{
		fWriteWorkingThreadCountLock = [[NSConditionLock alloc] initWithCondition:1];
		
		// run worker thread
		//zzz someday we will spawn multiple worker threads
		[NSThread detachNewThreadSelector:@selector(runWriteWorker:) toTarget:self withObject:nil];
		
		// block until condition == 0
		[fWriteWorkingThreadCountLock lockWhenCondition:0];
		[fWriteWorkingThreadCountLock unlock];
		[fWriteWorkingThreadCountLock release];

		if(fCancel == true)
		{
			break;
		}
	}while(fLoopTheWrite);
	
	// finish
	[fWriteButton setEnabled:true];
	[fLogoutButton setEnabled:true];

	[pool release];
}

- (void)runWriteWorker:(id)object
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSMutableString * readString;
	char  errString[108];
	UInt8 block[512];
	UInt32 transactionID = 1; // planning to use this for cancelling
	UInt32 writeStartBlocks = 0; UInt32 writeEndBlocks = 0;
	UInt32 i = 0;
	struct timeval t1, t2;	
	float t=0;
	IOReturn status = 0;
	
	writeStartBlocks = [fWriteStartBlocks floatValue];
	writeEndBlocks = [fWriteEndBlocks floatValue];
	
	if(writeEndBlocks < writeStartBlocks)
	{
		i = writeStartBlocks;
		writeStartBlocks = writeEndBlocks;
		writeEndBlocks = i;
	}
	
	FWLOG(( "LUNController : Total blocks to write = 0x%08lx\n", writeStartBlocks - writeEndBlocks));

	fCancel = false;
	[fWriteProgressIndicator setIndeterminate:FALSE];
	[fWriteProgressIndicator setMinValue:writeStartBlocks];
	[fWriteProgressIndicator setMaxValue:writeEndBlocks];
	[fWriteProgressIndicator setDoubleValue:writeStartBlocks];
	[fWriteProgressIndicator displayIfNeeded];

	bzero(&block, 512);

	sprintf(errString, "Write Block start ...");
	readString = [NSString stringWithCString:errString];
	[self logString:readString];

	gettimeofday(&t1, NULL);

	for(i=writeStartBlocks;i<=writeEndBlocks;i++)
	{
		// fill the buffer
		[self fillBuf:512:i:block];
	
		// write block
		status = (*fDriverInterface)->writeBlock( fDriverInterface, transactionID, i, &block );
		if(status != kIOReturnSuccess)
		{
			sprintf(errString, "Error: Write to Block %ld\n", i);
			readString = [NSString stringWithCString:errString];
			[self logString:readString];
		}		

		[fWriteProgressIndicator incrementBy: 1];
		[fWriteProgressIndicator displayIfNeeded];
		
		if(fCancel == true)
		{
			FWLOG(( "LUNController : Write blocks cancelled \n"));
			break;
		}
		//[readString release];
	}
	gettimeofday(&t2, NULL);
	t = [self elapsed:&t1:&t2];
	sprintf(errString, " ... %f seconds to finish Write\n", t);
	readString = [NSString stringWithCString:errString];
	[self logString:readString];
	
	[fWriteProgressIndicator setDoubleValue:writeEndBlocks];
	[fWriteProgressIndicator displayIfNeeded];

	[fWriteWorkingThreadCountLock lock];
	[fWriteWorkingThreadCountLock unlockWithCondition:0];
	
	[pool release];
}

//
// performRead
//
// read from a device
//

- (void)performRead:(id)sender
{
	if( fDriverInterface != NULL )
	{
		[fReadButton setEnabled:false];
		[fLogoutButton setEnabled:false];
	
		[NSThread detachNewThreadSelector:@selector(runReadTransaction:) toTarget:self withObject:nil];
	}
}

- (void)readLoop:(id)sender
{
	FWLOG(( "LUNController : Read Loop clicked \n"));
	if(fLoopTheRead == TRUE)
	{
		fLoopTheRead = FALSE;
	}
	else
	{
		fLoopTheRead = TRUE;
	}
}

- (void)runReadTransaction:(id)object
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
	do
	{
		fReadWorkingThreadCountLock = [[NSConditionLock alloc] initWithCondition:1];
	
		// run worker thread
		//zzz someday we will spawn multiple worker threads
		[NSThread detachNewThreadSelector:@selector(runReadWorker:) toTarget:self withObject:nil];
		
		// block until condition == 0
		[fReadWorkingThreadCountLock lockWhenCondition:0];
		[fReadWorkingThreadCountLock unlock];
		[fReadWorkingThreadCountLock release];
		
		if(fCancel == true)
		{
			break;
		}
	}while(fLoopTheRead);
	
	// finish
	[fReadButton setEnabled:true];
	[fLogoutButton setEnabled:true];

	[pool release];
}

- (void)runReadWorker:(id)object
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSMutableString * readString;
	char  errString[108];
	UInt8 block[512];
	UInt32 transactionID = 1; // planning to use this for cancelling
	UInt32 readStartBlocks = 0; UInt32 readEndBlocks = 0;
	UInt32 i = 0;
	struct timeval t1, t2;	
	float t=0;
	IOReturn status = 0;
	
	readStartBlocks = [fReadStartBlocks floatValue];
	readEndBlocks = [fReadEndBlocks floatValue];
		
	if(readEndBlocks < readStartBlocks)
	{
		i = readStartBlocks;
		readStartBlocks = readEndBlocks;
		readEndBlocks = i;
	}
	
	FWLOG(( "LUNController : Total blocks to read = 0x%08lx\n", readEndBlocks - readStartBlocks));

	fCancel = false;
	[fReadProgressIndicator setIndeterminate:FALSE];
	[fReadProgressIndicator setMinValue:readStartBlocks];
	[fReadProgressIndicator setMaxValue:readEndBlocks];
	[fReadProgressIndicator setDoubleValue:readStartBlocks];
	[fReadProgressIndicator displayIfNeeded];

	sprintf(errString, "Read Block start ...");
	readString = [NSString stringWithCString:errString];
	[self logString:readString];

	gettimeofday(&t1, NULL);

	for(i=readStartBlocks;i<=readEndBlocks;i++)
	{
		// read block 0
		status = (*fDriverInterface)->readBlock( fDriverInterface, transactionID, i, &block );
		if(status != kIOReturnSuccess)
		{
			sprintf(errString, "Error: Read from Block %ld\n", i);
			readString = [NSString stringWithCString:errString];
			[self logString:readString];
		}		
		
		[fReadProgressIndicator incrementBy: 1];
		[fReadProgressIndicator displayIfNeeded];
		
		if(fCancel == true)
		{
			FWLOG(( "LUNController : read blocks cancelled \n"));
			break;
		}
		//[readString release];
	}

	gettimeofday(&t2, NULL);
	t = [self elapsed:&t1:&t2];
	sprintf(errString, " ... %f seconds to finish Read\n", t);
	readString = [NSString stringWithCString:errString];
	[self logString:readString];

	[fReadProgressIndicator setDoubleValue:readEndBlocks];
	[fReadProgressIndicator displayIfNeeded];

	[fReadWorkingThreadCountLock lock];
	[fReadWorkingThreadCountLock unlockWithCondition:0];
	
	[pool release];
}

- (void)cancel:(id)sender
{
	FWLOG(( "LUNController : Cancel clicked \n"));
	// not supported yet
	fCancel = true;
}

- (float)elapsed:(struct timeval*)t1:(struct timeval*)t2
{
	float t;
	// locks aren't necessary until this goes mutli threaded
	[fGlobalLock lock];

	if (t2->tv_usec < t1->tv_usec) 
	{
		t2->tv_usec	+= 1000000;
		t2->tv_sec	-= 1;
	}
	t = (float)(t2->tv_sec - t1->tv_sec) + (float)(t2->tv_usec - t1->tv_usec)/(float)1000000.0;
	
	[fGlobalLock unlock];
	
	return t;
}

- (void)logString:(NSString *)string
{
	NSRange endMarker;
	
	// locks aren't necessary until this goes mutli threaded
	[fGlobalLock lock];
	
	endMarker = NSMakeRange([[fTextView string] length], 0);

	// add line to the end of the text
	[fTextView setSelectedRange: endMarker];
	[fTextView setEditable: YES];
	[fTextView insertText: string];
	[fTextView setEditable: NO];

	// scroll to the end 
	endMarker.location += [string length];
	[fTextView scrollRangeToVisible: endMarker];
	
	[fGlobalLock unlock];
}

@end
