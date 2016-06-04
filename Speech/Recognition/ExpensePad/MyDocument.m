/************************************************************

 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple’s copyrights in this original Apple software (the
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

	
	MyDocument.m
	ExpensePad
	
	Copyright (c) 2000-2005 Apple Computer. All rights reserved.

************************************************************/

#import "MyDocument.h"
#import "Expense.h"
#import "AppController.h"
#import "CocoaSpeechAppleEvents.h"
#import "Inspector.h"

#import <Carbon/Carbon.h>

NSString * ExpenseSelectionChangedNotification = @"ExpenseSelectionChanged";
NSString * ExpensePBoardType		       = @"ExpensePBoard";

@implementation MyDocument

- (id) init
{
    [super init];
    recSystem = [Utterance getRecognitionSystem];
    if (!recSystem || SRNewRecognizer(recSystem, &recognizer, kSRDefaultSpeechSource))
        recognizer = NULL;
        
    expenses 	= [[NSMutableArray alloc] init];
    utterances 	= [[NSMutableArray alloc] init];
    
    return self;
}

- (void) dealloc
{
    [expenses release];
    [utterances release];
    SRReleaseObject(recognizer);
    
    [super dealloc];
}

- (void) recalcTotal
{
    NSEnumerator 	* 	it 	= [expenses objectEnumerator];
    NSMutableDictionary	*	catSeen = [NSMutableDictionary dictionary];
    Expense	 	* 	e;
    double	   		sum = 0.0;
    SRLanguageModel		lang;
    
    [utterances removeAllObjects];
    
    if (!recognizer || SRNewLanguageModel(recSystem, &lang, "expense", 7))
        lang = NULL;
    
    while (e = [it nextObject]) {
        sum += [[e amount] doubleValue];
        if (lang && ![catSeen objectForKey:[e category]]) {
            NSString *	s = [NSString stringWithFormat:@"Add expense for %@", [e category]];
            Utterance * u = [Utterance utteranceWithTarget:self 
                                selector:@selector(addExpenseForCategory:)
                                withObject:[e category]];

            SRAddText(lang, [s cString], [s cStringLength],(long)u);
            [utterances addObject:u];
            [catSeen setObject:@"Seen" forKey:[e category]];            
        }
    }
    
    SRSetLanguageModel(recognizer, lang);
    SRReleaseObject(lang);
    
    [total setDoubleValue: sum];
}

- (int) numberOfRowsInTableView: (NSTableView *)tableView;
{
    return [expenses count];
}

- (id) tableView:(NSTableView *)tableView 
    objectValueForTableColumn:(NSTableColumn *)column 
    row:(int)row
{
    return [[expenses objectAtIndex:row] valueForKey:[column identifier]];
}

- (void) tableView:(NSTableView *)tableView 
    setObjectValue:(id)value 
    forTableColumn:(NSTableColumn *)column 
    row:(int)row
{
    [[expenses objectAtIndex:row] takeValue:value forKey:[column identifier]];
    [self recalcTotal];
}

- (void)tableView:(NSTableView *)aTableView
        willDisplayCell:(id)aCell
        forTableColumn:(NSTableColumn *)aTableColumn
        row:(int)rowIndex
{
    // may have to tweak if variables differently named
    NSString *category = [[expenses objectAtIndex:rowIndex] category];

    // guaranteed to return a color
    NSColor *color = [AppController colorForCategory:category];
    [aCell setTextColor:color];
}

- (void) addExpense:(id)sender
{
    [expenses addObject: [[[Expense alloc] init] autorelease]];
    [table noteNumberOfRowsChanged];
    [table selectRow:[expenses count]-1 byExtendingSelection:NO];
    [table editColumn:[table columnWithIdentifier:@"amount"] row:[expenses count]-1 
        withEvent:nil select:YES];
    [self recalcTotal];
    [self updateChangeCount: NSChangeDone];
}

- (void) addExpenseForCategory:(NSString *)category
{
    Expense * expense = [[[Expense alloc] init] autorelease];
    [expense setCategory:category];
    [expenses addObject: expense];
    [table noteNumberOfRowsChanged];
    [table selectRow:[expenses count]-1 byExtendingSelection:NO];
    [table editColumn:[table columnWithIdentifier:@"amount"] row:[expenses count]-1 
        withEvent:nil select:YES];
    [self recalcTotal];
    [self updateChangeCount: NSChangeDone];
}

- (void) deleteExpense:(id)sender
{
    int row = [table selectedRow];
    
    if (row >= 0) {
        [expenses removeObjectAtIndex:row];
        [table reloadData];
        [table selectRow: ((row>=[expenses count]) ? row-1 : row) byExtendingSelection:NO];
        [self recalcTotal];
        [self updateChangeCount: NSChangeDone];
    }
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    NSNotificationCenter * center = [NSNotificationCenter defaultCenter];
    id selection = [self selectedExpense];
    
    if (selection) {
        [notes setEditable:YES];
        [notes setString: [selection notes]];
        [deleteButton setEnabled:YES];
    } else {
        [notes setEditable:NO];
        [notes setString:@""];
        [deleteButton setEnabled:NO];
    }
        
    [center postNotificationName:ExpenseSelectionChangedNotification object:selection userInfo: nil];
}

