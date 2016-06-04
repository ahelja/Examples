/* KernelPane */

#include "Filters.h"

@class ImageFilterController;
@class Kernel;
@interface KernelPane : NSWindow
{
    IBOutlet ImageFilterController	*controller;
    IBOutlet NSButton 			*cancelButton;
    IBOutlet NSMatrix 			*intFloatSelector;
    IBOutlet NSTableView 		*kernelData;
    IBOutlet NSPopUpButton 		*kernelPrefabType;
    IBOutlet NSForm 			*kernelSize;
    IBOutlet NSButton 			*applyButton;
    
    NSMutableArray			*kernelList;
    NSMutableArray			*kernelListFP;
}
- (void)initObject;
- (IBAction)applyChanges:(id)sender;
- (IBAction)cancelChanges:(id)sender;
- (IBAction)setPrefabType:(id)sender;
- (IBAction)setSize:(id)sender;
- (IBAction)setIntOrFloat:(id)sender;

- (void)setupPrefabMenu;
- (void)setFilter:(int)filter;
- (BOOL)isShowingFP;
- (Kernel*)kernelForFilter:(int)filter  isFP:(BOOL)isFP;
@end
