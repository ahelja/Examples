/*
	File:		FWUtil.cpp

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

#import "FWUtil.h"
#import "FWUtilCommand.h"
#import "FWUtilGlobals.h"
#import <FWLib_Device.h>

#import <iomanip>

const char *kFWUtilVersionString = "v1.1" ;

extern FWUtilGlobals gGlobals ;

// ============================================================
// main
// ============================================================

int main(int argc, char **argv)
{
	FWUtilCommand*			currentCommand	= NULL ;
	bool					quitFlag 		= false ;
	FWUtilIndenter			i				= gGlobals.GetIndenter() ;
	IOReturn				status			= kIOReturnSuccess ;
	
    // ignore the INT signal (user pressed ^C)
	const struct sigaction	ignoreSignal	= { {SIG_IGN}, 0, 0} ;
	sigaction(SIGINT, & ignoreSignal, nil) ;
	
	// make sure we print hex values:
	cout << hex ;
	cerr << hex ;
	
	// intro
	cout << i << "***\n" ;
	cout << i << "*** FWUtil "<< kFWUtilVersionString << " (c) Apple Computer, Inc. 2000-2002\n" ;
	cout << i << "***\n" ;
	cout << i << "\n" ;
	cout << i << "Welcome to FWUtil. Type 'help' for help or 'quit' to exit.\n" ;
	cout << i << "\n" ;

	// process command line args, if any
	try {
		if (argc > 1)
			status = ProcessInputArgs(argc, argv) ;
		else
		{
			io_service_t service = 0 ;
			if ( gGlobals.GetDeviceInterface() )
				service = gGlobals.GetDeviceInterface()->Service() ;
		
			cout << endl ;
			if (service)
				cout << i << "User client opened on service 0x" << service << endl ;
			else
				cout << i << "User client not open on any service\n" ;

			cout << i << "To see a list of available services, type 'services'\n" ;
			cout << i << "To connect to a service type 'attach'\n\n" ;
		}
	}
	catch (FWUtilException e) 
	{
		quitFlag = e.IsQuitException() ;
		cerr << e << endl ;
	}
	catch (...) 
	{
		cerr << "unhandled exception!" << endl ;
	}

	// done processing cmd line args.. run interpreter
	while ( !quitFlag )
	{
		CFArrayRef commandArray = ReadCommandWithPrompt( kFWUtilPromptString ) ;
		
		i++ ;
		
		try 
		{
			currentCommand	= FWUtilCommand::CreateWithCFArray( commandArray ) ;			
			::CFRelease( commandArray ) ;
			
			cout << i << endl ;

			if (currentCommand)
			{
				status = currentCommand->Execute() ;
				cout << i << endl ;
			}

			if (kIOReturnSuccess != status)
				cout << i << "Command result was 0x" << status << endl << endl ;
		}
		catch (FWUtilException e)
		{
			if (!e.IsSilentException())
				cout << "\"" << currentCommand->GetName() << "\": " << e << endl << endl ;
			
			quitFlag = e.IsQuitException() ;
		}
		
		i-- ;
	}
	
	cout << "Quitting...  Thank you for using FWUtil!\n\n" ;
}

ostream & operator<<(ostream & outs, const FWAddress & addr)
{
	return (outs << addr.nodeID << ":" << addr.addressHi << ":" << addr.addressLo) ;
}


ostream & operator<<(ostream & outs, const CFStringRef str)
{
	const char*		cStr 			= CFStringGetCStringPtr(str, kCFStringEncodingASCII) ;
	
	if (!cStr)
	{
		char slowCStr[256] ;
		if (!CFStringGetCString(str, slowCStr, 256, kCFStringEncodingASCII))
			throw std::exception() ;
		else
			outs << slowCStr ;
	}
	else
		outs << cStr  ;	// zzz need to find out how to print these characters w/o adding endl
		
	outs.sync_with_stdio() ;
	return outs;
}

CFArrayRef
ReadCommandWithPrompt( CFStringRef prompt )
{
	CFMutableArrayRef		result = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, & kCFTypeArrayCallBacks ) ;
	while ( ::CFArrayGetCount( result ) == 0 )
	{
		char			string[256] = "" ;

		cout << prompt ;

		while ( cin.peek() != '\n' )
		{
			cin >> string ;
			::CFArrayAppendValue( result, ::CFStringCreateWithCString( kCFAllocatorDefault, string, kCFStringEncodingASCII ) ) ;
		}
		
		cin.get( string[0] ) ;
	}

	return result;
}

IOReturn ProcessInputArgs( int argc, char **argv )
{
	return kIOReturnSuccess ;
}

void CFStringGetFWAddress( const CFStringRef string, FWAddress & outAddr )
{
	if ( kCFCompareEqualTo == ::CFStringCompare( string, CFSTR("."), kCFCompareCaseInsensitive | kCFCompareNonliteral ) )
		outAddr = gGlobals.GetLastAddress() ;

	CFArrayRef 				tempArray = ::CFStringCreateArrayBySeparatingStrings( kCFAllocatorDefault, string, CFSTR(".") ) ;
	if (!tempArray)
	{
		throw FWUtilException_OutOfMemory ;
	}
	
	CFStringRef 			addressCFString	= ::CFStringCreateByCombiningStrings( kCFAllocatorDefault, tempArray, CFSTR("") ) ;

	::CFRelease( tempArray ) ;
	
	if ( kCFCompareEqualTo == ::CFStringCompare( addressCFString, CFSTR("random"),  kCFCompareCaseInsensitive | kCFCompareNonliteral ) )
	{
		outAddr = FWAddress(random(), random(), 0) ;
		return ;
	}
		
	// convert address to a number
	char tempString[42] ;
	if ( ! ::CFStringGetCString( addressCFString, tempString, 42, kCFStringEncodingASCII ) )
		throw FWUtilException( CFSTR( "Could not parse address" ) ) ;

	UInt64	address = strtouq( tempString, nil, 16 ) ;

	outAddr = FWAddress( (address >> 32) & 0xFFFF, address & 0xFFFFFFFF, (address >> 48) & 0xFFFF ) ;
	gGlobals.SetLastAddress( outAddr ) ;
}

UInt32 CFStringGetUInt32( const CFStringRef string )
{
	char tempString[42] ;
	if (!CFStringGetCString( string, tempString, 42, kCFStringEncodingASCII ) )
		throw FWUtilException( CFSTR("Could not parse number") ) ;
	
	return strtoul( tempString, nil, 16 ) ;
}

CFStringRef PopWordThrow(
	CFMutableArrayRef	inArray,
	const FWUtilException & e)
{
	CFStringRef	result = 0 ;
	if (!CFArrayGetCount(inArray))
		throw e ;

	result = (CFStringRef)CFArrayGetValueAtIndex(inArray, 0) ;
	CFArrayRemoveValueAtIndex(inArray, 0) ;
	
	return result ;
}

CFStringRef PopWord(CFMutableArrayRef	inArray)
{
	CFStringRef	result = 0 ;
	if (CFArrayGetCount(inArray) > 0)
	{
		result = (CFStringRef)CFArrayGetValueAtIndex(inArray, 0) ;
		CFArrayRemoveValueAtIndex(inArray, 0) ;
	}
	
	return result ;
}

bool
IsOptionWord(CFStringRef word)
{
	return CFStringHasPrefix(word, CFSTR("-")) ;
}

bool
IsOptionWord(CFStringRef word, CFStringRef optionName)
{
	CFMutableStringRef mutableWord = CFStringCreateMutableCopy(kCFAllocatorDefault, CFStringGetLength(word), word) ;
	CFStringDelete(mutableWord, CFRangeMake(0, 1)) ;

	bool result = (kCFCompareEqualTo == CFStringCompare(mutableWord, optionName, kCFCompareCaseInsensitive)) ;
	CFRelease(mutableWord) ;
	
	return result ;
}

void FillBlockRandom(Ptr buf, UInt32 size)
{
	for(UInt32 index=0; index < size>>2; index++)
		((UInt32*)buf)[index] = random() ;
}

void			FillBlockSequential(Ptr buf, UInt32 size)
{
	for(UInt32 index=0; index<size; index++)
		((UInt8*)buf)[index] = index ;
}

void			FillBlockConst(Ptr buf, UInt32 size, UInt32 valu)
{
	for(UInt32 index=0; index<size>>2; index++)
		((UInt32*)buf)[index] = valu ;
}

void PrettyPrintBuffer(
	Byte*				buf,
	UInt32				size,
	FWUtilIndenter&		i )
{
	// pretty print block contents
	cout << i ;		// indent!
//	cout << hex << setfill('0') << setw(10) ;
	for( unsigned index = 0; index < size; ++index )
	{
		if ((index & 0x3) == 0)
		{	
			cout << " " ;
			if ((index & 0x7) == 0)
			{
				cout << "   " ;
				if ((index & 0xF) == 0)
					cout << "\n" << i ;	// indent after newline!
			}
		}
		cout << hex << setfill('0') << setw(2) << (unsigned)buf[index] ;
	}
	
	// done!
	cout << "\n" ;		
}

