/*

File: verticaltext.c

Abstract: Shows how to draw vertical CJK text using ATSUI. 

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
#include "verticaltext.h"

// Constants
#define kVerticalTextMargin 25
#define kVerticalTextFontName "Hiragino Mincho Pro W3"
#define kVerticalTextFontSize 24


// This example demonstrates how to draw vertical CJK text.
// This consists of two things over horizontal text:
//
//    - Rotating the line -90 degrees
//    - Setting the StronglyVertical style attribute
//
// When the strongly vertical style attribute is set, ATSUI
// will automatically choose glyph variants from fonts that
// are more appropriate to veritcal text than to horizontal.
//
void DrawVerticalTextContents(WindowRef window)
{
	CFStringRef					string;
	UniChar						*text;
	UniCharCount				length;
    UniCharArrayOffset			currentStart, currentEnd;
	ATSUStyle					style;
	ATSUTextLayout				layout;
	ATSUFontID					font;
	Fixed						pointSize;
    ATSUAttributeTag			tags[2];
    ByteCount					sizes[2];
    ATSUAttributeValuePtr		values[2];
	Fixed						lineRotation, lineWidth, ascent, descent;
	ATSUVerticalCharacterType	charType;
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
	verify_noerr( ATSUCreateStyle(&style) );

    // Set the point size and the font, together (note the second parameter to ATSUSetAttributes)
	verify_noerr( ATSUFindFontFromName(kVerticalTextFontName, strlen(kVerticalTextFontName), kFontFullName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, &font) );
    tags[0] = kATSUFontTag;
    sizes[0] = sizeof(ATSUFontID);
    values[0] = &font;

	pointSize = Long2Fix(kVerticalTextFontSize);
	tags[1] = kATSUSizeTag;
    sizes[1] = sizeof(Fixed);
    values[1] = &pointSize;

    verify_noerr( ATSUSetAttributes(style, 2, tags, sizes, values) );
	
	// For this example, we need to set the text to be "strongly vertical".
	// This means the glyphs are actually rotated to line up vertically along the
	// text baseline. Combined with line rotation (see below), this creates the type
	// of vertical layout often seen in CJK typography.
	charType = kATSUStronglyVertical;
	tags[0] = kATSUVerticalCharacterTag;
    sizes[0] = sizeof(ATSUVerticalCharacterType);
    values[0] = &charType;
    verify_noerr( ATSUSetAttributes(style, 1, tags, sizes, values) );

	// Now we create the second of two objects necessary to draw text using ATSUI, the layout.
	verify_noerr( ATSUCreateTextLayout(&layout) );

	// Load the text we plan to use from the application bundle.
	// (The following helper function is in filesupport.c)
	string = LoadFileIntoCFString("JapaneseSample_UTF16.txt");

    // Extract the raw Unicode from the CFString, then dispose of the CFString
    length = CFStringGetLength(string);
    text = (UniChar *)malloc(length * sizeof(UniChar));
    CFStringGetCharacters(string, CFRangeMake(0, length), text);
	CFRelease(string);

    // Attach the resulting UTF-16 Unicode text to the layout
    verify_noerr( ATSUSetTextPointerLocation(layout, text, kATSUFromTextBeginning, kATSUToTextEnd, length) );
	
	// Now we tie the two necessary objects, the layout and the style, together
	verify_noerr( ATSUSetRunStyle(layout, style, kATSUFromTextBeginning, kATSUToTextEnd) );

	// Here is where we set up the line rotation. The value is specified in degrees.
	// Always check the ATSUI documentation or headers for information on 
	// tags such as kATSULineRotationTag.
    lineRotation = X2Fix(-90.0);
    tags[0] = kATSULineRotationTag;
    sizes[0] = sizeof(Fixed);
    values[0] = &lineRotation;
	verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );

	// In this example, we are breaking text into lines.
	// Therefore, we need to know the width (or since this is vertical text, height) of the line.
	windowHeight = portBounds.bottom - portBounds.top;
    lineWidth = X2Fix(windowHeight - (2.0*kVerticalTextMargin));
    tags[0] = kATSULineWidthTag;
    sizes[0] = sizeof(Fixed);
    values[0] = &lineWidth;
	verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );

	// Set up the CGContext for drawing
	//
	QDBeginCGContext(port, &cgContext);
	tags[0] = kATSUCGContextTag;
	sizes[0] = sizeof(CGContextRef);
	values[0] = &cgContext;
	verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );

	// Break the text into lines
	//
	// (For more information on line breaking, see paragraphs.c)
	//
	verify_noerr( ATSUBatchBreakLines(layout, kATSUFromTextBeginning, length, lineWidth, &numSoftBreaks) );
    verify_noerr( ATSUGetSoftLineBreaks(layout, kATSUFromTextBeginning, kATSUToTextEnd, 0, NULL, &numSoftBreaks) );
    theSoftBreaks = (UniCharArrayOffset *) malloc(numSoftBreaks * sizeof(UniCharArrayOffset));
    verify_noerr( ATSUGetSoftLineBreaks(layout, kATSUFromTextBeginning, kATSUToTextEnd, numSoftBreaks, theSoftBreaks, &numSoftBreaks) );

	// Prepare the coordiates for drawing.
	//
	x = kVerticalTextMargin; // leave a small left margin
	y = kVerticalTextMargin; // leave a small top margin
	cgY = windowHeight - y; // Subtract the y coordinate from the height of the
							// window to get the coordinate in CG-aware space.
								
    // Loop over all the lines and draw them
    currentStart = 0;
    for (i=0; i <= numSoftBreaks; i++) {
        currentEnd = ((numSoftBreaks > 0 ) && (numSoftBreaks > i)) ? theSoftBreaks[i] : length;

		// (For more information on line spacing, see paragraphs.c)

        ATSUGetLineControl(layout, currentStart, kATSULineAscentTag, sizeof(ATSUTextMeasurement), &ascent, NULL);
        ATSUGetLineControl(layout, currentStart, kATSULineDescentTag, sizeof(ATSUTextMeasurement), &descent, NULL);
        x += Fix2X(ascent);

        verify_noerr( ATSUDrawText(layout, currentStart, currentEnd - currentStart, X2Fix(x), X2Fix(cgY)) );

        x += Fix2X(descent);
        currentStart = currentEnd;
    }

    // Free previously allocated memory
    free(theSoftBreaks);

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
