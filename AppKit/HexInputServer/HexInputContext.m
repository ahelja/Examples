/*
 HexInputContext.m
 Copyright (c) 1998-2004, Apple Computer, Inc.
*/
/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
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

#import <AppKit/AppKit.h>
#import "HexInputContext.h"
#import "HexInputServer.h"

// Maps Unicode character (compatible to ASCII anyway) to corresponding value
#define UNICHAR_TO_NUMBER(theUnichar) ((theUnichar <= '9') ? theUnichar - '0' : (theUnichar < 'a' ? theUnichar - 'A' : theUnichar - 'a') + 0xA)

// Character set of "1234567890abcdfeABCDEF"
static NSCharacterSet *_HexadecimalDigitCharacterSet = nil;

// Character set of all Unicode printable chars (invert of control character set)
static NSCharacterSet *_PrintableCharacterSet = nil;

@implementation HexInputContext
/* Private Methods
*/
// This method is the guts of visual feedback.  It sets marked range & display aux window.
- (void)_updateFeedbackWithValue:(unsigned int)currentValue {
    NSTextField *textField = [HexInputServer inputFeedbackField];
    NSWindow *window = [textField window];
    NSRect frame;
    BOOL hasMark = NO;

    if (currentValue) { // Greater than zero
        unichar currentChar = currentValue;

        if ([_PrintableCharacterSet characterIsMember:currentChar]) { // Printable character.  Send the character code
            [_client setMarkedText:[NSString stringWithCharacters:&currentChar length:1] selectedRange:NSMakeRange(1, 0)];
            hasMark = YES;
        } else {
            [_client setMarkedText:@"" selectedRange:NSMakeRange(0, 0)]; // Reset marked length & unmark it
        }

        [textField setStringValue:[NSString stringWithFormat:@"0x%X", currentChar]];
    } else { // Value is zero
        [_client setMarkedText:@"" selectedRange:NSMakeRange(0, 0)]; // Reset marked length & unmark it
        [textField setStringValue:@"0x"];
    }

    // Re-calculate aux window origin & size.
    [textField sizeToFit];
    frame = [textField frame];
    [window setContentSize:frame.size];
    frame = [window frame];
    frame.origin = ([_client firstRectForCharacterRange:(hasMark ? [_client markedRange] : [_client selectedRange])]).origin;
    frame.origin.y -= frame.size.height;
    [window setFrameOrigin:frame.origin];
    [window orderFront:nil];
}

// This method implements actual logic for key binding commands. It redirect the commands to _client if no conversion in progress.
- (void)_performCommandForSelector:(SEL)aSelector {
    unsigned int currentValue;

    if (NSMapMember(_conversationTable, (const void*)_currentConversation, (void**)&_currentConversation, (void**)&currentValue)) { // _currentConversation is in the table.  Conversion is in progress.
        if (aSelector == @selector(insertNewline:)) {
            [_client unmarkText];
            NSMapRemove(_conversationTable, (const void*)_currentConversation);
            [self updateUserFeedbackWithState:NO];
        } else {
            if (aSelector == @selector(deleteBackward:)) {
                currentValue >>= 4; // One Hexadigit shift
            } else if (aSelector == @selector(moveForward:)) {
                currentValue++;
            } else if (aSelector == @selector(moveBackward:)) {
                currentValue--;
            } else if (aSelector == @selector(moveUp:)) {
                currentValue -= 0x100;
            } else if (aSelector == @selector(moveDown:)) {
                currentValue += 0x100;
            } else if (aSelector == @selector(moveToBeginningOfParagraph:)) {
                currentValue &= 0xFF00;
            } else if (aSelector == @selector(moveToEndOfParagraph:)) {
                currentValue |= 0xFF;
            }
            NSMapInsert(_conversationTable, (const void*)_currentConversation, (const void*)currentValue);
            [self _updateFeedbackWithValue:currentValue];
        }
    } else { // No conversation is in progress. Just send it back.
        [_client doCommandBySelector:aSelector];
    }
}

/* Initialization
*/
+ (void)initialize {
    if (!_HexadecimalDigitCharacterSet) {
        NSMutableCharacterSet *mutableSet = [[NSMutableCharacterSet controlCharacterSet] mutableCopy];

        [mutableSet formUnionWithCharacterSet:[NSCharacterSet nonBaseCharacterSet]];
        [mutableSet addCharactersInRange:NSMakeRange(0xD800, 0xE000 - 0xD800)]; // Surrogate range
        [mutableSet invert]; // character set excluding control, surrogates, & non-base (combining) chars

        _PrintableCharacterSet = mutableSet;
        _HexadecimalDigitCharacterSet = [[NSCharacterSet characterSetWithCharactersInString:@"1234567890abcdfeABCDEF"] retain];
    }
}

