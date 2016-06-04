/*
	File:		IOFireWireLibIsochTest/Indenter.h

	Synopsis:	This is an introduction to the isochronous services of the FireWire user client. 
				This source code is a good place to start if you are interested in sending or 
				receiving data isochronously. You could easily modify this sample code to receive 
				isochronous data instead of send it.
				
				We create a local port representing our talking Macintosh with a talking DCL program.
				We create a remote port that represents our remote device. We don't actually support a remote
				device, but you can see how the remote callbacks should be used.
				
				This sample code opens a user client on the local node and starts talking on an isochronous 
				channel.
				
				WARNING: Avoid running this sample on a laptop with no devices connected - the FireWire chip will
				have been put to sleep and bad things will happen. However, normally one wouldn't run an 
				isochronous channel with no devices on the bus, so this shouldn't be a problem.

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
	
	$Log: Indenter.h,v $
	Revision 1.1  2002/11/07 00:39:40  noggin
	move to new repository
	
	Revision 1.2  2002/08/22 21:44:54  noggin
	Got rid of global variables. Renamed Indenter class. Remove libstdc++ from project.
	
*/

#import <iostream>
#import <CoreFoundation/CoreFoundation.h>

using namespace std ;

class Indenter ;

class BlockIndenter
{
	friend ostream & operator<<(ostream & outs, const BlockIndenter & indenter) ;

	protected:

		Indenter& 			mIndenter ;
		CFStringRef			mString ;

	public:

		//
		// constructors/destructors
		//
		BlockIndenter( Indenter& indenter, CFStringRef string ) ;
		~BlockIndenter() ;
		
		//
		// operators
		//
		BlockIndenter		operator++(int) ;
		BlockIndenter 		operator++() ;
		BlockIndenter 		operator--(int) ;
		BlockIndenter 		operator--() ;
											
} ;

class Indenter
{
	friend ostream & operator<<(ostream &, const class Indenter *) ;
	friend ostream & operator<<(ostream &, const Indenter &) ;

	protected:

		char		mIndentString[256] ;
	
	public:
	
		//
		// constructors/destructors
		//
		inline Indenter() ;
		inline Indenter( const Indenter & indenter ) ;
	
		//
		// utils
		//
		inline BlockIndenter	BlockIndent( CFStringRef string ) ;
										
		// --- operators ---------------------------
		Indenter 			operator++(int);
		Indenter 			operator++();
		Indenter 			operator--(int);
		Indenter 			operator--();
} ;


ostream & operator<<(ostream & outs, const BlockIndenter & indenter) ;
ostream & operator<<(ostream & outs, const class Indenter * me) ;
ostream & operator<<(ostream & outs, const Indenter & me) ;

typedef BlockIndenter (*BlockIndentFunction)(CFStringRef) ;

Indenter::Indenter()
{
	strcpy( mIndentString, ""); 
}

Indenter::Indenter( const Indenter & indenter )
{
	strcpy( mIndentString, indenter.mIndentString ) ; 
}

BlockIndenter
Indenter::BlockIndent( CFStringRef string )
{
	return BlockIndenter( *this, string ) ;
}
