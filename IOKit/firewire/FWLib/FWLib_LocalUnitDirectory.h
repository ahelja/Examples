/*
	File:		FWLib_AddressSpace.h

	Synopsis: 	C++ class representing a unit directory in the Mac. Corresponds to
				IOFireWireLocalUnitDirectoryInterface.

	Copyright: 	© Copyright 2001-2003 Apple Computer, Inc. All rights reserved.

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

	$Log: FWLib_LocalUnitDirectory.h,v $
	Revision 1.3  2004/02/06 23:29:12  firewire
	*** empty log message ***
	
	Revision 1.2  2003/05/27 18:12:46  firewire
	SDK16
	
	Revision 1.1  2002/11/07 00:32:13  noggin
	move to new repository
	
	Revision 1.2  2002/10/14 22:24:55  noggin
	SDK14
	
	Revision 1.1  2002/06/05 19:20:47  noggin
	names changed. namespace names changes. nested IO in FWLib
	
	Revision 1.2  2002/05/31 21:21:51  noggin
	added version logging to all files
	
	Revision 1.1  2002/05/31 21:12:37  noggin
	first check-in
	
*/

#import <IOKit/firewire/IOFireWireLib.h>

namespace FWLib {

	class Device ;
	class LocalUnitDirectory
	{
		typedef ::IOFireWireLibLocalUnitDirectoryRef DirRef ;
	
		public:
									LocalUnitDirectory(
											Device&				inDevice ) ;
			virtual					~LocalUnitDirectory() ;

		public:
			IOReturn				AddEntry( UInt32 inKey, void* inData, IOByteCount inDataSize ) const
											{ return (**mUnitDirectory).AddEntry_Ptr( mUnitDirectory, inKey, inData, inDataSize, 0 ) ; }
			IOReturn				AddEntry( UInt32 inKey, UInt32 inValue ) const
											{ return (**mUnitDirectory).AddEntry_UInt32( mUnitDirectory, inKey, inValue, 0 ) ; }									
			IOReturn				Publish()
											{ return (**mUnitDirectory).Publish( mUnitDirectory ) ; }
			IOReturn				Unpublish()
											{ return (**mUnitDirectory).Unpublish( mUnitDirectory ) ; }

		protected:
			DirRef		mUnitDirectory ;
	} ;
	
}
