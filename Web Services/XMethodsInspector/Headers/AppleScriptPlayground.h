/* AppleScriptPlayground */

#import <Cocoa/Cocoa.h>

@interface AppleScriptPlayground : NSObject
{
    IBOutlet id fStubsPanel;
    IBOutlet id fEditPanel;
    IBOutlet id fResultPanel;
	IBOutlet id fProgressIndicator;
}

- (IBAction)runApplescript:(id) sender;

@end
