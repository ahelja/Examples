/* XMethodsServiceList */

#import <Cocoa/Cocoa.h>

@interface XMethodsServiceList : NSObject
{
    IBOutlet id fList;
	NSArray* fMethods;
}

-(NSString*) idForItem:(int) item;

@end
