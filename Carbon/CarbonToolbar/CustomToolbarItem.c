/*
    File:		CustomToolbarItem.c
    
    Version:	Mac OS X

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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include "CustomToolbarItem.h"

const EventTypeSpec kEvents[] = 
{
	{ kEventClassHIObject, kEventHIObjectConstruct },
	{ kEventClassHIObject, kEventHIObjectInitialize },
	{ kEventClassHIObject, kEventHIObjectDestruct },
	
	{ kEventClassToolbarItem, kEventToolbarItemGetPersistentData },
	{ kEventClassToolbarItem, kEventToolbarItemPerformAction },
	{ kEventClassToolbarItem, kEventToolbarItemCreateCustomView }
};

struct CustomToolbarItem
{
	HIToolbarItemRef		toolbarItem;
	CFStringRef				identifier;		// which type of custom toolbar item are we?
	CFTypeRef				configData;		// a CFURLRef for the URL toolbar item and a CFStringRef for the search toolbar item
};
typedef struct CustomToolbarItem CustomToolbarItem;

#pragma mark -

//-----------------------------------------------------------------------------
//	ConstructCustomToolbarItem
//-----------------------------------------------------------------------------
//	Create a new custom toolbar item with no config data yet.
//
static OSStatus
ConstructCustomToolbarItem( HIToolbarItemRef inItem, CustomToolbarItem** outItem )
{
	CustomToolbarItem*		item;
	OSStatus				err = noErr;
	
	item = (CustomToolbarItem*)malloc( sizeof( CustomToolbarItem ) );
	require_action( item != NULL, CantAllocItem, err = memFullErr );
	
	item->toolbarItem = inItem;
	item->identifier = NULL;
	item->configData = NULL;
	
	*outItem = item;

CantAllocItem:
	return err;
}

//-----------------------------------------------------------------------------
//	DestructCustomToolbarItem
//-----------------------------------------------------------------------------
//	Destroy our custom item. Be sure to release our config data.
//
static void
DestructCustomToolbarItem( CustomToolbarItem* inItem )
{
	if ( inItem->identifier != NULL )
		CFRelease( inItem->identifier );
	
	if ( inItem->configData != NULL )
		CFRelease( inItem->configData );

	free( inItem );
}

//-----------------------------------------------------------------------------
//	InitializeCustomToolbarItem
//-----------------------------------------------------------------------------
//	This is called after our item has been constructed. We are called here so
//	that we can pull parameters out of the Carbon Event that is passed into the
//	HIObjectCreate call.
//
static OSStatus
InitializeCustomToolbarItem( CustomToolbarItem* inItem, EventRef inEvent )
{
	CFTypeRef data = NULL;
	
	GetEventParameter( inEvent, kEventParamToolbarItemConfigData, typeCFTypeRef, NULL, sizeof( CFTypeRef ), NULL, &data );
	
	HIToolbarItemCopyIdentifier( inItem->toolbarItem, &(inItem->identifier) );
	
	// initialize a URL toolbar item
	if ( CFStringCompare( kURLToolbarItemClassID, inItem->identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		if ( data == NULL )
		{
			inItem->configData = CFURLCreateWithString( NULL, CFSTR( "http://www.apple.com" ), NULL );
		}
		else if ( CFGetTypeID( data ) == CFStringGetTypeID() )
		{
			inItem->configData = CFURLCreateWithString( NULL, data, NULL );
		}
		else if ( CFGetTypeID( data ) == CFURLGetTypeID() )
		{
			inItem->configData = CFRetain( data );
		}
				
		HIToolbarItemSetLabel( inItem->toolbarItem, CFSTR("URL Item") );
		
		IconRef iconRef;
		if ( GetIconRef( kOnSystemDisk, kSystemIconsCreator, kGenericURLIcon, &iconRef ) == noErr )
		{
			HIToolbarItemSetIconRef( inItem->toolbarItem, iconRef );
			ReleaseIconRef( iconRef );
		}
		
		if ( inItem->configData != NULL )
			HIToolbarItemSetHelpText( inItem->toolbarItem, CFURLGetString( inItem->configData ), NULL );
	}
	// initialize a back forward buttons toolbar item
	else if ( CFStringCompare( kButtonsToolbarItemClassID, inItem->identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		HIToolbarItemSetLabel( inItem->toolbarItem, CFSTR("History") );
	}
	// initialize a search field toolbar item
	else if ( CFStringCompare( kSearchToolbarItemClassID, inItem->identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		if ( data != NULL && CFGetTypeID( data ) == CFStringGetTypeID() )
			inItem->configData = CFRetain( data );
		
		HIToolbarItemSetLabel( inItem->toolbarItem, CFSTR("Search") );
	}
	
	return noErr;
}

//-----------------------------------------------------------------------------
//	CreateCustomToolbarItemPersistentData
//-----------------------------------------------------------------------------
//	This is called when the toolbar is about to write the config for this item
//	to preferences. It is your chance to save any extra data with the item. You
//	must make sure the data is something that can be saved to XML.
//
static CFTypeRef
CreateCustomToolbarItemPersistentData( CustomToolbarItem* inItem )
{
	if ( inItem->configData != NULL )
	{
		// convert the URL toolbar item's url to a string for persistent storage
		if ( CFStringCompare( kURLToolbarItemClassID, inItem->identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
		{
			return (CFTypeRef)CFStringCreateCopy( NULL, CFURLGetString( inItem->configData ) );
		}
		// return the search field toolbar item's descriptive text string for persistend storage
		else if ( CFStringCompare( kSearchToolbarItemClassID, inItem->identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
		{
			return CFRetain( inItem->configData );
		}
	}
	
	return NULL;
}

//-----------------------------------------------------------------------------
//	PerformCustomToolbarItemAction
//-----------------------------------------------------------------------------
//	This is called when a toolbar item has been clicked and should perform its
//	action.
//
static OSStatus
PerformCustomToolbarItemAction( CustomToolbarItem* inItem )
{
	OSStatus result = noErr;
	
	// open the URL for a custom URL toolbar item
	if ( CFStringCompare( kURLToolbarItemClassID, inItem->identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
		if ( inItem->configData != NULL && CFGetTypeID( inItem->configData ) == CFURLGetTypeID() )
			result = LSOpenCFURLRef( inItem->configData, NULL );
	
	return result;
}

//-----------------------------------------------------------------------------
//	GetImageFromPNG
//-----------------------------------------------------------------------------
//	Generates a CGImageRef from a png file located in the main bundle for use
//	in CreateCustomToolbarItemView.
//
static CGImageRef
GetImageFromPNG( CFStringRef inPNGFileName )
{
	CFURLRef			url;
	CGDataProviderRef	provider;
	CGImageRef			image = NULL;
	
	url = CFBundleCopyResourceURL( CFBundleGetMainBundle(), inPNGFileName, NULL, NULL );
	
	if ( url != NULL )
	{
		provider = CGDataProviderCreateWithURL( url );
		
		image = CGImageCreateWithPNGDataProvider( provider, NULL, false, kCGRenderingIntentDefault );
		
		CGDataProviderRelease( provider );
		
		CFRelease( url );
	}
	
	return image;
}

//-----------------------------------------------------------------------------
//	CreateCustomToolbarItemView
//-----------------------------------------------------------------------------
//	This is called when the toolbar is about to write the config for this item
//	to preferences. It is your chance to save any extra data with the item. You
//	must make sure the data is something that can be saved to XML.
//
static OSStatus
CreateCustomToolbarItemView( CustomToolbarItem* inItem, HIViewRef* outView )
{
	OSStatus result = eventNotHandledErr;
	
	// create an HISegmentedView for the custom back and forward buttons toolbar item
	if ( CFStringCompare( kButtonsToolbarItemClassID, inItem->identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		result = HISegmentedViewCreate( NULL, outView );
		
		if ( *outView != NULL )
		{
			HISegmentedViewSetSegmentCount( *outView, 2 );
			
			// add the left and right button images
			HIViewImageContentInfo content;
			content.contentType = kControlContentCGImageRef;
			
			content.u.imageRef = GetImageFromPNG( CFSTR( "left.png" ) );
			HISegmentedViewSetSegmentImage( *outView, 1, &content );
			CFRelease( content.u.imageRef );
			
			content.u.imageRef = GetImageFromPNG( CFSTR( "right.png" ) );
			HISegmentedViewSetSegmentImage( *outView, 2, &content );
			CFRelease( content.u.imageRef );
		}
	}
	// create an HISearchFieldView for the custom search field toolbar item
	else if ( CFStringCompare( kSearchToolbarItemClassID, inItem->identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		result = HISearchFieldCreate( NULL, kHISearchFieldAttributesCancel | kHISearchFieldAttributesSearchIcon,
								NULL, inItem->configData, outView );
	}
	
	return result;
}

static void
ModifyCustomToolbarItemConstraints( CustomToolbarItem* inItem, HISize* ioMin, HISize* ioMax )
{
	// return a reasonable set of constraints for the search field toolbar item
	if ( CFStringCompare( kSearchToolbarItemClassID, inItem->identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		ioMin->height = 22;
		ioMax->height = 22;
		
		// flexible between 60 and 200 pixels
		ioMin->width = 60;
		ioMax->width = 200;
	}
}

#pragma mark -

//-----------------------------------------------------------------------------
//	CustomToolbarItemHandler
//-----------------------------------------------------------------------------
//	This is where the magic happens. This is your method reception desk. When
//	your object is created, etc. or receives events, this is where we take the
//	events and call specific functions to do the work.
//
static pascal OSStatus
CustomToolbarItemHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	OSStatus			result = eventNotHandledErr;
	CustomToolbarItem*	object = (CustomToolbarItem*)inUserData;

	switch ( GetEventClass( inEvent ) )
	{
		case kEventClassHIObject:
			switch ( GetEventKind( inEvent ) )
			{
				case kEventHIObjectConstruct:
					{
						HIObjectRef			toolbarItem;
						CustomToolbarItem*	item;
						
						GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL,
								sizeof( HIObjectRef ), NULL, &toolbarItem );
						
						result = ConstructCustomToolbarItem( toolbarItem, &item );
						
						if ( result == noErr )
							SetEventParameter( inEvent, kEventParamHIObjectInstance, 'void', sizeof( void * ), &item );
					}
					break;

				case kEventHIObjectInitialize:
					if ( CallNextEventHandler( inCallRef, inEvent ) == noErr )
						result = InitializeCustomToolbarItem( object, inEvent );
					break;
				
				case kEventHIObjectDestruct:
					DestructCustomToolbarItem( object );
					result = noErr;
					break;
			}
			break;
		
		case kEventClassToolbarItem:
			switch ( GetEventKind( inEvent ) )
			{
				case kEventToolbarItemGetPersistentData:
				{
					CFTypeRef data;
					
					data = CreateCustomToolbarItemPersistentData( object );
					
					if ( data )
						SetEventParameter( inEvent, kEventParamToolbarItemConfigData, typeCFTypeRef,
											sizeof( CFTypeRef ), &data );
				}
				break;
		
				case kEventToolbarItemPerformAction:
				{
					result = PerformCustomToolbarItemAction( object );
				}
				break;
				
				case kEventToolbarItemCreateCustomView:
				{
					HIViewRef view = NULL;
					
					result = CreateCustomToolbarItemView( object, &view );
					
					if ( view != NULL )
					{
						// set a control size constraints handler for views that need a specific or flexible size
						const EventTypeSpec kViewEvents[] = { { kEventClassControl, kEventControlGetSizeConstraints } };
						
						InstallEventHandler( GetControlEventTarget( view ), CustomToolbarItemHandler,
												GetEventTypeCount( kViewEvents ), kViewEvents, object, NULL );
						
						result = SetEventParameter( inEvent, kEventParamControlRef, typeControlRef, sizeof( HIViewRef ), &view );
					}
				}
				break;
			}
			break;
		
		case kEventClassControl:
			switch ( GetEventKind( inEvent ) )
			{
				case kEventControlGetSizeConstraints:
				{
					HISize min, max;
					
					// first, get the control's existing size constraints if any
					CallNextEventHandler( inCallRef, inEvent );
					
					result = GetEventParameter( inEvent, kEventParamMinimumSize, typeHISize, NULL,
													sizeof( HISize ), NULL, &min );
					
					result = GetEventParameter( inEvent, kEventParamMaximumSize, typeHISize, NULL,
													sizeof( HISize ), NULL, &max );
					
					// then, modify them as necessary
					ModifyCustomToolbarItemConstraints( object, &min, &max );
					
					result = SetEventParameter( inEvent, kEventParamMinimumSize, typeHISize,
													sizeof( HISize ), &min );
					
					result = SetEventParameter( inEvent, kEventParamMaximumSize, typeHISize,
													sizeof( HISize ), &max );
				}
				break;
			}
			break;
	}
	
	return result;
}

#pragma mark -

//-----------------------------------------------------------------------------
//	RegisterURLToolbarItemClass
//-----------------------------------------------------------------------------
//	Our class registration call. We take care to only register once. We call this
//	in our CreateURLToolbarItem call below to make sure our class is in place
//	before trying to instantiate an object of the class.
//
void
RegisterURLToolbarItemClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		HIObjectRegisterSubclass( kURLToolbarItemClassID, kHIToolbarItemClassID, 0,
				CustomToolbarItemHandler, GetEventTypeCount( kEvents ), kEvents, 0, NULL );
		
		sRegistered = true;
	}
}

//-----------------------------------------------------------------------------
//	RegisterButtonsToolbarItemClass
//-----------------------------------------------------------------------------
//	Our class registration call. We take care to only register once. We call this
//	in our CreateButtonsToolbarItem call below to make sure our class is in place
//	before trying to instantiate an object of the class.
//
void
RegisterButtonsToolbarItemClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		HIObjectRegisterSubclass( kButtonsToolbarItemClassID, kHIToolbarItemClassID, 0,
				CustomToolbarItemHandler, GetEventTypeCount( kEvents ), kEvents, 0, NULL );
		
		sRegistered = true;
	}
}

//-----------------------------------------------------------------------------
//	RegisterSearchToolbarItemClass
//-----------------------------------------------------------------------------
//	Our class registration call. We take care to only register once. We call this
//	in our CreateSearchToolbarItem call below to make sure our class is in place
//	before trying to instantiate an object of the class.
//
void
RegisterSearchToolbarItemClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		HIObjectRegisterSubclass( kSearchToolbarItemClassID, kHIToolbarItemClassID, 0,
				CustomToolbarItemHandler, GetEventTypeCount( kEvents ), kEvents, 0, NULL );
		
		sRegistered = true;
	}
}

#pragma mark -

//-----------------------------------------------------------------------------
//	CreateCustomToolbarItem
//-----------------------------------------------------------------------------
//	Our private custom toolbar creation bottleneck.
//
static HIToolbarItemRef
CreateCustomToolbarItem( CFStringRef inIdentifier, CFTypeRef inConfigData )
{
	OSStatus			err;
	EventRef			event;
	UInt32				options = kHIToolbarItemAllowDuplicates;
	HIToolbarItemRef	result = NULL;
	
	err = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(), 0, &event );
	require_noerr( err, CantCreateEvent );
	
	SetEventParameter( event, kEventParamToolbarItemIdentifier, typeCFStringRef, sizeof( CFStringRef ), &inIdentifier );
	SetEventParameter( event, kEventParamAttributes, typeUInt32, sizeof( UInt32 ), &options );
	
	if ( inConfigData )
		SetEventParameter( event, kEventParamToolbarItemConfigData, typeCFTypeRef, sizeof( CFTypeRef ), &inConfigData );
	
	err = HIObjectCreate( inIdentifier, event, (HIObjectRef*)&result );
	check_noerr( err );
	
	ReleaseEvent( event );
	
CantCreateEvent:
	return result;
}

//-----------------------------------------------------------------------------
//	CreateURLToolbarItem
//-----------------------------------------------------------------------------
//	Our 'public' API to create our custom URL item.
//
HIToolbarItemRef
CreateURLToolbarItem( CFURLRef inURL )
{
	RegisterURLToolbarItemClass();
	
	return CreateCustomToolbarItem( kURLToolbarItemClassID, inURL );
}

//-----------------------------------------------------------------------------
//	CreateButtonsToolbarItem
//-----------------------------------------------------------------------------
//	Our 'public' API to create our custom back/forward buttons item.
//
HIToolbarItemRef
CreateButtonsToolbarItem()
{
	RegisterButtonsToolbarItemClass();
	
	return CreateCustomToolbarItem( kButtonsToolbarItemClassID, NULL );
}

//-----------------------------------------------------------------------------
//	CreateSearchToolbarItem
//-----------------------------------------------------------------------------
//	Our 'public' API to create our custom search field item.
//
HIToolbarItemRef
CreateSearchToolbarItem( CFStringRef inDescriptiveText )
{
	RegisterSearchToolbarItemClass();
	
	return CreateCustomToolbarItem( kSearchToolbarItemClassID, inDescriptiveText );
}
