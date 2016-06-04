/*
 HexInputServer.m
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
#import "HexInputServer.h"
#import "HexInputContext.h"
#import "HexInputModePalette.h"

// Converts client object's address to NSNumber
#define KEY_FROM_CLIENTID(theClient) ([NSNumber numberWithUnsignedInt:(unsigned int)theClient])
// UI settings for inputFeedbackField.
#define FEEDBACK_FIELD_FONT_SIZE (12.0)
#define NSInputCandidateWindowLevel (NSModalPanelWindowLevel + 1) // It must work with modal panels

// Class variable holding sharedInstance;
static HexInputServer *_SharedHexInputServerInstance = nil;

// Class variable holding textfiled for hex value display
static NSTextField *_InputFeedbackField = nil;

// TextField used as inputFeedbackField
@interface HexInputFeedbackField : NSTextField
@end

@implementation HexInputFeedbackField
- initWithFrame:(NSRect)theRect {
    [super initWithFrame:theRect];
    [self setEditable:NO];
    [self setSelectable:NO];
    [self setFont:[NSFont messageFontOfSize:FEEDBACK_FIELD_FONT_SIZE]];
    return self;
}

// acceptsFirstResponder == NO prevents from the window activate HexInputServer application
- (BOOL)acceptsFirstResponder { return NO; }
@end

@implementation HexInputServer
/* Class methods
*/
// Shared instance
+ (id)sharedInstance {
    if (!_SharedHexInputServerInstance) {
        _SharedHexInputServerInstance = [[HexInputServer alloc] init];
    }

    return _SharedHexInputServerInstance;
}

// Conversion feedback textfield
+ (NSTextField *)inputFeedbackField {
    if (!_InputFeedbackField) {
        NSPanel *panel = [[NSPanel allocWithZone:[self zone]] initWithContentRect:NSMakeRect(0, 0, 100, 40) styleMask:NSBorderlessWindowMask|NSUtilityWindowMask backing:NSBackingStoreBuffered defer:YES];

        [panel setHidesOnDeactivate:NO];
        [panel setFloatingPanel:YES];
        [panel setBecomesKeyOnlyIfNeeded:YES];

        _InputFeedbackField = [[[HexInputFeedbackField allocWithZone:[self zone]] initWithFrame:NSMakeRect(0, 0, 60, 24)] autorelease];
        [panel setContentView:_InputFeedbackField];
    }

    return _InputFeedbackField;
}

// input mode palette initialization
- (HexInputModePalette *)inputModePalette {
    if (!_hexInputModePalette) {
        [NSBundle loadNibNamed:@"HexInputModePalette" owner:self];
    }
    return _hexInputModePalette;
}

/* Deallocation
*/
- (void)dealloc {
    [_clientTable release];

    [super dealloc];
}

/* Mode settings
 */
- (void)setCurrentInputContextModeToHex:(BOOL)flag {
    [self doCommandBySelector:(flag ? @selector(beginHexInput:) : @selector(insertNewline:)) client:[_currentClientContext client]];
}

/* NSInputServiceProivder protocol methods
*/
- (BOOL)canBeDisabled {
    return YES; // This server can be disabled (it just send back messages when disabled)
}

- (BOOL)wantsToInterpretAllKeystrokes {
    return NO; // This server is using key binding manager
}

- (BOOL)wantsToHandleMouseEvents {
    return NO; // This server handles single character input, so no mouse support needed
}

- (BOOL)wantsToDelayTextChangeNotifications {
    return NO; // This server handles simple single character input, so no need to delay the notification
}

// Pass aString to sender's corresponding context if it's, indeed, "active" conversation.
- (void) insertText:(id)aString client:(id)sender {
    HexInputContext *context = [_clientTable objectForKey:KEY_FROM_CLIENTID(sender)];

    if (!_isEnabled || (_currentClientContext != context) || ([sender conversationIdentifier] != [context currentConversation])) { // This means sender is either NSInputText non-conformant simple client or non keyboard-focused view. Just send it back.
        [sender insertText:aString];
    } else {
        [context insertText:aString];
    }
}

