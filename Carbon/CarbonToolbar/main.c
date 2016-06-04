/*
    File:		main.c
    
    Version:	Mac OS X

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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

#include "CustomToolbarItem.h"

// not yet available in MacWindows.h
#define kWindowUnifiedTitleAndToolbarAttribute 1 << 7

//-----------------------------------------------------------------------------
//	Prototypes
//-----------------------------------------------------------------------------

static OSStatus		InstallToolbar( WindowRef window );
static void			GetToolbarDefaultItems( CFMutableArrayRef array );
static void			GetToolbarAllowedItems( CFMutableArrayRef array );
static OSStatus		ToolbarDelegateHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static IconRef		RegisterIcon( FourCharCode inCreator, FourCharCode inType, const char* inName );
static OSStatus		CommandHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static OSStatus		ToolbarWindowHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inRefcon );
static void			MenuUpdateTimer( EventLoopTimerRef inTimer, void* inRefcon );
static void			ReloadButtonMenus( WindowRef inWindow );

static HIToolbarItemRef			CreateToolbarItemForIdentifier( CFStringRef identifier, CFTypeRef configData );
static HIToolbarItemRef			CreateToolbarItemFromDrag( DragRef drag );

static const EventTypeSpec kToolbarEvents[] =
{
	{ kEventClassToolbar, kEventToolbarGetDefaultIdentifiers },
	{ kEventClassToolbar, kEventToolbarGetAllowedIdentifiers },
	{ kEventClassToolbar, kEventToolbarGetSelectableIdentifiers },
	{ kEventClassToolbar, kEventToolbarCreateItemWithIdentifier },
	{ kEventClassToolbar, kEventToolbarCreateItemFromDrag }
};

static const EventTypeSpec kCommandEvents[] =
{
	{ kEventClassCommand, kEventCommandProcess },
	{ kEventClassCommand, kEventCommandUpdateStatus }
};

static const EventTypeSpec kToolbarWindowEvents[] =
{
	{ kEventClassToolbar, kEventToolbarItemAdded },
	{ kEventClassToolbar, kEventToolbarItemRemoved }
};

enum
{
	kCmdLockToolbar			= 'Lock',
	kCmdUnlockToolbar		= 'ULck',
	kCmdMetal				= 'METL',
	kCmdNoMetalSeparator	= 'NSEP',
	kCmdUnifiedTitleToolbar	= 'UNIF',

	kCmdSelectableItems		= 'SLCT',
	kCmdToolbarItemsMenu	= 'ITEM',
	
	kCtlIDEnablingButton	= 'ENPB',
	kCtlIDSelectionButton   = 'SLPB',
	kCtlIDSelectableItems   = 'SLCT',
	kCtlIDEnablingWindow	= 'ENCK',
	kCtlIDSelectionWindow   = 'SLCK'
};

enum
{
	kToolbarPropCreator		= 'CBTB',
	kToolbarMenuTypeID			= 0,
	kToolbarItemRefID			= 1
};

static void		CreateToolbarWindow();

static OSStatus 	AppHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );

static EventLoopTimerRef	sTimer;
static HIToolbarRef			sToolbar;

//-----------------------------------------------------------------------------
//	main
//-----------------------------------------------------------------------------
//	
int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    OSStatus		err;
	HICommand		command = { 0, kHICommandNew };
	EventTypeSpec	kEvents[] = { { kEventClassCommand, kEventCommandProcess } };
	
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);

	InstallApplicationEventHandler( AppHandler, GetEventTypeCount( kEvents ), kEvents, 0, NULL );
	
	ProcessHICommand( &command );
	
    // Call the event loop
    RunApplicationEventLoop();
	
	CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );

CantSetMenuBar:
CantGetNibRef:
	return err;
}

//-----------------------------------------------------------------------------
//	AppHandler
//-----------------------------------------------------------------------------
//	Deal with commands for the appliation.
//
static OSStatus
AppHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	HICommand		command;
	OSStatus		result = eventNotHandledErr;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
			sizeof( HICommand ), NULL, &command );
	
	switch ( command.commandID )
	{
		case kHICommandNew:
			CreateToolbarWindow();
			result = noErr;
			break;
	}
	
	return result;
}

//-----------------------------------------------------------------------------
//	CommandHandler
//-----------------------------------------------------------------------------
//	Deal with commands for the toolbar menu. We allow the toolbar to be locked
// 	and unlocked. This shows how the toolbar behaves when configurable and not.
//
static OSStatus
CommandHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	OSStatus		result = eventNotHandledErr;
	HICommand		command;
	WindowRef		window = (WindowRef)inUserData;
	HIToolbarRef	toolbar;
	UInt32			attrs;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
			sizeof( HICommand ), NULL, &command );

	// Make sure it came from the menu

	if ( command.attributes & kHICommandFromMenu )
	{
		GetWindowToolbar( window, &toolbar );
		HIToolbarGetAttributes( toolbar, &attrs );
	
		switch ( GetEventKind( inEvent ) )
		{
			case kEventCommandUpdateStatus:
				switch ( command.commandID )
				{
					case kCmdLockToolbar:
						if ( ( attrs & kHIToolbarIsConfigurable ) != 0 )
							ChangeMenuItemAttributes( command.menu.menuRef, command.menu.menuItemIndex, 0, kMenuItemAttrHidden );
						else
							ChangeMenuItemAttributes( command.menu.menuRef, command.menu.menuItemIndex, kMenuItemAttrHidden, 0 );
						
						result = noErr;
						break;
					
					case kCmdUnlockToolbar:
						if ( ( attrs & kHIToolbarIsConfigurable ) == 0 )
							ChangeMenuItemAttributes( command.menu.menuRef, command.menu.menuItemIndex, 0, kMenuItemAttrHidden );
						else
							ChangeMenuItemAttributes( command.menu.menuRef, command.menu.menuItemIndex, kMenuItemAttrHidden, 0 );
						
						result = noErr;
						break;
					
					case kCmdMetal:
						{
							WindowAttributes		windAttrs;
							
							GetWindowAttributes( window, &windAttrs );
							if ( windAttrs & kWindowMetalAttribute )
								CheckMenuItem( command.menu.menuRef, command.menu.menuItemIndex, true );
							else
								CheckMenuItem( command.menu.menuRef, command.menu.menuItemIndex, false );
						
							result = noErr;
						}
						break;
					
					case kCmdNoMetalSeparator:
						{
							WindowAttributes		windAttrs;
							
							GetWindowAttributes( window, &windAttrs );
							if ( windAttrs & kWindowMetalNoContentSeparatorAttribute )
								CheckMenuItem( command.menu.menuRef, command.menu.menuItemIndex, true );
							else
								CheckMenuItem( command.menu.menuRef, command.menu.menuItemIndex, false );
						
							result = noErr;
						}
						break;
					
					case kCmdUnifiedTitleToolbar:
						{
							WindowAttributes		windAttrs;
							
							GetWindowAttributes( window, &windAttrs );
							if ( windAttrs & kWindowUnifiedTitleAndToolbarAttribute )
								CheckMenuItem( command.menu.menuRef, command.menu.menuItemIndex, true );
							else
								CheckMenuItem( command.menu.menuRef, command.menu.menuItemIndex, false );
						
							result = noErr;
						}
						break;
				}
				break;

			case kEventCommandProcess:
				switch ( command.commandID )
				{
					case kCmdLockToolbar:
						HIToolbarChangeAttributes( toolbar, 0, kHIToolbarIsConfigurable );
						result = noErr;
						break;
					
					case kCmdUnlockToolbar:
						HIToolbarChangeAttributes( toolbar, kHIToolbarIsConfigurable, 0 );
						result = noErr;
						break;
					
					case kCmdMetal:
						{
							WindowAttributes		windAttrs;
							
							GetWindowAttributes( window, &windAttrs );
							if ( windAttrs & kWindowMetalAttribute )
								ChangeWindowAttributes( window, 0, kWindowMetalAttribute );
							else
								ChangeWindowAttributes( window, kWindowMetalAttribute, 0 );
						
							result = noErr;
						}
						break;
					
					case kCmdNoMetalSeparator:
						{
							WindowAttributes		windAttrs;
							
							GetWindowAttributes( window, &windAttrs );
							if ( windAttrs & kWindowMetalNoContentSeparatorAttribute )
								ChangeWindowAttributes( window, 0, kWindowMetalNoContentSeparatorAttribute );
							else
								ChangeWindowAttributes( window, kWindowMetalNoContentSeparatorAttribute, 0 );
						
							result = noErr;
						}
						break;
					
					case kCmdUnifiedTitleToolbar:
						{
							WindowAttributes		windAttrs;
							
							GetWindowAttributes( window, &windAttrs );
							if ( windAttrs & kWindowUnifiedTitleAndToolbarAttribute )
								ChangeWindowAttributes( window, 0, kWindowUnifiedTitleAndToolbarAttribute );
							else
								ChangeWindowAttributes( window, kWindowUnifiedTitleAndToolbarAttribute, 0 );
						
							result = noErr;
						}
						break;
						
					case kCmdToolbarItemsMenu:
						{
							OSType				menuType;
							HIToolbarItemRef	item;
							ControlID			ctlIDEachWindow;
							ControlRef			eachWindowCtl;
							WindowRef			whichWindow = NULL;
							OptionBits			attr;
							OptionBits			attrToSet = 0;
							OptionBits			attrToClear = 0;
							
							GetMenuItemProperty( command.menu.menuRef, 0, kToolbarPropCreator, kToolbarMenuTypeID,
												 sizeof( menuType ), NULL, &menuType );
							GetMenuItemProperty( command.menu.menuRef, command.menu.menuItemIndex, kToolbarPropCreator, kToolbarItemRefID,
												 sizeof( item ), NULL, &item );
							
							ctlIDEachWindow.signature = ( menuType & 0xFFFF0000 ) | '\0\0CK';
							ctlIDEachWindow.id = 0;
							GetControlByID( window, &ctlIDEachWindow, &eachWindowCtl );
							if ( GetControlValue( eachWindowCtl ) != 0 )
								whichWindow = window;
							
							HIToolbarItemGetAttributesInWindow( item, whichWindow, NULL, &attr );
							
							if ( menuType == kCtlIDEnablingButton )
							{
								if ( ( attr & kHIToolbarItemDisabled ) == 0 )
									attrToSet = kHIToolbarItemDisabled;
								else
									attrToClear = kHIToolbarItemDisabled;
							}
							else
							{
								check( menuType == kCtlIDSelectionButton );
								if ( ( attr & kHIToolbarItemSelected ) == 0 )
									attrToSet = kHIToolbarItemSelected;
								else
									attrToClear = kHIToolbarItemSelected;
							}
							
							HIToolbarItemChangeAttributesInWindow( item, whichWindow, attrToSet, attrToClear, 0 );
							
							// allow popup button to change the selected item
							check( result == eventNotHandledErr );
						}
						break;
				}
				break;
		}
	}
	// handle commands from controls
	else if ( GetEventKind( inEvent ) == kEventCommandProcess )
	{
		switch ( command.commandID )
		{
			case kCmdSelectableItems:
				{
					HICommandExtended* extendedCommand = (HICommandExtended*) &command;

					if ( GetControlValue( extendedCommand->source.control ) == 0 )
					{
						HIToolbarItemRef item = NULL;
						HIToolbarGetSelectedItemInWindow( sToolbar, window, &item );
						if ( item != NULL )
							HIToolbarItemChangeAttributesInWindow( item, window, 0, kHIToolbarItemSelected, kHIToolbarItemSelected );
					}
					result = noErr;
				}
				break;
			
			default:
				break;
		}
	}
	
	return result;
}

//-----------------------------------------------------------------------------
//	InstallToolbar
//-----------------------------------------------------------------------------
//	Create and attach our toolbar to the window
//
static OSStatus
InstallToolbar( WindowRef window )
{
	OSStatus			err = noErr;

	// We're going to share one toolbar across each of our windows. We only want
	// to do the work of creating the toolbar once, so we'll store it in a static.
	// If the toolbar is non-NULL, that means we already created it in a previous
	// call to this function.
	
	if ( sToolbar == NULL )
	{
		// OK. Here's the beginning of the fun. Create our toolbar. We want to
		// automatically save our config to our prefs and we want to allow the
		// user to customize it.
	
		err = HIToolbarCreate( CFSTR( "com.apple.carbontoolbar" ),
							kHIToolbarAutoSavesConfig | kHIToolbarIsConfigurable,
							&sToolbar );
		require_noerr( err, CantCreateToolbar );
	
		// Now, a toolbar all by its lonesome is nothing to behold. You need to
		// add some items. The toolbar does this via a delegate. The delegate is
		// asked to supply items as they are needed by the toolbar. It is also
		// responsible for providing the default set of items, and the set of
		// allowable items (which is shown in the config sheet). Any HIObjectRef
		// can be the toolbar's delegate. The toolbar simply needs a place to
		// which it can send the appropriate carbon events. By default, the
		// toolbar starts out as its own delegate. You can change the delegate
		// to another HIObjectRef with the HIToolbarSetDelegate API if you wish.
		
		// For our purposes, we're fine with the default behavior of the toolbar
		// being its own delegate. All we need to do is install a carbon event
		// handler on the toolbar so we can handle the delegate events that the
		// toolbar sends out.
		
		InstallEventHandler( HIObjectGetEventTarget( sToolbar ), ToolbarDelegateHandler,
				GetEventTypeCount( kToolbarEvents ), kToolbarEvents, sToolbar, NULL );
	}
				
	// Now we just attach the toolbar to the window and make it visible.

	SetWindowToolbar( window, sToolbar );
	ShowHideWindowToolbar( window, true, false );

	// Now add the toolbar widget to the window. We could also just specify
	// it in the nib file, but let's do it programmatically.
	
	ChangeWindowAttributes( window, kWindowToolbarButtonAttribute, 0 );
	
	// The toolbar requires that automatic drag tracking be enabled.

	SetAutomaticControlDragTrackingEnabledForWindow( window, true );

CantCreateToolbar:
	return err;
}

//-----------------------------------------------------------------------------
//	ToolbarDelegateHandler
//-----------------------------------------------------------------------------
//	Here's our toolbar event handler. A delegate MUST handle the 'get default',
//	'get allowed', and 'create with identifier' events. The create-from-drag
//	event is optional, and generally isn't used.
//
static OSStatus
ToolbarDelegateHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	OSStatus			result = eventNotHandledErr;
	CFMutableArrayRef	array;
	CFStringRef			identifier;
	
	switch ( GetEventKind( inEvent ) )
	{
		case kEventToolbarGetDefaultIdentifiers:
			GetEventParameter( inEvent, kEventParamMutableArray, typeCFMutableArrayRef, NULL,
					sizeof( CFMutableArrayRef ), NULL, &array );
			GetToolbarDefaultItems( array );
			result = noErr;
			break;
			
		case kEventToolbarGetAllowedIdentifiers:
			GetEventParameter( inEvent, kEventParamMutableArray, typeCFMutableArrayRef, NULL,
					sizeof( CFMutableArrayRef ), NULL, &array );
			GetToolbarAllowedItems( array );
			result = noErr;
			break;
			
		case kEventToolbarGetSelectableIdentifiers:
			{
				WindowRef window = ActiveNonFloatingWindow();
				if ( window != NULL )
				{
					ControlID ctlID = { kCtlIDSelectableItems, 0 };
					ControlRef ctl = NULL;
					GetControlByID( window, &ctlID, &ctl );
					if ( ctl != NULL && GetControlValue( ctl ) != 0 )
					{
						GetEventParameter( inEvent, kEventParamMutableArray, typeCFMutableArrayRef, NULL,
								sizeof( CFMutableArrayRef ), NULL, &array );
						GetToolbarAllowedItems( array );
						result = noErr;
					}
				}
			}
			break;
			
		case kEventToolbarCreateItemWithIdentifier:
			{
				HIToolbarItemRef		item;
				CFTypeRef				data = NULL;
				
				GetEventParameter( inEvent, kEventParamToolbarItemIdentifier, typeCFStringRef, NULL,
						sizeof( CFStringRef ), NULL, &identifier );
				
				GetEventParameter( inEvent, kEventParamToolbarItemConfigData, typeCFTypeRef, NULL,
						sizeof( CFTypeRef ), NULL, &data );
					
				item = CreateToolbarItemForIdentifier( identifier, data );
				
				if ( item )
				{
					SetEventParameter( inEvent, kEventParamToolbarItem, typeHIToolbarItemRef,
						sizeof( HIToolbarItemRef ), &item );
					result = noErr;
				}
			}
			break;

		case kEventToolbarCreateItemFromDrag:
			{
				HIToolbarItemRef		item;
				DragRef					drag;
				
				GetEventParameter( inEvent, kEventParamDragRef, typeDragRef, NULL,
						sizeof( DragRef ), NULL, &drag );
				
				item = CreateToolbarItemFromDrag( drag );
				
				if ( item )
				{
					SetEventParameter( inEvent, kEventParamToolbarItem, typeHIToolbarItemRef,
						sizeof( HIToolbarItemRef ), &item );
					result = noErr;
				}
			}
			break;
	}

	return result;
}

//-----------------------------------------------------------------------------
//	GetToolbarDefaultItems
//-----------------------------------------------------------------------------
//	When the toolbar wants the default items, it asks us to fill an array out
//	with the identifiers of the items. Later, it will ask to have them created
//	via a separate event. Note that we are setting our defaults to both items
//	that are defined by us, and items supplied by the Toolbox.
//
static void
GetToolbarDefaultItems( CFMutableArrayRef array )
{
	CFArrayAppendValue( array, kButtonsToolbarItemClassID );
	CFArrayAppendValue( array, kHIToolbarSeparatorIdentifier );
	CFArrayAppendValue( array, kURLToolbarItemClassID );
	CFArrayAppendValue( array, CFSTR( "com.apple.carbontoolbar.trash" ) );
	CFArrayAppendValue( array, kHIToolbarSpaceIdentifier );
	CFArrayAppendValue( array, CFSTR( "com.apple.carbontoolbar.red" ) );
	CFArrayAppendValue( array, kHIToolbarFlexibleSpaceIdentifier );
	CFArrayAppendValue( array, kSearchToolbarItemClassID );
}

//-----------------------------------------------------------------------------
//	GetToolbarAllowedItems
//-----------------------------------------------------------------------------
//	When the toolbar wants the allowable items, it asks us to fill an array out
//	with the identifiers of the items, just like with the default items. Later,
//	it will ask to have them created via a separate event. Note that we are
//	setting our allowable items to both items that are defined by us, and items
//	supplied by the Toolbox.
//
static void
GetToolbarAllowedItems( CFMutableArrayRef array )
{
	CFArrayAppendValue( array, CFSTR( "com.apple.carbontoolbar.newdocument" ) );
	CFArrayAppendValue( array, CFSTR( "com.apple.carbontoolbar.anchored" ) );
	CFArrayAppendValue( array, CFSTR( "com.apple.carbontoolbar.permanent" ) );
	CFArrayAppendValue( array, CFSTR( "com.apple.carbontoolbar.trash" ) );
	CFArrayAppendValue( array, CFSTR( "com.apple.carbontoolbar.red" ) );
	CFArrayAppendValue( array, kURLToolbarItemClassID );
	CFArrayAppendValue( array, kButtonsToolbarItemClassID );
	CFArrayAppendValue( array, kSearchToolbarItemClassID );
	CFArrayAppendValue( array, kHIToolbarSeparatorIdentifier );
	CFArrayAppendValue( array, kHIToolbarSpaceIdentifier );
	CFArrayAppendValue( array, kHIToolbarFlexibleSpaceIdentifier );
	CFArrayAppendValue( array, kHIToolbarCustomizeIdentifier );
	CFArrayAppendValue( array, kHIToolbarPrintItemIdentifier );
	CFArrayAppendValue( array, kHIToolbarFontsItemIdentifier );
}

//-----------------------------------------------------------------------------
//	CreateToolbarItemForIdentifier
//-----------------------------------------------------------------------------
//	When the toolbar wants to create an item. It asks the delegate to do so.
//	For most items, the identifier is all that is necessary to create an item.
//	Some custom items, however, might require extra configuration data to 
//	properly initialize. Hence both parameters. Obviously, we only create the
//	items that we define. By returning NULL, it causes the event to be unhandled,
//	at which point the toolbar will try to create one of its default items.
//
static HIToolbarItemRef
CreateToolbarItemForIdentifier( CFStringRef identifier, CFTypeRef configData )
{
	HIToolbarItemRef		item = NULL;
		
	if ( CFStringCompare( CFSTR( "com.apple.carbontoolbar.permanent" ),
				identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{		
		// This item is marked as 'can't be removed'. This means the user can reposition
		// it in the toolbar, but it can't be dragged out or removed through the toolbar
		// context menu.

		if ( HIToolbarItemCreate( identifier, kHIToolbarItemCantBeRemoved, &item ) == noErr )
		{
			IconRef		icon;
			
			GetIconRef( kOnSystemDisk, kSystemIconsCreator, kFinderIcon, &icon );
			HIToolbarItemSetLabel( item, CFSTR( "Can't Remove Me" ) );
			HIToolbarItemSetIconRef( item, icon );
			HIToolbarItemSetCommandID( item, 'SHRT' );
			ReleaseIconRef( icon );
		}
	}

	if ( CFStringCompare( CFSTR( "com.apple.carbontoolbar.anchored" ),
				identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		// As an example, this item is marked as 'anchored left'. This means that it is not
		// only immovable, but nothing can be dragged in front of it either. For it to truly
		// work right, you need to have the items marked as such as the leftmost items. If you
		// allow a non-anchored item to be to the left of one of these, it gets a little weird.
		// This attribute is to make toolbars like the one in System Preferences, where the
		// Show All and the separator after it are fixed in place.

		if ( HIToolbarItemCreate( identifier, kHIToolbarItemAnchoredLeft, &item ) == noErr )
		{
			IconRef			icon;
			MenuRef			menu;
			static bool		sRegisteredIcon;
		
			if ( !sRegisteredIcon )
			{
				RegisterIcon( 'CTba', kHICommandCut, "cut.icns" );
				sRegisteredIcon = true;
			}
			
			GetIconRef( kOnSystemDisk, 'CTba', kHICommandCut, &icon );
			HIToolbarItemSetLabel( item, CFSTR( "Anchored" ) );
			HIToolbarItemSetCommandID( item, kHICommandCut );
			HIToolbarItemSetIconRef( item, icon );
			
			// For this item, we also attach a menu. This menu is used when the
			// item is clipped (and the 'more items' indicator is showing), as
			// well as when the toolbar is in text only mode.
	
			menu = NewMenu( 0, "\p" );
			AppendMenuItemTextWithCFString( menu, CFSTR( "Item 1" ), 0, 0, NULL );
			AppendMenuItemTextWithCFString( menu, CFSTR( "Item 2" ), 0, 0, NULL );
			AppendMenuItemTextWithCFString( menu, CFSTR( "Item 3" ), 0, 0, NULL );
			HIToolbarItemSetMenu( item, menu );
			ReleaseMenu( menu );

			ReleaseIconRef( icon );
		}
	}

	if ( CFStringCompare( CFSTR( "com.apple.carbontoolbar.trash" ),
				identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		// This item has no fancy stuff at all.

		if ( HIToolbarItemCreate( identifier, kHIToolbarItemNoAttributes, &item ) == noErr )
		{
			IconRef		icon;
			
			GetIconRef( kOnSystemDisk, kSystemIconsCreator, kTrashIcon, &icon );
			HIToolbarItemSetLabel( item, CFSTR( "Trash" ) );
			HIToolbarItemSetCommandID( item, 'TRSH' );
			HIToolbarItemSetIconRef( item, icon );
			ReleaseIconRef( icon );
		}
	}

	if ( CFStringCompare( CFSTR( "com.apple.carbontoolbar.newdocument" ),
				identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		if ( HIToolbarItemCreate( identifier, kHIToolbarItemNoAttributes, &item ) == noErr )
		{
			IconRef		icon;
			
			GetIconRef( kOnSystemDisk, kSystemIconsCreator, kGenericDocumentIcon, &icon );
			HIToolbarItemSetLabel( item, CFSTR( "New Document" ) );
			HIToolbarItemSetCommandID( item, kHICommandNew );
			HIToolbarItemSetIconRef( item, icon );
			ReleaseIconRef( icon );
		}
	}
	
	if ( CFStringCompare( CFSTR( "com.apple.carbontoolbar.red" ),
				identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		if ( HIToolbarItemCreate( identifier, kHIToolbarItemAllowDuplicates, &item ) == noErr )
		{
			IconRef			icon;
			static bool		sRegisteredIcon;
		
			if ( !sRegisteredIcon )
			{
				RegisterIcon( 'CTba', 'red ', "red.icns" );
				sRegisteredIcon = true;
			}
			
			GetIconRef( kOnSystemDisk, 'CTba', 'red ', &icon );
			HIToolbarItemSetLabel( item, CFSTR( "Red" ) );
			HIToolbarItemSetIconRef( item, icon );
			
			ReleaseIconRef( icon );
		}
	}
	
	if ( CFStringCompare( kURLToolbarItemClassID, identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		item = CreateURLToolbarItem( configData );
	}
	
	if ( CFStringCompare( kButtonsToolbarItemClassID,
				identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		item = CreateButtonsToolbarItem();
	}
	
	if ( CFStringCompare( kSearchToolbarItemClassID, identifier, kCFCompareBackwards ) == kCFCompareEqualTo )
	{
		item = CreateSearchToolbarItem( CFSTR("search") );
	}
	
	return item;
}

//-----------------------------------------------------------------------------
//	CreateToolbarItemFromDrag
//-----------------------------------------------------------------------------
//	Currently not used. Here for completeness.
//
static HIToolbarItemRef
CreateToolbarItemFromDrag( DragRef drag )
{
	UInt16				i, itemCount;
	HIToolbarItemRef	result = NULL;
	
	CountDragItems( drag, &itemCount );

	for ( i = 1; i <= itemCount; i++ )
	{
		DragItemRef			itemRef;
		FlavorFlags			flags;

		GetDragItemReferenceNumber( drag, i, &itemRef );
	
		if ( GetFlavorFlags( drag, itemRef, 'TEXT', &flags ) == noErr )
		{
			Size			dataSize;
			unsigned char	string[256];
			CFURLRef		url;

			dataSize = sizeof( string );
			GetFlavorData( drag, itemRef, 'TEXT', &string, &dataSize, 0 );
			
			url = CFURLCreateWithBytes( NULL, string, dataSize, kCFStringEncodingMacRoman, NULL );

			result = CreateURLToolbarItem( url );
			
			CFRelease( url );

			break;
		}
	}
	
	return result;
}

//-----------------------------------------------------------------------------
//	RegisterIcon
//-----------------------------------------------------------------------------
//	Utility routine to register an icns file.
//
static IconRef
RegisterIcon( FourCharCode inCreator, FourCharCode inType, const char* inName )
{
	IconRef result = 0;
	OSErr err = fnfErr;
	
	if ( inName != NULL && *inName != 0 )
	{
		CFBundleRef appBundle = CFBundleGetMainBundle();
		if ( appBundle )
		{
			CFStringRef fileName = CFStringCreateWithCString( NULL, inName, kCFStringEncodingASCII );
			if (fileName != NULL)
			{
				CFURLRef iconFileURL = CFBundleCopyResourceURL( appBundle, fileName, NULL, NULL );
				if (iconFileURL != NULL)
				{
					FSRef		fileRef;
					
					if ( CFURLGetFSRef( iconFileURL, &fileRef ) )
					{
						FSSpec		iconFileSpec;
						
						err = FSGetCatalogInfo( &fileRef, kFSCatInfoNone, NULL, NULL, &iconFileSpec, NULL );
						if ( err == noErr )
							err = RegisterIconRefFromIconFile( inCreator, inType, &iconFileSpec, &result );
					}
					CFRelease( iconFileURL );
				}
				CFRelease( fileName );
			}
		}
	}
	
	return result;
}

//-----------------------------------------------------------------------------
//  ToolbarWindowHandler
//-----------------------------------------------------------------------------
//  Handles toolbar events on behalf of the window.
//
static OSStatus
ToolbarWindowHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inRefcon )
{
	if ( sTimer == NULL )
		InstallEventLoopTimer( GetMainEventLoop(), 0, 0, MenuUpdateTimer, inRefcon, &sTimer );
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------
//	MenuUpdateTimer
//-----------------------------------------------------------------------------
//
static void
MenuUpdateTimer( EventLoopTimerRef inTimer, void* inRefcon )
{
	ReloadButtonMenus( (WindowRef) inRefcon );
	RemoveEventLoopTimer( inTimer );
	sTimer = NULL;
}

//-----------------------------------------------------------------------------
//	ReloadButtonMenus
//-----------------------------------------------------------------------------
//
static void
ReloadButtonMenus( WindowRef inWindow )
{
	ControlID   ctlIDEnabling = { kCtlIDEnablingButton, 0 };
	ControlID   ctlIDSelection = { kCtlIDSelectionButton, 0 };
	ControlRef  enablingButton;
	ControlRef  selectionButton;
	MenuRef		menu;
	MenuRef		menu2;
	CFArrayRef  items;
	CFIndex		count;
	int			i;
	OSType		menuType;
	
	GetControlByID( inWindow, &ctlIDEnabling, &enablingButton );
	GetControlByID( inWindow, &ctlIDSelection, &selectionButton );
	
	CreateNewMenu( 0, 0, &menu );
	
	HIToolbarCopyItems( sToolbar, &items );
	count = CFArrayGetCount( items );
	for ( i = 0; i < count; i++ )
	{
		HIToolbarItemRef	item = (HIToolbarItemRef) CFArrayGetValueAtIndex( items, i );
		CFStringRef			label;
		MenuItemIndex		newItem;
		
		HIToolbarItemCopyLabel( item, &label );
		AppendMenuItemTextWithCFString( menu, label, 0, kCmdToolbarItemsMenu, &newItem );
		CFRelease( label );
		
		SetMenuItemProperty( menu, newItem, kToolbarPropCreator, kToolbarItemRefID, sizeof( item ), &item );
	}
	
	CFRelease( items );
	
	menuType = kCtlIDEnablingButton;
	SetMenuItemProperty( menu, 0, kToolbarPropCreator, kToolbarMenuTypeID, sizeof( menuType ), &menuType );
	SetControlData( enablingButton, kControlNoPart, kControlPopupButtonOwnedMenuRefTag, sizeof( menu ), &menu );
	
	DuplicateMenu( menu, &menu2 );
	
	menuType = kCtlIDSelectionButton;
	SetMenuItemProperty( menu2, 0, kToolbarPropCreator, kToolbarMenuTypeID, sizeof( menuType ), &menuType );
	SetControlData( selectionButton, kControlNoPart, kControlPopupButtonOwnedMenuRefTag, sizeof( menu2 ), &menu2 );
	
	SetControlMaximum( enablingButton, count );
	SetControlMaximum( selectionButton, count );
}

//-----------------------------------------------------------------------------
//	CreateToolbarWindow
//-----------------------------------------------------------------------------
//
static void
CreateToolbarWindow()
{
	IBNibRef		nibRef;
	OSStatus		err;
	WindowRef		window;
	
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.

    err = CreateNibReference( CFSTR("main"), &nibRef );
    require_noerr( err, CantGetNibRef );
    
    err = CreateWindowFromNib( nibRef, CFSTR("MainWindow"), &window );
    require_noerr( err, CantCreateWindow );

	// Install a handler to deal with the toolbar menu

	InstallWindowEventHandler( window, CommandHandler, GetEventTypeCount( kCommandEvents ),
			kCommandEvents, window, NULL );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
	
    // The window was created hidden so show it.
    ShowWindow( window );

	// Install our toolbar
	InstallToolbar( window );

	// Install a handler so this window can track changes to the toolbar
	InstallEventHandler( HIObjectGetEventTarget( sToolbar ), ToolbarWindowHandler,
			GetEventTypeCount( kToolbarWindowEvents ), kToolbarWindowEvents, window, NULL );
	
	ReloadButtonMenus( window );
	
CantCreateWindow:
CantGetNibRef:
	return;
}
