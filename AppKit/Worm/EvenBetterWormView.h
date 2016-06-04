#import <Cocoa/Cocoa.h>
#import "BetterWormView.h"

@interface EvenBetterWormView : BetterWormView {
    NSTextStorage *wormStorage;
    NSLayoutManager *wormLayout;
    NSTextContainer *wormContainer;
}

@end
