//
//  DrawerController.h
//  DrawerMadness

// You may freely copy, distribute, and reuse the code in this example.
// Apple disclaims any warranty of any kind, expressed or implied, as to its
// fitness for any particular use.

#import <AppKit/AppKit.h>

@interface DrawerController : NSObject {
    IBOutlet NSWindow *myParentWindow;
    IBOutlet NSDrawer *leftDrawer;
    NSDrawer *bottomDrawer;
    NSDrawer *upperRightDrawer;
    NSDrawer *lowerRightDrawer;
}

- (IBAction)openLeftDrawer:(id)sender;
- (IBAction)closeLeftDrawer:(id)sender;
- (IBAction)toggleLeftDrawer:(id)sender;

- (IBAction)openBottomDrawer:(id)sender;
- (IBAction)closeBottomDrawer:(id)sender;
- (IBAction)toggleBottomDrawer:(id)sender;

- (IBAction)openUpperRightDrawer:(id)sender;
- (IBAction)closeUpperRightDrawer:(id)sender;
- (IBAction)toggleUpperRightDrawer:(id)sender;

- (IBAction)openLowerRightDrawer:(id)sender;
- (IBAction)closeLowerRightDrawer:(id)sender;
- (IBAction)toggleLowerRightDrawer:(id)sender;

@end
