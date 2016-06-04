/*
	File:		BasicDataBrowser.cp
	
	Contains:	Basic Data Browser sample, Carbon API

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

	Copyright © 2000-2005 Apple Computer, Inc., All Rights Reserved
*/

#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#ifndef __CARBON__
#include <Carbon.h>
#endif
#endif

// ANSI Headers
#include <limits.h>

// --------------------------------------------------------------------
// Globals
enum {
	kFolder,
	kDocument,
	kFolderAlias,
	
	kTotalNumIcons,
	
	kSettingsDialog = 130,
	kCustomizeDialog = 129
};
	
enum {
	kNoDisjoint = 1,
	kDragSelect,
	kAlwaysExtend,
	kSelectOnlyOne,
	kResetSelection,
	kCommandToggles,
	
	kActiveItems,
	kFrameAndFocus,
	kVerticalScroll,
	kHorizontalScroll,
	kAllowWindowResizing,
	
	kSettingsControlCount = kAllowWindowResizing
};

enum {
	kDisclosure = 1,
	kVarHeightRows,
	kExpandableRows,
	
	kPlainBackground,
	kFillHiliteStyle,
	kUseRelativeDates,
	kShowHeaderButtons,
	kColumnDividers,
	kButtonlessPopups,
	kAlternateRowColors,
	
	kCustomizeControlCount = kAlternateRowColors
};

enum { 
	kDBSettings			= 'stng',
	kCustomizeView		= 'cust',
	kShowPlacard		= 'plac',
};

enum {
	kItemsPerContainer	= 10,
	
	kMyCreator			= 'DEMO',
	kMyDataBrowser		= 'Brsr',
	kMyEventHandler		= 'Evnt',
	kMyCustomizerID		= ' ID ',

	kCheckboxColumn		= 'cbox',
	kFlavorColumn		= 'flvr',
	kColorColumn		= 'colr',
	kIndexColumn		= 'indx',
	kItemIDColumn		= 'item',
	kIconOnlyColumn		= 'icon',
	kDateTimeColumn		= 'date',
	kPopupMenuColumn	= 'menu',
	kProgressBarColumn	= 'pbar',
	kSliderColumn		= 'sldr'
};

static MenuRef menu = NULL;
static ControlRef placard = NULL;
static UInt16 headerBtnHeight = 0;
static IconRef icon[kTotalNumIcons] = {NULL};

enum { kBtnHeight = 20, kBtnWidth = 60, kScrollBarWidth = 16 };
	
#define Alias(itemID) (itemID == 44 || itemID == 486 || itemID == 509)

#define Container(itemID) (!((itemID % 5) == 2 || itemID > 10000) && !Alias(itemID))

const DataBrowserTableViewColumnID columns[] = 
	{ kCheckboxColumn, kFlavorColumn, kIconOnlyColumn, kColorColumn, kIndexColumn, 
	  kItemIDColumn, kDateTimeColumn, kSliderColumn, kProgressBarColumn, kPopupMenuColumn };

// --------------------------------------------------------------------
pascal void	MyPlacardDrawProc(ControlRef, SInt16);
static ControlUserPaneDrawUPP placardDrawProc = 
	NewControlUserPaneDrawUPP(MyPlacardDrawProc);

static void ConfigureDataBrowser(ControlRef);
static void InstallDataBrowserCallbacks(ControlRef);
static void CreateDataBrowser(WindowRef, ControlRef*);

static void SetupGlobals(void);
static void CleanupGlobals(void);
static OSStatus InstallStandardMenuBar(void);
static ControlRef GetDataBrowserFromWindow(WindowRef);
static bool SelectionFlagOn(ControlRef, DataBrowserSelectionFlags);
static bool PropertyFlagOn(ControlRef browser, DataBrowserPropertyID propertyID, DataBrowserPropertyFlags flag);
static bool AttributeOn(ControlRef browser, OptionBits attr);

OSStatus HandleDataBrowserCommand(ControlRef browser, HICommand command);
OSStatus UpdateDataBrowserCommand(ControlRef browser, HICommand command);

pascal OSStatus MyWindowEventHandler(EventHandlerCallRef, EventRef, void*);
pascal OSStatus MyControlEventHandler(EventHandlerCallRef, EventRef, void*);
pascal OSStatus MyDialogEventHandler(EventHandlerCallRef, EventRef, void*);
pascal OSStatus MyCommandEventHandler(EventHandlerCallRef, EventRef, void*);

// --------------------------------------------------------------------
int main(void)
{
    WindowRef window = NULL;
	ControlRef browser = NULL;
	OSStatus err = noErr;
	
    Rect bounds = { 100, 100, 350, 1000 };

    EventTypeSpec windowEvents[] = {
		{ kEventClassCommand,	kEventCommandProcess }, 
		{ kEventClassCommand,	kEventCommandUpdateStatus }, 

		{ kEventClassWindow,	kEventWindowClose }, 
		{ kEventClassWindow,	kEventWindowGetIdealSize },
		{ kEventClassWindow,	kEventWindowGetClickActivation }, 
		{ kEventClassWindow,	kEventWindowContextualMenuSelect } 
	};

	EventTypeSpec controlEvents[] = {
		{ kEventClassControl,	kEventControlBoundsChanged }
	};
	
    // Setup
    InitCursor();
    SetupGlobals();
    verify_noerr(InstallStandardMenuBar());

    // Setup a window
    verify_noerr(
    	CreateNewWindow(
            kDocumentWindowClass,
            kWindowInWindowMenuAttribute | 
            kWindowStandardHandlerAttribute | 
			kWindowLiveResizeAttribute |
			kWindowCompositingAttribute |
            kWindowStandardDocumentAttributes, &bounds, &window));
    
    ControlRef rootControl;
	verify_noerr(
    	CreateRootControl(window, &rootControl));
   		
    verify_noerr(
    	SetAutomaticControlDragTrackingEnabledForWindow(window, true));
    
    verify_noerr(InstallWindowEventHandler(window, 
    		NewEventHandlerUPP(MyWindowEventHandler), 
    		GetEventTypeCount(windowEvents), windowEvents, NULL, NULL));

    verify_noerr(
    	SetWindowTitleWithCFString(window, CFSTR("DataBrowser Demo")));
    
	//
	// Use the window's content view to resize the browser in response to window size changes.
	// This is preferred to using kEventWindowBoundsChanged because this allows for lag-free
	// updating of the browser control size.
	//
	HIViewRef contentView;
	HIViewFindByID( HIViewGetRoot(window), kHIViewWindowContentID, &contentView );
	verify_noerr( InstallControlEventHandler(contentView,
			NewEventHandlerUPP(MyControlEventHandler),
			GetEventTypeCount(controlEvents), controlEvents, window, NULL));
	
    // Create the DataBrowser
    CreateDataBrowser(window, &browser);
	InstallDataBrowserCallbacks(browser);

    // Configure the DataBrowser
	ConfigureDataBrowser(browser);
    err = SetDataBrowserTarget(browser, 50);
    
    // Set the keyboard focus
	SetKeyboardFocus(window, browser, kControlDataBrowserPart);
	
    // Store DB as a window property
    SetWindowProperty(window,
        kMyCreator, kMyDataBrowser,
        sizeof(browser), &browser);

    // Show window & run
    ShowWindow(window);
    
    RunApplicationEventLoop();
    
    CleanupGlobals();
    
    return 0;
}

#pragma mark -
#pragma mark • Dialogs •
// --------------------------------------------------------------------
static OSStatus GetDialogItemWithControlID(WindowRef window,
	int controlID, ControlRef* outControl )
{
	ControlID ctlID = { 'DEMO', controlID };
	return GetControlByID( window, &ctlID, outControl );
}

