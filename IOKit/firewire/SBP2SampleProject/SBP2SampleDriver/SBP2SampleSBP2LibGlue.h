/*
	File:		SBP2SampleLibGlue.h

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
 
#ifndef _IOKIT_SBP2SAMPLESBP2LIBGLUE_H_
#define _IOKIT_SBP2SAMPLESBP2LIBGLUE_H_

#include <IOKit/IOCFPlugIn.h>
#include <IOKit/sbp2/IOFireWireSBP2Lib.h>

#include <IOKit/firewire/IOFireWireLib.h>

#include "SBP2SampleDriverInterface.h"
#include "SBP2SampleORB.h"

class SBP2SampleSBP2LibGlue
{

public:	
	SBP2SampleSBP2LibGlue( void );
	virtual ~SBP2SampleSBP2LibGlue();
	
protected:
	
	// sbp2 plugin interfaces
	IOCFPlugInInterface 				**	fSBP2CFPlugInInterface;
	IOFireWireSBP2LibLUNInterface 		**	fSBP2LUNInterface;
	IOFireWireSBP2LibLoginInterface 	**	fSBP2LoginInterface;
	IOFireWireSBP2LibMgmtORBInterface 	**	fLUNResetORBInterface;

	// firewire plugin interfaces
	IOCFPlugInInterface 				**	fFWCFPlugInInterface;
	IOFireWireDeviceInterface 			**  fFWDeviceInterface;
	
	// callback methods
	
	static void staticMessage( void * self, UInt32 type, void * arg );
	virtual void message( UInt32 type, void * arg );

	static void staticLoginCompletion( void * self, FWSBP2LoginCompleteParams * params );
	virtual void loginCompletion( FWSBP2LoginCompleteParams * params );

	static void staticLogoutCompletion( void * self, FWSBP2LogoutCompleteParams * params );
	virtual void logoutCompletion( FWSBP2LogoutCompleteParams * params );
	
	static void staticUnsolicitedStatusNotify( void * self, FWSBP2NotifyParams * params );
	virtual void unsolicitedStatusNotify( FWSBP2NotifyParams * params );
	
	static void staticStatusNotify( void * self, FWSBP2NotifyParams * params );
	virtual void statusNotify( FWSBP2NotifyParams * params );
	
	static void staticLUNResetCompletion( void * refCon, IOReturn status, void * orb );
	virtual void LUNResetCompletion( IOReturn status, void * orb );
	
	static void staticFetchAgentResetCompletion(  void * self, IOReturn status );
	virtual void fetchAgentResetCompletion( IOReturn status );
	
	static void staticFetchAgentWriteCompletion(  void * self, IOReturn status, void * refCon );
	virtual void fetchAgentWriteCompletion( IOReturn status, void * refCon );


public:

	// orb factory
	virtual SBP2SampleORB * createORB( void );
	
	// IOCFPlugin virtual methods
    virtual IOReturn start( CFDictionaryRef propertyTable, io_service_t service );
	virtual IOReturn stop( void );
	
	// SBP2SampleDriverInterface virtual methods
	virtual IOReturn setUpCallbacksWithRunLoop( CFRunLoopRef cfRunLoopRef );
};

#endif
