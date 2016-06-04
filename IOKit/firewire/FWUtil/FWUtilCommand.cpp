/*
	File:		FWUtilCommand.cpp

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

#import <unistd.h>
#import <CoreFoundation/CoreFoundation.h>

#import "FWUtil.h"
#import "FWUtilCommand.h"
#import "FWUtilGlobals.h"

#import <FWLib_Command.h>

// ============================================================
// ReadWriteCallback
//
// handles async command completions
// ============================================================
void ReadWriteCallback(void* refCon, IOReturn completionStatus)
{
	FWLib::Command * command = reinterpret_cast<FWLib::Command *>( refCon ) ;
	
	cout << "Async: Command " << command << " completed w/ status " << completionStatus << endl ;
	cout << "Async: Bytes transferred " << command->GetTransferredBytes() << endl ;
}

// ============================================================
// class FWUtilCommand
// ============================================================

FWUtilCommand::FWUtilCommand(): mCommandName(CFSTR("")), 
							  mCommandArray(0) 
{
	if (mCommandName) 
		CFRetain(mCommandName); 
}

FWUtilCommand::FWUtilCommand( CFStringRef commandName, CFArrayRef commandArray )
: mCommandName( commandName )
{
	if ( commandArray )
	{
		mCommandArray = CFArrayCreateMutableCopy(kCFAllocatorDefault, CFArrayGetCount( commandArray ), commandArray ) ;
		if (mCommandArray) 
			CFRetain(mCommandArray);
	}
	else
		mCommandArray = 0 ;
		
	if (mCommandName)  CFRetain(mCommandName);
}

FWUtilCommand::~FWUtilCommand()
{

	if (mCommandArray) 
	{
		CFRelease(mCommandArray);
		mCommandArray = nil ;
	}

	if (mCommandName)
	{
		CFRelease(mCommandName);
		mCommandName = nil ;
	}
}

FWUtilCommand*
FWUtilCommand::CreateWithCFArray( CFArrayRef commandArray )
{
	CFArrayRef		constructors	= gGlobals.GetCommandConstructors() ;
	FWUtilCommand*	result			= nil ;
	CFIndex			index			= 0 ;
	
	CFIndex count = CFArrayGetCount( constructors ) ;
	
	while ((result == nil) && (index < count ) )
	{
		result = (*(CommandCreatorFunction)CFArrayGetValueAtIndex(constructors, index++))(commandArray) ;
	}
	
	if (!result)
		result = new FWUtilNullCommand((CFStringRef) CFArrayGetValueAtIndex(commandArray,0)) ;
		
	return result ;
	
}

ostream & operator << (ostream & outs, const FWUtilCommand & command)
{
	CFStringRef		outString = CFStringCreateByCombiningStrings(
										kCFAllocatorDefault,
										command.mCommandArray,
										kFWUtilSeparatorString) ;
	
	outs << outString ;
	CFRelease(outString) ;
	
	return outs ;
}

// ============================================================
// help <topic>
// FWUtilCommand_help
// ============================================================
IOReturn
FWUtilCommand_help::Execute()
{
	IOReturn		result			= kIOReturnSuccess ;
	int				argCount 		= CFArrayGetCount(mCommandArray) ;
	FWUtilIndenter&	i				= gGlobals.GetIndenter() ;
	
	if (argCount == 1)
	{
		cout << i << "Available commands listed below. Use help <command> for details.\n\n" ;

		CFStringRef		tempString = this->GetHelpText(kFWUtilHelp_Short) ;		// GetHelpText gives us a new reference which we must CFRelease()
		cout << i.BlockIndent(tempString) ;
		CFRelease(tempString) ;
		
	}
	else
	{
		// copy the command we were issued into a new array, but exclude the first word (which is "help")
		CFMutableArrayRef	tempCommandArray	= CFArrayCreateMutable(kCFAllocatorDefault, CFArrayGetCount(mCommandArray) - 1, 0) ;

		for ( CFIndex index = 1, count = CFArrayGetCount(mCommandArray); count < index; ++index )
		{
			CFArraySetValueAtIndex(tempCommandArray, index - 1, CFArrayGetValueAtIndex(mCommandArray, index)) ;
		}

		FWUtilCommand*	command 		= nil ;
		
		CFArrayRef constructors	= gGlobals.GetCommandConstructors() ;
		CFIndex index = 0 ;
		CFIndex count = CFArrayGetCount( constructors ) ;

		while ((command == nil) && (index < count) )
		{
			command = (*(CommandCreatorFunction)CFArrayGetValueAtIndex(constructors, index++))(tempCommandArray) ;
		}
		
		if (command)
		{
			CFStringRef		text = command->GetHelpText(kFWUtilHelp_Full) ;
			
			cout << i.BlockIndent(text) << endl ;
			CFRelease(text) ;
		}	
		else
			throw FWUtilException(CFSTR("Can't open help -- there is no command by that name"), false, __LINE__, __FILE__) ;

	}

	return result ;
}

CFStringRef
FWUtilCommand_help::GetHelpText(FWUtilHelpType	/*inHelpType*/)
{
	CFMutableArrayRef	tempCommand 	= CFArrayCreateMutable(kCFAllocatorDefault, 1, nil) ;
	CFMutableStringRef	result			= CFStringCreateMutable(kCFAllocatorDefault, 0) ;
	CFArrayRef			constructors	= gGlobals.GetCommandConstructors() ;
	CFArrayRef			names			= gGlobals.GetCommandNames() ;
	FWUtilCommand*		command			= nil ;

	for( CFIndex index = 0, count = CFArrayGetCount( constructors ); index < count; index++ )
	{
		// Check here to make sure we're not calling GetHelpText() on the 'help' command to avoid
		// infinite recursion:
		if (kCFCompareEqualTo != CFStringCompare(mCommandName, (CFStringRef) CFArrayGetValueAtIndex(names, index), 0))
		{
			CFArraySetValueAtIndex(tempCommand, 0, CFArrayGetValueAtIndex(names, index)) ;
			command = (*(CommandCreatorFunction)CFArrayGetValueAtIndex(constructors, index))(tempCommand) ;
			
			assert(command) 	;	// zzz debug only

			if (command)
			{
				CFStringAppend(result, (CFStringRef)CFArrayGetValueAtIndex(names, index)) ;
				CFStringAppend(result, CFSTR("\t- ")) ;
				CFStringAppend(result, command->GetHelpText(kFWUtilHelp_Short)) ;
				CFStringAppend(result, CFSTR("\n")) ;
			}
		}
	}

	return result ;
}

