/*
 HexInputContext.h
*/

#import <AppKit/NSInputManager.h>

// This class represents conversion context for each client applications
@interface HexInputContext : NSObject {
    id _client; // Holds the real client object
    NSMapTable *_conversationTable; // Mapping table for conversationIdentifier/in-progress conversion hex value
    long _currentConversation; // Current conversationIdentifier
}

// Factory methods
+ (HexInputContext *)hexInputContextWithClient:(id <NSTextInput>)theClient;
- (id)initWithClient:(id <NSTextInput>)theClient;

// Accessor methods
- (long)currentConversation;
- (void)setCurrentConversation:(long)theConversation;
- (BOOL)isHexInputMode;
- (id)client;

// Actual conversion methods
- (void)insertText:(NSString *)aString;
- (void)doCommandBySelector:(SEL)aSelector;
- (void)abandonCurrentConversation;

// Updates user feedback (i.e. aux window).  Updates feedback with current hex value if flag is YES.  Closes aux window when NO.
- (void)updateUserFeedbackWithState:(BOOL)flag;

// Keyboard command methods called from doCommandBySelector:
- (void)beginHexInput:(id)sender;
- (void)insertNewline:(id)sender;
- (void)deleteBackward:(id)sender;
- (void)moveForward:(id)sender;
- (void)moveBackward:(id)sender;
- (void)moveUp:(id)sender;
- (void)moveDown:(id)sender;
- (void)moveToBeginningOfParagraph:(id)sender;
- (void)moveToEndOfParagraph:(id)sender;

@end
