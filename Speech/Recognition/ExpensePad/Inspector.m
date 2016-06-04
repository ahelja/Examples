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

	
	Inspector.m
	ExpensePad
	
	Copyright (c) 2000-2005 Apple Computer. All rights reserved.

************************************************************/

#import "Inspector.h"
#import "MyDocument.h"
#import "AppController.h"

@implementation Inspector

static NSString *nibName = @"Inspector";
NSString *Category_Color_Changed_Notification = @"Category_Color_Changed_Notification";

- init {
    NSNotificationCenter *dnc;
    [super init];
    
    // register for expense selection changed notifications
    dnc = [NSNotificationCenter defaultCenter];
    [dnc addObserver:self
        selector:@selector(expenseSelectionDidChange:)
        name:ExpenseSelectionChangedNotification
        object:nil];
    [dnc addObserver:self
        selector:@selector(windowDidBecomeMain:)
        name:NSWindowDidBecomeMainNotification
        object:nil];
    [dnc addObserver:self
        selector:@selector(windowDidResignMain:)
        name:NSWindowDidResignMainNotification
        object:nil];
    return self;
}

- (void)awakeFromNib {
    // set the initial view
    [self inspectSelectedExpenseFromWindow:
            [[NSApplication sharedApplication] mainWindow]];
}


- (void)expenseSelectionDidChange:(NSNotification *)note {
    // if we get this notification the expense is passed as the
    // notification's object -- nil if there is none
    [self expenseSelectionDidChangeTo:[note object]];
}

- (void)expenseSelectionDidChangeTo:(Expense *)expense {
    if (expense == nil) {
        // if no expense, show "no selection" tab
        [tabView selectTabViewItemAtIndex:0];
        [categoryText setStringValue:@""];
        // [colorWell setEnabled:NO];
    }
    else {
        NSColor *color = [AppController colorForCategory:[expense category]];
        [categoryText setStringValue:[expense category]];
        [colorWell setColor:color];
        [tabView selectTabViewItemAtIndex:1];
    }
}

- (IBAction)colorDidChange:(id)sender {
    if (![[categoryText stringValue] isEqual:@""]) {
        NSNotificationCenter *dnc;
        [AppController setColor:[sender color] forCategory:[categoryText stringValue]];
        dnc = [NSNotificationCenter defaultCenter];
        [dnc postNotificationName:Category_Color_Changed_Notification
            object:self
            userInfo:nil];
    }
}


// watch out for key window changing
// -- selected expense will obviously change at the same time
- (void)windowDidResignMain:(NSNotification *)aNotification {
    [self expenseSelectionDidChangeTo:nil];
}
- (void)windowDidBecomeMain:(NSNotification *)aNotification {
    [self inspectSelectedExpenseFromWindow:[aNotification object]];
}

- (void)inspectSelectedExpenseFromWindow:(NSWindow *)window {
    Expense *expense = nil;
    id windowDelegate = [window delegate];
    
    if ([windowDelegate respondsToSelector:@selector(selectedExpense)]) {
        expense = [windowDelegate selectedExpense];
    }
    [self expenseSelectionDidChangeTo:expense];
}



- (IBAction)show:(id)sender
{
    // load panel lazily
    if (!panel) {
        if (![NSBundle loadNibNamed: nibName owner: self])
            NSLog(@"Unable to load nib \"%@\"",nibName);
    }
    [panel makeKeyAndOrderFront: self];
}

- (void)dealloc {
    // release instance variables and remove from notification center
    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
    [dnc removeObserver:self];
    [super dealloc];
}

@end