// ============================================================
// bread <addr> <size>
// FWUtilCommand_bread
// ============================================================
IOReturn
FWUtilCommand_bread::Execute()
{
	int					argCount	 	= CFArrayGetCount(mCommandArray) ;
	FWUtilIndenter&		i				= gGlobals.GetIndenter() ;
	IOReturn			status			= kIOReturnSuccess ;	
	CFStringRef			word ;
	bool				option_Quiet	= false ;
	bool				option_Async	= false ;
	bool				option_Abs		= false ;
	
	if (argCount < 3)
		throw FWUtilException_TooFewArgs ;
	
	word = PopWord(mCommandArray) ;
	word = PopWord(mCommandArray) ;

	while (IsOptionWord(word))
	{
		if (IsOptionWord(word, CFSTR("quiet")))
			option_Quiet = true ;
		else if (IsOptionWord(word, CFSTR("async")))
			option_Async = true ;
		else if (IsOptionWord(word, CFSTR("abs")))
			option_Abs = true ;
		else
			throw FWUtilException_UnknownOption ;

		word = PopWord(mCommandArray) ;
	}
	
	argCount = CFArrayGetCount(mCommandArray) + 1 ;
	if (argCount > 2)
		throw FWUtilException_TooManyArgs ;
	
	Device* interface = gGlobals.GetDeviceInterface() ;
	if ( !interface )
		throw FWUtilException_NoInterface ;
	
	if ( kIOReturnSuccess == status )
	{
		// convert address to a number
		FWAddress		fwAddress ;
		CFStringGetFWAddress( word, fwAddress ) ;
		
		UInt32			size		= CFStringGetUInt32( 
										PopWordThrow( mCommandArray, 
										FWUtilException( CFSTR("Couldn't read size") ) ) ) ;
		UInt8*			buf			= new Byte[size] ;

		UInt32			generation ;
		IOReturn		result		= interface->GetBusGeneration( generation ) ;

		// normal code
		if (!buf)
			throw FWUtilException_OutOfMemory ;
		
		if (option_Async)
		{
			FWLib::ReadCommand	readCommand( *interface, fwAddress, buf, size, & ReadWriteCallback, option_Abs, kFWDontFailOnReset, generation ) ;
		
			result = readCommand.Submit() ;
			
			cout << i << "Async: Command " << & readCommand << " submitted with status " << result << endl ;
			
			if (kCFRunLoopRunTimedOut == CFRunLoopRunInMode(kCFRunLoopDefaultMode, (CFTimeInterval) 10, true))
			{
				throw FWUtilException(CFSTR("Async: Still no callback after 10 seconds!")) ;
			}
		}
		else
		{
			result = interface->Read( fwAddress, buf, size, option_Abs, kFWDontFailOnReset, generation ) ;
		}
		
		if (!option_Quiet)
		{
			if ( kIOReturnSuccess == result )
			{
				cout << i++ << "BLOCK READ\n" ;
				cout << i	<< "address: 0x" << fwAddress << endl ;
				cout << i	<< "size: 0x" << size << endl ;
				cout << i++	<< "data: " << endl ;
				
				PrettyPrintBuffer( buf, size, i ) ;
			}
			
			if ( result )
				cout << i << "command got error " << result << endl ;

			// unindent!			
			i-- ; i-- ;

		}
		
		delete[] buf ;
	}
		
	return kIOReturnSuccess ;
}

