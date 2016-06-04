/* FunctionMenu */

#include "MyImageFilterView.h"

@interface FunctionMenu : NSMenu
{
     IBOutlet id controller;
     IBOutlet MyImageFilterView *imageView;
}
- (void)initObject;
- (void)enableMenuItems;
- (void)turnOnItem:(id)item;
- (IBAction)useVector:(id)sender;
- (IBAction)doTrace:(id)sender;
- (void)turnOnDefaultItem;
- (id)currentFilterMenuItem;

@end

