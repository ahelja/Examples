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

#include "THistoryItemMenu.h"

static const EventTypeSpec kEvents[] = {
	{ kEventClassMenu, kEventMenuOpening },
	{ kEventClassCommand, kEventCommandProcess }
};

static const EventTypeSpec kSubmenuEvents[] = {
	{ kEventClassCommand, kEventCommandProcess }
};

static CGImageRef	ResizeImage( CGImageRef inImage, CGSize inSize );
static void 		_ReleaseOffscreenImage( void *info, const void *data, size_t size );

THistoryItemMenu::THistoryItemMenu() : fMenu(NULL), fHandler(NULL)
{
}

THistoryItemMenu::~THistoryItemMenu()
{
	if ( fHandler )
		RemoveEventHandler( fHandler );

	if ( fMenu )
		ReleaseMenu( fMenu );
}

void
THistoryItemMenu::Init( MenuRef inMenu )
{
	fMenu = inMenu;
	InstallMenuEventHandler( fMenu, EventHandlerProc,
			GetEventTypeCount( kEvents ), kEvents, this, &fHandler );
}

MenuRef
THistoryItemMenu::CreateSubmenu()
{
	MenuRef		menu;
	
	CreateNewMenu( 0, 0, &menu );
	InstallMenuEventHandler( menu, EventHandlerProc,
			GetEventTypeCount( kSubmenuEvents ), kSubmenuEvents, this, &fHandler );
	
	return menu;
}

MenuItemIndex
THistoryItemMenu::AddItemsToMenu( MenuRef inMenu, CFArrayRef inArray, MenuItemIndex inAfterItem )
{
	CFIndex			i, count;
	
	count = CFArrayGetCount( inArray );

	for ( i = 0; i < count; i++ )
	{
		WebHistoryItem* item = (WebHistoryItem*)CFArrayGetValueAtIndex( inArray, i );
		AddItemToMenu( inMenu, item, inAfterItem++ );
	}
	
	return inAfterItem;
}

MenuItemIndex
THistoryItemMenu::AddItemsToMenuBackwards( MenuRef inMenu, CFArrayRef inArray, MenuItemIndex inAfterItem )
{
	CFIndex			i, count;
	
	count = CFArrayGetCount( inArray );

	for ( i = count-1; i >= 0; i-- )
	{
		WebHistoryItem* item = (WebHistoryItem*)CFArrayGetValueAtIndex( inArray, i );
		AddItemToMenu( inMenu, item, inAfterItem++ );
	}
	
	return inAfterItem;
}

void
THistoryItemMenu::AddItemToMenu( MenuRef inMenu, WebHistoryItem* inItem, MenuItemIndex inAfterItem )
{
	CFStringRef			title = (CFStringRef)[inItem alternateTitle];
	NSImage*			image = [inItem icon];
	CGImageRef			icon;
	
	if ( title == NULL )
		title = (CFStringRef)[inItem title];

	require( title != NULL, CantGetTitle );

	InsertMenuItemTextWithCFString( inMenu, title, inAfterItem++, 0, 'LINK' );	
	SetMenuItemRefCon( inMenu, inAfterItem, (UInt32)inItem );

	if ( image )
	{
		icon = WebConvertNSImageToCGImageRef( image );
		if ( icon )
		{
			CGImageRef	resizedImage = ResizeImage( icon, CGSizeMake( 16, 16 ) );
			if ( resizedImage )
			{
				SetMenuItemIconHandle( inMenu, inAfterItem, kMenuCGImageRefType, (Handle)resizedImage );
				CGImageRelease( resizedImage );
			}
			CGImageRelease( icon );
		}
	}

CantGetTitle:
	return;
}

OSStatus
THistoryItemMenu::EventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	OSStatus 			result = eventNotHandledErr;
	THistoryItemMenu*	menu = static_cast<THistoryItemMenu*>(inUserData);
	
	switch ( GetEventClass( inEvent ) )
	{
		case kEventClassMenu:
			menu->Populate();
			result = noErr;
			break;
		
		case kEventClassCommand:
			{
				HICommand		command;
				
				GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
						sizeof( HICommand ), NULL, &command );
				
				if ( command.commandID == 'LINK'
						&& ( command.attributes & kHICommandFromMenu ) )
				{
					WebHistoryItem*		item;
					
					GetMenuItemRefCon( command.menu.menuRef, command.menu.menuItemIndex, (UInt32*)&item );
					menu->GoToItem( item );
                                        result = noErr;
				}
			}
			break;
	}
	
	return result;
}

static CGImageRef
ResizeImage( CGImageRef inImage, CGSize inSize )
{
	HIRect				bounds;
	CGImageRef			image = NULL;
	size_t				bytesPerRow;
	size_t				size;
	size_t				height, width;
	void *				ptr;
	CGContextRef		context;
	CGDataProviderRef	provider;
	CGColorSpaceRef		colorSpace;
	
	if ( CGImageGetWidth( inImage ) == inSize.width &&
		CGImageGetHeight( inImage ) == inSize.height )
	{
		CGImageRetain( inImage );
		return inImage;
	}
	
	bounds.origin.x = bounds.origin.y = 0;
	bounds.size.width = inSize.width;
	bounds.size.height = inSize.height;

	height = (size_t)ceil( bounds.size.height );
	width = (size_t)ceil( bounds.size.width );

	bytesPerRow = ( ( width * 8 * 4 + 7 ) / 8 );
	size = bytesPerRow * height;
	
	ptr = malloc( size );
	require( ptr != NULL, CantAllocateImageTemp );

	BlockZero( ptr, size );

	context = CGBitmapContextCreate( ptr, width, height, 8, bytesPerRow, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedFirst );
	check( context != NULL );
		
	CGContextDrawImage( context, bounds, inImage );

	CGContextRelease( context );

	// Create a CGImage from our offscreen.

	provider = CGDataProviderCreateWithData( 0, ptr, size, _ReleaseOffscreenImage );
	colorSpace = CGColorSpaceCreateDeviceRGB();
	
	image = CGImageCreate( width, height, 8, 32, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedFirst,
							provider, NULL, false, kCGRenderingIntentDefault );

	CGColorSpaceRelease( colorSpace );
	CGDataProviderRelease( provider );

CantAllocateImageTemp:

	return image;
}

static void 
_ReleaseOffscreenImage( void *info, const void *data, size_t size )
{
	free( (void *)data );
}
