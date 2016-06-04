/*
	File:		IOFireWireLibIsochTest/main.cpp

	Synopsis:	This is an introduction to the isochronous services of the FireWire user client. 
				This source code is a good place to start if you are interested in sending or 
				receiving data isochronously. You could easily modify this sample code to receive 
				isochronous data instead of send it.
				
				We create a local port representing our talking Macintosh with a talking DCL program.
				We create a remote port that represents our remote device. We don't actually support a remote
				device, but you can see how the remote callbacks should be used.
				
				This sample code opens a user client on the local node and starts talking on an isochronous 
				channel.
				
				WARNING: Avoid running this sample on a laptop with no devices connected - the FireWire chip will
				have been put to sleep and bad things will happen. However, normally one wouldn't run an 
				isochronous channel with no devices on the bus, so this shouldn't be a problem.

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.

	Written by: NWG
	
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
	
	$Log: main.cpp,v $
	Revision 1.1  2002/11/07 00:39:40  noggin
	move to new repository
	
	Revision 1.6  2002/08/22 21:44:54  noggin
	Got rid of global variables. Renamed Indenter class. Remove libstdc++ from project.
	
*/

#import <iostream>
#import <unistd.h>

#import <CoreFoundation/CoreFoundation.h>
#import <IOKit/firewire/IOFireWireLibIsoch.h>
#import <IOKit/IOKitLib.h>
#import <mach/mach.h>

#import "Indenter.h"

#define _success	(error ? "failed" : "ok")
#define _file		__FILE__ << ':' << __LINE__ << ":   "

typedef IOCFPlugInInterface** IOCFPlugInInterfaceRef ;

const IOByteCount	kBufferSize = 8192 ;

//
// globals
//

Indenter						i ;

#pragma mark -
#pragma mark REMOTE PORT

//
// callback handler
//

void
DCLCallbackHandler( DCLCommand* dcl )
{
	static int count = 0 ;
	
	if (++count % 100 == 0)
		printf("isoch callback %u\n", count) ;
}

//
// remote port overrides
//

IOReturn
RemotePort_GetSupported( IOFireWireLibIsochPortRef interface, IOFWSpeed* outMaxSpeed, UInt64* outChanSupported)
{
	// In this routine we return the capabilities of our remote
	// device. We don't have an actual listener out on the bus
	// so we just say we run at all speeds and on all isochronous
	// channel numbers.

	cout << i << "+RemotePort_GetSupported\n" ;

	// we support all speeds...
	// if we didn't, we could set this to kFWSpeed200MBit or another
	// appropriate value
	*outMaxSpeed		= kFWSpeedMaximum ;
	
	// we support all channels
	// if you had a specific channel number in mind, you could set
	// 'outChanSupported' to allow only that channel you are interested in
	// to be used
	*outChanSupported	= (UInt64)0xFFFFFFFF << 32 | (UInt64)0xFFFFFFFF ;
	
	// ok!
	return kIOReturnSuccess ;
}
	
IOReturn
RemotePort_AllocatePort( IOFireWireLibIsochPortRef interface, IOFWSpeed maxSpeed, UInt32 channel )
{
	// In this routine we tell our remote device (if we had one) to prepare
	// to listen. You can perform and register reads/writes need to get your
	// prepare your device for listening here...

	cout << i << "+RemotePort_AllocatePort\n" ;
	return kIOReturnSuccess ;
}
	
IOReturn
RemotePort_ReleasePort( IOFireWireLibIsochPortRef interface )
{
	// Here we tell our device we are done using the channel. Do any cleanup
	// needed after receiving isochronous data here.

	cout << i << "+RemotePort_ReleasePort\n" ;
	return kIOReturnSuccess ;
}
	
IOReturn
RemotePort_Start( IOFireWireLibIsochPortRef interface )
{
	cout << i << "+RemotePort_Start\n" ;
	// Talk to remote device and tell it to start listening.

	return kIOReturnSuccess ;
}
	
IOReturn
RemotePort_Stop( IOFireWireLibIsochPortRef interface )
{
	cout << i << "+RemotePort_Stop\n" ;
	// Talk to remote device and tell it to stop listening.

	return kIOReturnSuccess ;
}

