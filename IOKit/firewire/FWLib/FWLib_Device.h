/*
	File:		FWLib_Device.h

	Synopsis: 	C++ class to represent an device on the FireWire bus. Corresponds
				to IOFireWireDeviceInterface.

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

	$Log: FWLib_Device.h,v $
	Revision 1.5  2005/02/02 23:20:17  firewire
	*** empty log message ***
	
	Revision 1.4  2004/02/06 23:29:12  firewire
	*** empty log message ***
	
	Revision 1.3  2003/05/27 18:12:46  firewire
	SDK16
	
	Revision 1.2  2003/03/13 02:34:18  firewire
	*** empty log message ***
	
	Revision 1.1  2002/11/07 00:32:13  noggin
	move to new repository
	
	Revision 1.6  2002/10/18 00:27:29  noggin
	added more methods to device interface
	added absolute addressing to command objects
	
	Revision 1.5  2002/10/14 22:24:55  noggin
	SDK14
	
	Revision 1.4  2002/08/21 22:26:02  noggin
	*** empty log message ***
	
	Revision 1.3  2002/06/12 02:09:50  noggin
	*** empty log message ***
	
	Revision 1.2  2002/06/07 21:25:42  noggin
	*** empty log message ***
	
	Revision 1.1  2002/06/05 19:20:47  noggin
	names changed. namespace names changes. nested IO in FWLib
	
	Revision 1.2  2002/05/31 21:21:51  noggin
	added version logging to all files
	
	Revision 1.1  2002/05/31 21:12:37  noggin
	first check-in
	
*/

/*! @header FWLib_Device.h

	This file contains C++ class abstractions of the COM interfaces in
	IOFireWireLib. I guess it's sort of like the C++ interface we never had. 
	Feel free to use it however you like.
*/


#import <CoreFoundation/CoreFoundation.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/firewire/IOFireWireLib.h>

namespace FWLib {

	class ReadCommand ;
	class WriteCommand ;
	class CompareSwapCommand ;
	class LocalUnitDirectory ;
	class PseudoAddressSpace ;
	class ConfigDirectory ;
	class NuDCLPool ;
	
	class Device
	{
			typedef ::IOFireWireLibDeviceRef DeviceRef ;
			
		private:
		
			Device( const Device& rVal ) {}
			
		public:
		
			// --- ctor/dtor
			Device( const io_service_t service ) ;
			virtual ~Device() ;
		
		public:
		
			io_service_t	Service()															{ return (**mDevice).GetDevice(mDevice) ; }

			// --- client methods
			IOReturn		Open() 																{ return (**mDevice).Open( mDevice ) ; }
			void			Close()																{ (**mDevice).Close( mDevice ) ; }

			// bus transactions
			IOReturn		Write( const FWAddress& addr, const void* buf, 
									IOByteCount& size, bool abs = false,
									bool failOnReset = kFWDontFailOnReset, 
									UInt32 generation = 0) const								{ return (**mDevice).Write( mDevice, abs ? 0 : (**mDevice).GetDevice( mDevice ), & addr, buf, & size, failOnReset, generation) ; }
			IOReturn		WriteQuadlet( const FWAddress & addr, UInt32 quad, 
									bool abs = false, bool failOnReset = kFWDontFailOnReset,
									UInt32 generation = 0 ) const								{ return (**mDevice).WriteQuadlet( mDevice, abs ? 0 : (**mDevice).GetDevice( mDevice ), & addr, quad, failOnReset, generation ) ; }
			IOReturn		Read( const FWAddress& addr, void* buf, IOByteCount& size,
									bool abs = false, bool failOnReset = kFWDontFailOnReset,
									UInt32 generation = 0 ) const								{ return (**mDevice).Read( mDevice, abs ? 0 : (**mDevice).GetDevice( mDevice ), & addr, buf, & size, failOnReset, generation ) ; }
			IOReturn		ReadQuadlet( const FWAddress & addr, UInt32 * quad, 
									bool abs = false, bool failOnReset = kFWDontFailOnReset,
									UInt32 generation = 0 ) const								{ return (**mDevice).ReadQuadlet( mDevice, abs ? 0 : (**mDevice).GetDevice( mDevice ), & addr, quad, failOnReset, generation ) ; }
			IOReturn		CompareSwap64( const FWAddress& addr, 
									UInt32* expectedVal, UInt32* newVal, UInt32* oldVal, 
									IOByteCount size, bool abs = false, 
									bool failOnReset = kFWDontFailOnReset, 
									UInt32 generation = 0 )										{ return (**mDevice).CompareSwap64( mDevice, abs ? 0 : (**mDevice).GetDevice( mDevice ), &addr, expectedVal, newVal, oldVal, size, failOnReset, generation ) ; }
			IOReturn		FireBugMsg( const char* msg ) const									{ return (**mDevice).FireBugMsg( mDevice, msg ) ; }
			
			// bus misc.
			IOReturn		BusReset()															{ return (**mDevice).BusReset( mDevice ) ; }

			// user client management
			IOReturn		AddCallbackDispatcherToRunLoop( 
									CFRunLoopRef runloop = ::CFRunLoopGetCurrent() ) const		{ return (**mDevice).AddCallbackDispatcherToRunLoop( mDevice, runloop ) ; }
			IOReturn		AddIsochCallbackDispatcherToRunLoop(
									CFRunLoopRef runloop = ::CFRunLoopGetCurrent() ) ;
			void			RemoveCallbackDispatcherFromRunLoop() const							{ (**mDevice).RemoveCallbackDispatcherFromRunLoop( mDevice ) ; }
			void			RemoveIsochCallbackDispatcherFromRunLoop() const					{ (**mDevice).RemoveIsochCallbackDispatcherFromRunLoop( mDevice ) ; }
			IOReturn		Seize() const														{ return (**mDevice).Seize( mDevice, 0 ) ; }

			// topology
			IOReturn		GetBusGeneration( UInt32& generation ) const						{ return (**mDevice).GetBusGeneration( mDevice, & generation ) ; }
			IOReturn		GetLocalNodeIDWithGeneration( UInt32 checkGeneration, 
									UInt16& localNodeID ) const									{ return (**mDevice).GetLocalNodeIDWithGeneration( mDevice, checkGeneration, & localNodeID ) ; }
			IOReturn		GetRemoteNodeID( UInt32 checkGeneration, 
									UInt16& remoteNodeID ) const								{ return (**mDevice).GetRemoteNodeID( mDevice, checkGeneration, & remoteNodeID ) ; }
			IOReturn		GetSpeedToNode( UInt32 checkGeneration, IOFWSpeed& speed ) const	{ return (**mDevice).GetSpeedToNode( mDevice, checkGeneration, & speed ) ; }
			IOReturn		GetSpeedBetweenNodes( UInt32 checkGeneration, UInt32 srcNodeID,
									UInt16 destNodeID, IOFWSpeed& speed ) const					{ return (**mDevice).GetSpeedBetweenNodes( mDevice, checkGeneration, srcNodeID, destNodeID, &speed ) ; }

		public:
		
			DeviceRef		Interface() const													{ return mDevice ; }

		private:
		
			void 			InitWithService( io_service_t service ) ;

		protected:
		
			IOCFPlugInInterface**		mIOCFPlugInInterface ;
			DeviceRef					mDevice ;
			CFRunLoopRef				mIsochRunLoop ;
	} ;	
}
