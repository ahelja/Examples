/* MyImageFilterView */

#import "Filters.h"
#import "FilterTest.h"

@interface MyImageFilterView : NSView
{
    IBOutlet 			NSTextField *sizeOfDisplay;
    NSString 			*startingImagePath;
    volatile NSBitmapImageRep 	*image;
    int 			filter;
    int				dataFormat;
    int				testIterations;
    BOOL			useVector;
    BOOL			isInterleaved;
    BOOL			shouldDoTrace;
    BOOL			leaveAlphaUnchanged;
    NSLock			*imageModificationLock;
    volatile FilterTest 	*currentTest;
}


- (void)applyFilter:(id)imageFilterController;
- (const char*)channelLayout;
- (void) doTrace:(BOOL)shouldDoTrace;
- (void)enableVectorUnit:(BOOL)isOn;
- (NSString*)filterName;
- (int)dataFormat;
- (int)filter;
- (void)flushTestFrame:(FilterTest*)test;
- (int)imageColorChannelCount;
- (NSString*)imageColorSpaceName;
- (NSSize)imageSize;
- (void)initObject;
- (BOOL)isInterleaved;
- (BOOL)isVectorEnabled;
- (BOOL)isVectorAvailable;
- (int)iterationCount;
- (int)restoreImage;
- (void)setDataFormat: (int) formatType;
- (int)setImage:(NSString *)imagePath;
- (void)setFilter:(int) filter;
- (void)setInterleaved: (BOOL) isInterleaved;
- (void)setLeaveAlphaUnchanged: (BOOL) leaveAlphaUnchanged;
- (void)setTestIterations:(int)count;
- (void)stopFilter;
- (void)testThread:(id)controller;

@end
