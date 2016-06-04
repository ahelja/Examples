/*
	File:		SBP2SampleORBTransport.cpp

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
#include "SBP2SampleORBTransport.h"
#include "SBP2SampleDriver.h"

// parse status results 

enum
{
	kStatusDeadBitSet,
	kStatusDummyORBComplete,
	kStatusORBComplete,
	kStatusORBError,
	kStatusORBReset,
	kStatusORBTimeout
};

// factory method
//
//

SBP2SampleORBTransport * SBP2SampleORBTransport::withDriverLayer( SBP2SampleDriver * driverLayer )
{
	SBP2SampleORBTransport * me;
	
	me = new SBP2SampleORBTransport( driverLayer );
	
	return me;
}

// ctor / dtor
//
//

SBP2SampleORBTransport::SBP2SampleORBTransport( SBP2SampleDriver * driverLayer ) : SBP2SampleLoginController()
{
	fDriverLayer = driverLayer;

#ifdef CHAINING_ORB_TRANSPORT
	fDummyORB = NULL; 
#else
	fORBInProgress = false;
#endif

}

SBP2SampleORBTransport::~SBP2SampleORBTransport()
{
}

// start
//
//

IOReturn SBP2SampleORBTransport::start( CFDictionaryRef propertyTable, io_service_t service )
{
	IOReturn status = kIOReturnSuccess;
	
	status = SBP2SampleLoginController::start( propertyTable, service );

#ifdef CHAINING_ORB_TRANSPORT
	
	if( status == kIOReturnSuccess )
	{
		fDummyORB = createORB();
		if( fDummyORB == NULL )
			status = kIOReturnError;
	}
	
	if( status == kIOReturnSuccess )
	{
		fDummyORB->setTimeout( 5000 );
		fDummyORB->setCommandFlags( kFWSBP2CommandImmediate | kFWSBP2CommandCompleteNotify | kFWSBP2CommandDummyORB );												
	}

#else
	fORBInProgress = false;
#endif

	if( status == kIOReturnSuccess )
	{
		// sbp2 will automatically set this register with the give value after logins complete
		status = (*fSBP2LoginInterface)->setBusyTimeoutRegisterValue( fSBP2LoginInterface, 0x0000000f );
	}
	
	return status;
}

// stop
//
//

IOReturn SBP2SampleORBTransport::stop( void )
{
	IOReturn status = kIOReturnSuccess;

#ifdef CHAINING_ORB_TRANSPORT	
	if( fDummyORB )
	{
		fDummyORB->release();
		fDummyORB = NULL;
	}
#endif
	
	status = SBP2SampleLoginController::stop();
	
	return status;
}

//////////////////////////////////////////////////////////////////
// downstream methods for driver layer

//
// submitORB
//
	
IOReturn SBP2SampleORBTransport::submitORB( SBP2SampleORB * orb )
{
	IOReturn status = kIOReturnSuccess;

	if( !fLoggedIn | fORBInProgress )
		status = kIOReturnError;

	if( status == kIOReturnSuccess )
	{	
		UInt32 flags = orb->getCommandFlags();
#ifdef CHAINING_ORB_TRANSPORT
		orb->setCommandFlags( flags | kFWSBP2CommandCompleteNotify );												
#else
		orb->setCommandFlags( flags | kFWSBP2CommandImmediate | kFWSBP2CommandCompleteNotify );
#endif

	}
	
	if( status == kIOReturnSuccess )
	{	
		FWLOG(( "SBP2SampleORBTransport : submit orb = 0x%08lx\n", (UInt32)orb ));
		status = (*fSBP2LoginInterface)->submitORB( fSBP2LoginInterface, orb->getORBInterface() );												
	}

	if( status == kIOReturnSuccess )
	{
#ifdef CHAINING_ORB_TRANSPORT
		// we ignore an error from ringDoorbell because
		// we want an error from submitORB to mean that the orb
		// was not submitted.  If ringDoorbell returns an error it
		// is too late, for the orb will already have been submitted.
		
		// If the doorbell failed to ring, the orb will timeout and
		// the driver layer will be notified of the error.
		
		(*fSBP2LoginInterface)->ringDoorbell( fSBP2LoginInterface );
#endif
	}
	else
	{
		fORBInProgress = true;
	}
	
	return status;
}

//////////////////////////////////////////////////////////////////
// Callback methods

// parseStatus
//
// utility function for statusNotify()

UInt32 SBP2SampleORBTransport::parseStatus( FWSBP2NotifyParams * params )
{
	UInt32 event = kStatusORBError;
	
	switch( params->notificationEvent )
	{
		case kFWSBP2NormalCommandStatus:
			if( (params->message != NULL) && params->length )
			{
				FWSBP2StatusBlock * statusBlock = (FWSBP2StatusBlock*)params->message;

				if ( statusBlock->details & 0x08 ) 
				{
					event = kStatusDeadBitSet;			
				}
				else if ( ((statusBlock->details & 0x30) == 0) &&	// resp == REQUEST_COMPLETE
						  (statusBlock->sbpStatus == 11) )			// sbp_status == dummy orb complete
				{
					event = kStatusDummyORBComplete;
				}
				else if ( ((statusBlock->details & 0x30) == 0) && 	// resp == REQUEST_COMPLETE
						  (statusBlock->sbpStatus == 0) )			// sbp_status == No additional info
				{
					event = kStatusORBComplete;
				}
				else
				{
					event = kStatusORBError;
				}
			}
			break;
			
		case kFWSBP2NormalCommandTimeout:
			event = kStatusORBTimeout;
			break;
			
		case kFWSBP2NormalCommandReset:
			event = kStatusORBReset;	
			break;
			
		default:
			event = kStatusORBError;	
			break;
			
	}
	
	return event;
}

// statusNotify
//
//

void SBP2SampleORBTransport::statusNotify( FWSBP2NotifyParams * params )
{
	FWLOG(( "SBP2SampleORBTransport : statusNotify params 0x%08lx\n", (UInt32)params ));
	
	SBP2SampleORB * orb = (SBP2SampleORB *)params->refCon;
	
	UInt32 event = parseStatus( params );
	
	switch( event )
	{	
	
		case kStatusDeadBitSet:
			
			FWLOG(( "SBP2SampleORBTransport : statusNotify orb = 0x%08lx - kStatusDeadBitSet\n", (UInt32)orb ));
			
			// dead bit set
			
			// suspend driver layer
			fDriverLayer->suspendORBs();
			
			// complete this orb	
			completeORB( orb, kIOReturnError ); 
			
			// reset the fetch agent
			(*fSBP2LoginInterface)->submitFetchAgentReset( fSBP2LoginInterface );
	
			break;
			
		case kStatusDummyORBComplete:

			FWLOG(( "SBP2SampleORBTransport : statusNotify orb = 0x%08lx - kStatusDummyORBComplete\n", (UInt32)orb ));

#ifdef CHAINING_ORB_TRANSPORT			
			// dummy orb complete
			if( orb == fDummyORB )
			{
				// all initialized, resume driver layer
				fDriverLayer->resumeORBs();
			}
			else
			{
				// must be an aborted orb
				completeORB( orb, kIOReturnError );
			}
#else
			// must be an aborted orb
			completeORB( orb, kIOReturnError );
#endif
			break;
		
		case kStatusORBComplete:
	
			FWLOG(( "SBP2SampleORBTransport : statusNotify orb = 0x%08lx - kStatusORBComplete\n", (UInt32)orb ));
			
			// complete driver layer orbs
			completeORB( orb, kIOReturnSuccess );
			
			break;
		
		case kStatusORBError:
			
			FWLOG(( "SBP2SampleORBTransport : statusNotify orb = 0x%08lx - kStatusORBError\n", (UInt32)orb ));
			
			completeORB( orb, kIOReturnIOError );
	
			break;

		case kStatusORBReset:
			
			FWLOG(( "SBP2SampleORBTransport : statusNotify orb = 0x%08lx - kStatusORBReset\n", (UInt32)orb ));
			
			// command reset, tell driver layer
			
			// driver layer should already be suspended at this point
			
			completeORB( orb, kIOReturnNotReady );
			
			break;
			
		case kStatusORBTimeout:
			
			FWLOG(( "SBP2SampleORBTransport : statusNotify orb = 0x%08lx - kStatusORBTimeout\n", (UInt32)orb ));
			
			// suspend driver layer
			fDriverLayer->suspendORBs();	
			
			// complete this orb
			completeORB( orb, kIOReturnError ); 
			
			// reset the lun
			(*fLUNResetORBInterface)->submitORB( fLUNResetORBInterface );		
			
			break;			
			
		default:
			
			FWLOG(( "SBP2SampleORBTransport : statusNotify orb = 0x%08lx - default !?\n", (UInt32)orb ));
			
			// should not be possible
			
			break;
			
	}
}

void SBP2SampleORBTransport::completeORB( SBP2SampleORB * orb, IOReturn status )
{

#ifdef CHAINING_ORB_TRANSPORT

	// make sure we only complete the driver layer's orbs
	if( orb != fDummyORB )
		fDriverLayer->completeORB( orb, status );

#else

	fORBInProgress = false;
	fDriverLayer->completeORB( orb, status );

#endif

}

void SBP2SampleORBTransport::LUNResetCompletion( IOReturn status, void * orb )
{
	SBP2SampleLoginController::LUNResetCompletion( status, orb );
	
	// reset the fetch agent for good measure
	
	// driver layer should already be suspended
	
	(*fSBP2LoginInterface)->submitFetchAgentReset( fSBP2LoginInterface );
}

void SBP2SampleORBTransport::fetchAgentResetCompletion( IOReturn status )
{
	SBP2SampleLoginController::fetchAgentResetCompletion( status );
	
	// okay, we're back ...

#ifdef CHAINING_ORB_TRANSPORT
		
	// append dummy orb 
	FWLOG(( "SBP2SampleORBTransport : submit dummy orb = 0x%08lx\n", (UInt32)fDummyORB ));
	(*fSBP2LoginInterface)->submitORB( fSBP2LoginInterface, fDummyORB->getORBInterface() );												

#else

	// all initialized, resume driver layer
	fDriverLayer->resumeORBs();

#endif

}

// loginLost
//
// called when we've determined that we aren't going to relogin
// to this device.  can only be followed by a loginResumed or
// no message at all.

void SBP2SampleORBTransport::loginLost( void )
{
	SBP2SampleLoginController::loginLost();
	
	// tell driver layer about loss of communication with device ...
	fDriverLayer->loginLost();
}

// loginSuspended
//
// called when we first lose our current login. no more orbs may be
// appended to the device and all non-complete orbs have been aborted.
// in the best case this routine will be followed by a loginResumed. 
// in the worst case it will be followed by a loginLost. 

void SBP2SampleORBTransport::loginSuspended( void )
{
	SBP2SampleLoginController::loginSuspended();
	
	fDriverLayer->suspendORBs();
}

// loginResumed
//
// sent after first login or after a loginSuspended if
// we successfully relogged into the device. we can now begin
// appending orbs again.

void SBP2SampleORBTransport::loginResumed( void )
{
	SBP2SampleLoginController::loginResumed();
	
	// allow unsolicited status
	(*fSBP2LoginInterface)->enableUnsolicitedStatus( fSBP2LoginInterface );

#ifdef CHAINING_ORB_TRANSPORT
	
	// append dummy orb 
	FWLOG(( "SBP2SampleORBTransport : submit dummy orb = 0x%08lx\n", (UInt32)fDummyORB ));
	(*fSBP2LoginInterface)->submitORB( fSBP2LoginInterface, fDummyORB->getORBInterface() );												

#else

	// all initialized, resume driver layer
	fDriverLayer->resumeORBs();

#endif

}
