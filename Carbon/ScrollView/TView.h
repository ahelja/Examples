/*
	File:		TView.h

    Version:	Mac OS X

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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#ifndef SimplerText_TVIEW
#define SimplerText_TVIEW

#include <Carbon/Carbon.h>

#include "TCarbonEvent.h"

#define PURE_VIRTUAL 0

class TView
{
	public:
		void							SetFrame( const HIRect& inBounds )
											{ HIViewSetFrame( GetViewRef(), &inBounds ); }
		HIRect							Frame()
											{ HIRect frame; HIViewGetFrame( GetViewRef(), &frame ); return frame; }
		HIRect							Bounds()
											{ HIRect bounds; HIViewGetBounds( GetViewRef(), &bounds ); return bounds; }

		void							Show() { HIViewSetVisible( GetViewRef(), true ); }
		void							Hide()  { HIViewSetVisible( GetViewRef(), false ); }
		EventTargetRef					GetEventTarget() { return HIObjectGetEventTarget( (HIObjectRef)GetViewRef() ); }

		void							AddSubView( TView* other ) { HIViewAddSubview( GetViewRef(), other->GetViewRef() ); }
		void							RemoveFromSuperView() { HIViewRemoveFromSuperview( GetViewRef() ); }

		HIViewRef						GetViewRef() { return fViewRef; }

	protected:
		enum Interface { kDragAndDrop = 1, kKeyboardFocus };

		typedef OSStatus				(*ConstructProc)( ControlRef inBaseControl, TView** outView );

			TView( ControlRef inControl );
		virtual ~TView();
		
		virtual OSStatus					Track( TCarbonEvent& inEvent, ControlPartCode* outPartHit );
		
		virtual void						Draw( RgnHandle limitRgn, CGContextRef inContext );
		virtual ControlPartCode				HitTest( const HIPoint& where );
		virtual OSStatus					GetRegion( ControlPartCode part, RgnHandle outRgn );
		virtual void						PrintDebugInfo();
		virtual ControlKind					GetKind() = PURE_VIRTUAL;
		virtual OSStatus					GetData( OSType tag, ControlPartCode part, Size inSize, Size* outSize, void* ptr );
		virtual OSStatus					SetData( OSType tag, ControlPartCode part, Size size, const void* ptr );

		virtual OSStatus					SetFocusPart( ControlPartCode desiredFocus, RgnHandle invalidRgn, Boolean focusEverything, ControlPartCode* actualFocus );
		
		virtual OSStatus					GetSizeConstraints( HISize* outMin, HISize* outMax );
		virtual OSStatus					GetOptimalSize( HISize* outSize, float* outBaseLine );

		virtual void						BoundsChanged( UInt32 options, const HIRect& originalBounds,
														const HIRect& currentBounds );

		WindowRef							GetWindowRef() { return GetControlOwner( GetViewRef() ); }
		
		static void							RegisterSubclass( CFStringRef inID, ConstructProc inProc );
		static EventRef						CreateInitializationEvent();

		virtual OSStatus					Initialize( TCarbonEvent& inEvent );
		virtual void						ActivateInterface( Interface );
		virtual UInt32						GetBehaviors();
		
		// Drag and drop
		virtual bool						DragEnter( DragRef inDrag );
		virtual bool						DragWithin( DragRef inDrag );
		virtual bool						DragLeave( DragRef inDrag );
		virtual OSStatus					DragReceive( DragRef inDrag );

		// Command processing
		virtual OSStatus					ProcessCommand( const HICommand& command );
		virtual OSStatus					UpdateCommandStatus( const HICommand& command );

	private:
		static OSStatus						ClassHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
		OSStatus							HandleEvent( EventHandlerCallRef inCallRef, TCarbonEvent& inEvent );

		HIViewRef					fViewRef;
		EventHandlerRef				fHandler;
};

typedef TView*		TViewPtr;

#endif // SimplerText_TVIEW
