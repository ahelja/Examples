/*
	File:		FWUtilThreads.cpp

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

*/

#import "FWUtilThreads.h"
#import "FWUtilGlobals.h"

#import <pthread.h>
#import <iostream>
#import <IOKit/IOKitLib.h>
#import <IOKit/IOMessage.h>

#import <exception>

FWUtil_Notifier::Matcher::Matcher()
: FWLib::Matcher( ::IOServiceMatching( "IOFireWireNub" ) ),
  mNotificationPort( 0 ),
  mNotifications( 0 )
{
	mNotifications = ::CFDictionaryCreateMutable( kCFAllocatorDefault, 0, nil, nil ) ;
	if ( !mNotifications )
		throw std::exception() ;
		
	mach_port_t		masterPort = 0 ;
	IOReturn 		error = IOMasterPort(MACH_PORT_NULL, & masterPort) ;
	if ( !error )
		if (0 == ( mNotificationPort = ::IONotificationPortCreate( masterPort ) ) )
		{
			cerr << "(!) Failed to create notification port!\n" ;
			error = kIOReturnError ;	// there may be a better error code we can return
		}

	//
	// Get a run loop event source from the notification port
	// and add it to our (default) runloop
	//
	if ( !error )
	{		
		CFRunLoopSourceRef source = :: IONotificationPortGetRunLoopSource( mNotificationPort );
		::CFRunLoopAddSource( ::CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode );
		
		// the run loop has the source, so we release it.
		::CFRelease( source ) ;
	}
	
	Init() ;
}

void
FWUtil_Notifier::Matcher::Match( const io_service_t service )
{
	io_object_t	notification ;

	::IOServiceAddInterestNotification( mNotificationPort, service, kIOGeneralInterest, & DeviceInterestCallback, this, & notification ) ;

	gGlobals.AddService( service ) ;

	::CFDictionaryAddValue( mNotifications, reinterpret_cast<const void*>( service ), reinterpret_cast<const void*>( notification ) ) ;
}

void
FWUtil_Notifier::Matcher::DeviceInterestCallback( void * refcon, io_service_t service, natural_t messageType, void * messageArgument )
{
	Matcher*	me = reinterpret_cast<Matcher*>(refcon) ;

	switch(messageType)
	{
		case kIOMessageServiceIsRequestingClose:
		case kIOFWMessageServiceIsRequestingClose:
		case kIOMessageServiceIsTerminated:
		
			cout << "\n***device was removed\n" ;
			
			::CFDictionaryRemoveValue( me->mNotifications, (void*)service ) ;			

			gGlobals.ReleaseInterface() ;

			gGlobals.RemoveService( service ) ;

			break ;

		default:
			break ;			
	}
}
	

// ============================================================
// FWUtil_Notifier
// ============================================================

FWUtil_Notifier::FWUtil_Notifier()
{
	pthread_create(& mPThread, 0/*& attr*/, FWUtil_Notifier::main, this) ;
}

FWUtil_Notifier::~FWUtil_Notifier()
{
	pthread_cancel(mPThread) ;
}

void*
FWUtil_Notifier::main(void* param)
{
	FWUtil_Notifier*	me = reinterpret_cast<FWUtil_Notifier*>( param ) ;
	me->mMatcher = new Matcher() ;

	::CFRunLoopRun() ;
	
	return 0 ;
}
