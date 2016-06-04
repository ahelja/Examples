//
//  Controller.h
//

#import <Cocoa/Cocoa.h>


@interface Controller : NSObject {
    IBOutlet NSMatrix *typeMatrix;
    IBOutlet NSTextView *dataView;
    IBOutlet NSTextField *typeInfoField;
    IBOutlet NSButton *saveButton;
    
    NSString *pasteboardName;
    int lastChangeCount;	// Change count last time updateTypesList: was called
    NSArray *lastTypes;		// Types list last time updateTypesList: was called
}

// Actions for Controller

- (IBAction)save:(id)sender;
- (IBAction)updateTypesList:(id)sender;
- (IBAction)showContents:(id)sender;
- (IBAction)setPasteboard:(id)sender;

// Other methods

- (void)clearContents;
- (NSString *)humanVisibleNameForType:(NSString *)type;
- (NSPasteboard *)pasteboard;

@end
