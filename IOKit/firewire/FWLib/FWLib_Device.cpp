/*
	File:		FWLib_Device.cpp

	Synopsis: 	C++ class to represent an device on the FireWire bus. Corresponds
				to IOFireWireDeviceInterface.

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

	$Log: FWLib_Device.cpp,v $
	Revision 1.4  2004/02/06 23:29:11  firewire
	*** empty log message ***
	
	Revision 1.3  2003/05/27 18:12:45  firewire
	SDK16
	
	Revision 1.2  2003/03/13 02:34:18  firewire
	*** empty log message ***
	
	Revision 1.1  2002/11/07 00:32:12  noggin
	move to new repository
	
	Revision 1.4  2002/10/14 22:24:55  noggin
	SDK14
	
	Revision 1.3  2002/08/21 22:26:02  noggin
	*** empty log message ***
	
	Revision 1.2  2002/06/07 21:25:42  noggin
	*** empty log message ***
	
	Revision 1.1  2002/06/05 19:20:47  noggin
	names changed. namespace names changes. nested IO in FWLib
	
	Revision 1.2  2002/05/31 21:21:51  noggin
	added version logging to all files
	
	Revision 1.1  2002/05/31 21:12:37  noggin
	first check-in
	
*/

#import "FWLib_Device.h"
#import <iostream>
#import <exception>
#import <IOKit/IOKitLib.h>

#define _file	__FILE__ << ":" << __LINE__ << " "
#define kDeviceIID				(CFUUIDGetUUIDBytes( kIOFireWireDeviceInterfaceID_v5 ))

using namespace std ;

namespace FWLib {

	Device::Device( const io_service_t service )
	: mIsochRunLoop( 0 )
	{
		IOReturn			err ;
	
		// get IOCFPlugInInterface plug-in interface
		SInt32				theScore ;
		err = ::IOCreatePlugInInterfaceForService( service, kIOFireWireLibTypeID, kIOCFPlugInInterfaceID, & mIOCFPlugInInterface, & theScore) ;
		if ( err )
		{
			cerr << "Couldn't create plug in interface\n" ;
			throw std::exception() ;
		}
		
		// get device interface
		err = (**mIOCFPlugInInterface).QueryInterface( mIOCFPlugInInterface, kDeviceIID, (void**) & mDevice ) ;
		
		if ( err != S_OK )
		{
			cerr << _file << "CFPlugInInterface->QueryInterface returned error\n" ;
			throw std::exception() ;
		}
		
	}
	
	Device::~Device()
	{
		if ( mDevice )
			(**mDevice).Release( mDevice ) ;
			
		if ( mIOCFPlugInInterface )
			::IODestroyPlugInInterface( mIOCFPlugInInterface) ;
	}	

	IOReturn
	Device::AddIsochCallbackDispatcherToRunLoop( CFRunLoopRef runloop )
	{
		mIsochRunLoop = runloop ;
		return (**mDevice).AddIsochCallbackDispatcherToRunLoop( mDevice, runloop ) ; 
	}
}
