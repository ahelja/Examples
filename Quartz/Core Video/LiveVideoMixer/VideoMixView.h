/* VideoMixView */

#import <Cocoa/Cocoa.h>
#import <pthread.h>

@interface VideoMixView : NSOpenGLView
{
    id				controller;
    pthread_mutex_t		drawingLock;	// this is the lock to serialize the multithreaded access to the view
    BOOL			contextIsInitialized;
    BOOL			dirty;
	
    NSOpenGLContext		*sharedContext;
	
    BOOL			needsReshape;
}

- (void)setupSharedContext;	    // create the shared context
- (NSOpenGLContext*)sharedContext;  // return the shared context. This is the context that QT will use for the textures

- (void)lock;			    // aquire the mutex lock
- (void)unlock;			    // release the mutex lock

- (void)reshape;		    // view was scrolled, moved in the window or resized

- (void)clear;			    // clean the output
- (void)flushOutput;		    // render the output to the screen

- (void)setDirty:(BOOL)inDirty;	    // set when something new is to be rendered
- (BOOL)dirty;			    // check if there is something new to be rendered

@end
