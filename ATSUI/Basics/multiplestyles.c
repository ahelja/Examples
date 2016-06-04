/*

File: multiplestyles.c

Abstract: Multiple styles ATSUI demo code. Contains an extremely simple
case for ATSUI drawing in a window using more than one style. 

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

#include "multiplestyles.h"

// Constants
#define kMultipleStylesMargin 25
#define kMultipleStylesFontName "Times Roman"
#define kMultipleStylesFontName2 "Times Italic"
#define kMultipleStylesFontSize 16

// This example is almost identical to the helloworld example, except that
// in this case, there are two styles instead of just one. ATSUSetRunStyle
// is used to apply a style to different parts of the text.
//
void DrawMultipleStylesContents(WindowRef window)
{
	CFStringRef					string;
	UniChar						*text;
	UniCharCount				length;
    UniCharArrayOffset			currentStart, currentEnd;
	ATSUStyle					style1, style2;
	ATSUTextLayout				layout;
	ATSUFontID					font;
	Fixed						pointSize;
    ATSUAttributeTag			tags[2];
    ByteCount					sizes[2];
    ATSUAttributeValuePtr		values[2];
	Fixed						lineWidth, ascent, descent;
	CGContextRef				cgContext;
	float						x, y, cgY, windowHeight;
    ItemCount					numSoftBreaks;
    UniCharArrayOffset			*theSoftBreaks;
    int							i;
    GrafPtr						port, savedPort;
	Rect						portBounds;

    // Set up the graphics port
	port = GetWindowPort(window);
    GetPort(&savedPort);
    SetPort(port);
    GetPortBounds(port, &portBounds);
    EraseRect(&portBounds);

	// Create a style object. This is one of two objects necessary to draw using ATSUI.
	// (The layout is the other.)
	verify_noerr( ATSUCreateStyle(&style1) );

    // Look up the font we are going to use, and set it in the style object, using
	// the aforementioned "triple" (tag, size, value) semantics. This is how almost
	// all settings in ATSUI are applied.
	verify_noerr( ATSUFindFontFromName(kMultipleStylesFontName, strlen(kMultipleStylesFontName), kFontFullName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, &font) );
    tags[0] = kATSUFontTag;
    sizes[0] = sizeof(ATSUFontID);
    values[0] = &font;
    verify_noerr( ATSUSetAttributes(style1, 1, tags, sizes, values) );

    // Set the point size, also using a triple. You can actually set multiple triples at once,
	// since the tag, size, and value parameters are arrays. Other examples do this, such as
	// the vertical text example.
	// 
	pointSize = Long2Fix(kMultipleStylesFontSize);
	tags[0] = kATSUSizeTag;
    sizes[0] = sizeof(Fixed);
    values[0] = &pointSize;
    verify_noerr( ATSUSetAttributes(style1, 1, tags, sizes, values) );
	
	// Now we create the second of two objects necessary to draw text using ATSUI, the layout.
	// You can specify a pointer to the text buffer at layout creation time, or later using
	// the routine ATSUSetTextPointerLocation(). Below, we do it after layout creation time.
	verify_noerr( ATSUCreateTextLayout(&layout) );

	// Before assigning text to the layout, we must first convert the string we plan to draw
	// from a CFStringRef into an array of UniChar.
	string = CFStringCreateWithCString(NULL, "In this example, various parts of the text have different styles applied. The same style is used more than once.", kCFStringEncodingASCII);

    // Extract the raw Unicode from the CFString, then dispose of the CFString
    length = CFStringGetLength(string);
    text = (UniChar *)malloc(length * sizeof(UniChar));
    CFStringGetCharacters(string, CFRangeMake(0, length), text);
	CFRelease(string);

    // Attach the resulting UTF-16 Unicode text to the layout
    verify_noerr( ATSUSetTextPointerLocation(layout, text, kATSUFromTextBeginning, kATSUToTextEnd, length) );
	
	// Now we tie the two necessary objects, the layout and the style, together
	verify_noerr( ATSUSetRunStyle(layout, style1, kATSUFromTextBeginning, kATSUToTextEnd) );

	// Now, for this example we create a second style, and assign it to various runs within
	// the text. For our example, the run offsets are hard-coded for simplicity's sake. In
	// a real application, style runs are often assigned from external sources, such as user
	// selection.
	verify_noerr( ATSUCreateAndCopyStyle(style1, &style2) );

	// Change the font for the second style
	verify_noerr( ATSUFindFontFromName(kMultipleStylesFontName2, strlen(kMultipleStylesFontName2), kFontFullName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, &font) );
    tags[0] = kATSUFontTag;
    sizes[0] = sizeof(ATSUFontID);
    values[0] = &font;
    verify_noerr( ATSUSetAttributes(style2, 1, tags, sizes, values) );

	// Apply the new style to the text in various places
	verify_noerr( ATSUSetRunStyle(layout, style2, 8, 7) ); // The word "example"
	verify_noerr( ATSUSetRunStyle(layout, style2, 65, 7) ); // The word "applied"
	verify_noerr( ATSUSetRunStyle(layout, style2, 83, 5) ); // The word "style"
	verify_noerr( ATSUSetRunStyle(layout, style2, 107, 4) ); // The word "once"

	// In this example, we are breaking text into lines.
	// Therefore, we need to know the width of the line.
    lineWidth = X2Fix(portBounds.right - portBounds.left - 2.0*kMultipleStylesMargin);
    tags[0] = kATSULineWidthTag;
    sizes[0] = sizeof(Fixed);
    values[0] = &lineWidth;
	verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );

	// Prepare the CGContext for drawing
	QDBeginCGContext(port, &cgContext);
	tags[0] = kATSUCGContextTag;
	sizes[0] = sizeof(CGContextRef);
	values[0] = &cgContext;
	verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );
	
	// Prepare the coordinates for drawing. In our example, "x" and "y" are the coordinates
	// in QD space. "cgY" contains the y coordinate in CG space.
	//
	windowHeight = portBounds.bottom - portBounds.top;
	x = kMultipleStylesMargin; // leave a small left margin
	y = kMultipleStylesMargin; // leave a small top margin
	cgY = windowHeight - y; // Subtract the y coordinate from the height of the
							// window to get the coordinate in CG-aware space.

	// Break the text into lines
	verify_noerr( ATSUBatchBreakLines(layout, kATSUFromTextBeginning, length, lineWidth, &numSoftBreaks) );
    verify_noerr( ATSUGetSoftLineBreaks(layout, kATSUFromTextBeginning, kATSUToTextEnd, 0, NULL, &numSoftBreaks) );
    theSoftBreaks = (UniCharArrayOffset *) malloc(numSoftBreaks * sizeof(UniCharArrayOffset));
    verify_noerr( ATSUGetSoftLineBreaks(layout, kATSUFromTextBeginning, kATSUToTextEnd, numSoftBreaks, theSoftBreaks, &numSoftBreaks) );
	
    // Loop over all the lines and draw them
    currentStart = 0;
    for (i=0; i <= numSoftBreaks; i++) {
        currentEnd = ((numSoftBreaks > 0 ) && (numSoftBreaks > i)) ? theSoftBreaks[i] : length;

        // This is the height of a line, the ascent and descent. Getting the values this way is the preferred method.
		ATSUGetLineControl(layout, currentStart, kATSULineAscentTag, sizeof(ATSUTextMeasurement), &ascent, NULL);
        ATSUGetLineControl(layout, currentStart, kATSULineDescentTag, sizeof(ATSUTextMeasurement), &descent, NULL);

        // Make room for the area above the baseline
		y += Fix2X(ascent);
		cgY = windowHeight - y;

		// Draw the text
        verify_noerr( ATSUDrawText(layout, currentStart, currentEnd - currentStart, X2Fix(x), X2Fix(cgY)) );

		// Make room for the area beloww the baseline
        y += Fix2X(descent);
		
		// Prepare for next line
        currentStart = currentEnd;
    }

	// This is a one-shot window, so we are now ready to dispose of all our objects.
	// Normally, we would want to keep everything around in case we needed to redraw or change
	// the text at some point.
	
    // Tear down the CGContext
	CGContextFlush(cgContext);
	QDEndCGContext(port, &cgContext);

	// Deallocate string storage
	free(text);

	// Layout and styles also need to be disposed
	verify_noerr( ATSUDisposeStyle(style1) );
	verify_noerr( ATSUDisposeStyle(style2) );
	verify_noerr( ATSUDisposeTextLayout(layout) );

    // Restore the graphics port
    SetPort(savedPort);
}