#pragma mark -
#pragma mark MISC
//
// functions
//
IOReturn
GetDeviceInterface( IOCFPlugInInterfaceRef* outCFPlugInInterface, IOFireWireLibNubRef* outLocalNode )
{
	IOReturn		error = kIOReturnSuccess ;

	mach_port_t		masterPort = 0 ;
	error = IOMasterPort( MACH_PORT_NULL, & masterPort ) ;
	
	io_service_t	theService = 0 ;
	if ( error == kIOReturnSuccess )
	{
		io_iterator_t	iterator	= 0 ;
		error = IOServiceGetMatchingServices( masterPort, IOServiceMatching( "IOFireWireLocalNode" ),
						& iterator ) ;
		if ( !iterator )
		{
			cout << i << _file << "Couldn't get iterator\n" ;
			error = kIOReturnError ;
		}
		else
		{
			theService = IOIteratorNext( iterator ) ;
			if ( !theService )
			{
				cout << i << _file << "Iterator returned no service\n" ;
				error = kIOReturnError ;
			}
		}
	}
	
	if ( error == kIOReturnSuccess )
	{
		SInt32		theScore ;
		error = IOCreatePlugInInterfaceForService( theService, 
						kIOFireWireLibTypeID, kIOCFPlugInInterfaceID,
						outCFPlugInInterface, & theScore) ;

		if ( error == kIOReturnSuccess )
		{
			error = (***outCFPlugInInterface).QueryInterface( *outCFPlugInInterface, 
												CFUUIDGetUUIDBytes( kIOFireWireNubInterfaceID ),
												(void**) outLocalNode ) ;
			if ( error != S_OK )
			{
				cout << i << _file << "CFPlugInInterface->QueryInterface returned error\n" ;
				error = kIOReturnError ;
			}
		}
	}
	
	return error ;
}

IOReturn
CreateDCLCommandPool( IOFireWireLibNubRef nub, IOFireWireLibDCLCommandPoolRef* outCommandPool )
{
	// This function allocates a DCLCommandPoolInterface. This is a pool of DCLs and it also contains methods
	// to simplify programmatic creation of DCL programs.

	cout << i-- << "+CreateDCLCommandPool\n" ;
	
	*outCommandPool = (**nub).CreateDCLCommandPool( nub, 0x1000, CFUUIDGetUUIDBytes( kIOFireWireDCLCommandPoolInterfaceID ) ) ; // 4K
	if (!*outCommandPool)
		return kIOReturnError ;
		
	return kIOReturnSuccess ;
}

DCLCommand*
WriteTalkingDCLProgram( UInt8* buffers[], IOFireWireLibDCLCommandPoolRef pool )
{
	// Here we programatically create our talking DCL program using the DCLCommandPool allocated earlier

	cout << i << "+WriteTalkingDCLProgram\n" ;
	DCLCommand*	dcl ;
	
	dcl = (**pool).AllocateLabelDCL( pool, nil ) ;
	DCLCommand*	firstDCL = dcl ;
	
	for( int i=0; i < 100; ++i )
		dcl = (**pool).AllocateSendPacketStartDCL( pool, dcl, buffers[0], 0x400 ) ;	// 1K packet

	dcl = (**pool).AllocateCallProcDCL( pool, dcl, & DCLCallbackHandler, 0 ) ;
	dcl = (**pool).AllocateJumpDCL( pool, dcl, nil ) ;
	DCLJump* 	testJumpDCL = reinterpret_cast<DCLJump*>(dcl) ;
	
	dcl = (**pool).AllocateLabelDCL( pool, dcl ) ;
	testJumpDCL->pJumpDCLLabel = reinterpret_cast<DCLLabel*>(dcl) ;

	dcl = (**pool).AllocateSendPacketStartDCL( pool, dcl, buffers[2], 0x400 ) ;	// 1K packet
	dcl = (**pool).AllocateSendPacketStartDCL( pool, dcl, buffers[2], 0 ) ;	// empty packet
	
	dcl = (**pool).AllocateJumpDCL( pool, dcl, (DCLLabel*)firstDCL ) ;
	
	return firstDCL ;
}

IOReturn
CreateLocalIsochPort( IOFireWireLibNubRef nub, UInt8* buffers[], IOFireWireLibDCLCommandPoolRef pool, IOFireWireLibLocalIsochPortRef* outPort )
{
	cout << i << "+CreateLocalIsochPort\n" ;

	DCLCommand*	dclProgram = WriteTalkingDCLProgram( buffers, pool ) ;

	// uncomment the next line if we want to inspect the DCL program we created...
//	(**nub).PrintDCLProgram( nub, dclProgram, 6 ) ;
	
	*outPort = (**nub).CreateLocalIsochPort( nub, true, dclProgram, 0, 0, 0, nil, 0, nil, 0, CFUUIDGetUUIDBytes( kIOFireWireLocalIsochPortInterfaceID ) ) ;

	return *outPort ? kIOReturnSuccess : kIOReturnError ;
}

