/* PicBrowserController */

#import <Cocoa/Cocoa.h>

@interface PicBrowserController : NSObject
{
    IBOutlet id imageView;
    IBOutlet id hostNameField;
    IBOutlet id pictureServiceList;
    IBOutlet NSProgressIndicator * progressIndicator;

    NSNetServiceBrowser * browser;
    NSMutableArray * services;
    NSMutableData * currentDownload;
}
- (IBAction)serviceClicked:(id)sender;
@end
