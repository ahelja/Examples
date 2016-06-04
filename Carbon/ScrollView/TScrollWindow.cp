/*
    File:		TScrollWindow.cp
    
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

#include "TScrollWindow.h"
#include "TScrollableView.h"

#define USE_PANTHER_FEATURES		1

static const Rect kInitialBounds = { 100, 100, 500, 500 };

static CGImageRef		GetTestImage();
static CGImageRef		GetLogoImage();

#if USE_PANTHER_FEATURES
const ControlID kScrollViewID = { 'Scrl', 1 };
const ControlID kImageViewID = { 'Imag', 1 };
#endif

TScrollWindow::TScrollWindow()
#if USE_PANTHER_FEATURES
	: TWindow( CFSTR( "main" ), CFSTR( "ScrollView" ) )
#else
	: TWindow( 	kDocumentWindowClass, kWindowStandardDocumentAttributes |
				kWindowLiveResizeAttribute |
				kWindowCompositingAttribute, kInitialBounds )
#endif
{
	HIRect			bounds;
	HIViewRef		contentView;
	CGImageRef		testImage;
	HILayoutInfo	layout;
	
	const EventTypeSpec	events[] =
		{
			{ kEventClassWindow, kEventWindowGetMinimumSize },
			{ kEventClassWindow, kEventWindowGetMaximumSize },
			{ kEventClassWindow, kEventWindowBoundsChanging },
			{ kEventClassWindow, kEventWindowBoundsChanged }
		};

#if !USE_PANTHER_FEATURES
	const EventTypeSpec	contentEvents[] =
		{
			{ kEventClassControl, kEventControlBoundsChanged }
		};	
#endif

	SetWindowTitleWithCFString( GetWindowRef(), CFSTR( "ScrollView" ) );

	RegisterForEvents( GetEventTypeCount( events ), events );

	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kHIViewWindowContentID, &contentView );

#if USE_PANTHER_FEATURES
	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kScrollViewID, &fScrollView );
#else
	HIScrollViewCreate( kHIScrollViewOptionsVertScroll | kHIScrollViewOptionsHorizScroll | kHIScrollViewOptionsAllowGrow, &fScrollView );

	HIScrollViewSetScrollBarAutoHide( fScrollView, true );
#endif

	HIViewAddSubview( contentView, fScrollView );
	HIViewGetBounds( contentView, &bounds );
	HIViewSetFrame( fScrollView, &bounds );
	HIViewSetVisible( fScrollView, true );

#if USE_PANTHER_FEATURES
	layout.version = 0;
	HIViewGetLayoutInfo( fScrollView, &layout );
	
	layout.scale.x.toView = NULL;
	layout.scale.x.kind = kHILayoutScaleAbsolute;
	layout.scale.x.ratio = 1.0;
	layout.scale.y.toView = NULL;
	layout.scale.y.kind = kHILayoutScaleAbsolute;
	layout.scale.y.ratio = 1.0;
	
	HIViewSetLayoutInfo( fScrollView, &layout );
#else
	InstallEventHandler( HIObjectGetEventTarget( (HIObjectRef)contentView ), 
						HandleContentBoundsChanged, GetEventTypeCount( contentEvents ),
						contentEvents, this, NULL );
#endif

	testImage = GetTestImage();
#if USE_PANTHER_FEATURES
	HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kImageViewID, &fImageView );
	HIImageViewSetImage( fImageView, testImage );
#else
	HIImageViewCreate( testImage, &fImageView );
	HIViewSetVisible( fImageView, true );
	HIViewAddSubview( fScrollView, fImageView );
#endif
	CGImageRelease( testImage );	
	
	TScrollableView::Create( &fScrollableView ); // hold onto this for now.

	RepositionWindow( GetWindowRef(), NULL, kWindowCascadeOnMainScreen );
	
	fLatentWidth = fLatentHeight = 0;

    SetUserFocusWindow( GetWindowRef() );
}

TScrollWindow::~TScrollWindow()
{
	if ( HIViewGetSuperview( fImageView ) == NULL )
		CFRelease( fImageView );
	
	if ( HIViewGetSuperview( fScrollableView ) == NULL )
		CFRelease( fScrollableView );
}

OSStatus
TScrollWindow::HandleEvent( EventHandlerCallRef inCallRef, TCarbonEvent& inEvent )
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
						
						HIViewGetSizeConstraints( fScrollView, &min, NULL );
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
						
						HIViewGetSizeConstraints( fScrollView, NULL, &max );
						size.h = (SInt16)max.width;
						size.v = (SInt16)max.height;
						
						inEvent.SetParameter( kEventParamDimensions, size );
						result = noErr;
					}
					break;
					
				case kEventWindowBoundsChanging:
					{
						UInt32		attrs;
					
						// This entire handler allows us to reshape the window when we hit
						// the bottom of the screen. It's just a neat demonstration, but in
						// some apps, it might not be a bad feature!
						
						inEvent.GetParameter( kEventParamAttributes, &attrs );
						
						if ( attrs & kWindowBoundsChangeUserDrag )
						{
							Rect		bounds;
							Rect		metrics;
							Rect		limits;
							
							inEvent.GetParameter( kEventParamCurrentBounds, &bounds );
							
							GetAvailableWindowPositioningBounds( GetMainDevice(), &limits );

							GetWindowStructureWidths( GetWindowRef(), &metrics );
							bounds.top -= metrics.top;
							bounds.left -= metrics.left;
							bounds.bottom += metrics.bottom;
							bounds.right += metrics.right;
							
							if ( fLatentWidth )
								bounds.right = bounds.left + fLatentWidth;
							
							if ( bounds.right > limits.right )
							{
								if ( fLatentWidth == 0 )
									fLatentWidth = (bounds.right - bounds.left);

								bounds.right = limits.right;
							}
							else
							{
								fLatentWidth = 0;
							}
							
							if ( fLatentHeight )
								bounds.bottom = bounds.top + fLatentHeight;

							if ( bounds.bottom > limits.bottom )
							{
								if ( fLatentHeight == 0 )
									fLatentHeight = (bounds.bottom - bounds.top);

								bounds.bottom = limits.bottom;
							}
							else
							{
								fLatentHeight = 0;
							}
							
							bounds.top += metrics.top;
							bounds.left += metrics.left;
							bounds.bottom -= metrics.bottom;
							bounds.right -= metrics.right;

							inEvent.SetParameter( kEventParamCurrentBounds, bounds );
							result = noErr;
						}						
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
TScrollWindow::UpdateCommandStatus( const HICommand& command )
{
	Boolean		handled = false;
	
	switch ( command.commandID )
	{
		case 'SCAL':
		case 'SMAL':
		case 'IMAG':
		case 'SWAP':
			EnableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			handled = true;
			break;

#if USE_PANTHER_FEATURES		
		case 'PGUP':
			if ( HIScrollViewCanNavigate( fScrollView, kHIScrollViewPageUp ) )
				EnableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			else
				DisableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			handled = true;
			break;

		case 'PGDN':
			if ( HIScrollViewCanNavigate( fScrollView, kHIScrollViewPageDown ) )
				EnableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			else
				DisableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			handled = true;
			break;

		case 'PGLT':
			if ( HIScrollViewCanNavigate( fScrollView, kHIScrollViewPageLeft ) )
				EnableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			else
				DisableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			handled = true;
			break;

		case 'PGRT':
			if ( HIScrollViewCanNavigate( fScrollView, kHIScrollViewPageRight ) )
				EnableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			else
				DisableMenuItem( command.menu.menuRef, command.menu.menuItemIndex );
			handled = true;
			break;
#endif

		default:
			handled = TWindow::UpdateCommandStatus( command );
			break;
	}
	
	return handled;
}

Boolean
TScrollWindow::HandleCommand( const HICommand& command )
{
	Boolean		handled = false;
	
	switch ( command.commandID )
	{
		case 'SCAL':
			{
				HIImageViewSetScaleToFit( fImageView, !HIImageViewGetScaleToFit( fImageView ) );
			}
			handled = true;
			break;

		case 'SMAL':
			{
				ControlSize		size;
				
				GetControlData( fScrollView, 0, kControlSizeTag, sizeof( size ), &size, NULL );
				if ( size == kControlSizeNormal )
					size = kControlSizeSmall;
				else
					size = kControlSizeNormal;
				SetControlData( fScrollView, 0, kControlSizeTag, sizeof( size ), &size );
			}
			handled = true;
			break;

		case 'IMAG':
			{
				HIViewRef		temp;
				HIRect			frame;
				float 			alpha = 0.5;
				Rect			bounds;
				SInt16			baseLine;
				HIViewRef		contentView;
				CGImageRef		logo = GetLogoImage();
				
				HIViewFindByID( HIViewGetRoot( GetWindowRef() ), kHIViewWindowContentID, &contentView );
				HIImageViewCreate( logo, &temp );
				CGImageRelease( logo );
				GetBestControlRect( temp, &bounds, &baseLine );
				frame.origin.x = frame.origin.y = 20;
				frame.size.width = bounds.right - bounds.left;
				frame.size.height = bounds.bottom - bounds.top;
				HIViewSetFrame( temp, &frame );
				HIImageViewSetAlpha( temp, alpha );
				HIImageViewSetOpaque( temp, false );
				HIViewSetVisible( temp, true );
				HIViewAddSubview( contentView, temp );				
			}

			handled = true;
			break;
				
		case 'SWAP':
			{
				if ( HIViewGetSuperview( fImageView ) != NULL )
				{
					HIViewRemoveFromSuperview( fImageView );
					HIViewSetVisible( fScrollableView, true );
					HIViewAddSubview( fScrollView, fScrollableView );
				}
				else
				{
					HIViewSetVisible( fScrollableView, false );
					HIViewRemoveFromSuperview( fScrollableView );
					HIViewAddSubview( fScrollView, fImageView );
				}
			}
			break;

#if USE_PANTHER_FEATURES		
		case 'PGUP':
			HIScrollViewNavigate( fScrollView, kHIScrollViewPageUp );
			handled = true;
			break;

		case 'PGDN':
			HIScrollViewNavigate( fScrollView, kHIScrollViewPageDown );
			handled = true;
			break;

		case 'PGLT':
			HIScrollViewNavigate( fScrollView, kHIScrollViewPageLeft );
			handled = true;
			break;

		case 'PGRT':
			HIScrollViewNavigate( fScrollView, kHIScrollViewPageRight );
			handled = true;
			break;
#endif
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

static CGImageRef
GetLogoImage()
{
	CGDataProviderRef	provider;
	CFBundleRef 		appBundle = ::CFBundleGetMainBundle();
	CGImageRef			image = NULL;
	
	if ( appBundle )
	{
		CFURLRef url = ::CFBundleCopyResourceURL( appBundle, CFSTR( "logo.png" ), NULL, NULL );
		
		provider = CGDataProviderCreateWithURL( url );

		image = CGImageCreateWithPNGDataProvider( provider, NULL, false,  kCGRenderingIntentDefault );
		
		CGDataProviderRelease( provider );
		CFRelease( url );
	}
	
	return image;
}

OSStatus
TScrollWindow::HandleContentBoundsChanged( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	HIRect			bounds;
	UInt32			attrs;
	TScrollWindow*	window = (TScrollWindow*)inUserData;
	
	GetEventParameter( inEvent, kEventParamAttributes, typeUInt32, NULL, sizeof( UInt32 ), NULL, &attrs );
	
	if ( attrs & kControlBoundsChangeSizeChanged )
	{
		HIViewRef		contentView;
		HIViewFindByID( HIViewGetRoot( window->GetWindowRef() ), kHIViewWindowContentID, &contentView );
		HIViewGetBounds( contentView, &bounds );
		HIViewSetFrame( window->fScrollView, &bounds );
	}
	
	return noErr;
}

