/*

File: atsui.c

Abstract: Main drawing code for SyntheticBoldDemo project. Text is 
drawn twice, once with and once without the synthetic bold. See
comments below for more detail. Also note this technique will work
on any CG-based text drawing code. ATSUI is not a requirement. In
this example, the API TXNDrawUnicodeTextBox is used, instead of
ATSUDrawText.

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

// Globals for just this source module
//
static ATSUStyle                        gStyle;
static UniChar                          *gText;
static UniCharCount                     gLength;
static Fixed                            gPointSize;
static ATSUFontID                       gFont;


// Sets the font
//
void SetATSUIStuffFont(ATSUFontID inFont)
{
    gFont = inFont;
}


// Sets the font size
//
void SetATSUIStuffFontSize(Fixed inSize)
{
    gPointSize = inSize;
}


// Updates the ATSUI style to the current font and size
//
void UpdateATSUIStyle(void)
{
    ATSUAttributeTag                    tags[2];
    ByteCount                           sizes[2];
    ATSUAttributeValuePtr               values[2];

    tags[0] = kATSUFontTag;
    sizes[0] = sizeof(ATSUFontID);
    values[0] = &gFont;
    
    tags[1] = kATSUSizeTag;
    sizes[1] = sizeof(Fixed);
    values[1] = &gPointSize;
    
    verify_noerr( ATSUSetAttributes(gStyle, 2, tags, sizes, values) );
}


// Sets up the text based on the specified CFString
//
void UpdateATSUIStuffString(CFStringRef string)
{
    free(gText);
    gLength = CFStringGetLength(string);
    gText = (UniChar *)malloc(gLength * sizeof(UniChar));
    CFStringGetCharacters(string, CFRangeMake(0, gLength), gText);
}


// Creates the ATSUI data
//
void SetUpATSUIStuff(void)
{    
    CFStringRef	string = CFSTR("Hello World!");

    verify_noerr( ATSUCreateStyle(&gStyle) );
    UpdateATSUIStyle();
    
    gLength = CFStringGetLength(string);
    gText = (UniChar *)malloc(gLength * sizeof(UniChar));
    CFStringGetCharacters(string, CFRangeMake(0, gLength), gText);
}


// Sets kATSUQDBoldfaceTag for the given style
//
void MySetBoldfaceTag(ATSUStyle iStyle)
{
    Boolean setToTrue = true;
    ATSUAttributeTag tag = kATSUQDBoldfaceTag;
    ByteCount size = sizeof(Boolean);
    ATSUAttributeValuePtr value = (ATSUAttributeValuePtr) &setToTrue;
    
    verify_noerr( ATSUSetAttributes(iStyle, 1, &tag, &size, &value) );
}


// Sets kATSUQDBoldfaceTag for the given style
//
void MyClearBoldfaceTag(ATSUStyle iStyle)
{
    ATSUAttributeTag tag = kATSUQDBoldfaceTag;

    verify_noerr( ATSUClearAttributes(iStyle, 1, &tag) );
}


// Checks the global preference to see if text at the specified point size
// will be drawn antialiased or not.
//
// Note that iSize is of type Fixed!
//
Boolean IsAntiAliased(Fixed iSize)
{
    Boolean keyExistsAndHasValidFormat;
    CFIndex value;

    value = CFPreferencesGetAppIntegerValue( CFSTR("AppleAntiAliasingThreshold"),
                                             kCFPreferencesCurrentApplication,
                                             &keyExistsAndHasValidFormat );

    if ( keyExistsAndHasValidFormat )
        return ( Fix2X(iSize) > value ); // 'value' is the maximum not-antialiasing size
    else
        return true;
}


// Draws the current ATSUI data.  Takes a GrafPtr as an argument so
// that it can handle printing as well as drawing into a window.
//
void DrawATSUIStuff(GrafPtr drawingPort)
{
    GrafPtr                             savedPort;
    Rect                                portBounds, quarterRect2, quarterRect3;
    float								windowHeight, quarter;
    CGContextRef						context;
    TXNTextBoxOptionsData				optionsData;
    Boolean								needToUseCGStrokeMethod;

    // Set up the GrafPort
    GetPort(&savedPort);
    SetPort(drawingPort);
    GetPortBounds(drawingPort, &portBounds);
    EraseRect(&portBounds);

    // Divide the window into vertical quarters, and draw the text in the middle two quarters
    windowHeight = portBounds.bottom - portBounds.top;
    quarter = windowHeight / 4.0;
    MacSetRect(&quarterRect2, portBounds.left, portBounds.top + quarter, portBounds.right, portBounds.bottom - (quarter * 2.0));
    FrameRect(&quarterRect2);
    MacSetRect(&quarterRect3, portBounds.left, portBounds.top + (quarter * 2.0), portBounds.right, portBounds.bottom - quarter);
    FrameRect(&quarterRect3);

    // Set up the CGContext
    if (gNewCG) QDBeginCGContext(drawingPort, &context); else CreateCGContextForPort(drawingPort, &context);

    // Setup the options to pass into TXNDrawUnicodeTextBox
    optionsData.optionTags = kTXNUseCGContextRefMask | kTXNSetFlushnessMask | kTXNUseFontFallBackMask;
    optionsData.flushness = X2Frac(0.5);   // Center the text horizontally, just for this demo.
    optionsData.options = (void *)context; // This parameter really needs to be renamed, see 3198383.

    // Draw the text once without the extr bold
    verify_noerr( TXNDrawUnicodeTextBox(gText, gLength, &quarterRect2, gStyle, &optionsData) );

    // ----------------------------------------------------------
    //
    // Here is where we change the setting to do the extra stroke
    // The value of gStrokeThicknessFactor determines how thick the extra stroke is.
    //   The "standard" value used by ATSUI is 0.024;
    //     this was changed to 0.044 for bug 3189696,
    //     and will probably be changed back, so if you
    //     want the extra stroke, you will have to do it
    //     manually, as is done below.
    //
    // The extra stroke method:
    //  - will look good on-screen when CG anti-aliasing is ON
    //  - will look good when printing
    //  - will *NOT* look good on-screen when CG anti-aliasing is OFF
    //     (just use kATSUQDBoldfaceTag in that case)
    //
    needToUseCGStrokeMethod = gCurrentlyPrinting || IsAntiAliased(gPointSize);
    if ( needToUseCGStrokeMethod ) {
        CGContextSaveGState(context);
        CGContextSetTextDrawingMode(context, kCGTextFillStroke);
        CGContextSetLineWidth(context, gStrokeThicknessFactor * Fix2X(gPointSize));
        // You might want to call CGContextSetStrokeColor() here,
        // just to make certain it is the same as the text/fill color.
    }
    else
        MySetBoldfaceTag(gStyle); // This will look very strong on-screen when CG anti-aliasing is off

    // Draw the text again with the extra bold for comparison
    verify_noerr( TXNDrawUnicodeTextBox(gText, gLength, &quarterRect3, gStyle, &optionsData) );

    // Undo the previous CG text mode setting
    if ( needToUseCGStrokeMethod )
        CGContextRestoreGState(context);
    else
        MyClearBoldfaceTag(gStyle);

    // Tear down the CGContext since we are done with it
    if (gNewCG) QDEndCGContext(drawingPort, &context); else CGContextRelease(context);    

    // Restore the GrafPort
    SetPort(savedPort);
}


// Disposes of the ATSUI data
//
void DisposeATSUIStuff(void)
{
    verify_noerr( ATSUDisposeStyle(gStyle) );
    free(gText);
}
