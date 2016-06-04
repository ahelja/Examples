#import "ImageFilterController.h"
#include "FunctionMenu.h"
#import "KernelPane.h"
#import "MyImageFilterView.h"

@implementation ImageFilterController


- (AlphaPane*)alphaPane
{
    return alphaPane;
}



- (IBAction)applyFilter:(id)sender
{
    if( YES == isWorking )
    {
        [ imageView stopFilter  ];
    }
    else
    {
        NSRect frame = [ progressBarSuperview frame ];
        NSPoint newOrigin;
        
        newOrigin.x = progressBarOffset.x + frame.size.width;
        newOrigin.y = progressBarOffset.y;
        isWorking = YES;
        [ progressBar setFrameOrigin: newOrigin ];
        [ progressBarSuperview 	addSubview: progressBar  ];
        [ progressBar setNeedsDisplay: YES ];
        [ progressBar incrementBy: 1.0 / (double) [ imageView  iterationCount ] ];
        [ imageView applyFilter:self  ];
    }
}

-(void)awakeFromNib
{
    NSRect progressBarFrame, superviewFrame;

    //Init our main window 
    [ imageView initObject ];

    //Init the kernel pane
    [ kernelPane initObject ];
    
    //Set up the function menu
    [ functionMenu initObject ];
    
    //Config the progress bar
    [ progressBar setMaxValue: 1.0 ];
    [ progressBar retain ];
    progressBarSuperview = [ progressBar superview ];
    progressBarFrame = [ progressBar frame ];
    superviewFrame = [ progressBarSuperview frame ];
    progressBarOffset.x = progressBarFrame.origin.x - superviewFrame.size.width;
    progressBarOffset.y = progressBarFrame.origin.y;
    [ progressBar removeFromSuperview ];
    
    //Init the number of test iterations
    [ self setTestIterations: testIterationPopup ];
}

- (void) dealloc
{
    [ progressBar release ];
}

- (void) endFilter
{
    [ goButton setTitle: [ imageView filterName ] ];
    [ progressBar removeFromSuperview ];
    isWorking = NO;
}



- (IBAction)findImage:(id)sender
{
    NSOpenPanel	*panel = [ NSOpenPanel openPanel ];
    
    [ panel setCanChooseFiles: YES ];
    [ panel setCanChooseDirectories: NO ];
    [ panel setResolvesAliases: YES ];
    [ panel setAllowsMultipleSelection: NO ];
    
    [panel 	beginSheetForDirectory: nil 
                file: nil
                types: [NSBitmapImageRep imageFileTypes]
                modalForWindow: theWindow
                modalDelegate: self
                didEndSelector: @selector( handleFoundImage:returnCode:contextInfo:)
                contextInfo: nil ];
}

- (void)flushTestFrame:(FilterTest*)test
{
    [ imageView flushTestFrame: test ];
}


- (FunctionPane*)functionPane
{
    return functionPane;
}



- (GeometryPane*)geometryPane
{
    return geometryPane;
}

- (void)handleFoundImage: (NSOpenPanel *)panel returnCode:(int)returnCode contextInfo:(void  *)contextInfo
{    
    NSString	*imagePath;
    
    if( NSOKButton != returnCode )
        return;

    imagePath = [[ panel filenames ] objectAtIndex: 0];
    if( 0 == [ imageView setImage: imagePath ] )
    {
        NSSize size = [ imageView imageSize ];

        if( size.height <= 0.0 || size.width <= 0.0 )
            [ imageSizeDisplayField setObjectValue: nil ];
        else
        {
            char	type[256];
            NSString	*colorSpaceName = [ imageView imageColorSpaceName ];
            NSString 	*sizeString;
            
            [ colorSpaceName getCString: type maxLength: 255 ];
            sizeString = [ NSString stringWithFormat: @"%i x %i (%i channels, %s, %s)", 
                                    (int) size.width, 
                                    (int) size.height, 
                                    [ imageView imageColorChannelCount ], 
                                    [ imageView channelLayout ],
                                    type ];
            [ imageSizeDisplayField setStringValue: sizeString ];
        }
        
        [ theWindow setTitleWithRepresentedFilename: imagePath ];
        [ timeDisplayField setObjectValue: nil ];
    }
}

