/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "TWindow.h"
#include "MyFrameLoadAdapter.h"
#include "MyWebResourceLoadAdapter.h"
#include "MyWebUIAdapter.h"
#include "TBackForwardMenu.h"

class TWebWindow : public TWindow
{
	public:
			TWebWindow();
		virtual ~TWebWindow();

		void				LoadURL( CFURLRef inURL );
		void				LoadRequest( NSURLRequest* inRequest );

		void				SetTextField( CFStringRef inString );
		void				SetStatus( CFStringRef inString );
		void				SetLoadProgress( SInt32 inCurrResources, SInt32 inTotalResources );

		void				FrameLoadStarted( WebDataSource* dataSource );
		void				ReceivedPageTitle( CFStringRef title, WebDataSource* dataSource );
		void				FrameLoadDone( NSError* error, WebDataSource* dataSource );


		// This will either use the top window, or create a new one if there are none open
		static void			GoToItem( WebHistoryItem* inItem );
		static TWebWindow*	GetFromWindowRef( WindowRef inWindow );
		
	protected:
		
		virtual OSStatus	HandleEvent( EventHandlerCallRef handler, TCarbonEvent& inEvent );

		virtual Boolean		HandleCommand( const HICommand& command );
		virtual Boolean		UpdateCommandStatus( const HICommand& command );
                
                void HandlePageSetup();
                void HandlePrint();

                void PageSetupDone(Boolean accepted);
                void PrintDialogDone(Boolean accepted);
                
                static void PageSetupSheetDone(PMPrintSession printSession, WindowRef documentWindow, Boolean accepted);
                static void PrintingSheetDone(PMPrintSession printSession, WindowRef documentWindow, Boolean accepted);

	private:
        
		static OSStatus		TextInputHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
		static OSStatus		ContentBoundsChanged( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );

		Boolean				AcceptTextInput( ControlRef inControl, UniChar inChar, UInt32 modifiers, EventRef inEvent );

		OSStatus			HandleFrameLoad( TCarbonEvent& inEvent );
		OSStatus			HandleResourceLoad( TCarbonEvent& inEvent );
		OSStatus			HandleWebUI( TCarbonEvent& inEvent );

		void				SetPageIcon( CGImageRef inImage );
		void				UpdateLoadStatus();
		void				MouseMovedOverElement( CFDictionaryRef inDict );
		CFStringRef			CopyStatusText();
		Boolean				CanViewSource();
		void				ViewSource();

		float				fBrowserBottomGap;
		float				fBrowserRightGap;
		float				fStatusBottomGap;
		float				fProgressBottomGap;
		float				fProgressRightGap;
		float				fStatusProgressGap;

		WebView*			fController;
		HIViewRef			fWebView;
		HIViewRef			fTextField;
		HIViewRef			fStatusText;
		HIViewRef			fProgressBar;
		HIViewRef			fBackButton;
		HIViewRef			fForwardButton;
		HIViewRef			fPageIcon;
		HIViewRef			fReloadButton;
		HIViewRef			fStopButton;
		
		MyFrameLoadAdapter*		fFrameLoadelegate;
		MyWebResourceLoadAdapter*	fResourceLoadDelegate;
		MyWebUIAdapter*			fWindowOperationsDelegate;
		
		TBackForwardMenu*		fBackMenu;
		TBackForwardMenu*		fForwardMenu;

		UInt32				fResourceCount;
		UInt32				fResourceCompletedCount;
		UInt32				fResourceFailedCount;
		
		bool				fIsComposited;
                
                PMPrintSession			fPrintSession;
                PMPageFormat			fPageFormat;
                PMPrintSettings			fPrintSettings;
};

