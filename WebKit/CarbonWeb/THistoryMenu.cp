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

#include "THistoryMenu.h"
#include "TWebWindow.h"

static const EventTypeSpec kEvents[] = {
	{ kEventClassMenu, kEventMenuOpening },
	{ kEventClassCommand, kEventCommandProcess }
};

THistoryMenu::THistoryMenu( MenuRef inMenu )
{
	Init( inMenu );
}

THistoryMenu::~THistoryMenu()
{
}

void
THistoryMenu::Populate()
{
	CFArrayRef		days;
	CFIndex			i;
	MenuItemIndex	indexStart, indexEnd;
	CFGregorianDate	today;
	CFTimeZoneRef	timeZone = CFTimeZoneCopySystem();
	bool			addedItems = false;
	
	// Remove any current history items
	GetIndMenuItemWithCommandID( GetMenuRef(), 'PBEG', 1, NULL, &indexStart );
	GetIndMenuItemWithCommandID( GetMenuRef(), 'PEND', 1, NULL, &indexEnd );
	
	if ( indexStart != ( indexEnd - 1 ) )
		DeleteMenuItems( GetMenuRef(), indexStart + 1, indexEnd - indexStart - 1 );
	
	days = (CFArrayRef)[[WebHistory optionalSharedHistory] orderedLastVisitedDays];
	
	today = CFAbsoluteTimeGetGregorianDate( CFAbsoluteTimeGetCurrent(), timeZone );

        CFIndex dayCount = days ? CFArrayGetCount( days ) : 0;
	for ( i = 0; i < dayCount; i++ )
	{
		CFDateRef		date = (CFDateRef)CFArrayGetValueAtIndex( days, i );
		CFGregorianDate	gdate;
		CFStringRef		title;

		gdate = CFAbsoluteTimeGetGregorianDate( CFDateGetAbsoluteTime( date ), timeZone );

		if ( gdate.month == today.month && gdate.day == today.day && gdate.year == today.year )
		{
			indexStart = AddItemsToMenuForDate( GetMenuRef(), date, indexStart );
		}
		else
		{			
			MenuRef menu = CreateSubmenu();
						
			AddItemsToMenuForDate( menu, date, 0 );

			title = CFStringCreateWithFormat( NULL, NULL, CFSTR( "%d-%d-%d" ), gdate.month, gdate.day, gdate.year );
			InsertMenuItemTextWithCFString( GetMenuRef(), title, indexStart++, 0, 0 );
			SetMenuItemHierarchicalMenu( GetMenuRef(), indexStart, menu );

			ReleaseMenu( menu );
		}
		
		addedItems = true;
	}
	
	if ( addedItems )
		InsertMenuItemTextWithCFString( GetMenuRef(), CFSTR( "" ), indexStart, kMenuItemAttrSeparator, 0 );
	
	CFRelease( timeZone );
}

MenuItemIndex
THistoryMenu::AddItemsToMenuForDate( MenuRef inMenu, CFDateRef inDate, MenuItemIndex inAfterItem )
{
	CFArrayRef 		items = (CFArrayRef)[[WebHistory optionalSharedHistory] orderedItemsLastVisitedOnDay:(NSCalendarDate *)inDate];
	
	inAfterItem = AddItemsToMenu( inMenu, items, inAfterItem );
	
//	CFRelease( items );
	
	return inAfterItem;
}

void
THistoryMenu::GoToItem( WebHistoryItem* inItem )
{
	TWebWindow::GoToItem( inItem );
}
