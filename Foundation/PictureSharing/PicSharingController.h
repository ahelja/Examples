// PicSharingController.h

#import <Cocoa/Cocoa.h>

@interface PicSharingController : NSObject
{
    IBOutlet id imageView;
    IBOutlet id longerStatusText;
    IBOutlet id serviceNameField;
    IBOutlet id shortStatusText;
    IBOutlet id toggleSharingButton;
    IBOutlet id picturePopUpMenu;

    NSNetService * netService;
    NSFileHandle * listeningSocket;
    
    int numberOfDownloads;
}
- (IBAction)toggleSharing:(id)sender;
- (IBAction)popupChangedPicture:(id)sender;
@end
