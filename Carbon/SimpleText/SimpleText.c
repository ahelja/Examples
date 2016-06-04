/*
	File:		SimpleText.c

	Contains:	SimpleText - a simple document editing application 

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

	Copyright © 1993-2001 Apple Computer, Inc., All Rights Reserved
*/

#include "MacIncludes.h"

#define CompilingMain 1

#include "SimpleText.h"
#include "Clipboard.h"

#define qSingleSelectionOnly 0

// amount of time we leave a menu title hilited after a cmd key is used
#define kMenuHiliteDelay 2

#define ff(x)		((Fixed)(x) << 16)

// --------------------------------------------------------------------------------------------------------------
// FORWARD DECLARES
// --------------------------------------------------------------------------------------------------------------
void	UnhiliteMenuDelayed(unsigned long keyTime);
OSStatus 	DoActivate(WindowPtr pWindow, Boolean activating);
OSStatus	DoCommand(WindowPtr pWindow, short commandID, long menuResult, long keyTime);
OSStatus	DoKeyEvent(WindowPtr pWindow, EventRecord * pEvent, Boolean processPageControls);
Boolean CommandToIDs(short commandID, short * menuID, short *itemID);

static Boolean CloseAllWindows( Boolean discard );

static void SynchronizeFiles( void );

static EventHandlerUPP		GetMouseWheelHandler();
static OSStatus			MouseWheelHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* userData );

static OSStatus         FlipMCMD(OSType dataDomain, OSType dataType, short id, void *dataPtr,
                                 UInt32 dataSize, Boolean currentlyNative, void *refcon);

// --------------------------------------------------------------------------------------------------------------
// GLOBAL VARIABLES
// --------------------------------------------------------------------------------------------------------------
EventRecord			gEvent;				// currently pending event
Boolean				gAllDone;			// true if the application is the in process of terminating
MachineInfoRec			gMachineInfo;			// info about abilities and options installed on this machine
short				gApplicationResFile;		// resource fork of application
RgnHandle			gCursorRgn;			// region to control the cursor apearence
#if CALL_NOT_IN_CARBON
AGRefNum			gAGRefNum = -1;			// AppleGuide database which is open
FSSpec				gAGSpec;			// where to find our database
AGCoachRefNum			gAGCoachRefNum = -1;		// coach handler refNum
ThreadID			gAGThread;			// thread that looks for AppleGuide database
#endif
ThreadID			gFontThread;			// thread that builds font menu
ThreadID			gStarterThread;			// starts our other threads for us
Boolean				gDontYield;			// whether our threads should yield
void*				gThreadResults;			// scratch space for thread results
ComponentInstance		gOSAComponent = NULL;		// for AppleScript: connection to OSA scripting component

// AEC, addded
ControlActionUPP	gVActionProc = NULL;
ControlActionUPP	gHActionProc = NULL;

// These variables are for the find/replace commands
Str255			gFindString = "\p", gReplaceString = "\p";
Boolean			gWrapAround = false, gCaseSensitive = false;

// Added these variables to get scrolling to happen on a delayed and accelerated curve
long			gVScrollTrackStartTicks = 0L;		// Holds starting value when scroll action began
#define			kVScrollAcceleratorIncrement  1L
long			gVScrollAccelerator = kVScrollAcceleratorIncrement;			// Value of accelerator incremented at each scroll call
const	long	kScrollAccelerateDelay = 30L;		// Number of ticks to delay until beginning acceleration
const 	long	kScrollAccelerateLag = 40L;			// Number of ticks to delay until next bump in acceleration

// A hack to allow kScrollBarSize to continue working in all the files, but
// still letting us use theme metrics
SInt16			kScrollBarSize;


// Metrowerks MWCRuntime.lib defines qd for us on PPC, and their
// __runtime module does under the 68K case. OTOH, neither SC nor
// MrC give us qd for free, so we need it there. I'm still not
// certain which way to go for the ThinkC or Symantec PPC case.
#if !defined(__MWERKS__)
// QuickDraw globals
//QDGlobals		qd;
#endif

// --------------------------------------------------------------------------------------------------------------
void ConductErrorDialog(OSStatus error, short commandID, short alertType)
{
	long		foundError;		// The error, converted to a number
	short		stringIndex;		// Index into the strings
	Str255		errorText;		// the error in a string format
	
	// Start with no error so far
	foundError = 0;
	
	// Start with the first string
	stringIndex = 1;
	
	// Loop until we find an error string
	errorText[0] = 0;

	do
	{
		// Get the string, and convert it to a number
		GetIndString(errorText, kErrorBaseID + commandID, stringIndex);
		if (errorText[0] == 0)
			break;
		StringToNum(errorText, &foundError);
		
		// If we reach the last string, or we match the error code
		if ((foundError == 0) || (foundError == error))
		{
			// Get the text string for this error
			GetIndString(errorText, kErrorBaseID + commandID, stringIndex+1);
		}
		else
		{
			// Otherwise, make us continue until we get a string
			errorText[0] = 0;
		}
			
		// Advance so we get the next string number
		stringIndex += 2;
		
	} while (errorText[0] == 0);				// errorText[0] == 0
		
	if (errorText[0] != 0)
	{
		DialogPtr	dPtr;
		short		hit;
		Cursor		arrow;
		
		SetCursor(GetQDGlobalsArrow(&arrow));
		ParamText(errorText, "\p", "\p", "\p");
		
		dPtr = GetNewDialog(kErrorBaseID + alertType, nil, (WindowPtr)-1);
		
		SetDialogDefaultItem(dPtr, ok);
		
		BeginMovableModal();
		
		do
		{
			MovableModalDialog(nil, &hit);
		} while (hit != ok);
			
		DisposeDialog(dPtr);
		EndMovableModal();
	}
		
} // ConductErrorDialog

// --------------------------------------------------------------------------------------------------------------
static void MovableModalMenus(DialogPtr dPtr, short *pItem, long menuResult, long keyTime)
{
	short	iCut, iCopy, iClear, iPaste;
	short	editMenu;
	short	menuItem = menuResult & 0xFFFF;
	
	// find out where edit menus are
	CommandToIDs(cCut, &editMenu, &iCut);
	CommandToIDs(cCopy, &editMenu, &iCopy);
	CommandToIDs(cClear, &editMenu, &iClear);
	CommandToIDs(cPaste, &editMenu, &iPaste);
	
	UnhiliteMenuDelayed(keyTime);
	switch (menuResult >> 16)
	{
		case mEdit:
		{
			short	type;
			Handle	item;
			Rect	box;
			short	editField = GetDialogKeyboardFocusItem(dPtr);
			
			// return typed item, if it isn't disabled
			GetDialogItem(dPtr, editField, &type, &item, &box);
			if ((type & itemDisable) == 0)
				*pItem = editField;
				
			if (menuItem == iCut)
				DialogCut(dPtr);
				
			if (menuItem == iCopy)
				DialogCopy(dPtr);
				
			if (menuItem == iClear)
				DialogDelete(dPtr);
				
			if (menuItem == iPaste)
				DialogPaste(dPtr);
		}
		break;
	}
		
} // MovableModalMenus

// --------------------------------------------------------------------------------------------------------------
void UnhiliteMenuDelayed(unsigned long keyTime)
{
	if (keyTime != 0)
	{
		keyTime = TickCount() - keyTime;
		if (keyTime < kMenuHiliteDelay)
			Delay(kMenuHiliteDelay - keyTime, &keyTime);
	}
	HiliteMenu(0);
}

// --------------------------------------------------------------------------------------------------------------
void MovableModalDialog(ModalFilterProcPtr filterProc, short *pItem)
/*
	Call this as you would ModalDialog, when the dialog is moveable
	modal.
	
	However, first call BeginMovableModal, and afterwards (after
	disposing of dialog) call EndMovableModal.
*/
{
	GrafPtr 	curPort;
	DialogPtr	dPtr = GetDialogFromWindow( FrontNonFloatingWindow() );
	RgnHandle	structRegion;

	*pItem = 0;
	
	if (dPtr)
	{
		GetPort(&curPort);
		SetPort(GetWindowPort(FrontNonFloatingWindow()));
		
		do
		{
			WaitNextEvent(mDownMask + mUpMask + keyDownMask + keyUpMask + autoKeyMask + updateMask + activMask + osMask,
							&gEvent, 0, nil);
			
			// call the filter proc
			if ( (filterProc) && ((*filterProc) (dPtr, &gEvent, pItem)) )
				break;
							
			// call the basic filtering
			if (StdFilterProc(dPtr, &gEvent, pItem))
				break;
				
			// handle keyboard
			if ((gEvent.what == keyDown || gEvent.what == autoKey))
			{
				long	result;
				long	tck;
				
				result = MenuEvent(&gEvent);
				tck = TickCount();
				
				if ( result != 0 )
					MovableModalMenus(dPtr, pItem, result, tck);
					
				if ( result != 0 || ( gEvent.modifiers & cmdKey ) != 0 )
					break;
			}
				
			// handle clicks and drags
			if (gEvent.what == mouseDown)
			{
				WindowPtr	whichWindow;
				short		part = FindWindow(gEvent.where, &whichWindow);
				DialogPtr	whichDialog = NULL;
				
				if (whichWindow)
					whichDialog = GetDialogFromWindow(whichWindow);

				// menu bar events
				if (part == inMenuBar)
				{
					InitCursor();	// make sure we've got an arrow during menu tracking
					MovableModalMenus(dPtr, pItem, MenuSelect(gEvent.where), 0);
					break;
				}
					
				structRegion = NewRgn();
				// check for outside of our window
				GetWindowRegion( GetDialogWindow( dPtr ), kWindowStructureRgn, structRegion );
				if (!PtInRgn(gEvent.where, structRegion))
				{
					SysBeep(1);
					gEvent.what = nullEvent;
				}
				DisposeRgn( structRegion );
					
				// drag the window around
				if ( (part == inDrag) && (whichDialog == dPtr) )
				{
					Rect	tempRect;
					
					DragWindow(whichWindow, gEvent.where, GetRegionBounds( GetGrayRgn(), &tempRect) );
					gEvent.what = nullEvent;
				}
			}
				
			// check with standard dialog stuff	
			{
			DialogPtr	tempDialog;
			
			if ( IsDialogEvent(&gEvent) && DialogSelect(&gEvent, &tempDialog, pItem) )
				break;
			}
			
			// handle updates
			if (gEvent.what == updateEvt)
			{
				HandleEvent(&gEvent);
				break;
			}
		} while (true);
		
		SetPort(curPort);
	}
		
} // MovableModalDialog

// --------------------------------------------------------------------------------------------------------------
void BeginMovableModal(void)
{
	DialogPtr	dPtr = GetDialogFromWindow(FrontNonFloatingWindow());
	WindowPtr	nextWindow = GetNextWindow(FrontNonFloatingWindow());
	
	// Disable the current indicator because the upcoming dialog is moveable modal
	HiliteMenu(0);
		
	if (nextWindow)
		DoActivate(nextWindow, false);
	AdjustMenus(GetDialogWindow(dPtr), (GetDialogKeyboardFocusItem(dPtr) > 0), false);

} // BeginMovableModal

// --------------------------------------------------------------------------------------------------------------
void EndMovableModal(void)
{
	WindowPtr	nextWindow = FrontNonFloatingWindow();
	
	AdjustMenus(nextWindow, true, false);
	if (nextWindow)
		DoActivate(nextWindow, true);
	
} // EndMovableModal

// --------------------------------------------------------------------------------------------------------------
short ConductFindOrReplaceDialog(short dialogID)
{
	DialogPtr	dPtr;
	short		hit;
	
	// menu shouldn't stay hilighted during the dialog
	HiliteMenu(0);
	
	dPtr = GetNewDialog(dialogID, nil, (WindowPtr)-1);
	if (dPtr)
	{
		short		kind;
		Rect		box;
		Handle		item;
		ControlRef	itemAsControl;
		ScrapRef	findScrap;
		
		// standard default behavior
		SetDialogDefaultItem(dPtr, ok);
		SetDialogCancelItem (dPtr, cancel);
		SetDialogTracksCursor(dPtr, true);
		
		// grab the global Find Scrap string every time the Find or Replace dialogs are run
		if( GetScrapByName( kScrapFindScrap, kScrapGetNamedScrap, &findScrap ) == noErr )
		{
			Size byteCount = 255;
			
			if( GetScrapFlavorData( findScrap, kScrapFlavorTypeText, &byteCount, gFindString+1 ) == noErr )
				gFindString[0] = ( byteCount < 255 ) ? byteCount : 255;
		}
		
		// find string
		GetDialogItemAsControl( dPtr, iFindEdit, &itemAsControl );
		SetDialogItemText( (Handle)itemAsControl, gFindString);

		// check boxes
		GetDialogItem(dPtr, iCaseSensitive, &kind, &item, &box);
		SetControlValue((ControlHandle)item, gCaseSensitive);
		GetDialogItem(dPtr, iWrapAround, &kind, &item, &box);
		SetControlValue((ControlHandle)item, gWrapAround);
		
		if (dialogID == kReplaceWindowID)
		{
			// Replace string
			GetDialogItemAsControl( dPtr, iReplaceEdit, &itemAsControl );
			SetDialogItemText( (Handle)itemAsControl, gReplaceString);
		}
		
		// select the search text by default
		SelectDialogItemText(dPtr, iFindEdit, 0, 32767);
		
		// and away we go!
		ShowWindow(GetDialogWindow(dPtr));
		BeginMovableModal();

		do
		{
			MovableModalDialog(nil, &hit);
			switch (hit)
			{
				case iCaseSensitive:
				case iWrapAround:
					GetDialogItem(dPtr, hit, &kind, &item, &box);
					SetControlValue((ControlHandle)item, 1-GetControlValue((ControlHandle)item));
					break;
			}
		} while ( (hit != ok) && (hit != cancel) && (hit != iReplaceAll) );
		
		if (hit != cancel)
		{
			// Find string
			GetDialogItem(dPtr, iFindEdit, &kind, &item, &box);
			GetDialogItemText(item, gFindString);
			
			// nothing to find is like a cancel
			if (gFindString[0] < 1)
				hit = cancel;
			
			// Set the global Find Scrap string
			if( hit != cancel )
				if( GetScrapByName( kScrapFindScrap, kScrapClearNamedScrap, &findScrap ) == noErr )
					PutScrapFlavor( findScrap, kScrapFlavorTypeText, kScrapFlavorMaskNone, gFindString[0], gFindString+1 );
			
			// check boxes
			GetDialogItem(dPtr, iCaseSensitive, &kind, &item, &box);
			gCaseSensitive = GetControlValue((ControlHandle)item);
			GetDialogItem(dPtr, iWrapAround, &kind, &item, &box);
			gWrapAround = GetControlValue((ControlHandle)item);
			
			if (dialogID == kReplaceWindowID)
			{
				// Replace string
				GetDialogItem(dPtr, iReplaceEdit, &kind, &item, &box);
				GetDialogItemText(item, gReplaceString);
			}
		}
			
		DisposeDialog(dPtr);
		EndMovableModal();
	}
		
	return(hit);
	
} // ConductFindOrReplaceDialog

// --------------------------------------------------------------------------------------------------------------
void LocalToGlobalRgn(RgnHandle rgn)
{
	Point		offset = { 0, 0 };

    LocalToGlobal( &offset );
    
	OffsetRgn(rgn, offset.h, offset.v);
	
} // LocalToGlobalRgn

// --------------------------------------------------------------------------------------------------------------
void GlobalToLocalRgn(RgnHandle rgn)
{
	Point		offset = { 0, 0 };

	GlobalToLocal( &offset );
		
	OffsetRgn(rgn, offset.h, offset.v);
	
} // GlobalToLocalRgn

// --------------------------------------------------------------------------------------------------------------
void SetWatchCursor(void)
{
	CursHandle	theWatch;
		
	theWatch = MacGetCursor(watchCursor);
	if (theWatch)
	{
		char	oldState;
		
		oldState = HGetState((Handle) theWatch);
		HLock((Handle) theWatch);
		SetCursor(*theWatch);
		HSetState((Handle) theWatch, oldState);
	}
		
} // SetWatchCursor

// --------------------------------------------------------------------------------------------------------------
void LongRectToRect(LongRect* longRect, Rect *rect)
{
	rect->top 		= longRect->top;
	rect->left 		= longRect->left;
	rect->bottom 	= longRect->bottom;
	rect->right 	= longRect->right;
	
} // LongRectToRect

// --------------------------------------------------------------------------------------------------------------
void RectToLongRect(Rect *rect, LongRect *longRect)
{
	longRect->top 		= rect->top;
	longRect->left 		= rect->left;
	longRect->bottom 	= rect->bottom;
	longRect->right 	= rect->right;
	
} // RectToLongRect

// --------------------------------------------------------------------------------------------------------------
void GetPICTRectangleAt72dpi(PicHandle hPicture, Rect *pictureRect)
{
	QDGetPictureBounds(hPicture, pictureRect);
} // GetPICTRectangleAt72dpi

// --------------------------------------------------------------------------------------------------------------
static WindowDataPtr	GetWindowInfo(WindowPtr pWindow)
{
	WindowDataPtr result = nil;
	
	if 	(
		(pWindow) &&
		(GetWindowKind(pWindow) == userKind)
		)
		result = (WindowDataPtr) GetWRefCon(pWindow);

	return result;
	
} // GetWindowInfo

// --------------------------------------------------------------------------------------------------------------
static short ZeroStringSub(Str255 destString, Str255 subStr)
	// returns number of substitutions performed
{
	OSStatus	anErr;
	Handle	destHandle = nil;
	Handle	subHandle = nil;
	short	count = 0;

	anErr = PtrToHand(&destString[1], &destHandle, destString[0]);
	if (anErr == noErr)
	{		
		anErr = PtrToHand(&subStr[1], &subHandle, subStr[0]);
		if (anErr == noErr)
		{
			count = ReplaceText(destHandle, subHandle, "\p^0");		// error or # of substitutions
						
			destString[0] = GetHandleSize(destHandle);
			BlockMoveData(*destHandle, &destString[1], destString[0]);
		}
	}

	DisposeHandle(destHandle);
	DisposeHandle(subHandle);

	if (count < 0)
		count = 0;		// change error code into count = 0 substitutions

	return count;

} // ZeroStringSub

// --------------------------------------------------------------------------------------------------------------
// BEGIN SCROLL ACTION PROCS
// --------------------------------------------------------------------------------------------------------------
void SetControlAndClipAmount(ControlHandle control, short * amount)
{
	short		value, max;
	
	value = GetControlValue(control);	/* get current value */
	max = GetControlMaximum(control);		/* and maximum value */
	*amount = value - *amount;
	if ( *amount < 0 )
	{
		*amount = 0;
		gVScrollAccelerator = kVScrollAcceleratorIncrement;
	}
	else
	{
		if ( *amount > max )
		{
			*amount = max;
			gVScrollAccelerator = kVScrollAcceleratorIncrement;
		}
	}
	SetControlValue(control, *amount);
	*amount = value - *amount;		/* calculate the real change */
	
} // SetControlAndClipAmount