// --------------------------------------------------------------------
static void SetupControls(WindowRef window, UInt16 customizerID, int controlCount)
{
	ControlRef browser = 
		GetDataBrowserFromWindow(window);
	
	for (int controlID = 1; 
		controlID <= controlCount; controlID++)
	{
		Boolean controlValue = 0;
		ControlRef control = NULL;
		GetDialogItemWithControlID(window, controlID, &control );
		
		switch (customizerID)
		{
			case kSettingsDialog:
			{
				switch (controlID)
				{
					case kActiveItems:
					{	GetDataBrowserActiveItems(browser, &controlValue);
					}	break;
								
					case kFrameAndFocus:
					{	GetControlData(browser, kControlNoPart, 
							kControlDataBrowserIncludesFrameAndFocusTag, 
							sizeof(controlValue), &controlValue, NULL);
					}	break;
					
					case kVerticalScroll:
					case kHorizontalScroll:
					{	Boolean *vBar = NULL, *hBar = NULL;
						
						switch (controlID)
						{
							case kVerticalScroll:	vBar = &controlValue; break;
							case kHorizontalScroll:	hBar = &controlValue; break;
						}
						
						GetDataBrowserHasScrollBars(browser, hBar, vBar);
					}	break;
					
					case kDragSelect:	
						controlValue = SelectionFlagOn(browser, 
							kDataBrowserDragSelect); break;
					
					case kSelectOnlyOne:	
						controlValue = SelectionFlagOn(browser, 
							kDataBrowserSelectOnlyOne); break;
					
					case kResetSelection:	
						controlValue = SelectionFlagOn(browser, 
							kDataBrowserResetSelection); break;
					
					case kCommandToggles:	
						controlValue = SelectionFlagOn(browser, 
							kDataBrowserCmdTogglesSelection); break;
					
					case kNoDisjoint:	
						controlValue = SelectionFlagOn(browser, 
							kDataBrowserNoDisjointSelection); break;
					
					case kAlwaysExtend:	
						controlValue = SelectionFlagOn(browser, 
							kDataBrowserAlwaysExtendSelection); break;
							
					case kAllowWindowResizing:
						controlValue = AttributeOn(browser,
							kDataBrowserAttributeColumnViewResizeWindow); break;
				}
			}	break;
			
			case kCustomizeDialog:
			{	switch (controlID)
				{
					case kVarHeightRows:
					{	GetDataBrowserTableViewGeometry(
							browser, NULL, &controlValue);
						
						if (!controlValue) 
						{
							GetDialogItemWithControlID(window, 
								kExpandableRows, &control);
							DeactivateControl(control);
						}
					}	break;
					
					case kDisclosure:
					case kExpandableRows:
					{	Boolean expandableRows = false;
						DataBrowserPropertyID disclosureColumn = 0;
						
						GetDataBrowserListViewDisclosureColumn(
							browser, &disclosureColumn, &expandableRows);
					
						if (controlID != kDisclosure)
							controlValue = expandableRows;
						else if (disclosureColumn != 0) 
						{	const UInt16 
								numColumns = sizeof(columns) / 
								sizeof(DataBrowserPropertyID);
							for (UInt16 i = 0; i < numColumns; i++)
								if (disclosureColumn == columns[i])
								{ controlValue = i + 3; break; }
						}
					}	break;
					
					case kUseRelativeDates:
						controlValue = PropertyFlagOn(browser, kDateTimeColumn,
							kDataBrowserDateTimeRelative ); break;
					
					case kFillHiliteStyle:
					{	DataBrowserTableViewHiliteStyle hiliteStyle;
						GetDataBrowserTableViewHiliteStyle(browser, &hiliteStyle);
						controlValue = hiliteStyle;
					}	break;
					
					case kShowHeaderButtons:
					{	GetDataBrowserListViewHeaderBtnHeight(browser, &headerBtnHeight);
						controlValue = headerBtnHeight != 0;
					}	break;
					
					case kPlainBackground:
					{	GetDataBrowserListViewUsePlainBackground(browser, &controlValue);
					}	break;
					
					case kColumnDividers:
						controlValue = AttributeOn(browser,
							kDataBrowserAttributeListViewDrawColumnDividers); break;
							
					case kButtonlessPopups:
						controlValue = PropertyFlagOn(browser, kPopupMenuColumn,
							kDataBrowserPopupMenuButtonless ); break;
					
					case kAlternateRowColors:
						controlValue = AttributeOn(browser,
							kDataBrowserAttributeListViewAlternatingRowColors); break;
				}
			}	break;
		}
		
		SetControlValue(control, controlValue);
	}
}

// --------------------------------------------------------------------
static WindowRef GetCustomizer(ControlRef browser, UInt16 customizerID)
{
	WindowRef customizer = NULL;
	
	if (browser != NULL)
		GetControlProperty(browser, kMyCreator, 
		customizerID, sizeof(customizer), NULL, &customizer);
	
	return customizer;
}

// --------------------------------------------------------------------
static UInt16 GetCustomerID(WindowRef customizer)
{
	UInt16 customizerID = 0;
	GetWindowProperty(
		customizer, kMyCreator, kMyCustomizerID, 
		sizeof(customizerID), NULL, &customizerID);
	
	return customizerID;
}

// --------------------------------------------------------------------
static void SetupCustomizer(ControlRef browser, UInt16 customizerID, ItemCount controlCount)
{
	WindowRef window = 
		GetCustomizer(browser, customizerID);
	
	const EventTypeSpec events[] = 
		{ { kEventClassWindow, kEventWindowClose }, 
          { kEventClassControl, kEventControlHit }, 
          { kEventClassCommand, kEventCommandProcess}, 
          { kEventClassCommand, kEventCommandUpdateStatus} };
	
	if (window == NULL)
	{
		IBNibRef nib;
		CFStringRef windowName;
		
		if ( customizerID == kSettingsDialog )
			windowName = CFSTR("Settings");
		else if ( customizerID == kCustomizeDialog )
			windowName = CFSTR("Customize");
		
		require_noerr( CreateNibReference( CFSTR("BasicDataBrowser"), &nib ), CouldntCreateNib );
		require_noerr( CreateWindowFromNib( nib, windowName, &window ), CouldntCreateWindow );

		{
			// Store window as a browser property
			SetControlProperty(browser,
				kMyCreator, customizerID,
				sizeof(window), &window);

			// Store browser as a window property
			SetWindowProperty(window, kMyCreator, 
				kMyDataBrowser,  sizeof(browser), &browser);
			
			// Setup initial control values
			SetupControls(window, customizerID, controlCount);
			
			// Store event handler as a window property
			EventHandlerUPP eventHandler = 
				NewEventHandlerUPP(MyDialogEventHandler);
				
			verify_noerr(InstallWindowEventHandler(window, 
				eventHandler, GetEventTypeCount(events), events, NULL, NULL));
		   
			SetWindowProperty(window, kMyCreator, 
				kMyEventHandler, sizeof(eventHandler), &eventHandler);
			
			// Store customizerID as a window property
			SetWindowProperty(window, kMyCreator, 
				kMyCustomizerID,  sizeof(customizerID), &customizerID);
		}
		
CouldntCreateWindow:
		DisposeNibReference( nib );
	}
	
	ShowWindow(window);
	SelectWindow(window);
	
CouldntCreateNib:
	return;
}

#pragma mark -
#pragma mark • Utilities •
// --------------------------------------------------------------------
static void SetupGlobals(void)
{
	icon[kFolder] = NULL;
	::GetIconRef(kOnSystemDisk, 
		kSystemIconsCreator, kGenericFolderIcon, &icon[kFolder]);
	
	icon[kDocument] = NULL;
	::GetIconRef(kOnSystemDisk, 
		kSystemIconsCreator, kGenericDocumentIcon, &icon[kDocument]);
	
	icon[kFolderAlias] = NULL;
	IconRef aliasBadgeIcon = NULL; ::GetIconRef(kOnSystemDisk, 
		kSystemIconsCreator, kAliasBadgeIcon, &aliasBadgeIcon);
	::CompositeIconRef(icon[kFolder], aliasBadgeIcon, &icon[kFolderAlias]);
	if (aliasBadgeIcon != NULL) ::ReleaseIconRef(aliasBadgeIcon);
}

// --------------------------------------------------------------------
static void CleanupGlobals(void)
{
	if (menu != NULL)
		::DisposeMenu(menu);
	
	for (UInt32 i = 0; i < kTotalNumIcons; i++)
		if (icon[i] != NULL) ::ReleaseIconRef(icon[i]);
}

