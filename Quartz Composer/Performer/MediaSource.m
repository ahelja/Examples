#import "MediaSource.h"

//This global variable holds the registry of media source classes
static NSMutableArray*				_registry = NULL;

@implementation MediaSource

+ (void) registerMediaSourceClass:(Class)aClass
{
	//Allocate the registry if necessary
	if(_registry == NULL)
	_registry = [NSMutableArray new];
	
	//If the class is valid and not already in the registry, add it
	if([aClass isSubclassOfClass:[MediaSource class]] && ![_registry containsObject:aClass])
	[_registry addObject:aClass];
}

+ (Class) _mediaSourceClassForFile:(NSString*)path
{
	NSString*						extension = [path pathExtension];
	unsigned						i,
									j;
	Class							aClass;
	NSArray*						extensions;
	
	//Scan classes in the registry for one that supports the extension of that file
	for(i = 0; i < [_registry count]; ++i) {
		aClass = [_registry objectAtIndex:i];
		extensions = [aClass supportedFileExtensions];
		for(j = 0; j < [extensions count]; ++j) {
			if([extension caseInsensitiveCompare:[extensions objectAtIndex:j]] == NSOrderedSame)
			return aClass;
		}
	}
	
	return nil;
}

+ (BOOL) canCreateMediaSourceWithFile:(NSString*)path
{
	//Checks if there is a media source class that can handle this file
	return ([self _mediaSourceClassForFile:path] != nil ? YES : NO);
}

+ (MediaSource*) mediaSourceWithFile:(NSString*)path openGLContext:(NSOpenGLContext*)context pixelFormat:(NSOpenGLPixelFormat*)pixelFormat
{
	Class							aClass;
	
	//If there is a media source class that can handle this file, create an instance of it
	if(aClass = [self _mediaSourceClassForFile:path])
	return [[[aClass alloc] initWithFile:path openGLContext:context pixelFormat:pixelFormat] autorelease];
	
	return nil;
}

@end

@implementation MediaSource (Core)

+ (NSArray*) supportedFileExtensions
{
	[self doesNotRecognizeSelector:_cmd];
	
	return nil;
}

- (id) initWithFile:(NSString*)path openGLContext:(NSOpenGLContext*)context pixelFormat:(NSOpenGLPixelFormat*)pixelFormat
{
	[self doesNotRecognizeSelector:_cmd];
	
	return nil;
}

- (BOOL) isNewImageAvailableForTime:(const CVTimeStamp*)time
{
	[self doesNotRecognizeSelector:_cmd];
	
	return NO;
}

- (id) copyImageForTime:(const CVTimeStamp*)time
{
	[self doesNotRecognizeSelector:_cmd];
	
	return nil;
}

@end
