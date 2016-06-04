/* VideoChannel */

#import <Cocoa/Cocoa.h>
#import "VideoMixView.h"

@interface VideoChannel : NSObject
{
    IBOutlet id					channelOpacity;
    IBOutlet id					channelShape;
    NSRect					targetRect;
    GLfloat					lowerLeft[2]; 
    GLfloat					lowerRight[2]; 
    GLfloat					upperRight[2];
    GLfloat					upperLeft[2];
    
    Movie						channelMovie;
    QTVisualContextRef			visualContext;
	CVOpenGLTextureRef			currentTexture;
}

+ (id)createWithFilePath:(NSString*)theFilePath forView:(VideoMixView*)inView;


- (id)initWithFilePath:(NSString*)theFilePath forView:(VideoMixView*)inView;

- (void)startMovie;
- (void)stopMovie;

- (BOOL)renderChannel:(const CVTimeStamp*)syncTimeStamp;

- (float)opacity;

- (void)setTargetRect:(NSRect)inRect;
- (NSRect)targetRect;		

- (void)drawOutline:(NSRect)destRect;
- (void)compositeChannelInRect:(NSRect)destRect;
- (void)compositeChannelThumbnailInRect:(NSRect)destRect;

@end
