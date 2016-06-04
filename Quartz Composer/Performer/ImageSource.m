#import "MediaSource.h"

@interface ImageSource : MediaSource
{
	NSImage*					_image;
	BOOL						_newImageAvailable;
}
@end

@implementation ImageSource

+ (void) load
{
	//Register automatically this MediaSource subclass when the Obj-C runtime loads it
	[MediaSource registerMediaSourceClass:[self class]];
}

+ (NSArray*) supportedFileExtensions
{
	//Simply returns all the file extensions supported by NSImage
	return [NSImage imageUnfilteredFileTypes];
}

- (id) initWithFile:(NSString*)path openGLContext:(NSOpenGLContext*)context pixelFormat:(NSOpenGLPixelFormat*)pixelFormat
{
	if(self = [super init]) {
		//Create an NSImage from the file
		_image = [[NSImage alloc] initWithContentsOfFile:path];
		if(_image == nil) {
			[self release];
			return nil;
		}
		_newImageAvailable = YES;
	}
	
	return self;
}

- (BOOL) isNewImageAvailableForTime:(const CVTimeStamp*)time
{
	//Only return NO if we never provided an image
	return _newImageAvailable;
}

- (id) copyImageForTime:(const CVTimeStamp*)time
{
	//We won't have any new image to provide
	_newImageAvailable = NO;
	
	//Always return the same image
	return [_image retain];
}

- (void) dealloc
{
	//Release the image
	[_image release];
	
	[super dealloc];
}

@end
