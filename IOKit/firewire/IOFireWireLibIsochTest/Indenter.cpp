/*
	File:		IOFireWireLibIsochTest/Indenter.cpp

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
	
	$Log: Indenter.cpp,v $
	Revision 1.3  2005/02/03 01:16:26  firewire
	*** empty log message ***
	
	Revision 1.2  2004/03/23 23:28:27  firewire
	*** empty log message ***
	
	Revision 1.1  2002/11/07 00:39:40  noggin
	move to new repository
	
	Revision 1.2  2002/08/22 21:44:55  noggin
	Got rid of global variables. Renamed Indenter class. Remove libstdc++ from project.
	
*/

#import "Indenter.h"

// ============================================================
// BlockIndenter
// ============================================================

BlockIndenter::BlockIndenter( Indenter& indenter, CFStringRef string )
: mIndenter( indenter ), 
  mString( string )
{
	::CFRetain( string ); 
}

BlockIndenter::~BlockIndenter()
{
	if (mString)
		::CFRelease( mString );
}

BlockIndenter 
BlockIndenter::operator++(int)
{	
	BlockIndenter result(*this) ;	// post operator
	operator++() ;
	return result;
}

BlockIndenter
BlockIndenter::operator++()
{
	mIndenter++; 
	return *this;
}

BlockIndenter
BlockIndenter::operator--(int)
{
	BlockIndenter result(*this) ;	// post operator
	operator++() ;
	return result;
}
BlockIndenter
BlockIndenter::operator--()
{
	mIndenter++;
	return *this ;
}

// ============================================================
// Indenter
// ============================================================

Indenter
Indenter::operator++(int)
{	
	Indenter result(*this) ;	// post operator
	operator++(); 
	return result;
}
											
Indenter
Indenter::operator++()
{	
	if (strlen(mIndentString) < 250)
		strcat(mIndentString, "    ");
	return *this;
}
										
Indenter
Indenter::operator--(int)
{
	Indenter result(*this) ;
	operator--(); 
	return result; 
}
										
Indenter
Indenter::operator--()
{	
	if ( ::strlen( mIndentString ) > 4)
		mIndentString[ ::strlen( mIndentString ) - 4] = 0 ;
	else
		mIndentString[0] = 0 ; 
		
	return *this;
}

ostream & operator<<(ostream & outs, const class Indenter* me)
{
	return (outs << me->mIndentString) ;
}

ostream & operator<<(ostream & outs, const Indenter & me)
{
	return (outs << me.mIndentString) ;
}

ostream & operator<<(ostream & outs, const BlockIndenter & indenter)
{
	CFArrayRef	linesArray = ::CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, indenter.mString, CFSTR("\n")) ;
	
	if ((linesArray) && ( ::CFArrayGetCount(linesArray) > 0 ) )
	{
		outs << indenter.mIndenter << (CFStringRef) ::CFArrayGetValueAtIndex(linesArray, 0) ;
	
		for( CFIndex index=1, count=::CFArrayGetCount( linesArray ); index < count; ++index )
		{
			outs << endl << indenter.mIndenter << (CFStringRef)::CFArrayGetValueAtIndex(linesArray, index) ;
		}
		
		::CFRelease( linesArray ) ;
	}
	
	return outs ;
}

