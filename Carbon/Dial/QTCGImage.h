// =============================================================================
//	QTCGImage.h
// =============================================================================
//

#ifndef QTCGImage_H_
#define QTCGImage_H_

#include <Carbon/Carbon.h>

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//
OSStatus
CreateCGImageWithQT(
	CFDataRef			inData,
	CGImageRef*			outImage );
OSStatus
CreateCGImageWithQTFromResource(
	CFStringRef			inResourceName,
	CGImageRef*			outImage );
OSStatus
CreateCGImageWithQTFromFile(
	FSRef*				inFSRef,
	CGImageRef*			outImage );
OSStatus
CreateCGImageWithQTFromURL(
	CFURLRef			inURL,
	CGImageRef*			outImage );
OSStatus
CreateCGImageWithQTFromDrag(
	DragRef				inDrag,
	CGImageRef*			outImage );

#ifdef __cplusplus
}
#endif

#endif	// QTCGImage_H_
