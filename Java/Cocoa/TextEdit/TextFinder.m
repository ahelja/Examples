/*
        TextFinder.m
        Copyright (c) 1995-2005 by Apple Computer, Inc., all rights reserved.
        Author: Ali Ozer

        Find and replace functionality with a minimal panel...
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
#import "TextFinder.h"

NSString *FindStringChangedNotification = @"Find Selection Changed Notification";

@implementation TextFinder

static id sharedFindObject = nil;

+ (id)sharedInstance {
    if (!sharedFindObject) {
        [[self allocWithZone:[[NSApplication sharedApplication] zone]] init];
    }
    return sharedFindObject;
}

- (id)init {
    if (sharedFindObject) {
        [super dealloc];
        return sharedFindObject;
    }

    if (!(self = [super init])) return nil;

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(appDidActivate:) name:NSApplicationDidBecomeActiveNotification object:[NSApplication sharedApplication]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(addWillDeactivate:) name:NSApplicationWillResignActiveNotification object:[NSApplication sharedApplication]];

    [self setFindString:@""];
    [self loadFindStringFromPasteboard];

    sharedFindObject = self;
    return self;
}

- (void)appDidActivate:(NSNotification *)notification {
    [self loadFindStringFromPasteboard];
}

- (void)addWillDeactivate:(NSNotification *)notification {
    [self loadFindStringToPasteboard];
}

- (void)loadFindStringFromPasteboard {
    NSPasteboard *pasteboard = [NSPasteboard pasteboardWithName:NSFindPboard];
    if ([[pasteboard types] containsObject:NSStringPboardType]) {
        NSString *string = [pasteboard stringForType:NSStringPboardType];
        if (string && [string length]) {
            [self setFindString:string];
            findStringChangedSinceLastPasteboardUpdate = NO;
        }
    }
}

- (void)loadFindStringToPasteboard {
    NSPasteboard *pasteboard = [NSPasteboard pasteboardWithName:NSFindPboard];
    if (findStringChangedSinceLastPasteboardUpdate) {
        [pasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
        [pasteboard setString:[self findString] forType:NSStringPboardType];
	findStringChangedSinceLastPasteboardUpdate = NO;
    }
}

- (void)loadUI {
    if (!findTextField) {
        if (![NSBundle loadNibNamed:@"FindPanel" owner:self])  {
            NSLog(@"Failed to load FindPanel.nib");
            NSBeep();
        }
	if (self == sharedFindObject) [[findTextField window] setFrameAutosaveName:@"Find"];
    }
    [findTextField setStringValue:[self findString]];
}

- (void)dealloc {
    if (self != sharedFindObject) {
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        [findString release];
        [super dealloc];
    }
}

- (NSString *)findString {
    return findString;
}

- (void)setFindString:(NSString *)string {
    if ([string isEqualToString:findString]) return;
    [findString autorelease];
    findString = [string copyWithZone:[self zone]];
    if (findTextField) {
        [findTextField setStringValue:string];
        [findTextField selectText:nil];
    }
    findStringChangedSinceLastPasteboardUpdate = YES;
    [[NSNotificationCenter defaultCenter] postNotificationName:FindStringChangedNotification object:findString];
}

- (NSTextView *)textObjectToSearchIn {
    id obj = [[NSApp mainWindow] firstResponder];
    return (obj && [obj isKindOfClass:[NSTextView class]]) ? obj : nil;
}

- (NSPanel *)findPanel {
    if (!findTextField) [self loadUI];
    return (NSPanel *)[findTextField window];
}

/* The primitive for finding; this ends up setting the status field (and beeping if necessary)...
*/
- (BOOL)find:(BOOL)direction {
    NSTextView *text = [self textObjectToSearchIn];
    lastFindWasSuccessful = NO;
    if (text) {
        NSString *textContents = [text string];
        unsigned textLength;
        if (textContents && (textLength = [textContents length])) {
            NSRange range;
            unsigned options = 0;
	    if (direction == Backward) options |= NSBackwardsSearch;
            if ([ignoreCaseButton state]) options |= NSCaseInsensitiveSearch;
            range = [textContents findString:[self findString] selectedRange:[text selectedRange] options:options wrap:YES];
            if (range.length) {
                [text setSelectedRange:range];
                [text scrollRangeToVisible:range];
                lastFindWasSuccessful = YES;
            }
        }
    }
    if (!lastFindWasSuccessful) {
        NSBeep();
        [statusField setStringValue:NSLocalizedStringFromTable(@"Not found", @"FindPanel", @"Status displayed in find panel when the find string is not found.")];
    } else {
        [statusField setStringValue:@""];
    }
    return lastFindWasSuccessful;
}