// --------------------------------------------------------------------------------------------------------------
static pascal void VActionProc(ControlHandle control, short part)
{
	if (part != 0)
	{
		WindowPtr		pWindow = GetControlOwner(control);
		WindowDataPtr 		pData = GetWindowInfo(pWindow);
		short			amount = 0;
		
		switch (part)
		{
			case kControlUpButtonPart:
				if (gVScrollAccelerator > kVScrollAcceleratorIncrement) // We're already in accellerated mode
				{
					//Only increment the accelerator if it's been longer than the lag time. Then reset the timer
					if (TickCount() > (gVScrollTrackStartTicks + kScrollAccelerateLag)) 
					{
						gVScrollAccelerator += kVScrollAcceleratorIncrement;
						gVScrollTrackStartTicks = TickCount();
					}	
				} 
				else // We haven't started acceleration yet
				{
					if (TickCount() > (gVScrollTrackStartTicks + kScrollAccelerateDelay))
					{	
						gVScrollAccelerator += kVScrollAcceleratorIncrement; // start acceleration
						gVScrollTrackStartTicks = TickCount(); 			     // reset the timer to start our lag acceleration
					}
				}
				amount = pData->vScrollAmount * gVScrollAccelerator;
				//printf("Scroll Amount: %d Scrolling Accelerator: %ld Will Scroll By: %d\r\n", pData->vScrollAmount, gVScrollAccelerator, amount);
				SetControlAndClipAmount(control, &amount);
				break;
				
			case kControlDownButtonPart:
				if (gVScrollAccelerator > kVScrollAcceleratorIncrement) // We're already in accellerated mode
				{
					//Only increment the accelerator if it's been longer than the lag time. Then reset the timer
					if (TickCount() > (gVScrollTrackStartTicks + kScrollAccelerateLag)) 
					{
						gVScrollAccelerator += kVScrollAcceleratorIncrement;
						gVScrollTrackStartTicks = TickCount();
					}	
				}
				else // We haven't started acceleration yet
				{
					if (TickCount() > (gVScrollTrackStartTicks + kScrollAccelerateDelay))
					{	
						gVScrollAccelerator += kVScrollAcceleratorIncrement; // start acceleration
						gVScrollTrackStartTicks = TickCount(); 			     // reset the timer to start our lag acceleration
					}
				}
				amount = -pData->vScrollAmount * gVScrollAccelerator;
				//printf("Scroll Amount: %d Scrolling Accelerator: %ld Will Scroll By: %d\r\n", pData->vScrollAmount, gVScrollAccelerator, amount);
				SetControlAndClipAmount(control, &amount);
				break;
				
			// vertical page scrolling should be a multiple of the incremental scrolling -- so that
			// we avoid half-lines of text at the bottom of pages.
			
			// More generically, if there was a method for dealing with text scrolling by a non-constant
			// amount, this would be better -- but SimpleText currently doesn't have a framework to allow
			// the document object to override the scroll amount dynamically.  Maybe something to add in
			// the future.
			case kControlPageUpPart:
				amount = (((pData->contentRect.bottom - pData->contentRect.top) / pData->vScrollAmount)-1) * pData->vScrollAmount;
				if (amount == 0)
					amount = pData->contentRect.bottom - pData->contentRect.top;
				SetControlAndClipAmount(control, &amount);
				break;

			case kControlPageDownPart:
				amount = (((pData->contentRect.top - pData->contentRect.bottom) / pData->vScrollAmount)+1) * pData->vScrollAmount;
				if (amount == 0)
					amount = pData->contentRect.top - pData->contentRect.bottom;
				SetControlAndClipAmount(control, &amount);
				break;

			default:
				amount = pData->oldVValue - GetControlValue(control);
				break;
		}
		
		pData->oldVValue = GetControlValue(control);
		DoScrollContent(pWindow, pData, 0, amount);
	}
		
} // VActionProc


// --------------------------------------------------------------------------------------------------------------
static pascal void HActionProc(ControlHandle control, short part)
{
	if (part != 0)
	{
		WindowPtr		pWindow = GetControlOwner(control);
		WindowDataPtr 		pData = GetWindowInfo(pWindow);
		short			amount = 0;
		
		switch (part)
		{
			case kControlUpButtonPart:
				amount = pData->hScrollAmount;
				SetControlAndClipAmount(control, &amount);
				break;
				
			case kControlDownButtonPart:
				amount = -pData->hScrollAmount;
				SetControlAndClipAmount(control, &amount);
				break;
				
			case kControlPageUpPart:
				amount = pData->contentRect.right - pData->contentRect.left;
				SetControlAndClipAmount(control, &amount);
				break;

			case kControlPageDownPart:
				amount = pData->contentRect.left - pData->contentRect.right;
				SetControlAndClipAmount(control, &amount);
				break;

			default:
				amount = pData->oldHValue - GetControlValue(control);
				pData->oldHValue = GetControlValue(control);
				break;
		}
		
		DoScrollContent(pWindow, pData, amount, 0);
	}
		
} // HActionProc


// --------------------------------------------------------------------------------------------------------------
// END SCROLL ACTION PROCS
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
// SEARCH/REPLACE UTILITY FUNCTIONS
// --------------------------------------------------------------------------------------------------------------
static Boolean IsThisTheString(
			Ptr p,						// pointer to check
			Str255 searchString,		// string to check for
			Boolean isCaseSensitive)	// case sensitive check or not
/*
	Returns true if the supplied string is at the specified offset.
	Otherwise returns false.
*/
{
	Handle 	itl2Table;
	long	offset, length;
	short	script;
	Boolean result = false;

	script = FontToScript(GetSysFont());
	GetIntlResourceTable(script, smWordSelectTable, &itl2Table, &offset, &length);

	if (itl2Table != NULL)
	{
		if (isCaseSensitive)
			result = (CompareText(p, &searchString[1], searchString[0], searchString[0], itl2Table) == 0);
		else
			result = (IdenticalText(p, &searchString[1], searchString[0], searchString[0], itl2Table) == 0);
	}

	return result;
} // IsThisTheString


// --------------------------------------------------------------------------------------------------------------

Boolean PerformSearch(
		Handle	h,					// handle to search
		long start,					// offset to begin with
		Str255 searchString,		// string to search for
		Boolean isCaseSensitive,	// case sensitive search
		Boolean isBackwards,		// search backwards from starting point
		Boolean isWraparound,		// wrap search around from end->begining
		long * pNewStart,			// returned new selection start
		long * pNewEnd)				// returned new selection end
/*
	Performs a search on the supplied handle, starting at the provided
	offset.  Returns the new selection start and end values, and true
	if the search is successful.  Otherwise it returns false.
*/
{
	char	flags;
	Ptr		startPtr;
	Ptr		endPtr;
	Ptr		searchPtr;
	Boolean	foundIt = false;
	
	flags = HGetState(h);
	HLock(h);
			
	// back up one when searching backwards, or we'll hit every time on the current
	// character
	if (isBackwards)
	{
		if (start != 0)
		{
			--start;
		}
		else
		{
			if (isWraparound)
				start = GetHandleSize(h);
			else
				return(false);
		}
	}
		
	// determine the bounds of the searching
	startPtr = (*h) + start;
	if ( isWraparound )
	{
		if (isBackwards)
		{
			// go backwards until just after the start, or begining of
			// document is start is the end
			if (start == GetHandleSize(h))
				endPtr = *h;
			else
				endPtr = startPtr + 1;
		}
		else
		{
			// go forwards until just before the start, or to the end
			// of the document is the start is already the begining
			if (start == 0)
				endPtr = *h + GetHandleSize(h);
			else
				endPtr = startPtr - 1;
		}
	}
	else
	{
		if (isBackwards)
		{
			// go back until hit begining of document
			endPtr = *h-1;	
		}
		else
		{
			// go forward until hit end of document
			endPtr = *h + GetHandleSize(h);
		}
	}
		
	searchPtr = startPtr;
	while (searchPtr != endPtr)
	{
		short	byteType;
		
		byteType = CharacterByteType(startPtr, searchPtr - startPtr, smCurrentScript);
		
		if ( ((byteType == smSingleByte) || (byteType == smFirstByte)) &&
			IsThisTheString(searchPtr, searchString, isCaseSensitive)
			)
		{
			foundIt = true;
			*pNewStart = searchPtr - *h;
			*pNewEnd = *pNewStart + searchString[0];
			break;
		}
			
		if (isBackwards)
			--searchPtr;
		else
			++searchPtr;
			
		if (isWraparound)
		{
			if (searchPtr < *h)
				searchPtr = *h + GetHandleSize(h);
			if (searchPtr > *h + GetHandleSize(h))
				searchPtr = *h;
		}
	}
		
	HSetState(h, flags);
	
	return(foundIt);
	
} // PerformSearch

// --------------------------------------------------------------------------------------------------------------
// SELECTION UTILITY ROUTINES
// --------------------------------------------------------------------------------------------------------------
void DrawSelection(WindowDataPtr pData, Rect *pSelection, short * pPhase, Boolean bumpPhase)
{
	if	(!EmptyRect(pSelection) ) 
	{
		RgnHandle	oldClip = NewRgn();
		Pattern		aPattern;
		Rect		newClip;

		
		if 	( 
			(bumpPhase) && 
			(MOVESELECTION(TickCount()) ) 
			)
		{
			if ((++(*pPhase)) > 7 )
				*pPhase = 1;
		}
			
		// setup for drawing in this window
		SetPortWindowPort (pData->theWindow);
		GetClip(oldClip);
		PenMode(notPatXor);
		
		// offset the draw area (SetOrigin a must to preserve pattern appearence)
		// and the clip area to avoid stepping on the scroll bars
		SetOrigin(GetControlValue(pData->hScroll), GetControlValue(pData->vScroll));
		newClip = pData->contentRect;
		OffsetRect(&newClip, GetControlValue(pData->hScroll), GetControlValue(pData->vScroll));
		ClipRect(&newClip);
		
		// do the draw
		GetIndPattern(&aPattern, kPatternListID, (*pPhase)+1);
		PenPat(&aPattern);
		FrameRect(pSelection);
		SetOrigin(0, 0);
		
		// restore the old port settings
		SetClip(oldClip);
		DisposeRgn(oldClip);
		PenNormal();

	}

} // DrawSelection

// --------------------------------------------------------------------------------------------------------------
OSStatus SelectContents(WindowPtr pWindow, WindowDataPtr pData, EventRecord *pEvent, Rect *pSelection, Rect *pContent, short *pPhase)
{

	OSStatus			anErr = noErr;
	Point			clickPoint = pEvent->where;
	Point			currentPoint;
	Boolean 		didJustScroll;
	ControlHandle	theControl;
	
	GlobalToLocal(&clickPoint);
	if (FindControl(clickPoint, pWindow, &theControl) == 0)
	{
	
		// move the click point into the proper range
		clickPoint.h += GetControlValue(pData->hScroll);
		clickPoint.v += GetControlValue(pData->vScroll);
		
		// if the shift key is held down then the selection starts from
		// a preexisting point such that we are doing an expand/contract
		// of the original selection
		if (pEvent->modifiers & shiftKey)
		{
			if (clickPoint.h < pSelection->right)
				clickPoint.h = pSelection->right;
			else
				clickPoint.h = pSelection->left;

			if (clickPoint.v < pSelection->bottom)
				clickPoint.v = pSelection->bottom;
			else
				clickPoint.v = pSelection->top;
		}
						
		while (StillDown())
		{					
			CGrafPtr thePort = GetQDGlobalsThePort();

			// get the current mouse 
			GetMouse(&currentPoint);
			
			didJustScroll = false;
			// scroll contents if needed
			{
			short	deltaH = 0;
			short	deltaV = 0;
			Rect	bounds;

			GetPortBounds(thePort, &bounds);
			if (currentPoint.h < 0)
				deltaH = pData->hScrollAmount;
			if (currentPoint.h > bounds.right)
				deltaH = -pData->hScrollAmount;
			if (currentPoint.v < 0)
				deltaV = pData->vScrollAmount;
			if (currentPoint.v > bounds.bottom)
				deltaV = -pData->vScrollAmount;
				
			if ( (deltaH != 0) || (deltaV != 0) )
				{				
				if (deltaH)
					SetControlAndClipAmount(pData->hScroll, &deltaH);
				if (deltaV)
					SetControlAndClipAmount(pData->vScroll, &deltaV);

				DoScrollContent(pWindow, pData, deltaH, deltaV);
				
				didJustScroll = true;
				}
			}
			
			// map mouse into proper range
			currentPoint.h += GetControlValue(pData->hScroll);
			currentPoint.v += GetControlValue(pData->vScroll);
	
			// clip to the document size
			if (currentPoint.h < 0)
				currentPoint.h = 0;
			if (currentPoint.v < 0)
				currentPoint.v = 0;
			if (currentPoint.h > pContent->right)
				currentPoint.h = pContent->right;
			if (currentPoint.v > pContent->bottom)
				currentPoint.v = pContent->bottom;
				
			// draw the new selection if it is time or we are about to 
			// exit this loop
			if ((MOVESELECTION(TickCount())) || (!Button()) || (didJustScroll) )
			{
				// first, erase any old selection we might have had
				DrawSelection(pData, pSelection, pPhase, false);

				// make a rectangle out of the two points
				pSelection->left 	= Min(currentPoint.h, clickPoint.h);
				pSelection->right 	= Max(currentPoint.h, clickPoint.h);
				pSelection->top 	= Min(currentPoint.v, clickPoint.v);
				pSelection->bottom 	= Max(currentPoint.v, clickPoint.v);
	
				// draw the new selection
				DrawSelection(pData, pSelection, pPhase, true);
			}
		}
		
		// we handled the selection
		anErr = eActionAlreadyHandled;
	}
		
	return(anErr);
	
} // SelectContents

// --------------------------------------------------------------------------------------------------------------
void DragAndDropArea(WindowPtr pWindow, WindowDataPtr pData, EventRecord* event, Rect *pFrameRect)
{
	RgnHandle		hilightRgn;
	DragReference	theDrag;
	OSStatus			anErr = noErr;

	if (!WaitMouseMoved (event->where))
	{
		return;
	}
		
	if (NewDrag(&theDrag) == noErr)
	{
		
		// add the flavor for the window title, errors can be ignored as this
		// is a cosmetic addition
		{
		enum
			{
			kFlavorTypeClippingName = 'clnm'
			};
		Str255	windowTitle;
		
		GetWTitle(pWindow, windowTitle);
		(void) 	AddDragItemFlavor(theDrag, 1, kFlavorTypeClippingName, &windowTitle, windowTitle[0]+1, flavorNotSaved);
		}
		
		if (pData->pDragAddFlavors)
			anErr = (*(pData->pDragAddFlavors)) (pWindow, pData, theDrag);
		
		if (anErr == noErr)
		{
			Rect	globalRect = *pFrameRect;
			
			hilightRgn = NewRgn();	
			LocalToGlobal(&TopLeft(globalRect));
			LocalToGlobal(&BotRight(globalRect));
			RectRgn(hilightRgn, &globalRect);
			SetDragItemBounds(theDrag, 1, &globalRect);
	
			// turn the region from a fill into a frame
			{	
				RgnHandle tempRgn = NewRgn();
	
				CopyRgn(hilightRgn, tempRgn);
				InsetRgn(tempRgn, 1, 1);
				DiffRgn(hilightRgn, tempRgn, hilightRgn);
				DisposeRgn(tempRgn);
			}
			
			TrackDrag(theDrag, event, hilightRgn);
			DisposeDrag(theDrag);
			DisposeRgn(hilightRgn);
		}
	}

} // DragAndDropArea

// --------------------------------------------------------------------------------------------------------------
// WINDOW UTILITY ROUTINES
// --------------------------------------------------------------------------------------------------------------
static void CalculateGrowIcon(WindowPtr pWindow,WindowDataPtr pData, Rect * location)
{
#pragma unused (pWindow)

	Rect bounds;

	if (pData->vScroll)
	{
		GetControlBounds(pData->vScroll, &bounds);
		location->top = bounds.bottom;
	} else
	{
		if (pData->hScroll)
		{
			GetControlBounds(pData->hScroll, &bounds);
			location->top = bounds.top;
		} else
		{
			GetWindowPortBounds(pData->theWindow, &bounds);
			location->top = bounds.bottom - 15;
		}
	}
		
	if (pData->hScroll)
	{
		GetControlBounds(pData->hScroll, &bounds);
		location->left = bounds.right;
	} else
	{
		if (pData->vScroll) {
			GetControlBounds(pData->vScroll, &bounds);
			location->left = bounds.left;
		} else
		{
			GetWindowPortBounds(pData->theWindow, &bounds);
			location->left = bounds.right - 15;
		}
	}

	location->right = location->left + 16;
	location->bottom = location->top + 16;
	
} // CalculateGrowIcon

// --------------------------------------------------------------------------------------------------------------
//
// Set the modified bit for the window
//
void SetDocumentContentChanged( WindowDataPtr pData, Boolean changed )
{
	pData->changed = changed;
		
	if( gMachineInfo.haveProxyIcons )
		SetWindowModified( pData->theWindow, changed );

}


