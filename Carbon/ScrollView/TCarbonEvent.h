/*
    File:		TCarbonEvent.h
    
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

#ifndef CarbonClasses_CARBONEVENT
#define CarbonClasses_CARBONEVENT

#include <Carbon/Carbon.h>

class TCarbonEvent
{
	public:
			TCarbonEvent( UInt32 inClass, UInt32 inKind );
			TCarbonEvent( EventRef inEvent );
		virtual ~TCarbonEvent();
		
		UInt32		GetClass() const;
		UInt32		GetKind() const;
		
		void		SetTime( EventTime inTime );
		EventTime	GetTime() const;
		
		void		Retain();
		void		Release();
		
		OSStatus 	PostToQueue( EventQueueRef inQueue, EventPriority inPriority = kEventPriorityStandard );

		void		SetParameter( EventParamName inName, EventParamType inType, UInt32 inSize, const void* inData );
		OSStatus	GetParameter( EventParamName inName, EventParamType inType, UInt32 inBufferSize, void* outData );

		OSStatus	GetParameterType( EventParamName inName, EventParamType* outType );
		OSStatus	GetParameterSize( EventParamName inName, UInt32* outSize );
	
		// Simple types
		void		SetParameter( EventParamName inName, Boolean inValue )
					{ SetParameter( inName, typeBoolean, sizeof( Boolean ), &inValue ); }
		OSStatus	GetParameter( EventParamName inName, Boolean* outValue )
					{ return GetParameter( inName, typeBoolean, sizeof( Boolean ), outValue ); }

		void		SetParameter( EventParamName inName, Point inPt )
					{ SetParameter( inName, typeQDPoint, sizeof( Point ), &inPt ); }
		OSStatus	GetParameter( EventParamName inName, Point* outPt )
					{ return GetParameter( inName, typeQDPoint, sizeof( Point ), outPt ); }

		void		SetParameter( EventParamName inName, const HIPoint& inPt )
					{ SetParameter( inName, typeHIPoint, sizeof( HIPoint ), &inPt ); }
		OSStatus	GetParameter( EventParamName inName, HIPoint* outPt )
					{ return GetParameter( inName, typeHIPoint, sizeof( HIPoint ), outPt ); }

		void		SetParameter( EventParamName inName, const Rect& inRect )
					{ SetParameter( inName, typeQDRectangle, sizeof( Rect ), &inRect ); }
		OSStatus	GetParameter( EventParamName inName, Rect* outRect )
					{ return GetParameter( inName, typeQDRectangle, sizeof( Rect ), outRect ); }

		void		SetParameter( EventParamName inName, const HIRect& inRect )
					{ SetParameter( inName, typeHIRect, sizeof( HIRect ), &inRect ); }
		OSStatus	GetParameter( EventParamName inName, HIRect* outRect )
					{ return GetParameter( inName, typeHIRect, sizeof( HIRect ), outRect ); }

		void		SetParameter( EventParamName inName, const HISize& inSize )
					{ SetParameter( inName, typeHISize, sizeof( HISize ), &inSize ); }
		OSStatus	GetParameter( EventParamName inName, HISize* outSize )
					{ return GetParameter( inName, typeHISize, sizeof( HISize ), outSize ); }

		void		SetParameter( EventParamName inName, RgnHandle inRegion )
					{ SetParameter( inName, typeQDRgnHandle, sizeof( RgnHandle ), &inRegion ); }
		OSStatus	GetParameter( EventParamName inName, RgnHandle* outRegion )
					{ return GetParameter( inName, typeQDRgnHandle, sizeof( RgnHandle ), outRegion ); }

		void		SetParameter( EventParamName inName, WindowRef inWindow )
					{ SetParameter( inName, typeWindowRef, sizeof( WindowRef ), &inWindow ); }
		OSStatus	GetParameter( EventParamName inName, WindowRef* outWindow )
					{ return GetParameter( inName, typeWindowRef, sizeof( WindowRef ), outWindow ); }

		void		SetParameter( EventParamName inName, ControlRef inControl )
					{ SetParameter( inName, typeControlRef, sizeof( ControlRef ), &inControl ); }
		OSStatus	GetParameter( EventParamName inName, ControlRef* outControl )
					{ return GetParameter( inName, typeControlRef, sizeof( ControlRef ), outControl ); }

		void		SetParameter( EventParamName inName, MenuRef inMenu )
					{ SetParameter( inName, typeMenuRef, sizeof( MenuRef ), &inMenu ); }
		OSStatus	GetParameter( EventParamName inName, MenuRef* outMenu )
					{ return GetParameter( inName, typeMenuRef, sizeof( MenuRef ), outMenu ); }

		void		SetParameter( EventParamName inName, DragRef inDrag )
					{ SetParameter( inName, typeDragRef, sizeof( DragRef ), &inDrag ); }
		OSStatus	GetParameter( EventParamName inName, DragRef* outDrag )
					{ return GetParameter( inName, typeDragRef, sizeof( DragRef ), outDrag ); }

		void		SetParameter( EventParamName inName, UInt32 inValue )
					{ SetParameter( inName, typeUInt32, sizeof( UInt32 ), &inValue ); }
		OSStatus	GetParameter( EventParamName inName, UInt32* outValue )
					{ return GetParameter( inName, typeUInt32, sizeof( UInt32 ), outValue ); }
		
		void		SetParameter( EventParamName inName, const HICommand& inValue )
					{ SetParameter( inName, typeHICommand, sizeof( HICommand ), &inValue ); }
		OSStatus	GetParameter( EventParamName inName, HICommand* outValue )
					{ return GetParameter( inName, typeHICommand, sizeof( HICommand ), outValue ); }

		template <class T> void SetParameter( EventParamName inName, EventParamType inType, const T& inValue )
		{
			SetParameter( inName, inType, sizeof( T ), &inValue );
		}
				
		template <class T> OSStatus GetParameter( EventParamName inName, EventParamType inType, T* outValue )
		{
			return GetParameter( inName, inType, sizeof( T ), outValue );
		}
		
		// I want to put this back in, but I first wanted to make sure everything
		// was being done in terms of TCarbonEvent and not TEvent for now.
		//operator	EventRef&() { return fEvent; };
		EventRef	GetEventRef() { return fEvent; }
		
	private:
		EventRef	fEvent;
};

#endif // CarbonClasses_CARBONEVENT
