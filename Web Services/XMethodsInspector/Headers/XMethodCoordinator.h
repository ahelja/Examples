/* XMethodCoordinator */

#import <Cocoa/Cocoa.h>

@interface XMethodCoordinator : NSObject
{
    IBOutlet id fCPlusPlusStubView;
    IBOutlet id fCPlusPlusStubView_h;
    IBOutlet id fInspector;
    IBOutlet id fServiceList;
    IBOutlet id fServiceTable;
	IBOutlet id fProgressIndicator;
	IBOutlet id fAppleScriptPlayground;
	IBOutlet id fAppleScriptStubView;
	IBOutlet id fAppleScriptResults;
	IBOutlet id fObjcStubView;
	IBOutlet id fObjcStubView_h;
}
- (IBAction)fetch:(id) sender;

@end
