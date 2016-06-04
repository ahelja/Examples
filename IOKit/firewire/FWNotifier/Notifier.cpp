/*
	File:		Notifer/Notifier.cpp

	Synopsis: 	This is a short code fragment demonstrating notification for added 
				and removed devices. It uses two types of notification to see
				device removals, matching and service interest notifications. You
				can use whichever is more convenient in your own code.

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

	$Log: Notifier.cpp,v $
	Revision 1.5  2005/02/03 00:54:51  firewire
	*** empty log message ***
	
	Revision 1.4  2005/01/31 23:38:20  firewire
	*** empty log message ***
	
	Revision 1.3  2004/06/10 22:01:37  firewire
	*** empty log message ***
	
	Revision 1.2  2004/03/25 23:14:42  firewire
	*** empty log message ***
	
	Revision 1.1  2004/03/03 23:53:39  firewire
	*** empty log message ***
	
	Revision 1.3  2003/03/18 00:29:37  firewire
	fix readme
	
	Revision 1.2  2002/11/13 21:41:48  noggin
	fix build failure
	
	Revision 1.1  2002/11/07 00:32:26  noggin
	move to new repository
	

2/25/2002	NWG		Took out CFRelease() call after CFRunLoopAddSource(). Calling CFRelease() is not good.
2/20/2002	NWG		You no longer need to use Open() before using the user client
					config ROM browsing API. This sample has been changed to reflect this.

*/

#import <Carbon/Carbon.h>
#import <IOKit/IOMessage.h>

#import "FWLib_Listener.h"
#import "FWLib_Device.h"
#import "FWLib_Matcher.h"

CFMutableArrayRef gListeners = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, NULL ) ;
CFMutableArrayRef gDevices = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, NULL ) ;

#pragma mark -
#pragma mark Listener

class Listener : public FWLib::Listener
{
	public :

		Listener( io_service_t service )
		: FWLib::Listener( service )
		{
		}

		virtual void
		Message( io_service_t service, natural_t messageType, void* message )
		{
			printf( "service 0x%x says ", service ) ;

			switch( messageType )
			{
				case kIOMessageServiceIsTerminated:
					// handle device removed here
					printf( "kIOMessageServiceIsTerminated" ) ;
					break ;
				case kIOMessageServiceIsSuspended:
					// handle bus reset begin here
					printf( "kIOMessageServiceIsSuspended (bus reset begin)" ) ;
					break ;
				case kIOMessageServiceIsResumed:
					// handle bus reset done
					printf( "kIOMessageServiceIsResumed (bus reset end)" ) ;
					break ;
					
				case kIOMessageServiceIsRequestingClose:
					printf( "kIOMessageServiceIsRequestingClose" ) ;

					#if TEST_WITH_OPEN
					
					FWLib::Device * device = DeviceForService( service ) ;
					if( device )
					{
						device->Close() ;
					}
					
					#endif
					
					break ;
					
				//
				// the more esoteric messages:
				//
				
				case kIOMessageServiceIsAttemptingOpen:
					printf( "kIOMessageServiceIsAttemptingOpen" ) ;
					break ;

				case kIOMessageServiceWasClosed:
					printf( "kIOMessageServiceWasClosed" ) ;
					break ;
					
				case kIOMessageServiceBusyStateChange:
					printf( "kIOMessageServiceBusyStateChange" ) ;
					break ;
					
				case kIOMessageCanDevicePowerOff:
					printf( "kIOMessageCanDevicePowerOff" ) ;
					break ;

				case kIOMessageDeviceWillPowerOff:
					printf( "kIOMessageDeviceWillPowerOff" ) ;
					break ;

				case kIOMessageDeviceWillNotPowerOff:
					printf( "kIOMessageDeviceWillPowerOff" ) ;
					break ;

				case kIOMessageDeviceHasPoweredOn:
					printf( "kIOMessageDeviceHasPoweredOn" ) ;
					break ;

				case kIOMessageCanSystemPowerOff:
					printf( "kIOMessageCanSystemPowerOff" ) ;
					break ;

				case kIOMessageSystemWillPowerOff:
					printf( "kIOMessageSystemWillPowerOff" ) ;
					break ;

				case kIOMessageSystemWillNotPowerOff:
					printf( "kIOMessageSystemWillNotPowerOff" ) ;
					break ;

				case kIOMessageCanSystemSleep:
					printf( "kIOMessageCanSystemSleep" ) ;
					break ;

				case kIOMessageSystemWillSleep:
					printf( "kIOMessageSystemWillSleep" ) ;
					break ;

				case kIOMessageSystemWillNotSleep:
					printf( "kIOMessageSystemWillNotSleep" ) ;
					break ;

				case kIOMessageSystemHasPoweredOn:
					printf( "kIOMessageSystemHasPoweredOn" ) ;
					break ;

				case kIOFWMessageServiceIsRequestingClose:
					printf( "kIOFWMessageServiceIsRequestingClose" ) ;
					break ;

				default:
					printf( "0x%x", messageType ) ;
					break ;
			}
			
			printf("\n") ;
		}
		
		FWLib::Device *
		DeviceForService( io_service_t service )
		{
			FWLib::Device * result = 0 ;
			
			for( unsigned index=0, count = ::CFArrayGetCount( gDevices ); index < count; ++index )
			{
				FWLib::Device * device = reinterpret_cast<FWLib::Device*>( const_cast<void *>( ::CFArrayGetValueAtIndex( gDevices, index ) ) ) ;
				if ( device->Service() == service )
				{
					result = device ;
				}
					
			}
			
			return result ;
		}

} ;

#pragma mark -
#pragma mark Matcher

class Matcher : public FWLib::Matcher
{
	public :
	
		Matcher( CFMutableDictionaryRef matching )
		: FWLib::Matcher( matching )				
		{
		}
		
		virtual void
		Match( const io_service_t service )
		{
			printf( "service 0x%x found.\n", service ) ;

			io_name_t className ;
			IOReturn error ;
			
			error = ::IOObjectGetClass( service, className ) ;
			assert( !error ) ;
			
			printf( "class is %s\n", className ) ;
			
			//
			// We set up to get interest notification for the device just added. This allows us
			// to find out when this device has been removed. (Among other things)
			// The refCon value set above with IOServiceAddMatchingNotification() was used 
			// to pass a pointer to MyDeviceInterestCallback.
			//

			Listener * listener = new Listener( service ) ;
			::CFArrayAppendValue( gListeners, listener ) ;

			{
				FWLib::Device * device = new FWLib::Device( service ) ;
				
				device->Open() ;
			}
		}
} ;

#pragma mark -
#pragma mark main

int main()
{
	Matcher * matcher = new Matcher( ::IOServiceMatching( "IOFireWireNub" ) ) ;
	matcher->Init() ;
	
	RunApplicationEventLoop() ;
	
	delete matcher ;
}
