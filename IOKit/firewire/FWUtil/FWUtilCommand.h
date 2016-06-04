/*
	File:		FWUtilCommand.h

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

#import <CoreFoundation/CoreFoundation.h>
#import "FWUtilException.h"
#import "FWUtil.h"

class FWUtilCommand ;
typedef FWUtilCommand* (*CommandCreatorFunction)(CFArrayRef) ;
class FWUtilCommandInfo
{
	public:
		CommandCreatorFunction		creator ;
		CFStringRef					name ;
} ;


// this macro can be used to define FWUtil command class which conforms
// to the required FWUtil command structure

#define FWUtilDeclareCommand(commandClass__, commandName__) 												\
class commandClass__: public FWUtilCommand																	\
	{																										\
	 public:																								\
				commandClass__(CFArrayRef commandArray): 													\
						FWUtilCommand(CFSTR(commandName__), commandArray) {}	;							\
				static FWUtilCommand* CreateWithCFString(CFArrayRef		commandArray)						\
				{																							\
					if (kCFCompareEqualTo == CFStringCompare(												\
													CFSTR(commandName__),									\
													(CFStringRef)CFArrayGetValueAtIndex(commandArray, 0),	\
													kCFCompareCaseInsensitive))								\
						return new commandClass__(commandArray) ;											\
					else																					\
						return nil ;																		\
				}																							\
				virtual IOReturn			Execute() ;														\
				virtual CFStringRef			GetHelpText(FWUtilHelpType) ;									\
	} ;

#define FWUtilCommandArrayEntry(commandClass__, commandName__)\
	{ & commandClass__::CreateWithCFString, CFSTR(commandName__) }

ostream & operator << (ostream & outs, const FWUtilCommand & command) ;

class FWUtilCommand
{
		friend ostream & operator << (ostream &, const FWUtilCommand &) ;

	protected:
	
		CFStringRef				mCommandName ;
		CFMutableArrayRef		mCommandArray ;

	protected:

		FWUtilCommand() ;
		
	public:
	
		FWUtilCommand( CFStringRef name, CFArrayRef commandArray = 0 ) ;
		virtual ~FWUtilCommand() ;
	
		static FWUtilCommand*		CreateWithCFArray( CFArrayRef commandArray ) ;
		virtual IOReturn			Execute() 							{ return kIOReturnUnsupported; } ;
		virtual CFStringRef			GetName()							{ return mCommandName; }
		virtual CFStringRef			GetHelpText(FWUtilHelpType) 		{ CFStringRef result = CFSTR(""); ::CFRetain( result ) ; return result ; }						
} ;

class FWUtilNullCommand: public FWUtilCommand
{
 public:
								FWUtilNullCommand(CFStringRef inName): FWUtilCommand(inName) {}

	virtual IOReturn			Execute() 				{ throw FWUtilException_NoSuchCommand; return FWUtilCommand::Execute(); }
} ;

// ============================================================
//
//	Built in command class definitions
//
// ============================================================

FWUtilDeclareCommand(FWUtilCommand_help, "help") ;

FWUtilDeclareCommand(FWUtilCommand_bread, "bread") ;
FWUtilDeclareCommand(FWUtilCommand_busreset, "busreset") ;
FWUtilDeclareCommand(FWUtilCommand_bwrite, "bwrite") ;
FWUtilDeclareCommand(FWUtilCommand_services, "services") ;
FWUtilDeclareCommand(FWUtilCommand_history, "history") ;
FWUtilDeclareCommand(FWUtilCommand_info, "info") ;
FWUtilDeclareCommand(FWUtilCommand_qread, 	"qread") ;
FWUtilDeclareCommand(FWUtilCommand_qwrite, "qwrite") ;
FWUtilDeclareCommand(FWUtilCommand_attach, "attach") ;
FWUtilDeclareCommand(FWUtilCommand_quit, 	"quit") ;
FWUtilDeclareCommand(FWUtilCommand_repeat, "repeat") ;
FWUtilDeclareCommand(FWUtilCommand_lock, "lock") ;
