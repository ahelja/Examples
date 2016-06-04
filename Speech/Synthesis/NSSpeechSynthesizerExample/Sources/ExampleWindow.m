/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.


	ExampleWindow.m
	NSSpeechSynthesizerExample
	
	Copyright (c) 2003-2005 Apple Computer. All rights reserved.
*/

#import "ExampleWindow.h"

const UInt32 kNumOfFixedMenuItemsInVoicePopup = 2;

@implementation NSSpeechExampleWindow
- (void)awakeFromNib
{
	_speechSynthesizer 	= [NSSpeechSynthesizer new];
	[_speechSynthesizer setDelegate:self];
	[_characterView setExpression:kCharacterExpressionIdentifierIdle];

	[self getSpeechVoices];
}

- (IBAction) speakTextButtonSelected:(id)sender
{
    [self startSpeakingTextViewToURL:NULL];
}

- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender willSpeakPhoneme:(short)phonemeOpcode
{
    [_characterView setExpressionForPhoneme:[NSNumber numberWithShort:phonemeOpcode]];
}

- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didFinishSpeaking:(BOOL)finishedSpeaking
{
    [_textToSpeechExampleTextView setSelectedRange:_orgSelectionRange];	// Set selection length to zero.
    [_textToSpeechExampleSpeakButton setTitle:NSLocalizedString(@"Start Speaking", @"Speaking button name (start)")];
    [_saveButton setTitle:NSLocalizedString(@"Save As File...", @"Save button title")];
    [_textToSpeechExampleSpeakButton setEnabled:YES];
    [_saveButton setEnabled:YES];
    [_voicePop setEnabled:YES];
}

- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender willSpeakWord:(NSRange)characterRange ofString:(NSString *)string
{
    UInt32	selectionPosition = characterRange.location + _offsetToSpokenText;
    UInt32	wordLength = characterRange.length;
	
    [_textToSpeechExampleTextView scrollRangeToVisible:NSMakeRange(selectionPosition, wordLength)];
    [_textToSpeechExampleTextView setSelectedRange:NSMakeRange(selectionPosition, wordLength)];
    [_textToSpeechExampleTextView display];
}

- (IBAction) savetButtonSelected:(id)sender;
{
    if([_speechSynthesizer isSpeaking]) {
        [_speechSynthesizer stopSpeaking];
	}
    else {    
		NSURL * selectedFileURL = NULL;

        NSSavePanel *	theSavePanel = [NSSavePanel new];
        [theSavePanel setPrompt:NSLocalizedString(@"Save", @"Save button name")];
        if (NSFileHandlingPanelOKButton == [theSavePanel runModalForDirectory:NULL file:NSLocalizedString(@"Synthesized Speech.aiff", @"Default save filename")]) {
            selectedFileURL = [theSavePanel URL];
            [self startSpeakingTextViewToURL:selectedFileURL];
        }
    }
}

- (void)startSpeakingTextViewToURL:(NSURL *)url
{
    if([_speechSynthesizer isSpeaking]) {
        [_speechSynthesizer stopSpeaking];
	}
    else {

        // Grab the selection substring, or if no selection then grab entire text.
        _orgSelectionRange = [_textToSpeechExampleTextView selectedRange];
        
        NSString *	theViewText;
        if (_orgSelectionRange.length == 0) {
            theViewText = [_textToSpeechExampleTextView string];
            _offsetToSpokenText = 0;
        }
        else {
            theViewText = [[_textToSpeechExampleTextView string] substringWithRange:_orgSelectionRange];
            _offsetToSpokenText = _orgSelectionRange.location;
        }
        
        if ([_voicePop indexOfSelectedItem] == 0) {
			// Pass NULL as the voice to use the system voice.
            [_speechSynthesizer setVoice:NULL];	
        }
        else {
            [_speechSynthesizer setVoice:[[NSSpeechSynthesizer availableVoices] objectAtIndex:[_voicePop indexOfSelectedItem] - kNumOfFixedMenuItemsInVoicePopup]];
        }
        
        if (url) {
            [_speechSynthesizer startSpeakingString:theViewText toURL:url];
            [_textToSpeechExampleSpeakButton setEnabled:NO];
            [_saveButton setTitle:NSLocalizedString(@"Stop Saving", @"Save file button name (stop)")];
        }
        else {
            [_speechSynthesizer startSpeakingString:theViewText];
            [_textToSpeechExampleSpeakButton setTitle:NSLocalizedString(@"Stop Speaking", @"Speaking button name (stop)")];
            [_saveButton setEnabled:NO];
        }
        [_voicePop setEnabled:NO];
        
    }
}

- (void)getSpeechVoices 
{
    // Delete any items in the voice menu
    while([_voicePop numberOfItems] > kNumOfFixedMenuItemsInVoicePopup) {
        [_voicePop removeItemAtIndex:[_voicePop numberOfItems] - 1];
	}
    
	NSString * aVoice = NULL;
	NSEnumerator * voiceEnumerator = [[NSSpeechSynthesizer availableVoices] objectEnumerator];
	while(aVoice = [voiceEnumerator nextObject]) {
		NSDictionary * dictionaryOfVoiceAttributes = [NSSpeechSynthesizer attributesForVoice:aVoice];
		NSString *	voiceDisplayName = [dictionaryOfVoiceAttributes objectForKey:NSVoiceName];

		[_voicePop addItemWithTitle:voiceDisplayName];
	}
}

@end


