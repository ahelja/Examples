/* FunctionPane */

#import <Cocoa/Cocoa.h>

@interface FunctionPane : NSWindow
{
    IBOutlet NSStepper *kernelWidth;
    IBOutlet NSButton *OK;
    IBOutlet NSTextField *textBox;
}

- (IBAction)hitOK:(id)sender;

@end
