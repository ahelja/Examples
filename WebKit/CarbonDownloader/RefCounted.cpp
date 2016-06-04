//
// RefCounted.cpp
//
// Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//
//
 
#include "RefCounted.h"

const CFArrayCallBacks RefCounted::kCFArrayCallbacks = 
    {
        0,
	RefCounted::CFRetainCallBack,
	RefCounted::CFReleaseCallBack,
	RefCounted::CFCopyDescriptionCallBack,
	RefCounted::CFEqualCallBack
    };

const CFSetCallBacks RefCounted::kCFSetCallbacks =
    {
        0,
	RefCounted::CFRetainCallBack,
	RefCounted::CFReleaseCallBack,
	RefCounted::CFCopyDescriptionCallBack,
	RefCounted::CFEqualCallBack,
        RefCounted::CFHashCallBack
    };

const CFDictionaryKeyCallBacks RefCounted::kCFDictionaryKeyCallbacks =
    {
        0,
	RefCounted::CFRetainCallBack,
	RefCounted::CFReleaseCallBack,
	RefCounted::CFCopyDescriptionCallBack,
	RefCounted::CFEqualCallBack,
        RefCounted::CFHashCallBack
    };
    
const CFDictionaryValueCallBacks RefCounted::kCFDictionaryValueCallbacks =
    {
        0, 
	RefCounted::CFRetainCallBack,
	RefCounted::CFReleaseCallBack,
	RefCounted::CFCopyDescriptionCallBack,
	RefCounted::CFEqualCallBack
    };

/* --------------------------------------------------------------- RefCounted */
RefCounted::RefCounted()
{
}

/* --------------------------------------------------------------- RefCounted */
RefCounted::RefCounted(const RefCounted &itemToCopy)
{
}

/* -------------------------------------------------------------- ~RefCounted */
RefCounted::~RefCounted()
{
}

/* ------------------------------------------------------------------- Retain */
/*
    RefCounted is an abstract class and subclasses should override this method
*/
unsigned long RefCounted::Retain()
{
    return 0;
}

/* ------------------------------------------------------------------ Release */
/*
    RefCounted is an abstract class and subclasses should override this method
*/
unsigned long RefCounted::Release()
{
    return 0;
}

/* -------------------------------------------------------------------- Equal */
/*
    This is a very un-intelligent default Equal routine.  It is most likely
    that subclasses will want to override this method.
*/
bool RefCounted::Equal(RefCounted *other)
{
    return other == this;
}

/* ---------------------------------------------------------- CopyDescription */
/*
    This is a pretty un-intelligent copy description routine.  Subclasses
    should override this to provide more interesting behavior.
*/
CFStringRef RefCounted::CopyDescription()
{
    return CFStringCreateWithFormat(NULL, 0, CFSTR("RefCounted : %x"), (unsigned long) this);
}

/* --------------------------------------------------------------------- Hash */
/*
    Pretty efficient hash algorithm, but may not provide for the
    efficient operation of algorithms that use hash codes so subclasses should
    probably override this method as well.
*/
CFHashCode RefCounted::Hash()
{
    return reinterpret_cast<CFHashCode>(this); 
}

/* -------------------------------------------------------------- GetRefCount */
/*
    RefCounted is an abstract class and subclasses should override this method.
*/
long RefCounted::GetRefCount() const
{
    return 0;
}

/* --------------------------------------------------------- CFRetainCallBack */
const void *RefCounted::CFRetainCallBack(
    CFAllocatorRef /* allocator */, 
    const void *value)
{
    RefCounted *itemToRetain = reinterpret_cast<RefCounted *>((void *)value);
    itemToRetain->Retain();
    
    return itemToRetain;
}

/* -------------------------------------------------------- CFReleaseCallBack */
void RefCounted::CFReleaseCallBack(
    CFAllocatorRef /* allocator */, 
    const void *value)
{
    RefCounted *itemToRelease = reinterpret_cast<RefCounted *>((void *)value);
    itemToRelease->Release();
}

/* ------------------------------------------------ CFCopyDescriptionCallBack */
CFStringRef RefCounted::CFCopyDescriptionCallBack(const void *value)
{
    RefCounted *itemToDescribe = reinterpret_cast<RefCounted *>((void *)value);
    return itemToDescribe->CopyDescription();
}

/* ---------------------------------------------------------- CFEqualCallBack */
Boolean RefCounted::CFEqualCallBack(const void *value1, const void *value2)
{
    RefCounted *itemToCompare1 = reinterpret_cast<RefCounted *>((void *)value1);
    RefCounted *itemToCompare2 = reinterpret_cast<RefCounted *>((void *)value2);
    return itemToCompare1->Equal(itemToCompare2);
}

/* ----------------------------------------------------------- CFHashCallBack */
CFHashCode RefCounted::CFHashCallBack(const void *value)
{
    RefCounted *itemToHash = reinterpret_cast<RefCounted *>((void *)value);
    return itemToHash->Hash();
}

#pragma mark __ Concrete Ref Count __

/* --------------------------------------------------------- ConcreteRefCount */
ConcreteRefCount::ConcreteRefCount() :
    mRefCount(1)
{
}

/* --------------------------------------------------------- ConcreteRefCount */
ConcreteRefCount::ConcreteRefCount(const ConcreteRefCount &itemToCopy) :
    mRefCount(itemToCopy.GetRefCount())
{
}

/* -------------------------------------------------------- ~ConcreteRefCount */
ConcreteRefCount::~ConcreteRefCount()
{
}

/* ------------------------------------------------------------------- Retain */
unsigned long ConcreteRefCount::Retain()
{
    return (unsigned long) ++mRefCount;
}

/* ------------------------------------------------------------------ Release */
unsigned long ConcreteRefCount::Release()
{
    if(--mRefCount <= 0) {
        delete this;
    }
    
    return (unsigned long) mRefCount;
}

/* -------------------------------------------------------------- GetRefCount */
long ConcreteRefCount::GetRefCount() const
{
    return mRefCount;
}
