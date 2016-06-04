/*
 
 File: GridFeederArgumentContainerController.m
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright 2002, 2005 Apple Computer, Inc., All Rights Reserved
 
*/

#import "GridFeederArgumentContainerController.h"
#import "GridFeederArgumentViewController.h"
#import "GridFeederColorBackgroundView.h"

@interface GridFeederArgumentContainerController (PrivateMethods)

- (void)_fixKeyViewChain;
- (void)_updateArguments;
- (void)_tileViewsWithChangeAtIndex:(int)changeIndex;

@end

@implementation GridFeederArgumentContainerController

- (void)awakeFromNib;
{
    _originalContainerDocumentViewNextKeyView = [[_container documentView] nextKeyView];

    _viewControllers = [[NSMutableArray alloc] init];

	NSRect frame = [[_container documentView] frame];
	
	frame.size.height = 0;
	
	[[_container documentView] setFrame:frame];

    _blueColor = [[NSColor colorWithCalibratedRed:(float)(239.0/255.0) green:(float)(247.0/255.0) blue:1.0 alpha:1.0] retain];
    _whiteColor = [[NSColor whiteColor] retain];

    [self performSelector:@selector(addArgument:) withObject:nil afterDelay:0.0];
}

- (void)dealloc;
{
    [_viewControllers release];
    [_blueColor release];
    [_whiteColor release];

    [super dealloc];
}

- (void)setEditable:(BOOL)editable;
{
    NSEnumerator *controllerEnumerator = [_viewControllers objectEnumerator];
    GridFeederArgumentViewController *controller;

    while (controller = [controllerEnumerator nextObject]) {
	
        [controller setEditable:editable];
    }

    if (editable == YES) {
	
        [self _updateArguments];
    }
}

- (IBAction)addArgument:(id)sender;
{
    int insertionIndex;
    
    if (sender != nil) {
	
        insertionIndex = [_viewControllers indexOfObject:sender] + 1;
    }
    else {
	
        insertionIndex = [_viewControllers count];
    }

    GridFeederArgumentViewController *argumentViewController = [[[GridFeederArgumentViewController alloc] initWithParent:self] autorelease];

    [_viewControllers insertObject:argumentViewController atIndex:insertionIndex];

    [self _tileViewsWithChangeAtIndex:insertionIndex];

    [self _updateArguments];
    [self _fixKeyViewChain];
}

- (IBAction)removeArgument:(id)sender;
{
    int removalIndex = [_viewControllers indexOfObject:sender];

    [self _tileViewsWithChangeAtIndex:removalIndex];

    [_viewControllers removeObjectAtIndex:removalIndex];

    [self _updateArguments];
    [self _fixKeyViewChain];
}

- (NSArray *)arrayOfArgumentStringArrays;
{
    NSMutableArray *array = [NSMutableArray arrayWithCapacity:[_viewControllers count]];

    NSEnumerator *enumerator = [_viewControllers objectEnumerator];
    GridFeederArgumentViewController *controller;

    while (controller = [enumerator nextObject]) {
	
        NSArray *innerArray = [controller argumentStringArray];

        if (innerArray != nil) {
		
            [array addObject:innerArray];
        }
    }

    return array;
}

- (void)_fixKeyViewChain;
{
    NSEnumerator *enumerator = [_viewControllers objectEnumerator];
    GridFeederArgumentViewController *previousController = [enumerator nextObject];

    if (previousController != nil) {
        
        [_container setNextKeyView:[previousController firstKeyView]];

        GridFeederArgumentViewController *nextController;

        while (nextController = [enumerator nextObject]) {

            [[previousController lastKeyView] setNextKeyView:[nextController firstKeyView]];

            previousController = nextController;
        }

        [[previousController lastKeyView] setNextKeyView:_originalContainerDocumentViewNextKeyView];
    }
    else {
	
        [_container setNextKeyView:_originalContainerDocumentViewNextKeyView];
    }
}
        
- (void)_updateArguments;
{
    int index;
    int count = [_viewControllers count];
    
    BOOL enabled = count > 1;

    for (index = 0; index < count; index++) {

        GridFeederArgumentViewController *controller = [_viewControllers objectAtIndex:index];

        [controller setRemoveEnabled:enabled];
    }
}

- (void)_tileViewsWithChangeAtIndex:(int)changeIndex;
{
    int newHeight = 0;
    int index;
    int argIndex = 1;
    int count = [_viewControllers count];
    
    NSView *parent = [_container documentView];    
    NSRect parentFrame = [parent frame];

    NSWindow *window = [_container window];
    NSRect windowFrame = [window frame];

    float deltaHeight;

    for (index = 0; index < count; index++) {
	
        int currentHeight = newHeight;
        
        GridFeederArgumentViewController *controller = [_viewControllers objectAtIndex:index];
        GridFeederColorBackgroundView *controllerView = (GridFeederColorBackgroundView *)[controller view];

        [controller setTitle:[NSString stringWithFormat:@"Argument %d:", argIndex]];
        [controllerView setBackgroundColor:(argIndex % 2) == 0 ? _blueColor : _whiteColor];        

        newHeight += [controllerView frame].size.height;
        argIndex += 1;

        if (index == changeIndex) {
		
            if ([controllerView superview] == nil) {
			
                [parent addSubview:controllerView];
    
                NSRect frame;
                frame.origin.x = 0;
                frame.origin.y = parentFrame.size.height - currentHeight;
                frame.size.height = 0;
                frame.size.width = parentFrame.size.width;
    
                [controllerView setFrame:frame];
            }
            else {
			
                newHeight -= [controllerView frame].size.height;
                argIndex -= 1;
            }

            [controllerView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        }
        else if (index < changeIndex) {
		
            [controllerView setAutoresizingMask:NSViewWidthSizable | NSViewMinYMargin];
        }
        else {
		
            [controllerView setAutoresizingMask:NSViewWidthSizable | NSViewMaxYMargin];
        }
    }

    [parent setAutoresizesSubviews:YES];

    deltaHeight = newHeight - parentFrame.size.height;

    windowFrame.size.height += deltaHeight;
    windowFrame.origin.y -= deltaHeight;

    [window setFrame:windowFrame display:YES animate:YES];
}

@end
