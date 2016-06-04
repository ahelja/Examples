/*
 HexInputModePalette.m
 Copyright (c) 1998-2002, Apple Computer, Inc.
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

#import "HexInputModePalette.h"
#import "HexInputServer.h"

#define OFFSET_FROM_SCREEN_EDGE (20.0)

@implementation HexInputModePalette
- (void)awakeFromNib {
    [self setFloatingPanel:YES];
    [self setBecomesKeyOnlyIfNeeded:YES];

    if (![self setFrameUsingName:@"HexInputModePaletteFrame"]) {
        NSRect screenFrame = [[NSScreen mainScreen] frame];
	[self setFrameOrigin:NSMakePoint(screenFrame.size.width - _frame.size.width - OFFSET_FROM_SCREEN_EDGE, OFFSET_FROM_SCREEN_EDGE)];
    }
    [self setFrameAutosaveName:@"HexInputModePaletteFrame"];
}

- (void)buttonSelected:(id)sender {
    BOOL isHexInput = ([sender selectedTag] ? YES : NO);

    [[_buttonMatrix cellWithTag:(isHexInput ? 0 : 1)] setState:NO];
    [[HexInputServer sharedInstance] setCurrentInputContextModeToHex:isHexInput];
}

// Updates mode palette states according to isHexInput
- (void)updateModePaletteWithState:(BOOL)isHexInput {
    [[_buttonMatrix cellWithTag:0] setState:!isHexInput]; // Alpha input button cell
    [[_buttonMatrix cellWithTag:1] setState:isHexInput]; // Hex input button cell
}

- (void)setEnabled:(BOOL)flag {
    [_buttonMatrix setEnabled:flag];
}
@end

@implementation HexInputModeMatrix
- (BOOL)acceptsFirstResponder { return NO; }
- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent { return YES; }
@end
