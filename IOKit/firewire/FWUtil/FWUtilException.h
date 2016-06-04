/*
	File:		FWUtilException.h

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

#import <iostream>
#import <CoreFoundation/CoreFoundation.h>

#define FWUtilException_NoSuchCommand	FWUtilException(CFSTR("No such command"), false, __LINE__, __FILE__) ;
#define FWUtilException_OutOfMemory		FWUtilException(CFSTR("Out of memory"), false, __LINE__, __FILE__) ;
#define FWUtilException_TooFewArgs 		FWUtilException(CFSTR("Too few arguments"), false, __LINE__, __FILE__) ;
#define FWUtilException_TooManyArgs 	FWUtilException(CFSTR("Too many arguments"), false, __LINE__, __FILE__) ;
#define FWUtilException_NoInterface 	FWUtilException(CFSTR("No interface"), false, __LINE__, __FILE__) ;
#define FWUtilException_UnknownOption	FWUtilException(CFSTR("No such option"), false, __LINE__, __FILE__) ;
#define FWUtilException_Quit			FWUtilException(CFSTR("Quit!"), true, __LINE__, __FILE__, true) ;

/*
typedef enum FWUtilExceptionType_t
{
	kFWUtilException_None = 0,
	kFWUtilException_NoSuchCommand,
	kFWUtilException_TooFewArgs,
	kFWUtilException_TooManyArgs,
	kFWUtilException_OutOfMemory,
	kFWUtilException_NoInterface,
	kFWUtilException_Quit,
	kFWUtilNumExceptionTypes,
} FWUtilExceptionType ;

#ifndef kFWUtilExceptionNames
const CFStringRef	kFWUtilExceptionNames[kFWUtilNumExceptionTypes] =
{
	CFSTR("No exception"),
	CFSTR("Unknown command"),
	CFSTR("Too few arguments"),
	CFSTR("Too many arguments"),
	CFSTR("Out of memory"),
	CFSTR("No user client found"),
	CFSTR("Quit!")
} ;
#endif
*/
using namespace std ;

class FWUtilException
{
	friend ostream & operator<<(ostream & ins, const FWUtilException & e) ;

 public:
	FWUtilException() ;
	FWUtilException(const FWUtilException & e) ;
	FWUtilException(
				CFStringRef 		inString) ;
	FWUtilException(
				CFStringRef			inDesc,
				bool				inMustQuit,
				int					inLine,
				const char *		file,
				bool				inSilentFlag = false) ;				
	
	virtual ~FWUtilException() ;


	const bool	IsQuitException() const ;
	const bool	IsSilentException() const ;
	
 protected:
	CFStringRef				desc ;
//	FWUtilExceptionType		type ;
	int						line ;
	bool					mustQuit ;
	bool					silent ;
	char					file[80] ;
} ;

