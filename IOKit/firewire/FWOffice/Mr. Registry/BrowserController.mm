/*
	Created by: nwg
	
	Copyright: 	© Copyright 2002-2003 Apple Computer, Inc. All rights reserved.
	
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

	$Log: BrowserController.mm,v $
	Revision 1.4  2005/02/03 00:37:41  firewire
	*** empty log message ***
	
	Revision 1.3  2004/06/08 22:54:33  firewire
	*** empty log message ***
	
	Revision 1.2  2003/05/27 17:46:59  firewire
	SDK16
	
	Revision 1.1  2002/11/07 00:36:18  noggin
	move to new repository
	
	Revision 1.7  2002/08/15 18:34:44  noggin
	*** empty log message ***
	
	Revision 1.6  2002/08/06 22:23:51  noggin
	we were creating extra retains on services.. now we release them properly.
	
	Revision 1.5  2002/07/23 16:36:36  noggin
	first draft of graphing classes
	
	Revision 1.4  2002/07/18 22:04:18  noggin
	added spiffy outline tree lines
	
	Revision 1.3  2002/07/16 22:07:12  noggin
	now have NSData view and new icon
	
	Revision 1.2  2002/07/15 21:17:43  noggin
	*** empty log message ***
	
	Revision 1.1  2002/07/15 16:09:09  noggin
	moved files around. made properties inspector into a drawer.
	
	Revision 1.5  2002/05/30 18:35:49  noggin
	Fixed a few bugs. First FireWire bus view implemented.
	
	Revision 1.4  2002/05/23 19:51:05  noggin
	autorelease window delegate to avoid crash receiving hotplug notifications after window has been closed.
	
*/
#import "BrowserController.h"
#import "OutlineItem.h"
#import "OutlineViewDataSource.h"
#import "ApplicationController.h"
#import "InspectorController.h"

#import <IOKit/IOKitLib.h>

extern const NSString*	kRegistryChangedNotification ;
extern const NSString*	kServiceChangedNotification ;
extern const NSString* kRegistryOutlineColumnIdentifier ;

@implementation BrowserController

-(void)awakeFromNib
{
	// IOService item in plane popup has no represented object.
	// Set it.
	
	[ [ mPlanePopup itemAtIndex:0 ] setRepresentedObject:[ NSString stringWithCString:kIOServicePlane ] ] ;

	//
	// Configure registry outline view
	//
	{
		NSTableColumn* column = [ (NSTableColumn*)[ NSTableColumn alloc ] initWithIdentifier:@"Name" ] ;
		{
			[ column setWidth:[ mRegistryOutlineView frame ].size.width ] ;
			[ column setEditable:NO ] ;
		}
		[ mRegistryOutlineView addTableColumn:column ] ;
		[ mRegistryOutlineView setOutlineTableColumn:column ] ;		 
	
		// set root of registry view
		RegistryOutlineItem*	root = [ [ RegistryOutlineItem alloc ]
				initWithService:[ ApplicationController rootService ] 
				inPlane:kIOServicePlane ] ;
		
		{
			// get plane names and populate plane selection pop-up menu
			mPlanes = [ NSMutableDictionary dictionaryWithCapacity:10 ] ;
			[ mPlanes addEntriesFromDictionary:
					(NSDictionary*)::IORegistryEntryCreateCFProperty( [ root service ], CFSTR(kIORegistryPlanesKey), 
					kCFAllocatorDefault, 0 ) ] ;
					
			// the IOService plane is already in the planes popup menu by default; remove it from
			// discovered planes so we don't add it twice...
			[ mPlanes removeObjectForKey:(NSString*)CFSTR(kIOServicePlane) ] ;
			
			if ( [ mPlanes count ] > 0 )
			{
				[ [ mPlanePopup menu ] addItem: [ NSMenuItem separatorItem ] ] ;

				for( unsigned index=0; index < [ [ mPlanes allKeys ] count ]; ++index )
				{
					NSString*	name = [ [ mPlanes allKeys ] objectAtIndex:index ] ;
					NSMenuItem*	item = [ [ NSMenuItem alloc ] initWithTitle:name action:@selector(changePlanes:) 
							keyEquivalent: ((index < 9) ? [ NSString stringWithFormat:@"%u", index+1 ] : @"") ] ;
					[ item setTarget:self ] ;
					[ item setRepresentedObject: name ] ;
					[ [ mPlanePopup menu ] addItem: item ] ;
				}
			}
		}
		
		OutlineViewDataSource*	dataSource = (OutlineViewDataSource*)[ mRegistryOutlineView dataSource ] ;
		[ dataSource setRoot:root ] ;
		[ root release ] ;
		
		[ dataSource setShowRoot:YES ] ;
	
		// tell outline view to get all items
		[ mRegistryOutlineView reloadData ] ;
	
		// start with all items expanded
		id item = [ mRegistryOutlineView itemAtRow:0 ] ;
		if (item)
			[ mRegistryOutlineView expandItem:item expandChildren:YES ] ;
	
		// set font sizes for all cells in all columns...
		NSArray* columns = [ mRegistryOutlineView tableColumns ] ;

//		id dataCell = [ [ OutlineTableCell alloc ] init ] ;
//		[ dataCell setFont:[ NSFont fontWithName:@"Monaco" size:9.0f ] ] ;
		for(unsigned index=0; index< [ columns count ]; ++index)
		{
//			[ [ columns objectAtIndex:index ] setDataCell:dataCell ] ;			
			[ [ [ columns objectAtIndex:index ] dataCell ] setFont:[ NSFont fontWithName:@"Monaco" size:9.0f ] ] ;
		}
	}

	[ mPropertiesDrawer open ] ;

	// set up for notifications...
	[ (NSNotificationCenter*)[ NSNotificationCenter defaultCenter ] addObserver:self selector:@selector(registryChanged:) name:(NSString*)kRegistryChangedNotification object:nil ] ;
}

