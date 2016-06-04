#import <Cocoa/Cocoa.h>

@class WormView;

@interface WormController : NSObject {
    NSTextField *actualFrameRateTextField;
    NSTextField *scoreTextField;
    id wormView;
    NSButton *startStopButton;
    NSTimer *updateTimer;
}

- (void)toggleGame:(id)sender;
- (void)resetGame:(id)sender;

- (void)changeWormString:(id)sender;
- (void)changeFrameRate:(id)sender;
- (void)changeViewType:(id)sender;

- (void)scoreChanged:(WormView *)view;
- (void)gameStatusChanged:(WormView *)view;

@end
