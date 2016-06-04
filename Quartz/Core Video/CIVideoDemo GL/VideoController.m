#import <QuartzCore/QuartzCore.h>
#import "VideoController.h"
#import "VideoView.h"

@interface VideoController (private)

- (void)_loadMovie:(NSString *)path;

@end

@implementation VideoController

//--------------------------------------------------------------------------------------------------

- (void)awakeFromNib
{
    // only needed if you rely on Image Units [CIPlugIn loadAllPlugIns];
    [filterDrawer open];
}

//--------------------------------------------------------------------------------------------------

- (void)applicationWillFinishLaunching:(NSNotification *)note
{
    NSString *moviePath;

    /* See if we have a known pathname */
    moviePath = [[NSUserDefaults standardUserDefaults] stringForKey:@"MoviePath"];
    if(moviePath)
    {
        [self _loadMovie:moviePath];
    }
}

//--------------------------------------------------------------------------------------------------

- (void)openMovie:(id)sender
{
    NSOpenPanel *openPanel;
    int rv;
    
    openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:NO];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setResolvesAliases:YES];
    [openPanel setCanChooseFiles:YES];
    
    rv = [openPanel runModalForTypes:nil];
    if(rv == NSFileHandlingPanelOKButton)
        [self _loadMovie:[openPanel filename]];
}

//--------------------------------------------------------------------------------------------------

- (void)_loadMovie:(NSString *)path
{
    QTMovie	*qtMovie = nil;
    NSError	*error;
        
    qtMovie = [QTMovie movieWithFile:path error:&error];
    if(qtMovie)
    {
        [[NSUserDefaults standardUserDefaults] setObject:path forKey:@"MoviePath"];
        [[NSUserDefaults standardUserDefaults] synchronize];
        [videoView setQTMovie:qtMovie];
	[[videoView window] setTitleWithRepresentedFilename:path];
	[nextButton setEnabled:YES];
	[prevButton setEnabled:YES];
	[playButton setEnabled:YES];
	[videoScrubber setEnabled:YES];
    }
}

//--------------------------------------------------------------------------------------------------
// DELEGATE methods
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------

- (BOOL)windowShouldClose:(id)sender	//close box quits the app
{
    [NSApp terminate:self];
    return YES;
}

//--------------------------------------------------------------------------------------------------

- (void)movieTimeChanged:(VideoView*)sender
{
    QTTime  currentTime = [sender currentTime];
    QTTime  duration	= [sender movieDuration];
    NSString	*timeString = QTStringFromTime(currentTime);
    
    if(timeString)
	[timecodeField setStringValue:timeString];
    [videoScrubber setDoubleValue:(double)currentTime.timeValue / (double)duration.timeValue];
        
}

//--------------------------------------------------------------------------------------------------

- (void)movieStateChanged
{

}

//--------------------------------------------------------------------------------------------------

@end
