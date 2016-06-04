/*

File: fontsubstitution.c

Abstract: Demonstrates various font substitution modes for ATSUI. 

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

#include "filesupport.h"
#include "fontsubstitution.h"

// Constants
#define kFontSubstitutionMargin 25
#define kFontSubstitutionFontName "Times Roman"
#define kFontSubstitutionFontSize 16

// This example shows how to perform font susbtitution.
// ATSUI can automatically select new fonts on the fly when
// drawing text if the currently selected font does not
// have the necessary glyphs for the text being drawn.
//
void DrawFontSubstitutionContents(WindowRef window)
{
	CFStringRef				string;
	UniChar					*text;
	UniCharCount			length;
	ATSUStyle				style;
	ATSUTextLayout			layout;
	ATSUFontFallbacks		fallbacks;
	ATSUFontID				font;
	Fixed					pointSize;
    ATSUAttributeTag		tags[2];
    ByteCount				sizes[2];
    ATSUAttributeValuePtr	values[2];
	CGContextRef			cgContext;
	float					x, y, spacer, cgY;
    GrafPtr					port, savedPort;
	Rect					portBounds;

    // Set up the graphics port
	port = GetWindowPort(window);
    GetPort(&savedPort);
    SetPort(port);
    GetPortBounds(port, &portBounds);
    EraseRect(&portBounds);

	// First we create the style and configure some basic settings
	verify_noerr( ATSUCreateStyle(&style) );

	// Set up the font and point size
	verify_noerr( ATSUFindFontFromName(kFontSubstitutionFontName, strlen(kFontSubstitutionFontName), kFontFullName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, &font) );
    tags[0] = kATSUFontTag;
    sizes[0] = sizeof(ATSUFontID);
    values[0] = &font;

	pointSize = Long2Fix(kFontSubstitutionFontSize	);
	tags[1] = kATSUSizeTag;
    sizes[1] = sizeof(Fixed);
    values[1] = &pointSize;
    verify_noerr( ATSUSetAttributes(style, 2, tags, sizes, values) );
	
	// Now create a layout. Text and styles are tied together here (see below)
	verify_noerr( ATSUCreateTextLayout(&layout) );

	// Load the text we plan to use from the application bundle.
	// (The following helper function is in filesupport.c)
	string = LoadFileIntoCFString("FontSubstitutionExample_UTF16.txt");

    // Extract the raw Unicode from the CFString, then dispose of the CFString
    length = CFStringGetLength(string);
    text = (UniChar *)malloc(length * sizeof(UniChar));
    CFStringGetCharacters(string, CFRangeMake(0, length), text);
	CFRelease(string);

    // Attach the resulting UTF-16 Unicode text to the layout
    verify_noerr( ATSUSetTextPointerLocation(layout, text, kATSUFromTextBeginning, kATSUToTextEnd, length) );
	
	// Now we tie the two necessary objects, the layout and the style, together
	verify_noerr( ATSUSetRunStyle(layout, style, kATSUFromTextBeginning, kATSUToTextEnd) );

	// Set up a CGContext for drawing
	QDBeginCGContext(port, &cgContext);
	tags[0] = kATSUCGContextTag;
	sizes[0] = sizeof(CGContextRef);
	values[0] = &cgContext;
	verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );
	
	// Set up initial coordinates for drawing
	spacer = (portBounds.bottom - portBounds.top) / 4.0; // There are three examples in this window,
														 // so divide the window up vertially as needed
	x = kFontSubstitutionMargin; // leave a small left margin
	y = spacer; // start near the top, same about of margin
	cgY = (portBounds.bottom - portBounds.top) - y; // CG-aware y coordinate

	// Draw for the first time, with no font substitution. Any text that has no glyphs
	// available in the currently selected font will show up as boxes. This is generally
	// considered to be unfriendly UI. At the very least, last-resort font substitution
	// should always be used, see below.
	//
	verify_noerr( ATSUDrawText(layout, kATSUFromTextBeginning, kATSUToTextEnd, X2Fix(x), X2Fix(cgY)) );
	
	// Now we will create a font substitution object and attach it to the layout. Font
	// substitution (or "fallbacks") objects are reusable, thread-safe containers
	// for your font substitution settings. A single fallbacks object can be applied to
	// just one layout, or perhaps every layout in your application, or anywhere inbetween.
	// In general, you only need to create multiple fallbacks objects if you are going to be
	// using different types of fallbacks at the same time. Otherwise, one object should
	// suffice for all layouts in an application. To start with, we will create a font
	// substitution object and apply settings for the simplest kind of substitution -- last resort only.
	// This draws any text that cannot be drawn with the current font in the special last resort
	// font, which contains no specific glyphs, but single, symbolic glyphs which cover a large
	// range of Unicode code space.
	//
	verify_noerr( ATSUCreateFontFallbacks(&fallbacks) );
	verify_noerr( ATSUSetObjFontFallbacks(fallbacks, 0, NULL, kATSULastResortOnlyFallback) );

	// Now we apply the font fallbacks object to our layout. Remeber, a fallbacks object
	// can be used simultaneously in as many layouts as you like. You only need more than one
	// if you need to apply different settings in different places.
	tags[0] = kATSULineFontFallbacksTag;
	sizes[0] = sizeof(ATSUFontFallbacks);
	values[0] = &fallbacks;
	verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );

	// To get ATSUI to apply the fallback settings on-the-fly, use the following setting.
	// To apply the settings manually, call the function ATSUMatchFontsToText().
	verify_noerr( ATSUSetTransientFontMatching(layout, true) );
	
	// Let's see what the results look like now, with last resort substitution turned on
	y += spacer; // move down for the next case
	cgY = (portBounds.bottom - portBounds.top) - y; // CG-aware y coordinate
	verify_noerr( ATSUDrawText(layout, kATSUFromTextBeginning, kATSUToTextEnd, X2Fix(x), X2Fix(cgY)) );

	// And now let's draw it again with default font substitution turned on, which will search
	// all available fonts on the system for glyphs needed to draw all text in the layout.
	// For more information about the various types of font substitution available, including
	// how you can specify a specific list of fonts to be used as fallbacks, see the definition
	// of ATSUFontFallbackMethod in ATSUnicodeTypes.h.
	//
	verify_noerr( ATSUSetObjFontFallbacks(fallbacks, 0, NULL, kATSUDefaultFontFallbacks) );
	y += spacer; // move down for the next case
	cgY = (portBounds.bottom - portBounds.top) - y; // CG-aware y coordinate
	verify_noerr( ATSUDrawText(layout, kATSUFromTextBeginning, kATSUToTextEnd, X2Fix(x), X2Fix(cgY)) );

	// This is a one-shot window, so we are now ready to dispose of all our objects.
	// Normally, we would want to keep everything around in case we needed to redraw or change
	// the text at some point.
	
    // Tear down the CGContext
	CGContextFlush(cgContext);
	QDEndCGContext(port, &cgContext);

	// Deallocate string storage
	free(text);

	// Layout, style and font fallbacks also need to be disposed
	verify_noerr( ATSUDisposeStyle(style) );
	verify_noerr( ATSUDisposeTextLayout(layout) );
	verify_noerr( ATSUDisposeFontFallbacks(fallbacks) );

    // Restore the graphics port
    SetPort(savedPort);
}
