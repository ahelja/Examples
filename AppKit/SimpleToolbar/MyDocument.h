//
//  MyDocument.h
//
//  Copyright (c) 2001-2002, Apple. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MyDocument : NSDocument {
@private
    IBOutlet NSTextView 	*documentTextView;	// The textview part of our document.
    IBOutlet NSTextField	*searchFieldOutlet;	// "Template" textfield needed to create our toolbar searchfield item.
    IBOutlet NSToolbarItem	*activeSearchItem;	// A reference to the search field in the toolbar, null if the toolbar doesn't have one!
    IBOutlet NSWindow		*documentWindow;
    IBOutlet NSView		*contentView;
    
    NSData			*dataFromFile;		
}
@end
