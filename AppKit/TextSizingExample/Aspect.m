/*
        Aspect.m
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

#import "Aspect.h"
#import "Controller.h"

const float LargeNumberForText = 1.0e7;

@implementation Aspect

- (id)initWithController:(Controller *)controller {
    self = [super init];
    if (self) {
        _controller = controller;
    }
    return self;
}

+ (id)aspectWithController:(Controller *)controller {
    Aspect *aspect = [[self alloc] initWithController:controller];
    return [aspect autorelease];
}

- (id)init {
    return [self initWithController:nil];
}

- (void)dealloc {
    if ([_controller currentAspect] == self) {
        [_controller swapInAspectAtIndex:-1];
    }
    [view release];
    [super dealloc];
}

- (Controller *)controller {
    return _controller;
}

- (NSString *)aspectName {
    return NSLocalizedString(@"Aspect", @"Default aspect name.");
}

- (NSString *)aspectNibName {
    // Aspects that only need the box and create the rest of their stuff programmatically, can just use the default nib.  Aspects with their own nibs should override.
    return @"Aspect";
}

- (void)loadNibIfNeeded {
    if (!_nibLoaded) {
        if ([NSBundle loadNibNamed:[self aspectNibName] owner:self]) {
            // We don't want the window, only the view.  So we retain the view and remove it from the window.  Then we can release the window.
            _nibLoaded = YES;
            [view retain];
            [view removeFromSuperview];
            [view setAutoresizesSubviews:YES];
            [view setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
            [view setFrameOrigin:NSMakePoint(0.0, 0.0)];
            [window release];
            window = nil;
            [self didLoadNib];
        } else {
            NSLog(@"Failed to load aspect nib %@.nib for class %@.", [self aspectName], [self class]);
        }
    }
}

- (NSView *)aspectView {
    [self loadNibIfNeeded];
    return view;
}

- (void)didLoadNib {
    // For subclassers
}

- (void)didSwapIn {
    // For subclassers
}

@end
