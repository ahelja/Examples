/*
    Controller.m
    Author: Ali Ozer
    Created: Feb 2002
 
    Controller object for the pasteboard viewer.
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

#import "Controller.h"
#import "LazyDataTextStorage.h"


@implementation Controller

/* Do some initialization after loading the nib file.
*/
- (void)awakeFromNib {
    // Replace the NSTextStorage for the contents view to be the lazy variety.
    if ([[dataView textStorage] class] != [LazyDataTextStorage class]) {
        LazyDataTextStorage *myTextStorage = [[LazyDataTextStorage alloc] init];
        // Use 10-pt version of the user's fixed pitch font
        [myTextStorage setAttributes:[NSDictionary dictionaryWithObjectsAndKeys:[NSFont userFixedPitchFontOfSize:10.0], NSFontAttributeName, nil]];
        [[dataView layoutManager] replaceTextStorage:myTextStorage];
    }

    // And display the info for the general pasteboard
    [self performSelector:@selector(updateTypesList:) withObject:nil afterDelay:0.0];
}

/* The pasteboard being examined (funnel point for returning the pasteboard, given stored name)
*/
- (NSPasteboard *)pasteboard {
    NSPasteboard *pb = nil;
    if (pasteboardName) pb = [NSPasteboard pasteboardWithName:pasteboardName];
    if (!pb) pb = [NSPasteboard generalPasteboard];
    return pb;
}

/* Return a human-visible name for the specified pasteboard type. Basically looks for the special Carbon pasteboards and converts the four-digit name to the human readable form.
*/
- (NSString *)humanVisibleNameForType:(NSString *)type {
    NSString *result = type;
    NSScanner *scanner = [NSScanner scannerWithString:type];
    unsigned int typeCode;
    if ([scanner scanString:@"CorePasteboardFlavorType" intoString:NULL] && [scanner scanHexInt:&typeCode]) {
        int i;
        unsigned char typeName[4];
        for (i = 0; i < 4; i++) {   // Extract the bytes in an endian-independent manner
            typeName[i] = (typeCode & (0xFF000000U >> (i * 8))) >> ((3 - i) * 8); // Take bytes one-by-one, starting from most significant byte
            if (typeName[i] < 0x20 || typeName[i] > 0x7F) break;
        }
        if (i == 4) result = [NSString stringWithFormat:@"'%.4s' (%@)", typeName, type];
    }
    return result;
}

/* To be called on pasteboard contents and pasteboard changes, this will clear out the type information.
*/
- (void)clearContents {
    [typeInfoField setStringValue:NSLocalizedString(@"No type selected", "String shown when the types list has been reloaded and nothing is selected")];
    if ([[dataView textStorage] class] == [LazyDataTextStorage class]) [(LazyDataTextStorage *)[dataView textStorage] setData:nil];
    [saveButton setEnabled:NO];
}

/* Put up a sheet for saving the currently selected type. On dismissal of the sheet, didEndSaveSheet:returnCode:contextInfo: is called.
*/
- (IBAction)save:(id)sender {
    [[NSSavePanel savePanel] beginSheetForDirectory:@"" file:@"" modalForWindow:[dataView window] modalDelegate:self didEndSelector:@selector(didEndSaveSheet:returnCode:contextInfo:) contextInfo:NULL];
}

/* Called when the save panel is dismissed. If OK, saves the data.
*/
- (void)didEndSaveSheet:(NSSavePanel *)savePanel returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    [savePanel orderOut:nil];
    if (returnCode == NSOKButton) {
        NSString *filename = [savePanel filename];
        NSData *data = [(LazyDataTextStorage *)[dataView textStorage] data];
        if (![data writeToFile:filename atomically:YES]) NSBeep();
    }
}

/* Call when a different pasteboard is selected or the pasteboard contents changes.  Reloads the type list for the selected pasteboard.
*/
- (IBAction)updateTypesList:(id)sender {
    NSPasteboard *pb = [self pasteboard];
    NSArray *types = [pb types];
    int numTypes = [types count];
    int i;

    lastChangeCount = [pb changeCount];

    [lastTypes release];
    lastTypes = [types copy];
        
    [typeMatrix renewRows:numTypes columns:1];
    [typeMatrix sizeToCells];

    for (i = 0; i < numTypes; i++) {
        [[typeMatrix cellAtRow:i column:0] setTitle:[self humanVisibleNameForType:[types objectAtIndex:i]]];
    }

    [self clearContents];
}

/* Called when a type is selected in the type list; shows its contents.
*/
- (IBAction)showContents:(id)sender {
    int row;
    NSPasteboard *pb = [self pasteboard];
    NSArray *types = [pb types];
    NSString *selectedType;
    NSData *data;
    BOOL changeCountBugFixed = (NSAppKitVersionNumber >= 637);
    
    // If pasteboard seems to have changed since the last check, return after reloading
    // (In 10.1 and earlier, changeCount can't be counted on as it increments more frequently than needed)
    // Also do a few other checks to make sure the displayed type list is still valid...
    if ((changeCountBugFixed && ([pb changeCount] != lastChangeCount)) ||	// Post-10.1 check
        (!changeCountBugFixed && ![lastTypes isEqual:types]) ||			// 10.1 and earlier check
        !((row = [sender selectedRow]) < [types count]) ||			// Somehow types still mismatch
        !(selectedType = [types objectAtIndex:row]) ||				// No type
        !(data = [pb dataForType:selectedType])) {				// No data for the type
            [self updateTypesList:nil];
            [typeInfoField setStringValue:NSLocalizedString(@"Clipboard contents changed", "String shown when the clipboard contents have changed since the types list was last loaded")];
            NSBeep();
            return;
    }

    // Show size of selected type
    [typeInfoField setStringValue:[NSString stringWithFormat:NSLocalizedString(@"%d bytes: %@", "String showing type name and total size in clipboard for the type"), [data length], [self humanVisibleNameForType:selectedType]]];
    
    // And now tell the contents view where to get the data 
    [(LazyDataTextStorage *)[dataView textStorage] setData:data];

    // And enable the save button
    [saveButton setEnabled:YES];
}

/* Called when the user selects a pasteboard. Because this is hooked up to a combobox, it allows either an existing selection or a brand new arbitrary name.
*/
- (IBAction)setPasteboard:(id)sender {
    int index = [sender indexOfSelectedItem];
    [pasteboardName autorelease];
    if ((index != -1) && ([[sender itemObjectValueAtIndex:index] isEqual:[sender stringValue]])) {
        switch (index) {
            case 1: pasteboardName = NSFontPboard; break;
            case 2: pasteboardName = NSRulerPboard; break;
            case 3: pasteboardName = NSFindPboard; break;
            case 4: pasteboardName = NSDragPboard; break;
            default: pasteboardName = NSGeneralPboard; break;
        }
    } else {
        pasteboardName = [sender stringValue];
    }
    [pasteboardName retain];
    [self updateTypesList:nil];
}

/* NSApplication delegate method; quit the app if the window is closed.
*/
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

/* On deallocation, release the window (which should release everything else in it) and ivars
*/
- (void)dealloc {
    [[dataView window] release];
    [lastTypes release];
    [pasteboardName release];
    [super dealloc];
}

@end
