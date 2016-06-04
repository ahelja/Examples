/*
	File:		SBP2SampleLibGlue.cpp

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.
	
	Written by:	cpieper

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

#include <Carbon/Carbon.h> 

#include "FWDebugging.h"
#include "SBP2SampleSBP2LibGlue.h"

// ctor / dtor
//
//

SBP2SampleSBP2LibGlue::SBP2SampleSBP2LibGlue( void )
{
	fSBP2CFPlugInInterface 	= NULL;
	fSBP2LUNInterface 		= NULL;
	fSBP2LoginInterface 	= NULL;
	fLUNResetORBInterface 	= NULL;
}

SBP2SampleSBP2LibGlue::~SBP2SampleSBP2LibGlue()
{
}

// createORB
//
// orb factory

SBP2SampleORB * SBP2SampleSBP2LibGlue::createORB( void )
{
    IOReturn		status = kIOReturnSuccess;
	
	SBP2SampleORB *  me = NULL;
	IOFireWireSBP2LibORBInterface** orbInterface = NULL;
	
	//
	// create orb
	//
	
	if( status == kIOReturnSuccess )
	{
		orbInterface = (IOFireWireSBP2LibORBInterface**)
							(*fSBP2LoginInterface)->createORB( fSBP2LoginInterface, CFUUIDGetUUIDBytes(kIOFireWireSBP2LibORBInterfaceID) );
		if( orbInterface == NULL )
			status = kIOReturnError;
	}
		
	if( status == kIOReturnSuccess )
	{
		me = new SBP2SampleORB;
		if( me == NULL )
			status = kIOReturnError;
	}
		
	if( status == kIOReturnSuccess )
	{
		me->retain();
		status = me->init( orbInterface );
	}
	
	if( status != kIOReturnSuccess && me != NULL )
	{
		me->release();
		me = NULL;
	}
	
	return me;
}

// start
//
//

IOReturn SBP2SampleSBP2LibGlue::start( CFDictionaryRef propertyTable, io_service_t service )
{
	IOReturn status = kIOReturnSuccess;
	SInt32 score = 0;
	IOFireWireSessionRef sessionRef = 0;
	
	FWLOG(( "SBP2SampleSBP2LibGlue : start\n" ));
	
	if( status == kIOReturnSuccess )
	{
		status = IOCreatePlugInInterfaceForService( service,
													  kIOFireWireSBP2LibTypeID, 
													  kIOCFPlugInInterfaceID,
													  &fSBP2CFPlugInInterface,
													  &score );	
		FWLOG(( "SBP2SampleSBP2LibGlue : IOCreatePlugInInterfaceForService for SBP2 status = 0x%08x\n", status ));
	}
	
	if( status == kIOReturnSuccess )
	{
		HRESULT res;
		res = (*fSBP2CFPlugInInterface)->QueryInterface( fSBP2CFPlugInInterface, 
										CFUUIDGetUUIDBytes(kIOFireWireSBP2LibLUNInterfaceID),
										(void **) &fSBP2LUNInterface );
		
		if( res != S_OK )
			status = kIOReturnError;
	}
		
	// open
	if( status == kIOReturnSuccess )
	{
		status = (*fSBP2LUNInterface)->open( fSBP2LUNInterface );
	}
	
	// getSessionRef
	if( status == kIOReturnSuccess )
	{
		sessionRef = (*fSBP2LUNInterface)->getSessionRef( fSBP2LUNInterface );
	}
	
	////////////////////////////

#if 0
	io_registry_entry_t parent = 0;

	if( status == kIOReturnSuccess )
	{
		bool found = false;
		io_registry_entry_t child = service;
		while( 	!found &&
				IORegistryEntryGetParentEntry( child, kIOServicePlane, &parent ) == kIOReturnSuccess &&
				parent != NULL )
		{
			char name[256];
			
			IORegistryEntryGetName( parent, name );
			FWLOG(( "SBP2SampleSBP2LibGlue : parent = %s\n", name ));

			if( !strcmp( name, "IOFireWireDevice" ) )
				found = true;
		
			child = parent; // next level
		}
		
		if( !found )
			status = kIOReturnError;
	}
	
	if( status == kIOReturnSuccess )
	{
		status = IOCreatePlugInInterfaceForService( parent,
													  kIOFireWireLibTypeID, 
													  kIOCFPlugInInterfaceID,
													  &fFWCFPlugInInterface,
													  &score );	
													  
		FWLOG(( "SBP2SampleSBP2LibGlue : IOCreatePlugInInterfaceForService for FW status = 0x%08x\n", status ));
	}
	
	if( status == kIOReturnSuccess )
	{
		HRESULT res;
		res = (*fFWCFPlugInInterface)->QueryInterface( fFWCFPlugInInterface, 
										CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID),
										(void **) &fFWDeviceInterface );
		
		if( res != S_OK )
			status = kIOReturnError;
	}
		
	// open
	if( status == kIOReturnSuccess )
	{
		status = (*fFWDeviceInterface)->OpenWithSessionRef( fFWDeviceInterface, sessionRef );
	}

#endif

	////////////////////////////
	
	//
	// create login
	//
	
	if( status == kIOReturnSuccess )
	{
		fSBP2LoginInterface = (IOFireWireSBP2LibLoginInterface 	**)
								(*fSBP2LUNInterface)->createLogin( fSBP2LUNInterface, CFUUIDGetUUIDBytes(kIOFireWireSBP2LibLoginInterfaceID) );
		if( fSBP2LoginInterface == NULL )
			status = kIOReturnError;
	}
		
	// 
	// set callbacks
	//
			
	if( status == kIOReturnSuccess )
	{
		(*fSBP2LoginInterface)->setMaxPayloadSize( fSBP2LoginInterface, 2048 );
		(*fSBP2LoginInterface)->setLoginCallback( fSBP2LoginInterface, 
													this, staticLoginCompletion );
		(*fSBP2LoginInterface)->setLogoutCallback( fSBP2LoginInterface, 
													this, staticLogoutCompletion );
		(*fSBP2LoginInterface)->setUnsolicitedStatusNotify( fSBP2LoginInterface, 
												this, staticUnsolicitedStatusNotify );
		(*fSBP2LoginInterface)->setStatusNotify( fSBP2LoginInterface, 
													this, staticStatusNotify );
		(*fSBP2LoginInterface)->setFetchAgentResetCallback( fSBP2LoginInterface, 
													this, staticFetchAgentResetCompletion );
		(*fSBP2LoginInterface)->setFetchAgentWriteCallback( fSBP2LoginInterface, 
													this, staticFetchAgentWriteCompletion );
	}

	//
	// create management orb
	//
	
	if( status == kIOReturnSuccess )
	{
		fLUNResetORBInterface = (IOFireWireSBP2LibMgmtORBInterface**)(*fSBP2LUNInterface)->createMgmtORB( fSBP2LUNInterface, CFUUIDGetUUIDBytes(kIOFireWireSBP2LibMgmtORBInterfaceID) );
		if( fLUNResetORBInterface == NULL )
			status = kIOReturnError;
	}
	
	if( status == kIOReturnSuccess )
	{
		(*fLUNResetORBInterface)->setORBCompleteCallback( fLUNResetORBInterface, 
													this, staticLUNResetCompletion );
	}

	if( status == kIOReturnSuccess )
	{
		status = (*fLUNResetORBInterface)->setCommandFunction( fLUNResetORBInterface, kFWSBP2LogicalUnitReset );
	}

	if( status == kIOReturnSuccess )
	{
		status = (*fLUNResetORBInterface)->setManageeLogin( fLUNResetORBInterface, fSBP2LoginInterface );
	}
	
	FWLOG(( "SBP2SampleSBP2LibGlue : start return status = 0x%08x\n", status ));
	
	return status;
}

// stop
//
//

IOReturn SBP2SampleSBP2LibGlue::stop( void )
{
	FWLOG(( "SBP2SampleSBP2LibGlue : stop\n" ));

	// undo addCallbackDispatcherToRunLoop
	(*fSBP2LUNInterface)->removeCallbackDispatcherFromRunLoop( fSBP2LUNInterface );

	if( fLUNResetORBInterface != NULL )
	{
		(*fLUNResetORBInterface)->Release(fLUNResetORBInterface);
		fLUNResetORBInterface = NULL;
	}
	
	if( fSBP2LoginInterface != NULL )
	{
		(*fSBP2LoginInterface)->Release(fSBP2LoginInterface);
		fSBP2LoginInterface = NULL;
	}
	
#if 0
	
	// release device interface
	if( fFWDeviceInterface != NULL )
	{
		(*fFWDeviceInterface)->Close( fFWDeviceInterface );
		(*fFWDeviceInterface)->Release(fFWDeviceInterface);
	}
	fFWDeviceInterface = NULL;
		
	// release plugin interface (calls stop)
	if( fFWCFPlugInInterface != NULL )
	{	
		IOReturn status = IODestroyPlugInInterface(fFWCFPlugInInterface);
		FWLOG(( "SBP2SampleSBP2LibGlue : IODestroyPlugInInterface for FW status = 0x%08x\n", status ));
	}
	fFWCFPlugInInterface = NULL;
	
#endif
		
	// release LUN interface
	if( fSBP2LUNInterface != NULL )
	{
		(*fSBP2LUNInterface)->close( fSBP2LUNInterface );
		(*fSBP2LUNInterface)->Release(fSBP2LUNInterface);
	}
	fSBP2LUNInterface = NULL;
		
	// release plugin interface (calls stop)
	if( fSBP2CFPlugInInterface != NULL )
	{	
		IOReturn status = IODestroyPlugInInterface(fSBP2CFPlugInInterface);
		FWLOG(( "SBP2SampleSBP2LibGlue : IODestroyPlugInInterface for SBP2 status = 0x%08x\n", status ));
	}
	fSBP2CFPlugInInterface = NULL;

	return kIOReturnSuccess;
}

//////////////////////////////////////////////////////////////////
// SBP2SampleDriverInterface methods

// setUpCallbacksWithRunLoop
//
// sets up the async dispatcher and a few callback routines

IOReturn SBP2SampleSBP2LibGlue::setUpCallbacksWithRunLoop( CFRunLoopRef cfRunLoopRef )
{
	IOReturn status = kIOReturnSuccess;
	
	FWLOG(( "SBP2SampleSBP2LibGlue : setUpCallbacksWithRunLoop\n"));

	if( status == kIOReturnSuccess )
	{
		(*fSBP2LUNInterface)->setMessageCallback( fSBP2LUNInterface, this, 
																	&staticMessage );
		status = (*fSBP2LUNInterface)->addCallbackDispatcherToRunLoop( 	fSBP2LUNInterface,
																		cfRunLoopRef );
	}
	
	return status;
}

//////////////////////////////////////////////////////////////////
// Callback methods

// loginComplete
//
//

void SBP2SampleSBP2LibGlue::staticLoginCompletion( void * self, FWSBP2LoginCompleteParams * params )
{
	((SBP2SampleSBP2LibGlue*)self)->loginCompletion(params);
}

void SBP2SampleSBP2LibGlue::loginCompletion( FWSBP2LoginCompleteParams * params )
{
}


// message
//
//

void SBP2SampleSBP2LibGlue::staticMessage( void * self, UInt32 type, void * arg )
{
	((SBP2SampleSBP2LibGlue*)self)->message(type, arg);
}

void SBP2SampleSBP2LibGlue::message( UInt32 type, void * arg )
{
}

// logoutComplete
//
//

void SBP2SampleSBP2LibGlue::staticLogoutCompletion( void * self, FWSBP2LogoutCompleteParams * params )
{
	((SBP2SampleSBP2LibGlue*)self)->logoutCompletion(params);
}

void SBP2SampleSBP2LibGlue::logoutCompletion( FWSBP2LogoutCompleteParams * params )
{
}

// statusNotify
//
//

void SBP2SampleSBP2LibGlue::staticStatusNotify( void * self, FWSBP2NotifyParams * params )
{
	((SBP2SampleSBP2LibGlue*)self)->statusNotify( params );	
}

void SBP2SampleSBP2LibGlue::statusNotify( FWSBP2NotifyParams * params )
{
}

// unsolicitedStatusNotify
//
//

void SBP2SampleSBP2LibGlue::staticUnsolicitedStatusNotify( void * self, FWSBP2NotifyParams * params )
{
	((SBP2SampleSBP2LibGlue*)self)->unsolicitedStatusNotify( params );	
}

void SBP2SampleSBP2LibGlue::unsolicitedStatusNotify( FWSBP2NotifyParams * params )
{
	// reenable unsolicited status
	(*fSBP2LoginInterface)->enableUnsolicitedStatus( fSBP2LoginInterface );
}

// LUNResetCompletion
//
//

void SBP2SampleSBP2LibGlue::staticLUNResetCompletion( void * self, IOReturn status, void * orb )
{
	((SBP2SampleSBP2LibGlue*)self)->LUNResetCompletion( status, orb );	
}

void SBP2SampleSBP2LibGlue::LUNResetCompletion( IOReturn status, void * orb )
{
}

// fetchAgentResetCompletion
//
//

void SBP2SampleSBP2LibGlue::staticFetchAgentResetCompletion( void * self, IOReturn status )
{
	((SBP2SampleSBP2LibGlue*)self)->fetchAgentResetCompletion( status );	
}

void SBP2SampleSBP2LibGlue::fetchAgentResetCompletion( IOReturn status )
{
}

// fetchAgentWriteCompletion
//
//

void SBP2SampleSBP2LibGlue::staticFetchAgentWriteCompletion( void * self, IOReturn status, void * refCon )
{
	((SBP2SampleSBP2LibGlue*)self)->fetchAgentWriteCompletion( status, refCon );	
}

void SBP2SampleSBP2LibGlue::fetchAgentWriteCompletion( IOReturn status, void * refCon )
{
}
