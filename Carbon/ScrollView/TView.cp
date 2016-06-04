/*
    File:		TView.cp
    
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

/*
	NOTE:	 This is NOWHERE near a completely enhaustive implementation of a view. There are
			many more carbon events one could intercept and hook into this.
*/

#include "TView.h"

const EventTypeSpec kEvents[] =
{	{ kEventClassHIObject, kEventHIObjectConstruct },
	{ kEventClassHIObject, kEventHIObjectInitialize },
	{ kEventClassHIObject, kEventHIObjectDestruct },
	
	{ kEventClassCommand, kEventCommandProcess },
	{ kEventClassCommand, kEventCommandUpdateStatus },
	
	{ kEventClassControl, kEventControlInitialize },
	{ kEventClassControl, kEventControlDraw },
	{ kEventClassControl, kEventControlHitTest },
	{ kEventClassControl, kEventControlGetPartRegion },
	{ kEventClassControl, kEventControlGetData },
	{ kEventClassControl, kEventControlSetData },
	{ kEventClassControl, kEventControlGetOptimalBounds },
	{ kEventClassControl, kEventControlBoundsChanged },
	{ kEventClassControl, kEventControlTrack },
	{ kEventClassControl, kEventControlGetSizeConstraints }
};

TView::TView( HIViewRef inControl )
	: fViewRef( inControl )
{
}

TView::~TView()
{
}

//-----------------------------------------------------------------------------------
//	• Initialize
//-----------------------------------------------------------------------------------
//	Called during HIObject construction, this is your subclasses' chance to extract
//	any parameters it might have added to the initialization event passed into the
//	HIObjectCreate call.
//
OSStatus
TView::Initialize( TCarbonEvent& inEvent )
{
	return noErr;
}

//-----------------------------------------------------------------------------------
//	• GetBehaviors
//-----------------------------------------------------------------------------------
//	Returns our behaviors. Any subclass that overrides this should OR in its behaviors
//	into the inherited behaviors.
//
UInt32
TView::GetBehaviors()
{
	return kControlSupportsDataAccess | kControlSupportsGetRegion;
}

//-----------------------------------------------------------------------------------
//	• Draw
//-----------------------------------------------------------------------------------
//	Draw your view. You should draw based on VIEW coordinates, not frame coordinates.
//	Currently, the input context is not being passed, this will come in time...
void
TView::Draw( RgnHandle limitRgn, CGContextRef inContext )
{
}

//-----------------------------------------------------------------------------------
//	• HitTest
//-----------------------------------------------------------------------------------
//	Asks your view to return what part of itself (if any) is hit by the point given
//	to it. The point is in VIEW coordinates, so you should get the view rect to do
//	bounds checking.
//
ControlPartCode
TView::HitTest( const HIPoint& where )
{
	return kControlNoPart;
}

//-----------------------------------------------------------------------------------
//	• GetRegion
//-----------------------------------------------------------------------------------
//	This is called when someone wants to know certain metrics regarding this view.
//	The base class does nothing. Subclasses should handle their own parts, such as
//	the content region by overriding this method. The structure region is, by default,
//	the view's bounds. If a subclass does not have a region for a given part, it 
//	should always call the inherited method.
//
OSStatus
TView::GetRegion( ControlPartCode part, RgnHandle outRgn )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	• PrintDebugInfo
//-----------------------------------------------------------------------------------
//	This is called when asked to print debugging information.
//
void
TView::PrintDebugInfo()
{
}

