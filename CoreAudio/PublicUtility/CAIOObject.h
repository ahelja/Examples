/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

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
/*=============================================================================
	CAIOObject.h

=============================================================================*/
#if !defined(__CAIOObject_h__)
#define __CAIOObject_h__

//=============================================================================
//	Includes
//=============================================================================

//	System Includes
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

#if !defined(IO_OBJECT_NULL)
	#define	IO_OBJECT_NULL	((io_object_t) 0)
#endif

//=============================================================================
//	CAIOObject
//=============================================================================

class CAIOObject
{

//	Construction/Destruction
public:
					CAIOObject() : mIOObject(IO_OBJECT_NULL), mWillRelease(true) {}
					CAIOObject(io_object_t inIOObject, bool inWillRelease = true) : mIOObject(inIOObject), mWillRelease(inWillRelease) {}
					~CAIOObject() { Release(); }
					CAIOObject(const CAIOObject& inObject) : mIOObject(inObject.mIOObject), mWillRelease(inObject.mWillRelease) { Retain(); }
	CAIOObject&		operator=(const CAIOObject& inObject) { Release(); mIOObject = inObject.mIOObject; mWillRelease = inObject.mWillRelease; Retain(); return *this; }
	CAIOObject&		operator=(io_object_t inIOObject) { Release(); mIOObject = inIOObject; mWillRelease = true; return *this; }

private:
	void			Retain() { if(mWillRelease && (mIOObject != IO_OBJECT_NULL)) { IOObjectRetain(mIOObject); } }
	void			Release() { if(mWillRelease && (mIOObject != IO_OBJECT_NULL)) { IOObjectRelease(mIOObject); mIOObject = IO_OBJECT_NULL; } }
	
	io_object_t		mIOObject;
	bool			mWillRelease;

//	Operations
public:
	void			AllowRelease() { mWillRelease = true; }
	void			DontAllowRelease() { mWillRelease = false; }
	bool			IsValid() const { return mIOObject != IO_OBJECT_NULL; }
	bool			IsEqual(io_object_t inIOObject) { return IOObjectIsEqualTo(inIOObject, mIOObject); }
	bool			ConformsTo(const io_name_t inClassName) { return IOObjectConformsTo(mIOObject, inClassName); }
	CFTypeRef		GetCFProperty(CFStringRef inKey) const { return IORegistryEntryCreateCFProperty(mIOObject, inKey, NULL, 0); }
	CFNumberRef		GetCFNumberProperty(CFStringRef inKey) const { return static_cast<CFNumberRef>(GetCFProperty(inKey)); }
	CFStringRef		GetCFStringProperty(CFStringRef inKey) const { return static_cast<CFStringRef>(GetCFProperty(inKey)); }

//	Value Access
public:
	io_object_t		GetIOObject() const { return mIOObject; }
	io_object_t		CopyIOObject() const { if(mIOObject != IO_OBJECT_NULL) { IOObjectRetain(mIOObject); } return mIOObject; }

};

#endif
