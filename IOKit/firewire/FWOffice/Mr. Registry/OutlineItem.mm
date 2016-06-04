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

	$Log: OutlineItem.mm,v $
	Revision 1.3  2005/01/31 20:27:11  firewire
	3979637 - gcc 4.0 exposed a lurking bug in FWOffice
	
	Revision 1.2  2003/05/27 17:47:00  firewire
	SDK16
	
	Revision 1.1  2002/11/07 00:36:20  noggin
	move to new repository
	
	Revision 1.6  2002/10/14 17:32:59  noggin
	added split view to properties inspector; alphabetized properties in properties inspector; fix nib problems
	
	Revision 1.5  2002/08/15 18:34:45  noggin
	*** empty log message ***
	
	Revision 1.4  2002/08/06 22:23:51  noggin
	we were creating extra retains on services.. now we release them properly.
	
	Revision 1.3  2002/07/18 22:04:18  noggin
	added spiffy outline tree lines
	
	Revision 1.2  2002/07/16 22:07:12  noggin
	now have NSData view and new icon
	
	Revision 1.1  2002/07/15 16:09:10  noggin
	moved files around. made properties inspector into a drawer.
	
	Revision 1.5  2002/05/30 18:35:49  noggin
	Fixed a few bugs. First FireWire bus view implemented.
	
	Revision 1.4  2002/05/23 19:54:05  noggin
	added CVS Logging
	
*/

#import "OutlineItem.h"

extern NSCharacterSet* keyValueDelimCharacterSet ;
extern NSCharacterSet* keyValueEndCharacterSet ;
extern NSCharacterSet* containerStartCharacterSet ;
extern NSCharacterSet* containerEndCharacterSet ;

NSRange
FindNextKey( NSString* string, NSRange searchRange )
{
	NSRange delimRange = [ string rangeOfCharacterFromSet:keyValueDelimCharacterSet options:0 range:searchRange ] ;
	if (delimRange.length == 0)
		return searchRange ;
		
	return NSMakeRange( searchRange.location, delimRange.location-searchRange.location ) ;
}

NSRange
ContainerRange( NSString* string, NSRange searchRange )
{
	if (searchRange.length < 2 )
		return NSMakeRange( searchRange.location, 0 ) ;
	
	NSRange startRange = [ string rangeOfCharacterFromSet:containerStartCharacterSet options:0 range:searchRange ] ;
	NSRange endRange = [ string rangeOfCharacterFromSet:containerEndCharacterSet options:0 range:NSMakeRange( startRange.location+1, searchRange.location + searchRange.length - startRange.location - 1 ) ] ;

	if (endRange.location - startRange.location < 3)
		return NSMakeRange( startRange.location , endRange.location - startRange.location ) ;
		
	// check for sub containers:
	NSRange insideStartRange = [ string rangeOfCharacterFromSet:containerStartCharacterSet options:0 
			range:NSMakeRange( startRange.location + 1, endRange.location - startRange.location - 1 ) ] ;
			
	if ( insideStartRange.length > 0 )
	{
		NSRange subContainerRange = ContainerRange( string, NSMakeRange( insideStartRange.location, endRange.location - insideStartRange.location - 1 ) ) ;
		endRange = [ string rangeOfCharacterFromSet:containerEndCharacterSet 
				options:0 range:NSMakeRange( subContainerRange.location+subContainerRange.length+1, endRange.location-(subContainerRange.location+subContainerRange.length+1) ) ] ;
	}
	
	return NSMakeRange( startRange.location, endRange.location - startRange.location + 1 ) ;
}

NSRange
FindNextValue( NSString* string, NSRange searchRange )
{
	// return range from start to first ',' or closing '}'
	NSRange commaRange = [ string rangeOfCharacterFromSet:keyValueEndCharacterSet options:0 range:searchRange ] ;

	if ( commaRange.length > 0 && ( commaRange.location - searchRange.location ) > 0 )
	{
		// skip over '='
		++searchRange.location ;
		--searchRange.length ;

		// find first non-whitespace char in 'searchRange'
		NSRange containerStartRange = [ string rangeOfCharacterFromSet:[ [ NSCharacterSet whitespaceAndNewlineCharacterSet ] invertedSet ]
				options:0 range:searchRange ] ;
	
		// is it the beginning of a container value? if yes - get container's range
		if ( [ containerStartCharacterSet characterIsMember:[ string characterAtIndex:containerStartRange.location ] ] )
			return ContainerRange( string, NSMakeRange( searchRange.location , searchRange.length - 1 ) ) ;

	}
	
	return NSMakeRange( searchRange.location, commaRange.location - searchRange.location ) ;
}

