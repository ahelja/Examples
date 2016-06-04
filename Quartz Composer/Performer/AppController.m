#import <OpenGL/CGLMacro.h>

#import "AppController.h"

/* APPLICATION DESIGN NOTES:
- This application demonstrates a simple high-performance visual media mixer using Mac OS X Tiger new graphics technologies.
- It uses a Quartz Composer composition to mix two visual media sources and render the result through OpenGL in a window.
- Using a composition allows to easily write complex video mixers in Quartz Composer and leverage the power of OpenGL and Core Image without typing a single line of code.
- To be compatible with this application, the mixing composition must have the following inputs:
	* an image input with the key "SourceA",
	* an image input with the key "SourceB",
	* a number input with the key "SourceMix" that expects values in the range [0,1] where 0 means only SourceA is displayed and 1 only SourceB is displayed.
- The provided mixing composition performs a complex mask transition on configurations that run Core Image hardware accelerated and a simple rotating cube transition otherwise.
- Since Quartz Composer allows image inputs to receive NSImage, CIImage, CGImageRef or CVImageBuffer objects, that gives a lot of flexibility regarding the contents that can be feed to the mixer.
- This flexibility is expressed in this example application by using a modular "Media Source" system based on the MediaSource class.
- The various types of media sources (static images, QuickTime movies, Quartz Composer compositions...) are implemented through distinct subclasses of MediaSource.
- A MediaSource subclass is responsible for reporting the kind of media files it can handle and creating "sources of images" from those files.
- The rendering is driven by a Core Video Display Link which continuously updates the mixing amount, grabs the latest images from both media sources and renders the composition.
*/

#define kSourceAInputKey		@"SourceA"
#define kSourceBInputKey		@"SourceB"
#define kSourceMixKey			@"SourceMix"

/*
This function is called by the Core Video Display Link whenever it's appropriate to render a frame.
*/
static CVReturn _displayLinkCallBack(CVDisplayLinkRef displayLink, const CVTimeStamp* inNow, const CVTimeStamp* inOutputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
	AppController*			controller = (AppController*)displayLinkContext;
	NSAutoreleasePool*		pool;
	
	//Create an autorelease pool (necessary to call Obj-C code from non-Obj-C code)
	pool = [NSAutoreleasePool new];
	
	//Simply ask the application controller to render and display a new frame
	[controller renderAtTime:inOutputTime];
	
	//Destroy the autorelease pool
	[pool release];
	
	return kCVReturnSuccess;
}

@implementation AppController

- (void) applicationDidFinishLaunching:(NSNotification*)notification
{
	NSOpenGLPixelFormatAttribute	attributes[] = {NSOpenGLPFAAccelerated, NSOpenGLPFANoRecovery, NSOpenGLPFADoubleBuffer, NSOpenGLPFADepthSize, 24, 0};
	long							swapInterval = 1;
	NSString*						path;
	
	//Create the OpenGL context used to render the composition and attach it to the rendering view
	_glPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	_glContext = [[NSOpenGLContext alloc] initWithFormat:_glPixelFormat shareContext:nil];
	[_glContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
	[_glContext setView:renderView];
	
	//We need to know when the rendering view frame changes so that we can update the OpenGL context
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateRenderView:) name:NSViewFrameDidChangeNotification object:renderView];
	
	//Get the path to the composition to use to mix the media sources
	path = [[NSBundle mainBundle] pathForResource:@"Mixer" ofType:@"qtz"];
	
	//Create the QCRenderer to render the composition in the OpenGL context
	if(_glContext && path)
	_renderer = [[QCRenderer alloc] initWithOpenGLContext:_glContext pixelFormat:_glPixelFormat file:path];
	
	//Create the CoreVideo display link (for the main display only)
	if(_renderer) {
		if(CVDisplayLinkCreateWithCGDisplay(kCGDirectMainDisplay, &_displayLink) == kCVReturnSuccess)
		CVDisplayLinkSetOutputCallback(_displayLink, _displayLinkCallBack, self);
		else
		_displayLink = NULL;
	}
	
	//Check for errors
	if(!_renderer || !_displayLink) {
		NSLog(@"Initialization failed");
		[NSApp terminate:nil];
	}
	
	//Configure the media source views
	[mediaViewA setOpenGLContext:_glContext pixelFormat:_glPixelFormat];
	[mediaViewB setOpenGLContext:_glContext pixelFormat:_glPixelFormat];
	
	//Setup the initial media sources and mixing amount
	[mediaViewA setMediaSourceWithFile:@"/System/Library/CoreServices/Setup Assistant.app/Contents/Resources/TransitionSection.bundle/Contents/Resources/Intro.mov"];
	[mediaViewB setMediaSourceWithFile:@"/System/Library/Screen Savers/Spectrum.qtz"];
	_mixAmount = 0.5;
	
	//Start rendering
	_startTime = -1.0;
	CVDisplayLinkStart(_displayLink);
}

