/*
	File:		FWLib_Command.cpp

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

	$Log: FWLib_Command.cpp,v $
	Revision 1.4  2004/02/06 23:29:11  firewire
	*** empty log message ***
	

*/
/*
 *  FWLib_Command.cpp
 *  FWLib
 *
 *  Created by Niels on Wed May 29 2002.
 *  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 */

/*
	$Log: FWLib_Command.cpp,v $
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
	
	Revision 1.2  2002/06/07 21:25:42  noggin
	*** empty log message ***
	
	Revision 1.1  2002/06/05 19:20:47  noggin
	names changed. namespace names changes. nested IO in FWLib
	
	Revision 1.2  2002/05/31 21:21:51  noggin
	added version logging to all files
	
	Revision 1.1  2002/05/31 21:12:37  noggin
	first check-in
	
*/

#import "FWLib_Command.h"
#import <IOKit/firewire/IOFireWireLib.h>

#define kReadCommandIID	(CFUUIDGetUUIDBytes( kIOFireWireReadCommandInterfaceID_v2 ))
#define kWriteCommandIID (CFUUIDGetUUIDBytes( kIOFireWireWriteCommandInterfaceID_v2 ))
#define kCompareSwapCommandIID (CFUUIDGetUUIDBytes( kIOFireWireCompareSwapCommandInterfaceID_v2 ))

namespace FWLib {

#pragma mark -
	Command::Command( IOFireWireLibCommandRef interface )
	: mCmd(interface)
	{
		if ( !mCmd )
		{
			throw kIOReturnError ;
		}
	}

	Command::~Command()
	{
		if (mCmd)
			(**mCmd).Release( mCmd ) ;
	}

#pragma mark -
	WriteCommand::WriteCommand( Device& device,  const FWAddress& addr,  const void* buf,  IOByteCount size, 
			IOFireWireLibCommandCallback callback,  bool abs,  bool failOnReset,  UInt32 generation )
	: BlockCommand( (**device.Interface()).CreateWriteCommand( device.Interface(), 
			abs ? 0 : (**device.Interface()).GetDevice(device.Interface()), 
			&addr, const_cast<void*>(buf), size, callback, failOnReset, generation, this, kWriteCommandIID ))
	{
	}
	
#pragma mark -
	ReadCommand::ReadCommand( Device& device,  const FWAddress& addr,  void* buf,  IOByteCount size,
			IOFireWireLibCommandCallback callback,  bool abs,  bool failOnReset, UInt32 generation )
	: BlockCommand( (**device.Interface()).CreateReadCommand( device.Interface(), 
			abs ? 0 : (**device.Interface()).GetDevice(device.Interface()), 
			&addr, buf, size, callback, failOnReset, generation, this, kReadCommandIID ))
	{
	}

	CompareSwapCommand::CompareSwapCommand( Device& device,  const FWAddress& addr,  UInt32 cmpVal,  UInt32 newVal,
			IOFireWireLibCommandCallback callback,  bool abs,  bool failOnReset,  UInt32 generation)
	: Command( (**device.Interface()).CreateCompareSwapCommand64( device.Interface(), 
			abs ? 0 : (**device.Interface()).GetDevice(device.Interface()), & addr, cmpVal, newVal, 
			callback, failOnReset, generation, this, kCompareSwapCommandIID ))
	{
	}
}
