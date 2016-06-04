/*
 *  UCarbonEvent.cpp
 *  HIViewFramework
 *
 *  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 */
 
#include "UCarbonEvent.h"

UCarbonEvent *UCarbonEvent::CreateCarbonEvent(UInt32 eventClass, UInt32 eventKind)
{
    UCarbonEvent *retVal = NULL;
    EventRef newEvent = NULL;
    
    OSStatus createEventErr = MacCreateEvent(
                                NULL,
                                eventClass,
                                eventKind,
                                GetCurrentEventTime(),
                                0L,
                                &newEvent);

    if(noErr == createEventErr && NULL != newEvent) {
        try {
            retVal = new UCarbonEvent(newEvent);
            
            // new object now owns the event so we can give it up
            ::ReleaseEvent(newEvent);
        }

        catch(...) {
            // clean up the event we tried to create
            ::ReleaseEvent(newEvent);
            newEvent = NULL;
        }
    }

    return retVal;
}

UCarbonEvent::UCarbonEvent(EventRef inEvent) :
    mEvent(inEvent)
{
    if(NULL != mEvent) {
        ::RetainEvent(mEvent);
    }
}

UCarbonEvent::~UCarbonEvent()
{
    if(NULL != mEvent) {
        ::ReleaseEvent(mEvent);
    }
}