-(Kernel*)kernel
{
    Kernel *k = nil;
    int format = [ imageView dataFormat ];
    int filter = [ imageView filter ];
    
    switch( format )
    {
        case Depth_8_bits_per_channel:
            k = [ kernelPane kernelForFilter: filter	isFP: NO ];
            break;
        case Depth_32_bit_FP_per_channel:
            k = [ kernelPane kernelForFilter: filter	isFP: YES ];
            break;
        default:
            //Do nothing
            break;
    }
            
    return k;
}

- (IBAction)resetImage:(id)sender
{
    [ imageView restoreImage];
}

- (IBAction)setDataType:(id)sender
{
    NSMenu	*menu = [ sender menu ];
    id	competitor = nil;
    
    if( [sender isSeparatorItem ] )
        return;

    switch( [sender tag ] )
    {
        case 1:
            competitor = [ menu itemWithTag: 2 ];
            [ sender setState: NSOnState ];
            [ competitor setState: NSOffState ];
            [ imageView setDataFormat: 8 ];
            break;
        case 2:
            competitor = [ menu itemWithTag: 1 ];
            [ sender setState: NSOnState ];
            [ competitor setState: NSOffState ];
            [ imageView setDataFormat: 32 ];
            break;
        case 3: 
            competitor = [ menu itemWithTag: 4 ];
            [ sender setState: NSOnState ];
            [ competitor setState: NSOffState ];
            [ imageView setInterleaved: NO ];
            [ [menu itemWithTag: 5] setEnabled: NO ];
            break;
        case 4:
            competitor = [ menu itemWithTag: 3 ];
            [ sender setState: NSOnState ];
            [ competitor setState: NSOffState ];
            [ imageView setInterleaved: YES ];
            [ [menu itemWithTag: 5] setEnabled: YES ];
            break;
        case 5:
            [ sender setState: ! [ sender state ] ];
            [ imageView setLeaveAlphaUnchanged: [ sender state ] ];
            break;
    }
    
    [ functionMenu enableMenuItems ];
    
    if( NO == [ [ functionMenu currentFilterMenuItem] isEnabled ] )
        [ functionMenu turnOnDefaultItem ];
}

- (IBAction)setFilterType:(id)sender
{
    [ (FunctionMenu*) functionMenu turnOnItem: sender ];

    if( nil != sender && NSOnState == [ sender state ] )
    {
        [ goButton setTitle: [ sender title ] ];
        [ goButton setEnabled: YES ];
        [ imageView setFilter: [ sender tag ]  ];
    }
    else
    {
        [ goButton setTitle: @"(None)" ];
        [ goButton setEnabled: NO ];
        [ imageView setFilter: kNoFilter  ];
    }

    //Set up the kernel pane to display the current filter
    [ kernelPane setFilter: [ imageView filter ] ];    
}

- (void)setProgress: (double)progress
{
    if( progress > 1.0 )
        progress = 1.0;
        
    if( progress < 0.0 )
         progress = 0.0;

    [ progressBar setDoubleValue: progress ];
    [ goButton setTitle: @"Stop" ];
}

- (IBAction)setTestIterations:(id)sender
{
    NSMenuItem *item = [ sender selectedItem ];
    int value = [item tag];
    [ imageView setTestIterations: value ];
}

-( void ) showTime: (double) time
{
    NSString *timeString = [ NSString stringWithFormat: @"%4.4e", time ];
    
    [ timeDisplayField setStringValue: timeString ];

    [ imageView setNeedsDisplay: YES ];
}



@end
