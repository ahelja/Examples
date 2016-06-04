#import <Cocoa/Cocoa.h>

@interface ClockCell : NSActionCell {
    NSCalendarDate *time;
}

- (void)setTime:(NSCalendarDate *)newTime;
- (NSCalendarDate *)time;

- (IBAction)takeMinuteValueFrom:(id)sender;
- (IBAction)takeHourValueFrom:(id)sender;

// Used by accessibility to implement increment/decrement actions
- (void)moveRight:(id)sender;
- (void)moveLeft:(id)sender;

@end


@interface ClockControl : NSControl {
}

- (void)setTime:(NSCalendarDate *)newTime;
- (NSCalendarDate *)time;

- (void)takeMinuteValueFrom:(id)sender;
- (void)takeHourValueFrom:(id)sender;

@end
