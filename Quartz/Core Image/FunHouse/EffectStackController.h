// EffectStackController.h
// Author: Mark Zimmer
// 12/08/04
// Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.

#import <AppKit/AppKit.h>

@class CoreImageView;
@class EffectStack;
@class FilterView;
@class CIFilter;

@interface EffectStackController : NSWindowController
{
    IBOutlet NSButton *topPlusButton;           // the plus button at the top of the effect stack inspector (outside of any layer box)
    IBOutlet NSButton *resetButton;             // the reset button at the top of the effect stack inspector (outside of any layer box)
    IBOutlet NSButton *playButton;              // the play button at the top of the effect stack inspector (outside of any layer box)
    CoreImageView *_inspectingCoreImageView;    // pointer to the core image view that is currently associated with the effect stack inspector
    EffectStack *_inspectingEffectStack;        // pointer to the effect stack that is currently associated with the effect stack inspector
    BOOL needsUpdate;                           // set this to re-layout the effect stack inspector on update
    NSMutableArray *boxes;                      // an array of FilterView (subclass of NSBox) that make up the effect stack inspector's UI
    // filter palette stuff
    IBOutlet NSWindow *filterPalette;           // the filter palette (called image units)
    IBOutlet NSButton *filterOKButton;          // the apply button, actually
    IBOutlet NSButton *filterCancelButton;      // the cancel button
    IBOutlet NSTableView *categoryTableView;    // the category table view
    IBOutlet NSTableView *filterTableView;      // the filter list table view
    IBOutlet NSButton *filterTextButton;        // the text button
    int currentCategory;                        // the currently selected row in the category table view
    int currentFilterRow;                       // the currently selected row in the filter table view
    NSMutableDictionary *categories;            // a dictionary containing all filter category names and the filters that populate the category
    NSString *filterClassname;                  // returned filter's classname from the modal filter palette (when a filter has been selected)
    NSTimer *timer;                             /// playing all transitions
    // globals used in the sequential animation of all transitions
    double transitionStartTime;
    double transitionDuration;
    double transitionEndTime;
}

+ (id)sharedEffectStackController;

- (void)setAutomaticDefaults:(CIFilter *)f atIndex:(int)index;
- (IBAction)topPlusButtonAction:(id)sender;
- (IBAction)plusButtonAction:(id)sender;
- (IBAction)minusButtonAction:(id)sender;
- (IBAction)resetButtonAction:(id)sender;
- (void)playButtonAction:sender;
- (void)layoutInspector;
- (FilterView *)createUIForFilter:(CIFilter *)f index:(int)index;
- (FilterView *)createUIForImage:(CIImage *)im filename:(NSString *)filename index:(int)index;
- (FilterView *)createUIForText:(NSString *)string index:(int)index;
- (void)_loadFilterListIntoInspector;

- (IBAction)filterOKButtonAction:(id)sender;
- (IBAction)filterCancelButtonAction:(id)sender;
- (IBAction)filterImageButtonAction:(id)sender;
- (IBAction)filterTextButtonAction:(id)sender;
- (IBAction)tableViewDoubleClick:(id)sender;
- (void)setNeedsUpdate:(BOOL)b;
- (void)updateLayout;
- (BOOL)effectStackFilterHasMissingImage:(CIFilter *)f;
- (void)closeDown;
- (void)setLayer:(int)index image:(CIImage *)im andFilename:(NSString *)filename;
- (void)setChanges;
- (void)setCoreImageView:(CoreImageView *)v;
- (void)removeFilterImageOrTextAtIndex:(NSNumber *)index;
- (void)reconfigureWindow; // called when dragging into or choosing base image to reconfigure the document's window

// for retaining full file names of images
- (void)registerImageLayer:(int)index imageFilePath:(NSString *)path;
- (void)registerFilterLayer:(CIFilter *)filter key:(NSString *)key imageFilePath:(NSString *)path;
- (NSString *)imageFilePathForImageLayer:(int)index;
- (NSString *)imageFilePathForFilterLayer:(CIFilter *)filter key:(NSString *)key;
@end

@interface EffectStackBox : NSBox
{
    CIFilter *filter;
    EffectStackController *master;
}

- (void)drawRect:(NSRect)r;
- (void)setFilter:(CIFilter *)f;
- (void)setMaster:(EffectStackController *)m;
@end