OSStatus	AdjustScrollBars(WindowRef pWindow,
	Boolean moveControls, 				// might the controls have moved?
	Boolean didResize, 					// did we just resize the window?
	Boolean *needInvalidate)			// does the caller need to invalidate contents as a result?
{
	OSStatus		anErr = noErr;
	LongRect		docRect;
	WindowDataPtr 		pData = GetWindowInfo(pWindow);
	Rect			growIconRect;
	
	if (needInvalidate)
		*needInvalidate = false;

	if (pData)
	{
		short	oldHMax = 0;
		short	oldVMax = 0;
		short	oldHValue = 0;
		short	oldVValue = 0;
                Rect bounds;
		
		// cache current values, we'll force an update if we needed to change em!
		if (pData->hScroll)
		{
			oldHMax = GetControlMaximum(pData->hScroll);
			oldHValue = GetControlValue(pData->hScroll);
		}
		if (pData->vScroll)
		{
			oldVMax = GetControlMaximum(pData->vScroll);
			oldVValue = GetControlValue(pData->vScroll);
		}
			
		// if we have a grow box but not all controls we have to invalidate the grow bar areas
		// by caclulating them
		if ( (didResize) && (pData->hasGrow) )
		{
                        GetWindowPortBounds(pWindow, &bounds);

			// if we regrow without any scroll bars, we need to update the content area
			if ( (needInvalidate) && (pData->hScroll == nil) && (pData->vScroll == nil) )
				*needInvalidate = true;
			
			// invalidate old grow bar areas
			if (pData->vScroll == nil)
			{
				growIconRect = bounds;
				growIconRect.left = pData->contentRect.right;
				InvalWindowRect(pWindow,&growIconRect);
			}
			if (pData->hScroll == nil)
			{
                                growIconRect = bounds;
				growIconRect.top = pData->contentRect.bottom;
				InvalWindowRect(pWindow,&growIconRect);
			}
			
			// invalidate new grow bar areas
			if (pData->vScroll == nil)
			{
                                growIconRect = bounds;
				growIconRect.left = growIconRect.right - kScrollBarSize;
				InvalWindowRect(pWindow,&growIconRect);
			}
			if (pData->hScroll == nil)
			{
                                growIconRect = bounds;
				growIconRect.top = growIconRect.bottom - kScrollBarSize;
				InvalWindowRect(pWindow,&growIconRect);
			}
		}
			
		// if the controls need moving, recalculate the visible area
		if (moveControls)
		{
                        growIconRect = bounds;
			if ((pData->hScroll) || (pData->hasGrow) )
#if 0
//				pData->contentRect.bottom -= kScrollBarSize;
//				This made the viewRect smaller and smaller when backspacing.
//				See TextKeyEvent (TextFile.c)  Adjust from real bounds instead of previous.
#endif
				pData->contentRect.bottom = bounds.bottom - kScrollBarSize;
			if ((pData->vScroll) || (pData->hasGrow) )
#if 0
//				pData->contentRect.right -= kScrollBarSize;
//				(same fix as 'contentRect.bottom' above.)
#endif
				pData->contentRect.right = bounds.right - kScrollBarSize;
		}
			
		// before doing anything, make the controls invisible
		if (pData->hScroll)
                    SetControlVisibility(pData->hScroll, false, false);
		if (pData->vScroll)
                    SetControlVisibility(pData->vScroll, false, false);

		// based on document and visiable area, adjust possible control values
		if ( (pData->pGetDocumentRect) && ((pData->hScroll) || (pData->vScroll)) )
		{
			// let the object calc the size and content if it wishes to
			anErr = (*(pData->pGetDocumentRect)) (pWindow, pData, &docRect, false);
			if (anErr == noErr)
			{
				short	amountOver;
				short	newMax;
				
				amountOver = (docRect.right - docRect.left) - (pData->contentRect.right - pData->contentRect.left);
				if 	(
					(pData->hScroll) &&
					(amountOver > 0)
					)
					newMax = amountOver;
				else
					newMax = 0;
	
				if (pData->hScroll)
				{
					if (GetControlValue(pData->hScroll) > newMax)
					{
						if (needInvalidate)
							*needInvalidate = true;
					}
					SetControlMaximum(pData->hScroll, newMax);
				}
				
				amountOver = (docRect.bottom - docRect.top) - (pData->contentRect.bottom - pData->contentRect.top);
				if 	(
					(pData->vScroll) &&
					(amountOver > 0)
					)
					newMax = amountOver;
				else
					newMax = 0;
					
				if (pData->vScroll)
				{
					if (GetControlValue(pData->vScroll) > newMax)
					{
						if (needInvalidate)
							*needInvalidate = true;
					}
					SetControlMaximum(pData->vScroll, newMax);
				}
			}
		}
			
		// then, if the controls need moving, we move them and inval the old
		// and new locations
		if (moveControls)
		{
			// if we have grow box we invalidate the old grow location
			if ( pData->hasGrow) 
			{
				CalculateGrowIcon(pWindow, pData, &growIconRect);
				InvalWindowRect(pWindow,&growIconRect);
			}
				
			if (pData->hScroll)
			{
				short	widthAdjust;
				Rect bounds;
				
				if ((pData->vScroll) || (pData->hasGrow))
					widthAdjust = -kGrowScrollAdjust;
				else
					widthAdjust = -1;
					
				GetControlBounds(pData->hScroll, &bounds);
				InvalWindowRect( pWindow, &bounds);

				GetWindowPortBounds(pWindow, &bounds);
                                MoveControl(pData->hScroll, pData->hScrollOffset - 1, bounds.bottom - kScrollBarSize);
                                SizeControl(pData->hScroll, (bounds.right - bounds.left) + widthAdjust - pData->hScrollOffset, 16);

                                GetControlBounds(pData->hScroll, &bounds);
                                InvalWindowRect( pWindow, &bounds);
			}

			if (pData->vScroll)
			{
				short	heightAdjust;
				
				if ((pData->hScroll) || (pData->hasGrow))
					heightAdjust = -kGrowScrollAdjust;
				else
					heightAdjust = -1;
					
				GetControlBounds(pData->vScroll, &bounds);
                                InvalWindowRect( pWindow, &bounds);

                                GetWindowPortBounds(pWindow, &bounds);
                                MoveControl(pData->vScroll, bounds.right - kScrollBarSize, pData->vScrollOffset-1);
                                SizeControl(pData->vScroll, 16, (bounds.bottom - bounds.top) + heightAdjust - pData->vScrollOffset);

                                GetControlBounds(pData->vScroll, &bounds);
                                InvalWindowRect( pWindow, &bounds);
			}
				
			// if we have scroll bars, update the grow icon
			if ( pData->hasGrow )
			{
				CalculateGrowIcon(pWindow,pData, &growIconRect);
				InvalWindowRect(pWindow,&growIconRect);
			}
			
		}

		// let the document adjust anything it needs to
		if (pData->pAdjustSize)
			anErr = (*(pData->pAdjustSize)) (pWindow, pData, &didResize);
			
		if ((didResize) && (needInvalidate))
			*needInvalidate = true;


                if ( IsWindowHilited(pWindow) )
		{
			// after doing something, make the controls visible
			if (pData->hScroll)
			{
				if ((oldHMax != GetControlMaximum(pData->hScroll)) || (oldHValue != GetControlValue(pData->hScroll)) )
                                    ShowControl(pData->hScroll);
				else
                                    SetControlVisibility(pData->hScroll, true, false);	
				pData->oldHValue = GetControlValue(pData->hScroll);
			}
			if (pData->vScroll)
			{
				if ((oldVMax != GetControlMaximum(pData->vScroll)) || (oldVValue != GetControlValue(pData->vScroll)) )
					ShowControl(pData->vScroll);
				else
                                    SetControlVisibility(pData->vScroll, true, false);	
				pData->oldVValue = GetControlValue(pData->vScroll);
			}
		}

	}
		
	return anErr;
	
} // AdjustScrollBars

// --------------------------------------------------------------------------------------------------------------
// MENU UTILITY ROUTINES
// --------------------------------------------------------------------------------------------------------------
Boolean CommandToIDs(short commandID, short * menuID, short *itemID)
{

	short	** commandHandle;
	short	whichMenu;
	short	oldResFile = CurResFile();
	Boolean	returnValue = false;
	
	UseResFile(gApplicationResFile);
	for (whichMenu = mApple; whichMenu <= mLastMenu; whichMenu++)
	{
		commandHandle = (short**) Get1Resource('MCMD', whichMenu);
		if (commandHandle)
		{
			short	* pCommands = *commandHandle;
			short	commandIndex;
			short	numCommands = pCommands[0];
			
			for (commandIndex = 1; commandIndex <= numCommands; ++commandIndex)
				if (pCommands[commandIndex] == commandID)
				{
					*menuID = whichMenu;
					*itemID = commandIndex;
					
					returnValue = (commandIndex == numCommands);
				}
		}	
	}
		
	UseResFile(oldResFile);
	
	return returnValue;
	
} // CommandToIDs

// --------------------------------------------------------------------------------------------------------------
Boolean IsCommandEnabled(short commandID)
/*
	returns true if a given command is currently enabled
*/
{
	short		whichMenu, whichItem;
	MenuHandle	menu;
	
	CommandToIDs(commandID, &whichMenu, &whichItem);
	menu = GetMenuHandle(whichMenu);
	
	if (IsMenuItemEnabled(menu, whichItem))
		return(true);
	
	return(false);
	
} // IsCommandEnabled

// --------------------------------------------------------------------------------------------------------------
void EnableCommand(short commandID)
/*
	Given a command ID, enables the first menu item with that command ID.
	
	If the command table for a given menu is less than the number of items in the menu,
	and the command being enabled is the last item in the command table, then all
	items from there on down are also enabled.  This is useful for menus that get
	appended to, such as the desk accessory list, font list, or speaking voices list.
*/
{
	short	whichMenu;
	short	whichItem;
	
	if (CommandToIDs(commandID, &whichMenu, &whichItem))
	{
		short		i;
		MenuHandle	menu = GetMenuHandle(whichMenu);
		
		if (menu)
		{
			short		numItems = CountMenuItems(menu);
			
			for (i = whichItem; i <= numItems; ++i)
				EnableMenuItem(menu, i);
		}
	}
	else
	{
		MenuHandle	menu = GetMenuHandle(whichMenu);

		if (menu)
			EnableMenuItem(menu, whichItem);
	}
		
} // EnableCommand

// --------------------------------------------------------------------------------------------------------------
void ChangeCommandName(short commandID, short resourceID, short resourceIndex)
{
	short		whichMenu;
	short		whichItem;
	MenuHandle	menu;
	
	// figure out how this command maps into the menu bar
	CommandToIDs(commandID, &whichMenu, &whichItem);
	menu = GetMenuHandle(whichMenu);
	
	// then make this item into the requested new string
	{
	Str255		theString;
	
	GetIndString(theString, resourceID, resourceIndex);
	SetMenuItemText(menu, whichItem, theString);
	}
	
} // ChangeCommandName

// --------------------------------------------------------------------------------------------------------------
void EnableCommandCheck(short commandID, Boolean check)
{

	short	whichMenu;
	short	whichItem;
	
	if (CommandToIDs(commandID, &whichMenu, &whichItem))
	{
		short		i;
		MenuHandle	menu = GetMenuHandle(whichMenu);
		short		numItems = CountMenuItems(menu);
		
		for (i = whichItem; i <= numItems; ++i)
		{
			EnableMenuItem(menu, i);
			MacCheckMenuItem(menu, i, check);
		}
	}
	else
	{
		MenuHandle	menu = GetMenuHandle(whichMenu);

		EnableMenuItem(menu, whichItem);
		MacCheckMenuItem(menu, whichItem, check);
	}
		
} // EnableCommandCheck


// --------------------------------------------------------------------------------------------------------------
void EnableCommandCheckStyle(short commandID, Boolean check, short style)
{

	short	whichMenu;
	short	whichItem;
	
	if (CommandToIDs(commandID, &whichMenu, &whichItem))
	{
		short		i;
		MenuHandle	menu = GetMenuHandle(whichMenu);
		short		numItems = CountMenuItems(menu);
		
		for (i = whichItem; i <= numItems; ++i)
		{
			EnableMenuItem(menu, i);
			MacCheckMenuItem(menu, i, check);
			SetItemStyle(menu, i, style);
		}
	}
	else
	{
		MenuHandle	menu = GetMenuHandle(whichMenu);

		EnableMenuItem(menu, whichItem);
		MacCheckMenuItem(menu, whichItem, check);
		SetItemStyle(menu, whichItem, style);
	}
		
} // EnableCommandCheckStyle

// --------------------------------------------------------------------------------------------------------------
Boolean AdjustMenus(WindowPtr pWindow, Boolean editDialogs, Boolean forceTitlesOn)
{
	Boolean 				wasEnabled[mNumberMenus];	// Old state of menus
	MenuHandle				menus[mNumberMenus];
	short					whichMenu;			// for stepping through menus
	MenuHandle				menu;				// for reading in menu IDs
	WindowDataPtr 			pData = GetWindowInfo(pWindow);
	
	// Step through all of the menus 
	for (whichMenu = mApple; whichMenu <= mLastMenu; whichMenu++)
	{
		// Save the old state of the menu title 
		menus[whichMenu - mApple] = menu = GetMenuHandle(whichMenu);
		if (menu)		// because contents menu may not be around
		{
			if (forceTitlesOn)				
				wasEnabled[mLastMenu - whichMenu] = false;
			else
				wasEnabled[mLastMenu - whichMenu] = IsMenuItemEnabled(menu, 0);
			
			// Disable the entire menu 
			DisableAllMenuItems( menu );
			DisableMenuItem( menu, 0 );
		}
	}
	
	// select all, unless someone else changes it
	ChangeCommandName(cSelectAll, kMiscStrings, iSelectAllCommand);

	// if we have NO windows, or the current window is one we understand
	if ((pWindow == nil) || (pData))
	{
		// enable the default commands
		EnableCommand(cAbout);
		EnableCommand(cDeskAccessory);
		
		EnableCommand(cNew);
		EnableCommand(cOpen);
		EnableCommand(cQuit);
	
		EnableCommand(cShowClipboard);
	}
	else
	{
		// it's printing or a dialog, so enable cut/copy/paste
		if (editDialogs)
		{
			EnableCommand(cCut);
			EnableCommand(cCopy);
			EnableCommand(cPaste);
			EnableCommand(cClear);
		}
		
		// and desk accs too!		
		EnableCommand(cDeskAccessory);

	}
		
	if ( (pWindow) && (pData) )
	{
		// all windows can be closed
		if (FrontNonFloatingWindow())
			EnableCommand(cClose);

		// changed documents can be saved, but only if the file is open for write
		if ( 	(pData->changed) && 
				((pData->isWritable) || (pData->dataRefNum == -1)) )
			EnableCommand(cSave);
		
		// objects with a print method can be printed and page setup-ed
		if (pData->pPrintPage)
		{
			EnableCommand(cPrint);
			EnableCommand(cPageSetup);
			EnableCommand(cPrintOneCopy);
		}
			
		// let object enable anything else that needs to be enabled
		if (pData->pAdjustMenus)
			(*(pData->pAdjustMenus)) (pWindow, pData);
	}
		
	// Now determine if any of the menus have changed state
	{
	Boolean gotToRedraw = false;
	
	for (whichMenu = mApple; whichMenu <= mLastMenu; ++whichMenu)
	{
		menu = menus[whichMenu - mApple];
	
		if (menu)		// because contents menu may not be around
		{
			// If any of the menu is enabled
			if (MenuHasEnabledItems(menu))
			{
				// Make sure to turn on the menu title
				EnableMenuItem(menu, 0);
			}
				
			/* 	If this new state is different than the saved state, then the menu bar
				will need to be redrawn */
			if (wasEnabled[mLastMenu - whichMenu] != IsMenuItemEnabled(menu, 0))
			{
				gotToRedraw = true;
			}
		}
	}
	
	// Prior to Carbon, it was necessary to invalidate the menubar after changing 
	// a menu's title enable state; in Carbon, the Menu Manager invalidates the
	// menubar automatically.
	
	return gotToRedraw;
	}
		
} // AdjustMenus

// --------------------------------------------------------------------------------------------------------------
// FILE UTILITY ROUTINES
// --------------------------------------------------------------------------------------------------------------
static Boolean BringToFrontIfOpen(FSRefPtr pRef)
{
	WindowPtr		pWindow;
	
	pWindow = FrontNonFloatingWindow();
	while (pWindow)
	{
		WindowDataPtr pData = GetWindowInfo(pWindow);
		
		if ( pData && (FSCompareFSRefs (&(pData->fileRef), pRef) == noErr) )
		{
			SelectWindow(pWindow);
			return true;
		}
			
		pWindow = GetNextWindow(pWindow);
	}
		
	return false;
	
} // BringToFrontIfOpen

// --------------------------------------------------------------------------------------------------------------
static Boolean BringToFrontIfExists(ResType windowKind)
{
	WindowPtr		pWindow;
	
	pWindow = FrontNonFloatingWindow();
	while (pWindow)
	{
		WindowDataPtr pData = GetWindowInfo(pWindow);
		
		if ((pData) && (pData->windowKind == windowKind))
		{
			SelectWindow(pWindow);
			return true;
		}
			
		pWindow = GetNextWindow(pWindow);
	}
		
	return false;
	
} // BringToFrontIfExists