CFStringRef
FWUtilCommand_bread::GetHelpText(FWUtilHelpType	inHelpType)
{
	CFStringRef	result = 0 ;
	
	switch(inHelpType)
	{	
		case kFWUtilHelp_Short:
			result = CFSTR("FireWire block read") ;
			break ;
		case kFWUtilHelp_Full:
			result = CFSTR(
"FireWire block read\n\n\
usage: bread [-quiet] [-async] addr size\n\
\t\tOptions:\n\
\t\t\t-quiet:  Suppress output\n\
\t\t\t-async:  Execute using an asynchronous command\n\
\t\t<addr> = address to read, of the form [xxxx.]xxxx.xxxx.xxxx (period '.' is optional separator)\n\
\t\t<size> = size of block read") ;
			break ;
	}
	
	CFRetain(result) ;
	
	return result ;
}

// ============================================================
// repeat
// FWUtilCommand_repeat
// ============================================================
IOReturn
FWUtilCommand_repeat::Execute()
{
	CFIndex argCount = CFArrayGetCount(mCommandArray) ;
	
	if (argCount < 3)
		throw FWUtilException_TooFewArgs ;
	
	PopWord(mCommandArray) ;
	
	UInt32 			repeatCount 	= CFStringGetUInt32(
											PopWordThrow(mCommandArray, FWUtilException(CFSTR("Couldn't read repeat count"))) ) ;
	FWUtilCommand*	repeatedCommand = nil;
	if (repeatCount > 0)
	{
		for(UInt32 index=0; index<repeatCount; index++)	// this is probably not the optimal way...
		{
			CFArrayRef	repeatedCommandArray = CFArrayCreateMutableCopy(
													kCFAllocatorDefault, 
													0,
													mCommandArray) ;
	
			repeatedCommand = FWUtilCommand::CreateWithCFArray(repeatedCommandArray) ;
		
			if (repeatedCommand)
			{
				try {
					repeatedCommand->Execute() ;
				} catch (FWUtilException e) {
					if (! e.IsSilentException())
						cerr << "\"" << repeatedCommand->GetName() << "\": " << e << endl ;
					
					delete repeatedCommand ;
					
					throw e ;
				}
				delete repeatedCommand ;
			}
		}
	}

    return kIOReturnSuccess ;
}

CFStringRef
FWUtilCommand_repeat::GetHelpText(
	FWUtilHelpType	inHelpType)
{
	CFStringRef	result = 0 ;
	
	switch(inHelpType)
	{	
		case kFWUtilHelp_Short:
			result = CFSTR("Repeat execution of a command") ;
			break ;
		case kFWUtilHelp_Full:
			result = CFSTR(
"Repeat execution of a command\n\n\
usage: repeat count command\n\
\t\t<count> = number of times to repeat command\n\
\t\t<command> = FWUtil command to execute. (Enter just as you would at the '>' prompt)") ;
			break ;
	}
	
	CFRetain(result) ;
	
	return result ;
}

// ============================================================
// busreset
// FWUtilCommand_busreset
// ============================================================
IOReturn
FWUtilCommand_busreset::Execute()
{
    UInt32			argCount	= CFArrayGetCount(mCommandArray) ;

    if (argCount > 1)
		throw FWUtilException_TooManyArgs ;
    
    Device* interface = gGlobals.GetDeviceInterface() ;
    if ( !interface )
		throw FWUtilException_NoInterface ;

    return interface->BusReset() ;
}

CFStringRef
FWUtilCommand_busreset::GetHelpText(
	FWUtilHelpType	inHelpType)
{
	CFStringRef		result = 0 ;
	
	switch(inHelpType)
	{
		case kFWUtilHelp_Short:
			result = CFSTR("Cause a FireWire bus reset") ;
			break ;
		case kFWUtilHelp_Full:
			result	= CFSTR("Cause a FireWire bus reset\n\nusage: busreset") ;
			break ;
	}
	
    CFRetain(result) ;
    return result ;
}

