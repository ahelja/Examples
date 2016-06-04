/*
	File:		FWLib_ConfigDirectory.h

	Synopsis: 	C++ interface for FireWire device config ROMs. Corresponds to config
				ROM interfaces in IOFireWireLib.

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

	$Log: FWLib_ConfigDirectory.h,v $
	Revision 1.6  2005/05/13 00:26:43  niels
	*** empty log message ***
	
	Revision 1.5  2003/05/27 18:12:45  firewire
	SDK16
	
	Revision 1.4  2003/05/27 17:34:04  firewire
	*** empty log message ***
	
	Revision 1.3  2003/03/13 02:34:18  firewire
	*** empty log message ***
	
	Revision 1.1  2002/11/07 00:32:12  noggin
	move to new repository
	
	Revision 1.2  2002/08/21 22:26:02  noggin
	*** empty log message ***
	
	Revision 1.1  2002/06/05 19:20:48  noggin
	names changed. namespace names changes. nested IO in FWLib
	
	Revision 1.2  2002/05/31 21:21:52  noggin
	added version logging to all files
	
	Revision 1.1  2002/05/31 21:12:37  noggin
	first check-in
	
*/

#import <IOKit/firewire/IOFireWireLib.h>
#import <exception>

#define kConfigDirIID			(CFUUIDGetUUIDBytes( kIOFireWireConfigDirectoryInterfaceID ))

namespace FWLib {
	class Device ;
	class ConfigDirectory
	{
		typedef ::IOFireWireLibConfigDirectoryRef DirRef ;
		
		public:
			// ctor/dtor
			ConfigDirectory( Device& inDevice ) ;
			ConfigDirectory( const ConfigDirectory& otherDirectory, int key ) ;
			virtual ~ConfigDirectory() ;
		
			// methods
			IOReturn			GetKeyValue( int key, CFDataRef& outData ) const		{ return (**mConfigDirectory).GetKeyValue_Data( mConfigDirectory, key, & outData, 0 ) ; }
			ConfigDirectory*	GetKeyValue( int key ) const							{ return new ConfigDirectory( *this, key ) ; }
			IOReturn			GetKeyOffset( int key, FWAddress& outAddress ) const	{ return (**mConfigDirectory).GetKeyOffset_FWAddress( mConfigDirectory, key, & outAddress, 0 ) ; }

			IOReturn			GetIndexValue( int index, CFStringRef& string ) const	{ return (**mConfigDirectory).GetIndexValue_String( mConfigDirectory, index, & string ) ; }
			IOReturn			GetIndexValue( int index, CFDataRef& data ) const		{ return (**mConfigDirectory).GetIndexValue_Data( mConfigDirectory, index, & data ) ; }

		protected:
			static inline DirRef	AllocHelper( DirRef dir, int key ) ;

		protected:
			DirRef mConfigDirectory ;
	} ;

	inline ConfigDirectory::DirRef
	ConfigDirectory::AllocHelper( DirRef dir, int key )
	{
		DirRef	result ;
		IOReturn error = (**dir).GetKeyValue_ConfigDirectory( dir, key, & result, kConfigDirIID, 0 ) ;
		if (error)
			throw std::exception() ;
	
		return result ;
	}

}
