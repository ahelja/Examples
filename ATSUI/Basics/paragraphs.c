/*

File: paragraphs.c

Abstract: This example reads in some unicode text from a file, then sets up
and stores some ATSUI layouts and styles based on this text persistently, so that
the text may be redrawn at a moment's notice. The window supports live resize,
demonstrating that keeping ATSUI objects around persistently results in very
smooth drawing performance.

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

#include "findparagraph.h"
#include "filesupport.h"
#include "paragraphs.h"

// ****** Demo behavior constants
//
// These constants control the way ATSUI is used in this source file.

// Determines if the ATSUBatchBreakLines() is to be used
#define USE_BATCHBREAKLINES 1

// Determines if a pointer to the entire text buffer or just
// the local portion of the text buffer is to be used when
// setting up a layout. Setting thie to 0 on systems prior
// to 10.4 can be problematic, and is not recommended.
#define USE_LOCAL_POINTERS 1

// Controls the method used to determine line height
#define USE_GETGLYPHBOUNDS 0

// ****** other constants
#define kParagraphsMargin 25
#define kParagraphsFontName "Times Roman"
#define kParagraphsFontSize 16

// Globals
static UniChar						*gText;
static UniCharCount					gLength;
static CFMutableArrayRef			gLayouts;
static ATSUStyle					gStyle;


// Functions


// This will draw the contents of the window during live resize
//
OSStatus DoWindowBoundsChanged(EventHandlerCallRef nextHandler, EventRef theEvent, void *userData)
{
    WindowRef			window;
    UInt32				eventAttributes;

    // Find out if the window was dragged or resized.  If dragged, don't do anything.
    verify_noerr( GetEventParameter(theEvent, kEventParamAttributes, typeUInt32, NULL, sizeof(typeUInt32), NULL, &eventAttributes) );
	if ( ! (eventAttributes & kWindowBoundsChangeSizeChanged) ) {
		return eventNotHandledErr;
    }

    // Get the WindowRef from the event structure
    verify_noerr( GetEventParameter(theEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window) );

    // Draw the ATSUI content into the window
    DrawParagraphsContents(window);

    return noErr;
}


// Install the live resize event handler for this window
//
void InstallParagraphsDemoResizeHandler(WindowRef window)
{
	EventTypeSpec					eventType;

    // Install a handler to draw the contents of the window during live resize
    eventType.eventClass = kEventClassWindow;
    eventType.eventKind  = kEventWindowBoundsChanged;
    verify_noerr( InstallWindowEventHandler(window, NewEventHandlerUPP(DoWindowBoundsChanged), 1, &eventType, NULL, NULL) );
}


// Initializes everything for this particular demo
//
void SetUpParagraphsContents(WindowRef window)
{
	CFStringRef						string;
	ATSUTextLayout					tempLayout;
	ATSUFontID						font;
	Fixed							pointSize;
    ATSUAttributeTag				tags[2];
    ByteCount						sizes[2];
    ATSUAttributeValuePtr			values[2];
	UniCharArrayOffset				currentParagraphStart;
	UniCharArrayOffset				currentParagraphEnd;
	Boolean							endOfDocument;

    // Make sure the window has a resize event handler
    InstallParagraphsDemoResizeHandler(window);

	// **** Initialize the text
	// Load the text we plan to use from the application bundle.
	// (The following helper function is in filesupport.c)
	string = LoadFileIntoCFString("EnglishSample_UTF16.txt");

    // Extract the raw Unicode from the CFString, then dispose of the CFString
    gLength = CFStringGetLength(string);
    gText = (UniChar *)malloc(gLength * sizeof(UniChar));
    CFStringGetCharacters(string, CFRangeMake(0, gLength), gText);
	CFRelease(string);

	// **** Set up the style
	// (this should be a familiar process by now)
	verify_noerr( ATSUCreateStyle(&gStyle) );

	verify_noerr( ATSUFindFontFromName(kParagraphsFontName, strlen(kParagraphsFontName), kFontFullName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, &font) );
    tags[0] = kATSUFontTag;
    sizes[0] = sizeof(ATSUFontID);
    values[0] = &font;

	pointSize = Long2Fix(kParagraphsFontSize);
	tags[1] = kATSUSizeTag;
    sizes[1] = sizeof(Fixed);
    values[1] = &pointSize;

    verify_noerr( ATSUSetAttributes(gStyle, 2, tags, sizes, values) );

	// **** Create all of the layouts
	// Loop over all the text, using a helper function that finds the edges of
	// paragraphs, and assign each paragraph to a layout object.
	gLayouts = CFArrayCreateMutable(NULL, 0, NULL);
	endOfDocument = false;
	currentParagraphEnd = 0;
	do {
		UniCharCount runLength;
		
		currentParagraphStart = currentParagraphEnd;
		endOfDocument = FindParagraph(gText, gLength, currentParagraphStart, &currentParagraphEnd);

		runLength = currentParagraphEnd - currentParagraphStart;
		if ( runLength > 0 ) {
			// NOTE: Systems prior to 10.4 exhibit problems with non-local pointers. It is recommended
			// that only pointers local to each layout be used on 10.3 and earlier systems.
			#if USE_LOCAL_POINTERS
				verify_noerr( ATSUCreateTextLayoutWithTextPtr(&(gText[currentParagraphStart]), 0, runLength, runLength, 1, &runLength, &gStyle, &tempLayout) );
			#else
				verify_noerr( ATSUCreateTextLayoutWithTextPtr(gText, currentParagraphStart, runLength, gLength, 1, &runLength, &gStyle, &tempLayout) );
			#endif
			CFArrayAppendValue(gLayouts, (void *)tempLayout);
		}
	} while ( ! endOfDocument );
}


// Draws the current ATSUI contents in the window
//
void DrawParagraphsContents(WindowRef window)
{
	ATSUTextLayout				layout;
    UniCharArrayOffset			layoutStart, layoutEnd, currentStart, currentEnd;
	UniCharCount				layoutLength;
    ATSUAttributeTag			tags[2];
    ByteCount					sizes[2];
    ATSUAttributeValuePtr		values[2];
	Fixed						lineWidth, ascent, descent;
	CGContextRef				cgContext;
	float						x, y, cgY, windowHeight;
    ItemCount					numSoftBreaks;
    UniCharArrayOffset			*theSoftBreaks;
	int							i, j;
	Boolean						done = false;
    GrafPtr						port, savedPort;
	Rect						portBounds;

    // Set up the graphics port
	port = GetWindowPort(window);
    GetPort(&savedPort);
    SetPort(port);
    GetPortBounds(port, &portBounds);
    EraseRect(&portBounds);

	// Set up the CGContext
	QDBeginCGContext(port, &cgContext);

	// Prepare the coordinates for drawing. In our example, "x" and "y" are the coordinates
	// in QD space. "cgY" contains the y coordinate in CG space.
	//
	lineWidth = X2Fix(portBounds.right - portBounds.left - 2.0*kParagraphsMargin);
	windowHeight = portBounds.bottom - portBounds.top;
	x = kParagraphsMargin; // leave a small left margin
	y = kParagraphsMargin; // leave a small top margin
	cgY = windowHeight - y; // Subtract the y coordinate from the height of the
							// window to get the coordinate in CG-aware space.

	// Loop over all the layouts, break them into lines, then draw them
	for (i=0; i < CFArrayGetCount(gLayouts); i++) {
		
		layout = (ATSUTextLayout)CFArrayGetValueAtIndex(gLayouts, i);
		
		// In this example, we are breaking text into lines.
		// Therefore, we need to make sure the layout knows the width of the line.
		tags[0] = kATSULineWidthTag;
		sizes[0] = sizeof(Fixed);
		values[0] = &lineWidth;
		verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );

		// Make sure the layout knows the proper CGContext to use for drawing
		tags[0] = kATSUCGContextTag;
		sizes[0] = sizeof(CGContextRef);
		values[0] = &cgContext;
		verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );
		
		// FInd out about this layout's text buffer
	    verify_noerr( ATSUGetTextLocation(layout, NULL, NULL, &layoutStart, &layoutLength, NULL) );
		
		// Break the text into lines
		//
		// There are two methods for doing this. ATSUBreakLine() will do line breaks one at a time,
		// while ATSUBatchBreakLines() will break an entire paragraph at once. ATSUBatchBreakLines()
		// offers up to 50% greater performance than ATSUBreakLine(), but is only available on 10.2
		// and later.
		//
        #if USE_BATCHBREAKLINES
            verify_noerr( ATSUBatchBreakLines(layout, layoutStart, layoutLength, lineWidth, &numSoftBreaks) );
        #else
            currentStart = layoutStart;
            currentEnd = layoutStart + layoutLength;
            do {
                status = ATSUBreakLine(layout, currentStart, lineWidth, true, &currentEnd);
				verify( status != 
                currentStart = currentEnd;
            } while ( currentEnd < layoutStart + layoutLength );
        #endif
		
		// Obtain a list of all the line break positions
		verify_noerr( ATSUGetSoftLineBreaks(layout, layoutStart, layoutLength, 0, NULL, &numSoftBreaks) );
		theSoftBreaks = (UniCharArrayOffset *) malloc(numSoftBreaks * sizeof(UniCharArrayOffset));
		verify_noerr( ATSUGetSoftLineBreaks(layout, layoutStart, layoutLength, numSoftBreaks, theSoftBreaks, &numSoftBreaks) );
		
		// Loop over all the lines and draw them
		currentStart = layoutStart;
		for (j=0; j <= numSoftBreaks; j++) {
			currentEnd = ((numSoftBreaks > 0 ) && (numSoftBreaks > j)) ? theSoftBreaks[j] : layoutStart + layoutLength;

			// This is the height of a line, the ascent and descent.
			//
			// The ascent is the amount of text that extends above the baseline.
			// The descent is the amount of text that extends below the baseline.
			// (The y-coordinate that is passed to ATSUDrawText is where the baseline will be drawn.)
			//
			// Many fonts also include "leading", which is extra space specified by the font designer
			// to be applied below the baseline when spacing apart lines. Leading is usually included
			// when fetching the descent, unless the kATSLineIgnoreFontLeading layout control is set,
			// or when using ATSUGetAttribute to fetch the descent directly from a style (in that case,
			// use kATSULeadingTag to fetch the leading separately).
			//
			// There are two methods for getting these values: using ATSUGetLineControl() and ATSUGetGlyphBounds()
			// The ATSUGetLineControl method is preferred, but only works on 10.2 and later systems. The
			// ATSUGetGlyphBounds method works on all systems, including Classic and CarbonLib on Mac OS 8 and 9.
			//
			#if USE_GETGLYPHBOUNDS
				ATSTrapezoid theBounds;
				ItemCount numBounds;

				// Note that when calling ATSUGetGlyphBounds on an entire line at once, there is always only one trapezoid returned.
				ATSUGetGlyphBounds(layout, 0, 0, currentStart, currentEnd - currentStart, kATSUseFractionalOrigins, 1, &theBounds, &numBounds);
				ascent = - theBounds.upperLeft.y;
				descent = theBounds.lowerLeft.y;
			#else
				ATSUGetLineControl(layout, currentStart, kATSULineAscentTag, sizeof(ATSUTextMeasurement), &ascent, NULL);
				ATSUGetLineControl(layout, currentStart, kATSULineDescentTag, sizeof(ATSUTextMeasurement), &descent, NULL);
			#endif

			// Make room for the area above the baseline.
			y += Fix2X(ascent);
			if ( y > portBounds.bottom ) {
				done = true;
				break; // if we go past the end of the window, stop
			}
			cgY = windowHeight - y;

			// Draw the text
			verify_noerr( ATSUDrawText(layout, currentStart, currentEnd - currentStart, X2Fix(x), X2Fix(cgY)) );

			// Make room for the area beloww the baseline
			y += Fix2X(descent);
			if ( y > portBounds.bottom ) {
				done = true;
				break; // if we go past the end of the window, stop
			}
			
			// Prepare for next line
			currentStart = currentEnd;
		}
		free(theSoftBreaks);
		if ( done ) break; // if we go past the end of the window, stop
	}

    // Tear down the CGContext
	CGContextFlush(cgContext);
	QDEndCGContext(port, &cgContext);

    // Restore the graphics port
    SetPort(savedPort);
}


// Disposes of all objects.
// This never actually gets called in our example app, because the window
// and all its associated objects are persistent for the life of the app,
// but it's good practice to have a routine like this around.
//
void DisposeParagraphsContents(void)
{
	ATSUTextLayout layout;
	int i;

	free(gText);
	verify_noerr( ATSUDisposeStyle(gStyle) );
	for (i=0; i < CFArrayGetCount(gLayouts); i++) {
		layout = (ATSUTextLayout)CFArrayGetValueAtIndex(gLayouts, i);
		verify_noerr( ATSUDisposeTextLayout(layout) );
	}
	CFRelease(gLayouts);
}
