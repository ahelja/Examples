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

#include "TObject.h"
#include "TCarbonEvent.h"

static const EventTypeSpec kHIObjectEvents[] =
{	{ kEventClassHIObject, kEventHIObjectConstruct },
	{ kEventClassHIObject, kEventHIObjectInitialize },
	{ kEventClassHIObject, kEventHIObjectDestruct },
	{ kEventClassHIObject, kEventHIObjectPrintDebugInfo }
};

TObject::TObject( HIObjectRef inObject )
{
	fObjectRef = inObject;
}

TObject::~TObject()
{
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	Perform any initialization work.
//
OSStatus
TObject::Initialize( TCarbonEvent& inEvent )
{
	return noErr;
}

//-----------------------------------------------------------------------------------
//	PrintDebugInfoSelf
//-----------------------------------------------------------------------------------
//	Prints debugging info specific to this object. Subclasses should call the
//	inherited method at the end of handling this so that superclass debugging info
//	also gets printed out.
//
void
TObject::PrintDebugInfoSelf()
{
	fprintf( stdout, "TObject" );
}


//-----------------------------------------------------------------------------------
//	RegisterSubclass
//-----------------------------------------------------------------------------------
//	This routine should be called by subclasses so they can be created as HIObjects.
//
OSStatus TObject::RegisterSubclass(
	CFStringRef			inID,
	CFStringRef			inBaseID,
	ConstructProc		inProc )
{
	return HIObjectRegisterSubclass( inID, inBaseID, 0, ObjectEventHandler,
			GetEventTypeCount( kHIObjectEvents ), kHIObjectEvents, (void*) inProc, NULL );
}

//-----------------------------------------------------------------------------------
//	CreateInitializationEvent
//-----------------------------------------------------------------------------------
// 	Create a basic intialization event.
//
EventRef TObject::CreateInitializationEvent()
{
	OSStatus		result = noErr;
	EventRef		event;

	result = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize,
					GetCurrentEventTime(), 0, &event );
	require_noerr_action( result, CantCreateEvent, event = NULL );
		
CantCreateEvent:
	return event;
}

//-----------------------------------------------------------------------------------
//	ObjectEventHandler
//-----------------------------------------------------------------------------------
//	Our static event handler proc. We handle any HIObject based events directly in
// 	here at present.
//
OSStatus TObject::ObjectEventHandler(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	OSStatus			result = eventNotHandledErr;
	TObject*			instance = (TObject*) inUserData;
	TCarbonEvent		event( inEvent );
	
	switch ( event.GetClass() )
	{
		case kEventClassHIObject:
			switch ( event.GetKind() )
			{
				case kEventHIObjectConstruct:
					{
						HIObjectRef		object;

						result = event.GetParameter<HIObjectRef>( kEventParamHIObjectInstance,
								typeHIObjectRef, &object );
						require_noerr( result, ParameterMissing );
						
						// on entry for our construct event, we're passed the
						// creation proc we registered with for this class.
						// we use it now to create the instance, and then we
						// replace the instance parameter data with said instance
						// as type void.

						result = (*(ConstructProc)inUserData)( object, &instance );

						if ( result == noErr )
						{
							event.SetParameter<TObject*>( kEventParamHIObjectInstance,
									typeVoidPtr, instance ); 
						}
					}
					break;
				
				case kEventHIObjectInitialize:
					result = CallNextEventHandler( inCallRef, inEvent );
					if ( result == noErr )
						result = instance->Initialize( event );
					break;
				
				case kEventHIObjectPrintDebugInfo:
					instance->PrintDebugInfoSelf();
					// don't set result, let it propagate
					break;

				case kEventHIObjectDestruct:
					delete instance;
					// result is unimportant
					break;
			}
			break;
	}

ParameterMissing:

	return result;
}