- (void) updateRenderView:(NSNotification*)notification
{
	bool					running = CVDisplayLinkIsRunning(_displayLink);
	NSRect					frame = [renderView frame];
	CGLContextObj			cgl_ctx = [_glContext CGLContextObj]; //By using CGLMacro.h there's no need to set the current OpenGL context
	
	//We have to stop rendering to prevent the Core Video Display Link thread from accessing the OpenGL context at the same time
	if(running)
	CVDisplayLinkStop(_displayLink);
	
	//Notify the OpenGL context its rendering view has changed
	[_glContext update];
	
	//Update the OpenGL viewport
	glViewport(0, 0, frame.size.width, frame.size.height);
	
	//We can safely restart rendering now
	if(running)
	CVDisplayLinkStart(_displayLink);
}

/*
WARNING: This method is called from the CoreVideo Display Link thread.
Make sure the operations performed here are not conflicting with the ones on the application's main thread.
*/
- (void) renderAtTime:(const CVTimeStamp*)time
{
	NSTimeInterval			videoTime,
							localTime;
	id						image;
	
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
	
	//Retrieve a new image from media source A and pass it to the composition (only if it is actually visible)
	if((_mixAmount < 1.0) && [_sourceA isNewImageAvailableForTime:time]) {
		image = [_sourceA copyImageForTime:time];
		[_renderer setValue:image forInputKey:kSourceAInputKey];
		[image release];
	}
	
	//Retrieve a new image from media source B and pass it to the composition (only if it is actually visible)
	if((_mixAmount > 0.0) && [_sourceB isNewImageAvailableForTime:time]) {
		image = [_sourceB copyImageForTime:time];
		[_renderer setValue:image forInputKey:kSourceBInputKey];
		[image release];
	}
	
	//Pass the mixing amount between the 2 media sources to the composition
	[_renderer setValue:[NSNumber numberWithDouble:_mixAmount] forInputKey:kSourceMixKey];
	
	//Render a frame from the composition
	[_renderer renderAtTime:localTime arguments:nil];
	
	//Display new frame
	[_glContext flushBuffer];
}

- (BOOL) windowShouldClose:(id)sender
{
	//Quits the app when the window is closed
	[NSApp terminate:self];
	return YES;
}

- (void) applicationWillTerminate:(NSNotification*)notification
{
	//Stop rendering
	CVDisplayLinkStop(_displayLink);
	
	//Stop observing the rendering view
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSViewFrameDidChangeNotification object:renderView];
}

- (void) dealloc
{
	//Release our objects
	if(_displayLink)
	CVDisplayLinkRelease(_displayLink);
	[_sourceA release];
	[_sourceB release];
	[_renderer release];
	[_glContext release];
	[_glPixelFormat release];
	
	[super dealloc];
}

@end

@implementation AppController (IBActions)

- (IBAction) takeMixingAmount:(id)sender
{
	//Set the mixing amount from the current value of the NSSlider in the UI (we use the "_mixAmount" variable to communicate that value to the Core Video Display Link thread)
	_mixAmount = [sender doubleValue];
}

- (IBAction) takeMediaSource:(id)sender
{
	bool					running = CVDisplayLinkIsRunning(_displayLink);
	
	//We have to stop rendering to prevent the Core Video Display Link thread from using the media sources while we are changing them
	if(running)
	CVDisplayLinkStop(_displayLink);
	
	//Replace the appropriate media source with the new one and reset the corresponding composition input (useful in case the new media source is nil)
	if(sender == mediaViewA) {
		[_sourceA release];
		_sourceA = [[mediaViewA mediaSource] retain];
		[_renderer setValue:nil forInputKey:kSourceAInputKey];
	}
	else if(sender == mediaViewB) {
		[_sourceB release];
		_sourceB = [[mediaViewB mediaSource] retain];
		[_renderer setValue:nil forInputKey:kSourceBInputKey];
	}
	
	//We can safely restart rendering now
	if(running)
	CVDisplayLinkStart(_displayLink);
}

@end
