// =============================================================================
//	QTCGImage.c
// =============================================================================
//

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

#include "QTCGImage.h"

// -----------------------------------------------------------------------------
//	macros
// -----------------------------------------------------------------------------
//
#define RECT_HEIGHT(R)	((R).bottom-(R).top)
#define RECT_WIDTH(R)	((R).right-(R).left)

#if !DEFINED_IN_TIGER
	#define kUTTypeFileURL  CFSTR( "public.file-url" )
	#define kUTTypeImage	CFSTR( "public.image" )
#endif

#define DEBUG_FLAVORS   0

// -----------------------------------------------------------------------------
//	local prototypes
// -----------------------------------------------------------------------------
//
OSStatus
CreateCGImageWithQTImporter(
	GraphicsImportComponent		inData,
	CGImageRef*					outImage );

// -----------------------------------------------------------------------------
//	GWorldImageBufferRelease
// -----------------------------------------------------------------------------
//
void
GWorldImageBufferRelease(
	void*						inInfo,
	const void*					inData,
	size_t						inSize )
{
	#pragma unused( inData, inSize )

	DisposePtr( (Ptr) inData );
}

// -----------------------------------------------------------------------------
//	CreateCGImageWithQTFromResource
// -----------------------------------------------------------------------------
//
OSStatus
CreateCGImageWithQTFromResource(
	CFStringRef			inName,
	CGImageRef*			outImage )
{
	OSStatus			err;
	CFURLRef			url;

	url = CFBundleCopyResourceURL( CFBundleGetMainBundle(), inName, NULL, NULL );
	require_action( url != NULL, CantGetURL, err = fnfErr );
	
	err = CreateCGImageWithQTFromURL( url, outImage );
	
	CFRelease( url );

CantGetURL:
	return err;
}

// -----------------------------------------------------------------------------
//	CreateCGImageWithQTFromURL
// -----------------------------------------------------------------------------
//
OSStatus
CreateCGImageWithQTFromURL(
	CFURLRef			inURL,
	CGImageRef*			outImage )
{
	OSStatus			err;
	FSRef				fileRef;

	require_action( CFURLGetFSRef( inURL, &fileRef ), CantFindFile, err = fnfErr );

	err = CreateCGImageWithQTFromFile( &fileRef, outImage );

CantFindFile:
	return err;
}

// -----------------------------------------------------------------------------
//	CreateCGImageWithQTImporter
// -----------------------------------------------------------------------------
//
OSStatus
CreateCGImageWithQTImporter(
	GraphicsImportComponent		inImporter,
	CGImageRef*					outImage )
{
	OSStatus					err;
	GWorldPtr					gWorld = NULL;
	Rect						bounds;
	CGDataProviderRef			provider;
	CGColorSpaceRef				colorspace;
	long						width;
	long						height;
	long						rowbytes;
	Ptr							dataPtr;

	err = GraphicsImportGetNaturalBounds( inImporter, &bounds );
	require_noerr( err, CantGetBounds );

	// Allocate the buffer
	width = RECT_WIDTH( bounds );
	height = RECT_HEIGHT( bounds );
	rowbytes = width * 4;
	dataPtr = NewPtr( height * rowbytes );
	require_action( dataPtr != NULL, CantAllocBuffer, err = memFullErr );
	
	err = NewGWorldFromPtr( &gWorld, 32, &bounds, NULL, NULL, 0, dataPtr, rowbytes );
	require_noerr( err, CantCreateGWorld );

	err = GraphicsImportSetGWorld( inImporter, gWorld, GetGWorldDevice( gWorld) );
	require_noerr( err, CantSetGWorld );

	err = GraphicsImportDraw( inImporter );
	require_noerr( err, CantDraw );
	
	provider = CGDataProviderCreateWithData( NULL, dataPtr, height * rowbytes,
			GWorldImageBufferRelease );
	require_action( provider != NULL, CantCreateProvider, err = memFullErr );

	colorspace = CGColorSpaceCreateDeviceRGB();
	require_action( colorspace != NULL, CantCreateColorSpace, err = memFullErr );
	
	*outImage = CGImageCreate( width, height, 8, 32, rowbytes, colorspace,
			kCGImageAlphaFirst, provider, NULL, false, kCGRenderingIntentDefault );
	require_action( *outImage != NULL, CantCreateImage, err = memFullErr );
	
CantCreateImage:
	CGColorSpaceRelease( colorspace );

CantCreateColorSpace:
	CGDataProviderRelease( provider );

CantCreateProvider:
CantDraw:
CantSetGWorld:
	if ( gWorld != NULL )
		DisposeGWorld( gWorld );

CantCreateGWorld:
CantAllocBuffer:
CantGetBounds:
	return err;
}

