@interface App : NSApplication
{
	IBOutlet GLView* glView;
}

- (IBAction) modeSelect: (id) sender;
- (IBAction) alphaSlider: (id) sender;

@end