- (void)textDidChange:(NSNotification *)notification
{
    [[self selectedExpense] setNotes: [notes string]];
}

- (void)categoryColorDidChange:(NSNotification *)notification
{
    [table reloadData];
}

- (id) selectedExpense
{
    int row = [table selectedRow];
    
    return row >= 0 ? [expenses objectAtIndex:row] : nil;
}

- (BOOL)keepBackupFile
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:CreatesBackupKey];
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    NSNotificationCenter * center = [NSNotificationCenter defaultCenter];
    
    [super windowControllerDidLoadNib:aController];
    [self recalcTotal];
    [center addObserver:self selector:@selector(categoryColorDidChange:)
        name: Category_Color_Changed_Notification object:nil];
 
    [table registerForDraggedTypes:[NSArray arrayWithObject:ExpensePBoardType]];
    [total registerForDraggedTypes:[NSArray arrayWithObject:NSFileContentsPboardType]];
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    return [NSArchiver archivedDataWithRootObject: expenses];
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
    [expenses release];
    expenses = [NSUnarchiver unarchiveObjectWithData: data];
    [expenses retain];
    [self recalcTotal];
    
    return expenses != nil;
}

- (BOOL) validateMenuItem: (NSMenuItem *) item
{
    BOOL isValid = NO;
    BOOL hasSelection = [table selectedRow] > -1;
    
    if ([item action] == @selector(addExpense:)) 
        isValid  = YES;
    else if ([item action] == @selector(deleteExpense:)) 
        isValid  = hasSelection;
    else if ([item action] == @selector(cut:)) 
        isValid  = hasSelection;
    else if ([item action] == @selector(copy:)) 
        isValid  = hasSelection;
    else if ([item action] == @selector(paste:)) 
        isValid  = [[NSPasteboard generalPasteboard] dataForType:ExpensePBoardType] != nil;
    else 
        isValid = [super validateMenuItem: item];
    
    return isValid;
}

- (void) cut:(id)sender
{
    [self copy:sender];
    [self deleteExpense:sender];
}

- (void) copy:(id)sender
{
    NSPasteboard * 	pasteBoard = [NSPasteboard generalPasteboard];
    id			expense	   = [self selectedExpense];
    
    [pasteBoard declareTypes:[NSArray arrayWithObjects:ExpensePBoardType, NSStringPboardType]
        owner:nil];
    [pasteBoard setData:[NSArchiver archivedDataWithRootObject:expense] forType:ExpensePBoardType];
    [pasteBoard setString:[expense description] forType:NSStringPboardType];
}

- (void) paste:(id)sender
{
    NSPasteboard * 	pasteBoard = [NSPasteboard generalPasteboard];
    NSData *		data	   = [pasteBoard dataForType:ExpensePBoardType];
    id			expense	   = [NSUnarchiver unarchiveObjectWithData:data];
    [expenses addObject:expense];
    [table noteNumberOfRowsChanged];
    [table selectRow:[expenses count]-1 byExtendingSelection:NO];    
    [self recalcTotal];
    [self updateChangeCount: NSChangeDone];
}

- (BOOL)tableView:(NSTableView *)tv writeRows:(NSArray*)rows toPasteboard:(NSPasteboard*)pboard
{
    NSPasteboard * 	pasteBoard = [NSPasteboard pasteboardWithName:NSDragPboard];
    id			expense	   = [expenses objectAtIndex:[[rows objectAtIndex:0] intValue]];
    
    [pasteBoard declareTypes:[NSArray arrayWithObjects:ExpensePBoardType, NSStringPboardType]
        owner:nil];
    [pasteBoard setData:[NSArchiver archivedDataWithRootObject:expense] forType:ExpensePBoardType];
    [pasteBoard setString:[expense description] forType:NSStringPboardType]; 
    
    return YES;
}

- (NSDragOperation)tableView:(NSTableView*)tv validateDrop:(id <NSDraggingInfo>)info proposedRow:(int)row proposedDropOperation:(NSTableViewDropOperation)op
{
    // don't drag and drop onto ourselves
    if ([info draggingSource] == table) {
	return NSDragOperationNone;
    }
    // tell the tableview we want to copy the expense at, not over,
    // the current insertion point (contrast NSTableViewDropOn) 
    [tv setDropRow:row dropOperation:NSTableViewDropAbove];
    return NSDragOperationCopy;
}

- (BOOL)tableView:(NSTableView*)tv acceptDrop:(id <NSDraggingInfo>)info row:(int)row dropOperation:(NSTableViewDropOperation)op
{
    BOOL b = NO;
    NSData *data = [[info draggingPasteboard] dataForType:ExpensePBoardType];
    Expense *expense = [NSUnarchiver unarchiveObjectWithData:data];
    if (expense) {
        if (row > -1)
            [expenses insertObject:expense atIndex:row];
        else 
            [expenses addObject:expense];
        [table noteNumberOfRowsChanged];
        [table selectRow:row byExtendingSelection:NO];    
        [self recalcTotal];
        [self updateChangeCount: NSChangeDone];
       b = YES;
    }
    
    return b;
}

- (void)windowDidBecomeMain:(NSNotification *)notification
{
    if (recognizer)
        SRStartListening(recognizer);
}

- (void)windowDidResignMain:(NSNotification *)notification
{
    if (recognizer)
        SRStopListening(recognizer);
}

@end