// --------------------------------------------------------------------------------------------------------------
// MAIN SIMPLETEXT ROUTINES
// --------------------------------------------------------------------------------------------------------------
static OSStatus MakeNewWindow(ResType windowKind, FSRefPtr fileRefPtr, OSType fileType, Boolean *pWasAlreadyOpen)
{
	OSStatus		anErr = fnfErr;
	PreflightRecord		thePreflight;
	PreflightWindowProc	pPreflight = nil;
	WindowRef		pWindow;
	WindowDataPtr		pData;
	ControlHandle		rootControl;
	FSSpec			fileSpec;
	CFStringRef		pCFFileName = NULL;
	
	if (fileRefPtr != nil)
	{
		HFSUniStr255	theFileName;
		OSStatus 	theErr = FSGetCatalogInfo( fileRefPtr, kFSCatInfoNone, NULL, &theFileName, &fileSpec, NULL );
		nrequire(theErr, FSGetCatalogInfo);

		pCFFileName = CFStringCreateWithCharacters( NULL, theFileName.unicode, theFileName.length );
	}

	// require a certain amount of RAM free before we allow the new window to be created
	if (FreeMem() < kRAMNeededForNew)
	anErr = memFullErr;
		
	// make sure we update the cursor
	SetEmptyRgn(gCursorRgn);
	
	// <50> if we already have a document open from this file, bring the window to the
	// front and return with no error
	if ( (fileRefPtr) && (fileType != 'sEXT') && (BringToFrontIfOpen(fileRefPtr)) )
	{
		if (pWasAlreadyOpen) *pWasAlreadyOpen = true;
		anErr = noErr;
		return(anErr);
	}
	if (pWasAlreadyOpen) *pWasAlreadyOpen = false;
	if (anErr != fnfErr)
	{
		nrequire(anErr, SanityCheckFailed);
	}
		
	// initialize our behavior
	thePreflight.continueWithOpen 		= true;
	thePreflight.resourceID 		= kDefaultWindowID;
	thePreflight.wantHScroll 		= false;
	thePreflight.wantVScroll 		= false;
	thePreflight.storageSize 		= sizeof(WindowDataRecord);
	thePreflight.makeProcPtr 		= nil;
	thePreflight.openKind			= fsRdPerm;
	thePreflight.needResFork		= false;
	thePreflight.doZoom			= false;
	thePreflight.fileType			= fileType;
	
	switch (windowKind)
	{
		case kAboutWindow:
			pPreflight = AboutPreflightWindow;
			break;

		case kPICTWindow:
			pPreflight = PICTPreflightWindow;
			break;

		case kMovieWindow:
			pPreflight = MoviePreflightWindow;
			break;

		case kClipboardWindow:
			pPreflight = ClipboardPreflightWindow;
			break;

		case kTextWindow:
			pPreflight = TextPreflightWindow;
			break;

#if ALLOW_3D
		case kThreeDWindow:
			pPreflight = ThreeDPreflightWindow;
			break;
#endif
	}
	
	// preflight the window	
	if (pPreflight) anErr = (*pPreflight) (&thePreflight);
	nrequire(anErr, PreflightFailed);
	
	if (thePreflight.continueWithOpen)
	{
		// allocate a place for the window
		pData = (WindowDataPtr)NewPtrClear(thePreflight.storageSize);
		anErr = MemError();
		nrequire(anErr, FailedToAllocateWindow);
		
		// then actually create the window
		pWindow = GetNewCWindow(thePreflight.resourceID, NULL, (WindowPtr)-1);
		if (!pWindow) anErr = memFullErr;
		nrequire(anErr, NewWindowFailed);

		pData->theWindow = pWindow;
		SetWRefCon(pWindow, (long) pData);
				
		if (gMachineInfo.haveAppearanceMgr)
			CreateRootControl(pWindow, &rootControl);
				
		// zoom the rectangle to big size on this monitor 
		// based upon which scroll bars they want
		{
		Rect	rect;
		Rect	bigRect;

		GetWindowPortBounds(pWindow, &rect);
		bigRect = (**GetMainDevice()).gdRect;

		bigRect.top += GetMBarHeight() * 2;
		bigRect.left += 4;
		bigRect.bottom -= 4;
		bigRect.right -= 65;	// ••• this is probably attempting to leave space for icons on the desktop.

		SetPortWindowPort (pWindow);
		LocalToGlobal(&TopLeft(rect));
		LocalToGlobal(&BotRight(rect));
		
		// can't have an invalid rectangle - this doesn't seem likely to happen
//		if (rect.right - rect.left < 32)
//			rect.right = rect.left + 32;

///		MoveWindow(pWindow, rect.left, rect.top, false);
		SizeWindow(pWindow, rect.right - rect.left, rect.bottom - rect.top, false);
		}
		
		// fill in the default contents of the window
		pData->windowKind 		= windowKind;
		pData->originalFileType		= fileType;
		pData->pMakeWindow 		= (MakeWindowProc)thePreflight.makeProcPtr;
		pData->resRefNum		= -1;
		pData->dataRefNum		= -1;
		pData->isClosing		= false;

		GetWindowPortBounds(pWindow, &pData->contentRect);
		
		// make the scroll bars
		{
		Rect	controlRect;
		
		if (thePreflight.wantHScroll)
		{
			pData->contentRect.bottom -= kScrollBarSize;
			GetWindowPortBounds(pWindow, &controlRect);
			controlRect.top = controlRect.bottom - (kScrollBarSize + 1);
			if (thePreflight.wantVScroll)
				controlRect.right -= kGrowScrollAdjust;
			OffsetRect(&controlRect, -1, 1);
			// if compiling native, use CreateScrollBarControl, else use NewControl till CarbonLib 1.1
#if ( COMPILING_CARBONLIB )     //      CarbonLib 1.1 will support new control APIs
			pData->hScroll = NewControl (pWindow,&controlRect,"\p",true,0,0,0,kControlScrollBarLiveProc,0);
#else
			CreateScrollBarControl(pWindow, &controlRect, 0, 0, 0, 0, true, gHActionProc, &pData->hScroll );
#endif
		}
		if (thePreflight.wantVScroll)
		{
			pData->contentRect.right -= kScrollBarSize;
			GetWindowPortBounds(pWindow, &controlRect);
			controlRect.left = controlRect.right - (kScrollBarSize + 1);
			if (thePreflight.wantVScroll)
				controlRect.bottom -= kGrowScrollAdjust;
			OffsetRect(&controlRect, 1, -1);
			// if compiling native, use CreateScrollBarControl, else use NewControl till CarbonLib 1.1
#if ( COMPILING_CARBONLIB )     //      CarbonLib 1.1 will support new control APIs
			pData->vScroll = NewControl (pWindow,&controlRect,"\p",true,0,0,0,kControlScrollBarLiveProc,0);
#else
			CreateScrollBarControl(pWindow, &controlRect, 0, 0, 0, 0, true, gVActionProc, &pData->vScroll );
#endif

			{
				EventTypeSpec		mouseWheelEvent = { kEventClassMouse, kEventMouseWheelMoved };
				InstallEventHandler( GetWindowEventTarget( pWindow ), GetMouseWheelHandler(), 1, &mouseWheelEvent, pData, NULL );
			}

		}
		}

		// got a name?  Open the file		
		if (fileRefPtr)
		{
			anErr = FSOpenFork(fileRefPtr, 0, NULL, thePreflight.openKind, &pData->dataRefNum);
			if 	( 
					((anErr == afpAccessDenied) || (anErr == opWrErr) || (anErr == permErr) ) && 
					(thePreflight.openKind != fsRdPerm)
				)
			{
				thePreflight.openKind = fsRdPerm;
				pData->isWritable = false;
				anErr = FSOpenFork(fileRefPtr, 0, NULL, thePreflight.openKind, &pData->dataRefNum);
			}
			else
				pData->isWritable = true;
			nrequire(anErr, FailedToOpenFile);

			// okay not to find a resource fork, because some don't have them				
			pData->resRefNum = FSOpenResFile(fileRefPtr, thePreflight.openKind);
			
			// save the file spec in case someone is interested
			pData->fileRef = *fileRefPtr;
			
			// alias for background file synchronization
			anErr = FSNewAlias( NULL, fileRefPtr, &pData->fileAlias );
			nrequire(anErr, GoshNoAlias );
			
			//
			// Most windows are not modifiable in SimpleText - only text files can be modified by SimpleText	
			//
			if( gMachineInfo.haveProxyIcons )
			{
				SetWindowModified( pWindow, false );
				SetWindowProxyFSSpec( pWindow, &fileSpec );
			}

			}
			
		if (pData->pMakeWindow)
		{
			Rect oldContent = pData->contentRect;
#ifdef __MWERKS__
#if __option(profile)
			ProfilerSetStatus(true);
#endif
#endif
			anErr = (*(pData->pMakeWindow)) (pWindow, pData);
#ifdef __MWERKS__
#if __option(profile)
			ProfilerDump("\pmakewindow.prof");
			ProfilerSetStatus(false);
			ProfilerClear();
#endif
#endif
			if (!EqualRect(&oldContent, &pData->contentRect))
			{
				SizeWindow(pWindow, 
						pData->contentRect.right  + (pData->vScroll != 0) * kScrollBarSize,
						pData->contentRect.bottom + (pData->hScroll != 0) * kScrollBarSize,
						false);
			}
		}
		nrequire(anErr, FailedMakeWindow);

		// got a name?  Use it as the window title
		if ( (fileRefPtr) && (!pData->openAsNew) )
		{
 			SetWindowTitleWithCFString(pWindow, pCFFileName );
		}
		else
		{
			if ((gMachineInfo.documentCount == 1) && (pData->windowKind == kTextWindow))
			{
				Str255 tempString;
		
				GetIndString(tempString, kMiscStrings, iFirstNewDocumentTitle);	// get the "untitled" string (no number)
				SetWTitle(pWindow, tempString);
			}
			else
			{
				Str255	tempString;
				Str32	numString;
	
				GetWTitle(pWindow, tempString);
				NumToString(gMachineInfo.documentCount, numString);
				(void) ZeroStringSub(tempString, numString);
				SetWTitle(pWindow, tempString);
			}

			if (pData->bumpUntitledCount)
				gMachineInfo.documentCount++;	// bump count if appropriate for this kind of document
		}

		// Make sure the scroll bars are reasonable in size, and move if they must
		AdjustScrollBars(pWindow, true, true, nil);
		
		// Show the scrollbars
		if( pData->hScroll != NULL )
			ShowControl( pData->hScroll );
			
		if( pData->vScroll != NULL )
			ShowControl( pData->vScroll );
		
		// finally, if all goes well, we can see the window itself!
		ShowWindow(pWindow);
		}

	if (pCFFileName) CFRelease(pCFFileName);
	return noErr;

// EXCEPTION HANDLING

FailedMakeWindow:
	DisposeHandle( (Handle)pData->fileAlias );
	
GoshNoAlias:
	if (pData->resRefNum != -1)
		CloseResFile(pData->resRefNum);
	if (pData->dataRefNum != -1)
		FSClose(pData->dataRefNum);
		
FailedToOpenFile:
	DisposeWindow(pWindow);
	
NewWindowFailed:
	DisposePtr((Ptr)pData);
	
FailedToAllocateWindow:
PreflightFailed:
SanityCheckFailed:
FSGetCatalogInfo:
	if (pCFFileName) CFRelease(pCFFileName);
	return anErr;
	
} // MakeNewWindow


static EventHandlerUPP
GetMouseWheelHandler()
{
	static EventHandlerUPP	sHandler = NULL;

	if ( sHandler == NULL )
		sHandler = NewEventHandlerUPP( MouseWheelHandler );

	return sHandler;
}

static OSStatus
MouseWheelHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* userData )
{
	OSStatus				result = eventNotHandledErr;
	EventMouseWheelAxis		axis;
	SInt32					delta;
	WindowDataPtr 			pData = (WindowDataPtr)userData;

	GetEventParameter( inEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis,
		NULL, sizeof( EventMouseWheelAxis ), NULL, &axis );	
	GetEventParameter( inEvent, kEventParamMouseWheelDelta, typeLongInteger,
		NULL, sizeof( SInt32 ), NULL, &delta );	

	if ( axis == kEventMouseWheelAxisY )
	{
		VActionProc( pData->vScroll, delta > 0 ? kControlUpButtonPart : kControlDownButtonPart );

		result = noErr;
	}
	
	return result;
}

// --------------------------------------------------------------------------------------------------------------
static Boolean CanCloseWindow( WindowPtr pWindow )
{
	WindowDataPtr	pData = GetWindowInfo( pWindow );
	return ( pData == NULL || !pData->changed );
}


// --------------------------------------------------------------------------------------------------------------
static OSStatus DoCloseWindow( WindowPtr pWindow, Boolean discard, long keyTime )
{
	OSStatus		anErr = noErr;
	WindowDataPtr 	pData = GetWindowInfo(pWindow);
	
	if ( !discard && (pData) && (pData->changed) )
	{
		if ( !pData->isClosing )
		{
			CFStringRef	wTitle;
			Cursor		arrow;
	
			pData->isClosing = true;

			CopyWindowTitleAsCFString(pWindow, &wTitle);
			SetCursor(GetQDGlobalsArrow(&arrow));
			
			anErr = ConfirmSaveDialog( pWindow, wTitle, gMachineInfo.isQuitting, pData, &pData->navDialog );
			if ( anErr != noErr )
			{
				pData->isClosing = false;
			}
			CFRelease(wTitle);
		}
	}
	else
	{
		if ( (pData) && (pData->pCloseWindow) )
		{
			// let the object close the window if it wishes to
			anErr = (*(pData->pCloseWindow)) (pWindow, pData);
		}

		// otherwise we close it the default way
		if (anErr == noErr)
		{
			if (pData)
			{
				// Close any Nav dialog that is running.
				if ( pData->navDialog != NULL )
				{
					TerminateDialog( pData->navDialog );
					pData->navDialog = NULL;
				}

				DisposeWindow(pWindow);
	
				if (pData->hPageFormat)
				{
					DisposeHandle((Handle) pData->hPageFormat);
					pData->hPageFormat = nil;
				}				
				if (pData->fileAlias)
					DisposeHandle( (Handle)pData->fileAlias );
				if (pData->resRefNum != -1)
					CloseResFile(pData->resRefNum);
				if (pData->dataRefNum != -1)
					FSClose(pData->dataRefNum);
				if (pData->printSession)
					PMRelease(pData->printSession);

				DisposePtr((Ptr) pData);
			}
		}
	}

	// If we closed the last window, clean up
	if (FrontNonFloatingWindow() == nil)
	{
		UnhiliteMenuDelayed(keyTime);		// unhilite before adjusting menus to avoid ugly menu title flashing
		AdjustMenus(nil, true, false);
		gMachineInfo.documentCount = 1;		// back to "untitled"
	}
	
	// make sure we update the cursor
	SetEmptyRgn(gCursorRgn);
	
	return anErr;
	
} // DoCloseWindow

// --------------------------------------------------------------------------------------------------------------
static OSStatus	DetermineWindowTypeOrOpen(
	FSRefPtr theRefPtr, OSType theType, 				// optional input params -- file to open
	OSType *returnedTypeList, short * pNumTypes,	// optional input params -- returns list of files
	Boolean *pWasAlreadyOpen)						// optional input params -- was file already open
{
	OSStatus	anErr = noErr;
	OSType		typeList[20];
	OSType		docList[20];
	short		numTypes;

	// use local copies if the input params are nil	
	if (returnedTypeList == nil)
		returnedTypeList = &typeList[0];
	if (pNumTypes == nil)
		pNumTypes = &numTypes;
	*pNumTypes = 0;
	
	// Load up all of the file types we know how to handle
	AboutGetFileTypes(returnedTypeList, docList, pNumTypes);
	PICTGetFileTypes(returnedTypeList, docList, pNumTypes);
	MovieGetFileTypes(returnedTypeList, docList, pNumTypes);
	ClipboardGetFileTypes(returnedTypeList, docList, pNumTypes);
	TextGetFileTypes(returnedTypeList, docList, pNumTypes);
#if ALLOW_3D
	ThreeDGetFileTypes(returnedTypeList, docList, pNumTypes);
#endif

	if (theRefPtr != nil)
	{
		short	 	index;
		OSType		windowType = '\?\?\?\?';

		for (index = 0; index < (*pNumTypes); ++index)
			if (theType == returnedTypeList[index])
				windowType = docList[index];
		
		if (windowType == '\?\?\?\?' && theType && gMachineInfo.haveQuickTime)
		{
			ComponentDescription	ci;
			ci.componentType = GraphicsImporterComponentType;
			ci.componentSubType = theType;
			ci.componentManufacturer = kAnyComponentManufacturer;
			ci.componentFlags = 0;
			ci.componentFlagsMask = movieImportSubTypeIsFileExtension;
			if( 0 != FindNextComponent( 0, &ci ) )
			{
				windowType = 'PICT';
			}
		}
		
		if (windowType != '\?\?\?\?')
		{
			
			if ( (theType == 'TEXT') || (theType == 'sEXT') )
			{
				FSCatalogInfo	theCatInfo;
				
				FSGetCatalogInfo (theRefPtr, kFSCatInfoFinderInfo, &theCatInfo, NULL, NULL, NULL);
				if ( ((FInfo*)&theCatInfo.finderInfo)->fdFlags & kIsStationery )
					theType = 'sEXT';
				else
					theType = 'TEXT';
			}
		}
		else
		{
			windowType = kTextWindow;
		}
			
		anErr = MakeNewWindow(windowType, theRefPtr, theType, pWasAlreadyOpen);
	}
		
		
	return anErr;
	
} // DetermineWindowTypeOrOpen

// --------------------------------------------------------------------------------------------------------------

static OSStatus DoOpenWindow(void)
{
	short				numTypes;
	OSType				typeList[20];
	OSType				fileType = '\?\?\?\?';
	NavDialogRef		navDialog;

	DetermineWindowTypeOrOpen( nil, fileType, &typeList[0], &numTypes, nil );
	
	// Open as many documents as the user wishes through Appleevents
	return OpenFileDialog( 'ttxt', numTypes, typeList, &navDialog );
} // DoOpenWindow

// --------------------------------------------------------------------------------------------------------------
static OSStatus DoUpdateWindow(WindowPtr pWindow)
{
	OSStatus		anErr = noErr;
	WindowDataPtr	pData = GetWindowInfo(pWindow);
	GrafPtr			curPort;
	
	// only handle updates for windows we know about
	if (pData)
	{
		GetPort(&curPort);
		SetPortWindowPort (pWindow);
		BeginUpdate(pWindow);
					
		if (pData->pUpdateWindow)
			anErr = (*(pData->pUpdateWindow)) (pWindow, pData);
	
		EndUpdate(pWindow);
		SetPort(curPort);
	}
	
	return anErr;
	
} // DoUpdateWindow

// --------------------------------------------------------------------------------------------------------------
OSStatus DoScrollContent(WindowPtr pWindow, WindowDataPtr pData, short deltaH, short deltaV)
{
	OSStatus	anErr = noErr;
	
	if ((deltaH) || (deltaV))
	{		
		if ((pData) && (pData->pScrollContent))
			anErr = (*(pData->pScrollContent)) (pWindow, pData, deltaH, deltaV);
			
		if (anErr == noErr)
		{
			RgnHandle	invalidRgn = NewRgn();
			
			ScrollRect(&pData->contentRect, deltaH, deltaV, invalidRgn);
			InvalWindowRgn(pWindow,invalidRgn);
			DisposeRgn(invalidRgn);
	
			(void) DoUpdateWindow(pWindow);
		}
	}
	
	return anErr;
	
} // DoScrollContent


// --------------------------------------------------------------------------------------------------------------
static OSStatus DoContentClick(WindowPtr pWindow)
{
	OSStatus			anErr = noErr;
	WindowDataPtr 	pData = GetWindowInfo(pWindow);
	
	
	if ( pData )
	{
		SetPortWindowPort (pWindow);
		
		if (pData->pContentClick)
		{
			// let the object handle the click if it wishes to
			anErr = (*(pData->pContentClick)) (pWindow, pData, &gEvent);
			
			// invalidate the cursor rgn in case the object changed its appearance
			if (anErr != noErr)
				SetEmptyRgn(gCursorRgn);
		}
		
		if (anErr == noErr) 
		{
			ControlHandle	theControl;
			short			part;
			
			GlobalToLocal(&gEvent.where);
			part = FindControl(gEvent.where, pWindow, &theControl);
			switch (part)
			{
				// do nothing for viewRect case
				case 0:
					break;

				// track the thumb, and then update all at once
				case kControlIndicatorPart:
				{
					short	value = GetControlValue(theControl);
					
					if (gMachineInfo.haveAppearanceMgr)
					{
						if (theControl == pData->hScroll)
						{
							pData->oldHValue = GetControlValue(theControl);
							part = TrackControl(theControl, gEvent.where, gHActionProc);
						}
						if (theControl == pData->vScroll)
						{
							pData->oldVValue = GetControlValue(theControl);
							part = TrackControl(theControl, gEvent.where, gVActionProc);
						}
					}
					else
					{
						part = TrackControl(theControl, gEvent.where, nil);
						if (part != 0)
						{
							// turn the value into a delta
							value -= GetControlValue(theControl);
							
							// if we actually moved
							if (value != 0)
							{
								if (theControl == pData->hScroll)
									DoScrollContent(pWindow, pData, value, 0);
								if (theControl == pData->vScroll)
									DoScrollContent(pWindow, pData, 0, value);
									
							}
						}
					}
				}
					break;

				// track the control, and scroll as we go
				default:
					if (theControl)
					{
						if (theControl == pData->hScroll)
							part = TrackControl(theControl, gEvent.where, gHActionProc);
						if (theControl == pData->vScroll)
						{
							gVScrollTrackStartTicks = TickCount();
							gVScrollAccelerator = kVScrollAcceleratorIncrement;
							part = TrackControl(theControl, gEvent.where, gVActionProc);
						}
					}
					break;
				}	// end switch (part)
		}	// end if (anErr == noErr)

	}	// end if ( pData )

		
	return anErr;
	
} // DoContentClick

