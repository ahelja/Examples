/* XMethodInspector */

#import <Cocoa/Cocoa.h>

@interface XMethodInspector : NSObject
{
    IBOutlet id fDescription;
    IBOutlet id fDiscussionURL;
    IBOutlet id fEmail;
    IBOutlet id fID;
    IBOutlet id fImplementationID;
    IBOutlet id fInfoURL;
    IBOutlet id fName;
    IBOutlet id fNotes;
    IBOutlet id fPublisherID;
    IBOutlet id fShortDescription;
    IBOutlet id fTModelID;
    IBOutlet id fUUID;
    IBOutlet id fWSDLURL;
}

-(void) updateFromDict:(NSDictionary*) dict;

@end
