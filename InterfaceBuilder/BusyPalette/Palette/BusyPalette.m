/**
 *      filename: BusyPalette.m
 *      created : Fri Mar 30 09:58:22 2001
 *      LastEditDate Was "Thu May 31 23:11:47 2001"
 *
 */

/*
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

#import "BusyPalette.h"

/*
 * Define our cell dragging type
 */

NSString *const BPCellPboardType = @"BPCellPboardType";

@implementation BusyPalette

- init
{
    [super init];

    /*
     * Add ourselves to the list of registered drag delegates. When a drag occurs,
     * our code will be consulted.
     */
    [NSView registerViewResourceDraggingDelegate:self];

    /*
     * Register the palette to listen to notifications sent by
     * IB when IB will inspect an object.
     */
    [[NSNotificationCenter defaultCenter] addObserver:self
                                          selector:@selector(willInspectObject:)
                                          name:IBWillInspectObjectNotification
                                          object:nil];
    return self;
}

    /*
     * This is called just after the palette is loaded into Interface Builder. A
     * palette is only loaded after the user first clicks on its icon. If you need
     * to do any work before this time (such as adding drag delegates), do it in
     * -(id)init
     *
     * Here we provide some a visual stand-in to allow the user to manipulate
     * the formatter and menuItem on the screen. Since formatters, menuItems and cells
     * are not NSViews and don't really draw themselves on the screen, we assign proxy
     * views which the user can drag off the palette.
     */
- (void)finishInstantiate
{
    _expressionFormatter = [[ExpressionFormatter alloc] init];
    _menuItem = [[NiftyMenuItem alloc] init];
    _checkBoxCell = [[checkBoxView cell] retain];

    [_menuItem setTitle:@"Item"];
    [self associateObject:_expressionFormatter
                   ofType:IBFormatterPboardType
                 withView:formatterImageView];

    [self associateObject:_menuItem
                   ofType:IBMenuItemPboardType
                 withView:menuImageView];

    [self associateObject:_checkBoxCell
                   ofType:BPCellPboardType
                 withView:checkBoxView];
}

- (void)dealloc
{
        /* Be sure to clean up before this object is released
         * remove all references to the expressionFormatter and
         * the menuItem.
         *
         * Also tell the NotificationCenter that we are no longer
         * interested in getting notifications
         */
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [_expressionFormatter release];
    [_menuItem release];
    [_checkBoxCell release];
    [super dealloc];
}

    /*
     * This is the method that will get called when the
     * IBWillInspectObjectNotification gets sent.
     *
     * If the object that is being inspected is a kind of NiftyMenuItem class then
     * the code will tell Interface Builder to add another inspector mode called
     * NiftyMenuItemAlternateAttributes.
     *
     */
- (void)willInspectObject:(NSNotification *)notification
{
    id object;

    object = [notification object];

    if ([object isKindOfClass:[NiftyMenuItem class]]) {
        [[IBInspectorManager sharedInspectorManager]
            addInspectorModeWithIdentifier:@"NiftyMenuItemAlternateAttributes"
                                 forObject:object                   	/* object being inspected */
                        localizedLabel:@"Alternate Attributes"  	/* Label to show in the popup */
                    inspectorClassName:@"NiftyMenuItemInspector" 	/* Inspector class name      */
                                  ordering:-1.0];			/* Order of mode in inspector popup. -1
                                                                         * implies the end
                                                                         */
    }
    if ([object isKindOfClass:[NSTableColumn class]]){
        NSTableColumn *tableColumn;

        tableColumn = object;

        if ([[tableColumn dataCell] isKindOfClass:[NSTextFieldCell class]] == NO){
            [[IBInspectorManager sharedInspectorManager]
                addInspectorModeWithIdentifier:@"BPCellAttributes"
                                     forObject:object                	/* object being inspected */
                                localizedLabel:@"DataCell Attributes"  	/* Label to show in the popup */
                            inspectorClassName:@"DataCellInspector" 	/* Inspector class name      */
                                      ordering:-1.0];			/* Order of mode in inspector popup. -1
                                                                         * implies the end
                                                                         */
        }
    }
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
    return [NSArray arrayWithObject:BPCellPboardType];
}

    /*
     * InterfaceBuilder will call this method to determine if object
     * should accept a drag at the current point. In this example,
     * our drag delegate knows how to deposit cells on NSTableColumns.
     */
- (BOOL)acceptsViewResourceFromPasteboard:(NSPasteboard *)pasteboard
				forObject:(id)object atPoint:(NSPoint)point
{
    return [object isKindOfClass: [NSTableHeaderView class]];
}

    /*
     * We have accepted the drag, so now our delegate must do the work of
     * depositing the cell on the object.
     */
- (void)depositViewResourceFromPasteboard:(NSPasteboard *)pasteboard
				 onObject:(id)object atPoint:(NSPoint)point
{
    id <IBDocuments> document;
    NSArray* pasteboardObjects;

    document = [NSApp documentForObject:object];
    pasteboardObjects = [document pasteType:BPCellPboardType fromPasteboard:pasteboard parent:object];

    if ([pasteboardObjects count] > 0) {
        NSCell* cell;

        cell = [pasteboardObjects objectAtIndex:0];
        if ([object respondsToSelector: @selector(setDataCell:)]) {
            [object setDataCell: cell];
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


@interface ExpressionFormatter (ExpressionFormatterPaletteInspector)
- (NSString *)inspectorClassName;
@end

@implementation ExpressionFormatter (ExpressionFormatterPaletteInspector)
/*
 * For the ExpressionFormatter we really don't want to make our
 * own attribute inspector, so we will end up using the super class
 * NSNumberFormatters attribute inspector already present in Interface Buidler
 *
 * This doesn't really need to be done, but is here as an example. If you want
 * another attributes inspector return the name of the class here.
 */
- (NSString *)inspectorClassName
{
    return [super inspectorClassName];
}

@end

