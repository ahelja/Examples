/*
        Controller.h
        TextSizingExample

        Author: Mike Ferris
*/

#import <AppKit/AppKit.h>

#define IBOutlet

@class Aspect;

@interface Controller : NSObject {
    IBOutlet NSPopUpButton *aspectPopUpButton;
    IBOutlet NSView *swapView;
    IBOutlet NSPanel *infoPanel;

    NSTextStorage *_textStorage;
    NSMutableArray *_aspects;
    int _currentAspectIndex;

    NSString *_openPanelPath;
}

- (NSArray *)aspects;
- (Aspect *)currentAspect;

- (void)swapInAspectAtIndex:(int)newIndex;

- (NSTextStorage *)textStorage;

- (void)aspectPopUpAction:(id)sender;
- (void)loadDocumentAction:(id)sender;

@end
