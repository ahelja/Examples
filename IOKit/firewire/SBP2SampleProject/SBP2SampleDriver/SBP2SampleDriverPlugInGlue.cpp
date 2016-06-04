/*
	File:		SBP2SampleDriverPlugInGlue.cpp

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
#include "SBP2SampleDriverPlugInGlue.h"

// static interface table for IOCFPlugInInterface
IOCFPlugInInterface SBP2SampleDriverPlugInGlue::sIOCFPlugInInterface = 
{
    0,
	&SBP2SampleDriverPlugInGlue::staticQueryInterface,
	&SBP2SampleDriverPlugInGlue::staticAddRef,
	&SBP2SampleDriverPlugInGlue::staticRelease,
	1, 0, // version/revision
	&SBP2SampleDriverPlugInGlue::staticProbe,
	&SBP2SampleDriverPlugInGlue::staticStart,
	&SBP2SampleDriverPlugInGlue::staticStop
};

// static interface table for SBP2SampleDriverInterface
SBP2SampleDriverInterface	SBP2SampleDriverPlugInGlue::sSBP2SampleDriverInterface =
{
    0,
	&SBP2SampleDriverPlugInGlue::staticQueryInterface,
	&SBP2SampleDriverPlugInGlue::staticAddRef,
	&SBP2SampleDriverPlugInGlue::staticRelease,
	1, 0, // version/revision
	&SBP2SampleDriverPlugInGlue::staticSetUpCallbacksWithRunLoop,
	&SBP2SampleDriverPlugInGlue::staticLoginToDevice,
	&SBP2SampleDriverPlugInGlue::staticLogoutOfDevice,
	&SBP2SampleDriverPlugInGlue::staticReadBlock,
	&SBP2SampleDriverPlugInGlue::staticGetNewTransactionID,
	&SBP2SampleDriverPlugInGlue::staticAbortReadsWithTransactionID,
	&SBP2SampleDriverPlugInGlue::staticWriteBlock
};

// SBP2SampleDriverFactory
//
// factory method

void *SBP2SampleDriverFactory( CFAllocatorRef allocator, CFUUIDRef typeID )
{
    if( CFEqual(typeID, kSBP2SampleDriverTypeID) )
        return (void *) SBP2SampleDriverPlugInGlue::alloc();
    else
        return NULL;
}

// alloc
//
// static allocator, called by factory method

IOCFPlugInInterface ** SBP2SampleDriverPlugInGlue::alloc()
{
    SBP2SampleDriverPlugInGlue *me;
	IOCFPlugInInterface ** interface = NULL;
	
    me = new SBP2SampleDriverPlugInGlue;
    if( me )
	{
		// we return an interface here. queryInterface will not be called. call addRef here
		me->addRef();
        interface = (IOCFPlugInInterface **) &me->fIOCFPlugInInterface.pseudoVTable;
    }
	
	return interface;
}

// ctor / dtor
//
//

SBP2SampleDriverPlugInGlue::SBP2SampleDriverPlugInGlue( void ) : SBP2SampleDriver()
{
	fRefCount = 0;
		
	// create plugin interface map
    fIOCFPlugInInterface.pseudoVTable = (IUnknownVTbl *) &sIOCFPlugInInterface;
    fIOCFPlugInInterface.obj = this;
	
	// create test driver interface map
	fSBP2SampleDriverInterface.pseudoVTable = (IUnknownVTbl *) &sSBP2SampleDriverInterface;
	fSBP2SampleDriverInterface.obj = this;
	
	// add instance for factory
	fFactoryId = kSBP2SampleDriverFactoryID;
	CFRetain( fFactoryId );
	CFPlugInAddInstanceForFactory( fFactoryId );
}

SBP2SampleDriverPlugInGlue::~SBP2SampleDriverPlugInGlue()
{
	// remove instance for factory
	CFPlugInRemoveInstanceForFactory( fFactoryId );
	CFRelease( fFactoryId );
}

//////////////////////////////////////////////////////////////////
// IUnknown methods
//

// queryInterface
//
//

HRESULT SBP2SampleDriverPlugInGlue::staticQueryInterface( void * self, REFIID iid, void **ppv )
{
	return getThis(self)->queryInterface( iid, ppv );
}

HRESULT SBP2SampleDriverPlugInGlue::queryInterface( REFIID iid, void **ppv )
{
    CFUUIDRef uuid = CFUUIDCreateFromUUIDBytes(NULL, iid);
    HRESULT result = S_OK;

	if( CFEqual(uuid, IUnknownUUID) ||  CFEqual(uuid, kIOCFPlugInInterfaceID) ) 
	{
        *ppv = &fIOCFPlugInInterface;
        addRef();
    }
	else if( CFEqual(uuid, kSBP2SampleDriverInterfaceID) ) 
	{
        *ppv = &fSBP2SampleDriverInterface;
        addRef();
    }
    else
        *ppv = 0;

    if( !*ppv )
        result = E_NOINTERFACE;

    CFRelease( uuid );
	
    return result;
}

// addRef
//
//

UInt32 SBP2SampleDriverPlugInGlue::staticAddRef( void * self )
{
	return getThis(self)->addRef();
}

UInt32 SBP2SampleDriverPlugInGlue::addRef()
{
    fRefCount += 1;
    return fRefCount;
}

// Release
//
//

UInt32 SBP2SampleDriverPlugInGlue::staticRelease( void * self )
{
	return getThis(self)->release();
}

UInt32 SBP2SampleDriverPlugInGlue::release( void )
{
	UInt32 retVal = fRefCount;
	
	if( 1 == fRefCount-- ) 
	{
		delete this;
    }
    else if( fRefCount < 0 )
	{
        fRefCount = 0;
	}
	
	return retVal;
}

//////////////////////////////////////////////////////////////////
// IOCFPlugInInterface methods
//

// probe
//
//

IOReturn SBP2SampleDriverPlugInGlue::staticProbe( void * self, CFDictionaryRef propertyTable, 
								io_service_t service, SInt32 *order )
{
	return getThis(self)->probe( propertyTable, service, order );
}

// start
//
//

IOReturn SBP2SampleDriverPlugInGlue::staticStart( void * self, CFDictionaryRef propertyTable, 
								io_service_t service )
{
	return getThis(self)->start( propertyTable, service );
}

// stop
//
//

IOReturn SBP2SampleDriverPlugInGlue::staticStop( void * self )
{
	return getThis(self)->stop();
}

//////////////////////////////////////////////////////////////////
// SBP2SampleDriverInterface methods

// setUpCallbacksWithRunLoop
//
// sets up the async dispatcher and a few callback routines

IOReturn SBP2SampleDriverPlugInGlue::staticSetUpCallbacksWithRunLoop( void *self, CFRunLoopRef cfRunLoopRef )
{
	return getThis(self)->setUpCallbacksWithRunLoop( cfRunLoopRef );
}

// loginToDevice / logoutOfDevice
//
//

void SBP2SampleDriverPlugInGlue::staticLoginToDevice( void * self )
{
	getThis(self)->loginToDevice();
}

void SBP2SampleDriverPlugInGlue::staticLogoutOfDevice( void * self )
{
	getThis(self)->logoutOfDevice();
}

//
// staticReadBlock
//
	
IOReturn SBP2SampleDriverPlugInGlue::staticReadBlock( void * self, UInt32 id, UInt32 block, void * buffer )
{
	return getThis(self)->readBlock( id, block, buffer );
}

//
// staticWriteBlock
//
	
IOReturn SBP2SampleDriverPlugInGlue::staticWriteBlock( void * self, UInt32 id, UInt32 block, void * buffer )
{
	return getThis(self)->writeBlock( id, block, buffer );
}

// staticGetNewTransactionID
//
//

UInt32 SBP2SampleDriverPlugInGlue::staticGetNewTransactionID( void * self )
{
	return getThis(self)->getNewTransactionID();
}

// staticAbortReadsWithTransactionID
//
//

void SBP2SampleDriverPlugInGlue::staticAbortReadsWithTransactionID( void * self, UInt32 id )
{
	return getThis(self)->abortReadsWithTransactionID( id );
}
