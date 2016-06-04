@interface MediaSource : NSObject

/*
Subclasses must call this method to add themselves to the media source classes registry.
*/
+ (void) registerMediaSourceClass:(Class)aClass;

/*
Given a file, checks if there is a registered media source class that can handle it.
*/
+ (BOOL) canCreateMediaSourceWithFile:(NSString*)path;

/*
Given a file, tries to find a registered media source class that can handle it and instantiate it on the provided OpenGL context.
*/
+ (MediaSource*) mediaSourceWithFile:(NSString*)path openGLContext:(NSOpenGLContext*)context pixelFormat:(NSOpenGLPixelFormat*)pixelFormat;

@end

@interface MediaSource (Core)

/*
Implemented by subclasses to return the list of file extensions they can handle.
*/
+ (NSArray*) supportedFileExtensions;

/*
Implemented by subclasses to initialize from a file and on a given destination OpenGL context.
*/
- (id) initWithFile:(NSString*)path openGLContext:(NSOpenGLContext*)context pixelFormat:(NSOpenGLPixelFormat*)pixelFormat;

/*
Implemented by subclasses to return YES is there is a new image available since the last time -copyImage was called.
Note that this method will be called from the Core Video Display Link thread.
*/
- (BOOL) isNewImageAvailableForTime:(const CVTimeStamp*)time;

/*
Implemented by subclasses to return the newest image.
The returned image must be a Quartz Composer compatible object i.e. NSImage, CIImage, CGImageRef or CVImageBufferRef.
Note that this method will be called from the Core Video Display Link thread.
*/
- (id) copyImageForTime:(const CVTimeStamp*)time;

@end
