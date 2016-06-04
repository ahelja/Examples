/*
    File:		TScrollableView.cp
    
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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include "TScrollableView.h"

#define kClassID CFSTR( "com.apple.sample.scrollableview" )

static const EventTypeSpec kEvents[] =
{
	{ kEventClassScrollable, kEventScrollableGetInfo },
	{ kEventClassScrollable, kEventScrollableScrollTo }
};

const HIRect		kVirtualBounds = { { 0, 0 }, { 800, 800 } };

TScrollableView::TScrollableView( ControlRef inControl ) : TView( inControl )
{
	InstallEventHandler( GetEventTarget(), EventHandler,
		GetEventTypeCount( kEvents ), kEvents, this, &fHandler );

	fImageOrigin.x = fImageOrigin.y = 0;
}

TScrollableView::~TScrollableView()
{
	if ( fHandler )
		RemoveEventHandler( fHandler );
}

void
TScrollableView::RegisterClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		TView::RegisterSubclass( kClassID, Construct );
		sRegistered = true;
	}
}

OSStatus
TScrollableView::Create( ControlRef* outControl )
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	
	RegisterClass();

	err = HIObjectCreate( kClassID, event, (HIObjectRef*)outControl );
	
	ReleaseEvent( event );

	return err;
}


void
TScrollableView::Draw( RgnHandle limitRgn, CGContextRef inContext )
{
    CGAffineTransform 	m;
    int 				j, k;
    double 				a0, a1;
	HIRect				bounds;
	
	bounds = Bounds();
	
	// Paint everything white before drawing on top of it. Do this before
	// translating the coordinate system to the scroll position.
	CGContextSetRGBFillColor( inContext, 1, 1, 1, 1 );
	CGContextFillRect( inContext, bounds );
	
	CGContextTranslateCTM( inContext, -fImageOrigin.x, -fImageOrigin.y );

	// Draw fancy flower with bezier curves with code I don't fully understand.
	// Math is HARD!!

    m = CGAffineTransformIdentity;
    m = CGAffineTransformScale( m, kVirtualBounds.size.width / 2, kVirtualBounds.size.height / 2 );
	
    CGContextTranslateCTM( inContext, CGRectGetWidth( kVirtualBounds )/2,
			  CGRectGetHeight( kVirtualBounds )/2);
    CGContextConcatCTM(inContext, m);
    CGContextTranslateCTM( inContext, -0.5, -0.5 );

    CGContextSaveGState( inContext );
    CGContextAddRect( inContext, CGRectMake(0, 0, 1, 1) );
    m = CGContextGetCTM( inContext );
    CGContextConcatCTM( inContext, CGAffineTransformInvert( m ) );
    CGContextStrokePath( inContext );
    CGContextRestoreGState( inContext );

    CGContextTranslateCTM( inContext, 0.5, 0.5 );
    CGContextScaleCTM( inContext, 0.25, 0.25 );
    for ( k = 0; k < 20; k++ )
	{
		CGContextSetRGBFillColor( inContext, k/19.0, k/30.0, 0, 1 );
		CGContextMoveToPoint( inContext, 1, 0 );
		for ( j = 0; j < 12; j++ )
		{
			a0 = 2 * pi * (j + 0.5) / 12;
			a1 = 2 * pi * (j + 1.0) / 12;
			CGContextAddCurveToPoint( inContext, 2*cos(a0), 2*sin(a0),
						2*cos(a0), 2*sin(a0),
						cos(a1), sin(a1));
		}
		
		CGContextClosePath( inContext );
		CGContextFillPath( inContext );
		CGContextScaleCTM( inContext, 0.9, 0.9 );
    }
}

ControlPartCode
TScrollableView::HitTest( const HIPoint& where )
{
	if ( CGRectContainsPoint( Bounds(), where ) )
		return 1;
	else
		return kControlNoPart;
}
		
OSStatus
TScrollableView::Construct( ControlRef inControl, TView** outView )
{
	*outView = new TScrollableView( inControl );
	
	return noErr;
}

ControlKind
TScrollableView::GetKind()
{
	const ControlKind kMyKind = { 'Samp', 'Sble' };
	
	return kMyKind;
}

OSStatus
TScrollableView::GetSizeConstraints( HISize* outMin, HISize* outMax )
{
	outMin->height = 0;
	outMin->width = 0;
	outMax->height = kVirtualBounds.size.height;
	outMax->width = kVirtualBounds.size.width;

	return noErr;
}

OSStatus
TScrollableView::EventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	TScrollableView*		view = (TScrollableView*)inUserData;
	OSStatus				err = eventNotHandledErr;
	TCarbonEvent			event( inEvent );
	
	switch ( event.GetClass() )
	{
		case kEventClassScrollable:
			switch( event.GetKind() )
			{
				case kEventScrollableGetInfo:
					{
						event.SetParameter( kEventParamImageSize, view->ImageSize() );
						event.SetParameter( kEventParamViewSize, view->ViewSize() );
						event.SetParameter( kEventParamOrigin, view->ImageOrigin() );
						event.SetParameter( kEventParamLineSize, view->LineSize() );
						
						err = noErr;
					}
					break;
				
				case kEventScrollableScrollTo:
					{
						HIPoint		location;
						
						event.GetParameter( kEventParamOrigin, &location );

						view->ScrollTo( location );
						err = noErr;
					}
					break;
			
		}
	}
	
	return err;
}

void
TScrollableView::ScrollTo( const HIPoint& where )
{
	float		dX, dY;
	
	dX = fImageOrigin.x - where.x;
	dY = fImageOrigin.y - where.y;
	
	fImageOrigin = where;

	HIViewScrollRect( GetViewRef(), NULL, dX, dY );
}

HISize
TScrollableView::ImageSize()
{	
	HISize		size;
	
	size.width = kVirtualBounds.size.width;
	size.height = kVirtualBounds.size.height;
	
	return size;
}

HIPoint
TScrollableView::ImageOrigin()
{	
	return fImageOrigin;
}

HISize
TScrollableView::ViewSize()
{
	return Bounds().size;
}

HISize
TScrollableView::LineSize()
{
	HISize		size = { 10, 10 };

	return size;
}

OSStatus
TScrollableView::GetRegion( ControlPartCode inPart, RgnHandle outRgn )
{
	OSStatus		err = noErr;

	switch ( inPart )
	{
		case -3: //kControlOpaqueMetaPart: // opaque region
			{
				Rect		bounds;
				HIRect		temp = Bounds();
				
				bounds.top = (SInt16)CGRectGetMinY( temp );
				bounds.left = (SInt16)CGRectGetMinX( temp );
				bounds.bottom = (SInt16)CGRectGetMaxY( temp );
				bounds.right = (SInt16)CGRectGetMaxX( temp );

				RectRgn( outRgn, &bounds );
			}
			break;
		
		default:
			err = TView::GetRegion( inPart, outRgn );
			break;
	}
	
	return err;
}
