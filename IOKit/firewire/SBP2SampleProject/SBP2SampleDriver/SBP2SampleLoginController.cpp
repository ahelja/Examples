/*
	File:		SBP2SampleLoginController.cpp

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
#include "SBP2SampleLoginController.h"

// ctor / dtor
//
//

SBP2SampleLoginController::SBP2SampleLoginController( void ) : SBP2SampleSBP2LibGlue()
{
	// unnecessary
	fNeedLogin 	= false;
    fMaintainLogin = false;
	fLoggedIn = false;
	
	// necessary
	
	// this is not always correct, see below
	 fPhysicallyConnected = true;
	 
	// We handle the case where this is an invalid assumption,
	// but not very elegantly.  We will try and fail to login
	// kMaxLoginRetries times and then stop. If the device
	// reappears we will receive a resume message and start
	// from there.
   
}

SBP2SampleLoginController::~SBP2SampleLoginController()
{
}

// stop
//
//

IOReturn SBP2SampleLoginController::stop( void )
{
	// clean up from sloppy clients
	
	// if we didn't do this and a client terminated us without logging out
	// noone would be able to login to this device until the next bus reset
	
	if( fMaintainLogin )
	{
		logoutOfDevice();
	}
	
	return SBP2SampleSBP2LibGlue::stop();
}


//////////////////////////////////////////////////////////////////
// SBP2SampleDriverInterface methods

// loginToDevice / logoutOfDevice
//
//

void SBP2SampleLoginController::loginToDevice( void )
{
	IOReturn status = kIOReturnSuccess;
	
	FWLOG(( "SBP2SampleLoginController : loginToDevice\n"));
	
	//
	// submit login
	//
	
	if( status == kIOReturnSuccess )
	{
		// don't allow driver to unload
		fMaintainLogin = true;

		// when this is set we attempt to login to the device when we recieve a resume message
		// since we are about to login we don't want this to happen
		fNeedLogin = false;
		fLoggedIn = false;
	
		FWLOG(( "SBP2SampleLoginController : submitLogin\n" ));
		
		fLoginRetries = kMaxLoginRetries;
		(*fSBP2LoginInterface)->setLoginFlags( fSBP2LoginInterface, kFWSBP2ExclusiveLogin );
		status = (*fSBP2LoginInterface)->submitLogin(fSBP2LoginInterface);	
		//zzz there is a bug in MacOS X 1.0 where we will return kIOReturnBusy when we mean kIOReturnSuccess
		if( status == kIOReturnBusy )
			status = kIOReturnSuccess;
	}
	
	if( status != kIOReturnSuccess )
	{
		fNeedLogin = true;
		loginSuspended();
	}
}

void SBP2SampleLoginController::logoutOfDevice( void )
{
	IOReturn status = kIOReturnSuccess;
	
	FWLOG(( "SBP2SampleLoginController : logoutOfDevice\n"));
	
	// allow the driver to unload
    fMaintainLogin = false;

    // we don't want to start logging in if we receive a resume messsage during logout
    fNeedLogin = false;

	FWLOG(( "SBP2SampleLoginController : submitLogout\n" ));

	status = (*fSBP2LoginInterface)->submitLogout(fSBP2LoginInterface);
	//zzz there is a bug in MacOS X 1.0 where we will return kIOReturnBusy when we mean kIOReturnSuccess
	if( status == kIOReturnBusy )
		status = kIOReturnSuccess;

	fLoggedIn = false;  // we do this in the completion too
}

// loginComplete
//
//

void SBP2SampleLoginController::loginCompletion( FWSBP2LoginCompleteParams * params )
{	
	FWLOG(( "SBP2SampleLoginController : loginCompletion\n" ));

	// NOTE: the param block and any pointers in the param block
    // are only guaranteed to be valid until this function returns

    // NOTE: do not modify the loginResponse block or the statusBlock

    // check params->status for success
    // if params->statusBlock != NULL, the status block was received; check for further status
    // login response is supplied in params->loginResponse on a successful login
    
	FWSBP2StatusBlock * statusBlock = params->statusBlock;
	
	//zzz may want to check device specific status for success as well
    if( params->status == kIOReturnSuccess &&
		(statusBlock->details & 0x30) == 0 && 	// resp == REQUEST_COMPLETE
		statusBlock->sbpStatus == 0 )			// sbp_status == No additional info
    {
        FWLOG(( "SBP2SampleLoginController : login successful\n" ));
		fLoggedIn = true;
		loginResumed();
    }
    else
    {
		// we just retry a bunch of times if we didn't get good status
		//zzz we could look at the status and determine if there is any chance a retry will succeed
		//zzz may want to check device specific errors as well
		
        FWLOG( ( "SBP2SampleLoginController : login unsuccessful, status = 0x%08x\n", params->status ) );

		if( fPhysicallyConnected )
		{
			if( fLoginRetries > 0 )
			{
				fLoginRetries--;
				(*fSBP2LoginInterface)->submitLogin( fSBP2LoginInterface );
			}
			else
			{
				fNeedLogin = true;
				loginLost();
			}
		}
		else
		{
			fNeedLogin = true;
		}
	}
}


//////////////////////////////////////////////////////////////////
// Callback methods

// message
//
//

void SBP2SampleLoginController::message( UInt32 type, void * arg )
{
	FWLOG(( "SBP2SampleLoginController : message type = 0x%08lx\n", type ));
	
    FWSBP2ReconnectParams * params;

    // arg and any pointers in it are only valid until this function returns
    // do not modify and data in the reconnectStatusBlock

	switch (type)
	{
		case kIOMessageServiceIsSuspended:
			
			// a bus reset occured
			FWLOG( ( "SBP2SampleLoginController : kIOMessageServiceIsSuspended\n" ) );
		
			fPhysicallyConnected = false;
		
			if( fLoggedIn )
			{
				fLoggedIn = false;
				loginSuspended();				
			}
			
			break;

		case kIOMessageServiceIsResumed:
			
			// we have processed the self-ids and rediscovered this device
			FWLOG( ( "SBP2SampleLoginController : kIOMessageServiceIsResumed\n" ) );

			fPhysicallyConnected = true;
			
			if( fNeedLogin )
			{
				fNeedLogin = false;
				fLoginRetries = kMaxLoginRetries;
				(*fSBP2LoginInterface)->submitLogin(fSBP2LoginInterface);
			}
			
			break;

		case kIOMessageFWSBP2ReconnectComplete:
			// we reconnected sucessfully to this device
			// status block is supplied in params->statusBlock for further status
			params = (FWSBP2ReconnectParams*)arg;
			fLoggedIn = true;
			FWLOG(( "SBP2SampleORBTransport : kIOMessageFWSBP2ReconnectComplete\n" ));
			
			loginResumed();
			
			break;

		case kIOMessageFWSBP2ReconnectFailed:
			FWLOG(( "SBP2SampleORBTransport : kIOMessageFWSBP2ReconnectFailed\n" ));
			
			// we did not reconnect successfully to this device
		
			// check params->status for error
			// if params->statusBlock != NULL, the status block was received; check for further status

			if( fPhysicallyConnected )
			{
				fLoginRetries = kMaxLoginRetries;
				
				//zzz could decide if this has any chance of success from returned status
				(*fSBP2LoginInterface)->submitLogin(fSBP2LoginInterface);
			}
			else
			{
				fNeedLogin = true;
			}
				
			params = (FWSBP2ReconnectParams*)arg;

			
			break;
						
		case kIOMessageServiceIsRequestingClose:
			// we have processsed the self-ids and did not rediscover this device
			FWLOG( ( "SBP2SampleORBTransport : kIOMessageServiceIsRequestingClose\n" ));
			
			// firewire is requesting to close this driver, calling close on the provider here
			// results in terminiation
			
			loginLost();
			
			break;

		case kIOMessageServiceIsTerminated:
			// the driver has been terminated
			FWLOG( ( "SBP2SampleORBTransport : kIOMessageServiceIsTerminated\n" ) );
			break;

		default:
			break;
	}
 }


// logoutComplete
//
//

void SBP2SampleLoginController::logoutCompletion( FWSBP2LogoutCompleteParams * params )
{
	FWLOG(( "SBP2SampleLoginController : logoutCompletion\n" ));
	
	fLoggedIn = false;
	
	SBP2SampleSBP2LibGlue::logoutCompletion( params );
}

//////////////////////////////////////////////////////////////////
// methods for subclassing

// loginLost
//
// called when we've determined that we aren't going to relogin
// to this device.  can only be followed by a loginResumed or
// no message at all.

void SBP2SampleLoginController::loginLost( void )
{
}

// loginSuspended
//
// called when we first lose our current login. no more orbs may be
// appended to the device and all non-complete orbs have been aborted.
// in the best case this routine will be followed by a loginResumed. 
// in the worst case it will be followed by a loginLost. 

void SBP2SampleLoginController::loginSuspended( void )
{
}

// loginResumed
//
// sent after first login or after a loginSuspended if
// we successfully relogged into the device. we can now begin
// appending orbs again.

void SBP2SampleLoginController::loginResumed( void )
{
}
