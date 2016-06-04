//
//  LazyDataTextStorage.h
//

#import <Cocoa/Cocoa.h>


@interface LazyDataTextStorage : NSTextStorage {
    NSString *myString;
    NSDictionary *myAttributes;
}
- (void)setAttributes:(NSDictionary *)attrs;
- (void)setData:(NSData *)data;
- (NSData *)data;
@end
