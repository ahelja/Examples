/* VideoController */

#import <Cocoa/Cocoa.h>
#import "VideoView.h"

@interface VideoController : NSObject
{
    IBOutlet id filterDrawer;
    IBOutlet id nextButton;
    IBOutlet id playButton;
    IBOutlet id prevButton;
    IBOutlet id timecodeField;
    IBOutlet id videoScrubber;
    IBOutlet VideoView* videoView;
}
- (IBAction)openMovie:(id)sender;

- (void)movieTimeChanged:(VideoView*)sender;
- (void)movieStateChanged;

@end
