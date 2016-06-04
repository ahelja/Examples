/*
	File:		SelectorController.m

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

#import "SelectorController.h"
#import "LUNController.h"
#import <mach/mach.h>

@implementation SelectorController

- (id)init
{
    mach_port_t 	masterPort = 0;
    CFDictionaryRef matchDictionary = 0;
	io_iterator_t 	iterator = 0 ;
	kern_return_t 	status = kIOReturnSuccess;

	FWLOG(( "SelectorController : init\n" ));

	[super init];
	
	fLUNArray = [[NSMutableArray alloc] init];

	// get master port
	status = IOMasterPort( bootstrap_port, &masterPort );
	
	// get matching devices
	
	if( status == kIOReturnSuccess )
	{
		FWLOG(( "SelectorController : got master port\n" ));
		matchDictionary = IOServiceMatching( "IOFireWireSBP2LUN" );
		if( matchDictionary == NULL )
			status = kIOReturnError;
	}

	if( status == kIOReturnSuccess )
	{
		FWLOG(( "SelectorController : got match dictionary\n" ));
		status = IOServiceGetMatchingServices( masterPort, matchDictionary, &iterator );
	}

    matchDictionary = NULL;
	
	if( status == kIOReturnSuccess )
	{
		io_object_t 	lunReference = 0 ;
	
		IOIteratorReset( iterator );
		while ( lunReference = IOIteratorNext( iterator ) )
		{
			LUNController * lun = [[LUNController alloc]initWithLUNReference:lunReference];
			[fLUNArray addObject:[lun autorelease]];
			FWLOG(( "SelectorController : lunController = 0x%08lx\n", (UInt32)lun ));
		}
	}

	if( iterator ) 
	{
        IOObjectRelease(iterator);
        iterator = 0 ;
    }

	// release master port
    if( masterPort ) 
	{
        mach_port_deallocate( mach_task_self(), masterPort );
        masterPort = 0 ;
    }
	
	return self;
}

- (void)awakeFromNib
{
	[fBrowser setTarget:self];
	[fBrowser setDoubleAction:@selector(selectItem)];
	[fWindow makeKeyAndOrderFront:self];
	[fBrowser selectRow:0 inColumn:0];
}

- (void)dealloc
{
	FWLOG(( "SelectorController : dealloc\n" ));
	
	[fLUNArray removeAllObjects];
	[fLUNArray release];

	[super dealloc];
}

- (int)browser:(NSBrowser *)sender numberOfRowsInColumn:(int)column
{
	if( column == 0 )
		return [fLUNArray count];
	else
		return 0;
}

- (void)browser:(NSBrowser *)sender willDisplayCell:(id)cell atRow:(int)row column:(int)column
{
	LUNController *	lun = NULL;
	IOReturn		status = kIOReturnSuccess;
	
	if( fLUNArray == NULL || row >= [fLUNArray count])
		status = kIOReturnError;
		
	if( status == kIOReturnSuccess )
	{	
		lun = [fLUNArray objectAtIndex:row];
		if( lun == NULL )
			status = kIOReturnError;
	}
 	
	if( status == kIOReturnSuccess )
	{
		[cell setStringValue:[lun getName]];
		[cell setLeaf:true];
		[cell setRepresentedObject:lun];
	}
}

- (void)selectItem:(id)sender
{
	id cell;
	cell = [fBrowser selectedCell];
	[[cell representedObject] instantiateLUNConnection];
}

@end
