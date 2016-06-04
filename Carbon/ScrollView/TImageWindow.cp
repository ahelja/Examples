/*
    File:		TImageWindow.cp
    
    Version:	Mac OS X

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

#include "TImageWindow.h"

static const Rect kInitialBounds = { 100, 100, 500, 500 };

static CGImageRef		GetTestImage();

TImageWindow::TImageWindow()
	: TWindow( 	kDocumentWindowClass, kWindowStandardDocumentAttributes |
				kWindowLiveResizeAttribute |
				kWindowCompositingAttribute, kInitialBounds )
{
	HIViewRef		contentView;
	HIRect			bounds;
	CGImageRef		testImage;
	
	const EventTypeSpec	events[] =
		{
			{ kEventClassWindow, kEventWindowGetMinimumSize },
			{ kEventClassWindow, kEventWindowGetMaximumSize }
		};

	const EventTypeSpec	contentEvents[] =
		{
			{ kEventClassControl, kEventControlBoundsChanged }
		};	
    
	SetWindowTitleWithCFString( GetWindowRef(), CFSTR( "ImageView" ) );

	RegisterForEvents( GetEventTypeCount( events ), events );

	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kHIViewWindowContentID, &contentView );
	
	InstallEventHandler( HIObjectGetEventTarget( (HIObjectRef)contentView ), 
						HandleContentBoundsChanged, GetEventTypeCount( contentEvents ),
						contentEvents, this, NULL );

	testImage = GetTestImage();
	HIImageViewCreate( testImage, &fImageView );
	HIViewGetBounds( contentView, &bounds );
	HIViewSetFrame( fImageView, &bounds );
	HIViewSetVisible( fImageView, true );
	HIViewAddSubview( contentView, fImageView );
	CGImageRelease( testImage );
	
	RepositionWindow( GetWindowRef(), NULL, kWindowCascadeOnMainScreen );
	
    SetUserFocusWindow( GetWindowRef() );
}

TImageWindow::~TImageWindow()
{
	// no need to dispose the view - since its a control, it will be torn down automatically
}

OSStatus
TImageWindow::HandleEvent( EventHandlerCallRef inCallRef, TCarbonEvent& inEvent )
{
	OSStatus	result = eventNotHandledErr;
	UInt32		kind = inEvent.GetKind();
	
	switch ( inEvent.GetClass() )
	{
		case kEventClassWindow:
			switch ( kind )
			{
				case kEventWindowGetMinimumSize:
					{
						HISize		min;
						Point		size;
						
						HIViewGetSizeConstraints( fImageView, &min, NULL );
						size.h = (SInt16)min.width;
						size.v = (SInt16)min.height;
						
						inEvent.SetParameter( kEventParamDimensions, size );
						result = noErr;
					}
					break;

				case kEventWindowGetMaximumSize:
					{
						HISize		max;
						Point		size;
						
						HIViewGetSizeConstraints( fImageView, NULL, &max );
						size.h = (SInt16)max.width;
						size.v = (SInt16)max.height;
						
						inEvent.SetParameter( kEventParamDimensions, size );
						result = noErr;
					}
					break;
			}
			break;
	}

	if ( result == eventNotHandledErr )
		result = TWindow::HandleEvent( inCallRef, inEvent );

	return result;
}


Boolean
TImageWindow::UpdateCommandStatus( const HICommand& command )
{
	Boolean		handled = false;
	
	switch ( command.commandID )
	{
		case 'TGRO':
			EnableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			handled = true;
			break;
		
		default:
			handled = TWindow::UpdateCommandStatus( command );
			break;
	}
	
	return handled;
}

Boolean
TImageWindow::HandleCommand( const HICommand& command )
{
	Boolean		handled = false;
	
	switch ( command.commandID )
	{
		case 'TGRO':
			{
				HIViewRef		view;
				
				HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kHIViewWindowGrowBoxID, &view );
				if ( view )
					HIGrowBoxViewSetTransparent( view, !HIGrowBoxViewIsTransparent( view ) );
			}
			handled = true;
			break;
	}
	
	return handled;
}


static CGImageRef
GetTestImage()
{
	CGDataProviderRef	provider;
	CFBundleRef 		appBundle = ::CFBundleGetMainBundle();
	CGImageRef			image = NULL;
	
	if ( appBundle )
	{
		CFURLRef url = ::CFBundleCopyResourceURL( appBundle, CFSTR( "Car.jpg" ), NULL, NULL );
		
		provider = CGDataProviderCreateWithURL( url );

		image = CGImageCreateWithJPEGDataProvider( provider, NULL, false,  kCGRenderingIntentDefault );
		
		CGDataProviderRelease( provider );
		CFRelease( url );
	}
	
	return image;
}

OSStatus
TImageWindow::HandleContentBoundsChanged( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	HIRect			bounds;
	UInt32			attrs;
	TImageWindow*	window = (TImageWindow*)inUserData;
	
	GetEventParameter( inEvent, kEventParamAttributes, typeUInt32, NULL, sizeof( UInt32 ), NULL, &attrs );
	
	if ( attrs & kControlBoundsChangeSizeChanged )
	{
		HIViewRef		contentView;
		
		HIViewFindByID( HIViewGetRoot( window->GetWindowRef() ), kHIViewWindowContentID, &contentView );
		HIViewGetBounds( contentView, &bounds );
		HIViewSetFrame( window->fImageView, &bounds );
	}
	
	return noErr;
}
