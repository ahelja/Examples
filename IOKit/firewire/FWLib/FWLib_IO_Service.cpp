/*
	File:		FWLib_IO_Service.cpp

	Synopsis: 	C++ interface that represents IOService objects.

	Copyright: 	© Copyright 2001-2003 Apple Computer, Inc. All rights reserved.

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

	$Log: FWLib_IO_Service.cpp,v $
	Revision 1.4  2004/02/06 23:29:12  firewire
	*** empty log message ***
	
	Revision 1.3  2003/05/27 18:12:46  firewire
	SDK16
	
	Revision 1.2  2003/05/27 17:34:04  firewire
	*** empty log message ***
	
	Revision 1.1  2002/11/07 00:32:13  noggin
	move to new repository
	
	Revision 1.5  2002/08/21 22:26:02  noggin
	*** empty log message ***
	
	Revision 1.4  2002/06/20 19:10:08  noggin
	*** empty log message ***
	
	Revision 1.3  2002/06/12 03:54:52  noggin
	added to repository
	
	Revision 1.2  2002/06/07 21:25:43  noggin
	*** empty log message ***
	
	Revision 1.1  2002/06/05 19:20:49  noggin
	names changed. namespace names changes. nested IO in FWLib
	
*/

#import "FWLib_IO_Service.h"
#import <iostream> // for debugging

namespace FWLib {
namespace IO {
#if 0
	Service::Service( io_service_t service )
	: mService(service)
	{
		if (!mService)
			throw std::exception() ;
		
		::IOObjectRetain(mService) ;
	}
#endif	
	CFArrayRef
	Service::GetMatchingServices( CFDictionaryRef matching )
	{
		mach_port_t	masterPort ;
		IOReturn error = ::IOMasterPort( MACH_PORT_NULL, &masterPort ) ;
		if (error)
			throw std::exception() ;
		
		io_iterator_t iterator ;
		error = ::IOServiceGetMatchingServices( masterPort, matching, & iterator ) ;
		if (error)
			throw std::exception() ;
		
		CFMutableArrayRef	result = ::CFArrayCreateMutable( kCFAllocatorDefault, 1, nil ) ;
		if (!result)
			throw std::bad_alloc() ;
		
		io_service_t service ;
		while (nil != (service = ::IOIteratorNext(iterator)))
			::CFArrayAppendValue( result, (const void*)service ) ;
		
		return result ;
	}

	io_service_t
	Service::GetOneMatching( CFDictionaryRef matching )
	{
		io_service_t service ;
		CFArrayRef	matchingServices = GetMatchingServices(matching) ;
		if (::CFArrayGetCount(matchingServices) > 0)
			service = reinterpret_cast<io_service_t>( ::CFArrayGetValueAtIndex( matchingServices, 0 ) ) ;
		else
			service = 0 ;
		::CFRelease(matchingServices) ;
		
		return service ;
	}

#if 0
#pragma mark -
// future use:
//	const CFArrayCallBacks RegistryEntry::sCallBacks = { 0, nil, & RegistryEntry::S_CFRelease, nil, nil } ;
	
	RegistryEntry::RegistryEntry( mach_port_t masterPort, const io_string_t path )
	:Service( ::IORegistryEntryFromPath( masterPort, path ) )
	{
		::IOObjectRelease(mService) ;
	}
	
	RegistryEntry::RegistryEntry( CFDictionaryRef matching )
	:Service( Service::GetOneMatching(matching) )
	{
		::IOObjectRelease(mService) ;
	}

	RegistryEntry::RegistryEntry( const io_registry_entry_t service )
	: Service( service )
	{
	}

	CFArrayRef
	RegistryEntry::Children( const io_name_t plane ) const
	{
		CFArrayCallBacks*	callbacks = nil;
//		CFArrayCallBacks*	callbacks = & sCallBacks ;	// <-- future use

		CFMutableArrayRef	result = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, callbacks ) ;
		io_iterator_t	iterator ;
		
		IOReturn error = IORegistryEntryGetChildIterator( mService, plane, & iterator ) ;
		if (error)
			throw std::exception() ;
		
		io_registry_entry_t	entry ;
		while( nil != ( entry = ::IOIteratorNext( iterator ) ) )
		{
			::CFArrayAppendValue( result, new RegistryEntry(entry) ) ;
		}
		
		::IOObjectRelease(iterator) ;
		return result ;
	}
	
#pragma mark -
	FireWireNub::FireWireNub( mach_port_t masterPort, const io_string_t path )
	: RegistryEntry( masterPort, path )
	{
		if ( !::IOObjectConformsTo(mService, "IOFireWireNub"))
			throw std::exception() ;
	}
	
	FireWireNub::FireWireNub( CFDictionaryRef matching )
	: RegistryEntry(matching)
	{
		if ( !::IOObjectConformsTo(mService, "IOFireWireNub"))
			throw std::exception() ;
	}
	
	FireWireNub::FireWireNub( const io_service_t service )
	: RegistryEntry(service)
	{
		if ( !::IOObjectConformsTo(mService, "IOFireWireNub"))
			throw std::exception() ;
	}

	UInt16
	FireWireNub::NodeID() const
	{
		CFNumberRef cfNodeID = static_cast<CFNumberRef>( CreateCFProperty( CFSTR("FireWire Node ID") ) ) ;
		if (!cfNodeID)
			throw std::exception() ;
		
		UInt16 result ;
		if (!::CFNumberGetValue( cfNodeID, kCFNumberSInt16Type, & result ))
			throw std::exception() ;
		
		::CFRelease(cfNodeID) ;
		
		return result ;
	}

	CFDataRef
	FireWireNub::SelfIDs() const
	{
		return static_cast<CFDataRef>( CreateCFProperty( CFSTR("FireWire Self IDs") ) ) ;
	}

#pragma mark -
	FireWireLocalNode::FireWireLocalNode()
	: FireWireNub( ::IOServiceMatching("IOFireWireLocalNode"))
	{
	}
#endif
}}

