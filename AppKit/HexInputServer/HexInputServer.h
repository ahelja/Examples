/*
 HexInputServer.h
*/

#import <AppKit/NSInputServer.h>

@class NSTextField;
@class HexInputModePalette;

// This class implements NSInputServiceProvider protocol
@interface HexInputServer : NSObject <NSInputServiceProvider> {
    NSMutableDictionary *_clientTable; // table contains HexInputContext objects
    HexInputModePalette *_hexInputModePalette;
    id _currentClientContext;
    BOOL _isEnabled;
}

+ (id)sharedInstance;
+ (NSTextField *)inputFeedbackField;
- (HexInputModePalette *)inputModePalette;
- (void)setCurrentInputContextModeToHex:(BOOL)flag;
@end