NSDictionary*
String2Dictionary( NSString* string )
{
	NSMutableDictionary* dict = [ NSMutableDictionary dictionaryWithCapacity:4 ] ;
	if ( [ string length ] > 2 )
	{
		unsigned index = 1 ;
		unsigned length = [ string length ] ;
		while (index < length )
		{
			NSRange	keyRange = FindNextKey( string, NSMakeRange( index, length - index - 1 ) ) ;
			NSString* key = [ string substringWithRange:keyRange ] ;
			key = [ key stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet] ] ;

			index = keyRange.location + keyRange.length ;

			NSRange valueRange = FindNextValue( string, NSMakeRange( index, length - index ) ) ;

			NSString* value = @"" ;
			if ( valueRange.length > 0 )
			{
				value = [ string substringWithRange:valueRange ] ;
				if ( [ value length ] < 3 )
					value = [ value stringByPaddingToLength:3 withString:@" " startingAtIndex:0 ] ;
				value = [ value stringByTrimmingCharactersInSet:[ NSCharacterSet whitespaceAndNewlineCharacterSet ] ] ;
			}

			index = valueRange.location + valueRange.length + 1 ;

			[ dict setObject:value forKey:key ] ;
		}
	}
	
	return dict ;
}

NSData*
String2Data( NSString* string )
{
	return nil ;
}


#pragma mark -

//
// OutlineItem
//

@implementation OutlineItem

-(id)init
{
	mChildren = nil ;
	mParent = nil ;

	return [ super init ] ;
}

-(void)dealloc
{
	[ mChildren release ] ;
	[ super dealloc ] ;
}

-(void)addChild:(OutlineItem*)child
{
	if (!mChildren)
		mChildren = [ [ NSMutableArray arrayWithCapacity:0 ] retain ] ;
	
	[ mChildren addObject:child ] ;
	child->mParent = self ;
}

-(id)child:(int)index
{
	if (!mChildren)
		return nil ;
		
	return [ mChildren objectAtIndex:index ] ;
}

-(BOOL)isExpandable
{
	if (!mChildren)
		return NO ;
	
	else return [ mChildren count ] > 0 ;
}

-(unsigned)numberOfChildren
{
	if (!mChildren)
		return 0 ;
	
	return [ mChildren count ] ;
}

- (NSArray*)children
{
	return mChildren ;
}

-(id)objectValueForTableColumn:(NSTableColumn*)column
{
	return [ column identifier ] ;
}

-(OutlineItem*)prev
{
	if (!mParent)
		return nil ;

	unsigned myChildIndex = [ mParent->mChildren indexOfObject:self ] ;
	if ( myChildIndex == 0 )
		return mParent ;
		
	OutlineItem*	 prevItem = [ mParent->mChildren objectAtIndex:myChildIndex-1 ] ;
	while ( prevItem->mChildren )
		prevItem = [ prevItem->mChildren objectAtIndex:[ prevItem->mChildren count ] - 1 ] ;
		
	return prevItem ;
}

-(OutlineItem*)next
{
	if ( mChildren )
		return [ mChildren objectAtIndex:0 ] ;
	if ( !mParent )
		return nil ;

	OutlineItem*	 item = self ;
	while ( item )
	{
		unsigned parentNextChildIndex = [ item->mParent->mChildren indexOfObject:item ] + 1 ;
		if ( parentNextChildIndex < [ item->mParent->mChildren count ] )
			return [ item->mParent->mChildren objectAtIndex:parentNextChildIndex ] ;

		item = item->mParent ;
		if (!item->mParent)
			return nil ;			
	}
	
	return nil ;
}

-(BOOL)doesMatch:(NSString*)findText
{
	return NO ;
}

-(BOOL)hasParent
{
	return mParent != nil ;
}

-(OutlineItem*)parent
{
	return mParent ;
}

-(NSString*)name
{
	return (nil != mName) ? mName : [ self createName ] ;
}

-(NSString*)createName
{
	return ( mName = [ @"" retain ] ) ;
}

-(void)sortChildrenByName
{
	[ mChildren sortUsingSelector:@selector(nameCompare:) ] ;
}

- (NSComparisonResult)nameCompare:(OutlineItem*)otherItem
{
	return [ mName compare:otherItem->mName options:NSCaseInsensitiveSearch ] ;
}

@end

//
// RegistryOutlineItem
//

#pragma mark -
@implementation RegistryOutlineItem

-(void)dealloc
{	
	[ mName release ] ;
	[ mClassName release ] ;
	
	[ super dealloc ] ;
}

