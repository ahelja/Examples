//
//  BlotApp.m
//  Blot
//
//  Created by John Sullivan on Mon Jan 19 2004.
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import "BlotApp.h"
#import "BlotDocument.h"

#import <WebKit/WebArchive.h>
#import <WebKit/WebResource.h>

#import "BlotPreferences.h"

@implementation BlotApp

+ (void)initialize
{
#if 0
    // Keep around for when (if?) we need to install some defaults.
    if (self == [BlotApp class]) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [[NSUserDefaults standardUserDefaults] registerDefaults:dict];
    }
#endif
}

- (void)awakeFromNib
{
    NSAppleEventManager *eventManager = [NSAppleEventManager sharedAppleEventManager];
    [eventManager setEventHandler:self
                      andSelector:@selector(handleURLEvent:withReplyEvent:)
                    forEventClass:kInternetEventClass
                       andEventID:(AEEventID)kAEGetURL];
    [eventManager setEventHandler:self
                      andSelector:@selector(handleURLEvent:withReplyEvent:)
                    forEventClass:'WWW!'
                       andEventID:'OURL'];
    [eventManager setEventHandler:self
                      andSelector:@selector(handleMailPageEvent:withReplyEvent:)
                    forEventClass:'mail'
                       andEventID:'mlpg'];
    [eventManager setEventHandler:self
                      andSelector:@selector(handleMailLinkEvent:withReplyEvent:)
                    forEventClass:'mail'
                       andEventID:'mllk'];
}

- (void)handleURLEvent:(NSAppleEventDescriptor *)event withReplyEvent: (NSAppleEventDescriptor *)replyEvent
{
    // FIXME: Report errors (3193872).
    NSString *URLString = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
    if ([URLString length]) {
        NSURL *URL = [NSURL URLWithString:URLString];
        if (URL) {
            BlotDocument *document = [[NSDocumentController sharedDocumentController] openUntitledDocumentOfType:HTMLDocumentType display:YES];
            [document goToURL:URL];
        }
    }
}

- (void)handleMailPageEvent:(NSAppleEventDescriptor *)event withReplyEvent: (NSAppleEventDescriptor *)replyEvent
{
    NSData *data = [[event paramDescriptorForKeyword:keyDirectObject] data];
    if (!data) {
        // Report error.
        return;
    }
    
    WebArchive *archive = [[WebArchive alloc] initWithData:data];
    if (!archive) {
        // Report error.
        return;
    }
        
    BlotDocument *document = [[NSDocumentController sharedDocumentController] openUntitledDocumentOfType:HTMLDocumentType display:YES];
    [document loadArchive:archive];
    [archive release];
}

- (void)handleMailLinkEvent:(NSAppleEventDescriptor *)event withReplyEvent: (NSAppleEventDescriptor *)replyEvent
{
    NSString *URLString = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
    if (!URLString) {
        // Report error.
        return;
    }
    
    BlotDocument *document = [[NSDocumentController sharedDocumentController] openUntitledDocumentOfType:HTMLDocumentType display:YES];
    [document insertLinkWithURLString:URLString];
}

- (IBAction)showPreferences:(id)sender
{
    [[BlotPreferences sharedPreferences] showWindow:sender];
}

