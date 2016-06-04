/*
	File:		IOFireWireLibVersionTest/IOFireWireLibVersionTest.cpp

	Synopsis:	This code demonstrates how to obtain the version of IOFireWireLib installed on the system.
				You need to open the user client to do this. While this example opens the user client on the
				local node, you can obtain the version of IOFireWireLib with the user client open on any
				device; it is not necessary to specifically use IOFireWireLocalNode.

	Copyright: 	© Copyright 2000-2002 Apple Computer, Inc. All rights reserved.

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
	
	$Log: IOFireWireLibPacketQueueTest.cpp,v $
	Revision 1.4  2005/10/12 16:05:50  ayanowit
	Updated for Tumeric Dev tools.
	
	Revision 1.3  2005/02/03 01:20:13  firewire
	*** empty log message ***
	
	Revision 1.2  2002/11/11 19:40:08  noggin
	prepare for SDK
	
	Revision 1.1  2002/11/07 00:45:58  noggin
	move to new repository
	
	
*/

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <unistd.h>

// user client includes:
#include <IOKit/firewire/IOFireWireLib.h>
#include <IOKit/firewire/IOFireWireFamilyCommon.h>

Ptr 	gBuf = 0 ;

// ============================================================
// callbacks for handling packets...
// ============================================================

//
// handle block writes to our address space
//
UInt32
PacketWriteHandler(
	IOFireWireLibPseudoAddressSpaceRef	addressSpace,
	FWClientCommandID			commandID,
	UInt32						packetLen,
	void*						packet,
	UInt16						nodeID,
	UInt32						addressHi,
	UInt32						addressLo,
	UInt32						refCon)
{
	printf("PacketWriteHandler:\n") ;
	printf("\trefCon\t%08lX\n", (UInt32) refCon) ;
	printf("\tnodeID\t%08lX\n", (UInt32)	nodeID) ;
	printf("\taddr\t%04lX:%04lX:%08lX\n", addressHi>>16, addressHi & 0xFFFF, addressLo) ;
	printf("\tlen\t%08lX\n", packetLen) ;
	printf("\tpacket:\t%p\n", packet) ;	
	
	printf("\t\t") ;
	for(UInt32 index=0; index<packetLen; index++)
	{
		if ((index & 0x3) == 0)
		{	printf(" ") ;
			if ((index & 0x7) == 0)
			{	printf("   ") ;
				if ((index & 0xF) == 0)
					printf("\n\t\t") ;
			} }
		printf("%02x", ((unsigned char*)packet)[index]) ;
	}
	
	printf("\n\n") ;
	
	printf("backing store range:\n") ;
	
	printf("\t\t") ;
	for(UInt32 index=0; index<packetLen; index++)
	{
		if ((index & 0x3) == 0)
		{	printf(" ") ;
			if ((index & 0x7) == 0)
			{	printf("   ") ;
				if ((index & 0xF) == 0)
					printf("\n\t\t") ;
			} }
		printf("%02X", (unsigned char)gBuf[index+(addressLo-0)]) ;	// just for now, i know that the
																	// base of this address space starts at
																	// 0
	}
	
	printf("\n\n") ;
	(*addressSpace)->ClientCommandIsComplete(addressSpace, commandID, kIOReturnSuccess) ;
	return 0 ;
}

// handle dropped packets (this callback is called when your queue of incoming packets
// is not being emptied fast enough and some packets must be dropped)
void SkippedPacketHandler(
	IOFireWireLibPseudoAddressSpaceRef	addressSpace,
	FWClientCommandID			commandID,
	UInt32						skippedPacketCount)
{
	printf("SkippedPacketHandler:\n") ;
	printf("\tskippedPacketCount\t%08lX\n", skippedPacketCount) ;
	
	(*addressSpace)->ClientCommandIsComplete(addressSpace, commandID, kIOReturnSuccess); 
}

