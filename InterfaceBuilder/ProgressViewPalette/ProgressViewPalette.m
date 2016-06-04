/*
        ProgressViewPalette.m
 	Copyright (c) 1997-2001 Apple Computer, Inc.
 	All rights reserved.

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
*/

#import "ProgressViewPalette.h"

@implementation ProgressViewPalette

/* `finishInstantiate' can be used to associate non-view objects with
 * a view in the palette's nib.  For example:
 *   [self associateObject:aNonUIObject ofType:IBObjectPboardType withView:aView];
 *
 */
- (void)finishInstantiate
{
}

- init
{
    [super init];

        /*
         * Add ourselves to the list of registered drag delegates. When a drag occurs,
         * our code will be consulted.
         */
    [NSView registerViewResourceDraggingDelegate:self];
    return self;
}

/*
 * Implementation functions for IBViewResourceDraggingDelegates protocol
 */

    /*
     * This method will be called by InterfaceBuilder to determine which drag
     * types our drag delegate is responsible for.
     */
- (NSArray *)viewResourcePasteboardTypes
{
    return [NSArray arrayWithObject:NSColorPboardType];
}

    /*
     * InterfaceBuilder will call this method to determine if object
     * should accept a drag at the current point. In this example,
     * our drag delegate knows how to deposit cells on NSTableColumns.
     */
- (BOOL)acceptsViewResourceFromPasteboard:(NSPasteboard *)pasteboard
                                forObject:(id)object atPoint:(NSPoint)point
{
    int           row;
    int           column;
    ProgressCell *cell;

    if ([[pasteboard types] containsObject:NSColorPboardType]){
        if ([object isKindOfClass:[NSMatrix class]]){
                /* Here check if object is a matrix */
                if ([(NSMatrix *)object getRow:&row column:&column forPoint:point]) {
                    cell = [(NSMatrix *)object cellAtRow:row column:column];
                    if ([cell respondsToSelector: @selector(setColor:)]) {
                        return YES;
                    }
                }
        }else{
            if ([object respondsToSelector: @selector(setColor:)]) {
                return YES;
            }
        }
    }
    return NO;
}

    /*
     * We have accepted the drag, so now our delegate must do the work of
     * depositing the cell on the object.
     */
- (void)depositViewResourceFromPasteboard:(NSPasteboard *)pasteboard
                                 onObject:(id)object atPoint:(NSPoint)point
{
    NSColor       *color;
    int           row;
    int           column;
    ProgressCell *cell;

    color = [NSColor colorFromPasteboard:pasteboard];

    if ([object isKindOfClass:[NSMatrix class]]){
            /* Here check if object is a matrix */
        if ([(NSMatrix *)object getRow:&row column:&column forPoint:point]) {
            cell = [(NSMatrix *)object cellAtRow:row column:column];
            if ([cell respondsToSelector: @selector(setColor:)]) {
                [cell setColor:color];
            }
        }
    }else{
        if ([object respondsToSelector: @selector(setColor:)]) {
            [object setColor:color];
        }
    }
}

    /*
     * If we can accept the drag, should a drop target ring be
     * drawn around the view? In our case, yes.
     */

- (BOOL)shouldDrawConnectionFrame
{
    return YES;
}

@end

@implementation ProgressView (ProgressViewPaletteInspector)

/*
 * IB Will call this method when ProgressView is selected
 * This method returns the name of the class to put into the Attribute
 * Inspector (Info Panel)
 */
- (NSString *)inspectorClassName
{
    return @"ProgressViewInspector";
}

/*
 * IB calls this method to determine if pressing alt will allow a control
 * to be converted into a matrix of cells.
 */
- (BOOL)allowsAltDragging
{
    return YES;
}

@end

@implementation ProgressCell (ProgressViewPaletteInspector)
/*
 * IB Will call this method when ProgressView is selected
 * This method returns the name of the class to put into the Attribute
 * Inspector (Info Panel)
 *
 * Here we just used the same inspector as the View, since there
 * is nothing really special about the cell.
 */
- (NSString *)inspectorClassName
{
    return @"ProgressViewInspector";
}

/*
 * IB will call this method when the user presses match prototype in the
 * matrix inspector. its used to make sure all cells in the matrix
 * match the prototype cell.
 */
- (void)ibMatchPrototype:(NSCell *)prototype
{
    [super ibMatchPrototype:prototype];

    [self setPercentageIncrement:[(ProgressCell *)prototype percentageIncrement]];
}

@end

