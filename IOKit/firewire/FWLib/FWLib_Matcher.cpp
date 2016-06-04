/*
	File:		FWLib_Matcher.cpp

	Synopsis: 	"Matcher" C++ class to help you handle hot-plug events with ease.

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

	$Log: FWLib_Matcher.cpp,v $
	Revision 1.4  2004/03/03 23:45:18  niels
	*** empty log message ***
	
	Revision 1.3  2004/02/06 23:29:13  firewire
	*** empty log message ***
	
	Revision 1.2  2003/05/27 18:12:46  firewire
	SDK16
	
	Revision 1.1  2002/11/07 00:32:13  noggin
	move to new repository
	
	Revision 1.3  2002/10/14 22:24:55  noggin
	SDK14
	
	Revision 1.2  2002/08/21 22:26:02  noggin
	*** empty log message ***
	
	Revision 1.1  2002/06/12 02:30:00  noggin
	first commit
	
*/

#import "FWLib_Matcher.h"
#import <exception>
#import <IOKit/IOKitLib.h>

namespace FWLib {
	
	Matcher::Matcher( CFMutableDictionaryRef matching, const io_name_t type, CFRunLoopRef runloop)
	: mMatching( matching ),
	  mType( type ),
	  mRunLoop( runloop ),
	  mNotificationPort(0)
	{
		::CFRetain(matching) ;
		::CFRetain(runloop) ;
	
		mach_port_t		masterPort ;
		kern_return_t	err = ::IOMasterPort( MACH_PORT_NULL, & masterPort ) ;
	
		if ( err )		
			throw std::exception() ;
	
		mNotificationPort = ::IONotificationPortCreate( masterPort );	
	}
	
	Matcher::~Matcher()
	{
		if (mRunLoop)
			::CFRelease(mRunLoop) ;
		if (mMatching)
			::CFRelease(mMatching) ;
		if (mNotificationPort)
			IONotificationPortDestroy( mNotificationPort ) ;
	}
	
	void
	Matcher::Init()
	{
		io_iterator_t	iterator ;
		IOReturn error ;
		
		error = ::IOServiceAddMatchingNotification( mNotificationPort, mType, mMatching, S_Match, this, & iterator ) ;
		if ( error )
		{
			throw error ;
		}
		
		CFRunLoopSourceRef source = ::IONotificationPortGetRunLoopSource( mNotificationPort ) ;
		if (!source)
			throw std::exception() ;
		
		::CFRunLoopAddSource( mRunLoop, source, kCFRunLoopDefaultMode ) ;		

		S_Match( this, iterator ) ;
	}

	void
	Matcher :: Match ( const io_service_t service )
	{
	}
	
	
	void
	Matcher :: S_Match ( void * refcon, io_iterator_t iterator )
	{
		Matcher* me = reinterpret_cast<Matcher*>(refcon) ;
	
		io_service_t		service = ::IOIteratorNext( iterator ) ;
		while ( service )
		{
			me->Match( service ) ;
			service = ::IOIteratorNext( iterator ) ;
		}
	}	
}