// ============================================================
// bwrite
// FWUtilCommand_bwrite
// ============================================================
IOReturn
FWUtilCommand_bwrite::Execute()
{
	bool							option_Quiet 	= false ;
	bool							option_Async	= false ;
	bool							option_Abs		= false ;
	
	UInt32							argCount		= CFArrayGetCount(mCommandArray) ;
	Device*							interface		= gGlobals.GetDeviceInterface() ;
	FWAddress	 					addr ;

	// check we have an interface to talk to
	if ( !interface )
		throw FWUtilException_NoInterface ;
		
	// check number of arguments ;
	if (argCount < 2)
		throw FWUtilException_TooFewArgs ;
	
	//
	// get rid of command name
	//
	PopWord( mCommandArray ) ;

	//
	// get first argument
	//
	CFStringRef 		word = PopWord(mCommandArray) ;
	
	//
	// check for option flags
	//
	while ( IsOptionWord( word ) )
	{
		if ( IsOptionWord( word, CFSTR("quiet") ) )
			option_Quiet = true ;
		else if ( IsOptionWord( word, CFSTR("async") ) )
			option_Async = true ;
		else if ( IsOptionWord( word, CFSTR("abs") ) )
			option_Abs = true ;
		else
			throw FWUtilException_UnknownOption ;
			
		word = PopWord( mCommandArray ) ;
	}

	argCount = ::CFArrayGetCount( mCommandArray ) ;
	if ( argCount > 3 )
		throw FWUtilException_TooManyArgs ;
	if ( argCount < 2 )
		throw FWUtilException_TooFewArgs ;
		
	if ( ! word )
		throw FWUtilException(CFSTR("Could not parse address")) ;
	
	CFStringGetFWAddress( word, addr ) ;
	
	UInt32			size 	= CFStringGetUInt32(PopWordThrow(mCommandArray, FWUtilException(CFSTR("Could not parse size")))) ;
	Ptr				buf 	= new char[size] ;

	if (!buf)
		throw FWUtilException_OutOfMemory ;
	
	if (word = PopWord(mCommandArray))
	{
		if (kCFCompareEqualTo == CFStringCompare(word, CFSTR("rand"), kCFCompareCaseInsensitive))
			FillBlockRandom(buf, size) ;
		else if (kCFCompareEqualTo == CFStringCompare(word, CFSTR("seq"), kCFCompareCaseInsensitive))
			FillBlockSequential(buf, size) ;
		else
		{
			UInt32 fillValue = CFStringGetUInt32(word) ;
			FillBlockConst(buf, size, fillValue) ;
		}
	}
	else
		FillBlockConst(buf, size, 0) ;
	
	IOReturn 		result		 = kIOReturnSuccess ;
	UInt32			generation ;		
	result = interface->GetBusGeneration( generation ) ;
		
	if (option_Async)
	{
		FWLib::WriteCommand		writeCommand( *interface, addr, (void*) buf, size, & ReadWriteCallback, option_Abs, kFWDontFailOnReset, generation ) ;
		result = writeCommand.Submit() ;
		
		cout << "Async: Command submitted with status " << result << endl ;
		
		if (kCFRunLoopRunTimedOut == CFRunLoopRunInMode(kCFRunLoopDefaultMode, (CFTimeInterval) 10, true))
		{
			throw FWUtilException(CFSTR("Async: Still no callback after 10 seconds!")) ;
		}
	}
	else
	{
		result = interface->Write( addr, buf, size, option_Abs, kFWDontFailOnReset, generation ) ;
	}
		
	delete[] buf ;
	
	return result ;
}

CFStringRef
FWUtilCommand_bwrite::GetHelpText(
	FWUtilHelpType	inHelpType)
{
	CFStringRef	result = 0 ;
	
	switch(inHelpType)
	{	
		case kFWUtilHelp_Short:
			result = CFSTR("FireWire block write") ;
			break ;
		case kFWUtilHelp_Full:
			result = CFSTR(""
"BWRITE: FireWire block write\n\n"
"usage: bwrite [-quiet] [-async] [-abs] addr size [data]\n"
"\t\tOptions:"
"\t\t\t-quiet:  Suppress output\n"
"\t\t\t-async:  Execute using an asynchronous command\n"
"\t\t\t-abs:    Use 64-bit absolute addressing\n"
"\n"
"\t\taddr = Address to write"
"\t\t           (Takes the form [xxxx.]xxxx.xxxx.xxxx where"
"\t\t           '.' is optional separator.)\n"
"\t\tsize = Size of block write\n"
"\t\tdata = Data to send; defaults to all zeros."
"\t\t       Can optionally be one of the following:\n"
"\t\t\trand  : block is filled with random data\n"
"\t\t\tseq   : block is filled with sequentially numbered quadlets\n"
"\t\t\tx     : hex value - block is filled with a constant value\n") ;
			break ;
	}
	
	CFRetain(result) ;
	
	return result ;
}