IOReturn
CreateRemoteIsochPort( IOFireWireLibNubRef nub, IOFireWireLibRemoteIsochPortRef* outPort )
{
	cout << i << "+CreateRemoteIsochPort\n" ;

	IOFireWireLibRemoteIsochPortRef		port ;
	
	port = (**nub).CreateRemoteIsochPort( nub, false, CFUUIDGetUUIDBytes( kIOFireWireRemoteIsochPortInterfaceID ) ) ;

	if (!port)
		return kIOReturnError ;
	
	(**port).SetGetSupportedHandler( port, & RemotePort_GetSupported ) ;
	(**port).SetAllocatePortHandler( port, & RemotePort_AllocatePort ) ;
	(**port).SetReleasePortHandler( port, & RemotePort_ReleasePort ) ;
	(**port).SetStartHandler( port, & RemotePort_Start ) ;
	(**port).SetStopHandler( port, & RemotePort_Stop ) ;
		
	*outPort = port ;
	return kIOReturnSuccess ;
}

IOReturn
CreateIsochChannel( IOFireWireLibNubRef nub, IOFireWireLibIsochChannelRef* outChannel )
{
	cout << i << "+CreateIsochChannel\n" ;

	*outChannel = (**nub).CreateIsochChannel( nub, true, 1024, kFWSpeed400MBit, CFUUIDGetUUIDBytes( kIOFireWireIsochChannelInterfaceID ) ) ;
	
	return (*outChannel) ? kIOReturnSuccess : kIOReturnError  ;
}

#pragma mark -
#pragma mark MAIN