// --------------------------------------------------------------------------------------------------------------
static OSStatus DoGrowWindow(WindowPtr pWindow, EventRecord *pEvent)
{
	OSStatus			anErr = noErr;
	WindowDataPtr 	pData = GetWindowInfo(pWindow);
	Rect			tempRect;
	LongRect		docRect;
	long			growResult;
	
	if (pData)
	{
		SetPortWindowPort (pWindow);
		
		RectToLongRect(&pData->contentRect, &docRect);
		if (pData->pGetDocumentRect)
			(*(pData->pGetDocumentRect)) (pWindow, pData, &docRect, true);
		if (pData->vScroll)
			docRect.right += 16;
		if (pData->hScroll)
			docRect.bottom += 16;
		
		if ( (pData->hasGrow) && (pData->hScroll == nil) && (pData->vScroll == nil) )
		{
			docRect.right += 16;
			docRect.bottom += 16;
		}
			
		// set up resize constraints
		tempRect.left = pData->minHSize;
		if (tempRect.left == 0)
			tempRect.left = kMinDocSize;
		tempRect.right = docRect.right - docRect.left;
		if (tempRect.right < tempRect.left)
			tempRect.right = tempRect.left;
		tempRect.top = pData->minVSize;
		if (tempRect.top == 0)
			tempRect.top = kMinDocSize;
		tempRect.bottom = docRect.bottom - docRect.top;
		if (tempRect.bottom < tempRect.top)
			tempRect.bottom = tempRect.top;
			
		growResult = GrowWindow(pWindow, pEvent->where, &tempRect);
		if ( growResult != 0 ) 
		{
			Rect		oldRect;
			Boolean		needInvalidate;
			
			// save old content area
			oldRect = pData->contentRect;
			
			// grow window and recalc what is needed
			SizeWindow(pWindow, growResult & 0xFFFF, growResult >> 16, true);
			GetWindowPortBounds(pWindow, &pData->contentRect);
			AdjustScrollBars(pWindow, true, true, &needInvalidate);
			
			if (needInvalidate)
			{
				InvalWindowRect(pWindow, &pData->contentRect);
			}

			// if we have offset scrollbars, then force a redraw of them
			if (pData->hScrollOffset)
			{
				GetWindowPortBounds(pWindow, &oldRect);
				oldRect.right = oldRect.left + pData->hScrollOffset;
				oldRect.top = oldRect.bottom - kScrollBarSize;
				InvalWindowRect(pWindow,&oldRect);
			}
			if (pData->vScrollOffset)
			{
				GetWindowPortBounds(pWindow, &oldRect);
				oldRect.bottom = oldRect.top + pData->vScrollOffset;
				oldRect.left = oldRect.right - kScrollBarSize;
				InvalWindowRect(pWindow,&oldRect);
			}
		}
			
	}
		
	
	return anErr;
	
} // DoGrowWindow

// --------------------------------------------------------------------------------------------------------------
static OSStatus DoZoomWindow(WindowPtr pWindow, short zoomDir)
{
	WindowDataPtr 		pData = GetWindowInfo(pWindow);
	Rect				windRect, zoomRect;
	Rect				globalPortRect, theSect, dGDRect;
	GDHandle			nthDevice, dominantGDevice;
	long				sectArea, greatestArea;
	short 				hMax, vMax;
	Rect				bounds;

	// determine the max size of the window
	{
	WindowDataPtr			pData = GetWindowInfo(pWindow);
	LongRect			docRect;
	
	RectToLongRect(&pData->contentRect, &docRect);
	if (pData->pGetDocumentRect)
		(*(pData->pGetDocumentRect)) (pWindow, pData, &docRect, true);
	if (pData->vScroll)
		docRect.right += kScrollBarSize;
	if (pData->hScroll)
		docRect.bottom += kScrollBarSize;
	
	if ( (pData->hasGrow) && (pData->hScroll == nil) && (pData->vScroll == nil) )
		{
		docRect.right += kScrollBarSize;
		docRect.bottom += kScrollBarSize;
		}

	hMax = docRect.right - docRect.left;
	vMax = docRect.bottom - docRect.top;
	}

	SetPortWindowPort (pWindow);
	GetWindowPortBounds(pWindow, &bounds);
	EraseRect(&bounds);	// recommended for cosmetic reasons

	if (zoomDir == inZoomOut) 
	{

		/*
		 *	ZoomWindow() is a good basic tool, but it doesn't do everything necessary to
		 *	implement a good human interface when zooming. In fact it's not even close for
		 *	more high-end hardware configurations. We must help it along by calculating an
		 *	appropriate window size and location any time a window zooms out.
		 */
		{
		RgnHandle	structRgn = NewRgn();
		
		GetWindowRegion(pWindow, kWindowStructureRgn, structRgn);

		GetRegionBounds( structRgn, &windRect);

		DisposeRgn(structRgn);
		}
		dominantGDevice = nil;

		/*
		 *	Color QuickDraw implies the possibility of multiple monitors. This is where
		 *	zooming becomes more interesting. One should zoom onto the monitor containing
		 *	the greatest portion of the window. This requires walking the gDevice list.
		 */

		nthDevice = GetDeviceList();
		greatestArea = 0;
		while (nthDevice != nil) 
		{
			if (TestDeviceAttribute(nthDevice, screenDevice)) 
			{
				if (TestDeviceAttribute(nthDevice, screenActive)) 
				{
					SectRect(&windRect, &(**nthDevice).gdRect, &theSect);
					sectArea = (long) RectWidth(theSect) * (long) RectHeight(theSect);
					if (sectArea > greatestArea) 
					{
						greatestArea = sectArea;		// save the greatest intersection
						dominantGDevice = nthDevice;	// and which device it belongs to
					}
				}
			}
			nthDevice = GetNextDevice(nthDevice);
		}

		/*
		 *	At this point, we know the dimensions of the window we're zooming, and we know
		 *	what screen we're going to put it on. To be more specific, however, we need a
		 *	rectangle which defines the maximum dimensions of the resized window's contents.
		 *	This rectangle accounts for the thickness of the window frame, the menu bar, and
		 *	one or two pixels around the edges for cosmetic compatibility with ZoomWindow().
		 */

		if (dominantGDevice != nil) 
		{
			dGDRect = (**dominantGDevice).gdRect;
			if (dominantGDevice == GetMainDevice())		// account for menu bar on main device
				dGDRect.top += GetMBarHeight();
		}
		else 
		{
			BitMap bitmap;
		    	GetQDGlobalsScreenBits(&bitmap);
			dGDRect = bitmap.bounds
;	// if no gDevice, use default monitor
			dGDRect.top += GetMBarHeight();
	}

		GetWindowPortBounds(pWindow, &globalPortRect);
		LocalToGlobal(&TopLeft(globalPortRect));		// calculate the window's portRect
		LocalToGlobal(&BotRight(globalPortRect));		// in global coordinates

		// account for the window frame and inset it a few pixels
		dGDRect.left	+= 2 + globalPortRect.left - windRect.left;
		dGDRect.top		+= 2 + globalPortRect.top - windRect.top;
		dGDRect.right	-= 1 + windRect.right - globalPortRect.right;
		dGDRect.bottom	-= 1 + windRect.bottom - globalPortRect.bottom;

		/*
		 *	Now we know exactly what our limits are, and since there are input parameters
		 *	specifying the dimensions we'd like to see, we can move and resize the zoom
		 *	state rectangle for the best possible results. We have three goals in this:
		 *	1. Display the window entirely visible on a single device.
		 *	2. Resize the window to best represent the dimensions of the document itself.
		 *	3. Move the window as short a distance as possible to achieve #1 and #2.
		 */

		//zoomRect = &(**(WStateDataHandle) ((WindowPeek) pWindow)->dataHandle).stdState;
		GetWindowIdealUserState(pWindow, &zoomRect);
		/*
		 *	Initially set the zoom rectangle to the size requested by the input parameters,
		 *	although not smaller than a minimum size. We do this without moving the origin.
		 */

		zoomRect.right = (zoomRect.left = globalPortRect.left) +
								Max(hMax, kMinDocSize);
		zoomRect.bottom = (zoomRect.top = globalPortRect.top) +
								Max(vMax, kMinDocSize);

		// Shift the entire rectangle if necessary to bring its origin inside dGDRect.
		OffsetRect(&zoomRect,
					Max(dGDRect.left - zoomRect.left, 0),
					Max(dGDRect.top - zoomRect.top, 0));

		/*
		 *	Shift the rectangle up and/or to the left if necessary to accomodate the view,
		 *	and if it is possible to do so. The rectangle may not be moved such that its
		 *	origin would fall outside of dGDRect.
		 */

		OffsetRect(&zoomRect,
					-Pin(zoomRect.right - dGDRect.right, 0, zoomRect.left - dGDRect.left),
					-Pin(zoomRect.bottom - dGDRect.bottom, 0, zoomRect.top - dGDRect.top));

		// Clip expansion to dGDRect, in case view is larger than dGDRect.
		zoomRect.right = Min(zoomRect.right, dGDRect.right);
		zoomRect.bottom = Min(zoomRect.bottom, dGDRect.bottom);

		SetWindowIdealUserState(pWindow, &zoomRect);
	}

	ZoomWindow(pWindow, zoomDir, pWindow == FrontNonFloatingWindow());
	
	GetWindowPortBounds(pWindow, &pData->contentRect);
	AdjustScrollBars(pWindow, true, true, nil);

	InvalWindowRect(pWindow, &pData->contentRect);
	
	return noErr;
	
} // DoZoomWindow

// --------------------------------------------------------------------------------------------------------------
OSStatus DoActivate(WindowPtr pWindow, Boolean activating)
{

	OSStatus			anErr = noErr;
	WindowDataPtr 	pData = GetWindowInfo(pWindow);

	SetPortWindowPort (pWindow);
	
	if ( pData )
	{
		if (pData->pActivateEvent)
		{
			anErr = (*(pData->pActivateEvent)) (pWindow, pData, activating);
		}
	
		if (anErr == noErr)
		{
			if (activating)
			{
				if (pData->hScroll)
					HiliteControl( pData->hScroll, 0 );
				if (pData->vScroll)
					HiliteControl( pData->vScroll, 0 );
			}
			else
			{
				if (pData->hScroll)
					HiliteControl( pData->hScroll, kControlDisabledPart );
				if (pData->vScroll)
					HiliteControl( pData->vScroll, kControlDisabledPart );
			}
			
			
			if ( pData->hasGrow )
			{
				Rect	growIconRect;
				
				CalculateGrowIcon(pWindow,pData, &growIconRect);
				InvalWindowRect(pWindow,&growIconRect);
			}
		}
	}

	if ( activating )
		AdjustMenus(pWindow, true, false);
		
	return anErr;
	
} // DoActivate

OSStatus DoDefault(WindowDataPtr pData) {
    OSStatus status = noErr;

    if (pData->printSession == nil)
    {
	status = PMCreateSession(&pData->printSession);
    }
    
    // create default page format if we don't have one already
    if ((status == noErr) && (pData->hPageFormat == nil))
    {
	PMPageFormat pageFormat = kPMNoPageFormat;

	status = PMCreatePageFormat(&pageFormat);
	if ((status == noErr) && (pageFormat != kPMNoPageFormat))
	    status = PMSessionDefaultPageFormat(pData->printSession, pageFormat);

	if (status == noErr)
	    status = PMFlattenPageFormat(pageFormat, &pData->hPageFormat);

	if (pageFormat != kPMNoPageFormat)
	    (void)PMRelease(pageFormat);
    }
	
    return status;
}

// --------------------------------------------------------------------------------------------------------------
OSStatus DoPageSetup(WindowPtr pWindow)
{
    OSStatus		status = noErr;
    WindowDataPtr 	pData = GetWindowInfo(pWindow);
    Boolean 		accepted;
		
    status = DoDefault(pData);
    nrequire(status, DoDefault);
	
    if (status == noErr && pData->hPageFormat != nil)
    {
	PMPageFormat pageFormat = kPMNoPageFormat;

	// retrieve the default page format
	status = PMUnflattenPageFormat(pData->hPageFormat, &pageFormat);

	if ((status == noErr) && (pageFormat != kPMNoPageFormat))
	{
		Cursor arrow;

		SetCursor(GetQDGlobalsArrow(&arrow));
		status = PMSessionPageSetupDialog(pData->printSession, pageFormat, &accepted);
		if (!accepted)
			status = kPMCancel;
	}

	// save any changes to the page format
	if (status == noErr && accepted)
	{
		DisposeHandle( pData->hPageFormat );
		pData->hPageFormat = nil;
		status = PMFlattenPageFormat(pageFormat, &pData->hPageFormat);
	}
	if (pageFormat != kPMNoPageFormat)
		(void)PMRelease(pageFormat);
    }

// FALL THROUGH EXCEPTION HANDLING
DoDefault:		
	if (pData->printSession){			// DMG850 begin Don't keep the session around after PageSetup
	    PMRelease(pData->printSession);
	    pData->printSession = NULL;
	}						// DMG850 end
	return status;
	
} // DoPageSetup

// --------------------------------------------------------------------------------------------------------------
static OSStatus	DoPrint(WindowPtr pWindow, Boolean oneCopy, Boolean doDialog)
{
	OSStatus status;
	PMPageFormat pageFormat = kPMNoPageFormat;
	PMPrintSettings printSettings = kPMNoPrintSettings;
	WindowDataPtr 	pData = GetWindowInfo(pWindow);

	status = DoDefault(pData);

	// create default print settings
	if (status == noErr)
	{
	    status = PMCreatePrintSettings(&printSettings);
	    if ((status == noErr) && (printSettings != kPMNoPrintSettings))
		status = PMSessionDefaultPrintSettings(pData->printSession, printSettings);
	}

	if (status == noErr && (printSettings != nil) && (pData->hPageFormat != nil))
	{    
	    // retrieve the page format
	    status = PMUnflattenPageFormat(pData->hPageFormat, &pageFormat);
	    if ((status == noErr) && (pageFormat != kPMNoPageFormat) && doDialog)
	    {
		Boolean accepted;
		Cursor arrow;
		SetCursor(GetQDGlobalsArrow(&arrow));

		status = PMSessionPrintDialog(pData->printSession, printSettings, pageFormat, &accepted );
		if (status == noErr && !accepted)
		    status = eUserCanceled;
	    }
    	}
		
	if (status == noErr)
	{
	    long	pageIndex;
	    UInt32	firstPage, lastPage;
			
		// be sure to get the page range BEFORE calling PrValidate(), 
		// which blows it away for many drivers.
	    status = PMGetFirstPage(printSettings, &firstPage);
	    if (status == noErr)
		status = PMGetLastPage(printSettings, &lastPage);

	    if (status == noErr)
		status = PMSessionValidatePrintSettings(pData->printSession, printSettings, kPMDontWantBoolean);

	    if (status == noErr)
		status = PMGetFirstPage(printSettings, &firstPage);
	    if (status == noErr)
		status = PMGetLastPage(printSettings, &lastPage);

	    if (status == noErr)
		status = PMSetFirstPage(printSettings, 1, true);
	    
	    if (status == noErr)
		    status = PMSetLastPage(printSettings, kPMPrintAllPages, true);

	    if ((status == noErr) && (oneCopy))
		    status = PMSetCopies(printSettings, 1, true);

	    if (status == noErr)
		{
		    Rect	pageRect;
				    
		    status = PMSessionBeginDocument(pData->printSession, printSettings, pageFormat);
		    if (status == noErr)
		    {
			OSStatus tempErr;
			PMRect  tempPageRect;
			status = PMGetAdjustedPageRect(pageFormat, &tempPageRect);
			if (status == noErr)
			{
			    pageRect.top = tempPageRect.top;
			    pageRect.left = tempPageRect.left;
			    pageRect.bottom = tempPageRect.bottom;
			    pageRect.right = tempPageRect.right;

			    if (firstPage < 1)
				firstPage = 1;
			    if (lastPage < firstPage)
				lastPage = firstPage;
			    for (pageIndex = firstPage; pageIndex <= lastPage && PMSessionError(pData->printSession) == noErr; ++pageIndex)
			    {
				status = PMSessionBeginPage(pData->printSession, pageFormat, NULL);
				if (status == noErr){
				    GrafPtr origPort, printingPort;
				    // Save the current QD grafport.
				    GetPort(&origPort);
	
				    status = PMSessionGetGraphicsContext(pData->printSession, 
							kPMGraphicsContextQuickdraw, (void**) &printingPort);
				    if (status == noErr) {
					// Set the printing port before drawing the page.
					SetPort((GrafPtr) printingPort);
							    
					// Draw the page.
					status = (OSStatus)(*(pData->pPrintPage)) (pWindow, pData, &pageRect, &pageIndex);
		    
					// Restore the QD grafport.
					SetPort(origPort);
				    }
				    // if we called PMSessionBeginPage and got no error then we MUST
				    // call PMSessionEndPage
				    tempErr = PMSessionEndPage(pData->printSession);
				    if(status == noErr)
					status = tempErr;
				}

				if ((status != noErr) || (pageIndex == -1))
				break;
			    }
			}

			tempErr = PMSessionEndDocument(pData->printSession);
			if(status == noErr)
			    status = tempErr;

		    }
		}
	}

	if (printSettings != kPMNoPrintSettings)
	    (void)PMRelease(printSettings);
	if (pageFormat != kPMNoPageFormat)
	    (void)PMRelease(pageFormat);
	
	if (pData->printSession){			// DMG850 release the session when done
	    PMRelease(pData->printSession);
	    pData->printSession = NULL;
	}
	
	return status;
	
	
} // DoPrint


