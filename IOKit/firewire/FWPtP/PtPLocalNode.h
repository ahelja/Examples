/*
	File:		FWPtP/PtPLocalNode.h

	Synopsis: 	Sample code for simple peer-to-peer FireWire protocol application.

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

	$Log: PtPLocalNode.h,v $
	Revision 1.4  2005/02/03 00:35:40  firewire
	*** empty log message ***
	
	Revision 1.3  2004/03/23 23:27:27  firewire
	*** empty log message ***
	
	Revision 1.2  2002/11/09 00:12:21  noggin
	first checkin of install project
	
	Revision 1.1  2002/11/07 00:37:59  noggin
	move to new repository
	
	Revision 1.2  2002/10/15 21:03:12  noggin
	now properly handle unplugged nodes. added skipped packet handler (just in case).
	
	Revision 1.1  2002/10/14 22:23:06  noggin
	ready for SDK14
	
*/

#import <FWLib/FWLib.h>

class Application ;
class PtPLocalNode: public FWLib::Device
{
	public :
	
		class MsgInAddressSpace ;

	 public:

		PtPLocalNode( Application & inApp) ;
		virtual ~PtPLocalNode() ;

		// --- getters/setters
		const MsgInAddressSpace&			GetMsgInAddressSpace() const			{ return *mAddressSpace ; }
		
		// 
		// class MsgInAddressSpace
		//
		class MsgInAddressSpace: public FWLib::PseudoAddressSpace
		{
			public:
			
				MsgInAddressSpace					( Application & app, Device & device ) ;
				virtual ~MsgInAddressSpace			() ;

				virtual	UInt32			HandleWrite	( FWClientCommandID commandID, UInt32 packetLen, void* packet, UInt16 srcNodeID, UInt32 destAddressHi, UInt32 destAddressLo ) ;

			protected:

				Application&		mApp ;
		} ;

		// 
		// class PtPUnitDirectory
		//
		class PtPUnitDirectory: public FWLib::LocalUnitDirectory
		{
			public:
			
				PtPUnitDirectory			( PtPLocalNode & device ) ;
				virtual ~PtPUnitDirectory	() ;
		} ;

	protected:
 
		FWLib::LocalUnitDirectory*			mUnitDirectory ;
		MsgInAddressSpace*					mAddressSpace ;
} ;