// --------------------------------------------------------------------
static void GenerateString(DataBrowserItemID itemID, 
	DataBrowserPropertyID property, StringPtr string)
{
	string[0] = 0; Str255 numString; 
	::NumToString(itemID, numString);
	
	Boolean itemXProperty = property == kItemIDColumn || 
		property == kDataBrowserItemSelfIdentityProperty;
		
	if (itemXProperty) PLstrcat(string, "\pItem ");
		
	PLstrcat(string, numString);
	
	if (!itemXProperty)
	{
		numString[0] = sizeof(property);
		::BlockMoveData(&property, &(numString[1]), sizeof(property));
		PLstrcat(string, "\p, "); PLstrcat(string, numString);
	}
}

// --------------------------------------------------------------------
static bool SelectionFlagOn(
	ControlRef browser, DataBrowserSelectionFlags flag)
{
	DataBrowserSelectionFlags flags;
	GetDataBrowserSelectionFlags(browser, &flags);
	
	return (flags & flag);
}

// --------------------------------------------------------------------
static bool PropertyFlagOn(
	ControlRef browser, DataBrowserPropertyID propertyID, DataBrowserPropertyFlags flag)
{	
	DataBrowserPropertyFlags flags = 0;
	GetDataBrowserPropertyFlags(
		browser, propertyID, &flags);
		
	return (flags & flag);
}

// --------------------------------------------------------------------
static bool AttributeOn(
	ControlRef browser, OptionBits attr)
{
	OptionBits attrs;
	DataBrowserGetAttributes( browser, &attrs );
	
	return (attrs & attr);
}

// --------------------------------------------------------------------
static void ActivateControl(WindowRef window, 
	int controlID, int numItems, const int items[])
{
	bool active = true;
	ControlRef control;
	
	for (int i = 0; active && i < numItems; i++)
	{	verify_noerr(GetDialogItemWithControlID(
			window, items[i], &control));
		
		active &= !GetControlValue(control);
	}
	
	verify_noerr(GetDialogItemWithControlID(
		window, controlID, &control));
	
	if (active) 
		verify_noerr(ActivateControl(control));
	else verify_noerr(DeactivateControl(control));
}

// --------------------------------------------------------------------
static bool ToggleSelectionFlag(ControlRef browser, 
	DataBrowserSelectionFlags flag, ControlRef control)
{
	DataBrowserSelectionFlags flags;
	GetDataBrowserSelectionFlags(browser, &flags);
	SetDataBrowserSelectionFlags(browser, flags ^ flag);
	
	// Make sure flag and representative 
	SetControlValue(control, // control are synch'd
		(flags & flag) ? kThemeButtonOff : kThemeButtonOn);
	
	return !(flags & flag);
}

// --------------------------------------------------------------------
static bool TogglePropertyFlag(ControlRef browser,
	DataBrowserPropertyID propertyID, DataBrowserPropertyFlags flag)
{	
	DataBrowserPropertyFlags flags = 0;
	GetDataBrowserPropertyFlags(
		browser, propertyID, &flags);
	SetDataBrowserPropertyFlags(browser, 
		propertyID, flags ^ flag);
		
	return !(flags & flag);
}

// --------------------------------------------------------------------
static bool ToggleAttribute(ControlRef browser, 
	OptionBits attr, ControlRef control)
{
	OptionBits attrs;
	DataBrowserGetAttributes( browser, &attrs );
	attrs &= attr;
	DataBrowserChangeAttributes( browser, attrs ^ attr, attrs );
	
	// Make sure flag and representative 
	SetControlValue(control, // control are synch'd
		(attrs & attr) ? kThemeButtonOff : kThemeButtonOn);
	
	return !(attrs & attr);
}

// --------------------------------------------------------------------
static void InvalidateControl(ControlRef control)
{
	if ( HIViewIsCompositingEnabled(control))
	{
		HIViewSetNeedsDisplay(control, true);
	}
	else
	{
		Rect bounds;
		GetControlBounds(control, &bounds);
		InvalWindowRect(GetControlOwner(control), &bounds);
	}
}

// --------------------------------------------------------------------
OSStatus InstallStandardMenuBar()
{
	IBNibRef		nib;
	
	if ( CreateNibReference( CFSTR("BasicDataBrowser"), &nib ) == noErr )
	{
		SetMenuBarFromNib( nib, CFSTR("MainMenu") );
		CreateMenuFromNib( nib, CFSTR("ContextMenu"), &menu );
		DisposeNibReference( nib );
	}
	
    return noErr;
}

// --------------------------------------------------------------------
static ControlRef GetDataBrowserFromWindow(WindowRef window)
{
	ControlRef browser = NULL;
	
	if (window != NULL)
		GetWindowProperty(window, kMyCreator, 
		kMyDataBrowser, sizeof(browser), NULL, &browser);
	
	return browser;
}

#pragma mark -
#pragma mark • Handlers •
// --------------------------------------------------------------------
static OSStatus HandleWindowCommand(EventRef inEvent)
{
	HICommand command;
    OSStatus result = eventNotHandledErr;
	
	GetEventParameter(inEvent, 
		kEventParamDirectObject, typeHICommand, 
		NULL, sizeof(command), NULL, &command);
	
	WindowRef window = GetUserFocusWindow();
	check(command.attributes & kHICommandFromMenu);
	ControlRef browser = GetDataBrowserFromWindow(window);
	
	switch (GetEventKind(inEvent))
	{
		case kEventCommandProcess:
		{
			if (command.commandID == kHICommandClose)
			{
				result = noErr;
				
				EventRef event; CreateEvent(
					NULL, kEventClassWindow, 
					kEventWindowClose, GetCurrentEventTime(), 
					kEventAttributeUserEvent, &event);
				
				SetEventParameter(
					event, kEventParamDirectObject, 
			        typeWindowRef, sizeof(window), &window);
				
				SendEventToWindow(event, GetUserFocusWindow());
			}
			else result = HandleDataBrowserCommand(browser, command);
		}	break;
		
		case kEventCommandUpdateStatus:
		{	result = UpdateDataBrowserCommand(browser, command);
		}	break;
	}
	
	return result;
}

