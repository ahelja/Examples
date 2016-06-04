

#import "MyImageFilterView.h"
#import "ImageFilterController.h"
#import "FilterTest.h"
#include <sys/sysctl.h>

#ifndef USING_ACCELERATE
    #import <vImage/vImage_Types.h>
#endif

@implementation MyImageFilterView


- (void) applyFilter: (ImageFilterController*)control
{
    if( nil != currentTest )
        return;

    //Start the test in another thread
    [ NSThread 	detachNewThreadSelector:	@selector( testThread: )  
                toTarget:			self 
                withObject:			control 	];

}


- (const char*)channelLayout
{
    const char *layout = "";
    
    if( image != nil )
    {
        if( [  (NSBitmapImageRep*)  image isPlanar ] )
        {
            if( [   (NSBitmapImageRep*) image hasAlpha ] )
                layout = "planar w/alpha";
            else
                layout = "planar";
        }
        else
        {
            if( [  (NSBitmapImageRep*)  image hasAlpha ] )
                layout = "interleaved w/alpha";
            else
                layout = "interleaved";
        }
    }
    
    return layout;
}


- (int) dataFormat
{
    return dataFormat;
}

- (void) dealloc
{
    [ (NSBitmapImageRep*) image release ];
}

- (void) doTrace:(BOOL)isTraced
{
    shouldDoTrace = isTraced;
}


- (void) drawRect:(NSRect)aRect
{
    NSRect insideRect = aRect;
    
    NSDrawWhiteBezel( [self bounds], aRect );

    insideRect.origin.x += 2.0;
    insideRect.origin.y += 2.0;
    insideRect.size.width -= 4.0;
    insideRect.size.height -= 4.0;
    
    if( insideRect.size.width > 0.0 && insideRect.size.height > 0.0 && nil != image )
    {
        NSSize  size = [ (NSBitmapImageRep*) image size ];
        int height = ceil( size.height );
        int width = ceil( size.width );
        float widthRatio =  insideRect.size.width / (float) width;
        float heightRatio = insideRect.size.height / (float) height;
        float heightReduction = 0.0;
        float widthReduction = 0.0;
        
        if( widthRatio < heightRatio )
        {
            float newHeight = insideRect.size.height * widthRatio / heightRatio;
            heightReduction = insideRect.size.height - newHeight;
        }
        else
        {
            float newWidth = insideRect.size.width * heightRatio / widthRatio;
            widthReduction = insideRect.size.width - newWidth;
        }
        
        insideRect.origin.x += widthReduction  * 0.5;
        insideRect.size.width -= widthReduction;
        insideRect.origin.y += heightReduction  * 0.5;
        insideRect.size.height -= heightReduction;
        
        [ imageModificationLock lock ];
        [  (NSBitmapImageRep*) image drawInRect: insideRect ];
        [ imageModificationLock unlock ];

        [ sizeOfDisplay setStringValue: [NSString stringWithFormat: @"%i x %i", 
                                                                    (int) insideRect.size.width,
                                                                    (int) insideRect.size.height  ]];
    }
    else
        [ sizeOfDisplay setObjectValue: nil ];


}

- (void)enableVectorUnit:(BOOL)isOn
{
    useVector = isOn;
}


- (int) filter;
{
    return filter;
}

- (NSString*) filterName
{
    int set = filter >> 16;
    int item = filter & 0xFFFF;
    
    return filterLists[set].list[item].name;
}

- (void)flushTestFrame:(FilterTest*)test
{
    [ imageModificationLock lock ];
    [ test copyResultToNSBitmapImageRep: (NSBitmapImageRep*) image ];
    [ imageModificationLock unlock ];

    //Notify self that I need to display
    [ self setNeedsDisplay: YES ];
}

- (int)imageColorChannelCount
{
    int result = 0;
    
    if( image != nil )
        result = [  (NSBitmapImageRep*) image samplesPerPixel ];
        
    return result;
}

- (NSSize)imageSize
{
    NSSize size = {0.0, 0.0 };

    if( nil != image )
        size = [ (NSBitmapImageRep*) image size];
        
    return size;
}

- (id)init
{
    [super init];

    return self;
}


- (NSString*)imageColorSpaceName 
{
    if( nil == image )
        return @"";

    return [ (NSBitmapImageRep*) image colorSpaceName ];
}

- (void)initObject
{
    startingImagePath = nil;
    image = nil;
    useVector = NO;
    isInterleaved = NO;
    dataFormat = Depth_8_bits_per_channel;
    filter = kNoFilter;
    testIterations = 1;
    
    imageModificationLock = [[ NSLock alloc] init ];
    [ imageModificationLock tryLock ];
    [ imageModificationLock unlock ];

    [ [ self window ] makeKeyAndOrderFront: nil ];
}


