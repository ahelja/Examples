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
/*
    UCarbonEvent.h
    HIViewFramework
    
    Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
    
    A utility class for working with Carbon events.  
    
    This is a pretty thin adapter to the Carbon Event APIs and makes use
    of the "template method" capabilities of C++ for the parameter getting
    and setting methods.  
    
    A point of caution... While most of the current and popular C++ compilers 
    for the Macintosh environment do support template methods, some older
    C++ compilers do not.  Check the documentation for your compiler to 
    determine if this advanced behavior is available.
 */
 
#include <Carbon/Carbon.h>

class UCarbonEvent
{
    public:
        /* factory for creating a new carbon event wrapped by a UCarbonEvent */
        static UCarbonEvent *CreateCarbonEvent(UInt32 eventClass, UInt32 eventKind);
        
        UCarbonEvent(EventRef inEvent);
        virtual ~UCarbonEvent();

        inline operator EventRef() const;

        inline UInt32		GetEventKind() const;
        inline UInt32		GetEventClass() const;
        inline EventTime	GetEventTime(EventRef inEvent) const;
        
        template <class ParamDataType>
        OSStatus GetParameter(EventParamName inName, EventParamType inDesiredType, ParamDataType &outParamValue);

        template <class ParamDataType>
        OSStatus SetParameter(EventParamName inName, EventParamType inType, const ParamDataType &inParamValue);

    private:
        EventRef mEvent;
};

inline UCarbonEvent::operator EventRef() const
{
    return mEvent;
}

inline UInt32 UCarbonEvent::GetEventKind() const
{
    UInt32 retVal = 0;
    
    if(NULL != mEvent) {
    	retVal = ::GetEventKind(mEvent);
    }
    
    return retVal;
}

inline UInt32 UCarbonEvent::GetEventClass() const
{
    UInt32 retVal = 0;

    if(NULL != mEvent) {
    	retVal = ::GetEventClass(mEvent);
    }

    return retVal;
}

inline EventTime UCarbonEvent::GetEventTime(EventRef inEvent) const
{
    EventTime retVal = 0;
    
    if(NULL != mEvent) {
        retVal = ::GetEventTime(mEvent);
    }
    
    return retVal;
}

template <class ParamDataType>
OSStatus UCarbonEvent::GetParameter(EventParamName inName, EventParamType inDesiredType, ParamDataType &outParamValue)
{
    return GetEventParameter(
                mEvent,
                inName,
                inDesiredType,
                NULL,
                sizeof(ParamDataType),
                NULL,
                &outParamValue);
}

template <class ParamDataType>
OSStatus UCarbonEvent::SetParameter(EventParamName inName, EventParamType inType, const ParamDataType &inParamValue)
{
    return  SetEventParameter(
                mEvent,
                inName,
                inType,
                sizeof(ParamDataType),
                &inParamValue); 
}

