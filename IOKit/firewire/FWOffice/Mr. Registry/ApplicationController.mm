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

	$Log: ApplicationController.mm,v $
	Revision 1.5  2005/02/03 00:37:41  firewire
	*** empty log message ***
	
	Revision 1.4  2004/06/09 01:34:15  firewire
	*** empty log message ***
	
	Revision 1.3  2004/06/08 22:54:33  firewire
	*** empty log message ***
	
	Revision 1.2  2003/05/27 17:46:59  firewire
	SDK16
	
	Revision 1.1  2002/11/07 00:36:18  noggin
	move to new repository
	
	Revision 1.5  2002/10/14 17:32:59  noggin
	added split view to properties inspector; alphabetized properties in properties inspector; fix nib problems
	
	Revision 1.4  2002/08/06 22:23:51  noggin
	we were creating extra retains on services.. now we release them properly.
	
	Revision 1.3  2002/07/23 16:36:35  noggin
	first draft of graphing classes
	
	Revision 1.2  2002/07/16 22:07:11  noggin
	now have NSData view and new icon
	
	Revision 1.1  2002/07/15 16:09:08  noggin
	moved files around. made properties inspector into a drawer.

	Revision 1.5  2002/05/30 18:35:48  noggin
	Fixed a few bugs. First FireWire bus view implemented.

	Revision 1.4  2002/05/23 19:54:04  noggin
	added CVS Logging

 */

#import "ApplicationController.h"
#import "BrowserController.h"
#import "../Common/Utils.h"

extern const NSString*		kBrowserNibName ;
extern const NSString*		kInspectorNibName ;
extern const NSString*		kRegistryChangedNotification ;
extern const NSString*		kPrefsNibName ;
extern const NSString*		kIOKitInspectorNibName ;
extern const NSString*		kPrefsClosedNotification ;
extern const NSString*		kIOKitInspectorClosedNotification ;
extern NSCharacterSet* keyValueDelimCharacterSet ;
extern NSCharacterSet* keyValueEndCharacterSet ;
extern NSCharacterSet* containerStartCharacterSet ;
extern NSCharacterSet* containerEndCharacterSet ;

//
// ApplicationController
//

@implementation ApplicationController

-(id)init
{
	keyValueDelimCharacterSet = [ [ NSCharacterSet characterSetWithCharactersInString:@",=" ] retain ] ;
	keyValueEndCharacterSet = [ [ NSCharacterSet characterSetWithCharactersInString:@",}" ] retain ] ;
	containerStartCharacterSet = [ [ NSCharacterSet characterSetWithCharactersInString:@"{<(" ] retain ];
	containerEndCharacterSet = [ [ NSCharacterSet characterSetWithCharactersInString:@"}>)" ] retain ];

	mStarted = false ;
	
	return [ super init ] ;
}

-(void)dealloc
{
	IOObjectRelease( mNotification ) ;

	[ [ NSNotificationCenter defaultCenter ] removeObserver:self ] ;
	IOObjectRelease( mNotification ) ;
	// clean up notification port 
	// clean up run loop source

	[ keyValueDelimCharacterSet release ] ;
	[ keyValueEndCharacterSet release ] ;
	[ containerStartCharacterSet release ] ;
	[ containerEndCharacterSet release ] ;

	[ super dealloc ] ;
}

-(void)awakeFromNib
{
	if (!mStarted)
	{
		mStarted = true ;

		// get registry root
		mach_port_t		masterPort ;
		IOReturn		err = IOMasterPort( MACH_PORT_NULL, & masterPort ) ;
		if ( err )
			[ NSException raise:@"" format:@"%s %u: couldn't get master port", __FILE__, __LINE__ ] ;

		// set up for registry change notifications...
		mNotificationPort	= IONotificationPortCreate( masterPort ) ;
		if ( ! mNotificationPort )
			[ NSException raise:@"" format:@"%s %u: couldn't get notification port", __FILE__, __LINE__ ] ;
		
		CFRunLoopSourceRef		runLoopSourceRef	= ::IONotificationPortGetRunLoopSource( mNotificationPort ) ;
		if ( ! runLoopSourceRef )
			[ NSException raise:@"" format:@"%s %u: couldn't get notification port run loop source", __FILE__, __LINE__ ] ;
	
		::CFRunLoopAddSource( ::CFRunLoopGetCurrent(), runLoopSourceRef, kCFRunLoopDefaultMode ) ;
		
		// make general hardware added/removed notification
		io_service_t	service ;
		err = ::IORegistryEntryGetChildEntry( [ ApplicationController rootService ], kIOServicePlane, & service) ;
		if ( err )
			[ NSException raise:@"" format:@"%s %u: couldn't setup registry changed notification", __FILE__, __LINE__ ] ;			

		err = ::IOServiceAddInterestNotification( mNotificationPort, service, 
					kIOBusyInterest, reinterpret_cast<IOServiceInterestCallback>(RegistryChanged), self, & mNotification ) ;
		if ( err )
			[ NSException raise:@"" format:@"%s %u: couldn't setup registry changed notification", __FILE__, __LINE__ ] ;			

		// open starting windows
		[ self newBrowser:self ] ;
	}
}

- (IBAction)newBrowser:(id)sender
{
	[ NSBundle loadNibNamed:(NSString*)kBrowserNibName owner:[ [ BrowserController alloc ] init ] ] ;
}

- (IBAction)doPrefs:(id)sender
{
	[ mPrefsWindow orderFront:self ] ;
}

// find menu items
- (IBAction)browserDoFind:(id)sender
{
	NSWindow* keyWindow = [ [ NSApplication sharedApplication ] keyWindow ] ;
	if (keyWindow)
		[ (BrowserController*)[ keyWindow delegate ] goToFind ] ;
}

- (IBAction)browserFindNext:(id)sender
{
	NSWindow* keyWindow = [ [ NSApplication sharedApplication ] keyWindow ] ;
	if (keyWindow)
		[ (BrowserController*)[ keyWindow delegate ] findNext ] ;
}

- (IBAction)browserFindPrev:(id)sender
{
	NSWindow* keyWindow = [ [ NSApplication sharedApplication ] keyWindow ] ;
	if (keyWindow)
		[ (BrowserController*)[ keyWindow delegate ] findPrev ] ;
}


+ (io_service_t)rootService 
{
	return RootService() ;
}

@end

void
RegistryChanged(
	ApplicationController*	controller,
	io_service_t			,//service,
	natural_t				,//messageType,
	void *					messageArgument )
{
	// only update when root goes not busy
	if ( messageArgument == 0 )
		[ (NSNotificationCenter*)[ NSNotificationCenter defaultCenter ] postNotificationName:(NSString*)kRegistryChangedNotification object:controller ];
}

