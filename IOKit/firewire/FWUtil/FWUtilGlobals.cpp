/*
	File:		FWUtilGlobals.cpp

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

#import <pthread.h>
#import <CoreFoundation/CoreFoundation.h>
#import <IOKit/IOKitLib.h>

#import "FWUtilGlobals.h"
#import "FWUtilThreads.h"
#import "FWUtilCommand.h"

#import <FWLib_Device.h>

const FWUtilCommandInfo	FWUtilGlobals::kBuiltInCommands[ ] =
{
	FWUtilCommandArrayEntry(FWUtilCommand_help, "help"),
	FWUtilCommandArrayEntry(FWUtilCommand_attach, "attach"),
	FWUtilCommandArrayEntry(FWUtilCommand_bread, "bread"),
	FWUtilCommandArrayEntry(FWUtilCommand_busreset, "busreset"),
	FWUtilCommandArrayEntry(FWUtilCommand_bwrite, "bwrite"),
	FWUtilCommandArrayEntry(FWUtilCommand_services, "services"),
	FWUtilCommandArrayEntry(FWUtilCommand_history, "history"),
	FWUtilCommandArrayEntry(FWUtilCommand_info, "info"),
	FWUtilCommandArrayEntry(FWUtilCommand_lock, "lock"),
	FWUtilCommandArrayEntry(FWUtilCommand_qread, "qread"),
	FWUtilCommandArrayEntry(FWUtilCommand_qwrite, "qwrite"),
	FWUtilCommandArrayEntry(FWUtilCommand_quit, "quit"),
	FWUtilCommandArrayEntry(FWUtilCommand_repeat, "repeat")
} ;
const UInt32			FWUtilGlobals::kBuiltInCommandsCount = sizeof(FWUtilGlobals::kBuiltInCommands) / sizeof(FWUtilCommandInfo) ;

FWUtilGlobals::FWUtilGlobals()
: mDevices( ::CFArrayCreateMutable( kCFAllocatorDefault, 0, nil ) ),
  mCommandConstructors( ::CFArrayCreateMutable( kCFAllocatorDefault, 0, nil ) ),
  mCommandNames( ::CFArrayCreateMutable( kCFAllocatorDefault, 0, nil ) ),
  mNotifier()
{
	pthread_mutex_init( & mDevicesLock, nil ) ;
	pthread_mutex_init( & gGlobals.mDevicesLock, nil ) ;

	// init user client
	if ( ! mDevices )
		throw std::exception() ;
		
	// init command constructors
	for(UInt32 index=0; index < kBuiltInCommandsCount ; index++)
	{
		CFArrayAppendValue( mCommandConstructors, (const void*)kBuiltInCommands[index].creator ) ;
		CFArrayAppendValue( mCommandNames, kBuiltInCommands[index].name ) ;
	}
}

FWUtilGlobals::~FWUtilGlobals()
{
	pthread_mutex_destroy(& mDevicesLock) ;

	cout << "cleaning up global data...\n" ;
	
	if (mCommandConstructors)
		CFRelease(mCommandConstructors) ;
		
	if (mCommandNames)
		CFRelease(mCommandNames) ;
	
	if (mDevices)
		CFRelease(mDevices) ;
	
	ReleaseInterface() ;
	
	cout << "...done!\n" ;
}

void
FWUtilGlobals::ReleaseInterface()
{
	if (gGlobals.mDevice)
	{
		gGlobals.mDevice->Close() ;
		gGlobals.mDevice->RemoveCallbackDispatcherFromRunLoop() ;
	}
	
	delete gGlobals.mDevice ; 
	gGlobals.mDevice = nil ;
}

IOReturn
FWUtilGlobals::NewInterfaceWithService( io_object_t service )
{
	IOReturn err = kIOReturnSuccess ;
	
	gGlobals.mDevice = new Device( service ) ;
	
	if (!gGlobals.mDevice)
		err = kIOReturnError ;
	
	if (kIOReturnSuccess == err)
		gGlobals.mDevice->AddCallbackDispatcherToRunLoop(CFRunLoopGetCurrent()) ;

	if (kIOReturnSuccess != err)
	{
		delete gGlobals.mDevice ;
		gGlobals.mDevice = nil ;
	}
	return err ;
}

const CFArrayRef
FWUtilGlobals::CopyDevices()
{
	pthread_mutex_lock( & gGlobals.mDevicesLock ) ;
	CFArrayRef	resultArray = CFArrayCreateCopy( kCFAllocatorDefault, gGlobals.mDevices ) ;
	pthread_mutex_unlock( & gGlobals.mDevicesLock ) ;
	
	return resultArray ;
}

void
FWUtilGlobals::AddService( io_service_t service )
{
	pthread_mutex_lock( & gGlobals.mDevicesLock ) ;

	::CFArrayAppendValue( gGlobals.mDevices, reinterpret_cast<void*>( service ) ) ;

	pthread_mutex_unlock( & gGlobals.mDevicesLock ) ;
}

void
FWUtilGlobals::RemoveService( io_service_t service )
{
	pthread_mutex_lock( & gGlobals.mDevicesLock ) ;

	CFIndex	index ;
	CFRange	range = CFRangeMake(0, CFArrayGetCount(gGlobals.mDevices)-1) ;
	
	while ( kCFNotFound != ( index = ::CFArrayGetFirstIndexOfValue( gGlobals.mDevices, 
			range, reinterpret_cast<const void*>(service) ) ) )
	{
		::CFArrayRemoveValueAtIndex( gGlobals.mDevices, index ) ;
	}
	
	pthread_mutex_unlock( & gGlobals.mDevicesLock ) ;
}


#pragma mark -
// ============================================================
// FWUtilBlockIndenter
// ============================================================

FWUtilBlockIndenter 
FWUtilBlockIndenter::operator++(int)
{	
	FWUtilBlockIndenter result(*this) ;	// post operator
	operator++() ;
	return result;
}

FWUtilBlockIndenter
FWUtilBlockIndenter::operator++()
{
	mIndenter++; 
	return *this;
}

FWUtilBlockIndenter
FWUtilBlockIndenter::operator--(int)
{
	FWUtilBlockIndenter result(*this) ;	// post operator
	operator++() ;
	return result;
}
FWUtilBlockIndenter
FWUtilBlockIndenter::operator--()
{
	mIndenter++;
	return *this ;
}

// ============================================================
// FWUtilIndenter
// ============================================================

FWUtilIndenter
FWUtilIndenter::operator++(int)
{	
	FWUtilIndenter result(*this) ;	// post operator
	operator++(); 
	return result;
}
											
FWUtilIndenter
FWUtilIndenter::operator++()
{	
	if (strlen(mIndentString) < 250)
		strcat(mIndentString, "    ");
	return *this;
}
										
FWUtilIndenter
FWUtilIndenter::operator--(int)
{
	FWUtilIndenter result(*this) ;
	operator--(); 
	return result; }
										
FWUtilIndenter
FWUtilIndenter::operator--()
{	
	if (strlen(mIndentString) > 4)
		mIndentString[strlen(mIndentString) - 4] = 0 ;
	else
		mIndentString[0] = 0 ; 
	return *this;
}

ostream & operator<<(ostream & outs, const class FWUtilIndenter* me)
{
	return (outs << me->mIndentString) ;
}

ostream & operator<<(ostream & outs, const FWUtilIndenter & me)
{
	return (outs << me.mIndentString) ;
}

ostream & operator<<(ostream & outs, const FWUtilBlockIndenter & indenter)
{
	CFArrayRef	linesArray = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, indenter.mString, CFSTR("\n")) ;
	
	if ( ( linesArray ) && ( CFArrayGetCount( linesArray ) > 0 ) )
	{
		outs << indenter.mIndenter << (CFStringRef) CFArrayGetValueAtIndex(linesArray, 0) ;
	
		for( CFIndex index = 1, count = CFArrayGetCount( linesArray ); index < count; ++index )
		{
			outs << endl << indenter.mIndenter << (CFStringRef) CFArrayGetValueAtIndex(linesArray, index) ;
		}
		
		CFRelease( linesArray ) ;
	}
	
	return outs ;
}

// this must come last in the file...
FWUtilGlobals	gGlobals ;
