/*
	File:		SBP2SampleORB.cpp

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.
	
	Written by:	cpieper

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

#include "SBP2SampleORB.h"

CFArrayCallBacks SBP2SampleORB::sArrayCallbacks = 
{
	0,								// version
	&SBP2SampleORB::staticRetain, 	// retain
   	&SBP2SampleORB::staticRelease, 	// release
	NULL, 							// copyDescription
    NULL,							// equal
};

// ctor/dtor
//
//

SBP2SampleORB::SBP2SampleORB( void )
{
	fSBP2ORBInterface = NULL;
	fFlags = 0;
	fRefCount = 0;
	fState = kORBIdle;
	
	pthread_mutex_init( &fORBCompleteLock, NULL );
	pthread_cond_init( &fORBCompleteCondition, NULL );
}

SBP2SampleORB::~SBP2SampleORB()
{
	pthread_cond_destroy( &fORBCompleteCondition );
	pthread_mutex_destroy( &fORBCompleteLock);

	if( fSBP2ORBInterface != NULL )
	{
		(*fSBP2ORBInterface)->Release(fSBP2ORBInterface);
		fSBP2ORBInterface = NULL;
	}
}

IOReturn SBP2SampleORB::init( IOFireWireSBP2LibORBInterface** orbInterface )
{
	IOReturn status = kIOReturnSuccess;
	
	fSBP2ORBInterface = orbInterface;
	if( !orbInterface )
		status = kIOReturnError;
		
	if( status == kIOReturnSuccess )
	{
		(*fSBP2ORBInterface)->setRefCon( fSBP2ORBInterface, (UInt32)this );
	}
	
	return status;
}

// retain
//
//

const void * SBP2SampleORB::staticRetain( CFAllocatorRef allocator, const void *value )
{
	//zzz should use dynamic cast
	((SBP2SampleORB*)value)->retain();
	
	return value;
}

void SBP2SampleORB::retain( void )
{
    fRefCount += 1;
}

// release
//
//

void SBP2SampleORB::staticRelease( CFAllocatorRef allocator, const void *value )
{
	//zzz should use dynamic cast
	((SBP2SampleORB*)value)->release();
}

void SBP2SampleORB::release( void )
{
	if( 1 == fRefCount-- ) 
	{
		delete this;
    }
    else if( fRefCount < 0 )
	{
        fRefCount = 0;
	}
}

void SBP2SampleORB::setCommandFlags( UInt32 flags )
{
	fFlags = flags;
	(*fSBP2ORBInterface)->setCommandFlags( fSBP2ORBInterface, fFlags );
}

UInt32 SBP2SampleORB::getCommandFlags( void )
{
	return fFlags;
}

void SBP2SampleORB::setTransactionID( UInt32 id )
{
	fTransactionID = id;
}

UInt32 SBP2SampleORB::getTransactionID( void )
{
	return fTransactionID;
}

	
void SBP2SampleORB::setMaxORBPayloadSize( UInt32 size )
{
	(*fSBP2ORBInterface)->setMaxORBPayloadSize( fSBP2ORBInterface, size );
}

void SBP2SampleORB::setTimeout( UInt32 timeout )
{
	(*fSBP2ORBInterface)->setCommandTimeout( fSBP2ORBInterface, timeout );
}

IOReturn SBP2SampleORB::setCommandBlock( void * buffer, UInt32 length )
{
	return (*fSBP2ORBInterface)->setCommandBlock( fSBP2ORBInterface, buffer, length );
}

IOReturn SBP2SampleORB::setCommandBuffersAsRanges( FWSBP2VirtualRange * ranges, UInt32 withCount,
											UInt32 withDirection, UInt32 offset, 
											UInt32 length )
{
	return (*fSBP2ORBInterface)->setCommandBuffersAsRanges( fSBP2ORBInterface, ranges, withCount, 
											withDirection, offset, length );
}

IOReturn SBP2SampleORB::releaseCommandBuffers( void )
{
	return (*fSBP2ORBInterface)->releaseCommandBuffers(fSBP2ORBInterface);
}

IOFireWireSBP2LibORBInterface ** SBP2SampleORB::getORBInterface( void )
{
	return fSBP2ORBInterface;
}	

////////////////////////////////////////////////////////////////////////////////////

void SBP2SampleORB::setState( UInt32 state )
{
	fORBStatus = kIOReturnSuccess;
	fState = state;
}

IOReturn SBP2SampleORB::sleepUntilComplete( void )
{
	pthread_mutex_lock( &fORBCompleteLock );
	
	if( fState == kORBInProgress )
		pthread_cond_wait( &fORBCompleteCondition, &fORBCompleteLock );

	pthread_mutex_unlock( &fORBCompleteLock );	

	return fORBStatus;
}

void SBP2SampleORB::signalCompletion( IOReturn status )
{
	fORBStatus = status;

	pthread_mutex_lock( &fORBCompleteLock );
	
	fState = kORBComplete;
	
	pthread_mutex_unlock( &fORBCompleteLock );	
	pthread_cond_signal( &fORBCompleteCondition );
}