//-----------------------------------------------------------------------------------
//	• GetData
//-----------------------------------------------------------------------------------
//	Gets some data from our view. Subclasses should override to handle their own
//	defined data tags. If a tag is not understood by the subclass, it should call the
//	inherited method. As a convienience, we map the request for ControlKind into our
//	GetKind method.
//
OSStatus
TView::GetData( OSType tag, ControlPartCode part, Size inSize, Size* outSize, void* ptr )
{
	OSStatus		err = noErr;
	
	switch( tag )
	{
		case kControlKindTag:
			if ( ptr )
			{
				if ( inSize != sizeof( ControlKind ) )
				{
					err = errDataSizeMismatch;
				}
				else
				{
					(*(ControlKind *) ptr) = GetKind();
				}
			}
			*outSize = sizeof( ControlKind );
			break;
		
		default:
			err = eventNotHandledErr;
			break;
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	• SetData
//-----------------------------------------------------------------------------------
//	Sets some data on our control. Subclasses should override to handle their own
//	defined data tags. If a tag is not understood by the subclass, it should call the
//	inherited method.
//
OSStatus
TView::SetData( OSType tag, ControlPartCode part, Size size, const void* ptr )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	• GetOptimalSize
//-----------------------------------------------------------------------------------
//	Someone wants to know this view's optimal size and text baseline, probably to help
//	do some type of layout. The base class does nothing, but subclasses should
//	override and do something meaningful here.
//
OSStatus
TView::GetOptimalSize( HISize* outSize, float* outBaseLine )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	• GetSizeConstraints
//-----------------------------------------------------------------------------------
//	Someone wants to know this view's minimum and maximum sizes, probably to help
//	do some type of layout. The base class does nothing, but subclasses should
//	override and do something meaningful here.
//
OSStatus
TView::GetSizeConstraints( HISize* outMin, HISize* outMax )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	• BoundsChanged
//-----------------------------------------------------------------------------------
//	The bounds of our view have changes. Subclasses can override here to make note
//	of it and flush caches, etc. The base class does nothing.
//
void
TView::BoundsChanged( 	UInt32 			options,
						const HIRect& 	originalBounds,
						const HIRect& 	currentBounds )
{
}

//-----------------------------------------------------------------------------------
//	• DragEnter
//-----------------------------------------------------------------------------------
//	A drag has entered our bounds. The Drag and Drop interface also should have been
//	activated or else this method will NOT be called. If true is returned, this view
//	likes the drag and will receive drag within/leave/receive messages as appropriate.
//	If false is returned, it is assumed the drag is not valid for this view, and no
//	further drag activity will flow into this view unless the drag leaves and is
//	re-entered.
//
bool
TView::DragEnter( DragRef inDrag )
{
	return false;
}

//-----------------------------------------------------------------------------------
//	• DragWithin
//-----------------------------------------------------------------------------------
//	A drag has moved within our bounds. In order for this method to be called, the
//	view must have signaled the drag as being desirable in the DragEnter method. The
//	Drag and Drop interface also should have been activated.
//
bool
TView::DragWithin( DragRef inDrag )
{
	return false;
}

//-----------------------------------------------------------------------------------
//	• DragLeave
//-----------------------------------------------------------------------------------
//	A drag has left. Deal with it. Subclasses should override as necessary. The
//	Drag and Drop interface should be activated in order for this method to be valid.
//	The drag must have also been accepted in the DragEnter method, else this method
//	will NOT be called.
//
bool
TView::DragLeave( DragRef inDrag )
{
	return false;
}

//-----------------------------------------------------------------------------------
//	• DragReceive
//-----------------------------------------------------------------------------------
//	Deal with receiving a drag. By default we return dragNotAcceptedErr. I'm not sure
//	if this is correct, or eventNotHandledErr. Time will tell...
//
OSStatus
TView::DragReceive( DragRef inDrag )
{
	return dragNotAcceptedErr;
}

//-----------------------------------------------------------------------------------
//	• Track
//-----------------------------------------------------------------------------------
//	Default tracking method. Subclasses should override as necessary. We do nothing
//	here in the base class, so we return eventNotHandledErr.
//
OSStatus
TView::Track( TCarbonEvent& inEvent, ControlPartCode* outPart )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	• SetFocusPart
//-----------------------------------------------------------------------------------
//	Handle focusing. Our base behavior is to punt.
//
OSStatus
TView::SetFocusPart( ControlPartCode desiredFocus, RgnHandle invalidRgn, Boolean focusEverything, ControlPartCode* actualFocus )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	• ProcessCommand
//-----------------------------------------------------------------------------------
//	Process a command. Subclasses should override as necessary.
//
OSStatus
TView::ProcessCommand( const HICommand& inCommand )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	• UpdateCommandStatus
//-----------------------------------------------------------------------------------
//	Update the status for a command. Subclasses should override as necessary.
//
OSStatus
TView::UpdateCommandStatus( const HICommand& inCommand )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	• ActivateInterface
//-----------------------------------------------------------------------------------
//	This routine is used to allow a subclass to turn on a specific event or suite of
//	events, like Drag and Drop. This allows us to keep event traffic down if we are
//	not interested, but register for the events if we are.
//
void
TView::ActivateInterface( TView::Interface inInterface )
{
	switch( inInterface )
	{
		case kDragAndDrop:
			{
				static const EventTypeSpec kDragEvents[] =
				{
					{ kEventClassControl, kEventControlDragEnter },			
					{ kEventClassControl, kEventControlDragLeave },			
					{ kEventClassControl, kEventControlDragWithin },			
					{ kEventClassControl, kEventControlDragReceive }
				};
				
				AddEventTypesToHandler( fHandler, GetEventTypeCount( kDragEvents ), kDragEvents );
			}
			break;
			
		case kKeyboardFocus:
			{
				static const EventTypeSpec kKeyboardFocusEvents[] =
				{
					{ kEventClassControl, kEventControlSetFocusPart }
				};
				
				AddEventTypesToHandler( fHandler, GetEventTypeCount( kKeyboardFocusEvents ), kKeyboardFocusEvents );
			}
			break;
	}
}

#define kHIViewBaseClassID		CFSTR( "com.apple.hiview" )

//-----------------------------------------------------------------------------------
//	• RegisterSubclass
//-----------------------------------------------------------------------------------
//	This routine should be called by subclasses so they can be created as HIObjects.
//
void
TView::RegisterSubclass( CFStringRef inID, ConstructProc inProc )
{
	HIObjectRegisterSubclass( inID, kHIViewBaseClassID, 0, ClassHandler, GetEventTypeCount( kEvents ), kEvents, (void *)inProc, NULL );
}

//-----------------------------------------------------------------------------------
//	• ClassHandler
//-----------------------------------------------------------------------------------
//	Our static event handler proc. We handle any HIObject based events directly in
// 	here at present, and forward the rest to the objects EventHandler method.
//
OSStatus
TView::ClassHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	OSStatus			result = eventNotHandledErr;
	TView*				view = (TView*)inUserData;
	TCarbonEvent		event( inEvent );
	
	switch ( event.GetClass() )
	{
		case kEventClassHIObject:
			switch ( event.GetKind() )
			{
				case kEventHIObjectConstruct:
					{
						ControlRef		control; // ControlRefs are HIObjectRefs
						TView*			view;

						result = event.GetParameter<HIObjectRef>( kEventParamHIObjectInstance, typeHIObjectRef, (HIObjectRef*)&control );
						require_noerr( result, ParameterMissing );
						
						// on entry for our construct event, we're passed the
						// creation proc we registered with for this class.
						// we use it now to create the instance, and then we
						// replace the instance parameter data with said instance
						// as type void.

						result = (*(ConstructProc)inUserData)( control, &view );
						if ( result == noErr )
							event.SetParameter<TViewPtr>( kEventParamHIObjectInstance, typeVoidPtr, view ); 
					}
					break;
				
				case kEventHIObjectInitialize:
					result = CallNextEventHandler( inCallRef, inEvent );
					if ( result == noErr )
						result = view->Initialize( event );
					break;
				
				case kEventHIObjectDestruct:
					delete view;
					break;
			}
			break;
		
		default:
			result = view->HandleEvent( inCallRef, event );
			break;
	}

ParameterMissing:

	return result;
}

//-----------------------------------------------------------------------------------
//	• HandleEvent
//-----------------------------------------------------------------------------------
//	Our objects virtual event handler method. I'm not sure if we need this these days.
//	We used to do various things with it, but those days are long gone...
//
OSStatus
TView::HandleEvent( EventHandlerCallRef inCallRef, TCarbonEvent& inEvent )
{
	OSStatus		result = eventNotHandledErr;
	HIPoint			where;
	OSType			tag;
	void *			ptr;
	Size			size, outSize;
	UInt32			features;
	RgnHandle		region = NULL;
	ControlPartCode	part;
	
	switch ( inEvent.GetClass() )
	{
		case kEventClassCommand:
			{
				HICommand		command;
				
				result = inEvent.GetParameter( kEventParamDirectObject, &command );
				require_noerr( result, MissingParameter );
				
				switch ( inEvent.GetKind() )
				{
					case kEventCommandProcess:
						result = ProcessCommand( command );
						break;
					
					case kEventCommandUpdateStatus:
						result = UpdateCommandStatus( command );
						break;
				}
			}
			break;

		case kEventClassControl:
			switch ( inEvent.GetKind() )
			{
				case kEventControlInitialize:
					features = GetBehaviors();
					inEvent.SetParameter( kEventParamControlFeatures, features );
					result = noErr;
					break;
					
				case kEventControlDraw:
					{
						CGContextRef		context = NULL;
						
						inEvent.GetParameter( kEventParamRgnHandle, &region );
						inEvent.GetParameter<CGContextRef>( kEventParamCGContextRef, typeCGContextRef, &context );

						Draw( region, context );
						result = noErr;
					}
					break;
				
				case kEventControlHitTest:
					inEvent.GetParameter<HIPoint>( kEventParamMouseLocation, typeHIPoint, &where );
					part = HitTest( where );
					inEvent.SetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, part );
					result = noErr;
					break;
					
				case kEventControlGetPartRegion:
					inEvent.GetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, &part );
					inEvent.GetParameter( kEventParamControlRegion, &region );
					result = GetRegion( part, region );
					break;
				
				case kEventControlGetData:
					inEvent.GetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, &part );
					inEvent.GetParameter<OSType>( kEventParamControlDataTag, typeEnumeration, &tag );
					inEvent.GetParameter<Ptr>( kEventParamControlDataBuffer, typePtr, (Ptr*)&ptr );
					inEvent.GetParameter<Size>( kEventParamControlDataBufferSize, typeLongInteger, &size );

					result = GetData( tag, part, size, &outSize, ptr );

					if ( result == noErr )
					{
						inEvent.SetParameter<Size>( kEventParamControlDataBufferSize, typeLongInteger, outSize );
					}
					break;
				
				case kEventControlSetData:
					inEvent.GetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, &part );
					inEvent.GetParameter<OSType>( kEventParamControlDataTag, typeEnumeration, &tag );
					inEvent.GetParameter<Ptr>( kEventParamControlDataBuffer, typePtr, (Ptr*)&ptr );
					inEvent.GetParameter<Size>( kEventParamControlDataBufferSize, typeLongInteger, &size );

					result = SetData( tag, part, size, ptr );
					break;
				
				case kEventControlGetOptimalBounds:
					{
						HISize		size;
						float		floatBaseLine;
						
						result = GetOptimalSize( &size, &floatBaseLine );
						if ( result == noErr )
						{
							Rect		bounds;
							SInt16		baseLine;

							GetControlBounds( GetViewRef(), &bounds );

							bounds.bottom = bounds.top + (SInt16)size.height;
							bounds.right = bounds.left + (SInt16)size.width;
							baseLine = (SInt16)floatBaseLine;
							
							inEvent.SetParameter( kEventParamControlOptimalBounds, bounds );
							inEvent.SetParameter<SInt16>( kEventParamControlOptimalBaselineOffset, typeShortInteger, baseLine );
						}
					}
					break;
				
				case kEventControlBoundsChanged:
					{
						HIRect		prevRect, currRect;
						UInt32		attrs;
						
						inEvent.GetParameter( kEventParamAttributes, &attrs );
						inEvent.GetParameter( kEventParamOriginalBounds, &prevRect );
						inEvent.GetParameter( kEventParamCurrentBounds, &currRect );

						BoundsChanged( attrs, prevRect, currRect );
						result = noErr;
					}
					break;

				case kEventControlDragEnter:
				case kEventControlDragLeave:
				case kEventControlDragWithin:
					{
						DragRef		drag;
						
						inEvent.GetParameter( kEventParamDragRef, &drag );

						switch ( inEvent.GetKind() )
						{
							case kEventControlDragEnter:
								if ( DragEnter( drag ) )
									result = noErr;
								break;
							
							case kEventControlDragLeave:
								DragLeave( drag );
								result = noErr;
								break;
							
							case kEventControlDragWithin:
								DragWithin( drag );
								result = noErr;
								break;
						}
					}
					break;
				
				case kEventControlDragReceive:
					{
						DragRef		drag;
						
						inEvent.GetParameter( kEventParamDragRef, &drag );

						result = DragReceive( drag );
					}
					break;
				
				case kEventControlTrack:
					{
						ControlPartCode		part;
						
						result = Track( inEvent, &part );
						if ( result == noErr )
						{
							inEvent.SetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, part );
						}
					}
					break;

				case kEventControlGetSizeConstraints:
					{
						HISize		minSize, maxSize;
						
						result = GetSizeConstraints( &minSize, &maxSize );

						if ( result == noErr )
						{
							inEvent.SetParameter( kEventParamMinimumSize, minSize );
							inEvent.SetParameter( kEventParamMaximumSize, maxSize );
						}
					}
					break;

				case kEventControlSetFocusPart:
					{
						ControlPartCode		desiredFocus;
						RgnHandle			invalidRgn;
						Boolean				focusEverything;
						ControlPartCode		actualFocus;
						
						result = inEvent.GetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, &desiredFocus ); 
						require_noerr( result, MissingParameter );
						
						inEvent.GetParameter( kEventParamControlInvalRgn, &invalidRgn );

						focusEverything = false; // a good default in case the parameter doesn't exist

						inEvent.GetParameter( kEventParamControlFocusEverything, &focusEverything );

						result = SetFocusPart( desiredFocus, invalidRgn, focusEverything, &actualFocus );
						
						if ( result == noErr )
						{
							inEvent.SetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, part );
						}
					}
					break;
			}
			break;
	}

MissingParameter:

	return result;
}

//-----------------------------------------------------------------------------------
//	• CreateInitializationEvent
//-----------------------------------------------------------------------------------
// 	Create a basic intialization event containing the parent control and bounds. At
//	present we set the bounds to empty and the parent to NULL. In theory, after creating
//	this event, any subclass could add its own parameter to receive in its
//	Initialize method.
//
EventRef
TView::CreateInitializationEvent()
{
	OSStatus		result = noErr;
	EventRef		event;

	result = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize,
					GetCurrentEventTime(), 0, &event );
	require_noerr( result, CantCreateEvent );
		
	return event;

CantCreateEvent:
	return NULL;
}