- (BOOL) isInterleaved
{
    return isInterleaved;
}

- (BOOL)isVectorAvailable
{
    int selectors[2] = { CTL_HW, HW_VECTORUNIT };
    int hasVectorUnit = 0;
    size_t length = sizeof(hasVectorUnit);
    int error = sysctl(selectors, 2, &hasVectorUnit, &length, NULL, 0); 
    
    if(0 == error && 0 != hasVectorUnit )
        return YES;
    
    return NO;
}

- (BOOL)isVectorEnabled
{
    return useVector;
}

- (int)iterationCount
{
    return testIterations;
}



- (int) restoreImage
{
    NSArray *array = nil;
    NSBitmapImageRep *theImage = nil;
    id das_image = nil;
    int count;
    int i;
    
    if( nil == startingImagePath )
        return -1;
    
    [ imageModificationLock lock ];
    array = [ NSImageRep imageRepsWithContentsOfFile: startingImagePath ];
    count = [ array count ];
    
    for( i = 0; i < count; i++ )
    {
        das_image = [ array objectAtIndex: i ];
        if( [ das_image isMemberOfClass: [NSBitmapImageRep class ] ] )
        {
            [ das_image retain ];
            theImage = (NSBitmapImageRep*) das_image;
            break;
        }
    }
    
    if( nil == theImage )
        return -1;
    
    [  (NSBitmapImageRep*)  image release ];
    image = theImage;
    
    [ self setNeedsDisplay: YES ];
    
    [ imageModificationLock unlock ];
                            
    return 0;
}



- (void)setInterleaved: (BOOL) is_interleaved
{
    isInterleaved = is_interleaved;
}


-(void)setTestIterations: (int) count
{
    testIterations = count;
}


- (void)setDataFormat: (int) formatType
{
    dataFormat = formatType;
}



- (void)setFilter:(int) the_filter
{
    int set = the_filter >> 16;
    int func = the_filter & 0xFFFFUL;
    
    //Bounds sanity checking
    if( the_filter >= 0 )
    {
        if( set >= kListCount )
            the_filter = -1L;
        else
            if( func >= filterLists[ set ].count )
                the_filter = -1L;
    }
    
    filter = the_filter;
}

- (int)setImage:(NSString*)imagePath
{
    [ startingImagePath release ];
    startingImagePath = imagePath;
    [ startingImagePath retain ];
    
    return [self restoreImage];
}

- (void)setLeaveAlphaUnchanged: (BOOL) new_leaveAlphaUnchanged
{
    leaveAlphaUnchanged = new_leaveAlphaUnchanged;
}


- (void) stopFilter
{
    //Race condition here probably
    FilterTest *test = (FilterTest*) currentTest;
    [ test setIterationCount:0 ];
}


-(void)testThread:(id)controller
{
    double time;
    vImage_Flags flags = kvImageNoFlags;
    
    if( NO != leaveAlphaUnchanged )
        flags |= kvImageLeaveAlphaUnchanged;
    
    if( YES == [ imageModificationLock tryLock ] )
    {
        NSAutoreleasePool *pool = [[ NSAutoreleasePool alloc] init];
    
        [ (ImageFilterController*) controller setProgress: 0.0 ];

        currentTest = [ FilterTest 	constructTestForFilter: filter
                                        usingVector: useVector
                                        isPlanar: ! isInterleaved 
                                        usingDataFormat: dataFormat	
                                        withKernel: [controller kernel]
                                        andFlags: flags ];
    
        //Run the test
        [ (FilterTest*) currentTest doAmberTrace: shouldDoTrace ];
        [ (FilterTest*) currentTest loadImageFromNSBitmapImageRep: (NSBitmapImageRep*) image ];

        //release the lock
        [ imageModificationLock unlock ];

        [ (FilterTest*) currentTest setIterationCount: testIterations ];
        time = [ (FilterTest*)  currentTest  runTest: controller ];

    
        [ imageModificationLock lock ];
        [ (FilterTest*) currentTest copyResultToNSBitmapImageRep: (NSBitmapImageRep*) image ];
        [ imageModificationLock unlock ];

        currentTest = nil;

        //show the time
        [ controller showTime: time ];
        
        //Notify self that I need to display
        [ self setNeedsDisplay: YES ];
        
        [ pool release ];
    }
    
    currentTest = nil;
}


@end