-(void)dealloc
{
	[ [ NSNotificationCenter defaultCenter ] removeObserver:self ] ;
	[ super dealloc ] ;
}

-(IBAction)newFind:(id)sender
{
	[ self doFind:[ mFindTextEditText stringValue ] startingItem:[ mRegistryOutlineView itemAtRow:0 ] forward:YES ] ;
}

-(IBAction)stepFind:(id)sender
{
	BOOL	forward = [ mFindStepper intValue ] < 0 ;
	
	if ( forward )
		[ self findNext ] ;
	else
		[ self findPrev ] ;

	
	[ mFindStepper setIntValue:0 ] ;
}

- (void)goToFind
{
	[ mFindTextEditText selectText:self ] ;
}

- (void)findNext
{
	RegistryOutlineItem*	startingItem = [ mRegistryOutlineView itemAtRow:[ mRegistryOutlineView selectedRow ] ] ;
	
	startingItem = [ startingItem next ] ;		
	[ self doFind:[ mFindTextEditText stringValue ] startingItem:startingItem forward:YES ] ;
}

- (void)findPrev
{
	RegistryOutlineItem*	startingItem = [ mRegistryOutlineView itemAtRow:[ mRegistryOutlineView selectedRow ] ] ;
	
	startingItem = [ startingItem prev ] ;		
	[ self doFind:[ mFindTextEditText stringValue ] startingItem:startingItem forward:NO ] ;
}

-(IBAction)changePlanes:(id)sender
{
	[ (NSString*)[ (NSMenuItem*)sender representedObject ] getCString:mCurrentPlane maxLength:sizeof(mCurrentPlane) ] ;

	RegistryOutlineItem* newRoot = [ [ RegistryOutlineItem alloc ]
										initWithService:[ ApplicationController rootService ]
										inPlane: mCurrentPlane ] ;
										
	[ (OutlineViewDataSource*)[ mRegistryOutlineView dataSource ] setRoot:newRoot ] ;
	[ newRoot release ] ;

	// tell outline view to get all items
	[ mRegistryOutlineView reloadData ] ;	
	
	// start with all items expanded
	id item = [ mRegistryOutlineView itemAtRow:0 ] ;
	if (item)
		[ mRegistryOutlineView expandItem:item expandChildren:YES ] ;
}

-(IBAction)toggleDrawer:(id)sender
{
	if ( [ (NSControl*)sender intValue ] == 0 )
		[ mPropertiesDrawer close ] ;
	else
		[ mPropertiesDrawer open ] ;
}

-(void)doFind:(NSString*)findText startingItem:(OutlineItem*)startingItem forward:(BOOL)forward
{
	BOOL	wrap = NO ;	// should be set per user prefs...

	OutlineItem*	nextFoundItem = [ [ mRegistryOutlineView dataSource ] itemMatching:findText startingItem:startingItem forward:forward wrap:wrap ] ;
	if ( !nextFoundItem )
	{
//		[ self findNotFound ] ;
	}
	else
	{
		unsigned row = [ mRegistryOutlineView rowForItem:nextFoundItem ] ;
		[ mRegistryOutlineView selectRow:row byExtendingSelection:NO ] ;
		[ mRegistryOutlineView scrollRowToVisible:row ] ;
	}
}

//
// NSWindow delegate methods
//
- (void)windowWillClose:(NSNotification *)notification
{
	[ self autorelease ] ;
}

//
// combo box delegate methods
//
- (void)comboBoxSelectionDidChange:(NSNotification *)notification
{
	[ self newFind:self ] ;
}

//
// outline view delegate methods
//
- (void)outlineViewSelectionDidChange:(NSNotification *)notification ;
{
	[ mInspectorController serviceChanged:mRegistryOutlineView ] ;
}

-(void)registryChanged:(id)sender
{
	[ reinterpret_cast<RegistryOutlineItem*>([ (OutlineViewDataSource*)[mRegistryOutlineView dataSource] root ]) checkForUpdates:mRegistryOutlineView ] ;
}

@end
