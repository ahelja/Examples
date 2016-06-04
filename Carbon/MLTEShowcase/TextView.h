/*
 *  HITextView.h
 *  MLTEShowcase
 *
 *  Created on Mon Apr 26 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 *	This file contains code to manage the HITextView in MLTEShowcase
 
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

#ifndef _SHOWCASE_TEXTVIEW_
#define _SHOWCASE_TEXTVIEW_

#include <Carbon/Carbon.h>

const FourCharCode kHITextViewSignature='mLTE';
const ControlID kTextViewControlID = { kHITextViewSignature, 1 };
const FourCharCode kHIScrollViewSignature='sCRL';
const ControlID kTextScrollViewControlID = { kHIScrollViewSignature, 1 };

class CMLTEViewData
{
	public:
	CMLTEViewData();
	~CMLTEViewData();
	void SetURL( CFURLRef url );
	CFURLRef fFileURL;
};

// Demo functions
OSStatus
TextViewDefaultSettings( HIViewRef textView );

OSStatus
TextViewDemoActionGroup( HIViewRef textView );

OSStatus
TextViewDemoSpellingSupport( HIViewRef textView );

OSStatus
TextViewDemoFontPanelSupport( HIViewRef textView );

// invoke navigation services code to pick a file to read
OSStatus
TextViewDemoReadFromCFURL( HIViewRef textView );

// invoke navigation services code to pick where to save a file
OSStatus
TextViewDemoWriteToCFURL( HIViewRef textView );

OSStatus
SetUpTheTextView(WindowRef window);

OSStatus
TextViewStoreMLTEInstanceData( HIViewRef textView, CMLTEViewData* mlteData );

CMLTEViewData*
TextViewRetrieveMLTEInstanceData( HIViewRef textView );

OSStatus
TextViewInstallMenuHandlers( HIViewRef textView );

OSStatus
TextViewMenuEventHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData );

OSStatus
TextViewProcessHICommand( HIViewRef textView, const HICommand& hiCommand );

OSStatus TextViewReadRangeFromURL( HIViewRef textView, CFURLRef url );

OSStatus
GetTextViewFromWindow( WindowRef window, HIViewRef& textView );

OSStatus
TextViewFocusInWindow( WindowRef window );

OSStatus
TextViewSaveObject( HIViewRef textView, TXNDataType dataType, CFURLRef cfURL );

OSStatus
TextViewWriteToCFURL( HIViewRef textView, CFURLRef fileURL, OSType type, Boolean replacing );

OSStatus
TextViewReadCFURL( HIViewRef textView, CFURLRef url );

OSStatus
TextViewEditCommandSupport( HIViewRef textView, Boolean on );

OSStatus
TextViewSpellingSupport( HIViewRef textView, Boolean on );

Boolean
TextViewIsSpellingSupportEnabled( HIViewRef textView );

OSStatus
TextViewFontPanelSupport( HIViewRef textView, Boolean on );

Boolean
TextViewIsFontPanelSupportEnabled( HIViewRef textView );

OSStatus
TextViewSpellCheckAsYouType( HIViewRef textView, Boolean enable );

Boolean
TextViewToggleSpellCheckAsYouType( HIViewRef textView );

OSStatus
TextViewScrollingOptions( HIViewRef textView, UInt32 opts );

OSStatus
TextViewBeginActionGroup( HIViewRef textView, CFStringRef iActionGroupName );

OSStatus
TextViewEndActionGroup( HIViewRef textView );

OSStatus
TextViewSetEventTarget( HIViewRef textView, HIObjectRef iEventTarget );

OSStatus 				
TextViewGetObjectControlData( HIViewRef textView, TXNControlTag controlTag, Boolean isSigned, 
                                SInt32& oSignedData, UInt32& oUnsignedData );
OSStatus
TextViewSetObjectControlData( HIViewRef textView, TXNControlTag controlTag, Boolean isSigned, 
                                 SInt32 iSignedData, UInt32 iUnsignedData );

// IncrementFontSizeOfCurrentSelection will call through to TXNSetTypeAttributes
OSStatus
TextViewIncrementFontSizeOfCurrentSelection( HIViewRef textView, Boolean incrementUp );

OSStatus
TextViewIncrementFontSizeOfThisRange( HIViewRef textView,
									  TXNOffset startRng, TXNOffset endRng,
									  Boolean incrementUp, UInt32 count );

// SetFontSizeOfCurrentSelection will call through to TXNSetTypeAttributes
OSStatus
TextViewSetFontSizeOfCurrentSelection( HIViewRef textView, UInt32 setSize );

OSStatus
TextViewSetFontSizeOfThisRange( HIViewRef textView, UInt32 setSize, TXNOffset startRng, TXNOffset endRng );

// SetFontStyleOfCurrentSelection will call through to TXNSetTypeAttributes
OSStatus
TextViewSetFontStyleOfCurrentSelection( HIViewRef textView, UInt32 setStyle); // style really SInt16?

OSStatus
TextViewSetFontStyleOfThisRange( HIViewRef textView, UInt32 setStyle, TXNOffset startRng, TXNOffset endRng );

OSStatus
TextViewSetMargins( HIViewRef textView, SInt16 top, SInt16 left, SInt16 right );

#endif
