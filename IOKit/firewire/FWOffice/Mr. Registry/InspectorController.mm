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

	$Log: InspectorController.mm,v $
	Revision 1.3  2004/06/08 22:54:33  firewire
	*** empty log message ***
	
	Revision 1.2  2003/05/27 17:46:59  firewire
	SDK16
	
	Revision 1.1  2002/11/07 00:36:19  noggin
	move to new repository
	
	Revision 1.5  2002/08/15 18:34:44  noggin
	*** empty log message ***
	
	Revision 1.4  2002/07/18 22:04:18  noggin
	added spiffy outline tree lines
	
	Revision 1.3  2002/07/16 22:07:12  noggin
	now have NSData view and new icon
	
	Revision 1.2  2002/07/15 21:17:44  noggin
	*** empty log message ***
	
	Revision 1.1  2002/07/15 16:09:09  noggin
	moved files around. made properties inspector into a drawer.
	
	Revision 1.4  2002/05/23 19:54:05  noggin
	added CVS Logging
	
*/
#import "InspectorController.h"
#import "OutlineItem.h"
#import "OutlineViewDataSource.h"
#import "NSDataTableDataSource.h"

extern const NSString*		kServiceChangedNotification ;

const NSString*				kTextValueTabIdentifier = @"Text" ;
const NSString*				kDataValueTabIdentifier = @"Data" ;

@implementation InspectorController

-(id)init
{
	if ( self != [ super init ] )
		return nil ;
	
	mServiceItem = nil ;
	
	return self ;
}

-(void)dealloc
{
	[ [ NSNotificationCenter defaultCenter ] removeObserver:self ] ;
//	[ mServiceItem release ] ;
	
	[ super dealloc ] ;
}

-(void)awakeFromNib
{
	[ mServiceName setStringValue:@"---" ] ;
	[ mClassName setStringValue:@"---" ] ;

	// set font sizes for all cells in all columns...
	{
		NSArray* columns = [ mOutlineView tableColumns ] ;
		for(unsigned index=0; index< [ columns count ]; ++index)
			[ [ [ columns objectAtIndex:index] dataCell ] setFont:[ NSFont fontWithName:@"Monaco" size:10 ] ] ;
	}
	
	[ mKeyTextField setFont: [ NSFont fontWithName:@"Monaco" size:10 ] ] ;
	[ mValueTextField setFont: [ NSFont fontWithName:@"Monaco" size:10 ] ] ;

	{
		NSArray* columns = [ mValueTableView tableColumns ] ;
		for(unsigned index=0; index< [ columns count ]; ++index)
			[ [ [ columns objectAtIndex:index] dataCell ] setFont:[ NSFont fontWithName:@"Monaco" size:10 ] ] ;
	}
}

-(IBAction)updateNow:(id)sender
{
	if ( mServiceItem )
	{
		CFMutableDictionaryRef	dict ;
		IORegistryEntryCreateCFProperties( [ mServiceItem service ], &dict, kCFAllocatorDefault, 0 ) ;
		
		DictionaryOutlineItem* newRoot = [ [ DictionaryOutlineItem alloc ] initWithValue:(NSDictionary*)dict key:@"Root"] ;
		[ (OutlineViewDataSource*)[ mOutlineView dataSource ] setRoot:newRoot ] ;
		[ newRoot release ] ;

		NSString*	name = [ mServiceItem name ] ;		
		[ mServiceName setStringValue:name ] ;

		NSString*	className = [ mServiceItem ioObjectClassName ] ;
		[ mClassName setStringValue: [ NSString stringWithFormat:@"Class: %@", className ] ] ;
		
		[ mOutlineView reloadData ] ;
		
		// expand all top level items in inspector's outline view if there are any
		if ([ mOutlineView numberOfRows ] > 0)
			for( int index=([ mOutlineView numberOfRows ] - 1) ; index >= 0; --index )
				[ mOutlineView expandItem:[ mOutlineView itemAtRow:index ] expandChildren:NO ] ;
	}
	else
	{
		[ mServiceName setStringValue:@"" ] ;
		[ mClassName setStringValue:@"" ] ;
		[ (OutlineViewDataSource*)[ mOutlineView dataSource ] setRoot:nil ] ;
		[ mOutlineView reloadData ] ;
	}
}

-(void)serviceChanged:(NSOutlineView*)outlineView
{
	if (mServiceItem)
	{
		[ mServiceItem removeObserver:self ] ;
		[ mServiceItem release ] ;
		mServiceItem = nil ;
	}

	unsigned selectedRow = [ outlineView selectedRow ] ;
	if (selectedRow != 0xFFFFFFFF)
	{
		mServiceItem = [ (RegistryOutlineItem*)[ outlineView itemAtRow:selectedRow ] retain ] ;
		[ mServiceItem addObserver:self ] ;
		
		[ self updateNow:self ] ;
	}
}

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
	DictionaryOutlineItem*	item = [ mOutlineView itemAtRow:[ mOutlineView selectedRow ] ] ;
	
	[ mKeyTextField setStringValue:( [ item name ] ? [ item name ] : @"" ) ] ;

	id value = [ item value ] ;

	if ( [ value isKindOfClass:[ NSData class ] ] )
	{
		[ mValueTabView selectTabViewItemWithIdentifier:(NSString*)kDataValueTabIdentifier ] ;
		
		[ (NSDataTableDataSource*)[ mValueTableView dataSource ] setData:value ] ;
		[ mValueTableView reloadData ] ;
	}
	else
	{
		[ mValueTabView selectTabViewItemWithIdentifier:(NSString*)kTextValueTabIdentifier ] ;
		
		if ( [ value isKindOfClass:[ NSNumber class ] ] )
			[ mValueTextField setString:[ NSString stringWithFormat:@"0x%qx (%lld)", [ (NSNumber*)value longLongValue ], [ (NSNumber*)value longLongValue ] ] ] ;
		else if ( [ value isKindOfClass:[ NSString class ] ] )
			[ mValueTextField setString:[ NSString stringWithFormat:@"\"%@\"", value ] ] ;
		else
			[ mValueTextField setString:( value ? [ value description ] : @"" ) ] ;
	}

}

-(void)windowWillClose:(NSNotification*)notification
{
	[ self autorelease ] ;
}

//
// NSDrawer notifications
//
- (void)drawerWillOpen:(NSNotification *)notification
{
	[ mDrawerButton setIntValue:1 ] ;
}

- (void)drawerDidClose:(NSNotification *)notification;
{
	[ mDrawerButton setIntValue:0 ] ;
}

//
// RegistryOutlineItemObserver protol implementation
//
- (void)serviceWasInvalidated:(id)sender ;
{
	[ mServiceItem removeObserver:self ] ;
	[ mServiceItem release ] ;
	mServiceItem = nil ;
	
	[ self updateNow:self] ;
}

@end
