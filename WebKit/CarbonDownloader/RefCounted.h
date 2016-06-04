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
    RefCounted.h

    Copyright (c) 2003 Apple Computer, Inc. All rights reserved.

    This header defines two classes, RefCounted is an abstract class for 
    objects that can be stored in certain Core Foundation container classes.
    
    As such it defines the abstract Release, Retain, GetRefCount mechanism as
    well as common idioms like the HashCode and CopyDescription mechanisms.
    
    The Abstract RefCounted class, however, does not actually define any
    storage for a refcount (it could be inherited by classes that want the
    CF container behavior but which already have a ref count).  
    
    The ConcreteRefCount class is a subclass of RefCount, and therefore inherits
    all of it's behavior, but adds storage for a reference count and overrides
    the appropriate methods to have that refCount used.
*/
 
#pragma once

#include <Carbon/Carbon.h>

class RefCounted
{
    public:
        RefCounted();
        RefCounted(const RefCounted &itemToCopy);
        virtual ~RefCounted();
        
        virtual unsigned long Retain() = 0;
        virtual unsigned long Release() = 0;
        virtual bool Equal(RefCounted *other);
        virtual CFStringRef   CopyDescription();
        virtual CFHashCode    Hash();
        virtual long GetRefCount() const = 0;
        
        static const CFArrayCallBacks 		kCFArrayCallbacks;
        static const CFSetCallBacks   		kCFSetCallbacks;
        static const CFDictionaryKeyCallBacks 	kCFDictionaryKeyCallbacks;
        static const CFDictionaryValueCallBacks kCFDictionaryValueCallbacks;

        static const void *	CFRetainCallBack(CFAllocatorRef allocator, const void *value);
	static void		CFReleaseCallBack(CFAllocatorRef allocator, const void *value);
        static CFStringRef	CFCopyDescriptionCallBack(const void *value);
	static Boolean		CFEqualCallBack(const void *value1, const void *value2);
        static CFHashCode	CFHashCallBack(const void *value);
};

class ConcreteRefCount : public RefCounted
{
    public:
        ConcreteRefCount();
        ConcreteRefCount(const ConcreteRefCount &itemToCopy);
        virtual ~ConcreteRefCount();
        
        virtual unsigned long Retain();
        virtual unsigned long Release();
        virtual long GetRefCount() const;

    private:
        long mRefCount;
};

template <class T>
class ref_auto_ptr
{
    public:
        typedef T* address_of_type;
        
        ref_auto_ptr() throw() : mValue(NULL) {}
        ref_auto_ptr(address_of_type newValue) throw() : mValue(newValue) {
            if(NULL != newValue) {
                // autoptr owns the object
                newValue->Retain();	
            }
        }
        
        ref_auto_ptr(const ref_auto_ptr<T> &ptrToCopy) : mValue(NULL) {
            reset(ptrToCopy.mValue);
        }
        
        // Assignment to another smart pointer
        //
        // Release the old object we owned and add a reference to the
        // object the other pointer owns.
        ref_auto_ptr<T> &operator = (const ref_auto_ptr<T> &ptrToCopy) {
            reset(ptrToCopy.mValue);
            return *this;
        }
        
        // Assignment to a T*
        //
        // Release the old object we owned and assume the new pointer
        ref_auto_ptr<T> &operator =(address_of_type newPtr) {
            reset(newPtr);
            return *this;
        }
        
        virtual ~ref_auto_ptr() {
            if(NULL != mValue) {
                mValue->Release();
                mValue = NULL;
            }
        }

        operator address_of_type(void) {
            return mValue;
        }
        
        T& operator *() const throw() {
            return *mValue;
        }
        
        address_of_type operator ->() const throw() {
            return mValue;
        }

        bool operator ==(const T& rhs) {
            return mValue == rhs.mValue;
        }
        
        bool operator !() {
            return (mValue == NULL);
        }
        
        address_of_type get() const throw() {
            return mValue;
        }
        
        void reset(address_of_type newPtr = 0) throw() {
            address_of_type oldValue = mValue;
            
            mValue = newPtr;
            
            if(NULL != mValue) {
                mValue->Retain();
            }
            
            if(NULL != oldValue) {
                oldValue->Release();
            }
        }
        
    private:
        address_of_type mValue;
};
