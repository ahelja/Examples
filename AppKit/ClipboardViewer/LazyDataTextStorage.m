/*
    LazyDataTextStorage.m
    Author: Ali Ozer
    Created: Feb 2002

    This file contains two classes:
    LazyDataString: A subclass of NSString whose backing store is an NSData; the "contents"
        of the string is the standard human-readable text dump of binary data.
    LazyDataTextStorage: Subclass of NSTextStorage which uses a LazyDataString as it's
        string backing store (and a fixed set of text attributes)
*/

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation,
 modification or redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject to these
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in
 this original Apple software (the "Apple Software"), to use, reproduce, modify and
 redistribute the Apple Software, with or without modifications, in source and/or binary
 forms; provided that if you redistribute the Apple Software in its entirety and without
 modifications, you must retain this notice and the following text and disclaimers in all
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "LazyDataTextStorage.h"


/* LazyDataString is a subclass of NSString whose backing store is an NSData; the "contents" of the string is the standard human-readable text dump of binary data. As all good NSString subclassers, LazyDataString implements length, characterAtIndex:, and getCharacters:range:. It's only init method is initWithData:.
*/
@interface LazyDataString : NSString {
    unsigned bytesPerLine;
    NSData *data;
}

// Set/get the NSData which provides the strings contents
- (id)initWithData:(NSData *)obj;
- (NSData *)data;

// String primitives + one more
- (unsigned)length;
- (unichar)characterAtIndex:(unsigned)index;
- (void)getCharacters:(unichar *)buffer range:(NSRange)range;

@end


@implementation LazyDataString

- (id)initWithData:(NSData *)obj {
    if (self = [super init]) {
        data = [obj copy];
        bytesPerLine = 10;
    }
    return self;
}

- (id)init {
    return [self initWithData:[NSData data]];
}

- (void)dealloc {
    [data release];
    [super dealloc];
}

/* Number of characters needed to show a line's worth of the data
*/
static int charsPerLine(int bytesPerLine) {
    return bytesPerLine * 4 + 8 + 3 + 1 + 1;	 /* 00000000:  00 00 00 00  abcd */
}

/* The human visible representation for the specified byte
*/
static unsigned char charRepresentation(unsigned char dataByte) {
    return ((dataByte >= 32 && dataByte < 127) || (dataByte >= (128 + 32) && dataByte < 255)) ? dataByte : '.';
}

/* The data backing store
*/
- (NSData *)data {
    return data;
}

/* The length of the string (in number of unichars)
*/
- (unsigned)length {
    unsigned dataLength = [data length];
    unsigned numLines = (dataLength + bytesPerLine - 1) / bytesPerLine;
    return charsPerLine(bytesPerLine) * numLines;
}

/* Character at given index. Note that typically characterAtIndex: and length are the primitives; but here we implement getCharacters:range:, and have characterAtIndex: call that. This makes characterAtIndex: pretty inefficient overall, but it's not called very often.
*/
- (unichar)characterAtIndex:(unsigned)index {
    unichar buffer;
    [self getCharacters:&buffer range:NSMakeRange(index, 1)];
    return buffer;
}

