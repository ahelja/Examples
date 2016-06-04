// =============================================================================
//	DialView.cp
// =============================================================================
//

#include <Carbon/Carbon.h>

#include "TView.h"
#include "DialView.h"

#include "QTCGImage.h"
#include <math.h>

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
#define kTViewClassID CFSTR( "com.apple.sample.dialview" )

enum
{
	kImageDimpleDisabled = 0,
	kImageDimpleNormal,
	kImageDimpleSelect,
	kImageDialDisabled,
	kImageDialNormal,
	kImageDialSelect,

	// This has to always be last!
	kImageCount,
};

const float				kDegree				= M_PI/180;
const float				kDialRadius			= 16;
const float				kDimpleInset		= 4;
const ControlPartCode	kControlDialPart	= 1;
const float				kTickOutset			= 6;
const float				kTickHeight			= 4;

// -----------------------------------------------------------------------------
//	TDialView declaration
// -----------------------------------------------------------------------------
//
class TDialView
	: public TView
{
public:
	static OSStatus			Create(
								const HIRect*		inBounds,
								SInt32				inValue,
								SInt32				inMin,
								SInt32				inMax,
								UInt16				inNumTickMarks,
								HIViewRef*			outView );
	static void 			RegisterClass();

protected:
	// Constructor/Destructor
							TDialView(
								HIViewRef			inView );
	virtual					~TDialView();

	virtual void			Draw(
								RgnHandle			inLimitRgn,
								CGContextRef		inContext );
	virtual OSStatus		GetData(
								OSType				inTag,
								ControlPartCode		inPart,
								Size				inSize,
								Size*				outSize,
								void*				inPtr );
	virtual ControlKind		GetKind();
	virtual OSStatus		GetRegion(
								ControlPartCode		inPart,
								RgnHandle			outRgn );
	virtual ControlPartCode	HitTest(
								const HIPoint&		inWhere );
	virtual OSStatus		Initialize(
								TCarbonEvent&		inEvent );
	virtual OSStatus		SetData(
								OSType				inTag,
								ControlPartCode		inPart,
								Size				inSize,
								const void*			inPtr );
	virtual OSStatus		Track(
								TCarbonEvent&		inEvent,
								ControlPartCode*	outPart );

private:
	float					AngleFromValue();
	static OSStatus			Construct(
								HIObjectRef			inObjectRef,
								TObject**			outObject );
	CGImageRef				FetchStructureImage(
								ControlPartCode		inPart,
								UInt32				inBaseImageID );

	static CGImageRef*		fStructureImages;
	static UInt32			fStructureImageClientRefCount;
	
	UInt16					fTickMarkCount;
};

// -----------------------------------------------------------------------------
//	statics
// -----------------------------------------------------------------------------
//
CGImageRef*	TDialView::fStructureImages = NULL;
UInt32		TDialView::fStructureImageClientRefCount = 0;

// -----------------------------------------------------------------------------
//	local prototypes
// -----------------------------------------------------------------------------
//
HIPoint RotatePoint(
	HIPoint				inPoint,
	HIPoint				inRotationCenter,
	float				inAngle );
void RotateLine(
	HIPoint*			inStart,
	HIPoint*			inEnd,
	const HIPoint*		inRotationCenter,
	float				inAngle );

// -----------------------------------------------------------------------------
//	TDialView constructor
// -----------------------------------------------------------------------------
//
TDialView::TDialView(
	HIViewRef			inView )
	: TView( inView )
{
	ChangeAutoInvalidateFlags( kAutoInvalidateOnValueChange
		| kAutoInvalidateOnHilite | kAutoInvalidateOnActivate, 0 );
}

