#import "MediaSource.h"
#import "MediaSourceView.h"

@interface AppController : NSObject
{
	IBOutlet NSView*			renderView;
	IBOutlet MediaSourceView*	mediaViewA;
	IBOutlet MediaSourceView*	mediaViewB;
	
	NSOpenGLContext*			_glContext;
	NSOpenGLPixelFormat*		_glPixelFormat;
	QCRenderer*					_renderer;
	CVDisplayLinkRef			_displayLink;
	NSTimeInterval				_startTime;
	
	double						_mixAmount;
	MediaSource*				_sourceA;
	MediaSource*				_sourceB;
}
- (void) renderAtTime:(const CVTimeStamp*)time;
@end

@interface AppController (IBActions)
- (IBAction) takeMixingAmount:(id)sender;
- (IBAction) takeMediaSource:(id)sender;
@end
