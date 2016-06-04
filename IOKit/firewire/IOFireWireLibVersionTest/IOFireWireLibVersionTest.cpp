/*
	File:		IOFireWireLibVersionTest/IOFireWireLibVersionTest.cpp

	Synopsis:	This code demonstrates how to obtain the version of IOFireWireLib installed on the system.
				You need to open the user client to do this. While this example opens the user client on the
				local node, you can obtain the version of IOFireWireLib with the user client open on any
				device; it is not necessary to specifically use IOFireWireLocalNode.

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
	
	$Log: IOFireWireLibVersionTest.cpp,v $
	Revision 1.3  2005/02/03 01:34:11  firewire
	*** empty log message ***
	
	
	Revision 1.2  2002/11/11 19:47:52  noggin

	prepare for SDL
	
*/

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <unistd.h>

// user client includes:
#include <IOKit/firewire/IOFireWireLib.h>

int
main(int, char*)
{
	IOReturn				result				= kIOReturnSuccess ;
	IOFireWireLibDeviceRef	interface 			= 0 ;
	IOCFPlugInInterface**	cfPlugInInterface 	= 0 ;
	mach_port_t				masterPort ;
	
	//
	// get master port
	//
	result = ::IOMasterPort( MACH_PORT_NULL, & masterPort ) ;
	
	//
	// get a user client
	//
	if (result == kIOReturnSuccess)
	{
		SInt32 theScore ;
		result 		= IOCreatePlugInInterfaceForService( 
							IOServiceGetMatchingService( masterPort, IOServiceMatching( "IOFireWireLocalNode" ) ),
							kIOFireWireLibTypeID, kIOCFPlugInInterfaceID,
							& cfPlugInInterface, & theScore) ;
	}
	
	if ( result == kIOReturnSuccess )
	{
		(*cfPlugInInterface)->QueryInterface(
									cfPlugInInterface, 
									CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID),
									(void**) & interface) ;
	}

	if ( result == kIOReturnSuccess )
	{
		// get a reference to the IOFireWireLib plugin
		CFBundleRef	bundle = ::CFBundleGetBundleWithIdentifier( CFSTR("com.apple.IOFireWireLib") ) ;
		assert( bundle ) ;
		
		// read the version UInt32
		UInt32	version = ::CFBundleGetVersionNumber( bundle ) ;
		
		// print version info
		printf("IOFireWireLib version %lu.%lu.%lu (0x%lx)\n", version>>24 & 0xFF, version>>20 & 0xF, version>>16 & 0xF, version) ;
	}
	
	// clean up
	if ( interface )
		(*interface)->Release(interface) ;
	
	if (cfPlugInInterface)
		IODestroyPlugInInterface(cfPlugInInterface) ;
	
	return (result == kIOReturnSuccess) ? EXIT_SUCCESS : EXIT_FAILURE ;
}
