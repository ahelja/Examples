/*	Copyright: 	� Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
/*==================================================================================================
	CASettingsStorage.cpp

==================================================================================================*/

//==================================================================================================
//	Includes
//==================================================================================================

//	Self Include
#include "CASettingsStorage.h"

//	PublicUtility Includes
#include "CAAutoDisposer.h"
#include "CACFData.h"
#include "CACFNumber.h"

//	Stamdard Library Includes
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

//==================================================================================================
//	CASettingsStorage
//==================================================================================================

CASettingsStorage::CASettingsStorage(const char* inSettingsFilePath)
{
	UInt32 theLength = strlen(inSettingsFilePath);
	mSettingsFilePath = new char[theLength + 2];
	strcpy(mSettingsFilePath, inSettingsFilePath);
	
	mSettingsCache = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	mSettingsCacheTime.tv_sec = 0;
	mSettingsCacheTime.tv_nsec = 0;
}

CASettingsStorage::~CASettingsStorage()
{
	delete[] mSettingsFilePath;
	
	if(mSettingsCache != NULL)
	{
		CFRelease(mSettingsCache);
	}
}

void	CASettingsStorage::CopyBoolValue(CFStringRef inKey, bool& outValue, bool inDefaultValue) const
{
	//	initialize the return value
	outValue = inDefaultValue;
	
	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, NULL);
	
	//	for this type, NULL is an invalid value
	if(theValue != NULL)
	{
		//	bools can be made from either CFBooleans or CFNumbers
		if(CFGetTypeID(theValue) == CFBooleanGetTypeID())
		{
			//	get the return value from the CF object
			outValue = CFBooleanGetValue(static_cast<CFBooleanRef>(theValue));
		}
		else if(CFGetTypeID(theValue) == CFNumberGetTypeID())
		{
			//	get the numeric value
			SInt32 theNumericValue = 0;
			CFNumberGetValue(static_cast<CFNumberRef>(theValue), kCFNumberSInt32Type, &theNumericValue);
			
			//	non-zero indicates true
			outValue = theNumericValue != 0;
		}
		
		//	release the value since we aren't returning it
		CFRelease(theValue);
	}
}

void	CASettingsStorage::CopySInt32Value(CFStringRef inKey, SInt32& outValue, SInt32 inDefaultValue) const
{
	//	initialize the return value
	outValue = inDefaultValue;
	
	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, NULL);
	
	//	for this type, NULL is an invalid value
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFNumberGetTypeID())
		{
			//	get the return value from the CF object
			CFNumberGetValue(static_cast<CFNumberRef>(theValue), kCFNumberSInt32Type, &outValue);
		}
		
		//	release the value since we aren't returning it
		CFRelease(theValue);
	}
}

void	CASettingsStorage::CopyUInt32Value(CFStringRef inKey, UInt32& outValue, UInt32 inDefaultValue) const
{
	//	initialize the return value
	outValue = inDefaultValue;
	
	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, NULL);
	
	//	for this type, NULL is an invalid value
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFNumberGetTypeID())
		{
			//	get the return value from the CF object
			CFNumberGetValue(static_cast<CFNumberRef>(theValue), kCFNumberSInt32Type, &outValue);
		}
		
		//	release the value since we aren't returning it
		CFRelease(theValue);
	}
}

void	CASettingsStorage::CopySInt64Value(CFStringRef inKey, SInt64& outValue, SInt64 inDefaultValue) const
{
	//	initialize the return value
	outValue = inDefaultValue;
	
	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, NULL);
	
	//	for this type, NULL is an invalid value
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFNumberGetTypeID())
		{
			//	get the return value from the CF object
			CFNumberGetValue(static_cast<CFNumberRef>(theValue), kCFNumberSInt64Type, &outValue);
		}
		
		//	release the value since we aren't returning it
		CFRelease(theValue);
	}
}

void	CASettingsStorage::CopyUInt64Value(CFStringRef inKey, UInt64& outValue, UInt64 inDefaultValue) const
{
	//	initialize the return value
	outValue = inDefaultValue;
	
	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, NULL);
	
	//	for this type, NULL is an invalid value
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFNumberGetTypeID())
		{
			//	get the return value from the CF object
			CFNumberGetValue(static_cast<CFNumberRef>(theValue), kCFNumberSInt64Type, &outValue);
		}
		
		//	release the value since we aren't returning it
		CFRelease(theValue);
	}
}

