/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
	HLAudioObjectBrowserWindowController.mm

==================================================================================================*/

//==================================================================================================
//	Includes
//==================================================================================================

//	Self Include
#import "HLAudioObjectBrowserWindowController.h"

//	Local Includes
#include "HLApplicationDelegate.h"

//	PublicUtility Includes
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAHALAudioObject.h"

//==================================================================================================
//	HLAudioObjectBrowserItem
//==================================================================================================

@interface HLAudioObjectBrowserItem : NSObject
{

@public
	AudioObjectID	mAudioObjectID;

}

-(id)				initWithAudioObjectID:	(AudioObjectID)inAudioObjectID;
-(AudioObjectID)	GetAudioObjectID;

@end

@implementation HLAudioObjectBrowserItem

-(id)	initWithAudioObjectID:	(AudioObjectID)inAudioObjectID
{
	mAudioObjectID = inAudioObjectID;
	
	return [super init];
}

-(AudioObjectID)	GetAudioObjectID
{
	return mAudioObjectID;
}

@end

//==================================================================================================
//	HLAudioObjectBrowserItemTracker
//==================================================================================================

struct	HLAudioObjectBrowserItemTracker
{

	AudioObjectID				mAudioObjectID;
	HLAudioObjectBrowserItem*	mItem;
	
	HLAudioObjectBrowserItemTracker(HLAudioObjectBrowserItem* inItem) : mAudioObjectID(inItem->mAudioObjectID), mItem(inItem) {}
	HLAudioObjectBrowserItemTracker(AudioObjectID inAudioObjectID) : mAudioObjectID(inAudioObjectID), mItem(NULL) {}
	HLAudioObjectBrowserItemTracker(const HLAudioObjectBrowserItemTracker& inTracker) : mAudioObjectID(inTracker.mAudioObjectID), mItem(inTracker.mItem) { if(mItem != NULL) { [mItem retain]; } }
	HLAudioObjectBrowserItemTracker&	operator=(const HLAudioObjectBrowserItemTracker& inTracker) { if(mItem != NULL) { [mItem release]; } mAudioObjectID = inTracker.mAudioObjectID; mItem = inTracker.mItem; if(mItem != NULL) { [mItem retain]; } return *this; }
	~HLAudioObjectBrowserItemTracker() { if(mItem != NULL) { [mItem release]; } }

};

inline bool	operator<(const HLAudioObjectBrowserItemTracker& x, const HLAudioObjectBrowserItemTracker& y) { return  x.mAudioObjectID < y.mAudioObjectID; }
inline bool	operator==(const HLAudioObjectBrowserItemTracker& x, const HLAudioObjectBrowserItemTracker& y) { return x.mAudioObjectID == y.mAudioObjectID; }
inline bool	operator!=(const HLAudioObjectBrowserItemTracker& x, const HLAudioObjectBrowserItemTracker& y) { return !(x == y); }
inline bool	operator<=(const HLAudioObjectBrowserItemTracker& x, const HLAudioObjectBrowserItemTracker& y) { return (x < y) || (x == y); }
inline bool	operator>=(const HLAudioObjectBrowserItemTracker& x, const HLAudioObjectBrowserItemTracker& y) { return !(x < y); }
inline bool	operator>(const HLAudioObjectBrowserItemTracker& x, const HLAudioObjectBrowserItemTracker& y) { return !((x < y) || (x == y)); }

typedef std::set<HLAudioObjectBrowserItemTracker>	HLAudioObjectBrowserItemList;

//==================================================================================================
//	HLAudioObjectBrowserWindowController
//==================================================================================================

@implementation HLAudioObjectBrowserWindowController

-(id)	initWithApplicationDelegate:	(HLApplicationDelegate*)inApplicationDelegate
{
	CATry;
	
	//	initialize the super class
    [super initWithWindowNibName: @"AudioObjectBrowserWindow"];
	
	//	initialize the basic stuff
	mApplicationDelegate = inApplicationDelegate;
	mAudioObjectBrowserItems = new HLAudioObjectBrowserItemList();
	
	CACatch;
	
	return self;
}

-(void)	windowDidLoad
{
	CATry;
	
	CACatch;
}

-(void)	windowWillClose:	(NSNotification*)inNotification
{
	//	the window is closing, so arrange to get cleaned up
	[mApplicationDelegate DestroyAudioObjectBrowserWindow: self];
}

-(void)	dealloc
{
	CATry;
	
	delete mAudioObjectBrowserItems;
	
	CACatch;

	[super dealloc];
}

-(int)	outlineView:			(NSOutlineView*)inOutlineView
		numberOfChildrenOfItem:	(id)inItem
{
	int theAnswer = 0;
	
	CATry;
	
	if(inItem != NULL)
	{
		//	get the object ID
		AudioObjectID theObjectID = [inItem GetAudioObjectID];
	
		//	figure out how many children this object has
		CAHALAudioObject theAudioObject(theObjectID);
		theAnswer = theAudioObject.GetNumberOwnedObjects(0);
	}
	else
	{
		//	the root object contains just the system object
		theAnswer = 1;
	}
	
	CACatch;

	return theAnswer;
}