// -----------------------------------------------------------------------------
//	TDialView destructor
// -----------------------------------------------------------------------------
//
TDialView::~TDialView()
{
	UInt32				i;
	
	// Without this instance, there is one less image client
	fStructureImageClientRefCount--;
	
	// If there are no image clients, the images can be released
	if ( fStructureImageClientRefCount == 0 && fStructureImages != NULL )
	{
		for ( i = 0; i < kImageCount; i++ )
			CGImageRelease( fStructureImages[ i ] );
		
		delete fStructureImages;

		// Reset the static fStructureImages ptr so it can be reinitialized if neccessary
		fStructureImages = NULL;
	}
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TDialView::GetKind()
{
	const ControlKind kMyKind = { 'TDia', 'TDia' };
	
	return kMyKind;
}

//-----------------------------------------------------------------------------------
//	Create
//-----------------------------------------------------------------------------------
//
OSStatus TDialView::Create(
	const HIRect*		inBounds,
	SInt32				inValue,
	SInt32				inMin,
	SInt32				inMax,
	UInt16				inNumTickMarks,
	HIViewRef*			outView )
{
#pragma unused( inNumTickMarks )
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	
	// Register this class
	RegisterClass();

	// Make a new instantiation of this class
	err = HIObjectCreate( kTViewClassID, event, (HIObjectRef*) outView );
	
	ReleaseEvent( event );

	if ( err == noErr )
	{
		SetControl32BitMinimum( *outView, inMin );
		SetControl32BitMaximum( *outView, inMax );
		SetControl32BitValue( *outView, inValue );

		err = HIViewSetFrame( *outView, inBounds );
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	RegisterClass
//-----------------------------------------------------------------------------------
//	Register this class with the HIObject registry.
//
//	This API can be called multiple times, but will only register once.
//
void TDialView::RegisterClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		TView::RegisterSubclass( kTViewClassID, Construct );
		sRegistered = true;
	}
}

//-----------------------------------------------------------------------------------
//	Construct
//-----------------------------------------------------------------------------------
//
OSStatus TDialView::Construct(
	HIObjectRef			inObject,
	TObject**			outObject )
{
	*outObject = new TDialView( (HIViewRef) inObject );
	
	return noErr;
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	setting up the images.
//
OSStatus TDialView::Initialize(
	TCarbonEvent&		inEvent )
{
	OSStatus			err;
	
	fTickMarkCount = 0;
	
	err = TView::Initialize( inEvent );
	require_noerr( err, CantInitializeParent );

	// Load the images if they aren't already loaded
	if ( fStructureImages == NULL )
	{
		CFStringRef			imageNames[ kImageCount ] = {
								CFSTR( "rotate_dimple_disabled.tiff" ),
								CFSTR( "rotate_dimple_normal.tiff" ),
								CFSTR( "rotate_dimple_select.tiff" ),
								CFSTR( "rotate_knob_disabled.tiff" ),
								CFSTR( "rotate_knob_normal.tiff" ),
								CFSTR( "rotate_knob_select.tiff" ) };

		CFURLRef			url;
		FSRef				fileRef;
		int					i;

		fStructureImages = new CGImageRef[ kImageCount ];
		require_action( fStructureImages != NULL, CantMakeImageArray, err = memFullErr );
		
		// Load up the art work
		for ( i = 0; i < kImageCount; i++ )
		{
			url = CFBundleCopyResourceURL( CFBundleGetMainBundle(), imageNames[ i ], NULL, NULL );
			require_action( url != NULL, CantGetURL, err = paramErr );
			
			require_action( CFURLGetFSRef( url, &fileRef ), CantFindFile, err = fnfErr );
			
			err = CreateCGImageWithQTFromFile( &fileRef, &fStructureImages[ i ] );
			require_noerr( err, CantMakeImage );
			require_action( fStructureImages[ i ] != NULL, CantMakeImage, err = memFullErr );
		
			CFRelease( url );
		}
	}
	
	// Add 1 to the client count for these images
	fStructureImageClientRefCount++;
	
CantMakeImage:
CantMakeImageArray:
CantFindFile:
CantGetURL:
CantInitializeParent:
	return err;
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	Here's the fun stuff.
//
void TDialView::Draw(
	RgnHandle			inLimitRgn,
	CGContextRef		inContext )
{
#pragma unused( inLimitRgn )
	TRect				bounds( Bounds() );
	HIPoint				dialCenter;
	HIPoint				dimpleCenter;
	CGImageRef			dial = fStructureImages[ 5 ];
	CGImageRef			dimple = fStructureImages[ 5 ];
	
	switch ( GetControlHilite( GetViewRef() ) )
	{
		case kControlNoPart:
			dial = fStructureImages[ kImageDialNormal ];
			dimple = fStructureImages[ kImageDimpleNormal ];
			break;
		case kControlDisabledPart:
		case kControlInactivePart:
			dial = fStructureImages[ kImageDialDisabled ];
			dimple = fStructureImages[ kImageDimpleDisabled ];
			break;
		default:
			dial = fStructureImages[ kImageDialSelect ];
			dimple = fStructureImages[ kImageDimpleSelect ];
			break;
	}

	dialCenter = bounds.Center();
	bounds.SetAroundCenter( dialCenter.x, dialCenter.y, CGImageGetWidth( dial ),
			CGImageGetHeight( dial ) );
	HIViewDrawCGImage( inContext, &bounds, dial );
	dimpleCenter = dialCenter;
	dialCenter.y -= 1;	// because of the shadow
	dimpleCenter.y -= kDialRadius - kDimpleInset;
	dimpleCenter = RotatePoint( dimpleCenter, dialCenter, AngleFromValue() * kDegree );
	bounds.SetAroundCenter( dimpleCenter.x, dimpleCenter.y, CGImageGetWidth( dimple ),
			CGImageGetHeight( dimple ) );
	HIViewDrawCGImage( inContext, &bounds, dimple );
	
	// If tick marks are requested, draw them
	if ( fTickMarkCount > 0 )
	{
		CGPoint		start, end;
		UInt16		i;
		
		start = end = CGPointMake( dialCenter.x, dialCenter.y - kDialRadius - kTickOutset );
		end.y += kTickHeight;

		for ( i = 0; i < fTickMarkCount; i++ )
		{
			CGContextMoveToPoint( inContext, start.x, start.y );
			CGContextAddLineToPoint( inContext, end.x, end.y );
			RotateLine( &start, &end, &dialCenter, 360.0/fTickMarkCount * kDegree );
		}
		CGContextStrokePath( inContext );
	}
}

//-----------------------------------------------------------------------------------
//	HitTest
//-----------------------------------------------------------------------------------
//	Check to see if a point hits the view
//
ControlPartCode TDialView::HitTest(
	const HIPoint&		inWhere )
{
	ControlPartCode		part;
	TRect				bounds( Bounds() );
	float				deltaX = inWhere.x - bounds.CenterX();
	float				deltaY = inWhere.y - bounds.CenterY();

	// Is the mouse in the view?
	if ( deltaX * deltaX + deltaY * deltaY <= kDialRadius * kDialRadius )
		part = kControlDialPart;
	else
		part = kControlNoPart;

	return part;
}

//-----------------------------------------------------------------------------------
//	GetRegion
//-----------------------------------------------------------------------------------
//
OSStatus TDialView::GetRegion(
	ControlPartCode		inPart,
	RgnHandle			outRgn )
{
	OSStatus			err = noErr;
	TRect				bounds;
	Rect				qdBounds;
	
	if ( inPart == kControlContentMetaPart
			|| inPart == kControlStructureMetaPart
			/* || inPart == kControlOpaqueRegionMetaPart */ )
	{
		bounds = Bounds();
		qdBounds = bounds;
	
		RectRgn( outRgn, &qdBounds );
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	FetchStructureImage
//-----------------------------------------------------------------------------------
//
CGImageRef TDialView::FetchStructureImage(
	ControlPartCode		inPart,
	UInt32				inBaseImageID )
{
	UInt32				offset = 0;
		
	return fStructureImages[ inBaseImageID + offset ];
}

//-----------------------------------------------------------------------------------
//	AngleFromValue
//-----------------------------------------------------------------------------------
//
float TDialView::AngleFromValue()
{
	float		range = GetMaximum() - GetMinimum() + 1;
	float		angle;
	
	if ( range != 0 )
		angle = (float) GetValue() / range * 360;
	else
		angle = 0;

	return angle;
};

//-----------------------------------------------------------------------------------
//	Track
//-----------------------------------------------------------------------------------
//
OSStatus TDialView::Track(
	TCarbonEvent&			inEvent,
	ControlPartCode*		outPart )
{
	OSStatus				err;
	HIPoint					where;
	Point					lastWhere;
	Point					qdWhere;
	ControlPartCode			part;
	TRect					bounds( Bounds() );
	HIPoint					center;
	float					x,y;
	float					angle;

	center = bounds.Center();
	
	// Extract the mouse location
	err = GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint,
			NULL, sizeof( HIPoint ), NULL, &where );
	require_noerr( err, ParameterMissing );

	x = where.x - center.x;
	y = where.y - center.y;
	
	// Check again to see if the mouse is in the view
	part = HitTest( where );
	
	if ( part != kControlNoPart )
	{
		MouseTrackingResult		mouseResult;

		Hilite( part );

		GetGlobalMouse( &lastWhere );
		qdWhere = lastWhere;
		
		do
		{
			// Do relative tracking to avoid coordinate spacing conversion
			x += qdWhere.h - lastWhere.h;
			y += qdWhere.v - lastWhere.v;
			
			angle = 180 - atan2( x, y ) / kDegree;

			SetValue( (SInt32) ( (float) (GetMaximum() - GetMinimum() + 1) * angle/360 ) );
			
			lastWhere = qdWhere;
			
			TrackMouseLocationWithOptions( NULL, 0, kEventDurationForever,
					&qdWhere, NULL, &mouseResult );

		} while ( mouseResult != kMouseTrackingMouseReleased );

		Hilite( kControlNoPart );
	}
	
	*outPart = part;

ParameterMissing:
	return err;
}

//-----------------------------------------------------------------------------------
//	RotatePoint
//-----------------------------------------------------------------------------------
//	Rotate inPoint about inRotationCenter by inAngle radians
//
HIPoint RotatePoint(
	HIPoint				inPoint,
	HIPoint				inRotationCenter,
	float				inAngle )	// radians
{
	HIPoint				point;
	CGAffineTransform	transform;
	
	// Move to origin
	point.x = inPoint.x - inRotationCenter.x;
	point.y = inPoint.y - inRotationCenter.y;
	
	// Rotate
	transform = CGAffineTransformMakeRotation( inAngle );
	point = CGPointApplyAffineTransform( point, transform );
	
	// Restore from origin
	point.x += inRotationCenter.x;
	point.y += inRotationCenter.y;
	
	return point;
}

//-----------------------------------------------------------------------------------
//	RotateLine
//-----------------------------------------------------------------------------------
//	Rotate line segment from inStart to inEnd about inRotationCenter by inAngle
//	radians
//
void RotateLine(
	HIPoint*			inStart,
	HIPoint*			inEnd,
	const HIPoint*		inRotationCenter,
	float				inAngle )
{
	*inStart = RotatePoint( *inStart, *inRotationCenter, inAngle );
	*inEnd = RotatePoint( *inEnd, *inRotationCenter, inAngle );
}

// -----------------------------------------------------------------------------
//	DialViewRegister
// -----------------------------------------------------------------------------
//	Public API
//
void DialViewRegister()
{
	TDialView::RegisterClass();
}

// -----------------------------------------------------------------------------
//	CreateDialView
// -----------------------------------------------------------------------------
//	Public API
//
OSStatus CreateDialView(
	const HIRect*		inBounds,
	SInt32				inValue,
	SInt32				inMin,
	SInt32				inMax,
	UInt16				inNumTickMarks,
	ControlActionUPP	inTrackingProc,
	HIViewRef*			outView )
{
	OSStatus			err;
	
	err = TDialView::Create( inBounds, inValue, inMin, inMax,
			inNumTickMarks, outView );
	require_noerr( err, CantCreateView );

CantCreateView:
	return err;
}

// -----------------------------------------------------------------------------
//	GetDialTickMarks
// -----------------------------------------------------------------------------
//
OSStatus GetDialTickMarks(
	HIViewRef			inView,
	UInt16*				outNumTickMarks )
{
	OSStatus			err;
	
	err = GetControlData( inView, 0, 'tick', sizeof( UInt16 ),
			outNumTickMarks, NULL );
	require_noerr( err, CantSetData );

CantSetData:
	return err;
}

// -----------------------------------------------------------------------------
//	SetDialTickMarks
// -----------------------------------------------------------------------------
//
OSStatus SetDialTickMarks(
	HIViewRef			inView,
	UInt16				inNumTickMarks )
{
	OSStatus			err;
	
	err = SetControlData( inView, 0, 'tick', sizeof( UInt16 ), &inNumTickMarks );
	require_noerr( err, CantSetData );

CantSetData:
	return err;
}

//-----------------------------------------------------------------------------------
//	GetData
//-----------------------------------------------------------------------------------
//
OSStatus TDialView::GetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	Size*				outSize,
	void*				inPtr )
{
	OSStatus			err = noErr;
	
	switch( inTag )
	{
		case 'tick':
			if ( inPtr )
			{
				if ( inSize != sizeof( UInt16 ) )
					err = errDataSizeMismatch;
				else
					( *(UInt16 *) inPtr ) = fTickMarkCount;
			}
			*outSize = sizeof( UInt16 );
			break;

		default:
			err = TView::GetData( inTag, inPart, inSize, outSize, inPtr );
			break;
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	SetData
//-----------------------------------------------------------------------------------
//
OSStatus TDialView::SetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	const void*			inPtr )
{
	OSStatus			err = noErr;
	
	switch( inTag )
	{
		case 'tick':
			if ( inPtr )
			{
				if ( inSize != sizeof( UInt16 ) )
					err = errDataSizeMismatch;
				else
					fTickMarkCount = ( *(UInt16 *) inPtr );
			}
			break;

		default:
			err = TView::SetData( inTag, inPart, inSize, inPtr );
			break;
	}
	
	return err;
}