// --------------------------------------------------------------------
static void HandleCustomizerControl(ControlRef control)
{
	WindowRef window = 
		GetControlOwner(control);
	
	Boolean controlValue = 
		GetControlValue(control);
	
	ControlRef browser = 
		GetDataBrowserFromWindow(window);
		
	UInt32 controlID;
	GetControlCommandID(control, &controlID);
	
	switch (GetCustomerID(window))
	{
		case kCustomizeDialog:
		{
			switch (controlID)
			{
				case kDisclosure:
				case kExpandableRows:
				{	Boolean expandableRows = false;
					DataBrowserPropertyID disclosureColumn = 0;
					
					GetDataBrowserListViewDisclosureColumn(
						browser, &disclosureColumn, &expandableRows);
					
					if (controlID != kDisclosure)
						expandableRows = controlValue;
					else if (controlValue < 3)
						disclosureColumn = kDataBrowserItemNoProperty;
					else disclosureColumn = columns[controlValue-3];
					
					SetDataBrowserListViewDisclosureColumn(
						browser, disclosureColumn, expandableRows);
				}	break;
				
				case kShowHeaderButtons:
				{	SetDataBrowserListViewHeaderBtnHeight(
						browser, controlValue ? headerBtnHeight : 0);
				}	break;
				
				case kFillHiliteStyle:
				{	SetDataBrowserTableViewHiliteStyle(browser, controlValue);
				}	break;
				
				case kUseRelativeDates:
				{	TogglePropertyFlag(browser, kDateTimeColumn, kDataBrowserDateTimeRelative);
				}	break;
				
				case kVarHeightRows:
				{	GetDataBrowserTableViewGeometry(browser, NULL, &controlValue);
					SetDataBrowserTableViewGeometry(browser, true, !controlValue);
					
					SetControlValue(control, !controlValue);
					
					GetDialogItemWithControlID(
						GetControlOwner(control), 
						kExpandableRows, &control);
					
					if (!controlValue) 
						ActivateControl(control);
					else DeactivateControl(control);
				}	break;
				
				case kPlainBackground:
				{	SetDataBrowserListViewUsePlainBackground(browser, controlValue);
				}	break;
				
				case kColumnDividers:
				{	ToggleAttribute(browser, kDataBrowserAttributeListViewDrawColumnDividers, control);
					InvalidateControl(browser);
				}	break;
				
				case kButtonlessPopups:
				{	TogglePropertyFlag(browser, kPopupMenuColumn, kDataBrowserPopupMenuButtonless );
				}	break;
				
				case kAlternateRowColors:
				{	ToggleAttribute(browser, kDataBrowserAttributeListViewAlternatingRowColors, control);
					InvalidateControl(browser);
				}	break;
			}
		}	break;
		
		case kSettingsDialog:
		{
			switch (controlID)
			{
				case kFrameAndFocus:
				{	SetControlData(browser, kControlNoPart, 
						kControlDataBrowserIncludesFrameAndFocusTag, 
						sizeof(controlValue), &controlValue);
				}	break;

				case kActiveItems:
				{	SetDataBrowserActiveItems(browser, controlValue);
				}	break;
				
				case kVerticalScroll:
				case kHorizontalScroll:
				{	Boolean vBar, hBar;
					GetDataBrowserHasScrollBars(browser, &hBar, &vBar);
					
					switch (controlID)
					{
						case kVerticalScroll:	vBar = !vBar; break;
						case kHorizontalScroll:	hBar = !hBar; break;
					}
					
					SetDataBrowserHasScrollBars(browser, hBar, vBar);
					
					if (hBar || vBar)
					{	Rect sInset = { 0, 0, 0, 0 }; 
						GetDataBrowserScrollBarInset(browser, &sInset);
						
						if (hBar) sInset.right = kScrollBarWidth - 1;
						if (vBar) sInset.bottom = kScrollBarWidth - 1;
						SetDataBrowserScrollBarInset(browser, &sInset);
					}
				}	break;
				
				case kDragSelect:
				{	ToggleSelectionFlag(browser, kDataBrowserDragSelect, control);
				}	break;
				
				case kSelectOnlyOne:
				{	ToggleSelectionFlag(browser, kDataBrowserSelectOnlyOne, control);
				}	break;
				
				case kResetSelection:
				{	ToggleSelectionFlag(browser, kDataBrowserResetSelection, control);
				}	break;
				
				case kCommandToggles:
				{	ToggleSelectionFlag(browser, kDataBrowserCmdTogglesSelection, control);
				}	break;
				
				case kNoDisjoint:
				{	ToggleSelectionFlag(browser, kDataBrowserNoDisjointSelection, control);
				}	break;
				
				case kAlwaysExtend:
				{	ToggleSelectionFlag(browser, kDataBrowserAlwaysExtendSelection, control);
				}	break;
				
				case kAllowWindowResizing:
				{	ToggleAttribute(browser, kDataBrowserAttributeColumnViewResizeWindow, control);
				}	break;
			}
			
			switch (controlID)
			{
				case kNoDisjoint:
				case kSelectOnlyOne:
				{
					const int dragSelectOverrides[] = 
						{ kNoDisjoint, kSelectOnlyOne };
					ActivateControl(window, kDragSelect, 
						sizeof(dragSelectOverrides)/sizeof(dragSelectOverrides[0]), dragSelectOverrides);
				}	require_quiet(controlID == kSelectOnlyOne, kCommandToggles);
				
				case kResetSelection:
				{
					const int alwaysExtendOverrides[] = 
						{ kSelectOnlyOne, kResetSelection };
					ActivateControl(window, kAlwaysExtend, 
						sizeof(alwaysExtendOverrides)/sizeof(alwaysExtendOverrides[0]), alwaysExtendOverrides);
				}
				
				kCommandToggles:
				case kCommandToggles:
				// kCommandToggles is mutated by the others
				{	ControlRef cmdTogglesCntrl;
					verify_noerr(GetDialogItemWithControlID(
						window, kCommandToggles, &cmdTogglesCntrl));
					
					if (GetControlValue(cmdTogglesCntrl))
					{
						ThemeButtonValue value = kThemeButtonOn;
						const int overrides[] = { kNoDisjoint, 
							kSelectOnlyOne, kResetSelection };
					
						for (int i = 0; 
							i < (int) (sizeof(overrides)/sizeof(overrides[0])) && 
							value == kThemeButtonOn; i++)
						{
							verify_noerr(GetDialogItemWithControlID(
								window, overrides[i], &control));
							if (GetControlValue(control)) 
								value = kThemeButtonMixed;
						}
						
						SetControlValue(cmdTogglesCntrl, value);
					}
				}	break;
			}
		}	break;
	}
}

// --------------------------------------------------------------------
OSStatus HandleDataBrowserCommand(ControlRef browser, HICommand command)
{
    OSStatus result = noErr;
	
	switch (command.commandID)
	{
		default:
		{	result = eventNotHandledErr;
		}	break;
		
		case kShowPlacard:
		{	Rect sBarInset, placardBounds = { 0 };
			GetDataBrowserScrollBarInset(browser, &sBarInset);

			if (placard != NULL)
			{	GetControlBounds(
					placard, &placardBounds);
				DisposeControl(placard); placard = NULL;
				sBarInset.left -= placardBounds.right - placardBounds.left;
			}
			else
			{	GetControlBounds(browser, &placardBounds);
				placardBounds.top = placardBounds.bottom - sBarInset.bottom;
				placardBounds.right = placardBounds.left + 120;
				
				verify_noerr(CreateUserPaneControl(
					GetControlOwner(browser), &placardBounds, 0, &placard));
				
				SetControlReference(placard, (long)browser);
				verify_noerr(EmbedControl(placard, browser));
				sBarInset.left += placardBounds.right - placardBounds.left;
				
				ControlUserPaneDrawUPP drawProc = placardDrawProc;
				verify_noerr(SetControlData(placard, kControlNoPart, 
					kControlUserPaneDrawProcTag, sizeof(drawProc), &drawProc));
					
				SetControlVisibility(placard, true, true);
			}
			verify_noerr(SetDataBrowserScrollBarInset(browser, &sBarInset));
		}	break;
		
	/*	case kHICommandCut:
		case kHICommandUndo:
		case kHICommandRedo:
		case kHICommandCopy:
		case kHICommandPaste:
		case kHICommandClear:
		case kHICommandSelectAll:
		{	ExecuteDataBrowserEditCommand(
				browser, command.commandID);
		}	break;	*/
		
		case kDataBrowserListView:
		case kDataBrowserColumnView:
		{	DataBrowserViewStyle view;
			GetDataBrowserViewStyle(browser, &view);
			
			if (view != command.commandID)
			{
				SetDataBrowserViewStyle(
					browser, command.commandID);
				ConfigureDataBrowser(browser);
			}
		}	break;
		
		case kDBSettings:
		{	SetupCustomizer(browser, kSettingsDialog, kSettingsControlCount);
		}	break;
		
		case kCustomizeView:
		{	SetupCustomizer(browser, kCustomizeDialog, kCustomizeControlCount);
		}	break;
	}
	
	return result;
}

// --------------------------------------------------------------------
OSStatus UpdateDataBrowserCommand(ControlRef browser, HICommand command)
{
	OSStatus result = noErr;
	bool commandIsEnabled = true;
	
	switch (command.commandID)
	{
		default:
		{	result = eventNotHandledErr;
		}	break;
		
		case kShowPlacard:
		{	CheckMenuItem(command.menu.menuRef, 
				command.menu.menuItemIndex, placard != NULL);
		}	break;
		
	/*	case kHICommandCut:
		case kHICommandUndo:
		case kHICommandRedo:
		case kHICommandCopy:
		case kHICommandPaste:
		case kHICommandClear:
		case kHICommandSelectAll:
		{	commandIsEnabled = 
				EnableDataBrowserEditCommand(
				browser, command.commandID);
		}	break;	*/

		case kCustomizeView:
		{	DataBrowserViewStyle viewStyle;
			::GetDataBrowserViewStyle(browser, &viewStyle);
			commandIsEnabled = (viewStyle == kDataBrowserListView);
		}	break;
	}
	
	if (result == noErr)
	{
		if (commandIsEnabled)
			EnableMenuCommand(command.menu.menuRef, command.commandID);
		else DisableMenuCommand(command.menu.menuRef, command.commandID);
	}
	
	return result;		
}