static OSStatus DoQuit()
{
	OSStatus	status;
	UInt32		dirtyCount = 0;
	WindowPtr	pWindow, nextWindow, dirtyWindow = NULL;
	NavUserAction	userAction = kNavUserActionNone;

	if ( !gMachineInfo.isQuitting )
	{
		gMachineInfo.isQuitting = true;
	
		// See how many unsaved windows there are
		pWindow = FrontNonFloatingWindow();
		while ( (pWindow != NULL) && dirtyCount < 2 )
		{
			nextWindow = GetNextWindow(pWindow);
			if ( !CanCloseWindow( pWindow ))
			{
				dirtyWindow = pWindow;
				dirtyCount++;
			}
			pWindow = nextWindow;
		}	
		
		if ( dirtyCount == 1 )
		{
			CFStringRef 	wTitle;
			WindowDataPtr	pData = GetWindowInfo( dirtyWindow );
	
			CopyWindowTitleAsCFString( dirtyWindow, &wTitle);
			if ( pData )
			{
				pData->isClosing = true;
			}
			// Call the confirm save for the document, but present it app-modal
			status = ModalConfirmSaveDialog( wTitle, true, &userAction );
			CFRelease(wTitle);
			if ( status != noErr )
			{
				gMachineInfo.isQuitting = false;
				if ( pData )
				{
					pData->isClosing = false;
				}
			}
			else 
			{
				switch( userAction )
				{
					case kNavUserActionCancel:
					{
						// If we were closing, we're not now.
						pData->isClosing = false;
						gMachineInfo.isClosing = false;
						gMachineInfo.isQuitting = false;
					}
					break;
					
					case kNavUserActionSaveChanges:
					{
						// Do the save, which may or may not trigger the save dialog
						status = DoCommand( pData->theWindow, cSave, 0, 0 );
					}
					break;
					
					case kNavUserActionDontSaveChanges:
					{
						// OK to throw away this document
						status = DoCloseWindow( pData->theWindow, true, 0 );
					}
					break;
				}
			}
		}
		else if ( dirtyCount > 1 )
		{
			status = ModalConfirmSaveDialog( NULL, true, &userAction );
			if ( status != noErr || userAction == kNavUserActionCancel )
			{
				gMachineInfo.isQuitting = false;
			}
		}
	}

	// Still quitting after (possibly) running the modal confirm dialog?
	if ( !gMachineInfo.isQuitting ) return eUserCanceled;

	// Close the clean documents, others will be confirmed one by one.
	CloseAllWindows( false );		

	return noErr;
}


// --------------------------------------------------------------------------------------------------------------
OSStatus DoCommand(WindowPtr pWindow, short commandID, long menuResult, long keyTime)
{
	OSStatus		anErr = noErr;
	WindowDataPtr 	pData = nil;
	
	if (pWindow)
	{
		pData = (WindowDataPtr) GetWindowInfo(pWindow);
		
		if ( (pData) && (pData->pCommand) )
			anErr = (*(pData->pCommand)) (pWindow, pData, commandID, menuResult);
	}
	
	if (anErr == noErr)
	{
		// default command handling
		switch (commandID)
		{
#if !SIMPLER_TEXT
	    // About box command
			case cAbout:
				if (!BringToFrontIfExists(kAboutWindow))
					anErr = MakeNewWindow(kAboutWindow, nil, '\?\?\?\?', nil);
				break;
#endif
#if(0)				
			case cDeskAccessory:
				{
				Str255	tempString;
				
				GetMenuItemText(GetMenuHandle(menuResult>>16), menuResult & 0xFFFF, tempString);
				OpenDeskAcc(tempString);
				}
				break;
#endif				
			// New window command
			case cNew:
				anErr = MakeNewWindow(kTextWindow, nil, 'TEXT', nil);
				break;
				
			// Open window command
			case cOpen:
				anErr = DoOpenWindow();
				break;
				
			// Close window command
			case cClose:
				anErr = DoCloseWindow(pWindow, false, keyTime);
				break;

#if !SIMPLER_TEXT
			case cPageSetup:
				anErr = DoPageSetup(pWindow);
				break;

			case cPrint:
				anErr = DoPrint(pWindow, false, true);
				break;
				
			case cPrintOneCopy:
				anErr = DoPrint(pWindow, true, false);
				break;
#endif				
			// get out of here command!
			case cQuit:
				anErr = DoQuit();
				break;
	
			// show/hide clipboard
			case cShowClipboard:
				if (!BringToFrontIfExists(kClipboardWindow))
				{
					anErr = MakeNewWindow(kClipboardWindow, nil, '\?\?\?\?', nil);
				}
				else
				{
					pWindow = FrontNonFloatingWindow();
					anErr = DoCloseWindow(pWindow, false, keyTime);
				}
				break;
				
			case cNextPage:
				gEvent.what = keyDown;
				gEvent.message = kPageDown << 8;
				gEvent.modifiers = 0;
				DoKeyEvent(pWindow, &gEvent, false);
				break;
				
			case cPreviousPage:
				gEvent.what = keyDown;
				gEvent.message = kPageUp << 8;
				gEvent.modifiers = 0;
				DoKeyEvent(pWindow, &gEvent, false);
				break;
				
			// Do nothing command
			case cNull:
				break;
							
			default:
				break;
		}
	}
		
	// don't report cancels
	if (anErr == kPMCancel)
		anErr = noErr;
	
	if ( (anErr != noErr) && (anErr != eActionAlreadyHandled) && (anErr != eUserCanceled) )
	{
		// some commands are so similar to other commands that we map their IDs
		// for the purposes of the error strings
		if (commandID == cSaveAs)
			commandID = cSave;
		if (commandID == cPrintOneCopy)
			commandID = cPrint;
			
		ConductErrorDialog(anErr, commandID, cancel);
	}
		
	// in any case, unhilite the menu selected after command processing is done,
	// but first delay for a short while if the command was invoked via a cmd key
	// so that the menu title is hilighted long enough for the user to see it
	UnhiliteMenuDelayed(keyTime);
	
	return anErr;
	
} // DoCommand

// --------------------------------------------------------------------------------------------------------------
static OSStatus	DoMenuCommand(WindowPtr pWindow, long menuResult, long keyTime)
{
	OSStatus	anErr = noErr;
	short	commandID = cNull;
	short	** commandHandle;
	short	menuID = menuResult >> 16;

	if (menuID >= mFontSubMenusStart)
	{
		commandID = cSelectFontStyle;
	}
	else
	{
		// read in the resource that controls this menu
		{
			short	oldResFile = CurResFile();
			
			UseResFile(gApplicationResFile);
			commandHandle = (short**) Get1Resource('MCMD', menuID);
			UseResFile(oldResFile);
			anErr = ResError();
			nrequire(anErr, FailedToLoadCommandTable);
		}
		
		if (commandHandle)
		{
			short	item = menuResult & 0xFFFF;
			short	* pCommands = *commandHandle;
			
			if (item <= pCommands[0])
				commandID = pCommands[item];
			else
				commandID = pCommands[pCommands[0]];
		}
	}
	
	anErr = DoCommand(pWindow, commandID, menuResult, keyTime);
	
// FALL THROUGH EXCEPTION HANDLING
FailedToLoadCommandTable:

	return anErr;
	
} // DoMenuCommand


// --------------------------------------------------------------------------------------------------------------
static void DoKeyPageDown(WindowPtr pWindow, WindowDataPtr pData, Boolean processPageControls)
{

	if (GetControlValue(pData->vScroll) < GetControlMaximum(pData->vScroll))
		VActionProc(pData->vScroll, kControlPageDownPart);
	else
	{
		if ( (processPageControls) && (IsCommandEnabled(cNextPage)) )
		{
			short amount;

			if (DoCommand(pWindow, cNextPage, 0, 0) == eActionAlreadyHandled)
			{
				amount = GetControlValue(pData->vScroll);
				SetControlAndClipAmount(pData->vScroll, &amount);
				if (amount != 0)
					DoScrollContent(pWindow, pData, 0, amount);
			}
			
			AdjustMenus(pWindow, true, false);
		}
	}
	
} // DoKeyPageDown

// --------------------------------------------------------------------------------------------------------------
static void DoKeyPageUp(WindowPtr pWindow, WindowDataPtr pData, Boolean processPageControls)
{
	if (GetControlValue(pData->vScroll) > GetControlMinimum(pData->vScroll))
		VActionProc(pData->vScroll, kControlPageUpPart);
	else
	{
		if ( (processPageControls) && (IsCommandEnabled(cPreviousPage)) )
		{
			short amount;
			
			if (DoCommand(pWindow, cPreviousPage, 0, 0) == eActionAlreadyHandled)
			{
				amount = -(GetControlMaximum(pData->vScroll)-GetControlValue(pData->vScroll));
				SetControlAndClipAmount(pData->vScroll, &amount);
				if (amount != 0)
					DoScrollContent(pWindow, pData, 0, amount);
			}
			
			AdjustMenus(pWindow, true, false);
		}
	}
		
} // DoKeyPageUp

// --------------------------------------------------------------------------------------------------------------
OSStatus	DoKeyEvent(WindowPtr pWindow, EventRecord * pEvent, Boolean processPageControls)
{
	OSStatus			anErr = noErr;
	WindowDataPtr 	pData = nil;
	Boolean			passToObject = false;
	Boolean 		isMotionKey = false;
	long			menuResult = 0;
	
	char keyCode = (pEvent->message >> 8) & charCodeMask;

	if (pWindow)
		pData = GetWindowInfo(pWindow);
	
	// check for command key
	if ( (pData) && (pData->pPreMenuAccess) )
		(void) (*(pData->pPreMenuAccess)) (pWindow, pData);
	AdjustMenus(pWindow, true, false);
	menuResult = MenuEvent(pEvent);
	DoMenuCommand(pWindow, menuResult, TickCount());
	pWindow = FrontNonFloatingWindow();

	if (menuResult == 0)
	{
		if (pWindow)
		{
			if ( (pData) && (pData->pKeyEvent) )
				passToObject = true;
			SetPortWindowPort (pWindow);
		}
			
		if (pData)
		{
			switch (keyCode)
			{
				case kHome: // top of file
					isMotionKey = true;
					if (pData->vScroll)
					{
						short amount;
						
						if ( (processPageControls) && (IsCommandEnabled(cGotoPage)) )
							DoCommand(pWindow, cGotoPage, cGotoFirst, 0);

						amount = GetControlValue(pData->vScroll);
						SetControlAndClipAmount(pData->vScroll, &amount);
						if (amount != 0)
							DoScrollContent(pWindow, pData, 0, amount);
						passToObject = false;
					}
					break;
					
				case kEnd: // end of file
					isMotionKey = true;
					if (pData->vScroll)
					{
						short amount;

						if ( (processPageControls) && (IsCommandEnabled(cGotoPage)) )
							DoCommand(pWindow, cGotoPage, cGotoLast, 0);
							
						amount = -(GetControlMaximum(pData->vScroll)-GetControlValue(pData->vScroll));
						SetControlAndClipAmount(pData->vScroll, &amount);
						if (amount != 0)
							DoScrollContent(pWindow, pData, 0, amount);
						passToObject = false;
					}
					break;
					
				case kPageUp: // scroll bar page up
					isMotionKey = true;
					if (pData->vScroll)
					{
						DoKeyPageUp(pWindow, pData, processPageControls);
						passToObject = false;
					}
					break;
					
				case kPageDown: // scroll bar page down
					isMotionKey = true;
					if (pData->vScroll)
					{
						DoKeyPageDown(pWindow, pData, processPageControls);
						passToObject = false;
					}
					break;
							
				case kUpArrow:		// scroll bar up arrow
					isMotionKey = true;
					if ( (pData->vScroll) && (!pData->pKeyEvent) )
					{
						if ( pEvent->modifiers & cmdKey )			/* Command key down */
							DoKeyPageUp(pWindow, pData, processPageControls);
						else
							VActionProc(pData->vScroll, kControlUpButtonPart);
						passToObject = false;
					}
					break;
					
				case kDownArrow:	// scroll bar down arrow
					isMotionKey = true;
					if ( (pData->vScroll) && (!pData->pKeyEvent) )
					{
						if ( pEvent->modifiers & cmdKey )			/* Command key down */
							DoKeyPageDown(pWindow, pData, processPageControls);
						else
							VActionProc(pData->vScroll, kControlDownButtonPart);
						passToObject = false;
					}
					break;
		
				case kLeftArrow:	// scroll bar left arrow
					isMotionKey = true;
					if ( (pData->hScroll) && (!pData->pKeyEvent) )
					{
						if ( pEvent->modifiers & cmdKey )			/* Command key down */
							HActionProc(pData->hScroll, kControlPageUpPart);
						else
							HActionProc(pData->hScroll, kControlUpButtonPart);
						passToObject = false;
					}
					break;
					
				case kRightArrow:	// scroll bar right arrow
					isMotionKey = true;
					if ( (pData->hScroll) && (!pData->pKeyEvent) )
					{
						if ( pEvent->modifiers & cmdKey )			/* Command key down */
							HActionProc(pData->hScroll, kControlPageDownPart);
						else
							HActionProc(pData->hScroll, kControlDownButtonPart);
						passToObject = false;
					}
					break;
				}
		
			if (passToObject)
				anErr = (*(pData->pKeyEvent)) (pWindow, pData, pEvent, isMotionKey);
			else
			{
				if ( (pData->documentAcceptsText == false) && !( pEvent->modifiers & cmdKey ) && !(isMotionKey) )
					anErr = eDocumentNotModifiable;
			}
		}

		if ( (anErr != noErr) && (anErr != eActionAlreadyHandled) )
			ConductErrorDialog(anErr, cTypingCommand, ok);
			
	} // (menuResult == 0)
	
		
	return anErr;
	
} // DoKeyEvent

// --------------------------------------------------------------------------------------------------------------
OSStatus DoAdjustCursor(WindowPtr pWindow, Point *where)
{
	OSStatus		anErr = noErr;
	Point		whereMouse;
	Boolean		didAdjust = false;
		
	if (pWindow)
	{
		// not one of our windows?  don't do anything
		if (GetWindowKind(pWindow) != userKind)
			didAdjust = true;
			
		SetPortWindowPort (pWindow);
		
        // Put the local mouse coords in whereMouse
        if ( where )
        {
            whereMouse = *where;
            GlobalToLocal( &whereMouse );
        }
        else
        {
            GetMouse( &whereMouse );
        }
        
		if ( (!didAdjust) && (gMachineInfo.haveTSM) )
		{
#if !TARGET_CARBON // SetTSMCursor not needed (nor available) in Carbon (MG 6/9/99)
			if (SetTSMCursor(whereMouse))
				didAdjust = true;
#endif
		}
			
		if (!didAdjust)
		{
			WindowDataPtr	pData = GetWindowInfo(pWindow);
			RgnHandle		content = NewRgn();
			Point			globalMouse;
			
			SetEmptyRgn(gCursorRgn);
	
			globalMouse = whereMouse;
			LocalToGlobal(&globalMouse);
			
			GetWindowRegion(pWindow, kWindowContentRgn, content);
			if ((pData) && (PtInRgn(globalMouse, content)) && (PtInRect(whereMouse, &pData->contentRect)))
			{
				Rect			tempRect;
				
				tempRect = pData->contentRect;
				LocalToGlobal(&TopLeft(tempRect));
				LocalToGlobal(&BotRight(tempRect));
				RectRgn(gCursorRgn, &tempRect);
				
				if (pData->pAdjustCursor)
					anErr = (*(pData->pAdjustCursor)) (pWindow, pData, &whereMouse, gCursorRgn);
			}
			else
			{
				// The cursor is outside of region, so WaitNextEvent() will be returning immediately with moiseMoved event.
				// Since we don't want to call DoAdjustCursor() if mouse did not actually move (we don't want spending
				// processor time doing nothing) - setup small region (just one point) for current mouse position,
				// so when mouse moved out, we will readjust cursor and recreate the region for
				SetRectRgn(gCursorRgn, globalMouse.h - 1, globalMouse.v - 1, globalMouse.h + 1, globalMouse.v + 1);
			}
			DisposeRgn(content);
		}
		else
			anErr = eActionAlreadyHandled;
	}
	
	// nobody set the cursor, we do it ourselves
	if (anErr != eActionAlreadyHandled)
	{
		Cursor arrow;
		SetCursor(GetQDGlobalsArrow(&arrow));
	}
		
	return anErr;
	
} // DoAdjustCursor
 
// --------------------------------------------------------------------------------------------------------------
static long DetermineWaitTime(WindowPtr pWindow)
{
	long	waitTime = kMaxWaitTime;
	
	while (pWindow)
	{
		long			newWaitTime;
		WindowDataPtr	pData = GetWindowInfo(pWindow);
		
		if ((pData) && (pData->pCalculateIdleTime))
			newWaitTime = (*(pData->pCalculateIdleTime)) (pWindow, pData);
		else
			newWaitTime = kMaxWaitTime;
		
		if (newWaitTime < waitTime)
			waitTime = newWaitTime;
			
		pWindow = GetNextWindow(pWindow);
	}
	
	return(waitTime);
	
} // DetermineWaitTime


void HandleNavUserAction( NavDialogRef inNavDialog, NavUserAction inUserAction, void *inContextData )
{
	OSStatus	status = noErr;

	// We only have to handle the user action if the context data is non-NULL, which
	// means it is an action that applies to a specific document.
	if ( inContextData != NULL )
	{
		// The context data is a window data pointer
		WindowDataPtr	pData = (WindowDataPtr)inContextData;
		Boolean			discard = false;

		// The dialog is going away, so clear the window's reference
		pData->navDialog = NULL;

		switch( inUserAction )
		{
			case kNavUserActionCancel:
			{
				// If we were closing, we're not now.
				pData->isClosing = false;
				// If we were closing all, we're not now!
				gMachineInfo.isClosing = false;
				// If we were quitting, we're not now!!
				gMachineInfo.isQuitting = false;
			}
			break;
			
			case kNavUserActionSaveChanges:
			{
				// Do the save, which may or may not trigger the save dialog
				status = DoCommand( pData->theWindow, cSave, 0, 0 );
			}
			break;
			
			case kNavUserActionDontSaveChanges:
			{
				// OK to throw away this document
				discard = true;
			}
			break;
			
			case kNavUserActionSaveAs:
			{
				if ( pData->pSaveTo )
				{
					OSStatus		completeStatus;
					NavReplyRecord		reply;
					FSRef			theRef;
					status = BeginSave( inNavDialog, &reply, &theRef );
					nrequire( status, BailSaveAs );

					status = (*(pData->pSaveTo))( pData->theWindow, pData, &theRef, reply.isStationery );
					completeStatus = CompleteSave( &reply, &theRef, status == noErr );
					if ( status == noErr )
					{
						status = completeStatus;	// So it gets reported to user.
					}
					nrequire( status, BailSaveAs );

					// Leave both forks open
					if ( pData->dataRefNum == -1 )
					{
						status = FSOpenFork( &theRef, 0, NULL, fsRdWrPerm, &pData->dataRefNum );
						nrequire( status, BailSaveAs );
					}

					if ( pData->resRefNum == -1 )
					{
						pData->resRefNum = FSOpenResFile( &theRef, fsRdWrPerm );
						status = ResError();
						nrequire( status, BailSaveAs );
					}

				}
			}
		BailSaveAs:
			break;
		}
		
		// Now, close the window if it isClosing and
		// everything got clean or was discarded
		if ( status != noErr )
		{
			// Cancel all in-progress actions and alert user.
			pData->isClosing = false;
			gMachineInfo.isClosing = false;
			gMachineInfo.isQuitting = false;
			ConductErrorDialog( status, cSave, cancel );
		}
		else if ( pData->isClosing && ( discard || CanCloseWindow( pData->theWindow )))
		{
			status = DoCloseWindow( pData->theWindow, discard, 0 );

			// If we are closing all then start the close
			// process on the next window
			if ( status == noErr && gMachineInfo.isClosing )
			{
				WindowPtr nextWindow = FrontNonFloatingWindow();
				if ( nextWindow != NULL )
				{
					DoCloseWindow( nextWindow, false, 0 );
				}
			}
		}
	}
}


