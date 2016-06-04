/*
	File:		IOFireWireLibDemo.cpp

	Synopsis: 	This is an introduction to the FireWire user client. This source is a good place to start
				if you are new to the FireWire user client.
				
				This sample code opens the FireWire user client on every device connected to the system
				and prints the contents of their config ROMs.

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

*/


#import <CoreFoundation/CoreFoundation.h>
#import <IOKit/IOKitLib.h>

#import "FWLib_Device.h"

using namespace FWLib ;

// prototypes
void 				PrintDeviceConfigROM( const void * service, void*) ;
CFMutableArrayRef 	GetIOFireWireDevices() ;


int
main(int , char** )
{
	
	// get the FireWire devices from the registry
	CFMutableArrayRef	devices = GetIOFireWireDevices() ;

	// check that array exists
	if ( !devices )
	{
		return EXIT_FAILURE ;
	}
	
	// print ROMs for all devices
	::CFArrayApplyFunction( devices, CFRangeMake( 0, ::CFArrayGetCount( devices ) ), PrintDeviceConfigROM, nil ) ;
	
	// dispose our array of device references
	::CFRelease( devices ) ;		
		
	return EXIT_SUCCESS ;
}

/*	============================================================
 *	PrintDeviceConfigROM()
 *
 *		Takes an io_object_t reference to a device (from
 *		the registry) and prints the contents of its Config
 *		ROM.
 *	============================================================*/

void 
PrintDeviceConfigROM( const void * service, void * /*context*/ )
{
	if ( !service )
	{
		return ;
	}
	
	printf("\n\ndevice %p:", service ) ;

	Device device( reinterpret_cast<io_service_t>( service ) ) ;

	// Open the device so we can talk to it...
	IOReturn error = device.Open() ;
	
	if ( !error )
	{
		// initialize the address to read from to start of  the Config ROM
		FWAddress   currentAddress	= FWAddress(0xFFFF, 0xf0000400) ;
		UInt32		readValue ;
		unsigned	size			= 4 ;
		
		// Read quadlet at current address
		do 
		{
			error = device.ReadQuadlet( currentAddress, & readValue ) ;

			if ( error )
			{
				break ;
			}

			// output to terminal:
			if (! (currentAddress.addressLo & 0xF))
			{
				if (currentAddress.addressLo & 0x10)
				{
					printf("   %08lX", readValue) ;
				}
				else
				{
					printf("\n\t%04x.%08lx: %08lx", currentAddress.addressHi, currentAddress.addressLo, readValue) ;
				}
			}
			else
			{
				printf(" %08lX", readValue) ;
			}
			
			// move to next address
			currentAddress.addressLo += 4 ;
			
			// reset size to 4 for next read (just in case it changed)
			size = 4 ;
			
		} while ( currentAddress.addressLo < 0xFFFFF808 ) ;
		
		printf("\n") ;
		
		if ( error )
		{
			printf("\t<ReadQuadlet() returned error 0x%x>\n", error ) ;
		}
		
		// Need to do this to get rid of the user client we've attached to our device:
		device.Close() ;
	}
}

/*	============================================================
 *	GetIOFireWireDevices()
 *
 *		Finds every IOFireWireDevice in the registry and returns
 *		them in a CFArray.
 *	============================================================*/

CFMutableArrayRef GetIOFireWireDevices()
{
	io_iterator_t		iterator ;
	CFMutableArrayRef	deviceArray = NULL ;
	
	printf("***\n*** Getting IOFireWireDevices from registry\n***\n") ;

	// get a registry enumerator for all IOFireWireNub services
	// and subclasses thereof.
	// We're really interested in IOFireWireDevice's and IOFireWireLocalNode's, not
	// IOFireWireUnit's.
	
	IOReturn error = ::IOServiceGetMatchingServices(
							kIOMasterPortDefault,
							::IOServiceMatching( "IOFireWireNub" ),
							& iterator ) ;

	if( error )
	{
		printf("IOServiceGetMatchingServices = 0x%x\n", error ) ;
	}
	
	// add devices on the bus to our list
	if ( !error )
	{
		// Get devices into device array.
		deviceArray = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, NULL ) ;
		if ( !deviceArray )
		{
			error = kIOReturnNoMemory ;
		}
	}
	
	if ( !error )
	{
		io_object_t		newDevice ;		
		while( 0 != ( newDevice = ::IOIteratorNext( iterator ) ) )
		{
			io_string_t className ;
			error = ::IOObjectGetClass( newDevice, className ) ;
			if ( error )
			{
				break ;
			}
			
			printf("Found device 0x%x (%s)   ...", newDevice, className ) ;
			
			// ignore any IOFireWireUnits...
			if ( strcmp( className, "IOFireWireUnit" ) != 0 )
			{
				printf("adding\n") ;
				::CFArrayAppendValue( deviceArray, reinterpret_cast<const void **>( newDevice ) ) ;
			}
			else
			{
				printf("ignoring\n") ;
			}
		}
	}
	
	// on success, return an array of devices
	if ( error && deviceArray )
	{
		::CFRelease( deviceArray ) ;
		deviceArray = NULL ;
	}

	if ( iterator )
	{
		IOObjectRelease( iterator ) ;
	}
	
	return deviceArray ;
}