// ============================================================
// history
// FWUtilCommand_history
// ============================================================
IOReturn
FWUtilCommand_history::Execute()
{
	return kIOReturnUnsupported ;
}

CFStringRef
FWUtilCommand_history::GetHelpText(
	FWUtilHelpType	inHelpType)
{
	CFStringRef	result = CFSTR("Coming soon...") ;
	
	CFRetain(result) ;
	return result ;
}

// ============================================================
// info
// FWUtilCommand_info
// ============================================================
IOReturn
FWUtilCommand_info::Execute()
{
	UInt32				argCount	= CFArrayGetCount(mCommandArray) ;
	FWUtilIndenter&		i			= gGlobals.GetIndenter() ;
	
	Device*			interface = gGlobals.GetDeviceInterface() ;
	if (!interface)
		throw FWUtilException_NoInterface ;
	
	if (argCount > 1)
		throw FWUtilException_TooManyArgs ;
	
	UInt32		generation ;
	UInt16		nodeID ;
	IOReturn	error ;

	error = interface->GetBusGeneration( generation ) ;
	if (!error)
	{
		interface->GetRemoteNodeID( generation, nodeID ) ;
	
		cout << i++ << "Information:\n" ;
		cout << i << "bus generation: 0x" << generation << endl ;
		cout << i << "IOFireWireLib opened to node 0x" << nodeID << ", service 0x" << interface->Service() << endl ;

		error = interface->GetLocalNodeIDWithGeneration( generation, nodeID ) ;
		if (!error)
			cout << i << "local node ID is 0x" << nodeID << endl ;

		i-- ;	// unindent
	}
	
	return error ;
}

CFStringRef
FWUtilCommand_info::GetHelpText(
	FWUtilHelpType	inHelpType)
{
	CFStringRef	result = 0 ;
	
	switch(inHelpType)
	{
		case kFWUtilHelp_Short:
			result = CFSTR("Display some stats") ;
			break ;
		case kFWUtilHelp_Full:
			result	= CFSTR("Display some stats\n\nusage: info") ;
			break ;
	}
	
	CFRetain(result) ;
	
	return result ;
}

// ============================================================
// qread
// FWUtilCommand_qread
// ============================================================
IOReturn
FWUtilCommand_qread::Execute()
{
	FWUtilIndenter&	i				= gGlobals.GetIndenter() ;
	IOReturn		status			= kIOReturnSuccess ;
	UInt32			value			= 0x7FFFDEAD ;
	CFStringRef		word ;

	// option flags
	bool							option_Quiet	= false ;
	bool							option_Async	= false ;
	bool							option_Abs		= false ;

	int argCount 	= CFArrayGetCount(mCommandArray) ;
	if (argCount < 2)
		throw FWUtilException_TooFewArgs ;

	word = PopWord(mCommandArray) ;	// throw away first word which is this command's name
	word = PopWord(mCommandArray) ;
	
	while (IsOptionWord(word))
	{
		if (IsOptionWord(word, CFSTR("quiet")))
			option_Quiet = true ;
		else if (IsOptionWord(word, CFSTR("async")))
			option_Async = true ;
		else if (IsOptionWord(word, CFSTR("abs")))
			option_Abs = true ;
		else
			throw FWUtilException_UnknownOption ;
		
		word = PopWord(mCommandArray) ;
	}
	
	argCount 	= CFArrayGetCount(mCommandArray)+1 ;
	if (argCount > 1)
		throw FWUtilException_TooManyArgs ;
	if (argCount < 1)
		throw FWUtilException_TooFewArgs ;
		

	Device*	interface = gGlobals.GetDeviceInterface() ;
	if (!interface)
		throw FWUtilException_NoInterface ;
	
	FWAddress	addr ;
	if ( kIOReturnSuccess == status )
	{
		
		CFStringGetFWAddress( word, addr ) ;

		if (option_Async)
		{
			UInt32	generation ;
			interface->GetBusGeneration( generation ) ;

			FWLib::ReadCommand 	readCommand( *interface, addr, & value, 4, & ReadWriteCallback, option_Abs, kFWDontFailOnReset, generation ) ;

			// this should have a c++ wrapper too... probably in a future version

			status = readCommand.Submit() ;
			cout << "Async: Command submitted with status " << status << endl ;

			if (kCFRunLoopRunTimedOut == CFRunLoopRunInMode(kCFRunLoopDefaultMode, (CFTimeInterval) 10, true))
			{
				throw FWUtilException(CFSTR("Async: Still no callback after 10 seconds!")) ;
			}
		}
		else
			status = interface->ReadQuadlet( addr, &value, option_Abs, kFWDontFailOnReset, 0 ) ;
	}

	if (!option_Quiet)
	{
		cout << i++ << "QUADLET READ\n" ;
		cout << i	<< "address: " << addr << endl ;
		cout << i	<< "data: 0x" << (UInt32) value << endl ;
	
		i-- ;
	}

	return status ;
}

