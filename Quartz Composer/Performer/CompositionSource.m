#import <OpenGL/CGLMacro.h>

#import "MediaSource.h"

/*
Quartz Composer compositions are resolution-agnostic so we arbitrarily render them at 640x480
*/
#define kCompositionRenderWidth 640
#define kCompositionRenderHeight 480

@interface CompositionSource : MediaSource
{
	NSOpenGLContext*				_glContext;
	QCRenderer*						_renderer;
	CVOpenGLBufferPoolRef			_bufferPool;
	NSTimeInterval					_startTime;
}
@end

@implementation CompositionSource

+ (void) load
{
	//Register automatically this MediaSource subclass when the Obj-C runtime loads it
	[MediaSource registerMediaSourceClass:[self class]];
}

+ (NSArray*) supportedFileExtensions
{
	//We only handle Quartz Composer composition files
	return [NSArray arrayWithObject:@"qtz"];
}

- (id) initWithFile:(NSString*)path openGLContext:(NSOpenGLContext*)context pixelFormat:(NSOpenGLPixelFormat*)pixelFormat
{
	NSOpenGLPixelFormatAttribute	attributes[] = {NSOpenGLPFAAccelerated, NSOpenGLPFANoRecovery, NSOpenGLPFADepthSize, 24, 0};
	NSOpenGLPixelFormat*			glPixelFormat;
	NSMutableDictionary*			bufferOptions;
	CGLContextObj					cgl_ctx; //By using CGLMacro.h there's no need to set the current OpenGL context
	
	if(self = [super init]) {
		//Create the OpenGL context used to render the composition (a separate OpenGL context from the destination one is needed to render into CoreVideo OpenGL buffers)
		glPixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
		_glContext = [[NSOpenGLContext alloc] initWithFormat:glPixelFormat shareContext:nil];
		if(cgl_ctx = [_glContext CGLContextObj])
		glViewport(0, 0, kCompositionRenderWidth, kCompositionRenderHeight);
		
		//Create the QCRenderer to render the composition in the OpenGL context
		if(_glContext != nil)
		_renderer = [[QCRenderer alloc] initWithOpenGLContext:_glContext pixelFormat:glPixelFormat file:path];
		
		//Create a CoreVideo OpenGL buffer pool to provide the image buffers to render the composition in
		if(_renderer != nil) {
			bufferOptions = [NSMutableDictionary dictionary];
			[bufferOptions setValue:[NSNumber numberWithInt:kCompositionRenderWidth] forKey:(NSString*)kCVOpenGLBufferWidth];
			[bufferOptions setValue:[NSNumber numberWithInt:kCompositionRenderHeight] forKey:(NSString*)kCVOpenGLBufferHeight];
			if(CVOpenGLBufferPoolCreate(NULL, NULL, (CFDictionaryRef)bufferOptions, &_bufferPool) != kCVReturnSuccess)
			_bufferPool = NULL;
		}
		
		//Check for errors
		if(!_renderer || !_bufferPool) {
			[self release];
			return nil;
		}
		
		//Ready to start rendering
		_startTime = -1.0;
	}
	
	return self;
}

- (BOOL) isNewImageAvailableForTime:(const CVTimeStamp*)time
{
	//Since we render on demand, we always have new images available
	return YES;
}

- (id) copyImageForTime:(const CVTimeStamp*)time
{
	CVOpenGLBufferRef				imageBuffer = NULL;
	CGLContextObj					cgl_ctx = [_glContext CGLContextObj]; //By using CGLMacro.h there's no need to set the current OpenGL context
	NSTimeInterval					videoTime,
									localTime;
	
	//Compute the video time
	if(time->flags & kCVTimeStampVideoTimeValid)
	videoTime = (NSTimeInterval)time->videoTime / (NSTimeInterval)time->videoTimeScale;
	else
	videoTime = 0; //Not sure what the best thing to do is
	
	//Compute the local time as the difference between the current video time and the video time at which the first frame was rendered
	if(_startTime < 0) {
		_startTime = videoTime;
		localTime = 0;
	}
	else
	localTime = videoTime - _startTime;
	
	//Get a new OpenGL buffer from the pool
	if(CVOpenGLBufferPoolCreateOpenGLBuffer(NULL, _bufferPool, &imageBuffer) == kCVReturnSuccess) {
		//Use the buffer as the OpenGL context destination
		if(CVOpenGLBufferAttach(imageBuffer, cgl_ctx, 0, 0, 0) == kCVReturnSuccess) {
			//Render one frame of the Quartz Composer composition at the current time
			[_renderer renderAtTime:localTime arguments:nil];
			
			//Make sure OpenGL rendering commands were sent to the graphics driver so that the buffer contents is immediately usable
			glFlush();
		}
		else {
			CVOpenGLBufferRelease(imageBuffer);
			imageBuffer = NULL;
		}
	}
	
	return (id)imageBuffer;
}

- (void) dealloc
{
	//Release all objects
	if(_bufferPool)
	CVOpenGLBufferPoolRelease(_bufferPool);
	[_renderer release];
	[_glContext release];
	
	[super dealloc];
}

@end
