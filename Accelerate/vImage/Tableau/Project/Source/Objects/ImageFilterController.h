/* ImageFilterController */


@class MyImageFilterView;
@class FunctionMenu;
@class KernelPane;
@class Kernel;
@class GeometryPane;
@class FunctionPane;
@class FilterTest;
@class AlphaPane;
@interface ImageFilterController : NSObject
{
    IBOutlet MyImageFilterView *imageView;
    IBOutlet NSTextField *timeDisplayField;
    IBOutlet NSTextField *imageSizeDisplayField;
    IBOutlet NSWindow *theWindow;
    IBOutlet NSButton *goButton;
    IBOutlet NSPopUpButton *testIterationPopup;
    IBOutlet FunctionMenu *functionMenu;
    IBOutlet NSProgressIndicator *progressBar;
    IBOutlet KernelPane *kernelPane;
    IBOutlet GeometryPane *geometryPane;
    IBOutlet FunctionPane *functionPane;
    IBOutlet AlphaPane *alphaPane;
    
    NSView *progressBarSuperview;
    NSPoint  progressBarOffset;
    BOOL     isWorking;
}
- (IBAction)applyFilter:(id)sender;
- (void)awakeFromNib;
- (void)dealloc;
- (void)endFilter;
- (IBAction)findImage:(id)sender;
- (void)flushTestFrame:(FilterTest*)test;
- (void)handleFoundImage: (NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo;
- (Kernel*)kernel;
- (IBAction)resetImage:(id)sender;
- (IBAction)setDataType:(id)sender;
- (IBAction)setFilterType:(id)sender;
- (void)setProgress: (double)progress;
- (IBAction)setTestIterations:(id)sender;
- (void)showTime: (double)time;
- (AlphaPane*)alphaPane;
- (FunctionPane*)functionPane;
- (GeometryPane*)geometryPane;

@end
