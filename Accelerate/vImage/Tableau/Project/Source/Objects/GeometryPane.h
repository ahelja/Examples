/* GeometryPane */

#import <Cocoa/Cocoa.h>
#include "Filters.h"

#ifndef USING_ACCELERATE
    #include <vImage/vImage_Types.h>
#endif

@interface GeometryPane : NSWindow
{
    IBOutlet NSButton *backColor;
    IBOutlet NSButton *goButton;
    IBOutlet NSSlider *rotate;
    IBOutlet NSSlider *scale_X;
    IBOutlet NSSlider *scale_Y;
    IBOutlet NSSlider *shear_X;
    IBOutlet NSSlider *shear_Y;
    IBOutlet NSTableView *transformMatrix;
    IBOutlet NSSlider *translate_X;
    IBOutlet NSSlider *translate_Y;
    
    vImage_AffineTransform	transform;
    float	r, g, b, a;
    id		target;
    SEL		action;
    int		height, width;
    BOOL	quitting;
}
- (IBAction)doColor:(id)sender;
- (IBAction)go:(id)sender;
- (IBAction)updateTransformMatrix:(id)sender; //Take the current geometry information and use it to render an image

//selector is for -(void)action:(TransformInfo*)info;
-(void)initWithTarget:(id)target action:(SEL)action function:(uint32_t)func height:(int)h width:(int)w; 

//Send the image to the screen
-(void)flushImage;

//functions for supporting the table view holding the matrix
-(int)numberOfRowsInTableView:(NSTableView *)aTableView;
-(id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;
-(void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;

//functions for the color panel
-(void)changeColor:(id)sender;

@end
