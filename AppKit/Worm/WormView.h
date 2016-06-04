#import <Cocoa/Cocoa.h>
#import "WormGuts.h"
#import "WormController.h"

@interface WormView : NSView {
    WormController *controller;
    unsigned initialWormLength;
    unsigned wormLength;
    unsigned score;
    NSString *wormString;
    NSMutableDictionary *wormTextAttributes;
    float desiredFrameRate;
    float actualFrameRate;
    CFRunLoopTimerRef wormTimer;
    CFAbsoluteTime timeStamp;
    GamePosition *wormPositions;
    GameDirection wormDirection;
    GameHeading wormHeading;
    GamePosition targetPosition;
    NSColor *backgroundColor;
    BOOL gameOver;
}

/* Set/get worm body string */
- (void)setString:(NSString *)string;
- (NSString *)string;

/* Animation parameters */
- (void)setInitialLength:(unsigned)length;
- (void)setDesiredFrameRate:(float)rate;
- (float)actualFrameRate;

/* Controller */
- (void)setController:(WormController *)obj;
- (WormController *)controller;

/* Manage the score */
- (int)score;
- (void)setScore:(int)newScore;

/* Game control */
- (void)start;
- (void)stop:(BOOL)gameOverFlag;   // Pass in YES if game is over
- (void)reset;			   // Stop game and reset for new game
- (BOOL)gameIsRunning;
- (BOOL)gameIsOver;

/* For the use of subclasses */
- (BOOL)updateState;
- (NSRect)rectForPosition:(GamePosition)position;
- (NSRect)integralRectForRect:(NSRect)rect;

/* To be overridden in subclasses */
- (BOOL)performAnimation;
- (void)drawRect:(NSRect)rect;

@end