void	CASettingsStorage::CopyFloat32Value(CFStringRef inKey, Float32& outValue, Float32 inDefaultValue) const
{
	//	initialize the return value
	outValue = inDefaultValue;
	
	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, NULL);
	
	//	for this type, NULL is an invalid value
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFNumberGetTypeID())
		{
			//	get the return value from the CF object
			CFNumberGetValue(static_cast<CFNumberRef>(theValue), kCFNumberFloat32Type, &outValue);
		}
		
		//	release the value since we aren't returning it
		CFRelease(theValue);
	}
}

void	CASettingsStorage::CopyFloat64Value(CFStringRef inKey, Float64& outValue, Float64 inDefaultValue) const
{
	//	initialize the return value
	outValue = inDefaultValue;
	
	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, NULL);
	
	//	for this type, NULL is an invalid value
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFNumberGetTypeID())
		{
			//	get the return value from the CF object
			CFNumberGetValue(static_cast<CFNumberRef>(theValue), kCFNumberFloat64Type, &outValue);
		}
		
		//	release the value since we aren't returning it
		CFRelease(theValue);
	}
}

void	CASettingsStorage::CopyNumberValue(CFStringRef inKey, CFNumberRef& outValue, CFNumberRef inDefaultValue) const
{
	//	initialize the return value
	outValue = NULL;

	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, inDefaultValue);
	
	//	for this type, NULL is a valid value, but requires less work
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFNumberGetTypeID())
		{
			//	set the return value to the CF object we are returning
			outValue = static_cast<CFNumberRef>(theValue);
		}
		else
		{
			//	release the value since we aren't returning it
			CFRelease(theValue);
			
			//	set the return value to the default value
			outValue = inDefaultValue;
			
			//	and retain it
			CFRetain(outValue);
		}
	}
}
	
void	CASettingsStorage::CopyStringValue(CFStringRef inKey, CFStringRef& outValue, CFStringRef inDefaultValue) const
{
	//	initialize the return value
	outValue = NULL;

	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, inDefaultValue);
	
	//	for this type, NULL is a valid value, but requires less work
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFStringGetTypeID())
		{
			//	set the return value to the CF object we are returning
			outValue = static_cast<CFStringRef>(theValue);
		}
		else
		{
			//	release the value since we aren't returning it
			CFRelease(theValue);
			
			//	set the return value to the default value
			outValue = inDefaultValue;
			
			//	and retain it
			CFRetain(outValue);
		}
	}
}
	
void	CASettingsStorage::CopyArrayValue(CFStringRef inKey, CFArrayRef& outValue, CFArrayRef inDefaultValue) const
{
	//	initialize the return value
	outValue = NULL;

	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, inDefaultValue);
	
	//	for this type, NULL is a valid value, but requires less work
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFArrayGetTypeID())
		{
			//	set the return value to the CF object we are returning
			outValue = static_cast<CFArrayRef>(theValue);
		}
		else
		{
			//	release the value since we aren't returning it
			CFRelease(theValue);
			
			//	set the return value to the default value
			outValue = inDefaultValue;
			
			//	and retain it
			CFRetain(outValue);
		}
	}
}
	
void	CASettingsStorage::CopyDictionaryValue(CFStringRef inKey, CFDictionaryRef& outValue, CFDictionaryRef inDefaultValue) const
{
	//	initialize the return value
	outValue = NULL;

	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, inDefaultValue);
	
	//	for this type, NULL is a valid value, but requires less work
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFDictionaryGetTypeID())
		{
			//	set the return value to the CF object we are returning
			outValue = static_cast<CFDictionaryRef>(theValue);
		}
		else
		{
			//	release the value since we aren't returning it
			CFRelease(theValue);
			
			//	set the return value to the default value
			outValue = inDefaultValue;
			
			//	and retain it
			CFRetain(outValue);
		}
	}
}
	
void	CASettingsStorage::CopyDataValue(CFStringRef inKey, CFDataRef& outValue, CFDataRef inDefaultValue) const
{
	//	initialize the return value
	outValue = NULL;

	//	get the raw value
	CFTypeRef theValue = NULL;
	CopyCFTypeValue(inKey, theValue, inDefaultValue);
	
	//	for this type, NULL is a valid value, but requires less work
	if(theValue != NULL)
	{
		//	make sure we are dealing with the right kind of CF object
		if(CFGetTypeID(theValue) == CFDataGetTypeID())
		{
			//	set the return value to the CF object we are returning
			outValue = static_cast<CFDataRef>(theValue);
		}
		else
		{
			//	release the value since we aren't returning it
			CFRelease(theValue);
			
			//	set the return value to the default value
			outValue = inDefaultValue;
			
			//	and retain it
			CFRetain(outValue);
		}
	}
}