// this callback handles any read requests to this address space
UInt32
PacketReadHandler(
	IOFireWireLibPseudoAddressSpaceRef	addressSpace,
	FWClientCommandID					commandID,
	UInt32								packetLen,
	UInt32								packetOffset,
	UInt16								nodeID,			// nodeID of requester
	UInt32								destAddressHi,	// destination on this node
	UInt32								destAddressLo,
	UInt32								refCon)
{
	static UInt32 readCount = 0 ;

	char msg[256] ;
	
	UInt8* buffer = (UInt8*) (*addressSpace)->GetBuffer(addressSpace) ;
	
	readCount++ ;

	sprintf(msg, "%03lu read number %lu -- ", readCount, readCount ) ;

	for(UInt32 pos = packetOffset; pos < packetOffset + packetLen; pos++)
	{
		buffer[pos] = msg[pos % strlen(msg)] ;
	}
	
	(*addressSpace)->ClientCommandIsComplete( addressSpace, commandID, kFWResponseComplete ) ;

	printf("PacketReadHandler:\n") ;
	printf("\trefCon\t%08lx\n", refCon) ;
	printf("\tnodeID\t%04x\n", nodeID) ;
	printf("\tpacketLen\t%08lx\n", packetLen) ;
	printf("\tpacketOffset\t%08lx\n", packetOffset) ;
	printf("\tcommandID\t%p\n", commandID ) ;

	return 0 ;
}


int
main(int, char*)
{
	IOReturn				error	=	kIOReturnSuccess ;

	mach_port_t				masterPort ;
	error = ::IOMasterPort( MACH_PORT_NULL, & masterPort ) ;

	//
	// get a user client
	//
	
	IOFireWireLibDeviceRef	interface = 0 ;
	IOCFPlugInInterface**	cfPlugInInterface = 0 ;
	
	if ( ! error )
	{
		SInt32 		theScore ;
		
		error = ::IOCreatePlugInInterfaceForService(
						IOServiceGetMatchingService( masterPort, IOServiceMatching( "IOFireWireLocalNode" ) ),
						kIOFireWireLibTypeID, kIOCFPlugInInterfaceID,
						& cfPlugInInterface, & theScore) ;
	}
	
	if ( ! error )
	{
		(*cfPlugInInterface)->QueryInterface(
									cfPlugInInterface, 
									CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID),
									(void**) & interface) ;
	}

	if ( ! error )
		error = (*interface)->Open(interface) ;
	
	if ( !error )
		error = (*interface)->AddCallbackDispatcherToRunLoop(interface, CFRunLoopGetCurrent()) ;

	// ask user client to create a pseudo address space for receiving packets
	IOFireWireLibPseudoAddressSpaceRef addressSpace = 0 ;
	IOFireWireLibPhysicalAddressSpaceRef	physAddressSpace = 0 ;
	
	if ( !error )
	{
		gBuf = (Ptr) new char[40960] ;						// allocate the backing store
		sprintf(gBuf, "arrrr... there be text here!!") ;	// put some text in it. (just for show)
		
		// ask our interface to create a pseudo address space for us
		addressSpace = (*interface)->CreatePseudoAddressSpace(interface, 40960, 
											(void*) addressSpace, 8192, gBuf, 
											0,
											CFUUIDGetUUIDBytes(kIOFireWirePseudoAddressSpaceInterfaceID)) ;
		
		if (addressSpace)
		{
			FWAddress	address ;
			(*addressSpace)->GetFWAddress(addressSpace, & address) ;
			
			printf("allocated address space at %04x:%08lx\n", address.addressHi, address.addressLo) ;
			
			(*addressSpace)->SetWriteHandler( addressSpace, PacketWriteHandler ) ;
			(*addressSpace)->SetSkippedPacketHandler( addressSpace, SkippedPacketHandler ) ;
			(*addressSpace)->SetReadHandler( addressSpace, PacketReadHandler ) ;
			
			if (!(*addressSpace)->TurnOnNotification(addressSpace))
				error = kIOReturnError ;
		}
		else
			error = kIOReturnNoResources ;


	}

	if ( !error)
	{
		// choose how to run your app:
		
		CFRunLoopRun() ;
//		RunApplicationEventLoop() ;
//		CFRunLoopRunInMode( kCFRunLoopDefaultMode, 5, true ) ;
	}

	if (addressSpace)
		( *addressSpace )->Release( addressSpace ) ;
	if ( physAddressSpace )
		( *physAddressSpace)->Release( physAddressSpace ) ;
		
	if ( interface )
	{
		(*interface)->Close(interface) ;
		(*interface)->RemoveCallbackDispatcherFromRunLoop(interface) ;
		(*interface)->Release(interface) ;
	}
	
	if (cfPlugInInterface)
		IODestroyPlugInInterface(cfPlugInInterface) ;
	
	return ( !error ) ? EXIT_SUCCESS : EXIT_FAILURE ;
}