/* Factory methods
*/
+ (HexInputContext *)hexInputContextWithClient:(id <NSTextInput>)theClient {
    return [[[HexInputContext alloc] initWithClient:theClient] autorelease];
}

- (id)initWithClient:(id <NSTextInput>)theClient {
    [super init];

    _client = [(id)theClient retain];
    _conversationTable = NSCreateMapTableWithZone(NSIntMapKeyCallBacks, NSIntMapValueCallBacks, 10, [self zone]);

    return self;
}

/* Deallocation
*/
- (void)dealloc {
    [_client release];
    NSFreeMapTable(_conversationTable);

    [super dealloc];
}

/* Accessor Methods
*/
- (long)currentConversation {
    return _currentConversation;
}

- (void)setCurrentConversation:(long)theConversation {
    _currentConversation = theConversation;

    [self updateUserFeedbackWithState:YES];
}

- (BOOL)isHexInputMode {
    return NSMapMember(_conversationTable, (const void*)_currentConversation, (void**)NULL, (void**)NULL);
}

- (id)client {
    return _client;
}

/* Client interactions
*/
// This method processes aString sent from client.  Send back it if no conversion.
- (void)insertText:(NSString *)aString {
    unsigned int currentValue;

    if (NSMapMember(_conversationTable, (const void*)_currentConversation, (void**)&_currentConversation, (void**)&currentValue)) { // Conversion is in progress. Process the incoming string
        unsigned int length = [aString length];
        unichar inputChars[length + 1]; // Note dynamic array allocation is GNU extension
        unichar outputBuffer[length];
        unsigned int index;
        unsigned int outputIndex = 0;

        [aString getCharacters:inputChars];
        for (index = 0;index < length;index++) {
            if ([_HexadecimalDigitCharacterSet characterIsMember:inputChars[index]]) {
                unsigned char newDigit = UNICHAR_TO_NUMBER(inputChars[index]);

                currentValue = (currentValue << 4) | newDigit;

                if (currentValue > 0xFFFF) {
                    if ([_PrintableCharacterSet characterIsMember:currentValue >> 4]) {
                        outputBuffer[outputIndex++] = currentValue >> 4;
                    }
                    currentValue = newDigit;
                }
            } else {
                if ([_PrintableCharacterSet characterIsMember:currentValue]) {
                    outputBuffer[outputIndex++] = currentValue;
                }
                currentValue = 0;
            }
        }

        if (outputIndex > 0) {
            [_client setMarkedText:@"" selectedRange:NSMakeRange(0, 0)];
            [_client insertText:[NSString stringWithCharacters:outputBuffer length:outputIndex]];
        }


        NSMapInsert(_conversationTable, (const void*)_currentConversation, (const void*)currentValue);

        [self _updateFeedbackWithValue:currentValue];
    } else {
        [_client insertText:aString];
    }
}

// Dispatch key binding commands to appropriate destination
- (void)doCommandBySelector:(SEL)aSelector {
    if ([self respondsToSelector:aSelector]) {
        [self performSelector:aSelector withObject:self];
    } else {
        [_client unmarkText];
        [self abandonCurrentConversation];
        [_client doCommandBySelector:aSelector];
    }
}

// Discards conversion data
- (void)abandonCurrentConversation {
    NSMapRemove(_conversationTable, (const void*)_currentConversation);
    [self updateUserFeedbackWithState:NO];
}

// Public interface method for updating user Feedback
- (void)updateUserFeedbackWithState:(BOOL)flag {
    unsigned int currentValue;

    if (flag && NSMapMember(_conversationTable, (const void*)_currentConversation, (void**)&_currentConversation, (void**)&currentValue)) {
        [self _updateFeedbackWithValue:currentValue];
    } else {
        [[[HexInputServer inputFeedbackField] window] orderOut:self];
    }
}

/* KeyBinding selectors
*/
// This key binding command starts conversion. The default binding is Ctrl+X.
- (void)beginHexInput:(id)sender {
    NSMapInsert(_conversationTable, (const void*)_currentConversation, (const void*)0);
    [self _updateFeedbackWithValue:0];
}

- (void)insertNewline:(id)sender {
    [self _performCommandForSelector:_cmd];
}

- (void)deleteBackward:(id)sender {
    [self _performCommandForSelector:_cmd];
}

- (void)moveForward:(id)sender {
    [self _performCommandForSelector:_cmd];
}

- (void)moveBackward:(id)sender {
    [self _performCommandForSelector:_cmd];
}

- (void)moveUp:(id)sender {
    [self _performCommandForSelector:_cmd];
}

- (void)moveDown:(id)sender {
    [self _performCommandForSelector:_cmd];
}

- (void)moveToBeginningOfParagraph:(id)sender {
    [self _performCommandForSelector:_cmd];
}

- (void)moveToEndOfParagraph:(id)sender {
    [self _performCommandForSelector:_cmd];
}

@end