void	CASettingsStorage::CopyCFTypeValue(CFStringRef inKey, CFTypeRef& outValue, CFTypeRef inDefaultValue) const
{
	//	make sure our cache is up to date
	const_cast<CASettingsStorage*>(this)->RefreshSettings();

	//	check to see if we have a value for the given key
	if(!CFDictionaryGetValueIfPresent(mSettingsCache, inKey, &outValue))
	{
		//	the key wasn't in the cache, so return the default value
		outValue = inDefaultValue;
	}
		
	//	make sure we retain the return value
	if(outValue != NULL)
	{
		CFRetain(outValue);
	}
}

void	CASettingsStorage::SetSInt32Value(CFStringRef inKey, SInt32 inValue)
{
	CACFNumber theValue(inValue);
	SetCFTypeValue(inKey, theValue.GetCFNumber());
}

void	CASettingsStorage::SetUInt32Value(CFStringRef inKey, UInt32 inValue)
{
	CACFNumber theValue(inValue);
	SetCFTypeValue(inKey, theValue.GetCFNumber());
}

void	CASettingsStorage::SetSInt64Value(CFStringRef inKey, SInt64 inValue)
{
	CACFNumber theValue(inValue);
	SetCFTypeValue(inKey, theValue.GetCFNumber());
}

void	CASettingsStorage::SetUInt64Value(CFStringRef inKey, UInt64 inValue)
{
	CACFNumber theValue(inValue);
	SetCFTypeValue(inKey, theValue.GetCFNumber());
}

void	CASettingsStorage::SetFloat32Value(CFStringRef inKey, Float32 inValue)
{
	CACFNumber theValue(inValue);
	SetCFTypeValue(inKey, theValue.GetCFNumber());
}

void	CASettingsStorage::SetFloat64Value(CFStringRef inKey, Float64 inValue)
{
	CACFNumber theValue(inValue);
	SetCFTypeValue(inKey, theValue.GetCFNumber());
}

void	CASettingsStorage::SetNumberValue(CFStringRef inKey, CFNumberRef inValue)
{
	SetCFTypeValue(inKey, inValue);
}

void	CASettingsStorage::SetStringValue(CFStringRef inKey, CFStringRef inValue)
{
	SetCFTypeValue(inKey, inValue);
}

void	CASettingsStorage::SetArrayValue(CFStringRef inKey, CFArrayRef inValue)
{
	SetCFTypeValue(inKey, inValue);
}

void	CASettingsStorage::SetDictionaryValue(CFStringRef inKey, CFDictionaryRef inValue)
{
	SetCFTypeValue(inKey, inValue);
}

void	CASettingsStorage::SetDataValue(CFStringRef inKey, CFDataRef inValue)
{
	SetCFTypeValue(inKey, inValue);
}

void	CASettingsStorage::SetCFTypeValue(CFStringRef inKey, CFTypeRef inValue)
{
	//	make sure our cache is up to date
	RefreshSettings();
	
	//	add the new key/value to the dictionary
	CFDictionarySetValue(mSettingsCache, inKey, inValue);
	
	//	write the settings to the file
	SaveSettings();
}

void	CASettingsStorage::RemoveValue(CFStringRef inKey)
{
	//	make sure our cache is up to date
	RefreshSettings();
	
	//	remove the given key
	CFDictionaryRemoveValue(mSettingsCache, inKey);
	
	//	write the settings to the file
	SaveSettings();
}

void	CASettingsStorage::RemoveAllValues()
{
	//	make sure our cache is up to date
	RefreshSettings();
	
	//	remove the given key
	CFDictionaryRemoveAllValues(mSettingsCache);
	
	//	write the settings to the file
	SaveSettings();
}

void	CASettingsStorage::SendNotification(CFStringRef inName, CFDictionaryRef inData, bool inPostToAllSessions) const
{
	//	initialize the flags for the message
	CFOptionFlags theFlags = kCFNotificationDeliverImmediately;
	if(inPostToAllSessions)
	{
		theFlags += kCFNotificationPostToAllSessions;
	}
	
	//	send the message
	CFNotificationCenterPostNotificationWithOptions(CFNotificationCenterGetDistributedCenter(), inName, NULL, inData, theFlags);
}

