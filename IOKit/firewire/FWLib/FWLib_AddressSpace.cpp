/*
	File:		FWLib_AddressSpace.cpp

	Synopsis: 	C++ classes representing FireWire address spaces. Corresponds to address
				space interfaces in IOFireWireLib.

	Copyright: 	© Copyright 2001-2003 Apple Computer, Inc. All rights reserved.

	Written by: NWG
	
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

	$Log: FWLib_AddressSpace.cpp,v $
	Revision 1.5  2005/02/02 23:20:16  firewire
	*** empty log message ***
	
	Revision 1.4  2004/02/06 23:29:11  firewire
	*** empty log message ***
	
	Revision 1.3  2003/05/27 18:12:45  firewire
	SDK16
	
	Revision 1.2  2003/03/13 02:34:18  firewire
	*** empty log message ***
	
	Revision 1.1  2002/11/07 00:32:11  noggin
	move to new repository
	
	Revision 1.3  2002/10/18 00:27:28  noggin
	added more methods to device interface
	added absolute addressing to command objects
	
	Revision 1.2  2002/10/14 22:24:55  noggin
	SDK14
	
	Revision 1.1  2002/06/05 19:20:48  noggin
	names changed. namespace names changes. nested IO in FWLib
	
	Revision 1.2  2002/05/31 21:21:52  noggin
	added version logging to all files
	
	Revision 1.1  2002/05/31 21:12:37  noggin
	first check-in
	
*/

#import "FWLib_AddressSpace.h"
#import "FWLib_Device.h"

#define kPseudoAddressSpaceIID	(CFUUIDGetUUIDBytes( kIOFireWirePseudoAddressSpaceInterfaceID ))
#define kPhysicalAddressSpaceIID (CFUUIDGetUUIDBytes( kIOFireWirePhysicalAddressSpaceInterfaceID ))


namespace FWLib {

	PseudoAddressSpace::PseudoAddressSpace( Device& device, IOByteCount size, void * backingStore,
			UInt32 flags, IOByteCount queueBytes )
	: mAddressSpace((**device.Interface()).CreatePseudoAddressSpace( device.Interface(), size,
						this, queueBytes ? queueBytes : size * 2, mBackingStore, flags, 
						kPseudoAddressSpaceIID ))
	{	
		if (!mAddressSpace)
			throw ;
	}
		
	PseudoAddressSpace::~PseudoAddressSpace()
	{
		if (mAddressSpace)
			(**mAddressSpace).Release(mAddressSpace) ;
			
		delete[] mBackingStore ;
	}
	
	UInt32
	PseudoAddressSpace::HandleWrite( FWClientCommandID commandID, UInt32 packetLen, void* packet,
		UInt16 srcNodeID, UInt32 destAddressHi, UInt32 destAddressLo)
	{
		ClientCommandIsComplete( commandID, kIOReturnSuccess ) ;
		
		return 0 ;
	}
	
	UInt32
	PseudoAddressSpace::S_HandleWrite( AddressSpaceRef addressSpace, FWClientCommandID commandID, UInt32 packetLen,
			void* packet, UInt16 srcNodeID, UInt32 destAddressHi, UInt32 destAddressLo, UInt32 refCon)
	{
		PseudoAddressSpace*		me = (PseudoAddressSpace*)refCon ;
		return me->HandleWrite( commandID, packetLen, packet, srcNodeID, destAddressHi, destAddressLo ) ;
	}

	void
	PseudoAddressSpace::HandleSkip( FWClientCommandID commandID, UInt32 skippedPacketCount )
	{	
		ClientCommandIsComplete( commandID, kIOReturnSuccess ) ;
	}
	
	void
	PseudoAddressSpace::S_HandleSkip( AddressSpaceRef addressSpace, FWClientCommandID commandID, 
		UInt32 skippedPacketCount )
	{
		reinterpret_cast<PseudoAddressSpace*>((**addressSpace).GetRefCon( addressSpace ))->HandleSkip( commandID, skippedPacketCount ) ;
	}
	
}

namespace FWLib {

	PhysicalAddressSpace :: PhysicalAddressSpace (
		Device &					device, 
		IOByteCount					size, 
		void *						backingStore,
		UInt32						flags )
	: mAddressSpace( (**device.Interface()).CreatePhysicalAddressSpace( device.Interface(), size, backingStore, flags, kPhysicalAddressSpaceIID ) )
	{
		if ( !mAddressSpace )
		{
			throw ;
		}
	}

	PhysicalAddressSpace :: ~PhysicalAddressSpace ()
	{
		(**mAddressSpace).Release( mAddressSpace ) ;
	}
}
