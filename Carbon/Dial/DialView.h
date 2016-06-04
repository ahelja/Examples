// =============================================================================
//	DialView.h
// =============================================================================
//

#ifndef DialView_H_
#define DialView_H_

#include <Carbon/Carbon.h>

// If not creating the view programmatically via CreateDialView
// (i.e. From a nib) be sure to register the class before attempting
// to create the view or it will fail.

void DialViewRegister();

OSStatus CreateDialView(
	HIRect				inBounds,
	SInt32				inValue,
	SInt32				inMin,
	SInt32				inMax,
	UInt16				inNumTickMarks,
	HIViewRef*			outView );

OSStatus GetDialTickMarks(
	HIViewRef			inView,
	UInt16*				outNumTickMarks );
OSStatus SetDialTickMarks(
	HIViewRef			inView,
	UInt16				inNumTickMarks );

#endif // DialView_H_
