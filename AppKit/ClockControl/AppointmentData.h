#import <Foundation/Foundation.h>

@interface AppointmentData : NSObject {
    NSCalendarDate    *time;
    NSString          *info;
}

- (void)setTime:(NSCalendarDate *)newTime;
- (NSCalendarDate *)time;

- (void)setInfo:(NSString *)newInfo;
- (NSString *)info;

@end