-(id)initWithService:(io_service_t)service inPlane:(const io_name_t)plane
{
	if ( self != [ self init ] )
		return nil ;
	
	mPlane = (char*)plane ;
	mService = service ;
	mObserver = nil ;

	if ( mService )
	{
		mName = nil ;

		io_name_t			className ;
		IOReturn	error = ::IOObjectGetClass( service, className ) ;
		if(error)
			[NSException raise:@"IOReturn" format:@"couldn't get io object class"] ;
		
		mClassName = [ [ NSString alloc ] initWithCString:className ] ;

		// create children
		io_iterator_t	childIterator = 0 ;
		error = ::IORegistryEntryGetChildIterator( service, mPlane, & childIterator ) ;
		if ( error )
			[ NSException raise:@"IOReturn" format:@"couldn't get child iterator" ] ;

		io_service_t		childService ;
		while ( nil != ( childService = ::IOIteratorNext( childIterator ) ) )
		{
			RegistryOutlineItem* newItem = [ [ RegistryOutlineItem alloc ] initWithService:childService inPlane:mPlane ] ;
			[ self addChild:newItem ] ;
			[ newItem release ] ;
		}

		::IOObjectRelease( childIterator ) ;
	}
	else
	{
		mName = [ [ NSString alloc ] initWithString:@"Root" ] ;
		mClassName = [ [ NSString alloc ] initWithString:@"" ] ;
	}		

	return self ;
}

-(id)objectValueForTableColumn:(NSTableColumn*)column
{
	if ( [ [ column identifier ] isEqual:@"Name" ] )
		return [ self name ] ;
	else if ( [ [ column identifier ] isEqual:@"Class" ] )
		return mClassName ;
	
	[ NSException raise:@"" format:@"%s %u: unknown column name", __FILE__, __LINE__ ] ;
	
	return nil ;	// to avoid compiler warning
}

-(BOOL)checkForUpdates:(NSOutlineView*)outlineView
{
	unsigned		childCount 					= [ mChildren count ] ;
	
	BOOL			differentChildren 			= NO ;
	BOOL			childUpdated				= NO ;

	// get an iterator for what the registry says should be our children
	io_iterator_t	iterator ;
	IOReturn		err 			= IORegistryEntryGetChildIterator( mService, mPlane, &iterator ) ;

	if ( err )
		[ NSException raise:@"" format:@"%s %u: couldn't get iterator", __FILE__, __LINE__ ] ;

	// go through all children found in registry
	io_service_t	service 		= IOIteratorNext( iterator ) ;

	if ( childCount == 0)
	{
		differentChildren = (service != nil) ;

		while ( service )
		{
			RegistryOutlineItem*	newItem = [ [ RegistryOutlineItem alloc ] initWithService:service inPlane:mPlane ] ;
			[ self addChild: newItem ] ;
			[ newItem retain ] ;

			service = IOIteratorNext( iterator ) ;
		}

	}
	else
	{
		BOOL			stillExists[ 1 + childCount ] ;		// make at least 1
		
		// set all children as non-existant until they are discovered again
		for( unsigned index = 0; index < childCount; index++ )
			stillExists[index] = NO ;

		while ( service )
		{
			BOOL		childFound = NO ;
	
			// check all known children to see if any of them corresponds
			// to the child service we found in the registry
			for( unsigned index = 0; index < childCount; ++index )
			{
				RegistryOutlineItem*		child = [ mChildren objectAtIndex:index ] ;
				if ( [ child service ] == service )
				{
					// found it. let it do any updates it needs to
					childUpdated |= [ child checkForUpdates:outlineView ] ;
	
					// mark child as still existing
					stillExists[index] = YES ;
					childFound = YES ;
				}
			}
		
			// if none of our children has this service, it's new
			if ( !childFound || childCount == 0 )
			{
				RegistryOutlineItem*	newItem = [ [ RegistryOutlineItem alloc ] initWithService:service inPlane:mPlane ] ;
				[ self addChild: newItem ] ;
				[ newItem release ] ;

				differentChildren = YES ;
			}

			service = IOIteratorNext( iterator ) ;
		}
	
		for( int index = childCount - 1; index >= 0; --index )	// handle in last-to-first ordering...
		{
			// handle all services that are going away...
			if ( !stillExists[index] )
			{
				RegistryOutlineItem* child = [ mChildren objectAtIndex:index ] ;
				
				// if item has observer, tell observer item is going away:
				[ child invalidate ] ;
				[ mChildren removeObjectAtIndex:index ] ;
				
				differentChildren = YES ;
			}
		}
	}
	
	// if we added or removed children pass update up to parent
	if ( differentChildren )
	{
		[ outlineView reloadItem:self reloadChildren:YES ] ;

		return NO ;
	}
	
	// if any children updated, update ourselves
	if ( childUpdated )
	{
		[ outlineView reloadItem:self reloadChildren:YES ] ;
	}
	
	return NO ;
}

-(io_service_t)service
{
	return mService ;
}

-(BOOL)doesMatch:(NSString*)findText
{
	NSRange findRange = [ [ self name ] rangeOfString:findText options:NSCaseInsensitiveSearch ] ;
	return findRange.location != NSNotFound ;
}