#pragma mark -
#pragma mark • DB Callbacks •
// --------------------------------------------------------------------
static pascal Boolean MyAcceptDrag(ControlRef browser, 
	DragReference theDrag, DataBrowserItemID itemID)
{
	bool accept = Container(itemID);
	
	SInt16 modifiers = 0; 
	ThemeCursor cursor = kThemeArrowCursor;
	::GetDragModifiers(theDrag, &modifiers, NULL, NULL);
	
	switch (modifiers & ~btnState)	// Filter out btnState (on for drop)
	{
		case cmdKey:
		{	DataBrowserItemID target = kDataBrowserNoItem;
			::GetDataBrowserTarget(browser, &target);
			if (itemID != target) accept = false;
		}	break;
		
		case optionKey:
		{	cursor = kThemeCopyArrowCursor;
		}	break;
		
		case cmdKey | optionKey:
		{	cursor = kThemeAliasArrowCursor;
		}	break;
	}
	
	::SetThemeCursor(cursor);
	
	return accept;
}

// --------------------------------------------------------------------
static pascal Boolean MyReceiveDrag(ControlRef browser, 
	DragReference theDrag, DataBrowserItemID itemID)
{
	DataBrowserItemState itemState = 0L;
	
	if (itemID == kDataBrowserNoItem)
		itemState = kDataBrowserContainerIsOpen;
	else ::GetDataBrowserItemState(browser, itemID, &itemState);
	
	if (true || itemState & kDataBrowserContainerIsOpen)
	{
		UInt16 numItems; ::CountDragItems(theDrag, &numItems);
		
		for (UInt32 i = 1; i <= numItems; i++)
		{
			ItemReference theItemRef;
			DataBrowserItemID draggedItem;
			Size dataSize = sizeof(draggedItem);
			::GetDragItemReferenceNumber(theDrag, i, &theItemRef);
			::GetFlavorData(theDrag, theItemRef, typeUInt32, &draggedItem, &dataSize, 0);

			::RemoveDataBrowserItems(browser, kDataBrowserNoItem, 
				1, &draggedItem, kDataBrowserItemNoProperty);

			::AddDataBrowserItems(browser, itemID, 
				1, &draggedItem, kDataBrowserItemNoProperty);
		}
	}
/*	else
	{
		AlertStdAlertParamRec alertParameters;
		
		alertParameters.movable			= true;
		alertParameters.helpButton		= false;
		alertParameters.filterProc		= TEventDispatcher::ModalFilter();
		alertParameters.defaultText		= (StringPtr)kAlertDefaultOKText;
		alertParameters.cancelText		= NULL;
		alertParameters.otherText		= NULL;
		alertParameters.defaultButton	= kAlertStdAlertOKButton;
		alertParameters.cancelButton	= 0;
		alertParameters.position		= kWindowDefaultPosition;

		SInt16 itemHit;
		TPascalString theString("\pDrag received in ");
		
		if (itemID != 0L)
		{
			Str255 tempString;
			::NumToString(itemID, tempString);
			::AppendToString(theString, "\pItem ");
			::AppendToString(theString, tempString);
		}
		else ::AppendToString(theString, "\plist");
		
		::StandardAlert(kAlertPlainAlert, theString, NULL, &alertParameters, &itemHit);
	}	*/
	
	return true;
}

// --------------------------------------------------------------------
static pascal Boolean MyAddDragItem(ControlRef browser, 
	DragReference theDrag, DataBrowserItemID itemID, ItemReference* itemRef)
{
#pragma unused (browser)
	Str255 string;
	::GetDragItemReferenceNumber(theDrag, 0, itemRef);
	::GenerateString(itemID, 'col4', string); ::PLstrcat(string, "\p\n");
	::AddDragItemFlavor(theDrag, *itemRef, 'TEXT', &(string[1]), string[0], 0);
	::AddDragItemFlavor(theDrag, *itemRef, typeUInt32, &itemID, sizeof(itemID), 0);
	return true;
}

// --------------------------------------------------------------------
static pascal OSStatus MyGetSetItemData(ControlRef browser, 
    DataBrowserItemID itemID, DataBrowserPropertyID property, 
    DataBrowserItemDataRef itemData, Boolean changeValue)
{
#pragma unused (browser)
	Str255 pascalString;
	OSStatus err = noErr;
	
	if (!changeValue)  switch (property)
	{
		case kCheckboxColumn:
		if ((itemID % 5) == 2)
		{	err = ::SetDataBrowserItemDataButtonValue(itemData, kThemeButtonOn);
			err = ::SetDataBrowserItemDataDrawState(itemData, kThemeStateInactive);
		}	break;
		
		case kFlavorColumn:
		{	::GetIndString(pascalString, 128, itemID % 5 + 1);
			CFStringRef text = ::CFStringCreateWithPascalString(
				kCFAllocatorDefault, pascalString, kCFStringEncodingMacRoman);
			
			err = ::SetDataBrowserItemDataText(itemData, text); ::CFRelease(text);
		}	// Fall through to kIconOnlyColumn
		
		case kIconOnlyColumn:
		{	err = ::SetDataBrowserItemDataIcon(itemData, Container(itemID) ? 
				icon[kFolder] : Alias(itemID) ? icon[kFolderAlias] : icon[kDocument]);
		}	break;
		
		case kColorColumn:
		{	::GetIndString(pascalString, 129, itemID % 5 + 1);
			CFStringRef text = ::CFStringCreateWithPascalString(
				kCFAllocatorDefault, pascalString, kCFStringEncodingMacRoman);
			err = ::SetDataBrowserItemDataText(itemData, text); ::CFRelease(text);
		}	break;
		
		case kIndexColumn:
		{	SInt16 mod5 = itemID % 5;
			if (mod5 == 0) mod5 = 5;
			::NumToString(mod5, pascalString);
			CFStringRef text = ::CFStringCreateWithPascalString(
				kCFAllocatorDefault, pascalString, kCFStringEncodingMacRoman);
			err = ::SetDataBrowserItemDataText(itemData, text); ::CFRelease(text);
		}	break;
		
		case kDateTimeColumn:
		{	LongDateCvt dt;
			dt.hl.lHigh = 0;
			GetDateTime( &dt.hl.lLow );
			dt.hl.lLow -= (((itemID - 1) % 10) * 28800 );
			err = ::SetDataBrowserItemDataLongDateTime(itemData, &dt.c );
		}	break;
		
		case kSliderColumn:
		case kProgressBarColumn:
		{	err = ::SetDataBrowserItemDataValue(itemData, (itemID % 5) * 20);
		}	break;
		
		case kPopupMenuColumn:
		{	if ((itemID % 5 + 1) != 1)
			{	err = ::SetDataBrowserItemDataMenuRef(itemData, menu);
			}
			err = ::SetDataBrowserItemDataValue(itemData, itemID % 5 + 1);
		}	break;
		
		case kDataBrowserItemSelfIdentityProperty:
		{	err = ::SetDataBrowserItemDataIcon(itemData, Container(itemID) ? 
				icon[kFolder] : Alias(itemID) ? icon[kFolderAlias] : icon[kDocument]);
		}	// Fall through to text generator
		
		case kItemIDColumn:
		{	GenerateString(itemID, property, pascalString);
			CFStringRef text = ::CFStringCreateWithPascalString(
				kCFAllocatorDefault, pascalString, kCFStringEncodingMacRoman);
			err = ::SetDataBrowserItemDataText(itemData, text); ::CFRelease(text);
		}	break;
		
		case kDataBrowserItemIsActiveProperty:
		if ((itemID % 5) == 3)
		{	err = ::SetDataBrowserItemDataBooleanValue(itemData, false);
		}	break;
		
		case kDataBrowserItemIsEditableProperty:
		{	err = ::SetDataBrowserItemDataBooleanValue(itemData, true);
		}	break;
		
		case kDataBrowserItemIsContainerProperty:
		{	err = ::SetDataBrowserItemDataBooleanValue(itemData, Container(itemID));
		}	break;
		
		case kDataBrowserContainerAliasIDProperty:
		if (Alias(itemID))
		{	err = ::SetDataBrowserItemDataItemID(itemData, 4);
		}	break;
		
		case kDataBrowserItemParentContainerProperty:
		{	err = ::SetDataBrowserItemDataItemID(itemData, (itemID-1) / kItemsPerContainer);
		}	break;
		
		default:
		{	err = errDataBrowserPropertyNotSupported;
		}	break;
	}
	else err = errDataBrowserPropertyNotSupported;
	
	return err;
}

