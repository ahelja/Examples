/*
	File:		FWLib_Command.h

	Synopsis: 	C++ classes representing FireWire command objects. Corresponds to command
				object interfaces in IOFireWireLib.

	Copyright: 	© Copyright 2002-2003 Apple Computer, Inc. All rights reserved.

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

	$Log: FWLib_Command.h,v $
	Revision 1.4  2004/02/06 23:29:11  firewire
	*** empty log message ***
	

*/
/*
 *  FWLib_Command.h
 *  FWLib
 *
 *  Created by Niels on Wed May 29 2002.
 *  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 */

/*
	$Log: FWLib_Command.h,v $
	Revision 1.4  2004/02/06 23:29:11  firewire
	*** empty log message ***
	
	Revision 1.1  2002/11/07 00:32:12  noggin
	move to new repository
	
	Revision 1.5  2002/10/18 00:27:29  noggin
	added more methods to device interface
	added absolute addressing to command objects
	
	Revision 1.4  2002/08/21 22:26:02  noggin
	*** empty log message ***
	
	Revision 1.3  2002/06/12 02:09:49  noggin
	*** empty log message ***
	
	Revision 1.2  2002/06/07 21:25:41  noggin
	*** empty log message ***
	
	Revision 1.1  2002/06/05 19:20:46  noggin
	names changed. namespace names changes. nested IO in FWLib
	
	Revision 1.2  2002/05/31 21:21:50  noggin
	added version logging to all files
	
	Revision 1.1  2002/05/31 21:12:37  noggin
	first check-in
	
*/

#import "FWLib_Device.h"
#import <IOKit/firewire/IOFireWireLib.h>

namespace FWLib {

	class Command
	{
		protected:
			typedef ::IOFireWireLibCommandCallback CommandCallback ;
			typedef ::IOFireWireLibCommandRef CmdRef ;
		
		private:
			explicit Command() {}
		protected:
			explicit Command( IOFireWireLibCommandRef interface ) ;
		public:
			virtual ~Command() ;

		public:
			IOReturn			GetStatus()											{ return (**mCmd).GetStatus(mCmd) ; }
			UInt32				GetTransferredBytes	()								{ return (**mCmd).GetTransferredBytes(mCmd) ; }
			void				GetTargetAddress( FWAddress& addr)					{ (**mCmd).GetTargetAddress(mCmd, & addr) ; }
			void				SetTarget( const FWAddress& addr )					{ (**mCmd).SetTarget( mCmd, & addr ) ; }
			void				SetGeneration(UInt32 generation)					{ (**mCmd).SetGeneration( mCmd, generation) ; }
			void				SetCallback(IOFireWireLibCommandCallback callback)	{ (**mCmd).SetCallback( mCmd, callback) ; }
			void				SetRefCon(void* refcon)								{ (**mCmd).SetRefCon( mCmd, refcon) ; }
			bool				IsExecuting()										{ return (bool)((**mCmd).IsExecuting(mCmd)) ; }
			virtual IOReturn	Submit()											{ return (**mCmd).Submit(mCmd) ; }
			IOReturn			SubmitWithRefconAndCallback( void* refcon,
										IOFireWireLibCommandCallback callback )		{ return (**mCmd).SubmitWithRefconAndCallback( mCmd, refcon, callback) ; }
			IOReturn			Cancel( IOReturn reason ) 							{ return (**mCmd).Cancel( mCmd, reason) ; }

		protected:
			CmdRef	mCmd;
	} ;
	
#pragma mark -
	class BlockCommand: public Command
	{
		public:
			BlockCommand( CmdRef interface ): Command(interface)	{}

		public:
			void		SetBuffer( IOByteCount size, void* buf )				{ (**mCmd).SetBuffer( mCmd, size, buf ) ; }
			void 		GetBuffer( IOByteCount& size, void*& outBuf )			{ (**mCmd).GetBuffer( mCmd, & size, & outBuf ) ; }
			IOReturn	SetMaxPacket( IOByteCount maxPacketSize )				{ return (**mCmd).SetMaxPacket( mCmd, maxPacketSize ) ; }
			void		SetFlags(UInt32 flags)									{ (**mCmd).SetFlags(mCmd, flags) ; }
	} ;
	
#pragma mark -
	class WriteCommand: public BlockCommand
	{
		public:
			WriteCommand( Device& device, const FWAddress& addr, const void* buf, IOByteCount size, 
					IOFireWireLibCommandCallback callback = nil, bool abs = false, bool failOnReset = kFWDontFailOnReset, 
					UInt32 generation = 0 ) ;
	} ;
	
#pragma mark -
	class ReadCommand: public BlockCommand
	{
		public:
			ReadCommand( Device& device, const FWAddress& addr, void* buf, IOByteCount size,
					IOFireWireLibCommandCallback callback = nil, bool abs = false, bool failOnReset = kFWDontFailOnReset, 
					UInt32 generation = 0 ) ;
	} ;
	
#pragma mark -
	class CompareSwapCommand: public Command
	{
		private:
			typedef ::IOFireWireLibCompareSwapCommandRef CmdRef ;
	
		public:
			CompareSwapCommand( Device& device, const FWAddress& addr, UInt32 cmpVal, UInt32 newVal,
					IOFireWireLibCommandCallback callback = nil, bool abs = false, bool failOnReset = kFWDontFailOnReset, 
					UInt32 generation = 0 ) ;
		public:
			void		SetValues( UInt32 expected, UInt32 replacement )	{ (**reinterpret_cast<CmdRef>(mCmd)).SetValues( reinterpret_cast<CmdRef>(mCmd), expected, replacement ) ; }
			bool		DidLock()											{ return (**reinterpret_cast<CmdRef>(mCmd)).DidLock(reinterpret_cast<CmdRef>(mCmd)) ; }
			IOReturn	Locked( UInt32& oldValue )							{ return (**reinterpret_cast<CmdRef>(mCmd)).Locked( reinterpret_cast<CmdRef>(mCmd), & oldValue) ; }
			void		SetFlags( UInt32 flags )							{ (**reinterpret_cast<CmdRef>(mCmd)).SetFlags(reinterpret_cast<CmdRef>(mCmd), flags) ; }
	} ;
// unimplemented:
//	void	(*SetValues64)(IOFireWireLibCompareSwapCommandRef self, UInt64 cmpVal, UInt64 newVal) ;
//	IOReturn (*Locked64)(IOFireWireLibCompareSwapCommandRef self, UInt64* oldValue) ;

}
