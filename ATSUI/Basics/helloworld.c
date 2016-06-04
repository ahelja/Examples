	/*

File: helloworld.c

Abstract: Contains an extremely simple case for ATSUI drawing in a
window. 

Version: <1.0>

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2004 Apple Computer, Inc., All Rights Reserved

*/

#include "helloworld.h"

// Constants
#define kHelloWorldMargin 25
#define kHelloWorldFontName "Times Roman"
#define kHelloWorldFontSize 36

// This shows a simple example of how to draw text in a window using
// ATSUI. This may look like a lot of code just to draw one string,
// but the benefit of doing all this set up in ATSUI is that it takes
// very little additional code to do something much fancier. The
// other examples will make this more clear.
//
// The concept of setting a "triple" of tag, size, value when passing
// settings to ATSUI for styles and layouts is demonstrated here. This is
// a core concept of ATSUI. Nearly everything that is passed to ATSUI is
// done so as a triple. Note that even for very small objects, such as
// CGContextRefs, you still always pass a pointer to it in the "value"
// parameter, not the acutal value.
//
// The minimal case to draw in ATSUI is to create a layout and a style,
// then call ATSUIDrawText on the layout. The acutal string to be drawn
// is stored in an array of UniChar type (UTF-16) that is owned by you,
// the caller. ATSUI merely keeps a pointer to this array. The caller is
// responsible for the storage of the text buffer.
//
// Layouts and styles are meant to be persistent. Keep them around until
// you are completely done with them. It is much quicker to keep the
// layouts around and redraw them than to set everything up from scratch.
//
void DrawHelloWorldContents(WindowRef window)
{
	CFStringRef				string;
	UniChar					*text;
	UniCharCount			length;
	ATSUStyle				style;
	ATSUTextLayout			layout;
	ATSUFontID				font;
	Fixed					pointSize;
    ATSUAttributeTag		tags[2];
    ByteCount				sizes[2];
    ATSUAttributeValuePtr	values[2];
	CGContextRef			cgContext;
	float					x, y, cgY;
    GrafPtr					port, savedPort;
	Rect					portBounds;

    // Set up the graphics port
	port = GetWindowPort(window);
    GetPort(&savedPort);
    SetPort(port);
    GetPortBounds(port, &portBounds);
    EraseRect(&portBounds);

	// Create a style object. This is one of two objects necessary to draw using ATSUI.
	// (The layout is the other.)
	verify_noerr( ATSUCreateStyle(&style) );

	// Now we are going to set a few things in the style.
	// This is not strictly necessary, as the style comes
	// with some sane defaults after being created, but
	// it is useful to demonstrate.
	
    // Look up the font we are going to use, and set it in the style object, using
	// the aforementioned "triple" (tag, size, value) semantics. This is how almost
	// all settings in ATSUI are applied.
	verify_noerr( ATSUFindFontFromName(kHelloWorldFontName, strlen(kHelloWorldFontName), kFontFullName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, &font) );
    tags[0] = kATSUFontTag;
    sizes[0] = sizeof(ATSUFontID);
    values[0] = &font;
    verify_noerr( ATSUSetAttributes(style, 1, tags, sizes, values) );

	// Notice below the point size is set as Fixed, not an int or a float.
	// For historical reasons, most values in ATSUI are Fixed or Fract, not int or float.
	// See the header FixMath.h in the CarbonCore framework for conversion macros.

    // Set the point size, also using a triple. You can actually set multiple triples at once,
	// since the tag, size, and value parameters are arrays. Other examples do this, such as
	// the vertical text example.
	// 
	pointSize = Long2Fix(kHelloWorldFontSize);
	tags[0] = kATSUSizeTag;
    sizes[0] = sizeof(Fixed);
    values[0] = &pointSize;
    verify_noerr( ATSUSetAttributes(style, 1, tags, sizes, values) );
	
	// Now we create the second of two objects necessary to draw text using ATSUI, the layout.
	// You can specify a pointer to the text buffer at layout creation time, or later using
	// the routine ATSUSetTextPointerLocation(). Below, we do it after layout creation time.
	verify_noerr( ATSUCreateTextLayout(&layout) );

	// Before assigning text to the layout, we must first convert the string we plan to draw
	// from a CFStringRef into an array of UniChar.
	string = CFStringCreateWithCString(NULL, "Hello World!", kCFStringEncodingASCII);

    // Extract the raw Unicode from the CFString, then dispose of the CFString
    length = CFStringGetLength(string);
    text = (UniChar *)malloc(length * sizeof(UniChar));
    CFStringGetCharacters(string, CFRangeMake(0, length), text);
	CFRelease(string);

    // Attach the resulting UTF-16 Unicode text to the layout
    verify_noerr( ATSUSetTextPointerLocation(layout, text, kATSUFromTextBeginning, kATSUToTextEnd, length) );
	
	// Now we tie the two necessary objects, the layout and the style, together
	verify_noerr( ATSUSetRunStyle(layout, style, kATSUFromTextBeginning, kATSUToTextEnd) );

	// Before we draw, there is one last thing we must do. Set up a CGContext.
	//
	// On OS 9, ATSUI would draw using only Quickdraw. With Mac OS X, it can draw with
	// either Quickdraw or CoreGraphics. Quickdraw is now being de-emphasized in favor
	// of CoreGraphics, to the point where ATSUI will default to drawing using CoreGraphics.
	// By default ATSUI will work by using the cannonical CGContext that comes with every GrafPort.
	// However, it is preferred that clients set up their own CGContext and pass it to ATSUI
	// before drawing. This not only gives the client more control, it offers the best performance.
	//
	QDBeginCGContext(port, &cgContext);
	tags[0] = kATSUCGContextTag;
	sizes[0] = sizeof(CGContextRef);
	values[0] = &cgContext;
	verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );
	
	// Now, finally, we are ready to draw.
	//
	// When drawing it is important to note the difference between QD and CG style coordinates.
	// For QD, the y coordinate starts at zero at the top of the window, and goes down. For CG,
	// it is just the opposite. Because we have set a CGContext in our layout, ATSUI will be
	// expecting CG style coordinates. Otherwise, it would be expecting QD style coordinates.
	// Also, remember ATSUI only accepts coordinates in Fixed, not float or int. In our example,
	// "x" and "y" are the coordinates in QD space. "cgY" contains the y coordinate in CG space.
	//
	x = kHelloWorldMargin; // leave a small left margin
	y = (portBounds.bottom - portBounds.top) / 2.0; // Center the text vertically
	cgY = (portBounds.bottom - portBounds.top) - y; // Subtract the y coordinate from the height of the
													// window to get the coordinate in CG-aware space.

	verify_noerr( ATSUDrawText(layout, kATSUFromTextBeginning, kATSUToTextEnd, X2Fix(x), X2Fix(cgY)) );
	
	// This is a one-shot window, so we are now ready to dispose of all our objects.
	// Normally, we would want to keep everything around in case we needed to redraw or change
	// the text at some point.
	
    // Tear down the CGContext
	CGContextFlush(cgContext);
	QDEndCGContext(port, &cgContext);

	// Deallocate string storage
	free(text);

	// Layout and style also need to be disposed
	verify_noerr( ATSUDisposeStyle(style) );
	verify_noerr( ATSUDisposeTextLayout(layout) );

    // Restore the graphics port
    SetPort(savedPort);
}