// ---------------------------------------------------------------------------------
static pascal Boolean MyItemComparison(
	ControlRef browser, DataBrowserItemID itemOneID, 
	DataBrowserItemID itemTwoID, DataBrowserPropertyID sortProperty)
{
	SInt16 compareResult = 0;
	
	#define Compare(i1,i2,p) MyItemComparison(browser,i1,i2,p)
	
	switch (sortProperty)
	{
		case kFlavorColumn:
		{	Str255 s1, s2;
			::GetIndString(s1, 128, itemOneID % 5 + 1);
			::GetIndString(s2, 128, itemTwoID % 5 + 1);
			compareResult = ::CompareString(s1, s2, NULL);
			
			if (compareResult < 0) return true;
			else if (compareResult > 0) return false;
			else return Compare(itemOneID, itemTwoID, '????');
		}	break;
		
		case kColorColumn:
		{	Str255 s1, s2;
			::GetIndString(s1, 129, itemOneID % 5 + 1);
			::GetIndString(s2, 129, itemTwoID % 5 + 1);
			compareResult = ::CompareString(s1, s2, NULL);
			
			if (compareResult < 0) return true;
			else if (compareResult > 0) return false;
			else return Compare(itemOneID, itemTwoID, 'foo ');
		}	break;

		case kProgressBarColumn:
		{	SInt32 val1 = (itemOneID % 5) * 20;
			SInt32 val2 = (itemTwoID % 5) * 20;
			
			if (val1 < val2) return true;
			else if (val1 > val2) return false;
			else return Compare(itemOneID, itemTwoID, '????');
		}	break;
		
		default:
		{	return itemOneID < itemTwoID;
		}	break;
	}
}

// --------------------------------------------------------------------
static pascal void MyItemNotification(
	ControlRef browser, 
	DataBrowserItemID itemID, 
	DataBrowserItemNotification message)
{
	switch (message)
	{
		case kDataBrowserContainerOpened:
		{	// Generate some valid itemIDs
			DataBrowserItemID myItems[kItemsPerContainer];
			for (UInt32 i = 0; i < kItemsPerContainer; i++)
				myItems[i] = itemID * kItemsPerContainer + i + 1;
			
			AddDataBrowserItems(browser, itemID, kItemsPerContainer, myItems, kIndexColumn);
			
			{	// Set up variable height rows
				Boolean variableHeightRows;
				GetDataBrowserTableViewGeometry(
					browser, NULL, &variableHeightRows);
					
				if (variableHeightRows)
					for (UInt16 i = 0; i < kItemsPerContainer; i++)
						SetDataBrowserTableViewItemRowHeight(
							browser, myItems[i], 20 + (myItems[i] - 1) % 10 * 3);
			}
		}	break;
		
		case kDataBrowserSelectionSetChanged:
		{	if ( placard != NULL )
				InvalidateControl( placard );
		}	break;
	}
}

// --------------------------------------------------------------------
static pascal void MyItemHelpContent(
	ControlRef browser, DataBrowserItemID item, 
	DataBrowserPropertyID property, HMContentRequest inRequest, 
	HMContentProvidedType *outContentProvided, HMHelpContentPtr ioHelpContent)
{
#pragma unused (browser)
	bool helpContentProperty = (property == kFlavorColumn) || 
		(property == kDataBrowserItemSelfIdentityProperty);
	
	if (inRequest == kHMSupplyContent && item != kDataBrowserNoItem && helpContentProperty)
	{
		*outContentProvided = kHMContentProvided;
		ioHelpContent->content[kHMMaximumContentIndex].contentType = kHMStringResContent;
		ioHelpContent->content[kHMMaximumContentIndex].u.tagStringRes.hmmIndex = item % 5 + 1;
		ioHelpContent->content[kHMMaximumContentIndex].u.tagStringRes.hmmResID = 130;
	}
}

// --------------------------------------------------------------------
static pascal void MyGetContextualMenu(
	ControlRef browser, MenuRef* contextualMenu, UInt32 *helpType, 
	CFStringRef* helpItemString, AEDesc *selection)
{
#pragma unused (browser, selection)
	*helpItemString = CFSTR( "Testing 1-2-3" );
	
	*helpType = kCMHelpItemOtherHelp;

	if (menu != NULL)
		*contextualMenu = menu;
}

// --------------------------------------------------------------------
static pascal void MySelectContextualMenu(
	ControlRef browser, MenuRef contextualMenu, 
	UInt32 selectionType, SInt16 menuID, MenuItemIndex menuItem)
{
#pragma unused (browser, selectionType, menuID, menuItem)
}

#pragma mark -
#pragma mark • Window Callbacks •
// --------------------------------------------------------------------
pascal void	MyPlacardDrawProc(ControlRef control, SInt16)
{
	ControlRef browser = (ControlRef)
		GetControlReference(control);
	
	UInt32 numSelectedItems;
	GetDataBrowserItemCount(browser, kDataBrowserNoItem, 
		true, kDataBrowserItemIsSelected, &numSelectedItems);
	
	CFStringRef placardString = CFStringCreateWithFormat( NULL, NULL, CFSTR(" Items selected: %d"), numSelectedItems );
	
	Rect bounds;
	if ( HIViewIsCompositingEnabled( control ) )
	{
		HIRect r;
		HIViewGetBounds( control, &r );
		SetRect( &bounds, (short) r.origin.x, (short) r.origin.y, (short) CGRectGetMaxX( r ), (short) CGRectGetMaxY( r ) );
	}
	else
	{
		GetControlBounds( control, &bounds );
	}
	
	DrawThemePlacard(&bounds, IsControlActive(control) ? kThemeStateActive : kThemeStateInactive);

	SetThemeTextColor(
		IsControlActive(control) ? 
		kThemeTextColorPlacardActive : 
		kThemeTextColorPlacardInactive, 8, true);

	DrawThemeTextBox( placardString, kThemeSmallSystemFont, kThemeStateActive, false, &bounds, teCenter, NULL );
	CFRelease( placardString );
}

// --------------------------------------------------------------------
pascal OSStatus MyDialogEventHandler(
	EventHandlerCallRef, EventRef inEvent, void*)
{
    OSStatus result = eventNotHandledErr;
	
	switch (GetEventClass(inEvent))
	{
		case kEventClassCommand:
		{   result = HandleWindowCommand(inEvent);
		}	break;
		
		case kEventClassControl:
		{	result = noErr;
		
			ControlRef control = NULL;
			GetEventParameter(inEvent, 
				kEventParamDirectObject, typeControlRef, 
				NULL, sizeof(control), NULL, &control);
		
			HandleCustomizerControl(control);
		}	break;
		
	    case kEventClassWindow:
		{   WindowRef window = NULL;
			GetEventParameter(inEvent, 
		        kEventParamDirectObject, typeWindowRef, 
		        NULL, sizeof(window), NULL, &window);

		    switch (GetEventKind(inEvent))
		 	{
				case kEventWindowClose:
		     	{	result = noErr;
		     	
		     		ControlRef browser = 
		     			GetDataBrowserFromWindow(window);
		     		UInt16 customizerID = GetCustomerID(window);
		     			
		     		EventHandlerUPP eventHandler = NULL;
					GetWindowProperty(
						window, kMyCreator, kMyEventHandler, 
						sizeof(eventHandler), NULL, &eventHandler);
		     		
		     		DisposeEventHandlerUPP(eventHandler);
		     		DisposeWindow(window); window = NULL;
		     		
				    SetControlProperty(browser, kMyCreator, 
				        customizerID, sizeof(window), &window);
		     	}	break;
			}
		}
	}
	   
    return result;
}

