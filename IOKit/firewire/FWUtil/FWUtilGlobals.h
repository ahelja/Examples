/*
	File:		FWUtilGlobals.h

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

#import <IOKit/firewire/IOFireWireLib.h>

#import <iostream>

#import "FWUtilThreads.h"

class FWUtilGlobals ;
class FWUtilCommandInfo ;
class FWUtilIndenter ;

namespace FWLib {
	class Device ;
}

using namespace std ;
using namespace FWLib ;

extern FWUtilGlobals	gGlobals ;

#pragma mark -
class FWUtilBlockIndenter
{
	friend ostream & operator<<( ostream & outs, const FWUtilBlockIndenter & indenter ) ;

	public:

		FWUtilBlockIndenter( FWUtilIndenter & i, CFStringRef string )
				: mIndenter( i ), mString( string )							{ ::CFRetain( string ) ; }
		~FWUtilBlockIndenter() 												{ if ( mString ) ::CFRelease( mString ) ; }
	
	// --- operators ---------------------------
	FWUtilBlockIndenter		operator++(int) ;
	FWUtilBlockIndenter 	operator++() ;
	FWUtilBlockIndenter 	operator--(int) ;
	FWUtilBlockIndenter 	operator--() ;
											
 protected:
	FWUtilIndenter & 	mIndenter ;
	CFStringRef			mString ;
} ;

#pragma mark -
class FWUtilIndenter
{
		friend ostream & operator<<( ostream &, const class FWUtilIndenter * ) ;
		friend ostream & operator<<( ostream &, const FWUtilIndenter & ) ;
	
	protected:

		char	mIndentString[256] ;

	public:

		FWUtilIndenter()												{ strcpy( mIndentString, "" ) ; }
		FWUtilIndenter( const FWUtilIndenter & i )						{ strcpy( mIndentString, i.mIndentString ) ; }
	
		// --- utils -------------------------------
		FWUtilBlockIndenter 	BlockIndent( CFStringRef str )			{ return FWUtilBlockIndenter( *this, str ) ; }
										
		// --- operators ---------------------------
		FWUtilIndenter 			operator++(int);
		FWUtilIndenter 			operator++();
		FWUtilIndenter 			operator--(int);
		FWUtilIndenter 			operator--();
} ;

#pragma mark -
class FWUtilGlobals
{
	friend class FWUtilCommand ;

	private:

		static const FWUtilCommandInfo		kBuiltInCommands[ ] ;
		static const UInt32					kBuiltInCommandsCount ;
		
		pthread_mutex_t				mDevicesLock ;
		CFMutableArrayRef			mDevices ;
		CFMutableArrayRef			mCommandConstructors ;
		CFMutableArrayRef			mCommandNames ;
		Device*						mDevice ;
		mutable FWAddress			mLastAddress ;
		FWUtilIndenter				mIndenter ;
		FWUtil_Notifier				mNotifier ;

	public:
	
		FWUtilGlobals	() ;
		~FWUtilGlobals	() ;

	public:
	
		// --- getters -----------------------------
		Device *						GetDeviceInterface() 								{ return mDevice; }
		void							ReleaseInterface() ;
		IOReturn						NewInterfaceWithService( io_object_t service ) ;
		const CFArrayRef				GetCommandConstructors() 							{ return mCommandConstructors ; }
		const CFArrayRef				GetCommandNames()									{ return mCommandNames ; }
		const CFArrayRef				CopyDevices() ;
		FWUtilIndenter &				GetIndenter() 										{ return mIndenter; }
		
		void							AddService( io_service_t service ) ;
		void							RemoveService( io_service_t service ) ;
	
		void							SetLastAddress( FWAddress& addr )					{ mLastAddress = addr ; }
		const FWAddress &				GetLastAddress() const								{ return mLastAddress ; }
} ;

ostream & operator<<( ostream & outs, const FWUtilBlockIndenter & i ) ;
ostream & operator<<( ostream & outs, const class FWUtilIndenter * me ) ;
ostream & operator<<( ostream & outs, const FWUtilIndenter & me ) ;

typedef FWUtilBlockIndenter (*BlockIndentFunction)( CFStringRef ) ;
