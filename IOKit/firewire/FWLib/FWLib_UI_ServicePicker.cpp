/*
	File:		FWLib_UI_ServicePicker.cpp

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

 *  FWLib_UI_ServicePicker.cpp
 *  FWLib
 *
 *  Created by Niels on Tue Jun 04 2002.
 *  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 */

/*
	$Log: FWLib_UI_ServicePicker.cpp,v $
	Revision 1.4  2005/05/13 00:26:43  niels
	*** empty log message ***
	
	Revision 1.3  2004/02/07 00:02:51  firewire
	*** empty log message ***
	
	Revision 1.1  2002/11/07 00:32:14  noggin
	move to new repository
	
	Revision 1.3  2002/08/21 22:26:03  noggin
	*** empty log message ***
	
	Revision 1.2  2002/06/07 21:25:44  noggin
	*** empty log message ***
	
	Revision 1.1  2002/06/05 19:20:04  noggin
	added to repository
		
*/

#import "FWLib_UI_ServicePicker.h"
#import "FWLib.h"
#import <new>
#import <iostream>	//for debugging

#define CFArrayEntireRange(_array) (::CFRangeMake(0, ::CFArrayGetCount(_array)))

namespace FWLib {
namespace UI {

	const WindowClass ServicePicker::kWindowClass = ::kMovableModalWindowClass ;
	const WindowAttributes ServicePicker::kWindowAttributes 
			= ::kWindowCompositingAttribute + ::kWindowStandardHandlerAttribute ;//+ kWindowMetalAttribute ;
	const Rect ServicePicker::kBounds = { 0, 0, 400, 300 } ;

	ServicePicker::ServicePicker( CFDictionaryRef matching )
	: mChoices(IO::Service::GetMatchingServices( matching ))
//	  ,
//	  mItemNotificationUPP( ::NewDataBrowserItemNotificationUPP( 
//			static_cast<DataBrowserItemNotificationProcPtr>( & S_ItemNotificationCallback ) ) )
	{
		OSStatus error ;
		
		// make window
		{
			error = ::CreateNewWindow( kWindowClass, kWindowAttributes, & kBounds, & mWindow ) ;
			if (error)
				throw std::bad_alloc() ;
			
			error = ::RepositionWindow( mWindow, nil, kWindowAlertPositionOnMainScreen ) ;
			if (error)
				throw std::exception() ;
		}
		
		// make data browser
		{
			HIViewRef	rootView ;
			{
				rootView = ::HIViewGetRoot( mWindow ) ;
			}
			
			HIRect boundsHIRect ;
			{
				error = ::HIViewGetBounds( rootView, &boundsHIRect ) ;
				if (error)
					throw std::exception() ;	
			}
			
			Rect boundsRect ;
			{
				boundsRect.top = (short int)boundsHIRect.origin.y ;//+ 8 ;
				boundsRect.left = (short int)boundsHIRect.origin.x ;//+ 8 ;
				boundsRect.bottom = boundsRect.top + (short int)boundsHIRect.size.height ;//- 40 ;
				boundsRect.right = boundsRect.left + (short int)boundsHIRect.size.width ;//- 16 ;
			}
			
			// init mDataBrowser
			{
				// control
				{
					error = ::CreateDataBrowserControl( mWindow, & boundsRect, kDataBrowserListView, & mDataBrowser ) ;
					if (error)
						throw std::exception() ;
				}
				
				// callbacks
				{
					DataBrowserCallbacks callbacks ;
					{
						callbacks.version = kDataBrowserLatestCallbacks ;
						error = ::InitDataBrowserCallbacks( &callbacks ) ;
						if (error)
							throw std::exception() ;
						callbacks.u.v1.itemNotificationCallback = mItemNotificationUPP ;
					}
	
					error = ::SetDataBrowserCallbacks( mDataBrowser, & callbacks ) ;
					if (error)
						throw std::exception() ;
				}
				
				// items
				{
					const int itemCount = ::CFArrayGetCount( mChoices ) ;
					DataBrowserItemID items[ itemCount ] ;
					{
						::CFArrayGetValues(mChoices, CFArrayEntireRange(mChoices), static_cast<const void**>(items)) ;
					}
					
					error = ::AddDataBrowserItems( mDataBrowser, nil, itemCount, items, kDataBrowserItemNoProperty ) ;
					if (error)
						throw std::exception() ;
				}
			}
				
			error = ::HIViewAddSubview( rootView, mDataBrowser ) ;
			if (error)
				throw std::exception() ;
		}
	}
	
	ServicePicker::~ServicePicker()
	{
		if (mDataBrowser)
			::DisposeControl(mDataBrowser) ;
		if (mWindow)
			::ReleaseWindow( mWindow ) ;
		::DisposeDataBrowserItemNotificationUPP( mItemNotificationUPP ) ;
	}
			
	io_service_t
	ServicePicker::Choose(bool& cancelled)
	{
		::ShowWindow(mWindow) ;
		::SelectWindow(mWindow) ;
		
		OSStatus error ;
//		error = ::RunAppModalLoopForWindow(mWindow) ;
		::RunApplicationEventLoop() ;
		if (error)
			throw std::exception() ;
		
		::HideWindow(mWindow) ;
			
		cancelled = true ;
		return 0 ;
	}

/*	OSStatus
	ServicePicker::S_ItemNotificationCallback(ControlRef browser, DataBrowserItemID item, 
			DataBrowserItemNotification message, DataBrowserItemDataRef itemData)
	{
		if ( kDataBrowserItemAdded == message )
		{
			IO::FireWireNub* nub ;
			{
				nub = static_cast<IO::FireWireNub*>( item ) ;
			}

			OSStatus error = ::SetDataBrowserItemDataText( itemData, CFSTR("Hello")) ;
			if (error)
				std::cerr << "error " << error << " setting data browser item text\n" ;
			
			return error ;
		}
		return noErr ;
	} */

	OSStatus
	ServicePicker::S_ItemDataCallback(ControlRef browser, DataBrowserItemID item, 
			DataBrowserPropertyID property, DataBrowserItemDataRef itemData, 
			Boolean setValue)
	{
			OSStatus error = ::SetDataBrowserItemDataText( itemData, CFSTR("Hello")) ;
			if (error)
				std::cerr << "error " << error << " setting data browser item text\n" ;
	
	}

}}

