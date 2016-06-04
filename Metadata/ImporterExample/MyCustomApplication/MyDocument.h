#import <Cocoa/Cocoa.h>

@interface MyDocument : NSDocument
{
    NSMutableDictionary *myData;
}

- (void)setMyData:(NSMutableDictionary *)theDictionary;
- (NSMutableDictionary *)myData;
@end