CFStringRef
FWUtilCommand_qread::GetHelpText(
	FWUtilHelpType	inHelpType)
{
	CFStringRef		result = 0 ;

	switch(inHelpType)
	{
		case kFWUtilHelp_Short:
			result = CFSTR("FireWire quadlet read") ;
			break ;
		case kFWUtilHelp_Full:
			result = CFSTR(
"FireWire quadlet read\n"
"\n"
"usage: qread [-quiet] [-async] addr\n"
"\t\tOptions:\n"
"\t\t\t-quiet:  Suppress output\n"
"\t\t\t-async:  Execute using an asynchronous command\n"
"\t\t<addr> = address to read, of the form [xxxx.]xxxx.xxxx.xxxx (period '.' is optional separator)") ;
			break ;
	}
	
	CFRetain(result) ;
	
	return result ;
}

// ============================================================
// qwrite
// FWUtilCommand_qwrite
// ============================================================
IOReturn
FWUtilCommand_qwrite::Execute()
{
	CFStringRef		word ;

	// option bits
	bool							option_Async = false ;
	bool							option_Abs		= false ;
	
	Device*	interface = gGlobals.GetDeviceInterface() ;
	if (!interface)
		throw FWUtilException_NoInterface ;

	int				argCount 	= CFArrayGetCount(mCommandArray) ;
	if (argCount < 2)
		throw FWUtilException_TooFewArgs ;
	
	word = PopWord(mCommandArray) ;	// throw away first word which is this command's name
	word = PopWord(mCommandArray) ;

	while (IsOptionWord(word))
	{
		if (IsOptionWord(word, CFSTR("async")))
			option_Async = true ;
		else
			throw FWUtilException_UnknownOption ;
		
		word = PopWord(mCommandArray) ;
	}

	argCount = CFArrayGetCount(mCommandArray) ;
	if (argCount > 2)
		throw FWUtilException_TooManyArgs ;
	
	FWAddress	addr ;
	CFStringGetFWAddress(word, addr) ;

	word = PopWordThrow(mCommandArray, FWUtilException(CFSTR("Couldn't read value parameter")) ) ;
	UInt32			value		= CFStringGetUInt32(word) ;
	
	IOReturn result = kIOReturnSuccess ;
	UInt32	generation ;
	interface->GetBusGeneration( generation ) ;

	if (option_Async)
	{
		FWLib::WriteCommand		writeCommand( *interface, addr, & value, 4, & ReadWriteCallback, option_Abs, kFWDontFailOnReset, generation ) ;
		
		writeCommand.Submit() ;
		
		cout << "Async: Command " << & writeCommand << " submitted with status " << result << endl ;
		
		if (kCFRunLoopRunTimedOut == CFRunLoopRunInMode(kCFRunLoopDefaultMode, (CFTimeInterval) 10, true))
		{
			throw FWUtilException(CFSTR("Async: Still no callback after 10 seconds!")) ;
		}
	}
	else
		result = interface->WriteQuadlet( addr, value, option_Abs, kFWDontFailOnReset, generation ) ;
		
	return result ;
}

CFStringRef
FWUtilCommand_qwrite::GetHelpText(
	FWUtilHelpType	inHelpType)
{
	CFStringRef		result = 0 ;
	
	switch(inHelpType)
	{
		case kFWUtilHelp_Short:
			result = CFSTR("FireWire quadlet write") ;
			break ;
		case kFWUtilHelp_Full:
			result = CFSTR(
"FireWire quadlet write\n"
"\n"
"usage: qwrite [-quiet] [-async] addr value\n"
"\t\tOptions:\n"
"\t\t\t-quiet:  Suppress output\n"
"\t\t\t-async:  Execute using an asynchronous command\n"
"\t\t<addr> = address to write, of the form [xxxx.]xxxx.xxxx.xxxx (period '.' is optional separator)\n"
"\t\t<value> = value to write") ;
			break ;
	}

	CFRetain(result) ;
	
	return result ;
}