- (void)orderFrontFindPanel:(id)sender {
    NSPanel *panel = [self findPanel];
    [findTextField selectText:nil];
    [panel makeKeyAndOrderFront:nil];
}

/**** Action methods for gadgets in the find panel; these should all end up setting or clearing the status field ****/

- (void)findNextAndOrderFindPanelOut:(id)sender {
    [findNextButton performClick:nil];
    if (lastFindWasSuccessful) {
        [[self findPanel] orderOut:sender];
    } else {
	[findTextField selectText:nil];
    }
}

- (void)findNext:(id)sender {
    if (findTextField) [self setFindString:[findTextField stringValue]];	/* findTextField should be set */
    (void)[self find:Forward];
}

- (void)findPrevious:(id)sender {
    if (findTextField) [self setFindString:[findTextField stringValue]];	/* findTextField should be set */
    (void)[self find:Backward];
}

- (void)replace:(id)sender {
    NSTextView *text = [self textObjectToSearchIn];
    if (text && [text shouldChangeTextInRange:[text selectedRange] replacementString:[replaceTextField stringValue]]) {
        [[text textStorage] replaceCharactersInRange:[text selectedRange] withString:[replaceTextField stringValue]];
        [text didChangeText];
    } else {
        NSBeep();
    }
    [statusField setStringValue:@""];
}

- (void)replaceAndFind:(id)sender {
    [self replace:sender];
    [self findNext:sender];
}

#define ReplaceAllScopeEntireFile 42
#define ReplaceAllScopeSelection 43