void HandleEvent(EventRecord * pEvent)
{
    WindowPtr	pWindow;
    Boolean		handled;
	
    pWindow	= FrontNonFloatingWindow();
    handled	= false;
    
	switch (pEvent->what)
	{
		case kHighLevelEvent:
			AEProcessAppleEvent(pEvent);
			break;
			
		case osEvt:
			switch ((pEvent->message >> 24) & 0xFF) /* high byte of message */
			{		
				case mouseMovedMessage:
					DoAdjustCursor(pWindow, nil);
					break;
					
				case suspendResumeMessage:		/* suspend/resume is also an activate/deactivate */
					// we don't do anything special then activating
					break;
			}
			break;
			
		case activateEvt:
			pWindow = (WindowPtr) pEvent->message;
			DoActivate(pWindow, (pEvent->modifiers & activeFlag) != 0);
			break;
							
		// disk inserted events must be handled, or uninitialized floppies 
		// won't be recognized.
		case diskEvt:
			if ( HiWord(pEvent->message) != noErr ) 
			{
				Point	where;
			
				SetPt(&where, 70, 50);
				ShowCursor();
			//	(void) DIBadMount(where, pEvent->message);
			}		
			break;
				
		case mouseUp:
			break;
			
		case mouseDown:
			{
				short part = FindWindow(pEvent->where, &pWindow);					
			
				DoAdjustCursor(pWindow, &pEvent->where);
				switch ( part ) 
				{
				case inContent:
					if (pWindow != FrontNonFloatingWindow())
						SelectWindow(pWindow);
					else
						DoContentClick(pWindow);
					break;
					
				case inGoAway:
					if (TrackGoAway(pWindow, pEvent->where) )
					{
						//
						// added option-close, because it’s in the HIG
						//
						if( pEvent->modifiers & optionKey )
						{
							// Close the clean documents, others will be confirmed one by one.
							CloseAllWindows( false );							
						}
						else
						{
							DoCommand(pWindow, cClose, 0, 0);
						}
					}
					break;
					
				case inGrow:
					DoGrowWindow(pWindow, pEvent);
					break;
					
				case inZoomIn:
				case inZoomOut:
					if ( TrackBox(pWindow, pEvent->where, part) )
						DoZoomWindow(pWindow, part);
					break;
					
				case inProxyIcon:
		//
		// We’ve seen a hit in the window proxy, so drag the window proxy
		//
		// We should only be here on a machine with the native window manager
		// but we still want to conditionalize it to be fully paraniod
		//				
//				check(gMachineInfo.haveProxyIcons);
				{
					WindowDataPtr	pData = GetWindowInfo(pWindow);
					
					if( gMachineInfo.haveProxyIcons && (pData != NULL) )
					{
						OSStatus		status = TrackWindowProxyDrag( pWindow, pEvent->where );
			
						if( status == errUserWantsToDragWindow )
							handled = false;
						else
							handled = true;
					}
				}
				// fall through
				case inDrag:
					if( !handled )
					{
						WindowDataPtr	pData = GetWindowInfo(pWindow);
					
						//
						// Show the file path select popup
						//
						if( gMachineInfo.haveProxyIcons && (pData != NULL) )
						{
							if( IsWindowPathSelectClick( pWindow, pEvent ) )
							{
								SInt32 itemSelected;
							
								// regardless of what WindowPathSelect returns, we don't want to drag the window at this point
								handled = true;
								
								if (WindowPathSelect( pWindow, NULL, &itemSelected ) == noErr )
								{
									// Prior to Carbon, it was necessary to switch to the Finder
									// after calling WindowPathSelect; in Carbon, WindowPathSelect
									// does it for you automatically.
								}
							}
						}
						
						if( !handled )
						{
							BitMap	screenBits;
			    
							GetQDGlobalsScreenBits( &screenBits );
							if ( (pData) && (pData->dragWindowAligned) )
								DragAlignedWindow((WindowPtr) pWindow, pEvent->where, &screenBits.bounds, nil, nil);
							else
								DragWindow(pWindow, pEvent->where, &screenBits.bounds);
						}
					
					}
					break;
				
				case inMenuBar:				/* process a mouse menu command (if any) */
					{
						long			menuResult;
						WindowDataPtr	pData;
						
						// force these threads to run to completion so the
						// contents of the menus are fully initialized
						
						if (gFontThread != kNoThreadID)
						{
							gDontYield = true;
							SetThreadState(gFontThread, kReadyThreadState, gFontThread);
							YieldToThread(gFontThread);
							gDontYield = false;
						}
						
						pWindow = FrontNonFloatingWindow();
						pData = GetWindowInfo(pWindow);
						if ((pData) && (pData->pPreMenuAccess))
							(void) (*(pData->pPreMenuAccess)) (pWindow, pData);
						AdjustMenus(pWindow, true, false);
						InitCursor();
						menuResult = MenuSelect(pEvent->where);
						DoMenuCommand(pWindow, menuResult, 0);
					}
					break;
					
				} // switch(part)
			}
			break;
			
		case keyDown:
		case autoKey:						/* check for menukey equivalents */
			DoKeyEvent(pWindow, pEvent, true);
			break;
			
		case updateEvt:
			pWindow = (WindowPtr) pEvent->message;
			(void) DoUpdateWindow(pWindow);
			break;

	} // switch (pEvent->what)
	
} // HandleEvent

// -----------------------------------------------------------------------------------------------------------
static void SynchronizeFiles( void )
{
	//
	// File synchronization for all document windows
	//
	static UInt32			nextSynchTicks = 10;
	UInt32				currentTicks = TickCount();
	WindowPtr			currentWindow = FrontNonFloatingWindow();
    
	// only synchronize every so often...
	if( currentTicks > nextSynchTicks )
	{
		//
		// Loop over all our document windows,
		// searching for files whose locations have changed
		//
		while ( currentWindow != NULL )
		{
			WindowDataPtr	documentWindowData = GetWindowInfo(currentWindow);
			
			//
			// If it's a SimpleText-owned window and it has an associated file...
			//
			if( (documentWindowData != NULL)
       			&& (documentWindowData->dataRefNum != -1) )
			{
				Boolean		wasChanged = false;
				FSRef		newRef;
				FolderType	folder = 0;
				
				//
				// Ask the Alias Manager where the window went
				//
				(void) FSResolveAlias( NULL, documentWindowData->fileAlias, &newRef, &wasChanged );
				if( wasChanged )
				{
					CFStringRef	pCFFileName = NULL;
					HFSUniStr255	theFileName;
					FSCatalogInfo	theCatInfo;
					
					(void) FSGetCatalogInfo( &newRef, kFSCatInfoVolume+kFSCatInfoParentDirID, &theCatInfo, &theFileName, NULL, NULL );
                
                    pCFFileName = CFStringCreateWithCharacters( NULL, theFileName.unicode, theFileName.length );
					
					//
					// The file location has changed; update the window
					//
					documentWindowData->fileRef = newRef;
					
					// user might have renamed the file 
					SetWindowTitleWithCFString( currentWindow, pCFFileName );
					
					//
					// Close the window if the user moved the Is it in the trash?
					//
					IdentifyFolder( theCatInfo.volume, theCatInfo.parentDirID, &folder );
					
					if ( folder == kTrashFolderType )
					{
						DoCloseWindow( currentWindow, false, 0 );
					}
					
					CFRelease(pCFFileName);
				}
			}
				
			currentWindow = GetNextWindow( currentWindow );
		}
		
		//
		// To avoid flooding the CPU, wait at least one second
		// between file synch checks 
		//
		nextSynchTicks = ( currentTicks + 60 );
	}
}


static OSStatus	DoEventLoop(void)
{
	OSStatus	anErr = noErr;
	Boolean		gotEvent;
	Boolean		trueGotEvent;
	WindowPtr	pWindow;
	
	do
	{
		pWindow = GetWindowList();	// walk all of our windows, even invisible ones
		
		/*
			We used to call DoAdjustCursor every time through the event loop, but that produced
			nasty cursor flickering when using the magic marker cursor over GX documents. We're
			now smarter and allow the adjustCursor callback to manipulate the cursor region.
			However, there may still be cases where the cursor doesn't get adjusted properly.
			If you suspect a cursor adjustment problem, try putting this call to DoAdjustCursor
			back in and see what the cursor does. Then fix the code to accurately maintain the
			cursor region and remove the DoAdjustCursor call. -ecs 11/25/96
		*/
		// DoAdjustCursor(pWindow, nil);
		gotEvent = WaitNextEvent(everyEvent, &gEvent, DetermineWaitTime(pWindow), gCursorRgn);
		trueGotEvent = gotEvent;

		//
		// Synchronize all files on every event
		//
		SynchronizeFiles();

		// WNE may close the window if it's owned by some silly extension.
		pWindow = GetWindowList();		
		
		// let text services handle the event first if it wishes to do so
		if ( gMachineInfo.haveTSM )
		{
			ScriptCode	keyboardScript;
			WindowPtr	theFront = FrontNonFloatingWindow();
			
			if (theFront)
			{
				CGrafPtr fPort = GetWindowPort(theFront);
				SetPort(fPort);
				
				keyboardScript = GetScriptManagerVariable(smKeyScript);
				if (FontToScript(GetPortTextFont(fPort)) != keyboardScript)
					TextFont(GetScriptVariable(keyboardScript, smScriptAppFond));
			}
		}
			
		// let all windows filter this event, and get time if they wish to
		while (pWindow)
		{
			WindowDataPtr	pData = GetWindowInfo(pWindow);
			Boolean			finishedEvent = false;
								
			// if we hit a window we know about, then do filtering
			if (pData)
			{
				if (pData->pFilterEvent)
					finishedEvent = (*(pData->pFilterEvent)) (pWindow, pData, &gEvent);
			}

			// if filtering indicates complete handling of event, then stop, and
			// do no regular processing.
			if (finishedEvent)
			{
				gotEvent = false;
				pWindow = nil;
			}
			else
				pWindow = GetNextWindow(pWindow);
		}
			
		if (gotEvent)
			HandleEvent(&gEvent);
			
		if ( gMachineInfo.isQuitting && FrontNonFloatingWindow() == NULL )
		{
			gAllDone = true;
		}

		// our threads are low-priority, so we only give time to them on idle
		if (gMachineInfo.haveThreads && !trueGotEvent && !gAllDone)
			YieldToAnyThread();
		
	} while (!gAllDone);
		
	return anErr;
	
} // DoEventLoop


//
// Close all windows
//
// Returns false to cancel
//
static Boolean CloseAllWindows( Boolean discard )
{
	WindowPtr	pWindow, nextWindow;
	WindowPtr	topClosingWindow = NULL;

	OSStatus	closeError = noErr;
	
	// Start the closing process by closing clean windows
	gMachineInfo.isClosing = true;

	// Close the open file dialog, if there is one.
	TerminateOpenFileDialog();

	// Close the clean windows, find the top one
	// that is already closing.
	pWindow = FrontNonFloatingWindow();
	while ( (pWindow != NULL) && closeError == noErr )
	{
		nextWindow = GetNextWindow(pWindow);
		if ( discard || CanCloseWindow( pWindow ))
		{
			closeError = DoCloseWindow( pWindow, discard, 0 );
		}
		else if ( topClosingWindow == NULL )
		{
			WindowDataPtr	pData = GetWindowInfo( pWindow );
			if ( pData && pData->isClosing )
			{
				topClosingWindow = pWindow;
			}
		}
		pWindow = nextWindow;
	}
	
	// If there is a window that is already closing, bring it front.
	// Otherwise, start the close process on the front window.
	if ( topClosingWindow != NULL )
	{
		SelectWindow( topClosingWindow );
	}
	else if ( FrontNonFloatingWindow() != NULL )
	{
		DoCloseWindow( FrontNonFloatingWindow(), false, 0 );
	}

	return FrontNonFloatingWindow() == NULL;
}

// --------------------------------------------------------------------------------------------------------------
// DRAG MANAGEMENT GLOBAL SUPPORT ROUTINES
// --------------------------------------------------------------------------------------------------------------

// Globals for our drag handlers

Boolean				gCanAccept;				// if we can receive the item(s) being dragged

// --------------------------------------------------------------------------------------------------------------
static pascal OSErr GlobalTrackingHandler(short message, WindowPtr pWindow, void *handlerRefCon, DragReference theDragRef)
{
	#pragma unused(handlerRefCon)

	WindowDataPtr pData = GetWindowInfo(pWindow);

	// Call the tracking handler associated with this type of window. Only allow messages referencing
	// a specific window to be passed to the handler.

	if (pData)
	{	
		if (pData->pDragTracking)
			return ((*(pData->pDragTracking)) (pWindow, pData, theDragRef, message));
	}
	
	return noErr;

} // GlobalTrackingHandler

DragTrackingHandlerUPP gGlobalTrackingHandler;

// --------------------------------------------------------------------------------------------------------------
static pascal OSErr GlobalReceiveHandler(WindowPtr pWindow, void *handlerRefCon, DragReference theDragRef)
{
	#pragma unused(handlerRefCon)

	WindowDataPtr pData = GetWindowInfo(pWindow);
	
	if (pData)
	{
		if (pData->pDragTracking)
			return ((*(pData->pDragReceive)) (pWindow, pData, theDragRef));
	}

	return noErr;

} // GlobalReceiveHandler

DragReceiveHandlerUPP gGlobalReceiveHandler;

// --------------------------------------------------------------------------------------------------------------
//
// IsOnlyThisFlavor - Given a DragReference and a FlavorType, we iterate through the drag items to determine if
//					  all are of flavor theType. If this is so, we return true. If any of the items are not
//					  theType, we return false, indicating that we should not accept the drag.
//
Boolean IsOnlyThisFlavor(DragReference theDragRef, FlavorType theType)
{
	unsigned short	items, index;
	FlavorFlags		theFlags;
	ItemReference	itemID;
	OSStatus			anErr = noErr;

	CountDragItems(theDragRef, &items);
	
	for(index = 1; index <= items; index++)
	{
		GetDragItemReferenceNumber(theDragRef, index, &itemID);

		anErr = GetFlavorFlags(theDragRef, itemID, theType, &theFlags);
	if(anErr == noErr)
			continue;	// it's okay, this flavor is cool

		return false;	// this item has at least one flavor we don't like
	}

	return true;		// all flavors in this item were cool

} // IsOnlyThisFlavor

// --------------------------------------------------------------------------------------------------------------
//
// IsDropInFinderTrash - Returns true if the given dropLocation AEDesc is a descriptor of the Finder's Trash.
//
Boolean IsDropInFinderTrash(AEDesc *dropLocation)
{
	OSStatus			result;
	AEDesc			dropSpec;
	FSSpec			theSpec;
	CInfoPBRec		thePB;
	short			trashVRefNum;
	long			trashDirID;

	//	Coerce the dropLocation descriptor into an FSSpec. If there's no dropLocation or
	//	it can't be coerced into an FSSpec, then it couldn't have been the Trash.

	if ((dropLocation->descriptorType != typeNull) &&
		(AECoerceDesc(dropLocation, typeFSS, &dropSpec) == noErr)) 
	{
		AEGetDescData(&dropSpec, &theSpec, sizeof(FSSpec) );

		//	Get the directory ID of the given dropLocation object.

		thePB.dirInfo.ioCompletion = 0L;
		thePB.dirInfo.ioNamePtr = (StringPtr) theSpec.name;
		thePB.dirInfo.ioVRefNum = theSpec.vRefNum;
		thePB.dirInfo.ioFDirIndex = 0;
		thePB.dirInfo.ioDrDirID = theSpec.parID;

		result = PBGetCatInfoSync(&thePB);

		AEDisposeDesc(&dropSpec);

		if (result != noErr)
			return false;

		//	If the result is not a directory, it must not be the Trash.

		if (!(thePB.dirInfo.ioFlAttrib & (1 << 4)))
			return false;

		//	Get information about the Trash folder.

		FindFolder(theSpec.vRefNum, kTrashFolderType, kCreateFolder, &trashVRefNum, &trashDirID);

		//	If the directory ID of the dropLocation object is the same as the directory ID
		//	returned by FindFolder, then the drop must have occurred into the Trash.

		if (thePB.dirInfo.ioDrDirID == trashDirID)
			return true;
	}

	return false;

} // IsDropInFinderTrash

// --------------------------------------------------------------------------------------------------------------
// APPLE EVENT SUPPORT ROUTINES
// --------------------------------------------------------------------------------------------------------------
static OSStatus	MissingParameterCheck(
	const AppleEvent 	*inputEvent)
/*
	This routine checks an input AppleEvent for the missing keyword.
	If the missing keyword is found, that means that some required
	parameters were missing (ie, an error). 
	
	However, if the missing keyword isn't found, that means that we aren't missing 
	any required parameters (that is to say, all REQUIRED parameters were supplied
	by the person who created the event).
	
	SOME DAY, THE ABOVE COMMENT WILL MAKE SENSE TO YOU.  IT STILL DOESN'T
	TO ME AND I WAS THE ONE WHO WROTE IT.
*/
{
	OSStatus		anErr;
	AEKeyword	missingKeyword;
	DescType	ignoredActualType;
	Size		ignoredActualSize;
	
	anErr = AEGetAttributePtr(
		inputEvent, 
		keyMissedKeywordAttr,
		typeWildCard,
		&ignoredActualType,
		(Ptr) &missingKeyword,
		sizeof(AEKeyword),
		&ignoredActualSize);
			
	if (anErr == noErr)
		anErr = errAEParamMissed;
	else
		if (anErr == errAEDescNotFound)
			anErr = noErr;
		
	return anErr;
	
} // MissingParameterCheck


