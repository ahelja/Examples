/*
	File:		IOFireWireLibGlue.h

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.

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

*/

#ifndef __IOFireWireLibGlue_H__
#define __IOFireWireLibGlue_H__

//#import <Carbon/Carbon.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/firewire/IOFireWireLib.h>

class CIOFireWireDeviceInterface
{
 public:
						CIOFireWireDeviceInterface() ;
	virtual				~CIOFireWireDeviceInterface() ;
	
	IOReturn					InitWithService(io_service_t inObject) ;
	inline io_object_t			GetDevice() 
										{ return (*mDeviceInterface)->GetDevice(mDeviceInterface) ; }
	inline IOReturn				Open()
										{ return (*mDeviceInterface)->Open(mDeviceInterface); }
	inline void					Close()
										{ (*mDeviceInterface)->Close(mDeviceInterface); }

/*{
	IUNKNOWN_C_GUTS ;

	UInt32 version, revision ; // version/revision
	
		// --- maintenance methods -------------
	bool				(*InterfaceIsInited)(IOFireWireLibDeviceRef self) ;
	io_object_t			(*WhichDevice)(IOFireWireLibDeviceRef self) ;
	IOReturn			(*Open)(IOFireWireLibDeviceRef self) ;
	IOReturn			(*OpenWithSessionRef)(IOFireWireLibDeviceRef self, IOFireWireSessionRef sessionRef) ;
	void				(*Close)(IOFireWireLibDeviceRef self) ;
	
		// --- notification --------------------
	const bool		(*NotificationIsOn)(IOFireWireLibDeviceRef self) ; */
	const IOReturn 		AddCallbackDispatcherToRunLoop(CFRunLoopRef inRunLoop)
						{
							return (*mDeviceInterface)->AddCallbackDispatcherToRunLoop(mDeviceInterface, inRunLoop) ;
						}
						
	const void			RemoveCallbackDispatcherFromRunLoop()
						{
							(*mDeviceInterface)->RemoveCallbackDispatcherFromRunLoop(mDeviceInterface) ;
						}
						
/*	const bool 		(*TurnOnNotification)(IOFireWireLibDeviceRef self, CFRunLoopRef	inRunLoop) ;
	void				(*TurnOffNotification)(IOFireWireLibDeviceRef self) ;
	const IOFireWireBusResetHandler	
						(*SetBusResetHandler)(IOFireWireLibDeviceRef self, IOFireWireBusResetHandler handler) ;
	const IOFireWireBusResetDoneHandler	
						(*SetBusResetDoneHandler)(IOFireWireLibDeviceRef self, IOFireWireBusResetDoneHandler handler) ;
	void				(*ClientCommandIsComplete)(IOFireWireLibDeviceRef self, FWClientCommandID commandID, IOReturn status) ;
	
		// --- read/write/lock operations ------- */
	IOReturn			Read(
								io_object_t 		device, 
								const FWAddress* 	addr, 
								void* 				buf, 
								UInt32* 			size, 
								bool 			failOnReset, 
								UInt32 				generation)
								{ 
									return (*mDeviceInterface)->Read(mDeviceInterface, device, addr, buf, size, failOnReset, generation) ; 
								}
	
	IOReturn			ReadQuadlet(
								io_object_t 		device, 
								const FWAddress*	addr, 
								UInt32* 			val, 
								bool 			failOnReset, 
								UInt32 				generation)
								{ 
									return (*mDeviceInterface)->ReadQuadlet(mDeviceInterface, device, addr, val, failOnReset, generation); 
								}
	IOReturn			Write(
								io_object_t 		device, 
								const FWAddress* 	addr, 
								const void* 		buf, 
								UInt32* 			size, 
								bool 			failOnReset, 
								UInt32 				generation)
								{ 
									return (*mDeviceInterface)->Write(mDeviceInterface, device, addr, buf, size, failOnReset, generation); 
								}
	IOReturn			WriteQuadlet(
								io_object_t 		device, 
								const FWAddress* 	addr, 
								const UInt32 		val, 
								bool 			failOnReset, 
								UInt32 				generation)
								{
									return (*mDeviceInterface)->WriteQuadlet(mDeviceInterface, device, addr, val, failOnReset, generation) ;
								}
	IOReturn			CompareSwap(
								io_object_t 		device, 
								const FWAddress* 	addr, 
								UInt32 				cmpVal, 
								UInt32 				newVal,
								bool 			failOnReset, 
								UInt32 				generation)
								{
									return (*mDeviceInterface)->CompareSwap(mDeviceInterface, device, addr, cmpVal, newVal, failOnReset, generation) ;
								}
	
