/*
	File:		FWUtilException.cpp

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.

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

*/

#import "FWUtil.h"
#import "FWUtilException.h"

FWUtilException::FWUtilException(): desc(0), /*type(FWUtilException_None),*/ line(__LINE__), mustQuit(false)
{
	file[0] = '\0';
}

FWUtilException::FWUtilException(const FWUtilException &e)
{
	strcpy(file, e.file) ;
	if (desc = e.desc)
		CFRetain(desc) ;

	line		= e.line ;
	mustQuit	= e.mustQuit ;
	silent		= e.silent ;
}

FWUtilException::FWUtilException(
	CFStringRef 		inString): desc(inString), /*type(FWUtilException_None),*/ line(__LINE__), mustQuit(false)
{ 
	CFRetain(desc); 
}

FWUtilException::FWUtilException(
	CFStringRef			inDesc,
	bool				inMustQuit,
	int					inLine,
	const char *		inFile,
	bool				inSilentFlag): desc(inDesc), line(inLine),  mustQuit(inMustQuit), silent(inSilentFlag)
{
	if (desc)
		CFRetain(desc); 
	if (strlen(inFile) > 79)
		inFile += (strlen(inFile) - 79) ;

	strcpy(file, inFile);
}
				
FWUtilException::~FWUtilException()
{
	if (desc) CFRelease(desc); 
}

const bool
FWUtilException::IsQuitException() const
{
	return mustQuit;
}

const bool
FWUtilException::IsSilentException() const
{
	return silent ;
}

ostream & operator<<(ostream & ins, const FWUtilException & e)
{
	bool fatal = e.IsQuitException() ;// && (e.type != FWUtilException_Quit) ;
	
	if (fatal)
		ins << "FATAL EXCEPTION: " ;
		
	if (e.desc)
		ins << e.desc ;
	
	if (fatal)
		ins << " in file " << e.file << " at line #" << dec << e.line << endl ;
	
	return ins ;
	
}