// Pass aSelector to sender's corresponding context if it's, indeed, "active" conversation.
- (void) doCommandBySelector:(SEL)aSelector client:(id)sender {
    HexInputContext *context = [_clientTable objectForKey:KEY_FROM_CLIENTID(sender)];

    if (!_isEnabled || (_currentClientContext != context) || ([sender conversationIdentifier] != [context currentConversation])) { // This means sender is either NSInputText non-conformant simple client or non keyboard-focused view. Just send it back.
        [sender doCommandBySelector:aSelector];
    } else {
        [context doCommandBySelector:aSelector];
        if ((aSelector == @selector(beginHexInput:)) || (aSelector == @selector(insertNewline:))) { // Those two commands switches input states (alphabet/hex).  Update the status menu states.
            [[self inputModePalette] updateModePaletteWithState:[context isHexInputMode]];
        }
    }
}

// Send the message to sender's corresponding context.
- (void) markedTextAbandoned:(id)sender {
    [[_clientTable objectForKey:KEY_FROM_CLIENTID(sender)] abandonCurrentConversation];
}

// Unmark it & abandon it
- (void) markedTextSelectionChanged:(NSRange)newSel client:(id)sender {
    [sender unmarkText];
    [self markedTextAbandoned:sender];
}

// Client app is either exited or died
- (void) terminate:(id)sender {
    HexInputContext *context;
    id key = KEY_FROM_CLIENTID(sender);

    if ((context = [_clientTable objectForKey:key]) == _currentClientContext) { // if it's currect context, clear the state
        [[_InputFeedbackField window] orderOut:self];
        _currentClientContext = nil;
    }
    [_clientTable removeObjectForKey:key];
}

// sender became the active application using this server for input
- (void) inputClientBecomeActive:(id)sender {
    HexInputContext *context;
    id key = KEY_FROM_CLIENTID(sender);

    if (!(context = [_clientTable objectForKey:key])) { // If first time, register it in _clientTable. (It will retain the client proxy)
        context = [HexInputContext hexInputContextWithClient:sender];

        if (!_clientTable) {
            _clientTable = [[NSMutableDictionary allocWithZone:[self zone]] initWithCapacity:10];
        }
        [_clientTable setObject:context forKey:key];
    }

    _currentClientContext = context;

    // Show the mode palette
    [[self inputModePalette] orderFront:self];
}

// sender resigned the active application state or changed input manager
- (void) inputClientResignActive:(id)sender {
    _currentClientContext = nil;

    // Close the mode palette
    [[self inputModePalette] orderOut:self];
}

// a NSView conforming to NSTextInput within sender now has keyboard focus
- (void) inputClientEnabled:(id)sender {
    HexInputContext *context = [_clientTable objectForKey:KEY_FROM_CLIENTID(sender)];

    _isEnabled = YES;
    [context updateUserFeedbackWithState:_isEnabled]; // orderFront & reposition aux window showing hex value

    // Update mode palette & enable it
    [[self inputModePalette] updateModePaletteWithState:[context isHexInputMode]];
    [[self inputModePalette] setEnabled:_isEnabled];
}

// sender does not have valid (NSTextInput conforming) view as first responder
- (void) inputClientDisabled:(id)sender {
    _isEnabled = NO;
    [[_clientTable objectForKey:KEY_FROM_CLIENTID(sender)] updateUserFeedbackWithState:_isEnabled]; // orderOut aux window

    // Disable the mode palette
    [[self inputModePalette] setEnabled:_isEnabled];
}

// sender's keyboard focus is about to change
- (void) activeConversationWillChange:(id)sender fromOldConversation:(long)oldConversation {
    [[_InputFeedbackField window] orderOut:self];
}

// sender has new keyboard focus view
- (void) activeConversationChanged:(id)sender toNewConversation:(long)newConversation {
    HexInputContext *context = [_clientTable objectForKey:KEY_FROM_CLIENTID(sender)];

    [context setCurrentConversation:newConversation]; // Sets current conversation identifier & calls updateUserFeedbackWithState: internally
    [[self inputModePalette] updateModePaletteWithState:[context isHexInputMode]]; // Update status item states
}

@end
