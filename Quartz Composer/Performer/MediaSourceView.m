#import "MediaSourceView.h"

@implementation MediaSourceView

- (void) setOpenGLContext:(NSOpenGLContext*)context pixelFormat:(NSOpenGLPixelFormat*)pixelFormat
{
	//Keep the OpenGL around so that we can use it to create media sources
	if(context != _glContext) {
		[_glContext release];
		_glContext = [context retain];
	}
	if(pixelFormat != _glPixelFormat) {
		[_glPixelFormat release];
		_glPixelFormat = [pixelFormat retain];
	}
}

- (void) setMediaSourceWithFile:(NSString*)path
{
	//Create a media source from the file and set the displayed image to its icon on success
	[_mediaSource release];
	_mediaSource = [[MediaSource mediaSourceWithFile:path openGLContext:_glContext pixelFormat:_glPixelFormat] retain];
	[self setImage:(_mediaSource ? [[NSWorkspace sharedWorkspace] iconForFile:path] : nil)];
	
	//Notify the target the media source has changed
	[[self target] performSelector:[self action] withObject:self];
}

- (MediaSource*) mediaSource
{
	return _mediaSource;
}

- (void) dealloc
{
	//Release all objects
	[_mediaSource release];
	[_glContext release];
	[_glPixelFormat release];
	
	[super dealloc];
}

@end

/*
This subclass of NSImageView overrides the drag & drop methods so that only supported media files are accepted.
When a media file is dropped on the view, a media source is created from it and the displayed image is set to the icon of the file.
*/
@implementation MediaSourceView (NSDragging)

- (NSDragOperation) draggingEntered:(id<NSDraggingInfo>)sender
{
	NSPasteboard*			pboard = [sender draggingPasteboard];
	NSArray*				files;
	
	//Check if the dragging pasteboard contains a single file and that we can create a media source from it
	if([pboard availableTypeFromArray:[NSArray arrayWithObject:NSFilenamesPboardType]]) {
		files = [pboard propertyListForType:NSFilenamesPboardType];
		if([files count] == 1) {
			if([MediaSource canCreateMediaSourceWithFile:[files objectAtIndex:0]])
			return NSDragOperationCopy;
		}
	}
	
	return NSDragOperationNone;
}

- (void) draggingExited:(id<NSDraggingInfo>)sender
{
	//Do nothing
}

- (BOOL) prepareForDragOperation:(id<NSDraggingInfo>)sender
{
	//Do nothing
	return YES;
}

- (BOOL) performDragOperation:(id<NSDraggingInfo>)sender
{
	NSPasteboard*			pboard = [sender draggingPasteboard];
	
	//Set the media source from the file
	if([pboard availableTypeFromArray:[NSArray arrayWithObject:NSFilenamesPboardType]]) {
		[self setMediaSourceWithFile:[[pboard propertyListForType:NSFilenamesPboardType] objectAtIndex:0]];
		return YES;
	}
	
	return NO;
}

- (void) concludeDragOperation:(id<NSDraggingInfo>)sender
{
	//Do nothing
}

@end
