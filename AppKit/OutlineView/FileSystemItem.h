#import <Foundation/Foundation.h>

@interface FileSystemItem : NSObject {
    NSString *relativePath;
    FileSystemItem *parent;
    NSMutableArray *children;
}

+ (FileSystemItem *)rootItem;
- (int)numberOfChildren;			// Returns -1 for leaf nodes
- (FileSystemItem *)childAtIndex:(int)n;	// Invalid to call on leaf nodes
- (NSString *)fullPath;
- (NSString *)relativePath;

@end
