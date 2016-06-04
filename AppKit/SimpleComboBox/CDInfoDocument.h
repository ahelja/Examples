//
//  CDInfoDocument.h
//
//  Copyright (c) 2001-2002, Apple. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface CDInfoDocument : NSDocument {
@private
    IBOutlet NSForm 		*infoForm;
    IBOutlet NSFormCell 	*bandNameCell;
    IBOutlet NSFormCell 	*cdTitleCell;
    
    IBOutlet NSComboBox		*genreComboBox;
    IBOutlet NSComboBoxCell	*genreComboBoxCell;
    IBOutlet NSTextView 	*infoTextView;
    
    NSString			*initEditString;
    NSMutableArray 		*genres;
    
    NSData 			*dataFromFile;
}

- (void)setBandName:(NSString *)name;
- (NSString *)bandName;

- (void)setCDTitle:(NSString *)title ;
- (NSString *)cdTitle;

- (void)setMusicGenre:(NSString *)genre;
- (NSString *)musicGenre;

@end