// --------------------------------------------------------------------
pascal OSStatus MyWindowEventHandler(
	EventHandlerCallRef, EventRef inEvent, void*)
{
    OSStatus result = noErr;

	switch (GetEventClass(inEvent))
	{
    	default:
    	{	result = eventNotHandledErr;
    	}	break;
		
		case kEventClassCommand:
		{   result = HandleWindowCommand(inEvent);
		}	break;
		
	    case kEventClassWindow:
		{
		    WindowRef window = NULL;
			GetEventParameter(inEvent, 
		        kEventParamDirectObject, typeWindowRef, 
		        NULL, sizeof(window), NULL, &window);

		    switch (GetEventKind(inEvent))
		 	{
		    	default:
		    	{	result = eventNotHandledErr;
		    	}	break;
				
				case kEventWindowClose:
		     	{
		     		ControlRef browser = 
		     			GetDataBrowserFromWindow(window);
		     		
		     		const UInt16 IDs[] = { 
		     			kSettingsDialog, kCustomizeDialog };
		     		
		     		for (UInt16 i = 0; i < sizeof(IDs)/sizeof(IDs[0]); i++)
		     		{
			     		const UInt16 ID = IDs[i];
			     		WindowRef customizer = 
			     			GetCustomizer(browser, ID);
			     		
			     		if (customizer != NULL)
							DisposeWindow( customizer );
		     		}
		     		
		     		DisposeWindow(window);
		     		QuitApplicationEventLoop();
		     	}	break;

				case kEventWindowGetIdealSize:
				{	SInt16 baseLine = 0;
					Rect bestRect = { 0 }; 
					Point idealDimensions = { 0 };
					
					GetBestControlRect(
						GetDataBrowserFromWindow(window), &bestRect, &baseLine);
		 			
		 			SetEventParameter(inEvent, kEventParamDimensions, 
						typeQDPoint, sizeof(idealDimensions), &idealDimensions);
				}	break;
				
				case kEventWindowGetClickActivation:
				{	Point mousePt; UInt32 modifiers; 
					ClickActivationResult activation;
					
					GetEventParameter(inEvent, 
						kEventParamMouseLocation, typeQDPoint, 
						NULL, sizeof(mousePt), NULL, &mousePt);
		 			
		 			GetEventParameter(inEvent, 
						kEventParamKeyModifiers, typeUInt32, 
						NULL, sizeof(modifiers), NULL, &modifiers);
		   			
		 			GetControlClickActivation(
		 				GetDataBrowserFromWindow(window), 
		 				mousePt, modifiers, &activation);

		 			SetEventParameter(inEvent, kEventParamClickActivation, 
						typeClickActivationResult, sizeof(activation), &activation);
				}	break;
				
				case kEventWindowContextualMenuSelect:
				{	
				}	break;
			}
		}	break;
	}
	   
    return result;
}

// --------------------------------------------------------------------
pascal OSStatus MyControlEventHandler(
	EventHandlerCallRef, EventRef inEvent, void* inRefcon)
{
    OSStatus result = noErr;
	WindowRef window = (WindowRef) inRefcon;

	switch (GetEventClass(inEvent))
	{
    	default:
    	{	result = eventNotHandledErr;
    	}	break;
		
		case kEventClassControl:
			switch (GetEventKind(inEvent))
			{
				case kEventControlBoundsChanged:
		    	{	ControlRef browser = GetDataBrowserFromWindow(window);
		    		Rect bounds; 
					
					GetEventParameter(inEvent, kEventParamCurrentBounds, typeQDRectangle, NULL,
									  sizeof(bounds), NULL, &bounds );

					if ( placard != NULL )
					{
						Rect barInset;
						GetDataBrowserScrollBarInset(browser, &barInset);

						Rect placardBounds = bounds;
						placardBounds.top = bounds.bottom - bounds.top - barInset.bottom;
						placardBounds.right = bounds.left + 120;
						SetControlBounds(placard, &placardBounds);
					}
					
					SizeControl(browser, bounds.right - bounds.left, bounds.bottom - bounds.top);
		    	}	break;
				
			}
			break;
	}
	
	return result;
}

#pragma mark -
#pragma mark • DataBrowser Setup •
// --------------------------------------------------------------------
static void CreateDataBrowser(WindowRef window, ControlRef *browser)
{
    Rect bounds;
    Boolean frameAndFocus = false;
    
    // Create a DataBrowser
    bounds.top = bounds.left = 0;
    bounds.right = 900; bounds.bottom = 250;
    verify_noerr(CreateDataBrowserControl(window, 
    	&bounds, kDataBrowserListView, browser));

    // Turn off DB's focus frame
	verify_noerr(SetControlData(
		*browser, kControlNoPart, 
		kControlDataBrowserIncludesFrameAndFocusTag,
		sizeof(frameAndFocus), &frameAndFocus));
}

