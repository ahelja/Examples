/* LiveVideoMixerController */

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CoreVideo.h>


#define	kNumTextures				4
#define	kTextureTarget				GL_TEXTURE_RECTANGLE_EXT
#define	kSpeechBubbleTextureID			kNumTextures - 1
#define	kSpeechBubble				1
#define	kSelectionHandleSize			20.0f
#define	kVideoWidth				720.0f
#define	kVideoHeight				480.0f

extern GLuint				gTexNames[kNumTextures];

typedef enum SelectionHandleID
{
	kNoSelectionHandle = 0,
	kTopLeftCorner,
	kTopRightCorner,
	kBottomLeftCorner,
	kBottomRightCorner
} SelectionHandleID;

@class VideoMixView;

@interface LiveVideoMixerController : NSObject
{
    IBOutlet id channelController0;
    IBOutlet id channelController1;
    IBOutlet id channelController2;
    IBOutlet VideoMixView *mainView;
    IBOutlet id playButton;
    IBOutlet id positionSwitch0;
    IBOutlet id positionSwitch1;
    IBOutlet id positionSwitch2;
    
    BOOL				isPlaying;
    CVDisplayLinkRef			displayLink;
    SInt32				currentChannelPositioned;	
    NSTimer				*qtHousekeepingTimer;
    
    CGDirectDisplayID			mainViewDisplayID;
}
- (IBAction)openChannelFile:(id)sender;
- (IBAction)setChannelPosition:(id)sender;
- (IBAction)togglePlayback:(id)sender;

- (BOOL)isPlaying;

- (NSRect)mainVideoRect;
- (void)drawContent;

- (CVReturn)render:(const CVTimeStamp*)syncTimeStamp;

- (void)mouseDown:(NSEvent *)theEvent;

- (void)windowChangedScreen:(NSNotification*)inNotification;

@end

CVReturn myCVDisplayLinkOutputCallback(CVDisplayLinkRef displayLink, 
                                                const CVTimeStamp *inNow, 
                                                const CVTimeStamp *inOutputTime, 
                                                CVOptionFlags flagsIn, 
                                                CVOptionFlags *flagsOut, 
                                                void *displayLinkContext);
