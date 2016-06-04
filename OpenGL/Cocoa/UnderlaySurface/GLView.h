@interface GLView : NSOpenGLView
{
	IBOutlet NSTextField* fpsText;
	BOOL _mode;
	float _alpha;
	QMatrix _cube_xform;
}

- (void) timerUpdate;
- (void) setMode: (BOOL) mode;
- (void) setAlpha: (float) alpha;
@end
