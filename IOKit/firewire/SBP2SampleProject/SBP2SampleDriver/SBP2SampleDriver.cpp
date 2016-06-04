/*
	File:		SBP2SampleDriver.cpp

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

#include <Carbon/Carbon.h>  // gets me printf

#include "FWDebugging.h"
#include "SBP2SampleDriver.h"
#include "SBP2SampleORBTransport.h"

// ctor / dtor
//
//

SBP2SampleDriver::SBP2SampleDriver( void )
{
	fSuspended = true;
	fDeviceReady = false;
	fTransactionID = 0;
	
	pthread_mutex_init( &fLock, NULL );
	pthread_mutex_init( &fFreePoolLock, NULL );
	pthread_cond_init( &fFreePoolCondition, NULL );
	
}

SBP2SampleDriver::~SBP2SampleDriver()
{
	pthread_mutex_destroy( &fLock );
	pthread_cond_destroy( &fFreePoolCondition );
	pthread_mutex_destroy( &fFreePoolLock );
}

// probe
//
//

IOReturn SBP2SampleDriver::probe( CFDictionaryRef propertyTable, io_service_t service, SInt32 *order )
{
	// only load against LUN's
    if( !service || !IOObjectConformsTo(service, "IOFireWireSBP2LUN") )
        return kIOReturnBadArgument;
		
	return kIOReturnSuccess;
}

// start
//
//

IOReturn SBP2SampleDriver::start( CFDictionaryRef propertyTable, io_service_t service )
{
	IOReturn status = kIOReturnSuccess;
	
	FWLOG(( "SBP2SampleDriver : start\n" ));

	//
	// create the transport layer ...
	//
	
	if( status == kIOReturnSuccess )
	{
		fTransportLayer = SBP2SampleORBTransport::withDriverLayer(this);
		if( fTransportLayer == NULL )
			status = kIOReturnNoMemory;
	}

	//
	// ... and tell it to start
	//
	
	if( status == kIOReturnSuccess )
	{
		status = fTransportLayer->start( propertyTable, service );
	}
	
	//
	// stop is not called if start fails, so do cleanup here
	//
	
	if( status != kIOReturnSuccess && fTransportLayer )
	{
		delete fTransportLayer;
		fTransportLayer = NULL;
	}

	//
	// create TEST_UNIT_READY orb
	//
	
	if( status == kIOReturnSuccess )
	{
		fTURORB = fTransportLayer->createORB();
	}

	if( status == kIOReturnSuccess )
	{
		UInt8 command[12];
		command[0] = 0; 
		command[1] = 0;
		command[2] = 0;
		command[3] = 0;

		command[4] = 0;
		command[5] = 0;
		command[6] = 0;
		command[7] = 0;

		command[8] = 0;
		command[9] = 0;
		command[10] = 0;
		command[11] = 0;

		status = fTURORB->setCommandBlock( &command, 12 );
	}

	if( status == kIOReturnSuccess )
	{
		fTURORB->setTimeout( 5000 );
	}
		
	if( status == kIOReturnSuccess )
	{	
		fTURORB->setCommandFlags( kFWSBP2CommandTransferDataFromTarget |
										kFWSBP2CommandNormalORB );												
	}

	//
	// create array of free orbs
	//
	
	if( status == kIOReturnSuccess )
	{
	    fFreeORBPool = CFArrayCreateMutable( kCFAllocatorDefault, 
										  kSBP2SampleFreeORBCount, // max capacity
										  SBP2SampleORB::getCFArrayCallbacks() );
		if( fFreeORBPool == NULL )
			status = kIOReturnNoMemory;
	}
	
	for( UInt32 i = 0; status == kIOReturnSuccess && i < kSBP2SampleFreeORBCount; i++ )
	{
		SBP2SampleORB * orb = fTransportLayer->createORB();
		if( orb == NULL )
			status = kIOReturnError;
			
		if( status == kIOReturnSuccess )
		{
			addORBtoFreePool( orb );
		}
	}

	//
	// create array for in progress orbs
	//
	
	if( status == kIOReturnSuccess )
	{
	    fInProgressORBQueue = CFArrayCreateMutable(	kCFAllocatorDefault, 
												kSBP2SampleFreeORBCount, // max capacity 
												SBP2SampleORB::getCFArrayCallbacks() );
		if( fInProgressORBQueue == NULL )
			status = kIOReturnNoMemory;
	}
	
	FWLOG(( "SBP2SampleDriver : start return status = 0x%08x\n", status ));
	
	return status;
}

// stop
//
//

IOReturn SBP2SampleDriver::stop( void )
{
	IOReturn status = kIOReturnSuccess;

	FWLOG(( "SBP2SampleDriver : stop\n" ));

    if( fInProgressORBQueue )
        CFRelease( fInProgressORBQueue );  // release array and orbs

    if( fFreeORBPool )
        CFRelease( fFreeORBPool ); // release array and orbs

	if( fTURORB )
	{
		fTURORB->release();
		fTURORB = NULL;
	}	

	if( fTransportLayer )
	{
		status = fTransportLayer->stop();
		delete fTransportLayer;
		fTransportLayer = NULL;
	}

	FWLOG(( "SBP2SampleDriver : stop return status = 0x%08x\n", status ));

	return status;
}

//////////////////////////////////////////////////////////////////
// SBP2SampleDriverInterface methods

// setUpCallbacksWithRunLoop
//
// sets up the async dispatcher and a few callback routines

IOReturn SBP2SampleDriver::setUpCallbacksWithRunLoop( CFRunLoopRef cfRunLoopRef )
{
	IOReturn status = kIOReturnSuccess;

	// do a little more error checking on public APIs
	if( fTransportLayer == NULL )  
		status = kIOReturnError;
		
	//
	// just pass it on to the transport layer
	//

	if( status == kIOReturnSuccess )
	{
		status = fTransportLayer->setUpCallbacksWithRunLoop(cfRunLoopRef);
	}
			
	return status;
}

// loginToDevice / logoutOfDevice
//
//

IOReturn SBP2SampleDriver::loginToDevice( void )
{
	IOReturn status = kIOReturnSuccess;

	// do a little more error checking on public APIs
	if( fTransportLayer == NULL )  
		status = kIOReturnError;

	//
	// just pass it on to the transport layer
	//
	
	if( status == kIOReturnSuccess )
	{
		fTransportLayer->loginToDevice();
	}
			
	return status;
}

IOReturn SBP2SampleDriver::logoutOfDevice( void )
{
	IOReturn status = kIOReturnSuccess;

	// do a little more error checking on public APIs
	if( fTransportLayer == NULL )  
		status = kIOReturnError;

	//
	// just pass it on to the transport layer
	//
	
	if( status == kIOReturnSuccess )
	{
		fTransportLayer->logoutOfDevice();
	}
			
	return status;
}

/////////////////////////////////////////////////////////////////////////////////

UInt32 SBP2SampleDriver::getNewTransactionID( void )
{
	return fTransactionID++;
}

void SBP2SampleDriver::abortReadsWithTransactionID( UInt32 id )
{
	//zzz not done yet

#if 0	
#endif

}

/////////////////////////////////////////////////////////////////////////////////

// getORBFromFreePool
//
//

SBP2SampleORB * SBP2SampleDriver::getORBFromFreePool( void )
{
	CFIndex freeORBCount;
	
	pthread_mutex_lock(&fFreePoolLock);
	
	//zzz pass id in and check if we're aborted...
	//zzz wake up all threads 
	while( (freeORBCount = CFArrayGetCount( fFreeORBPool )) == 0 )
	{
		// wait for free orb
		pthread_cond_wait( &fFreePoolCondition, &fFreePoolLock );
	}

	// remove orb from pool
	SBP2SampleORB * orb = (SBP2SampleORB*)CFArrayGetValueAtIndex( fFreeORBPool, 0);
	orb->retain();	
	CFArrayRemoveValueAtIndex( fFreeORBPool, 0 );
	
	pthread_mutex_unlock( &fFreePoolLock );	
		
	return orb;
}

// addORBToFreePool
//
//

void SBP2SampleDriver::addORBtoFreePool( SBP2SampleORB * orb )
{
	pthread_mutex_lock(&fFreePoolLock);

	CFArrayAppendValue( fFreeORBPool, (void*) orb );
	orb->release();
	
	pthread_mutex_unlock( &fFreePoolLock );	
	
	// wake up threads waiting for orbs
	pthread_cond_signal( &fFreePoolCondition );

}
	
// appendORBToInProgressQueue
//
//

void SBP2SampleDriver::appendORBToInProgressQueue( SBP2SampleORB * orb )
{
	CFArrayAppendValue( fInProgressORBQueue, (void*) orb );
	orb->release();
}

// removeORBFromInProgressQueue
//
//

void SBP2SampleDriver::removeORBFromInProgressQueue( SBP2SampleORB * orb )
{
	CFIndex inProgressORBCount = CFArrayGetCount( fInProgressORBQueue );
	CFIndex index = CFArrayGetFirstIndexOfValue( 	fInProgressORBQueue, 
													CFRangeMake(0, inProgressORBCount), 
													(void *)orb);
	if( index != -1 )
	{												
		// remove orb from pool
		SBP2SampleORB * orb = (SBP2SampleORB*)CFArrayGetValueAtIndex( fInProgressORBQueue, index);
		orb->retain();	
		CFArrayRemoveValueAtIndex( fInProgressORBQueue, index );
	}
}

/////////////////////////////////////////////////////////////////////////////////

IOReturn SBP2SampleDriver::writeBlock( UInt32 id, UInt32 block, void * buffer )
{
	IOReturn status = kIOReturnSuccess;
	SBP2SampleORB * orb = NULL;
		
	// get orb from free queue else wait
	if( status == kIOReturnSuccess )
	{	
		// this will sleep the thread until an orb becomes free
		orb = getORBFromFreePool();
	}
	
	if( status == kIOReturnSuccess )
	{
		// begin using orb
		orb->setState( kORBInProgress );
		
		// do a little more error checking on public APIs
		if( fTransportLayer == NULL )  
			status = kIOReturnError;
	}
	
	if( status == kIOReturnSuccess )
	{
		UInt8 command[12];
		UInt16 count = 1;  // just one block
		
		command[0] = 0x2A; 			
		command[1] = 0;
		command[2] = (block >> 24) & 0xff;
		command[3] = (block >> 16) & 0xff;

		command[4] = (block >> 8) & 0xff;
		command[5] = block & 0xff;
		command[6] = 0;
		command[7] = (count >> 8) & 0xff;

		command[8] = count & 0xff;
		command[9] = 0;
		command[10] = 0;
		command[11] = 0x02;

		status = orb->setCommandBlock( &command, 12 );
	}

	if( status == kIOReturnSuccess )
	{
		orb->setTransactionID( id );
		orb->setTimeout( 5000 );  // 5 seconds 
	}
	
	if( status == kIOReturnSuccess )
	{
		UInt32 offset = 0;
		UInt32 blockSize = 512;
		FWSBP2VirtualRange range;		
		range.address = buffer;
		range.length = 512;
		status = orb->setCommandBuffersAsRanges( &range, 1, kIODirectionOut, offset, blockSize );
	}
	
	if( status == kIOReturnSuccess )
	{	
		orb->setCommandFlags( kFWSBP2CommandNormalORB );												
	}

	pthread_mutex_lock( &fLock );

	// add command to in use queue
	if( status == kIOReturnSuccess )
	{	
		appendORBToInProgressQueue( orb );
				
		// don't submit if we're not ready
		if( fDeviceReady )
			status = fTransportLayer->submitORB( orb );												

		if( status != kIOReturnSuccess )
			removeORBFromInProgressQueue( orb );
	}

	pthread_mutex_unlock( &fLock );

	if( status != kIOReturnSuccess )
	{
		orb->setState( kORBIdle );
		addORBtoFreePool( orb );
	}	
	
	FWLOG(( "SBP2SampleDriver : waiting for orb complete\n" ));
			
	if( status == kIOReturnSuccess )
		status = orb->sleepUntilComplete();  // will sleep thread until orb complete
	
	FWLOG(( "SBP2SampleDriver : orb complete\n" ));
	return status;
}

IOReturn SBP2SampleDriver::readBlock( UInt32 id, UInt32 block, void * buffer )
{
	IOReturn status = kIOReturnSuccess;
	SBP2SampleORB * orb = NULL;
		
	// get orb from free queue else wait
	if( status == kIOReturnSuccess )
	{	
		// this will sleep the thread until an orb becomes free
		orb = getORBFromFreePool();
	}
	
	if( status == kIOReturnSuccess )
	{
		// begin using orb
		orb->setState( kORBInProgress );
		
		// do a little more error checking on public APIs
		if( fTransportLayer == NULL )  
			status = kIOReturnError;
	}
	
	if( status == kIOReturnSuccess )
	{
		UInt8 command[12];
		UInt16 count = 1;  // just one block
		
		command[0] = 0x28; 			
		command[1] = 0;
		command[2] = (block >> 24) & 0xff;
		command[3] = (block >> 16) & 0xff;

		command[4] = (block >> 8) & 0xff;
		command[5] = block & 0xff;
		command[6] = 0;
		command[7] = (count >> 8) & 0xff;

		command[8] = count & 0xff;
		command[9] = 0;
		command[10] = 0;
		command[11] = 0x02;

		status = orb->setCommandBlock( &command, 12 );
	}

	if( status == kIOReturnSuccess )
	{
		orb->setTransactionID( id );
		orb->setTimeout( 5000 );  // 5 seconds 
	}
	
	if( status == kIOReturnSuccess )
	{
		UInt32 offset = 0;
		UInt32 blockSize = 512;
		FWSBP2VirtualRange range;		
		range.address = buffer;
		range.length = 512;
		status = orb->setCommandBuffersAsRanges( &range, 1, kIODirectionIn, offset, blockSize );
	}
	
	if( status == kIOReturnSuccess )
	{	
		orb->setCommandFlags( kFWSBP2CommandTransferDataFromTarget | kFWSBP2CommandNormalORB );												
	}

	pthread_mutex_lock( &fLock );

	// add command to in use queue
	if( status == kIOReturnSuccess )
	{	
		appendORBToInProgressQueue( orb );
				
		// don't submit if we're not ready
		if( fDeviceReady )
			status = fTransportLayer->submitORB( orb );												

		if( status != kIOReturnSuccess )
			removeORBFromInProgressQueue( orb );
	}

	pthread_mutex_unlock( &fLock );

	if( status != kIOReturnSuccess )
	{
		orb->setState( kORBIdle );
		addORBtoFreePool( orb );
	}	
	
	FWLOG(( "SBP2SampleDriver : waiting for orb complete\n" ));
			
	if( status == kIOReturnSuccess )
		status = orb->sleepUntilComplete();  // will sleep thread until orb complete
	
	FWLOG(( "SBP2SampleDriver : orb complete\n" ));
	return status;
}


//////////////////////////////////////////////////////////////////
// upstream calls from transport layer

// completeORB
//
// called from transport layer when orb completes

void SBP2SampleDriver::completeORB( SBP2SampleORB * orb, IOReturn completionStatus )
{
	IOReturn status = kIOReturnSuccess;
	
	// it is safe to check fSuspended outside of lock in this function
	// because completeORB is called on the runLoop and fSuspended
	// is only modified by the runLoop

	if( orb == fTURORB )
	{
		if( completionStatus == kIOReturnSuccess && !fSuspended )
		{
			FWLOG(( "SBP2SampleDriver : resubmit orbs now\n" ));
			
			pthread_mutex_lock( &fLock );
			
			fDeviceReady = true;
			
			CFIndex inProgressORBCount = CFArrayGetCount( fInProgressORBQueue );
			CFIndex index;
		
			for( index = 0; index < inProgressORBCount; index++ )
			{												
				SBP2SampleORB * orb = (SBP2SampleORB*)CFArrayGetValueAtIndex( fInProgressORBQueue, index);
				status = fTransportLayer->submitORB( orb );
			}
			
			pthread_mutex_unlock( &fLock );
		}

	}
        else if( completionStatus != kIOReturnNotReady )
	{		
		pthread_mutex_lock( &fLock );

		removeORBFromInProgressQueue( orb );
		
		pthread_mutex_unlock( &fLock );

		status = orb->releaseCommandBuffers();

		addORBtoFreePool( orb );

		orb->signalCompletion( completionStatus );		
	}
}

// resumeORBs
//
//

void SBP2SampleDriver::resumeORBs( void )
{
	FWLOG(( "SBP2SampleDriver : resumeORBs\n" ));
		
	pthread_mutex_lock( &fLock );

	// orb submits allowed
	fSuspended = false;
	
	// reestablish communication with device
	fTransportLayer->submitORB( fTURORB );
	
	pthread_mutex_unlock( &fLock );
	
}

// suspendORBs
//
//

void SBP2SampleDriver::suspendORBs( void )
{
	FWLOG(( "SBP2SampleDriver : suspendORBs\n" ));
	
	// no orb submits allowed until resumeORBs
	
	pthread_mutex_lock( &fLock );
	
	fSuspended = true;
	fDeviceReady = false;
	
	pthread_mutex_unlock( &fLock );
	
}

// loginLost
//
// we lost communication with the device

void SBP2SampleDriver::loginLost( void )
{
	FWLOG(( "SBP2SampleDriver : loginLost\n" ));
}