// --------------------------------------------------------------------
static void ConfigureDataBrowser(ControlRef browser)
{
	#ifndef kDataBrowserListViewAppendColumn
	#define kDataBrowserListViewAppendColumn ULONG_MAX
	#endif
	
	DataBrowserViewStyle viewStyle;
	::GetDataBrowserViewStyle(browser, &viewStyle);
	
	Rect insetRect;
	::GetDataBrowserScrollBarInset(browser, &insetRect);
	
	insetRect.right = kScrollBarWidth - 1;
	::SetDataBrowserScrollBarInset(browser, &insetRect);

	switch (viewStyle)
	{
		case kDataBrowserListView:
		{	DataBrowserListViewColumnDesc columnDesc;
			
			columnDesc.headerBtnDesc.titleOffset = 0;
			
			columnDesc.headerBtnDesc.version = 
				kDataBrowserListViewLatestHeaderDesc;
				
			columnDesc.headerBtnDesc.btnFontStyle.flags	= 
				kControlUseFontMask | kControlUseJustMask;
			
			columnDesc.headerBtnDesc.btnContentInfo.contentType = kControlNoContent;
			::GetIconRef(kOnSystemDisk, kSystemIconsCreator, kGenericFolderIcon, 
				&columnDesc.headerBtnDesc.btnContentInfo.u.iconRef);
			
			// Add the checkbox column
			
			columnDesc.propertyDesc.propertyID = kCheckboxColumn;
			columnDesc.propertyDesc.propertyType = kDataBrowserCheckboxType;
			columnDesc.propertyDesc.propertyFlags = kDataBrowserPropertyIsMutable | 
													kDataBrowserDefaultPropertyFlags;
			
			columnDesc.headerBtnDesc.minimumWidth = 30;
			columnDesc.headerBtnDesc.maximumWidth = 30;
			
			columnDesc.headerBtnDesc.btnFontStyle.just = teCenter;
			
			columnDesc.headerBtnDesc.btnFontStyle.font = kControlFontViewSystemFont;
			columnDesc.headerBtnDesc.btnFontStyle.style = normal;
			
			columnDesc.headerBtnDesc.titleString = CFSTR( "" );
			
			::AddDataBrowserListViewColumn(browser, 
				&columnDesc, kDataBrowserListViewAppendColumn),
			
			// Add the Flavor column
			
			columnDesc.propertyDesc.propertyID = kFlavorColumn;
			columnDesc.propertyDesc.propertyType = kDataBrowserIconAndTextType;
			columnDesc.propertyDesc.propertyFlags = kDataBrowserPropertyIsMutable | 
													kDataBrowserListViewSelectionColumn | 
													kDataBrowserListViewDefaultColumnFlags;
			
			columnDesc.headerBtnDesc.btnContentInfo.contentType = kControlContentIconRef;
			
			
			columnDesc.headerBtnDesc.minimumWidth = 0;
			columnDesc.headerBtnDesc.maximumWidth = 200;
			columnDesc.headerBtnDesc.btnFontStyle.just = teFlushDefault;
			
			columnDesc.headerBtnDesc.titleString = CFSTR( "Flavor" );
			
			::AddDataBrowserListViewColumn(browser, 
				&columnDesc, kDataBrowserListViewAppendColumn);
			
			// Add the icon column
			
			columnDesc.propertyDesc.propertyID = kIconOnlyColumn;
			columnDesc.propertyDesc.propertyType = kDataBrowserIconType;
			columnDesc.propertyDesc.propertyFlags = kDataBrowserListViewSelectionColumn | 
													kDataBrowserListViewDefaultColumnFlags;
			
			columnDesc.headerBtnDesc.btnContentInfo.contentType = kControlContentIconRef;
			
			columnDesc.headerBtnDesc.minimumWidth = 0;
			columnDesc.headerBtnDesc.maximumWidth = 100;
			columnDesc.headerBtnDesc.btnFontStyle.just = teCenter;
			
			columnDesc.headerBtnDesc.titleString = NULL;
			
			::AddDataBrowserListViewColumn(browser, 
				&columnDesc, kDataBrowserListViewAppendColumn);
			
			// Add the Index column
			
			columnDesc.headerBtnDesc.maximumWidth = 160;
			
			columnDesc.propertyDesc.propertyID = kIndexColumn;
			columnDesc.propertyDesc.propertyType = kDataBrowserTextType;
			
			columnDesc.headerBtnDesc.btnFontStyle.just = teCenter;

			columnDesc.headerBtnDesc.titleString = CFSTR( "Index" );
			
			::AddDataBrowserListViewColumn(browser, 
				&columnDesc, kDataBrowserListViewAppendColumn);
			
			// Add the Color column
			
			columnDesc.propertyDesc.propertyID = kColorColumn;
			
			columnDesc.headerBtnDesc.btnFontStyle.just = teFlushRight;

			columnDesc.propertyDesc.propertyFlags = kDataBrowserPropertyIsMutable | 
													kDataBrowserListViewDefaultColumnFlags;

			columnDesc.headerBtnDesc.titleString = CFSTR( "Color" );
			
			::AddDataBrowserListViewColumn(browser, 
				&columnDesc, kDataBrowserListViewAppendColumn);
			
			// Add the ItemID column
			
			columnDesc.propertyDesc.propertyID = kItemIDColumn;
			columnDesc.propertyDesc.propertyFlags = kDataBrowserListViewDefaultColumnFlags;
			
			columnDesc.headerBtnDesc.btnFontStyle.just = teFlushDefault;
			
			columnDesc.headerBtnDesc.titleString = CFSTR( "ItemID" );
			
			::AddDataBrowserListViewColumn(browser, 
				&columnDesc, kDataBrowserListViewAppendColumn);
			
		/*	// Add the slider column
			
			columnDesc.propertyDesc.propertyID = kSliderColumn;
			
			columnDesc.headerBtnDesc.btnFontStyle.just = teCenter;
			
			columnDesc.propertyDesc.propertyType = kDataBrowserSliderType;
			columnDesc.propertyDesc.propertyFlags = kDataBrowserSliderPlainThumb | 
													kDataBrowserPropertyIsMutable | 
													kDataBrowserListViewDefaultColumnFlags;
			
			columnDesc.headerBtnDesc.btnContentInfo.contentType = kControlContentTextOnly;
			
			columnDesc.headerBtnDesc.titleString = CFSTR( "Slider" );
			
			::AddDataBrowserListViewColumn(browser, 
				&columnDesc, kDataBrowserListViewAppendColumn);
		*/	
			// Add the progress bar column
			
			columnDesc.headerBtnDesc.maximumWidth = 250;
			
			columnDesc.propertyDesc.propertyID = kProgressBarColumn;
			
			columnDesc.headerBtnDesc.btnFontStyle.just = teFlushDefault;
			
			columnDesc.propertyDesc.propertyType = kDataBrowserRelevanceRankType;
			columnDesc.propertyDesc.propertyFlags = kDataBrowserListViewDefaultColumnFlags;
			
			columnDesc.headerBtnDesc.btnContentInfo.contentType = kControlContentTextOnly;
			
			columnDesc.headerBtnDesc.titleString = CFSTR( "Relevance Rank" );
			
			::AddDataBrowserListViewColumn(browser, 
				&columnDesc, kDataBrowserListViewAppendColumn);
			
			// Add date/time column
			
			columnDesc.headerBtnDesc.maximumWidth = 160;
			
			columnDesc.propertyDesc.propertyID = kDateTimeColumn;
			
			columnDesc.headerBtnDesc.btnFontStyle.just = teFlushLeft;
			
			columnDesc.propertyDesc.propertyType = kDataBrowserDateTimeType;
			columnDesc.propertyDesc.propertyFlags = kDataBrowserListViewSelectionColumn | 
													kDataBrowserListViewDefaultColumnFlags;
			
			columnDesc.headerBtnDesc.minimumWidth = 50;
			columnDesc.headerBtnDesc.maximumWidth = 200;

			columnDesc.headerBtnDesc.titleString = CFSTR( "Date/Time" );
			
			::AddDataBrowserListViewColumn(browser, 
				&columnDesc, kDataBrowserListViewAppendColumn);
						
			// Add column 8
			
			columnDesc.headerBtnDesc.maximumWidth = 320;
			
			columnDesc.propertyDesc.propertyID = kPopupMenuColumn;
			
			columnDesc.propertyDesc.propertyType = kDataBrowserPopupMenuType;
			columnDesc.propertyDesc.propertyFlags = kDataBrowserPropertyIsMutable | 
													kDataBrowserListViewDefaultColumnFlags;
			
			
			columnDesc.headerBtnDesc.titleString = CFSTR( "Popup Menu" );
			
			::AddDataBrowserListViewColumn(browser, 
				&columnDesc, kDataBrowserListViewAppendColumn);
			
			// Finish formatting the table
			//::SetDataBrowserTableViewRowHeight(browser, 10);
			//::SetDataBrowserSortProperty(browser, kFlavorColumn);
			::SetDataBrowserListViewDisclosureColumn(browser, kFlavorColumn, false);
			
			::ReleaseIconRef(columnDesc.headerBtnDesc.btnContentInfo.u.iconRef);
		}	break;
		
		case kDataBrowserColumnView:
		{	//DataBrowserItemID path[] = { 48, 485 };
			//::SetDataBrowserColumnViewPath(browser, 2, path);
			//::SetDataBrowserTarget(browser, 50695);
		}	break;
	}
	
	//RestoreUserState();
}

// --------------------------------------------------------------------
void InstallDataBrowserCallbacks(ControlRef browser)
{
    DataBrowserCallbacks myCallbacks;
    
    //Use latest layout and callback signatures
    myCallbacks.version = kDataBrowserLatestCallbacks;
    verify_noerr(InitDataBrowserCallbacks(&myCallbacks));
    
    myCallbacks.u.v1.itemDataCallback = 
        NewDataBrowserItemDataUPP(MyGetSetItemData);
	
	myCallbacks.u.v1.itemCompareCallback = 
		NewDataBrowserItemCompareUPP(MyItemComparison);

    myCallbacks.u.v1.itemNotificationCallback = 
        NewDataBrowserItemNotificationUPP(MyItemNotification);
     
 	myCallbacks.u.v1.acceptDragCallback =
 		NewDataBrowserAcceptDragUPP(MyAcceptDrag);
	myCallbacks.u.v1.receiveDragCallback = 
		NewDataBrowserReceiveDragUPP(MyReceiveDrag);
	myCallbacks.u.v1.addDragItemCallback = 
		NewDataBrowserAddDragItemUPP(MyAddDragItem);
	myCallbacks.u.v1.itemHelpContentCallback = 
		NewDataBrowserItemHelpContentUPP(MyItemHelpContent);

	myCallbacks.u.v1.getContextualMenuCallback = 
		NewDataBrowserGetContextualMenuUPP(MyGetContextualMenu);
	myCallbacks.u.v1.selectContextualMenuCallback = 
		NewDataBrowserSelectContextualMenuUPP(MySelectContextualMenu);

   verify_noerr(SetDataBrowserCallbacks(browser, &myCallbacks));
}

// --------------------------------------------------------------------