// --------------------------------------------------------------------------------------------------------------
static pascal OSErr	DoOpenApp(
	const AppleEvent 	*inputEvent,
	AppleEvent 	*outputEvent,
	SInt32		handlerRefCon)
{
#pragma unused (outputEvent, handlerRefCon)

	DoCommand(nil, cNew, 0, 0);
	
	// so that the initial document opens more quickly, we don't start
	// the threads until we get an OpenApp or OpenDocument AppleEvent
	if (gStarterThread != kNoThreadID)
		SetThreadState(gStarterThread, kReadyThreadState, gStarterThread);
	
	return(MissingParameterCheck(inputEvent));
	
} // DoOpenApp

// --------------------------------------------------------------------------------------------------------------
static pascal OSErr	DoReopenApp(
	const AppleEvent 	*inputEvent,
	AppleEvent 	*outputEvent,
	SInt32		handlerRefCon)
{
#pragma unused (outputEvent, handlerRefCon)

	if (FrontNonFloatingWindow() == nil)
		DoCommand(nil, cNew, 0, 0);
	
	return(MissingParameterCheck(inputEvent));
	
} // DoReopenApp

// --------------------------------------------------------------------------------------------------------------
static pascal OSErr	DoQuitApp(
	const AppleEvent 	*inputEvent,
	AppleEvent 	*outputEvent,
	SInt32		handlerRefCon)
{
#pragma unused (outputEvent, handlerRefCon)
	OSStatus	anErr;

	anErr = DoCommand(nil, cQuit, 0, 0);
	if (anErr == eUserCanceled) return userCanceledErr;

	return(MissingParameterCheck(inputEvent));
	
} // DoQuitApp


// --------------------------------------------------------------------------------------------------------------
static pascal OSStatus	DoOpenOrPrint(
	const AppleEvent 	*inputEvent,
	StringPtr	pPrinterName)	// nil == 0, zero length == print to default, other == printer name
{

	OSStatus		anErr, anErr2;
	AEDescList	docList;				// list of docs passed in
	long		index, itemsInList;
	Boolean		wasAlreadyOpen;
	
	anErr = AEGetParamDesc( inputEvent, keyDirectObject, typeAEList, &docList);
	nrequire(anErr, GetFileList);

	anErr = AECountItems( &docList, &itemsInList);			// how many files passed in
	nrequire(anErr, CountDocs);
	for (index = 1; index <= itemsInList; index++)			// handle each file passed in
	{	
		AEKeyword	keywd;
		DescType	returnedType;
		Size		actualSize;
		FSRef 		fileRef;
		FSCatalogInfo	theCatInfo;
		
		anErr = AEGetNthPtr( &docList, index, typeFSRef, &keywd, &returnedType,
						(Ptr)(&fileRef), sizeof( fileRef ), &actualSize );
		nrequire(anErr, AEGetNthPtr);

		anErr = FSGetCatalogInfo( &fileRef, kFSCatInfoFinderInfo, &theCatInfo, NULL, NULL, NULL );
		nrequire(anErr, FSGetCatalogInfo);

		if (anErr == noErr)
			anErr = DetermineWindowTypeOrOpen(&fileRef, ((FInfo*)&theCatInfo.finderInfo)->fdType, nil, nil, &wasAlreadyOpen);
			
		if (anErr == eDocumentWrongKind)
		{
			if (pPrinterName)
				ConductErrorDialog(anErr, cPrint, cancel);
			else
				ConductErrorDialog(anErr, cOpen, cancel);

			anErr = noErr;
			break;
		}
			
		nrequire(anErr, DetermineWindowTypeOrOpen);
		
		if (pPrinterName)
		{
			WindowPtr		pWindow = FrontNonFloatingWindow();
			WindowDataPtr	pData = GetWindowInfo(pWindow);
			
			if (pData->pPrintPage)
			{
				if (index == 1)
				{
					anErr = DoPrint(pWindow, false, true);
				}
			}
			
			if (!wasAlreadyOpen)
				DoCloseWindow(pWindow, false, 0);

			if (anErr != noErr)
				break;
		}
	}

	// finally, make sure we didn't miss any parameters
	anErr2 = MissingParameterCheck(inputEvent);
	if (anErr == noErr)
		anErr = anErr2;
		
// FALL THROUGH EXCEPTION HANDLING
DetermineWindowTypeOrOpen:
AEGetNthPtr:
CountDocs:
FSGetCatalogInfo:
	// done with doc list
	(void) AEDisposeDesc( &docList);						
	
GetFileList:

	// don't report cancels from prints
	if (pPrinterName)
		{
		if (anErr == kPMCancel)
			anErr = noErr;
		}
	
	if ( (anErr != noErr) && (anErr != eActionAlreadyHandled) && (anErr != eUserCanceled) )
		{
		if (pPrinterName)
			ConductErrorDialog(anErr, cPrint, cancel);
		else
			ConductErrorDialog(anErr, cOpen, cancel);
		}
		
	return anErr;
	
} // DoOpenOrPrint

// --------------------------------------------------------------------------------------------------------------
static pascal OSErr	DoOpenDocument(
	const AppleEvent 	*inputEvent,
	AppleEvent 	*outputEvent,
	SInt32		handlerRefCon)
{
#pragma unused (outputEvent, handlerRefCon)

	OSStatus		anErr;
	
	if (IsCommandEnabled(cOpen))
	{
		anErr = DoOpenOrPrint(inputEvent, nil);
	}
	else
	{
		anErr = errAEEventNotHandled;
		ConductErrorDialog(anErr, cOpen, cancel);
	}
		
	// so that the initial document opens more quickly, we don't start
	// the threads until we get an OpenApp or OpenDocument AppleEvent
	if (gStarterThread != kNoThreadID)
		SetThreadState(gStarterThread, kReadyThreadState, gStarterThread);
	
	return anErr;
	
} // DoOpenDocument

// --------------------------------------------------------------------------------------------------------------
static pascal OSErr	DoPrintDocument(
	const AppleEvent 	*inputEvent,
	AppleEvent 	*outputEvent,
	SInt32		handlerRefCon)
{
#pragma unused (outputEvent, handlerRefCon)
	OSStatus		anErr;
	FSSpec		printerFSS;
	AEDescList	dtpList;				// list of docs passed in
	
	if (IsCommandEnabled(cOpen))
	{
		// try to find out if this doc was dropped onto a printer
		anErr = AEGetAttributeDesc( inputEvent, keyOptionalKeywordAttr, typeAEList, &dtpList);
	
		if (anErr == noErr)	// doc dragged to dtp?
		{
			AEKeyword	keywd;
			DescType	returnedType;
			Size		actualSize;
	
			anErr = AEGetNthPtr( &dtpList, 1, typeFSS, &keywd, &returnedType,	// get dtp info
							(Ptr)(&printerFSS), sizeof(printerFSS), &actualSize);
		}
			
		// if it wasn't, that's not an error, just print normally
		if (anErr != noErr)
		{
			printerFSS.name[0] = 0;
			anErr = noErr;
		}
			
		anErr = DoOpenOrPrint(inputEvent, &printerFSS.name[0]);
	}
	else
	{
		anErr = errAEEventNotHandled;
		ConductErrorDialog(anErr, cPrint, cancel);
	}
		
	return anErr;
	
} // DoPrintDocument


// --------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------
// MAIN INITIALIZE/SHUTDOWN/LOOP ROUTINES
// --------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------
// must be in Main because it runs in a thread and we don't want the segment unloaded
// while some other thread is running

static OSStatus BuildFontMenu(MenuHandle menu)
{
	OSStatus	anErr = noErr;
	
	AppendResMenu(menu, 'FONT');
	
	return(anErr);
	
} // BuildFontMenu

// --------------------------------------------------------------------------------------------------------------
static OSStatus	DoInitialize(void)
{
	Handle			menuBar;		// for loading our menus in
	OSStatus		anErr = noErr;		// any errors we get, none so far
	long			version;		// version for Gestalt calls
	MenuRef			menu;

	gMachineInfo.haveAppearanceMgr	= (Gestalt(gestaltAppearanceAttr, &version) == noErr) && ((version & (1<<gestaltAppearanceExists)) != 0);
	if (gMachineInfo.haveAppearanceMgr)
		RegisterAppearanceClient();
	
	gAllDone = false;
	
	gMachineInfo.lastBalloonIndex = iNoBalloon;
	gMachineInfo.amInBackground = false;
	gMachineInfo.isQuitting = false;
	gMachineInfo.isClosing = false;
	gMachineInfo.documentCount  = 1;
	gMachineInfo.haveQuickTime 	= (Gestalt(gestaltQuickTime, &version) == noErr);
	gMachineInfo.haveRecording 	= (Gestalt(gestaltSoundAttr, &version) == noErr) && ((version & (1<<gestaltHasSoundInputDevice)) != 0);
#if SPEECH_GESTALT_IMPLEMENTED
	gMachineInfo.haveTTS 		= (Gestalt(gestaltSpeechAttr, &version) == noErr) && ((version & (1<<gestaltSpeechMgrPresent)) != 0);
#else
	gMachineInfo.haveTTS 		= true; 
#endif
	gMachineInfo.haveTSM 		= (Gestalt(gestaltTSMgrVersion, &version) == noErr) && (version >= 1);
	gMachineInfo.haveTSMTE 		= (Gestalt(gestaltTSMTEAttr, &version) == noErr) && ((version & (1<<gestaltTSMTE)) != 0);
	gMachineInfo.haveDragMgr	= (Gestalt(gestaltDragMgrAttr, &version) == noErr) && ((version & (1<<gestaltDragMgrPresent)) != 0) &&
									(Gestalt(gestaltTEAttr, &version) == noErr) && ((version & (1<<gestaltTEHasGetHiliteRgn)) != 0);
	gMachineInfo.haveThreeD		= false;
	gMachineInfo.haveAppleGuide	= (Gestalt(gestaltHelpMgrAttr, &version) == noErr) && ((version & (1<<gestaltAppleGuidePresent)) != 0);
	gMachineInfo.haveThreads	= (Gestalt(gestaltThreadMgrAttr, &version) == noErr) && ((version & (1<<gestaltThreadMgrPresent)) != 0);
	
	gMachineInfo.haveNavigationServices  = NavServicesAvailable();

	//
	// Record the presence of >= 8.5 window manager features.
	//
	gMachineInfo.haveProxyIcons = false;
	gMachineInfo.haveFloatingWindows = false;
	
#if TARGET_CPU_PPC

	//
	if( ( Gestalt( gestaltWindowMgrAttr, &version ) == noErr ) )
	{
		if( version & (1L << gestaltWindowMgrPresentBit) )
		{
			gMachineInfo.haveProxyIcons = true;
		
			if( version & (1L << gestaltHasFloatingWindows) )
				gMachineInfo.haveFloatingWindows = true;
		}
	}
#endif


	// initialize text services if they exist
	if (gMachineInfo.haveTSMTE)
	{
#if !TARGET_CARBON // InitTSMAwareApplication not needed (nor available) in Carbon (MG 6/9/99)
		if (InitTSMAwareApplication() != noErr)
		{
			gMachineInfo.haveTSM = false;
			gMachineInfo.haveTSMTE = false;
		}
#endif
	}
		
	// save away info we need from the get-go	
	gApplicationResFile = CurResFile();
	gCursorRgn = NewRgn();

    // install the flipper for the menus
    anErr = CoreEndianInstallFlipper(kCoreEndianResourceManagerDomain, 'MCMD', &FlipMCMD, NULL);
    nrequire(anErr, CoreEndianInstallFlipper);

	// load up the menus
	menuBar = (Handle) GetNewMBar(rMenuBar);	/* read menus into menu bar */
	anErr = ResError();
	if ( (anErr == noErr) && (menuBar == nil) )
		anErr = resNotFound;
	nrequire(anErr, GetNewMBar);
	
	// install menus
	SetMenuBar(menuBar);	
	DisposeHandle(menuBar);

	// Build the font menu
	anErr = BuildFontMenu(GetMenuHandle(mFont));
	nrequire(anErr, BuildFontMenu);
	
	// Add the window menu
	anErr = CreateStandardWindowMenu( kWindowMenuIncludeRotate, &menu );
	nrequire( anErr, CreateWindowMenu );
	InsertMenu( menu, 0 );
	
	// insert our heirarchical menus
	{
	MenuHandle 	menu = MacGetMenu( mVoices );
	short		menuID, itemID;
	
	InsertMenu( menu, hierMenu );
	
	CommandToIDs(cSelectVoice, &menuID, &itemID);
	menu = GetMenuHandle(menuID);

	SetItemCmd( menu, itemID, hMenuCmd );
	SetItemMark( menu, itemID, mVoices );
	}

	(void) AdjustMenus(nil, true, false);
	
	// start up QuickTime, but problems result in us pretending not to have it
	if (gMachineInfo.haveQuickTime)
		if (EnterMovies() != noErr)
			gMachineInfo.haveQuickTime = false;
		
	// Install AppleEvent handlers for the base classes

	#define INSTALL(event, handler) \
			AEInstallEventHandler(kCoreEventClass, event, handler, 0, false)
	// AEC, changed to use the correct handler procs
	INSTALL (kAEOpenApplication, NewAEEventHandlerUPP(DoOpenApp));
	INSTALL (kAEReopenApplication, NewAEEventHandlerUPP(DoReopenApp));
	INSTALL (kAEQuitApplication, NewAEEventHandlerUPP(DoQuitApp));
	INSTALL (kAEOpenDocuments,   NewAEEventHandlerUPP(DoOpenDocument));
	INSTALL (kAEPrintDocuments,  NewAEEventHandlerUPP(DoPrintDocument));

	#undef INSTALL
	
	// AEC, added control procs
	gVActionProc = NewControlActionUPP(VActionProc);
	gHActionProc = NewControlActionUPP(HActionProc);


	// Install our global dragging procs, but only if we have Drag and Drop. An error results
	// in us pretending that we don't have drag support. Notice that in the test above, we also
	// require TextEdit to have TEGetHiliteRgn avalilable, which is always the case with the
	// present Drag Manager.

	if (gMachineInfo.haveDragMgr)
	{
		gGlobalTrackingHandler = NewDragTrackingHandlerUPP(GlobalTrackingHandler);
		gGlobalReceiveHandler = NewDragReceiveHandlerUPP(GlobalReceiveHandler);
		
		anErr = InstallTrackingHandler(gGlobalTrackingHandler, nil, nil);

		if (anErr == noErr)
		{
			anErr = InstallReceiveHandler(gGlobalReceiveHandler, nil, nil);

			if (anErr != noErr)
			{
				RemoveTrackingHandler(gGlobalTrackingHandler, nil);
				gMachineInfo.haveDragMgr = false;
			}
		}
		else
			gMachineInfo.haveDragMgr = false;
	}

	return noErr;
	
	
// EXCEPTION HANDLING
CreateWindowMenu:
BuildFontMenu:
GetNewMBar:
CoreEndianInstallFlipper:
// SysEnvirons:
	ConductErrorDialog(anErr, cNull, cancel);
	
	return anErr;

} // DoInitialize

// --------------------------------------------------------------------------------------------------------------
static OSStatus	DoTerminate(void)
{
	OSStatus	anErr = noErr;
	
	if (gFontThread != kNoThreadID)
		DisposeThread(gFontThread, &gThreadResults, false);
	if (gStarterThread != kNoThreadID)
		DisposeThread(gStarterThread, &gThreadResults, false);

	if (gMachineInfo.haveQuickTime)
		ExitMovies();
		
#if !TARGET_CARBON // CloseTSMAwareApplication not needed (nor available) in Carbon (MG 6/9/99)
	if (gMachineInfo.haveTSMTE)
		CloseTSMAwareApplication();
#endif

	if (gMachineInfo.haveDragMgr)
	{
		RemoveReceiveHandler(gGlobalReceiveHandler, nil);
		RemoveTrackingHandler(gGlobalTrackingHandler, nil);
	}

	if (gMachineInfo.haveAppearanceMgr)
		UnregisterAppearanceClient();
		
	// for AppleScript
	if (gOSAComponent)
		CloseComponent(gOSAComponent);

	return anErr;
	
} // DoTerminate

// --------------------------------------------------------------------------------------------------------------
//	• GetOSAComponent
//	Lazily opens a connection to AppleScript.
// --------------------------------------------------------------------------------------------------------------
ComponentInstance GetOSAComponent()
{
	if ( gOSAComponent == NULL )
		OpenADefaultComponent( kOSAComponentType, kOSAGenericScriptingComponentSubtype, &gOSAComponent );
	
	return gOSAComponent;
}

// --------------------------------------------------------------------------------------------------------------
int main (int argc, char **argv)
{
	OSStatus	anErr;
	
#if !defined( __MWERKS__ ) && !TARGET_CARBON
	UnloadSeg((Ptr) _DataInit);						/* note that _DataInit must not be in Main! */
#endif

	MoreMasterPointers (0x40 * 3); /* we love handles */

#ifdef __MWERKS__
#if __option(profile)
	ProfilerInit(collectSummary, bestTimeBase, 500, 10);
#endif

#endif
	InitCursor(); // ••• hack to load carbon so our Gestalt calls will work
	{
		SInt32	metricSize;
		GetThemeMetric( kThemeMetricScrollBarWidth, &metricSize );
		kScrollBarSize = metricSize - 1;
	}
	anErr = DoInitialize();
#ifdef __MWERKS__
#if __option(profile)
	ProfilerDump("\pboot.prof");
	ProfilerSetStatus(false);
	ProfilerClear();
#endif
#endif

	if (anErr == noErr)
		{
		DoEventLoop();

#ifdef __MWERKS__
#if __option(profile)
		ProfilerTerm();
#endif
#endif

// REVIEW: don't want to unload the segment we're in!!
//		UnloadSeg((Ptr) DoEventLoop);
		DoTerminate();					
		}

	return 0;
} // main

// --------------------------------------------------------------------------------------------------------------
//     ??FlipMCMD
//     Flips MCMD resource data.
// --------------------------------------------------------------------------------------------------------------
static OSStatus FlipMCMD(OSType dataDomain, OSType dataType, short id, void *dataPtr,
                         UInt32 dataSize, Boolean currentlyNative, void *refcon)
{
    short count;
    char *p = dataPtr;
    int i;

    if (currentlyNative) {
        count = *(short *) p;
    } else {
        count = EndianU16_BtoN(*(short *) p);
    }
    *(short *) p = Endian16_Swap(*(short *) p);
    p += sizeof(short);
    for(i = 0; i < count; i++) {
        *(short *) p = Endian16_Swap(*(short *) p);
        p += sizeof(short);
    }
    return noErr;
}
