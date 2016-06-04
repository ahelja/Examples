/*
	File:		SBP2SampleDriverPlugInGlue.h

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

#ifndef _IOKIT_SBP2SAMPLEDRIVERPLUGINGLUE_H_
#define _IOKIT_SBP2SAMPLEDRIVERPLUGINGLUE_H_

#include <IOKit/IOCFPlugIn.h>

#include "SBP2SampleDriverInterface.h"
#include "SBP2SampleDriver.h"

__BEGIN_DECLS
void *SBP2SampleDriverFactory( CFAllocatorRef allocator, CFUUIDRef typeID );
__END_DECLS

class SBP2SampleDriverPlugInGlue : public SBP2SampleDriver
{

public:
	struct InterfaceMap 
	{
        IUnknownVTbl *pseudoVTable;
        SBP2SampleDriver *obj;
    };
	
	SBP2SampleDriverPlugInGlue( void );
	virtual ~SBP2SampleDriverPlugInGlue();
	
protected:
	static IOCFPlugInInterface 			sIOCFPlugInInterface;
	InterfaceMap 			   			fIOCFPlugInInterface;
	static SBP2SampleDriverInterface	sSBP2SampleDriverInterface;
	InterfaceMap						fSBP2SampleDriverInterface;

	CFUUIDRef 	fFactoryId;	
	UInt32 		fRefCount;
	
	// utility function to get "this" pointer from interface
	static inline SBP2SampleDriverPlugInGlue *getThis( void *self )
        { return (SBP2SampleDriverPlugInGlue *) ((InterfaceMap *) self)->obj; };
	
	// IUnknown static methods
	static HRESULT staticQueryInterface( void * self, REFIID iid, void **ppv );
	static UInt32 staticAddRef( void * self );
	static UInt32 staticRelease( void * self );

	// IUnknown virtual methods
	virtual HRESULT queryInterface( REFIID iid, void **ppv );
	virtual UInt32 addRef( void );
	virtual UInt32 release( void );
	
	// CFPlugin static methods
	static IOReturn staticProbe( void * self, CFDictionaryRef propertyTable, 
								 io_service_t service, SInt32 *order );
    static IOReturn staticStart( void * self, CFDictionaryRef propertyTable, 
								 io_service_t service );
	static IOReturn staticStop( void * self );
	
	// SBP2SampleDriverInterface static methods
	static IOReturn staticSetUpCallbacksWithRunLoop( void *self, CFRunLoopRef cfRunLoopRef );
	static void staticLoginToDevice( void * self );
	static void staticLogoutOfDevice( void * self );
	static IOReturn staticReadBlock( void * self, UInt32 id, UInt32 block, void * buffer );
	static UInt32 staticGetNewTransactionID( void * self );
	static void staticAbortReadsWithTransactionID( void * self, UInt32 id );
	static IOReturn staticWriteBlock( void * self, UInt32 id, UInt32 block, void * buffer );
			
public:
	static IOCFPlugInInterface **alloc( void );
	
};


#endif