// ============================================================
// attach
// FWUtilCommand_attach
// ============================================================

IOReturn
FWUtilCommand_attach::Execute()
{
	int				argCount 	= CFArrayGetCount(mCommandArray) ;
	FWUtilIndenter&	i	 		= gGlobals.GetIndenter() ;
	
	//
	// check argument count
	//
	if (argCount > 2)
		throw FWUtilException_TooManyArgs ;
	if (argCount < 2)
		throw FWUtilException_TooFewArgs ;
	
	io_object_t		service	= (io_object_t)::CFStringGetUInt32( reinterpret_cast<CFStringRef>( 
								::CFArrayGetValueAtIndex( mCommandArray, 1 ) ) ) ;

	if ( gGlobals.GetDeviceInterface() )
	{
		cout << i << "Removing current user client.\n" ;
		gGlobals.ReleaseInterface() ;
	}
	
	cout << i << "Connecting to service " << service << ".\n" ;
	IOReturn err = gGlobals.NewInterfaceWithService( service ) ;	
        
	if (kIOReturnSuccess == err)
		err = gGlobals.GetDeviceInterface()->Open() ;
	if (kIOReturnSuccess == err)
		cout << i << "Success!\n" ;
	else
		cout << i << "Couldn't create new user client! Error was 0x" << err << ".\n" ;
	
	return err ;
}

CFStringRef
FWUtilCommand_attach::GetHelpText(
	FWUtilHelpType	inHelpType)
{
	CFStringRef	result  = 0 ;
	
	switch(inHelpType)
	{
		case kFWUtilHelp_Short:
			result = CFSTR("Connect to a service") ;
			break ;
		case kFWUtilHelp_Full:
			result = CFSTR(
"Closes current user client connection (if any)\n"
"and opens a new connection on the specified service.\n"
"Use 'services' to get a list of services.\n"
"\n"
"usage: attach service\n"
"\t\t<service> = number of service to which to attach (see 'services' command)") ;
			break ;
	}
	
	CFRetain(result) ;
	
	return result ;
}

// ============================================================
// services
// FWUtilCommand_services
// ============================================================

IOReturn
FWUtilCommand_services::Execute()
{
	CFArrayRef			services	= gGlobals.CopyDevices() ;
	FWUtilIndenter&		i			= gGlobals.GetIndenter() ;
	
	cout << i++ << "Available services:\n";
	
	if ( ::CFArrayGetCount( services ) == 0 )
		cout << i << "<no services found>\n" ;
	else
	{
		for( CFIndex index = 0, count = CFArrayGetCount( services ); index < count; ++index)
		{
			io_registry_entry_t	regEntry = (io_registry_entry_t) CFArrayGetValueAtIndex(services, index) ;
			io_name_t			name ;
			IORegistryEntryGetName(regEntry, name) ;

			UInt16				nodeID = 0 ;
			CFNumberRef			cfNodeID = (CFNumberRef) IORegistryEntryCreateCFProperty(regEntry, CFSTR("FireWire Node ID"), kCFAllocatorDefault, 0) ;
			if (cfNodeID)
			{
				CFNumberGetValue(cfNodeID, kCFNumberSInt16Type, & nodeID) ;
				CFRelease(cfNodeID) ;
			}
			
			if ( ! gGlobals.GetDeviceInterface() )
			{
				cout << "    " ;
			}
			else
			{
				cout << ( regEntry == gGlobals.GetDeviceInterface()->Service() ? " *  " : "    ") ;
			}
				
			cout << "0x" << regEntry << ":  <" << name << "," << "node=" ;
			if (nodeID)
			{
				cout << nodeID ;
			}
			else
			{
				cout << "?" ;
			}
			
			cout << ">\n" ;
		}
	}

	if ( services )
	{
		::CFRelease( services ) ;
	}
		
	i-- ;

	return kIOReturnSuccess ;
}

CFStringRef
FWUtilCommand_services::GetHelpText( FWUtilHelpType type )
{
	CFStringRef		result = 0 ;
	
	switch ( type )
	{
		case kFWUtilHelp_Short:
			result = CFSTR("Lists all IOFireWireDevices that can be found in the registry") ;
			break ;
		case kFWUtilHelp_Full:
			result = CFSTR("Lists all IOFireWireDevices that can be found in the registry\n\nusage: services") ;
			break ;
	}
	
	::CFRetain( result ) ;
	return result ;
}

// ============================================================
// quit
// FWUtilCommand_quit
// ============================================================
IOReturn
FWUtilCommand_quit::Execute()
{	
	int		argCount = CFArrayGetCount(mCommandArray) ;
	
	if (argCount > 1)
		throw FWUtilException_TooManyArgs ;

	throw FWUtilException_Quit ;
}

