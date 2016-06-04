/*

File: findparagrph.c

Abstract: Contains helper code for breaking UTF-16 text into paragraphs.

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

// Finds a paragraph within a block of UniChar text, gives back
// the offset after the hard line break character or characters.
// Returns true if end of text is reached, false otherwise.
//
// theText          array of 16 bit Unicode chars
// theTextLength    should be the number of Unicode characters, NOT the number of bytes
// paragraphStart   ATSUI-style offset (i.e., "between" the array elements)
// paragraphEnd     ATSUI-style offset (i.e., "between" the array elements)
//
Boolean FindParagraph( UniChar *theText, UniCharCount theTextLength, UniCharArrayOffset paragraphStart, UniCharArrayOffset *paragraphEnd )
{
    UniChar         CR   = 0x000D;  // ASCII carrige return  '\r'
    UniChar         LF   = 0x000A;  // ASCII newline         '\n'
    UniChar         LSEP = 0x2028;  // Unicode line separator
    UniChar         PSEP = 0x2029;  // Unicode paragraph separator
    UniCharCount    currentPosition;
    UniChar         currentChar;
    Boolean         endOfText = false;

    // Check to see if we even have any text
    //
    if (theText == NULL) {
        *paragraphEnd = 0;
        return true;
    }

    // Loop over the text and check for hard line breaks
    //
    // There are five possbile sequences that constitute a hard line break:
    // (all five are considered valid)
    //
    //      LF      (Unix style)
    //      CR      (Mac style)
    //      CRLF    (Windows/DOS style)
    //      PSEP    (New Unicode style, used by BBEdit, WorldText, and TextEdit)
    //      LSEP    (Other new Unicode style)
    //
    for (currentPosition=paragraphStart; (currentPosition < theTextLength); currentPosition++) {

        currentChar = theText[currentPosition];

        if ( (currentChar == PSEP) || (currentChar ==  LSEP) || (currentChar ==  LF) ) {
            break;
        }

        if ( currentChar == CR ) {
            if ( currentPosition < (theTextLength - 1) ) {
                if ( theText[currentPosition + 1] == LF ) {     // Treat DOS/Windows style CRLF line breaks as a single entity
                    currentPosition++;
                }
            }
            break;
        }

    }

    // Special case -- if we reached the end of the text but didn't find a
    // separator, currentPostion will be one too many, so decrement it.
    //
    if (currentPosition == theTextLength) {
        currentPosition--;
    }

    // See if we went all the way to the end of the text
    //
    if (currentPosition == (theTextLength - 1)) {
        endOfText = true;
    }
    
    // This is an ATSUI-style offset (i.e., "between" the array elements),
    // so we have to return the offset that is AFTER the paragraph separator.
    // (Note that if we encounter a CRLF style break, we treat that a single entity; see above)
    //
    *paragraphEnd = currentPosition + 1;
    return endOfText;
}