	// --- FireWire command object methods ---------

	IOFireWireLibCommandRef	CreateReadCommand(
									io_object_t			device,
									const FWAddress *	addr,
									void*				buf,
									UInt32				size,
									IOFireWireLibCommandCallback callback,
									bool				failOnReset,
									UInt32				generation,
									void*				inRefCon) ;
/*	IOFireWireLibCommandRef	CreateReadQuadletCommand(
									io_object_t			device,
									const FWAddress *	addr,
									UInt32				quads[],
									UInt32				numQuads,
									IOFireWireLibCommandCallback callback,
									bool				failOnReset,
									UInt32				generation,
									void*				inRefCon ) ; */
	IOFireWireLibCommandRef
							CreateWriteCommand(
									io_object_t			device,
									const FWAddress *	addr,
									void*				buf,
									UInt32 				size,
									IOFireWireLibCommandCallback callback,
									bool				failOnReset,
									UInt32				generation,
									void*				inRefCon)
									{
										return (*mDeviceInterface)->CreateWriteCommand(
													mDeviceInterface, device, addr, buf, size, callback,
													failOnReset, generation, inRefCon,
													CFUUIDGetUUIDBytes(kIOFireWireWriteCommandInterfaceID_v2) ) ;
									}
/*	IOFireWireLibCommandRef	CreateWriteQuadletCommand(
									io_object_t			device,
									const FWAddress *	addr,
									UInt32				quads[],
									UInt32				numQuads,
									IOFireWireLibCommandCallback callback,
									bool				failOnReset,
									UInt32				generation,
									void*				inRefCon)
									{
										return (*mDeviceInterface)->CreateWriteQuadletCommand(
													mDeviceInterface, device, addr, quads, numQuads, callback, 
													failOnReset, generation, inRefCon, 
													CFUUIDGetUUIDBytes(kIOFireWireWriteQuadletCommandInterfaceID)) ;
									} */
	IOFireWireLibCommandRef
						CreateCompareSwapCommand(
									io_object_t			device,
									const FWAddress *	addr,
									UInt32 				cmpVal,
									UInt32 				newVal,
									IOFireWireLibCommandCallback callback,
									bool				failOnReset,
									UInt32				generation,
									void*				inRefCon)
									{
										return (*mDeviceInterface)->CreateCompareSwapCommand( mDeviceInterface, device, addr, cmpVal, newVal, callback, failOnReset, generation, inRefCon, CFUUIDGetUUIDBytes( kIOFireWireCompareSwapCommandInterfaceID ) ) ;
									}

		// --- other methods ---------------------------
	IOReturn			BusReset()
								{ return (*mDeviceInterface)->BusReset(mDeviceInterface) ; }
	IOReturn			GetCycleTime(
								UInt32* outCycleTime)
								{
									return (*mDeviceInterface)->GetCycleTime(mDeviceInterface, outCycleTime) ;
								}
	IOReturn			GetGenerationAndNodeID(
								UInt32* outGeneration, 
								UInt16* outNodeID)
								{
									return (*mDeviceInterface)->GetGenerationAndNodeID(mDeviceInterface, outGeneration, outNodeID) ;
								}
	IOReturn			GetLocalNodeID(
								UInt16* outLocalNodeID)
								{
									return (*mDeviceInterface)->GetLocalNodeID(mDeviceInterface, outLocalNodeID) ;
								}
	IOReturn			GetResetTime(
								AbsoluteTime* outResetTime)
								{
									return (*mDeviceInterface)->GetResetTime(mDeviceInterface, outResetTime) ;
								}

/*		// --- unit directory support ------------------
	IOFireWireLibUnitDirectoryRef
						(*CreateUnitDirectory)(IOFireWireLibDeviceRef self, REFIID iid) ;

		// --- address space support -------------------
	IOFireWireLibPseudoAddressSpaceRef
						(*CreatePseudoAddressSpace)(IOFireWireLibDeviceRef self, UInt32 inSize, void* inRefCon, UInt32 inQueueBufferSize, void* inBackingStore, UInt32 inFlags, REFIID iid) ;
	IOFireWireLibPhysicalAddressSpaceRef
						(*CreatePhysicalAddressSpace)(IOFireWireLibDeviceRef self, UInt32 inSize, void* inBackingStore, UInt32 inFlags, REFIID iid) ;
		
		// --- debugging -------------------------------
	IOReturn			(*FireBugMsg)(IOFireWireLibDeviceRef self, const char* msg) ;
}*/

 protected:
	IOFireWireLibDeviceRef	mDeviceInterface ;
};

#endif