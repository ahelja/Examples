/* AlphaPane */

#import <Cocoa/Cocoa.h>

@interface AlphaPane : NSWindow
{
    IBOutlet NSButton *okButton;
    IBOutlet NSSlider *transparencySlider;
    IBOutlet NSTextField *valueDisplay;

    float	alpha;
    id		target;
    SEL		action;
    int		height, width;
    BOOL	quitting;
}

-(void)initWithTarget:(id)target action:(SEL)action function:(uint32_t)func height:(int)h width:(int)w; 

- (IBAction)doOK:(id)sender;
- (IBAction)doSlider:(id)sender;

//Send the image to the screen
-(void)flushImage;


@end
