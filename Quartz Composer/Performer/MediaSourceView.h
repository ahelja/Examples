#import "MediaSource.h"

@interface MediaSourceView : NSImageView
{
	NSOpenGLContext*			_glContext;
	NSOpenGLPixelFormat*		_glPixelFormat;
	MediaSource*				_mediaSource;
}
- (void) setOpenGLContext:(NSOpenGLContext*)context pixelFormat:(NSOpenGLPixelFormat*)pixelFormat;

- (void) setMediaSourceWithFile:(NSString*)path;
- (MediaSource*) mediaSource;
@end
