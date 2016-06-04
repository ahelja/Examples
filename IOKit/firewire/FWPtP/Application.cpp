/*
	File:		FWPtP/Application.cpp

	Synopsis: 	Sample code for simple peer-to-peer FireWire protocol application.

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.

	Written by: NWG
	
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

	$Log: Application.cpp,v $
	Revision 1.3  2005/02/03 00:35:40  firewire
	*** empty log message ***
	
	Revision 1.2  2003/12/04 18:35:03  firewire
	Fixed "bug": 3451368 FWPtP: move stuff out of the loop as much as possible
	Meaningless change, but it makes the performance ppl happier...even
	though it's sample code
	
	Revision 1.1  2002/11/07 00:37:57  noggin
	move to new repository
	
	Revision 1.2  2002/10/15 21:03:13  noggin
	now properly handle unplugged nodes. added skipped packet handler (just in case).
	
	Revision 1.1  2002/10/14 22:23:03  noggin
	ready for SDK14
	
*/

#import "Application.h"
#import "Window.h"
#import "PtPLocalNode.h"
#import "PtPMatcher.h"
#import "PtPRemoteUnit.h"
#import <Carbon/Carbon.h>

const EventTypeSpec Application::kCommands[1]	= { { kEventClassCommand, kEventProcessCommand } } ;

#if LOOPBACK_TEST > 0
FWAddress	gLocalAddress ;
#endif

Application::Application():		mEventHandlerUPP( ::NewEventHandlerUPP( reinterpret_cast<EventHandlerProcPtr>(&Application::S_EventHandler) ) )
{
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = ::CreateNibReference( CFSTR("main"), & mNIBRef );
	if ( err )
		throw ;
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = ::SetMenuBarFromNib( mNIBRef, CFSTR("MenuBar"));
	if ( err )
		throw ;
    
	mLocalNode = new PtPLocalNode( *this ) ;
	mWindow = new Window( mNIBRef, CFSTR("MainWindow")) ;
	mRemoteUnits = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, NULL ) ;
	
	mMatcher = new PtPMatcher( *this ) ;

	err = ::InstallEventHandler( ::GetApplicationEventTarget(), mEventHandlerUPP, 1, kCommands, this, nil ) ;
	if ( err )
		throw ;
	
	#if LOOPBACK_TEST > 0
		mWindow->Echo( CFSTR("FWPtP"), CFSTR("Loopback test build")) ;
	#endif
}

Application::~Application()
{
	delete mMatcher ;
	delete mLocalNode ;
	delete mWindow ;

    // We don't need the nib reference anymore.
    ::DisposeNibReference( mNIBRef );
}
    
int
Application::Run()
{
	mWindow->Show() ;
	
	::RunApplicationEventLoop() ;
	
	return EXIT_SUCCESS ;
}

void
Application::AddRemote(
	PtPRemoteUnit*		inUnit ) 
{
	::CFArrayAppendValue( mRemoteUnits, inUnit ) ;
}

void
Application::RemoveRemote( PtPRemoteUnit* unit )
{
	::CFArrayRemoveValueAtIndex( mRemoteUnits, ::CFArrayGetFirstIndexOfValue( 
			mRemoteUnits, ::CFRangeMake( 0, ::CFArrayGetCount( mRemoteUnits ) ), unit ) ) ;
}

void
Application::Transmit()
{
	CFStringRef		msg = mWindow->GetInput() ;
	mWindow->EraseInput() ;
	
	int count = CFArrayGetCount( mRemoteUnits );
	for( CFIndex index = 0; index < count; index++ )
	{
		((PtPRemoteUnit*)::CFArrayGetValueAtIndex( mRemoteUnits, index))->Message( msg ) ;
	}
	
	mWindow->Echo( CFSTR("local"), msg ) ;
	
	::CFRelease( msg ) ;
}

void
Application::Receive(
	UInt16			inSenderNodeID,
	CFStringRef		inMsg)
{
	CFStringRef		senderName = ::CFStringCreateWithFormat( kCFAllocatorDefault, 0, CFSTR("%4x"), inSenderNodeID ) ;

	mWindow->Echo( senderName, inMsg ) ;

	::CFRelease( senderName ) ;
}

OSStatus
Application::S_EventHandler(
	EventHandlerCallRef 	inHandlerCallRef, 
	EventRef 				e, 
	Application*			me)
{
	HICommand	command ;
	::GetEventParameter( e, kEventParamDirectObject, typeHICommand, nil, sizeof( command ), nil, & command ) ;

	switch( command.commandID )
	{
		case 'xmit':
			me->Transmit() ;
			break ;
		default:
			return ::CallNextEventHandler( inHandlerCallRef, e ) ;
			break ;
	}

	return noErr ;
}