// -----------------------------------------------------------------------------
//	CreateCGImageWithQTFromFile
// -----------------------------------------------------------------------------
//
OSStatus
CreateCGImageWithQTFromFile(
	FSRef*						inFSRef,
	CGImageRef*					outImage )
{
	OSStatus					err;
	GraphicsImportComponent		importer;
	FSSpec						fileSpec;

	// Make an FSRef into an FSSpec
	err = FSGetCatalogInfo( inFSRef, kFSCatInfoNone, NULL, NULL, &fileSpec, NULL );
	require_noerr( err, CantMakeFSSpec );

	err = GetGraphicsImporterForFile( &fileSpec, &importer );
	require_noerr( err, CantGetImporter );
	
	err = CreateCGImageWithQTImporter( importer, outImage );

	// CloseComponent( importer );

CantGetImporter:
CantMakeFSSpec:
	return err;
}

// -----------------------------------------------------------------------------
//	CreateCGImageWithQT
// -----------------------------------------------------------------------------
//
OSStatus
CreateCGImageWithQT(
	CFDataRef			inData,
	CGImageRef*			outImage )
{
	OSStatus					err;
	GraphicsImportComponent		importer;
	PointerDataRef				pointerDataRef;
	
    pointerDataRef = (PointerDataRef) NewHandleClear( sizeof( PointerDataRefRecord ) );
	require_action( pointerDataRef != NULL, CantAlloc, err = memFullErr );
	(**pointerDataRef).data = (char*) CFDataGetBytePtr( inData );
	(**pointerDataRef).dataLength = CFDataGetLength( inData );

	err = GetGraphicsImporterForDataRef( (Handle) pointerDataRef, PointerDataHandlerSubType, &importer );
	require_noerr( err, CantGetImporter );
	
	err = CreateCGImageWithQTImporter( importer, outImage );

	CloseComponent( importer );

	DisposeHandle( (Handle) pointerDataRef );

CantGetImporter:
CantAlloc:
	return err;
}

