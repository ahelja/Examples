/*
	File:		FWLib_UI_ServicePicker.h

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

 *  FWLib_UI_ServicePicker.h
 *  FWLib
 *
 *  Created by Niels on Tue Jun 04 2002.
 *  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 */

/*
	$Log: FWLib_UI_ServicePicker.h,v $
	Revision 1.2  2005/05/13 00:26:43  niels
	*** empty log message ***
	
	Revision 1.1  2004/03/23 23:18:40  niels
	*** empty log message ***
	
	Revision 1.1  2002/11/07 00:32:14  noggin
	move to new repository
	
	Revision 1.2  2002/06/07 21:25:44  noggin
	*** empty log message ***
	
	Revision 1.1  2002/06/05 19:20:04  noggin
	added to repository
		
*/

#import <Carbon/Carbon.h>

namespace FWLib {
namespace UI {

	class ServicePicker
	{
		private:
			static const WindowClass		kWindowClass ;
			static const WindowAttributes	kWindowAttributes ;
			static const Rect				kBounds ;
	
		public:
			ServicePicker( CFDictionaryRef matching ) ;
			virtual ~ServicePicker() ;
			
		public:
			io_service_t		Choose(bool& cancelled) ;

		private:
//			static OSStatus		S_ItemNotificationCallback(ControlRef browser, DataBrowserItemID item, 
//										DataBrowserItemNotification message, DataBrowserItemDataRef itemData) ;
//			static OSStatus		S_ItemDataCallback(ControlRef browser, DataBrowserItemID item, 
//										DataBrowserPropertyID property, DataBrowserItemDataRef itemData, 
//										Boolean setValue) ;
		protected:
			CFArrayRef							mChoices ;
			WindowRef							mWindow ;
			ControlRef							mDataBrowser ;
//			DataBrowserItemNotificationUPP		mItemNotificationUPP ;
//			DataBrowserDataUPP					mItemDataUPP ;
	} ;

}}
