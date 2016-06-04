/*
        Controller.m
        TextSizingExample

        Author: Mike Ferris

        Copyright (c) 1995-2004, Apple Computer, Inc., all rights reserved.
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

#import "Controller.h"

#import "Aspect.h"
#import "VertScrollAspect.h"
#import "BiScrollAspect.h"
#import "FixedSizeAspect.h"
#import "TwoColumnsAspect.h"
#import "FieldAspect.h"


@implementation Controller

- (id)init {
    self = [super init];
    if (self) {
        _currentAspectIndex = -1;
    }
    return self;
}

- (void)dealloc {
    [self swapInAspectAtIndex:-1];
    [_aspects release];
    [_openPanelPath release];
    [_textStorage release];
    [[swapView window] autorelease];
    [super dealloc];
}

- (NSArray *)aspects {
    if (!_aspects) {
        _aspects = [[NSMutableArray allocWithZone:[self zone]] init];

        [_aspects addObject:[VertScrollAspect aspectWithController:self]];
        [_aspects addObject:[BiScrollAspect aspectWithController:self]];
        [_aspects addObject:[FixedSizeAspect aspectWithController:self]];
        [_aspects addObject:[TwoColumnsAspect aspectWithController:self]];
        [_aspects addObject:[FieldAspect aspectWithController:self]];
    }
    return _aspects;
}

- (Aspect *)currentAspect {
    if (_currentAspectIndex >= 0) {
        return [[self aspects] objectAtIndex:_currentAspectIndex];
    } else {
        return nil;
    }
}

- (void)swapInAspectAtIndex:(int)newIndex {
    if (newIndex != _currentAspectIndex) {
        NSArray *aspects = [self aspects];
        if (_currentAspectIndex >= 0) {
            // Swap out current aspect.
            [[[aspects objectAtIndex:_currentAspectIndex] aspectView] removeFromSuperview];
        }
        _currentAspectIndex = newIndex;
        if (_currentAspectIndex >= 0) {
            // Size and swap in new aspect.
            NSSize frameSize = [swapView frame].size;
            NSView *aspectView = [[aspects objectAtIndex:_currentAspectIndex] aspectView];
            [aspectView setFrame:NSMakeRect(0.0, 0.0, frameSize.width, frameSize.height)];
            [swapView addSubview:aspectView];
            // Notify the aspect it was just installed.
            [[aspects objectAtIndex:_currentAspectIndex] didSwapIn];
        }
    }
}

- (void)awakeFromNib {
    NSArray *aspects = [self aspects];
    unsigned i, c = [aspects count];
    
    NSString *path;
    NSData *rtfData = nil;

    // Create the NSTextStorage and load the default text file
    path = [[NSBundle bundleForClass:[self class]] pathForResource:@"README" ofType:@"rtf"];
    rtfData = [NSData dataWithContentsOfFile:path];
    _textStorage = [[NSTextStorage allocWithZone:[self zone]] initWithRTF:rtfData documentAttributes:NULL];
    
    // Load the popup.
    [aspectPopUpButton removeAllItems];
    for (i=0; i<c; i++) {
        [aspectPopUpButton addItemWithTitle:[[aspects objectAtIndex:i] aspectName]];
    }

    // Set up an initial aspect to view
    if (c > 0) {
        [self swapInAspectAtIndex:0];
    } else {
        // No aspects!
        [aspectPopUpButton addItemWithTitle:NSLocalizedString(@"No Aspect", @"Item title for aspect popup if no aspects exist.")];
    }
}

- (NSTextStorage *)textStorage {
    return _textStorage;
}

- (void)aspectPopUpAction:(id)sender {
    int selectedIndex = [sender indexOfSelectedItem];
    NSArray *aspects = [self aspects];
    if ((selectedIndex >= 0) && (selectedIndex < (int)[aspects count])) {
        [self swapInAspectAtIndex:selectedIndex];
    }
}

- (void)loadDocumentAction:(id)sender {
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    int runModalResult;

    // Configure the OpenPanel
    [openPanel setTitle:NSLocalizedString(@"Load", @"Title for open panel for load document action.")];
    [openPanel setTreatsFilePackagesAsDirectories:NO];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setCanChooseDirectories:NO];
    [openPanel setCanChooseFiles:YES];
    [openPanel setDirectory:(_openPanelPath ? _openPanelPath : NSHomeDirectory())];

    // RUn the panel
    runModalResult = [openPanel runModal];
    if (runModalResult == NSOKButton) {
        NSString *path = [openPanel filename];
        NSString *extension = [path pathExtension];
        NSAttributedString *attrString;

        if ([extension isEqualToString:@"rtfd"] || [extension isEqualToString:@"rtf"]) {
            // Rich text.
            attrString = [[NSAttributedString allocWithZone:[self zone]] initWithPath:path documentAttributes:NULL];
        } else {
            // Plain text.
            attrString = [[NSAttributedString allocWithZone:[self zone]] initWithString:[NSString stringWithContentsOfFile:path]];
        }

        [_textStorage replaceCharactersInRange:NSMakeRange(0, [_textStorage length]) withAttributedString:attrString];
        [attrString release];
    }
    [_openPanelPath release];
    _openPanelPath = [[openPanel directory] copyWithZone:[self zone]];
}

@end