// -----------------------------------------------------------------------------
//	CreateCGImageWithQT
// -----------------------------------------------------------------------------
//
OSStatus
CreateCGImageWithQTFromDrag(
	DragRef				inDrag,
	CGImageRef*			outImage )
{
	OSStatus			err;
	PasteboardRef		pasteboard;
	PasteboardItemID	item;
	CFDataRef			flavorData;
	CFIndex				flavorDataSize;
	CFURLRef			url;

	err = GetDragPasteboard( inDrag, &pasteboard );
	verify_noerr( err );

	err = PasteboardGetItemIdentifier( pasteboard, 1, &item );
	verify_noerr( err );

#if DEBUG_FLAVORS	// Pasteboard debug
	{
		CFArrayRef			flavors;
		CFIndex				flavorCount, i;

		err = PasteboardCopyItemFlavors( pasteboard, item, &flavors );
		verify_noerr( err );

		flavorCount = CFArrayGetCount( flavors );

		for ( i = 0; i < flavorCount; i++ )
		{
			CFStringRef flavor = (CFStringRef) CFArrayGetValueAtIndex( flavors, i );
			CFShow( flavor );
		}
		CFRelease( flavors );
	}
#endif

	// Try and get a pasteboard CFURL
	err = PasteboardCopyItemFlavorData( pasteboard, item, kUTTypeFileURL, &flavorData );
	if ( err == noErr )
	{
		flavorDataSize = CFDataGetLength( flavorData );
		verify( flavorDataSize > 0 );

		url = CFURLCreateWithBytes( NULL, CFDataGetBytePtr( flavorData ), flavorDataSize, kCFStringEncodingMacRoman, NULL );
		CFRelease( flavorData );
		require_action( url != NULL, CantLoadImage, err = paramErr );
		
		err = CreateCGImageWithQTFromURL( url, outImage );
		CFRelease( url );
		require_noerr( err, CantLoadImage );
	}
	else
	{
		// Otherwise, try and get a pasteboard item that conforms to kUTTypeImage
		CFArrayRef			flavors;
		CFIndex				flavorCount, i;
		CFStringRef			flavor = NULL;

		err = PasteboardCopyItemFlavors( pasteboard, item, &flavors );
		verify_noerr( err );

		flavorCount = CFArrayGetCount( flavors );

		err = paramErr;
		for ( i = 0; i < flavorCount; i++ )
		{
			flavor = (CFStringRef) CFArrayGetValueAtIndex( flavors, i );

			if ( UTTypeConformsTo( flavor, kUTTypeImage ) )
			{
				err = noErr;
				break;
			}
		}

		CFRelease( flavors );

		if ( err == noErr )
		{
		err = PasteboardCopyItemFlavorData( pasteboard, item, flavor, &flavorData );
			require_noerr( err, CantCopyFlavorData );

		err = CreateCGImageWithQT( flavorData, outImage );

			CFRelease( flavorData );
		}
	}

	// If none of that works, fall back on the Drag Manager
	if ( err != noErr )
	{
		UInt16		itemCount, i;

		err = CountDragItems( inDrag, &itemCount );
		require_noerr( err, CantGetDragItemCount );

		for ( i = 1; i <= itemCount; i++ )
		{
			DragItemRef	itemRef;
			UInt16		flavorCount, j;

			err = GetDragItemReferenceNumber( inDrag, i, &itemRef );
			require_noerr( err, CantGetDragItemReferenceNumber );

			err = CountDragItemFlavors( inDrag, itemRef, &flavorCount );
			require_noerr( err, CantGetDragItemFlavorCount );

			for( j = 1; j <= flavorCount; j++ )
			{
				FlavorType	flavor;
				Size		size;
				UInt8*		buffer;

				err = GetFlavorType( inDrag, itemRef, j, &flavor );
				require_noerr( err, CantGetDragFlavor );

#if DEBUG_FLAVORS // Drag Manager debug
				char		flavorString[] = "XXXX";
				*(UInt32*) flavorString = flavor;
				fprintf( stderr, "Drag flavor %s\n", flavorString );
#endif
				switch ( flavor )
				{
					case typeFileURL:
						// Drag is encoded CFURL data
						err = GetFlavorDataSize( inDrag, itemRef, flavor, &size );
						require_noerr( err, CantGetDragFlavorDataSize );

						// Allocate a buffer to hold the encoded CFURL
						buffer = malloc( size );
						require_action( buffer != NULL, CantAllocate, err = memFullErr );

						err = GetFlavorData( inDrag, itemRef, flavor, buffer, &size, 0 );
						require_noerr( err, CantGetDragFlavorData );

                    	url = CFURLCreateWithBytes( kCFAllocatorDefault, (UInt8*) buffer, size, kCFStringEncodingUTF8, NULL );
                    	require_action( url != NULL, CantCreateURL, err = memFullErr );

						err = CreateCGImageWithQTFromURL( url, outImage );
						CFRelease( url );
						require_noerr( err, CantLoadImage );

					CantCreateURL:
						free( buffer );
						break;

					case kDragFlavorTypeHFS:
						{
							HFSFlavor	hfsFlavor;
							FSRef		fileRef;

							err = GetFlavorDataSize( inDrag, itemRef, flavor, &size );
							require_noerr( err, CantGetDragFlavorDataSize );

							require_action( size == sizeof( HFSFlavor ), BadHFSFlavor, err = paramErr );

							err = GetFlavorData( inDrag, itemRef, flavor, &hfsFlavor, &size, 0 );
							require_noerr( err, CantGetDragFlavorData );

							err = FSpMakeFSRef( &hfsFlavor.fileSpec, &fileRef );
							require_noerr( err, CantMakeFSRef );

							err = CreateCGImageWithQTFromFile( &fileRef, outImage );

						CantMakeFSRef:
						BadHFSFlavor:
							;
						}
						break;
				}
			}
		}
	CantGetDragFlavorData:
	CantAllocate:
	CantGetDragFlavorDataSize:
	CantGetDragItemFlavorCount:
	CantGetDragFlavor:
	CantGetDragItemReferenceNumber:
	CantGetDragItemCount:
		;
	}

CantLoadImage:
CantCopyFlavorData:
	return err;
}
