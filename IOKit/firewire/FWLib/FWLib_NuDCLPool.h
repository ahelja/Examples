/*
	File:		FWLib_NuDCLPool.h

	Synopsis: 	C++ Interface for IOFireWireLibNuDCLPool

	Copyright: 	© Copyright 2001-2003 Apple Computer, Inc. All rights reserved.

	Written by: NWG
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Appleâ€™s
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

	$Log: FWLib_NuDCLPool.h,v $
	Revision 1.2  2004/02/06 23:29:13  firewire
	*** empty log message ***
	
	Revision 1.1  2003/03/13 02:34:18  firewire
	*** empty log message ***
	
*/

#import <IOKit/firewire/IOFireWireLibIsoch.h>

namespace FWLib {

	class Device ;
	
	class NuDCLPool
	{
		private:
		
			IOFireWireNuDCLPoolInterface**		mPool ;			
		
		public:
		
			NuDCLPool( Device& device, UInt32 capacity = 0 ) ;
			virtual ~NuDCLPool() ;

		public:
		
			// allocating DCLs:
			inline NuDCLRef				AllocateSendPacket( CFMutableSetRef updateSet, UInt32 numRanges, IOVirtualRange ranges[] )		{ return (**mPool).AllocateSendPacket( mPool, updateSet, numRanges, ranges ) ; }
			
			// modifying DCLs:
			inline IOReturn				AppendDCLRanges( NuDCLRef dcl, UInt32 numRanges, IOVirtualRange ranges[] )						{ return (**mPool).AppendDCLRanges( dcl, numRanges, ranges ) ; }
			inline IOReturn				SetDCLUpdateList( NuDCLRef dcl, CFSetRef dclList )												{ return (**mPool).SetDCLUpdateList( dcl, dclList ) ; }
			inline IOReturn				SetDCLBranch( NuDCLRef dcl, NuDCLRef branchDCL )												{ return (**mPool).SetDCLBranch( dcl, branchDCL ) ; }
			inline NuDCLRef				GetDCLBranch( NuDCLRef dcl )																	{ return (**mPool).GetDCLBranch( dcl ) ; }
			
			
			// other:
			inline void					PrintProgram()								{ (**mPool).PrintProgram( mPool ) ; }
			inline DCLCommand*			GetProgram()								{ return (**mPool).GetProgram( mPool ) ; }
			inline IOFireWireNuDCLPoolInterface** Interface()						{ return mPool ; }
	} ;
	

} // namespace
