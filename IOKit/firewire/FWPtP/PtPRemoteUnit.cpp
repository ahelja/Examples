/*
	File:		FWPtP/PtPRemoteUnit.cpp

	Synopsis: 	Sample code for simple peer-to-peer FireWire protocol application.

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

	$Log: PtPRemoteUnit.cpp,v $
	Revision 1.2  2004/03/23 23:27:27  firewire
	*** empty log message ***
	
	Revision 1.1  2002/11/07 00:38:00  noggin
	move to new repository
	
	Revision 1.2  2002/10/15 21:03:12  noggin
	now properly handle unplugged nodes. added skipped packet handler (just in case).
	
	Revision 1.1  2002/10/14 22:23:07  noggin
	ready for SDK14
	
	
*/

#import <FWLib/FWLib.h>

#import "PtPConstants.h"
#import "PtPRemoteUnit.h"
#import "Application.h"

#if LOOPBACK_TEST > 0
extern FWAddress	gLocalAddress ;
#endif

PtPRemoteUnit::PtPRemoteUnit( io_service_t service, Application& app )
: Device( service ),
  mApp( app )
{
	Open() ;

#if LOOPBACK_TEST > 0
	mMsgAddress = gLocalAddress ;
#else
	FWLib::ConfigDirectory*		configDirectory		= new FWLib::ConfigDirectory( *this ) ;
	CFDataRef					cfAddressData ;

	IOReturn					error	 			= configDirectory->GetKeyValue( kPtPMessageAddressKey, cfAddressData ) ;
	if ( error )
		throw std::exception() ;
	
	::CFDataGetBytes( cfAddressData, ::CFRangeMake(0, sizeof(FWAddress)), (UInt8*) & mMsgAddress ) ;
#endif

	// get master port
	mach_port_t			masterPort ;
	error = ::IOMasterPort( MACH_PORT_NULL, &masterPort ) ;
	if (error)
		throw std::exception() ;

	// make notification port
	mNotifyPort = ::IONotificationPortCreate( masterPort );

	// connect port to service of interest to get interest notifications
	error = ::IOServiceAddInterestNotification( mNotifyPort, service, kIOGeneralInterest, & S_InterestCallback, 
			this, & mNotification );
	if (error)
		throw std::exception() ;

	// add notification port to run loop
	::CFRunLoopAddSource( ::CFRunLoopGetCurrent(), 
			::IONotificationPortGetRunLoopSource( mNotifyPort ), kCFRunLoopDefaultMode ) ;
}

PtPRemoteUnit::~PtPRemoteUnit()
{
	Close() ;

	::CFRunLoopSourceInvalidate( ::IONotificationPortGetRunLoopSource( mNotifyPort ) ) ;
	::IOObjectRelease( mNotification ) ;
	::IONotificationPortDestroy( mNotifyPort ) ;
}

void
PtPRemoteUnit::Message( CFStringRef msg )
{
	CFDataRef data = ::CFStringCreateExternalRepresentation( kCFAllocatorDefault, msg, kCFStringEncodingUnicode, 0xFF);

	UInt32			retries = 0 ;
	IOReturn		error ;
	do
	{
		UInt32			generation ;

		error = GetBusGeneration( generation ) ;
		if (error)
			throw std::exception() ;
		
		UInt32		length = ::CFDataGetLength( data ) ;
		error = Write( mMsgAddress, ::CFDataGetBytePtr( data ), length, kFWFailOnReset, generation ) ;
	} while (++retries < 10 && error == kIOFireWireBusReset) ;
	
	::CFRelease(data) ;
}

void
PtPRemoteUnit::S_InterestCallback( void * refcon, io_service_t service,  natural_t messageType, 
		void * messageArgument )
{
	switch( messageType )
	{
		case kIOFWMessageServiceIsRequestingClose:
			PtPRemoteUnit*	me = reinterpret_cast<PtPRemoteUnit*>( refcon ) ;
			me->mApp.RemoveRemote( me ) ;
			delete me ;
			break ;
	}
}