int
main ( int argc, const char * argv[] ) 
{

	//
	// allocate our DMA buffers using vm_allocate
	//
	
	vm_address_t	buffer = nil ;
	::vm_allocate( mach_task_self(), & buffer, kBufferSize, true ) ;
	
	assert( buffer ) ;

	//
	// split up vm_allocated buffer into buffers we will use
	// in our DMA program
	//
	
	UInt8*	buffers[3] = { (UInt8*)buffer, (UInt8*)buffer + 1024, (UInt8*)buffer + 4096 } ;

	//
	// put test data into our DMA program buffers
	//
	
	snprintf( reinterpret_cast<char*>(buffers[0]), 1024, "sample text in a buffer" ) ;
	snprintf( reinterpret_cast<char*>(buffers[1]), 3048, "<this space intentionally left blank>" ) ;
	snprintf( reinterpret_cast<char*>(buffers[2]), 1024, "next sample text is here" ) ;

	//
	// run test
	//
	
	IOReturn	error 			= kIOReturnSuccess ;
	
	cout << i++ << "\n### SETTING UP\n" ;
	
	for( int index=0; index < 3; ++index )
		cout << i << "buffers[" << index << "] " << (char*)(buffers[index]) << endl ;
	
	cout << i++ << "Getting device interface...\n" ;

	IOFireWireLibNubRef				localNode		= 0 ;
	IOCFPlugInInterfaceRef			cfPlugInInterface ;

	error = GetDeviceInterface( & cfPlugInInterface, & localNode ) ;
	cout << i-- << "..." << _success << endl ;
			
	if ( !error )
	{
		cout << i << "Opening...\n" ;
		error = (**localNode).Open( localNode ) ;
	}
	
//	if (!error)
//	{
//		error = (**localNode).AddCallbackDispatcherToRunLoop( localNode, ::CFRunLoopGetCurrent() ) ;
//		if (!error)
//			(**localNode).TurnOnNotification( localNode ) ;
//	}

	if ( !error )
	{
		cout << i++ << "Adding isoch dispatcher to run loop...\n" ;

		error = (**localNode).AddIsochCallbackDispatcherToRunLoop( localNode, CFRunLoopGetCurrent() ) ;
	}
	
	
	IOFireWireLibDCLCommandPoolRef		commandPool 	= 0 ;
	IOFireWireLibRemoteIsochPortRef		remoteIsochPort	= 0 ;
	IOFireWireLibLocalIsochPortRef		localIsochPort	= 0 ;
	IOFireWireLibIsochChannelRef		isochChannel	= 0 ;
	
	if ( !error )
	{
		cout << i++ << "Creating DCL command pool...\n" ;
		error = CreateDCLCommandPool( localNode, & commandPool ) ;
	}
	
	if ( !error )
	{
		cout << i++ << "Creating remote isochronous port...\n" ;
		error = CreateRemoteIsochPort( localNode, & remoteIsochPort ) ;
		cout << i-- << "..." << _success << endl ;
	}
	
	if ( !error )
	{
		cout << i << "Creating local isoch port...\n" ;
		error = CreateLocalIsochPort( localNode, buffers, commandPool, & localIsochPort ) ;
		cout << i-- << "..." << _success << endl ;
	}

	if ( !error )
	{
		cout << i++ << "Creating isoch channel...\n" ;
		error = CreateIsochChannel( localNode, & isochChannel ) ;
		cout << i-- << "..." << _success << endl ;
	}
	
		// remote is listener
	if ( !error )
	{
		cout << i++ << "Adding listener to isoch channel...\n" ;
		error = (**isochChannel).AddListener( isochChannel, (IOFireWireLibIsochPortRef) remoteIsochPort ) ;
		cout << i-- << "..." << _success << endl ;
	}
	
		// local is talker
	if ( !error )
	{
		cout << i++ << "Setting talker on isoch channel...\n" ;
		error = (**isochChannel).SetTalker( isochChannel, (IOFireWireLibIsochPortRef) localIsochPort ) ;
		cout << i-- << "..." << _success << endl ;
	}
	
	if ( !error )
	{
		cout << i++ << "Allocating channel...\n" ;
		error = (**isochChannel).AllocateChannel( isochChannel ) ;
		cout << i-- << "..." << _success << endl ;
	}

	i-- ;	// unindent
	
	// run for approx. 15 seconds
	cout << i++ << "\n### RUNNING\n" ;

	if ( !error )
	{
		if ( !error )
		{
			cout << i++ << "Starting channel...\n" ;
			error = (**isochChannel).Start( isochChannel ) ;
			cout << i-- << "..." << _success << endl ;
		}
		
		cout << i << "\nRunning...\n" ;

		SInt16	runLoopResult ;
		while (  kCFRunLoopRunHandledSource == ( runLoopResult = CFRunLoopRunInMode( kCFRunLoopDefaultMode, 30, false ) ) ) 
		{
		}
		
		error = (**isochChannel).Stop( isochChannel ) ;
		
		cout << i-- << "done!\n" ;
				
		// clean up
		cout << i++ << "\n### CLEAN UP ###\n\n" ;
	
		// release all interfaces
		if ( isochChannel )
		{
			cout << i++ << "releasing isoch channel.\n" ;
	
			if ( error != kIOReturnSuccess )
				cout << i << _file << "error " << error << " stopping channel.\n" ;
			
			error = (**isochChannel).ReleaseChannel( isochChannel ) ;
			if ( error != kIOReturnSuccess )
				cout << i << _file << "error " << error << " from ReleaseChannel().\n" ;
	
			(**isochChannel).Release( isochChannel ) ;
			
			i-- ;
		}
			
		if ( localIsochPort )
		{
			cout << i << "releasing local port.\n" ;

			(**localIsochPort).Release( localIsochPort ) ;
		}
		
		if ( remoteIsochPort )
		{
			cout << i << "releasing remote port.\n" ;
			(**remoteIsochPort).Release( remoteIsochPort ) ;
		}
	}
	
	if ( localNode )
	{
		cout << i << "releasing local node interface.\n" ;
		(**localNode).Close( localNode ) ;
		(**localNode).Release( localNode ) ;
	}
	
	//
	// release the original CFPlugInInterface
	//
	
	if ( cfPlugInInterface )
	{
		cout << i << "releasing CFPlugInInterface.\n" ;
		IODestroyPlugInInterface( cfPlugInInterface ) ;
	}
	
	//
	// deallocate our DMA buffers
	//
	
	if (buffer)
	{
		::vm_deallocate( mach_task_self(), buffer, kBufferSize ) ;
	}
	
	cout << i << "### DONE\n" ;
	return (error == kIOReturnSuccess) ? EXIT_SUCCESS : EXIT_FAILURE ;
}