-(NSString*)createName
{
	// get name
	io_name_t			serviceName ;

	IOReturn			error = ::IORegistryEntryGetNameInPlane( mService, mPlane, serviceName ) ;
	if (error)
		[ NSException raise:@"IOReturn" format:@"couldn't get service name" ] ;
	mName = [[ NSString stringWithCString:serviceName ] retain ] ;
	
	return mName ;
}

- (NSString*)ioObjectClassName
{
	return mClassName ;
}

- (void)addObserver:(id)observer
{
	// note: we currently don't pay attention to who the observer is.
	assert(!mObserver) ;	// can't set observer if we already have one set!

	mObserver = observer ;
}

- (void)removeObserver:(id)observer
{
	// note: we currently don't pay attention to who the observer is.
	mObserver = nil ;
}

- (void)invalidate
{
	if ( mChildren )
	{
		unsigned childCount = [ mChildren count ] ;
		for( unsigned index=0; index < childCount; ++index )
			[ (RegistryOutlineItem*)[ mChildren objectAtIndex:index ] invalidate ] ;
	}
	
	[ mObserver serviceWasInvalidated:self ] ;
}

@end

#pragma mark -
@implementation DictionaryOutlineItem

-(void)dealloc
{
	[ mValue release ] ;
	[ super dealloc ] ;
}

-(id)initWithValue:(id)value key:(id)key
{
	if ( self != [ self init ] )
		return nil ;
	
	mValue = 0 ;
	id tempValue = value ;

	// handle broken power manager properties:
	// for some reason all the power manager property dictionaries come from the kernel
	// as strings ?!
	if ( [ tempValue isKindOfClass:[ NSString class ] ] )
	{
		// is value a dictionary as a string?
		if ([ tempValue hasPrefix:@"{"] && [ tempValue hasSuffix:@"}"])		//fixthis: should check first non-whitespace char to be in
																			// dictionary opener set, not first char
		{
			tempValue = String2Dictionary( tempValue ) ;
		}
		// is value data as a string?
		else if ([tempValue hasPrefix:@"<"] && [ tempValue hasPrefix:@">"])
		{
			tempValue = String2Data( tempValue ) ;
		}
	}

	if ( [ tempValue isKindOfClass:[ NSDictionary class ] ] )
	{
		[ self makeChildrenWithKeys:[ (NSDictionary*)tempValue allKeys ] values:[ (NSDictionary*)tempValue allValues ] ] ;
		[ self sortChildrenByName ] ;
	}
	else if ( [ tempValue isKindOfClass:[ NSArray class ] ] )
	{
		NSMutableArray* keys = [ NSMutableArray arrayWithCapacity:[ (NSArray*)tempValue count ] ] ;
		for( unsigned index=0; index < [ (NSArray*)tempValue count ]; ++index )
			[ keys addObject:[ NSNumber numberWithInt:index ] ] ;
		[ self makeChildrenWithKeys:keys values:tempValue ] ;
	}
	else
		mValue = [ tempValue retain ] ;

	mName = [ key retain ] ;
	
	return self ;
}

-(void)makeChildrenWithKeys:(NSArray*)keys values:(NSArray*)values
{
	for( unsigned index=0; index < [ keys count ]; ++index )
	{
		DictionaryOutlineItem* child = [ [ DictionaryOutlineItem alloc ] initWithValue:[ values objectAtIndex:index ] key:[ keys objectAtIndex:index] ] ;
		[ self addChild:child ] ;
	}
}

-(id)objectValueForTableColumn:(NSTableColumn*)column
{
	id result = nil ;
	if ( [ [ column identifier ] isEqual:@"Key" ] )
		result = [ self name ] ;
	else if ( [ [ column identifier ] isEqual:@"Value" ] )
	{
		if ( [ mValue isKindOfClass:[ NSNumber class ] ] )
		{		
			result = [ NSString stringWithFormat:@"0x%qx (%lld)", [ mValue longLongValue ], [ mValue longLongValue ] ] ;
		}
		else if ( [ mValue isKindOfClass:[ NSString class ] ] )
		{
			result = [ NSString stringWithFormat:@"\"%@\"", mValue ] ;
		}
		else
		{
			result =  mChildren ? @"" : mValue ;
		}
	}
	else
		[ NSException raise:@"" format:@"%s %u: unknown column name \"%@\"", __FILE__, __LINE__, [ column identifier ] ] ;
	
	return result ;	// to avoid compiler warning
}


-(id)value
{
	return mValue ;
}

//- (NSComparisonResult)nameCompare:(DictionaryOutlineItem*)otherItem
//{
//	return [ [self name] compare:[ otherItem name ] ] ;
//}

@end
