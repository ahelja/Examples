//
//  ServiceProvider.h
//  DemoAssistant
//
//  Created by Ali Ozer on Fri Apr 26 2002.
//  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface ServiceProvider : NSObject {
    NSString *demoScriptFileName;
    NSAttributedString *attributedString;
    NSRange lastShownLineRange;
    IBOutlet NSComboBox *demoScriptFileNameField;
}

/* Action methods */
- (IBAction)changeDemoScriptFileName:(id)sender;
- (IBAction)browseForDemoScriptFileName:(id)sender;

/* Service methods */
- (void)getNextLine:(NSPasteboard *)pboard userData:(NSString *)data error:(NSString **)error;
- (void)rewind:(NSPasteboard *)pboard userData:(NSString *)data error:(NSString **)error;
- (void)moveUpOneLine:(NSPasteboard *)pboard userData:(NSString *)data error:(NSString **)error;
- (void)moveDownOneLine:(NSPasteboard *)pboard userData:(NSString *)data error:(NSString **)error;

/* Other methods */
- (void)recordNewFileName:(NSString *)newFileName;
- (void)setDemoScriptFileName:(NSString *)newFileName;
- (NSAttributedString *)demoScriptText;

@end