- (IBAction)debugShowOperations:(id)sender
{
    static BOOL setUpOperations = NO;
    
    if (!setUpOperations) {
        const char *operationNames[] = {
            "alignCenter",
            "alignJustified",
            "alignLeft",
            "alignRight",
            "capitalizeWord",
            "centerSelectionInVisibleArea",
            "changeCaseOfLetter",
            "checkSpelling",
            "complete",
            "copy",
            "copyFont",
            "cut",
            "delete",
            "deleteBackward",
            "deleteBackwardByDecomposingPreviousCharacter",
            "deleteForward",
            "deleteToBeginningOfLine",
            "deleteToBeginningOfParagraph",
            "deleteToEndOfLine",
            "deleteToEndOfParagraph",
            "deleteToMark",
            "deleteWordBackward",
            "deleteWordForward",
            "ignoreSpelling",
            "indent",
            "insertBacktab",
            "insertLineBreak",
            "insertNewline",
            "insertNewlineIgnoringFieldEditor",
            "insertParagraphSeparator",
            "insertTab",
            "insertTabIgnoringFieldEditor",
            "insertTable",
            "lowercaseWord",
            "moveBackward",
            "moveBackwardAndModifySelection",
            "moveDown",
            "moveDownAndModifySelection",
            "moveForward",
            "moveForwardAndModifySelection",
            "moveLeft",
            "moveLeftAndModifySelection",
            "moveParagraphBackwardAndModifySelection",
            "moveParagraphForwardAndModifySelection",
            "moveRight",
            "moveRightAndModifySelection",
            "moveToBeginningOfDocument",
            "moveToBeginningOfDocumentAndModifySelection",
            "moveToBeginningOfLine",
            "moveToBeginningOfLineAndModifySelection",
            "moveToBeginningOfParagraph",
            "moveToBeginningOfParagraphAndModifySelection",
            "moveToEndOfDocument",
            "moveToEndOfDocumentAndModifySelection",
            "moveToEndOfLine",
            "moveToEndOfLineAndModifySelection",
            "moveToEndOfParagraph",
            "moveToEndOfParagraphAndModifySelection",
            "moveUp",
            "moveUpAndModifySelection",
            "moveWordBackward",
            "moveWordBackwardAndModifySelection",
            "moveWordForward",
            "moveWordForwardAndModifySelection",
            "moveWordLeft",
            "moveWordLeftAndModifySelection",
            "moveWordRight",
            "moveWordRightAndModifySelection",
            "outline",
            "pageDown",
            "pageDownAndModifySelection",
            "pageUp",
            "pageUpAndModifySelection",
            "paste",
            "pasteAsPlainText",
            "pasteAsRichText",
            "pasteFont",
            "scrollLineDown",
            "scrollLineUp",
            "scrollPageDown",
            "scrollPageUp",
            "selectAll",
            "selectLine",
            "selectParagraph",
            "selectToMark",
            "selectWord",
            "setMark",
            "showGuessPanel",
            "startSpeaking",
            "stopSpeaking",
            "subscript",
            "superscript",
            "swapWithMark",
            "takeFindStringFromSelection",
            "toggleContinuousSpellChecking",
            "toggleSmartInsertDelete",
            "transpose",
            "transposeWords",
            "underline",
            "unscript",
            "uppercaseWord",
            "yank",
            "yankAndSelect",
            NULL
        };

        NSFont *font = [NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSMiniControlSize]];
        NSDictionary *attributes = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];

        float maxWidth = 0;
        int i;
        for (i = 0; operationNames[i]; ++i) {
            NSString *string = [[NSString stringWithUTF8String:operationNames[i]] stringByAppendingString:@":"];
            float width = [string sizeWithAttributes:attributes].width;
            maxWidth = MAX(maxWidth, width);
        }
        maxWidth += 24;

        int columnHeight = (i + 2) / 3;

        NSView *superview = [operationsPanel contentView];

        [operationsPanel setContentSize:NSMakeSize(3 * maxWidth, columnHeight * 16 + 1)];

        float firstY = NSMaxY([superview frame]) - 1;
        float y = firstY;
        float x = 0;
        for (i = 0; operationNames[i]; ++i) {
            NSString *string = [[NSString stringWithUTF8String:operationNames[i]] stringByAppendingString:@":"];
            y -= 16;
            if (y < 0) {
                y = firstY - 16;
                x += maxWidth;
            }
            NSButton *button = [[NSButton alloc] initWithFrame:NSMakeRect(x, y, maxWidth, 16)];
            [button setBezelStyle:NSRoundedBezelStyle];
            [[button cell] setControlSize:NSMiniControlSize];
            [button setFont:font];
            [button setTitle:string];
            [button setAction:NSSelectorFromString(string)];
            [superview addSubview:button];
        }
        
        [operationsPanel center];
        [operationsPanel setFloatingPanel:YES];
        [operationsPanel setBecomesKeyOnlyIfNeeded:YES];

        setUpOperations = YES;
    }

    [operationsPanel orderFront:nil];
}

@end
