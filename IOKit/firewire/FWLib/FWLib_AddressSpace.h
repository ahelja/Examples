/*
	File:		FWLib_AddressSpace.h

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

	Change History (most recent first):

	$Log: FWLib_AddressSpace.h,v $
	Revision 1.3  2004/02/06 23:29:11  firewire
	*** empty log message ***
	
	Revision 1.2  2003/05/27 18:12:45  firewire
	SDK16
	
	Revision 1.1  2002/11/07 00:32:12  noggin
	move to new repository
	
	Revision 1.4  2002/10/18 00:27:29  noggin
	added more methods to device interface
	added absolute addressing to command objects
	
	Revision 1.3  2002/06/20 19:10:07  noggin
	*** empty log message ***
	
	Revision 1.2  2002/06/12 03:54:51  noggin
	added to repository
	
	Revision 1.1  2002/06/05 19:20:48  noggin
	names changed. namespace names changes. nested IO in FWLib
	
	Revision 1.2  2002/05/31 21:21:52  noggin
	added version logging to all files
	
	Revision 1.1  2002/05/31 21:12:37  noggin
	first check-in
	
*/

#ifndef __FWLIB_ADDRESSSPACE_H
#define __FWLIB_ADDRESSSPACE_H

#import <IOKit/firewire/IOFireWireLib.h>

namespace FWLib {

	class Device ;
	class AddressSpace
	{
		protected:
									AddressSpace() {}
			virtual					~AddressSpace() {}
	} ;
	
#pragma mark -
	class PseudoAddressSpace: public AddressSpace
	{
		typedef ::IOFireWirePseudoAddressSpaceWriteHandler 			WriteHandler ;
		typedef ::IOFireWirePseudoAddressSpaceSkippedPacketHandler	SkippedPacketHandler ;
		typedef ::IOFireWireLibPseudoAddressSpaceRef 				AddressSpaceRef ;
		
		public:
		
			PseudoAddressSpace				( Device& inDevice, IOByteCount inSize, void * backingStore = nil, UInt32 inFlags = kFWAddressSpaceNoFlags, IOByteCount inQueueBytes = 0 ) ;
			virtual	~PseudoAddressSpace		() ;

		public:
		
			void 						GetFWAddress				( FWAddress& outAddress ) const										{ (**mAddressSpace).GetFWAddress( mAddressSpace, & outAddress ) ; }
			FWAddress					GetFWAddress				() const															{ FWAddress result; GetFWAddress( result ); return result ; }
			bool 						TurnOnNotification			() const															{ return (**mAddressSpace).TurnOnNotification( mAddressSpace ) ; }

		protected:
		
			virtual	UInt32				HandleWrite					( FWClientCommandID commandID, UInt32 packetLen, void* packet, UInt16 srcNodeID, UInt32 destAddressHi, UInt32 destAddressLo) ;
			static UInt32				S_HandleWrite				( AddressSpaceRef addressSpace, FWClientCommandID commandID, UInt32 packetLen, void* packet, UInt16 srcNodeID, UInt32 destAddressHi, UInt32 destAddressLo, UInt32 refCon) ;
			virtual	void				HandleSkip					( FWClientCommandID commandID, UInt32 skippedPacketCount ) ;
			static void					S_HandleSkip				( AddressSpaceRef addressSpace, FWClientCommandID commandID, UInt32 skippedPacketCount ) ;

			// interface methods
			const WriteHandler			SetWriteHandler				( WriteHandler writer = & S_HandleWrite )							{ return (**mAddressSpace).SetWriteHandler( mAddressSpace, writer ) ; }
			const SkippedPacketHandler	SetSkippedPacketHandler		( SkippedPacketHandler handler = & S_HandleSkip )			{ return (**mAddressSpace).SetSkippedPacketHandler( mAddressSpace, handler ) ; }
			void						ClientCommandIsComplete		( FWClientCommandID commandID, IOReturn status ) const		{ (**mAddressSpace).ClientCommandIsComplete( mAddressSpace, commandID, status ) ; }

		protected:
		
			UInt8*			mBackingStore ;
			AddressSpaceRef	mAddressSpace ;
	} ;

	class PhysicalAddressSpace: public AddressSpace
	{
		typedef ::IOFireWireLibPhysicalAddressSpaceRef 				AddressSpaceRef ;

		public:
		
			PhysicalAddressSpace( Device & device, IOByteCount size, void * backingStore, UInt32 flags = 0 ) ;
			virtual ~PhysicalAddressSpace() ;
			
		public:
		
			inline unsigned				GetPhysicalSegments( UInt32 & ioSegmentCount, IOByteCount outSegments[], IOPhysicalAddress outAddresses[] ) ;
			
		private :
		
			AddressSpaceRef			mAddressSpace ;
	} ;

	inline unsigned
	PhysicalAddressSpace :: GetPhysicalSegments (
		UInt32 &				ioSegmentCount, 
		IOByteCount				outSegments[], 
		IOPhysicalAddress		outAddresses[] )
	{
		(**mAddressSpace).GetPhysicalSegments( mAddressSpace, & ioSegmentCount, outSegments, outAddresses ) ;
		return ioSegmentCount ;
	}
	
} // namespace

#endif
