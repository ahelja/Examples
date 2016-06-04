#import "MediaSource.h"

@interface MovieSource : MediaSource
{
	NSOpenGLContext*			_glContext;
	NSOpenGLPixelFormat*		_glPixelFormat;
	QTVisualContextRef			_visualContext;
	Movie						_movie;
}
@end

@implementation MovieSource

+ (void) load
{
	//Register automatically this MediaSource subclass when the Obj-C runtime loads it
	[MediaSource registerMediaSourceClass:[self class]];
	
	//We also need to initialize QuickTime
	EnterMovies();
}

+ (NSArray*) supportedFileExtensions
{
	//We handle movie files types (make sure QuickTime does not handle Quartz Composer ".qtz" files as there is an explicit media source class for them)
	return [NSArray arrayWithObjects:@"mov", @"avi", @"dv", @"mpg", nil];
}

- (id) initWithFile:(NSString*)path openGLContext:(NSOpenGLContext*)context pixelFormat:(NSOpenGLPixelFormat*)pixelFormat
{
	Boolean						active = TRUE;
	QTNewMoviePropertyElement	properties[] = {
									{kQTPropertyClass_DataLocation, kQTDataLocationPropertyID_CFStringNativePath, sizeof(CFStringRef), &path, 0},
									{kQTPropertyClass_NewMovieProperty, kQTNewMoviePropertyID_Active, sizeof(Boolean), &active, 0},
									{kQTPropertyClass_Context, kQTContextPropertyID_VisualContext, sizeof(QTVisualContextRef), &_visualContext, 0}
								};
	OSStatus					error;
	
	if(self = [super init]) {
		//Make sure the OpenGL context will not go away
		_glContext = [context retain];
		_glPixelFormat = [pixelFormat retain];
		
		//Create a pixel buffer visual context to play the movie into
		error = QTOpenGLTextureContextCreate(NULL, [context CGLContextObj], [pixelFormat CGLPixelFormatObj], NULL, &_visualContext);
		
		//Open the movie
		if(error == noErr)
		error = NewMovieFromProperties(sizeof(properties) / sizeof(QTNewMoviePropertyElement), properties, 0, NULL, &_movie);
		
		//Check for errors
		if(error != noErr) {
			[self release];
			return nil;
		}
		
		//Make sure movie plays in high-quality (much better for DV content) - FIXME: Should we specify "hintsDeinterlaceFields" too?
		SetMoviePlayHints(_movie, hintsHighQuality, hintsHighQuality);
		
		//Make the movie loop continuously
		SetTimeBaseFlags(GetMovieTimeBase(_movie), loopTimeBase);
		
		//Start playing movie immediately
		GoToBeginningOfMovie(_movie);
		StartMovie(_movie);
	}
	
	return self;
}

- (BOOL) isNewImageAvailableForTime:(const CVTimeStamp*)time
{
	//Give some time to QuickTime
	MoviesTask(_movie, 0);
	
	//Give some time to the visual context
	QTVisualContextTask(_visualContext); 
	
	//Check if we have a new image available
	return QTVisualContextIsNewImageAvailable(_visualContext, time);
}

- (id) copyImageForTime:(const CVTimeStamp*)time
{
	CVOpenGLTextureRef				imageBuffer;
	
	//Retrieve an image
	if(QTVisualContextCopyImageForTime(_visualContext, NULL, time, &imageBuffer) != kCVReturnSuccess)
	return NULL;
	
	return (id)imageBuffer;
}

- (void) dealloc
{
	//Release all objects
	if(_movie) {
		StopMovie(_movie);
		DisposeMovie(_movie);
	}
	if(_visualContext)
	QTVisualContextRelease(_visualContext);
	[_glContext release];
	[_glPixelFormat release];
	
	[super dealloc];
}

@end