/* This computes and returns the human-readable representation for a subset of the backing data. It first maps the specified range (in "string space") to the range in the data, then generates lines one at a time, copying the contents to the provided buffer. Because it is the responsibility of the caller to provide enough buffer space and assure range is valid, doesn't do any checks on the input parameters.
*/
- (void)getCharacters:(unichar *)buffer range:(NSRange)range {
    int cpl = charsPerLine(bytesPerLine);
    NSMutableString *mStr = [[NSMutableString alloc] initWithCapacity:cpl];
    const unsigned char *bytes = [data bytes];
    int byteLoc = (range.location / cpl) * bytesPerLine;	// loc in terms of bytes in data, not chars in string
    int byteEnd = ((NSMaxRange(range) + cpl - 1) / cpl) * bytesPerLine;	// same

    if (byteEnd > [data length]) byteEnd = [data length];

    // Do line at a time
    while (byteLoc < byteEnd) {
        int numBytesOnThisLine = (byteEnd - byteLoc < bytesPerLine) ? (byteEnd - byteLoc) : bytesPerLine;
        NSRange processedRange;
        int i;

        // Compute string for the whole line
        [mStr appendFormat:@"%08d: ", byteLoc];
        for (i = 0; i < numBytesOnThisLine; i++) [mStr appendFormat:@" %02x", bytes[byteLoc + i]];
        for (; i < bytesPerLine; i++) [mStr appendString:@"   "];
        [mStr appendString:@"  "];
        for (i = 0; i < numBytesOnThisLine; i++) [mStr appendFormat:@"%c", charRepresentation(bytes[byteLoc + i])];
        for (; i < bytesPerLine; i++) [mStr appendString:@" "];

        // Now, compute the processedRange, and intersect with the provided range (to deal with first and last lines properly)
        processedRange.location = (byteLoc / bytesPerLine) * cpl;
        processedRange.length = cpl;
        processedRange = NSIntersectionRange(processedRange, range);
        processedRange.location -= (byteLoc / bytesPerLine) * cpl;

        [mStr appendFormat:@"\n"];

        // Copy the required range to the output
        [mStr getCharacters:buffer range:processedRange];

        // Clear line; also increment loop variables
        [mStr setString:@""];
        byteLoc += numBytesOnThisLine;
        buffer += processedRange.length;
    }

    [mStr release];
}

@end


/* LazyDataTextStorage is a subclass of NSTextStorage which uses a LazyDataString as it's string backing store (and a fixed set of text attributes). Note that because this class treats its contents as immutable, it overrides the standard NSTextStorage mutation methods to do nothing.
*/
@implementation LazyDataTextStorage

- (id)init {
    if (self = [super init]) {
        myString = [[LazyDataString alloc] init];
        myAttributes = [[NSDictionary dictionary] copy];
    }
    return self;
}

- (void)dealloc {
    [myString release];
    [myAttributes release];
    [super dealloc];
}

/* These two methods are the way to change the string and attributes of this text storage
We assume non-editable backing store; in addition, the same attributes apply to the whole string.
*/
- (void)setAttributes:(NSDictionary *)attrs {
    if (myAttributes != attrs) {
        [myAttributes release];
        myAttributes = [attrs retain];
        [self edited:NSTextStorageEditedAttributes range:NSMakeRange(0, [self length]) changeInLength:0];
    }
}

- (void)setString:(NSString *)string {
    if (myString != string) {
        unsigned origLength = [self length];
        [myString release];
        myString = [string retain];
        [self edited:NSTextStorageEditedCharacters range:NSMakeRange(0, origLength) changeInLength:[self length] - origLength];
    }
}

- (NSString *)string {
    return myString;
}

- (void)setData:(NSData *)data {
    LazyDataString *newString = [[LazyDataString alloc] initWithData:data];
    [self setString:newString];
    [newString release];
}

- (NSData *)data {
    return [(LazyDataString *)myString data];
}

/* Primitve for returning the attributes; we just have a fixed set, so the result is easy...
*/
- (NSDictionary *)attributesAtIndex:(unsigned)location effectiveRange:(NSRangePointer)range {
    if (range) *range = NSMakeRange(0, [self length]);
    return myAttributes;
}

/* The actual mutable primitives, the two below, don't do anything. This means this text storage is really not editable. We generate a warning just to  make sure these are never called...
*/
- (void)replaceCharactersInRange:(NSRange)range withString:(NSString *)str {
    NSLog (@"Attempt to edit characters!");
}

- (void)setAttributes:(NSDictionary *)attrs range:(NSRange)range {
    NSLog (@"Attempt to edit attributes!");
}

/* Do nothing; because we're not editable, no need to try to fix (and, in fact, trying to fix attributes might cause trouble if the fixing causes some changes. Note that what this means is we need to make sure that the attributes assigned to the text are always valid.
*/
- (void)fixAttributesInRange:(NSRange)range {
}


@end