CFStringRef
FWUtilCommand_quit::GetHelpText(
	FWUtilHelpType	inHelpType )
{
	CFStringRef	result = 0 ;
	
	switch(inHelpType)
	{
		case kFWUtilHelp_Short:
			result = CFSTR("quit FWUtil") ;
			break ;
		case kFWUtilHelp_Full:
			result = CFSTR("quit FWUtil\n\t\tusage: quit") ;
			break ;
	}
	
	CFRetain(result) ;
	
	return result ;
}

// ============================================================
// lock
// FWUtilCommand_lock
// ============================================================
IOReturn
FWUtilCommand_lock::Execute()
{
	IOReturn		status			= kIOReturnSuccess ;
	bool			option_Quiet	= false ;
	bool			option_Async	= false ;
	bool			option_Abs		= false ;
	CFStringRef		word ;

	int argCount 	= CFArrayGetCount(mCommandArray) ;
	if (argCount < 4)
		throw FWUtilException_TooFewArgs ;

	word = PopWord(mCommandArray) ;	// throw away first word which is this command's name
	word = PopWord(mCommandArray) ;
	
	while (IsOptionWord(word))
	{
		if (IsOptionWord(word, CFSTR("quiet")))
			option_Quiet = true ;
		else if (IsOptionWord(word, CFSTR("async")))
			option_Async = true ;
		else if (IsOptionWord(word, CFSTR("abs")))
			option_Abs = true ;
		else
			throw FWUtilException_UnknownOption ;
		
		word = PopWord(mCommandArray) ;
	}
	
	argCount 	= CFArrayGetCount(mCommandArray)+1 ;
	if (argCount > 3)
		throw FWUtilException_TooManyArgs ;
	if (argCount < 3)
		throw FWUtilException_TooFewArgs ;
		
	Device*	interface = gGlobals.GetDeviceInterface() ;
	if (!interface)
		throw FWUtilException_NoInterface ;
	
	FWAddress	fwAddress ;
	if ( kIOReturnSuccess == status )
	{
		CFStringGetFWAddress( word, fwAddress ) ;
		
		word = PopWordThrow(mCommandArray, FWUtilException(CFSTR("Couldn't read value parameter")) ) ;
		UInt32			cmpVal		= CFStringGetUInt32(word) ;

		word = PopWordThrow(mCommandArray, FWUtilException(CFSTR("Couldn't read value parameter")) ) ;
		UInt32			newVal		= CFStringGetUInt32(word) ;
		UInt32			oldVal ;
		if (option_Async)
		{
			UInt32	generation ;
			interface->GetBusGeneration( generation ) ;

			FWLib::CompareSwapCommand	cmd( *interface, fwAddress, cmpVal, newVal, nil, option_Abs, kFWFailOnReset,generation ) ;
		
			status = cmd.Submit() ;

			cout << "Async: Command submitted with status " << status << endl ;

			if (kCFRunLoopRunTimedOut == CFRunLoopRunInMode(kCFRunLoopDefaultMode, (CFTimeInterval) 10, true))
			{
				throw FWUtilException(CFSTR("Async: Still no callback after 10 seconds!")) ;
			}
			else
				cmd.Locked( oldVal ) ;
		}
		else
			status = interface->CompareSwap64( fwAddress, & cmpVal, & newVal, & oldVal, 4, option_Abs, kFWDontFailOnReset, 0 ) ;

		cout << "old value was " << oldVal << endl ;
	}

	return status ;
}



CFStringRef
FWUtilCommand_lock::GetHelpText(
	FWUtilHelpType	inHelpType )
{
	CFStringRef	result = 0 ;
	
	switch(inHelpType)
	{
		case kFWUtilHelp_Short:
			result = CFSTR("FireWire lock") ;
			break ;
		case kFWUtilHelp_Full:
			result = CFSTR(
"FireWire lock\n"
"\n"
"usage: lock [-quiet] [-async] addr cmpVal newVal\n"
"\t\tOptions:\n"
"\t\t\t-quiet:  Suppress output\n"
"\t\t\t-async:  Execute using an asynchronous command\n"
"\t\t\t-abs:    Use 64-bit absolute addressing\n"
"\t\t<addr> = address to read, of the form [xxxx.]xxxx.xxxx.xxxx (period '.' is optional separator)"
"\t\t<cmpVal> = compare value"
"\t\t<newVal> = new value") ;
			break ;
	}
	
	CFRetain(result) ;
	
	return result ;
}
