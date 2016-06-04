/*
        Apsect.h
        TextSizingExample
   
        Author: Mike Ferris
*/

#import <AppKit/AppKit.h>

#define IBOutlet

extern const float LargeNumberForText;

@class Controller;

@interface Aspect : NSObject {
    IBOutlet NSWindow *window;
    IBOutlet NSView *view;

    Controller *_controller;
    BOOL _nibLoaded;
}

+ (id)aspectWithController:(Controller *)controller;

- (Controller *)controller;

- (NSString *)aspectName;
- (NSString *)aspectNibName;
- (NSView *)aspectView;

- (void)didLoadNib;

- (void)didSwapIn;

@end