inline bool	operator<(const struct timespec& inX, const struct timespec& inY)
{
	return ((inX.tv_sec < inY.tv_sec) || ((inX.tv_sec == inY.tv_sec) && (inX.tv_nsec < inY.tv_nsec)));
}

void	CASettingsStorage::RefreshSettings()
{
	//	first, we need to stat the file to check the mod date, this has the side effect of also
	//	telling us if the file exisits
	struct stat theFileInfo;
	int theStatError = stat(mSettingsFilePath, &theFileInfo);
	
	//	we use this boolean to make error recovery easier since we need a case for when there's no file anyway
	bool theSettingsWereCached = false;
	
	if(theStatError == 0)
	{
		//	stat says there is something there, only have to do work if we either don't have a cache or the cache is out of date
		if((mSettingsCache == NULL) || (mSettingsCacheTime < theFileInfo.st_mtimespec))
		{
			//	open the file
			FILE* theFile = fopen(mSettingsFilePath, "r");
			if(theFile != NULL)
			{
				//	lock the file (this call blocks until the lock is taken)
				int theError = flock(fileno(theFile), LOCK_EX);
				if(theError == 0)
				{
					//	get the length of the file
					fseek(theFile, 0, SEEK_END);
					UInt32 theFileLength = ftell(theFile);
					fseek(theFile, 0, SEEK_SET);
					
					if(theFileLength > 0)
					{
						//	allocate a block of memory to hold the data in the file
						CAAutoFree<Byte> theRawFileData(theFileLength);
						
						//	read all the data in
						fread(static_cast<Byte*>(theRawFileData), theFileLength, 1, theFile);
						
						//	release the lock
						flock(fileno(theFile), LOCK_UN);
						
						//	put it into a CFData object
						CACFData theRawFileDataCFData(static_cast<Byte*>(theRawFileData), theFileLength);
						
						//	get rid of the existing cache
						if(mSettingsCache != NULL)
						{
							CFRelease(mSettingsCache);
							mSettingsCache = NULL;
						}
						
						//	parse the data as a property list
						mSettingsCache = (CFMutableDictionaryRef)CFPropertyListCreateFromXMLData(NULL, theRawFileDataCFData.GetCFData(), kCFPropertyListMutableContainersAndLeaves, NULL);
						
						//	save the date of the cache
						mSettingsCacheTime = theFileInfo.st_mtimespec;
						
						//	mark that we're done
						theSettingsWereCached = true;
					}
				}
				
				//	close the file
				fclose(theFile);
			}
		}
	}
	
	if(!theSettingsWereCached && (theFileInfo.st_mtimespec < mSettingsCacheTime))
	{
		//	we get here if either there isn't a file or something wacky happenned while parsing it
		//	all we do here is make sure that the member variables are set to their initial values
		if(mSettingsCache != NULL)
		{
			CFRelease(mSettingsCache);
			mSettingsCache = NULL;
		}
		mSettingsCache = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		
		mSettingsCacheTime.tv_sec = 0;
		mSettingsCacheTime.tv_nsec = 0;
	}
}

void	CASettingsStorage::SaveSettings()
{
	if(mSettingsCache != NULL)
	{
		//	make a CFData that contains the new settings
		CACFData theNewRawPrefsCFData(CFPropertyListCreateXMLData(NULL, (CFPropertyListRef)mSettingsCache), true);
		
		//	open the file for writing
		FILE* theFile = fopen(mSettingsFilePath, "w+");
		if(theFile != NULL)
		{
			//	lock the file (this call blocks until the lock is taken)
			int theError = flock(fileno(theFile), LOCK_EX);
			if(theError == 0)
			{
				//	write the data
				fwrite(theNewRawPrefsCFData.GetDataPtr(), theNewRawPrefsCFData.GetSize(), 1, theFile);
				
				//	release the lock
				flock(fileno(theFile), LOCK_UN);
			
				//	close the file
				fclose(theFile);
				
				//	stat the file to get the mod date
				struct stat theFileInfo;
				stat(mSettingsFilePath, &theFileInfo);
				
				//	save the mod date
				mSettingsCacheTime = theFileInfo.st_mtimespec;
			}
			else
			{
				//	close the file
				fclose(theFile);
			}
		}
	}
}