/* The replaceAll: code is somewhat complex, and more complex than it used to be in DR1.  The main reason for this is to support undo. To play along with the undo mechanism in the text object, this method goes through the shouldChangeTextInRange:replacementString: mechanism. In order to do that, it precomputes the section of the string that is being updated. An alternative would be for this guy to handle the undo for the replaceAll: operation itself, and register the appropriate changes. However, this is simpler...

Turns out this approach of building the new string and inserting it at the appropriate place in the actual text storage also has an added benefit performance; it avoids copying the contents of the string around on every replace, which is significant in large files with many replacements. Of course there is the added cost of the temporary replacement string, but we try to compute that as tightly as possible beforehand to reduce the memory requirements.
*/
- (void)replaceAll:(id)sender {
    NSTextView *text = [self textObjectToSearchIn];
    if (!text) {
        NSBeep();
    } else {
        NSTextStorage *textStorage = [text textStorage];
        NSString *textContents = [text string];
        BOOL entireFile = replaceAllScopeMatrix ? ([replaceAllScopeMatrix selectedTag] == ReplaceAllScopeEntireFile) : YES;
        NSRange replaceRange = entireFile ? NSMakeRange(0, [textStorage length]) : [text selectedRange];
        unsigned searchOption = ([ignoreCaseButton state] ? NSCaseInsensitiveSearch : 0);
        unsigned replaced = 0;
        NSRange firstOccurence;
        
        if (findTextField) [self setFindString:[findTextField stringValue]];

        // Find the first occurence of the string being replaced; if not found, we're done!
        firstOccurence = [textContents rangeOfString:[self findString] options:searchOption range:replaceRange];
        if (firstOccurence.length > 0) {
            NSMutableAttributedString *temp;	/* This is the temporary work string in which we will do the replacements... */
            NSRange rangeInOriginalString;	/* Range in the original string where we do the searches */

	    // Find the last occurence of the string and union it with the first occurence to compute the tightest range...
            rangeInOriginalString = replaceRange = NSUnionRange(firstOccurence, [textContents rangeOfString:[self findString] options:NSBackwardsSearch|searchOption range:replaceRange]);

            temp = [[NSMutableAttributedString alloc] init];

            [temp beginEditing];

            while (rangeInOriginalString.length > 0) {
                NSRange foundRange = [textContents rangeOfString:[self findString] options:searchOption range:rangeInOriginalString];
		// Because we computed the tightest range above, foundRange should always be valid.
		NSRange rangeToCopy = NSMakeRange(rangeInOriginalString.location, foundRange.location - rangeInOriginalString.location + 1);	// Copy upto the start of the found range plus one char (to maintain attributes with the overlap)...
                [temp appendAttributedString:[textStorage attributedSubstringFromRange:rangeToCopy]];
                [temp replaceCharactersInRange:NSMakeRange([temp length] - 1, 1) withString:[replaceTextField stringValue]];
                rangeInOriginalString.length -= NSMaxRange(foundRange) - rangeInOriginalString.location;
                rangeInOriginalString.location = NSMaxRange(foundRange);
                replaced++;
            }

            [temp endEditing];

	    // Now modify the original string
            if ([text shouldChangeTextInRange:replaceRange replacementString:[temp string]]) {
                [textStorage replaceCharactersInRange:replaceRange withAttributedString:temp];
                [text didChangeText];
            } else {	// For some reason the string didn't want to be modified. Bizarre...
                replaced = 0;
            }

            [temp release];
        }
        if (replaced == 0) {
            NSBeep();
            [statusField setStringValue:NSLocalizedStringFromTable(@"Not found", @"FindPanel", @"Status displayed in find panel when the find string is not found.")];
        } else {
            [statusField setStringValue:[NSString localizedStringWithFormat:NSLocalizedStringFromTable(@"%d replaced", @"FindPanel", @"Status displayed in find panel when indicated number of matches are replaced."), replaced]];
        }
    }
}

- (void)takeFindStringFromSelection:(id)sender {
    NSTextView *textView = [self textObjectToSearchIn];
    if (textView) {
        NSString *selection = [[textView string] substringWithRange:[textView selectedRange]];
        [self setFindString:selection];
    }
}

- (void) jumpToSelection:sender {
    NSTextView *textView = [self textObjectToSearchIn];
    if (textView) {
        [textView scrollRangeToVisible:[textView selectedRange]];
    }
}

@end


@implementation NSString (NSStringTextFinding)

- (NSRange)findString:(NSString *)string selectedRange:(NSRange)selectedRange options:(unsigned)options wrap:(BOOL)wrap {
    BOOL forwards = (options & NSBackwardsSearch) == 0;
    unsigned length = [self length];
    NSRange searchRange, range;

    if (forwards) {
	searchRange.location = NSMaxRange(selectedRange);
	searchRange.length = length - searchRange.location;
	range = [self rangeOfString:string options:options range:searchRange];
        if ((range.length == 0) && wrap) {	/* If not found look at the first part of the string */
	    searchRange.location = 0;
            searchRange.length = selectedRange.location;
            range = [self rangeOfString:string options:options range:searchRange];
        }
    } else {
	searchRange.location = 0;
	searchRange.length = selectedRange.location;
        range = [self rangeOfString:string options:options range:searchRange];
        if ((range.length == 0) && wrap) {
            searchRange.location = NSMaxRange(selectedRange);
            searchRange.length = length - searchRange.location;
            range = [self rangeOfString:string options:options range:searchRange];
        }
    }
    return range;
}        

@end

/*

 2/21/95 aozer	Created for Edit II.
 2/24/95 aozer	Find pasteboard support
 8/16/95 aozer	Replace functionality
 10/4/95 aozer	Status field update
 3/26/98 aozer	Works with undo; faster Replace All

*/
