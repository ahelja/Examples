#import <Cocoa/Cocoa.h>

@interface Controller : NSObject {
    IBOutlet NSTextField *myTextField;
    IBOutlet NSWindow *myWindow;
}

- (IBAction)textFieldAction:(id)sender;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;

@end
