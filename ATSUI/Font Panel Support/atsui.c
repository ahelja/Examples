/*

File: atsui.c

Abstract: ATSUI handling routines for ATSUIFontPanelDemo project.
The layout and style are globals scoped to this file only. All
getters and setters operate on these gloals. The function
ATSUIStuffSetDictionary() contains font-panel specific style setting code.


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

#include "globals.h"
#include "atsui.h"

// Globals
//
static ATSUTextLayout gLayout;
static ATSUStyle gStyle;
static ATSUFontFallbacks gFallbacks;


// Functions


// Sets up the ATSUI stuff for the first time
//
void ATSUIStuffInitialize(void)
{
    ATSUFontID font;
    CFStringRef startingString;
    
    // Create the objects
    verify_noerr( ATSUCreateStyle(&gStyle) );
    verify_noerr( ATSUCreateTextLayout(&gLayout) );

    // Fill in the style
    verify_noerr( ATSUFindFontFromName(kStartingFontName, strlen(kStartingFontName), kFontFullName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, &font) );
    ATSUIStuffSetFont(font);
    ATSUIStuffSetSize(Long2Fix(kStartingFontSize));

    // Fill in the layout
    verify_noerr( GetControlData(gStringInputControl, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), (void *)&startingString, NULL) );
    ATSUIStuffSetText(startingString);
    CFRelease(startingString);
    verify_noerr( ATSUSetRunStyle(gLayout, gStyle, kATSUFromTextBeginning, kATSUToTextEnd) );
    
    // Set up the font substitution object
	ATSUIStuffSetFontFallbacks();
}


// Sets the font for the style
//
void ATSUIStuffSetFont(ATSUFontID font)
{
    ATSUAttributeTag                    tags[1];
    ByteCount                           sizes[1];
    ATSUAttributeValuePtr               values[1];

    tags[0] = kATSUFontTag;
    sizes[0] = sizeof(ATSUFontID);
    values[0] = &font;
    verify_noerr( ATSUSetAttributes(gStyle, 1, tags, sizes, values) );
}


// Sets the point size for the style
//
void ATSUIStuffSetSize(Fixed size)
{
    ATSUAttributeTag                    tags[1];
    ByteCount                           sizes[1];
    ATSUAttributeValuePtr               values[1];

    tags[0] = kATSUSizeTag;
    sizes[0] = sizeof(Fixed);
    values[0] = &size;
    verify_noerr( ATSUSetAttributes(gStyle, 1, tags, sizes, values) );
}


// Initializes full font fallbacks (ATSUI will automatically substitute
// available fonts for characters that are not covered by the current font).
//
void ATSUIStuffSetFontFallbacks(void)
{
    ATSUAttributeTag                    tags[1];
    ByteCount                           sizes[1];
    ATSUAttributeValuePtr               values[1];

    verify_noerr( ATSUCreateFontFallbacks(&gFallbacks) );
    verify_noerr( ATSUSetObjFontFallbacks(gFallbacks, 0, NULL, kATSUDefaultFontFallbacks) );

	tags[0] = kATSULineFontFallbacksTag;
	sizes[0] = sizeof(ATSUFontFallbacks);
	values[0] = &gFallbacks;
	verify_noerr( ATSUSetLayoutControls(gLayout, 1, tags, sizes, values) );
	verify_noerr( ATSUSetTransientFontMatching(gLayout, true) );
}


// Tells the font panel what the current style is.
// If you were writing a text editor, you would call something like this
// whenever the user changed the selection.
//
void ATSUIStuffSetFontPanelState(void)
{
    verify_noerr( SetFontInfoForSelection(kFontSelectionATSUIType, 1, (void *)&gStyle, NULL) );
}


// Sets the text for the layout from a CFString. ATSUI only deals in UTF-16 Unicode,
// so we must extract the UTF-16 from the CFString, and pass that to ATSUI.
//
void ATSUIStuffSetText(CFStringRef string)
{
    UniChar *text;
    UniCharCount length;

    // Dealloc the previous string, if there was one
    if ( kATSUInvalidTextLayoutErr !=  ATSUGetTextLocation(gLayout, (void **)(&text), NULL, NULL, NULL, NULL) )
        free(text);

    // Extract the raw Unicode from the CFString
    // (CFStringGetCharactersPtr almost never works, so skip it)
    length = CFStringGetLength(string);
    text = (UniChar *)malloc(length * sizeof(UniChar));
    CFStringGetCharacters(string, CFRangeMake(0, length), text);
    
    // Attach the Unicode text to the layout
    verify_noerr( ATSUSetTextPointerLocation(gLayout, text, kATSUFromTextBeginning, kATSUToTextEnd, length) );
}


// Parses the CFDictionary of settings that comes back from the font panel.
// Font features, variations, color, underline, dropshadow, etc. are all
// handled here.
//
void ATSUIStuffSetDictionary(CFDictionaryRef dict)
{
    CFDataRef axisData;
    CFDataRef valueData;
    CFDataRef featureTypesData;
    CFDataRef featureSelectorData;
    CFDataRef tagsData;
    CFDataRef sizesData;
    CFDataRef valuesData;
    CFDictionaryRef attributesDict;

    // Font variations, accessed from the "Typography" menu item in the font panel (see ReadMe)
    //
    if ( CFDictionaryGetValueIfPresent(dict, kFontPanelVariationAxesKey, (const void **)&axisData) &&
         CFDictionaryGetValueIfPresent(dict, kFontPanelVariationValuesKey, (const void **)&valueData) )
    {
        ItemCount count = CFDataGetLength(axisData)/sizeof(ATSUFontVariationAxis);
        ATSUFontVariationAxis *axisPtr = (ATSUFontVariationAxis *)CFDataGetBytePtr(axisData);
        ATSUFontVariationValue *valuePtr = (ATSUFontVariationValue *)CFDataGetBytePtr(valueData);
        verify_noerr( ATSUSetVariations(gStyle, count, axisPtr, valuePtr) );
    }

    // Font features, accessed from the "Typography" menu item in the font panel (see ReadMe)
    //
    if ( CFDictionaryGetValueIfPresent(dict, kFontPanelFeatureTypesKey, (const void **)&featureTypesData) &&
         CFDictionaryGetValueIfPresent(dict, kFontPanelFeatureSelectorsKey, (const void **)&featureSelectorData) )
    {
        ItemCount count = CFDataGetLength(featureTypesData)/sizeof(ATSUFontFeatureType);
        ATSUFontFeatureType *typePtr = (ATSUFontFeatureType *)CFDataGetBytePtr(featureTypesData);
        ATSUFontFeatureSelector *selectorPtr = (ATSUFontFeatureSelector *)CFDataGetBytePtr(featureSelectorData);
        verify_noerr( ATSUSetFontFeatures(gStyle, count, typePtr, selectorPtr) );
    }

    // Other settings (includes color, underline, strikethrough, and drop shadow)
    //
    if ( CFDictionaryGetValueIfPresent(dict, kFontPanelAttributesKey, (const void **)&attributesDict) )
    {
        if ( CFDictionaryGetValueIfPresent(attributesDict, kFontPanelAttributeTagsKey, (const void **)&tagsData) &&
             CFDictionaryGetValueIfPresent(attributesDict, kFontPanelAttributeSizesKey, (const void **)&sizesData) &&
             CFDictionaryGetValueIfPresent(attributesDict, kFontPanelAttributeValuesKey, (const void **)&valuesData) )
        {
            ItemCount count = CFDataGetLength(tagsData)/sizeof(ATSUAttributeTag);
            ATSUAttributeTag *tagPtr = (ATSUAttributeTag *)CFDataGetBytePtr(tagsData);
            ByteCount *sizePtr = (ByteCount *)CFDataGetBytePtr(sizesData);
            UInt32 *bytePtr = (UInt32*)CFDataGetBytePtr(valuesData);
            ATSUAttributeValuePtr *valuesPtr = malloc( count * sizeof(ATSUAttributeValuePtr));
            int i;
    
            for (i=0; i < count; i++) {
                valuesPtr[i] = bytePtr;
                bytePtr = (UInt32*)( (UInt8*)bytePtr + sizePtr[i]);
            }
            verify_noerr( ATSUSetAttributes(gStyle, count, tagPtr, sizePtr, valuesPtr) );
            
            free(valuesPtr);
        }
    }
}


// Draws the ATSUI stuff to the specified port.
// As with other examples, note that coordinates start at the top
// of the window, and go down (Quickdraw style coordinates). Also note
// that ATSUI does not accept floats or doubles, only Fixed. The macros
// in FixMath.h (found in the CarbonCore framework) will convert to and
// from Fixed. These macros are called "Fix2X" and "X2Fix".
//
void ATSUIStuffDraw(GrafPtr port)
{
    ATSUAttributeTag                    tags[1];
    ByteCount                           sizes[1];
    ATSUAttributeValuePtr               values[1];
    UniCharArrayOffset					textStart, currentStart, currentEnd;
    UniCharCount						length;
    Fixed								width, ascent, descent;
    Rect								portBounds;
    Rect								margin;
    CGContextRef						context;
    GrafPtr								savedPort;
    float								penX, penY, cgAwarePenY, windowHeight;
    ItemCount							numSoftBreaks;
    UniCharArrayOffset					*theSoftBreaks;
    int									i;
    OSStatus							status;

    // Set up the graphics port
    GetPort(&savedPort);
    SetPort(port);
    GetPortBounds(port, &portBounds);
    EraseRect(&portBounds);
    
    // Draw the margin
    MacSetRect(&margin, portBounds.left, portBounds.top, portBounds.right, portBounds.bottom);
    MacInsetRect(&margin, kMargin, kMargin);
    MacFrameRect(&margin);

    // Set up the line width
    width = Long2Fix(portBounds.right - portBounds.left - (kMargin * 2));
    tags[0] = kATSULineWidthTag;
    sizes[0] = sizeof(Fixed);
    values[0] = &width;
    verify_noerr( ATSUSetLayoutControls(gLayout, 1, tags, sizes, values) );

    // Set up the CGContext
	QDBeginCGContext(port, &context);
	tags[0] = kATSUCGContextTag;
	sizes[0] = sizeof(CGContextRef);
	values[0] = &context;
	verify_noerr( ATSUSetLayoutControls(gLayout, 1, tags, sizes, values) );
    
    // Initial pen position
    windowHeight = portBounds.bottom - portBounds.top;
    penX = kMargin;
    penY = kMargin;
    cgAwarePenY = windowHeight - penY; // This is how you transform between CG and QD style coordinates

    // Break the text into lines
    verify_noerr( ATSUGetTextLocation(gLayout, NULL, NULL, &textStart, &length, NULL) );
	verify_noerr( ATSUBatchBreakLines(gLayout, textStart, length, width, &numSoftBreaks) );

    // Get the soft breaks
    verify_noerr( ATSUGetSoftLineBreaks(gLayout, kATSUFromTextBeginning, kATSUToTextEnd, 0, NULL, &numSoftBreaks) );
    theSoftBreaks = (UniCharArrayOffset *) malloc(numSoftBreaks * sizeof(UniCharArrayOffset));
    verify_noerr( ATSUGetSoftLineBreaks(gLayout, kATSUFromTextBeginning, kATSUToTextEnd, numSoftBreaks, theSoftBreaks, &numSoftBreaks) );

    // Loop over all the softbreaks and draw them
    currentStart = textStart;
    for (i=0; i <= numSoftBreaks; i++) {
        currentEnd = ((numSoftBreaks > 0 ) && (numSoftBreaks > i)) ? theSoftBreaks[i] : length;

        ATSUGetLineControl(gLayout, currentStart, kATSULineAscentTag, sizeof(ATSUTextMeasurement), &ascent, NULL);
        ATSUGetLineControl(gLayout, currentStart, kATSULineDescentTag, sizeof(ATSUTextMeasurement), &descent, NULL);
        penY += Fix2X(ascent);

        cgAwarePenY = windowHeight - penY;
        verify_noerr( ATSUDrawText(gLayout, currentStart, currentEnd - currentStart, X2Fix(penX), X2Fix(cgAwarePenY)) );

        penY += Fix2X(descent);
        currentStart = currentEnd;
    }

    // Free previously allocated memory
    free(theSoftBreaks);

    // Tear down the CGContext
	CGContextFlush(context);
	QDEndCGContext(port, &context);

    // Restore the graphics port
    SetPort(savedPort);
}
