#define kSlideShowInterval			3.0

@interface SlideShowApplication : NSApplication
{
@private
	NSMutableArray*				_fileList;
	NSOpenGLContext*			_openGLContext;
	QCRenderer*					_renderer;
}
@end

@implementation SlideShowApplication

- (id) init
{
	//We need to be our own delegate
	if(self = [super init])
	[self setDelegate:self];
	
	return self;
}

- (void) applicationDidFinishLaunching:(NSNotification*)aNotification 
{
	NSArray*						imageFileTypes = [NSImage imageFileTypes];
	long							value = 1;
	NSOpenGLPixelFormatAttribute	attributes[] = {
														NSOpenGLPFAFullScreen,
														NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
														NSOpenGLPFANoRecovery,
														NSOpenGLPFADoubleBuffer,
														NSOpenGLPFAAccelerated,
														NSOpenGLPFADepthSize, 24,
														(NSOpenGLPixelFormatAttribute) 0
													};
	NSOpenGLPixelFormat*			format = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
	NSOpenPanel*					openPanel;
	NSDirectoryEnumerator*			enumerator;
	NSString*						basePath;
	NSString*						subPath;
	
	//Ask the user for a directory of images
	openPanel = [NSOpenPanel openPanel];
	[openPanel setAllowsMultipleSelection:NO];
	[openPanel setCanChooseDirectories:YES];
	[openPanel setCanChooseFiles:NO];
	if([openPanel runModalForDirectory:nil file:nil types:nil] != NSOKButton) {
		NSLog(@"No directory specified");
		[NSApp terminate:nil];
	}
	
	//Populate an array with all the image files in the directory (no recursivity)
	_fileList = [NSMutableArray new];
	basePath = [[openPanel filenames] objectAtIndex:0];
	enumerator = [[NSFileManager defaultManager] enumeratorAtPath:basePath];
	while(subPath = [enumerator nextObject]) {
		if([[[enumerator fileAttributes] objectForKey:NSFileType] isEqualToString:NSFileTypeDirectory]) {
			[enumerator skipDescendents];
			continue;
		}
		if([imageFileTypes containsObject:[subPath pathExtension]])
		[_fileList addObject:[basePath stringByAppendingPathComponent:subPath]];
	}
	if([_fileList count] < 2) {
		NSLog(@"The directory contain less than 2 image files");
		[NSApp terminate:nil];
	}
	
	//Capture the main screen
	CGDisplayCapture(kCGDirectMainDisplay);
	CGDisplayHideCursor(kCGDirectMainDisplay);
	
	//Create the fullscreen OpenGL context on the main screen (double-buffered with color and depth buffers)
	_openGLContext = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
	if(_openGLContext == nil) {
		NSLog(@"Cannot create OpenGL context");
		[NSApp terminate:nil];
	}
	[_openGLContext setFullScreen];
	[_openGLContext setValues:&value forParameter:kCGLCPSwapInterval];
	
	//Create the QuartzComposer Renderer with that OpenGL context and the transition composition file
	_renderer = [[QCRenderer alloc] initWithOpenGLContext:_openGLContext pixelFormat:format file:[[NSBundle mainBundle] pathForResource:@"Transition" ofType:@"qtz"]];
	if(_renderer == nil) {
		NSLog(@"Cannot create QCRenderer");
		[NSApp terminate:nil];
	}
	
	//Run first transition ASAP
	[self performSelector:@selector(_performTransition:) withObject:nil afterDelay:0.0];
}

- (void) _performTransition:(id)param
{
	double					time;
	NSImage*				image;
	
	//Load the next image
	image = [[NSImage alloc] initWithData:[NSData dataWithContentsOfFile:[_fileList objectAtIndex:0]]];
	if(image == nil)
	NSLog(@"Cannot load image file at path: %@", [_fileList objectAtIndex:0]);
	[_fileList removeObjectAtIndex:0];
	
	//Set transition source image (just get it from the previous destination image)
	[_renderer setValue:[_renderer valueForInputKey:@"destination"] forInputKey:@"source"];
	
	//Set transition destination image (the new image)
	[_renderer setValue:image forInputKey:@"destination"];
	
	//Release next image
	[image release];
	
	//Render transition - FIXME: do that from a runloop timer to avoid blocking the application and have accurate timing
	for(time = 0.0; time < 1.0; time += 0.01) {
		if(![_renderer renderAtTime:time arguments:nil])
		NSLog(@"Rendering failed at time %.3fs", time);
		[_openGLContext flushBuffer];
	}
	if(![_renderer renderAtTime:1.0 arguments:nil]) //This is necessary to make sure the last image rendered is at time 1.0 exactly (otherwise, we might have visual garbage)
	NSLog(@"Rendering failed at time %.3fs", time);
	[_openGLContext flushBuffer];
	
	//Schedule next transition
	if([_fileList count])
	[self performSelector:@selector(_performTransition:) withObject:nil afterDelay:kSlideShowInterval];
}

- (void) sendEvent:(NSEvent*)event
{
	//If the user pressed the [Esc] key, we need to exit
	if(([event type] == NSKeyDown) && ([event keyCode] == 0x35))
	[NSApp terminate:nil];
	
	[super sendEvent:event];
}

- (void) applicationWillTerminate:(NSNotification*)aNotification 
{
	//Destroy the renderer
	[_renderer release];
	
	//Destroy the OpenGL context
	[_openGLContext clearDrawable];
	[_openGLContext release];
	
	//Release main screen
	if(CGDisplayIsCaptured(kCGDirectMainDisplay)) {
		CGDisplayShowCursor(kCGDirectMainDisplay);
		CGDisplayRelease(kCGDirectMainDisplay);
	}
	
	//Release file list
	[_fileList release];
}

@end

int main(int argc, const char *argv[])
{
    return NSApplicationMain(argc, argv);
}