-(BOOL)	outlineView:		(NSOutlineView*)inOutlineView
		isItemExpandable:	(id)inItem
{
	BOOL theAnswer = NO;
	
	CATry;
	
	if(inItem != NULL)
	{
		//	get the object ID
		AudioObjectID theObjectID = [inItem GetAudioObjectID];
	
		//	figure out how many children this item has
		CAHALAudioObject theAudioObject(theObjectID);
		UInt32 theNumberObjects = theAudioObject.GetNumberOwnedObjects(0);
		
		//	the item is expandible if the number of owned objects is greater than 0
		if(theNumberObjects > 0)
		{
			theAnswer = YES;
		}
	}
	else
	{
		//	the root object contains just the system object
		theAnswer = YES;
	}
	
	CACatch;

	return theAnswer;
}

-(id)	outlineView:	(NSOutlineView*)inOutlineView
		child:			(int)inIndex
		ofItem:			(id)inItem
{
	HLAudioObjectBrowserItem* theAnswer = NULL;
	
	CATry;
	
	//	figure out the object ID of the child object
	AudioObjectID theChildObjectID = 0;
	if(inItem != NULL)
	{
		//	figure out how many children this item has
		AudioObjectID theObjectID = [inItem GetAudioObjectID];
		CAHALAudioObject theAudioObject(theObjectID);
		UInt32 theNumberObjects = theAudioObject.GetNumberOwnedObjects(0);

		if((theNumberObjects > 0) && (static_cast<UInt32>(inIndex) < theNumberObjects))
		{
			//	allocate the space for the children
			CAAutoArrayDelete<AudioObjectID> theObjectList(theNumberObjects);
			
			//	get their IDs
			theAudioObject.GetAllOwnedObjects(0, theNumberObjects, theObjectList);
			
			//	sort the list
			AudioObjectID* theFirstItem = &(theObjectList[0]);
			AudioObjectID* theLastItem = theFirstItem + theNumberObjects;
			std::sort(theFirstItem, theLastItem);
			
			//	get the object ID at the requested index
			theChildObjectID = theObjectList[inIndex];
		}
	}
	else
	{
		//	the root object contains just the system object
		Assert(inIndex == 0, "(HLAudioObjectBrowserWindowController) -outlineView:child:ofItem: index other than 0 requested of the root object");
		theChildObjectID = kAudioObjectSystemObject;
	}
	
	//	look to see if we already have an item for this ID
	HLAudioObjectBrowserItemList::iterator theIterator = mAudioObjectBrowserItems->find(HLAudioObjectBrowserItemTracker(theChildObjectID));
	if(theIterator != mAudioObjectBrowserItems->end())
	{
		//	we do, so return it
		theAnswer = theIterator->mItem;
	}
	else
	{
		//	we don't, so make a new one
		theAnswer = [[HLAudioObjectBrowserItem alloc] initWithAudioObjectID: theChildObjectID];
		
		//	and stick it in the tracking list
		mAudioObjectBrowserItems->insert(HLAudioObjectBrowserItemTracker(theAnswer));
	}
	
	CACatch;

	return theAnswer;
}

