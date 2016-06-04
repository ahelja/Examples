// Controller.h
// MenuMadness example

// You may freely copy, distribute, and reuse the code in this example.
// Apple disclaims any warranty of any kind, expressed or  implied, as to its
// fitness for any particular use.

#import <AppKit/AppKit.h>

@interface Controller : NSObject {
    IBOutlet NSWindow *window;

    NSPopUpButton *normalPopUp;
    NSPopUpButton *normalPullDown;
    NSPopUpButton *smallNormalPopUp;
    NSPopUpButton *smallNormalPullDown;
    NSPopUpButton *imagePopUp;
    NSPopUpButton *imagePullDown;
    NSPopUpButton *noSelPopUp;
    NSPopUpButton *noSelPullDown;

    int currentRadioSetting;
    BOOL currentSwitch1Setting;
    BOOL currentSwitch2Setting;
}

@end
