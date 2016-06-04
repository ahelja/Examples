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

	$Log: OutlineItem.h,v $
	Revision 1.3  2005/01/31 20:27:10  firewire
	3979637 - gcc 4.0 exposed a lurking bug in FWOffice
	
	Revision 1.2  2003/05/27 17:47:00  firewire
	SDK16
	
	Revision 1.1  2002/11/07 00:36:20  noggin
	move to new repository
	
	Revision 1.4  2002/10/14 17:32:59  noggin
	added split view to properties inspector; alphabetized properties in properties inspector; fix nib problems
	
	Revision 1.3  2002/08/15 18:34:44  noggin
	*** empty log message ***
	
	Revision 1.2  2002/07/18 22:04:18  noggin
	added spiffy outline tree lines
	
	Revision 1.1  2002/07/15 16:09:09  noggin
	moved files around. made properties inspector into a drawer.
	
	Revision 1.4  2002/05/30 18:35:49  noggin
	Fixed a few bugs. First FireWire bus view implemented.
	
	Revision 1.3  2002/05/23 19:54:05  noggin
	added CVS Logging
	
*/

#import <Cocoa/Cocoa.h>
#import <IOKit/IOKitLib.h>

@interface OutlineItem : NSObject {
	NSMutableArray*		mChildren ;
	OutlineItem*		mParent ;
	NSString*			mName ;
}

-(id)init ;
-(void)addChild:(OutlineItem*)child ;

-(id)child:(int)index ;
-(BOOL)isExpandable ;
-(unsigned)numberOfChildren ;
- (NSArray*)children ;
-(id)objectValueForTableColumn:(NSTableColumn*)column ;
-(OutlineItem*)prev ;
-(OutlineItem*)next ;
-(BOOL)doesMatch:(NSString*)findText ;
-(BOOL)hasParent ;
-(OutlineItem*)parent ;

-(NSString*)name ;
-(NSString*)createName ;
- (void)sortChildrenByName ;
- (NSComparisonResult)nameCompare:(OutlineItem*)otherItem ;

@end

#pragma mark -
@interface RegistryOutlineItem: OutlineItem {
	io_service_t		mService ;
	NSString*			mClassName ;
	char*				mPlane ;
	id					mObserver ;
}

-(id)initWithService:(io_service_t)service inPlane:(const io_name_t)plane ;
-(id)objectValueForTableColumn:(NSTableColumn*)column ;
-(BOOL)checkForUpdates:(NSOutlineView*)outlineView ;
-(io_service_t)service ;
-(BOOL)doesMatch:(NSString*)findText ;
-(NSString*)createName ;
-(NSString*)ioObjectClassName ;
- (void)addObserver:(id)observer ;
- (void)removeObserver:(id)observer ;
- (void)invalidate ;

@end

@interface RegistryOutlineItem(RegistryOutlineItemObserver)
- (void)serviceWasInvalidated:(id)sender ;
@end

#pragma mark -
@interface DictionaryOutlineItem: OutlineItem {
//	NSString*			mName ;
	id					mValue ;
}

-(id)initWithValue:(id)value key:(id)key ;
-(void)makeChildrenWithKeys:(NSArray*)keys values:(NSArray*)values ;

//-(id)initWithDictionary:(NSDictionary*)dict name:(NSString*)name ;
//-(id)initWithKey:(NSString*)key value:(id)value ;
-(id)value ;
//- (NSComparisonResult)nameCompare:(DictionaryOutlineItem*)otherItem ;

@end