-(id)	outlineView:				(NSOutlineView*)inOutlineView
		objectValueForTableColumn:	(NSTableColumn*)inTableColumn
		byItem:						(id)inItem
{
	id theAnswer = NULL;

	CATry;
	
	if(inItem != NULL)
	{
		//	get the object
		AudioObjectID theObjectID = [inItem GetAudioObjectID];
		CAHALAudioObject theObject(theObjectID);
		
		//	get it's name
		NSString* theObjectName = (NSString*)theObject.CopyName();
		if(theObjectName != NULL)
		{
			[theObjectName autorelease];
		}
		
		//	use it to make a display name that incorporates other specific info about the object
		
		//	get the class ID
		AudioClassID theObjectClassID = theObject.GetClassID();
		
		//	make a name based on the class
		char theTempString[5];
		AudioObjectPropertyAddress theAddress;
		UInt32 theSize;
		AudioObjectPropertyScope theScope;
		const char* theScopeString;
		AudioObjectPropertyElement theElement;
		switch(theObjectClassID)
		{
			//	the system object
			case kAudioSystemObjectClassID:
				//	use the given name or the constant
				if(theObjectName != NULL)
				{
					theAnswer = theObjectName;
				}
				else
				{
					theAnswer = @"System Object";
				}
				break;
			
			//	a plug-in object
			case kAudioPlugInClassID:
				{
					//	get it's bundle ID
					theAddress.mSelector = kAudioPlugInPropertyBundleID;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(UInt32);
					CFStringRef theBundleID = NULL;
					theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theBundleID);
					
					//	make a name with it
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (Plug-in: %@)", theObjectName, theBundleID];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"Plug-in: %@", theBundleID];
					}
					
					//	release the bundle ID
					CFRelease(theBundleID);
				}
				break;
			
			//	a device object
			case kAudioDeviceClassID:
				if(theObjectName != NULL)
				{
					theAnswer = [NSString stringWithFormat: @"%@ (Device: %d)", theObjectName, theObjectID];
				}
				else
				{
					theAnswer = [NSString stringWithFormat: @"Device: %d", theObjectID];
				}
				break;
			
			//	a stream object
			case kAudioStreamClassID:
				{
					//	get it's starting channel number
					theAddress.mSelector = kAudioStreamPropertyStartingChannel;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(UInt32);
					UInt32 theStartingChannel = 0;
					theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theStartingChannel);
					
					//	get it's direction
					theAddress.mSelector = kAudioStreamPropertyDirection;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(UInt32);
					UInt32 theDirection = 0;
					theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theDirection);
					
					//	make a name with it
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Stream: %d)", theObjectName, ((theDirection != 0) ? "Input" : "Output"), theStartingChannel];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Stream: %d", ((theDirection != 0) ? "Input" : "Output"), theStartingChannel];
					}
				}
				break;
			
			//	an aggregate devce
			case kAudioAggregateDeviceClassID:
				if(theObjectName != NULL)
				{
					theAnswer = [NSString stringWithFormat: @"%@ (Aggregate: %d)", theObjectName, theObjectID];
				}
				else
				{
					theAnswer = [NSString stringWithFormat: @"Aggregate: %d", theObjectID];
				}
				break;
			
			//	a sub-device of an aggregate
			case kAudioSubDeviceClassID:
				if(theObjectName != NULL)
				{
					theAnswer = [NSString stringWithFormat: @"%@ (Sub-device: %d)", theObjectName, theObjectID];
				}
				else
				{
					theAnswer = [NSString stringWithFormat: @"Sub-device: %d", theObjectID];
				}
				break;
			
			//	a general control
			case kAudioControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioLevelControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Level Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Level Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioVolumeControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Volume Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Volume Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioLFEVolumeControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s LFE Volume Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s LFE Volume Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioBootChimeVolumeControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Boot Chime Volume Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Boot Chime Volume Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioBooleanControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Boolean Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Boolean Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioMuteControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Mute Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Mute Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioSoloControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Solo Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Solo Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioJackControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Jack Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Jack Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioLFEMuteControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s LFE Mute Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s LFE Mute Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioISubOwnerControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s iSub Onwer Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s iSub Owner Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioSelectorControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Selector Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Selector Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioDataSourceControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Data Source Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Data Source Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioDataDestinationControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Data Destination Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Data Destination Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioClockSourceControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Clock Source Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Clock Source Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioLineLevelControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Line Level Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Line Level Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			case kAudioStereoPanControlClassID:
				{
					//	get the scope
					theAddress.mSelector = kAudioControlPropertyScope;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyScope);
					theScope = kAudioObjectPropertyScopeGlobal;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theScope);
					}
					catch(...)
					{
						theScope = kAudioObjectPropertyScopeGlobal;
					}
					
					//	make a string for the scope
					switch(theScope)
					{
						case kAudioObjectPropertyScopeGlobal:
							theScopeString = "Global";
							break;
						
						case kAudioDevicePropertyScopeInput:
							theScopeString = "Input";
							break;
						
						case kAudioDevicePropertyScopeOutput:
							theScopeString = "Output";
							break;
						
						case kAudioDevicePropertyScopePlayThrough:
							theScopeString = "Monitor";
							break;
						
						default:
							theTempString[0] = ((char*)&theScope)[0];
							theTempString[1] = ((char*)&theScope)[1];
							theTempString[2] = ((char*)&theScope)[2];
							theTempString[3] = ((char*)&theScope)[3];
							theTempString[4] = 0;
							theScopeString = theTempString;
							break;
					};
					
					//	get the element
					theAddress.mSelector = kAudioControlPropertyElement;
					theAddress.mScope = kAudioObjectPropertyScopeGlobal;
					theAddress.mElement = kAudioObjectPropertyElementMaster;
					theSize = sizeof(AudioObjectPropertyElement);
					theElement = kAudioObjectPropertyElementMaster;
					try
					{
						theObject.GetPropertyData(theAddress, 0, NULL, theSize, &theElement);
					}
					catch(...)
					{
						theElement = kAudioObjectPropertyElementMaster;
					}
					
					//	make a name
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ (%s Stereo Pan Control: %lu)", theObjectName, theScopeString, theElement];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"%s Stereo Pan Control: %lu", theScopeString, theElement];
					}
				}
				break;
			
			default:
				{
					//	get the class ID as a string
					char theObjectClassIDString[] = CA4CCToCString(theObjectClassID);
					
					//	build a name for this object
					if(theObjectName != NULL)
					{
						theAnswer = [NSString stringWithFormat: @"%@ ('%s' %d)", theObjectName, theObjectClassIDString, theObjectID];
					}
					else
					{
						theAnswer = [NSString stringWithFormat: @"'%s' %d", theObjectClassIDString, theObjectID];
					}
				}
				break;
		};
	}
	else
	{
		//	it's the system object
		theAnswer = @"root";
	}
	
	CACatch;
	
	return theAnswer;
}

@end
