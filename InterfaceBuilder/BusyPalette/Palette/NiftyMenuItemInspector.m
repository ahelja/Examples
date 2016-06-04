/**
 *	filename: NiftyMenuItemInspector.m
 *	created : Mon Apr  2 13:32:05 2001
 *	LastEditDate Was "Tue Apr  3 08:51:36 2001"
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

#import "NiftyMenuItemInspector.h"

#import <BusyPaletteFramework/BusyPaletteFramework.h>

@implementation NiftyMenuItemInspector

- init
{
    [super init];
    if ([NSBundle loadNibNamed:@"NiftyMenuItemInspector" owner:self] == NO) {
        NSLog(@"Can't load NiftyMenuItemInspector nib file");
    }
    return self;
}

- (void)ok:(id)sender
{
    NiftyMenuItem *editedObject;

    editedObject = [self object];

        /* Begin an Undo Grouping, all snapshots will be placed into the group
         * created by calling beingUndoGrouping.
         *
         * You can have Interface Builder take the snapshot of any object that is stored within
         * the document in the noteAttributesWillChangeForObject: IE. if the inspector changes more
         * than 1 object, be sure to called noteAttributesWillChangeForObject: on each of those
         * objects.
         */
    [self beginUndoGrouping];
    [self noteAttributesWillChangeForObject:editedObject];
    [editedObject setRandomString:[randomStringTextField stringValue]];

        /* Don't forget to call super ok: calling super ok: makes sure that the
         * document is marked as being edited and lets the rest of Interface Builder
         * know that something has changed
         */
    [super ok:sender];
}

- (void)revert:(id)sender
{
    [randomStringTextField setStringValue:[(NiftyMenuItem *)[self object] randomString]];

        /* Don't forget to call super revert: calling super revert: lets the rest of
         * Interface Builder know that something on the document has changed
         */
    [super revert:sender];
}
@end
