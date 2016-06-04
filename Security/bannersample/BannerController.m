/*
File:			BannerController.m

Description: 	This is the implementation file for the Controller class, which implements  It
				also contains a C wrapper so that it can be called from the Authorization API
				code.
				
				The -changeTransparency:action is called when the slider on the window is moved.
				By default, the slider is off the screen since it is just used for testing.
*/

#include <Security/AuthorizationPlugin.h>
#import "BannerController.h"

static BannerController *mBannerController = nil;
static void *lastMechanismRef;

// Inactivity timer, in seconds
#ifndef NDEBUG
	#define kInactivityDelay 60.0
#else
	#define kInactivityDelay 10.0	// shorter delay for debugging
#endif

#pragma mark ----------------- C Interface ----------------------

OSStatus setResult(AuthorizationMechanismRef inMechanism);
OSStatus initializeBanner(AuthorizationMechanismRef inMechanism, int modal);
OSStatus finalizeBanner(AuthorizationMechanismRef inMechanism);

OSStatus initializeBanner(AuthorizationMechanismRef inMechanism, int modal)
{
	if (!mBannerController)
	{
		mBannerController = [[BannerController alloc] init];
		[NSApp activateIgnoringOtherApps:YES];
		[mBannerController showWindow:nil];
	}
	[mBannerController setRef:inMechanism];
	return 0;
}

OSStatus finalizeBanner(AuthorizationMechanismRef inMechanism)
{
	[mBannerController release];
	return 0;
}

#pragma mark ----------------- Objective-C Interface ----------------------

@implementation BannerController

- (id)init
{
	if ([super init])
		self = [super initWithWindowNibName:@"bannersample"];

    return self;
}

- (void)dealloc
{
    [super dealloc];
	mBannerController = nil;
	lastMechanismRef = 0;
}

- (void)setRef:(void *)ref
{
	mMechanismRef = ref;
	lastMechanismRef = ref;
}

- (void)setModal:(BOOL)modal
{
	mModal = modal;
}

- (void)redisplayWindow
{
	[[self window] makeKeyAndOrderFront:nil];
	[[self window] display];
}

- (void)startTimer
{
	[NSTimer
		scheduledTimerWithTimeInterval: kInactivityDelay
								target: self
							  selector: @selector(redisplayWindow)
							  userInfo: nil
							   repeats: NO];
}

- (IBAction)okClicked: (id) sender
{
	[self showWarningWindow:sender];
}

- (void)dismissBanner
{
	// Signal to SecurityAgent that we are done
    if (mModal)
		setResult(lastMechanismRef);
	else
		[self startTimer];

	// Hide window in either case
	[[self window] orderOut:nil];
}

// This method changes the transparency for the *entire window*, not some particular object.  Thus,
// all objects drawn in this window, even if drawn at full alpha value, will pick up this setting.

- (IBAction)changeTransparency:(id)sender
{	
	// Set the window's alpha value from 0.0-1.0
	[[self window] setAlphaValue:[sender floatValue]];
	// Go ahead and tell the window to redraw things, which has the effect of calling CustomView's -drawRect: routine
	[[self window] display];
}

#pragma mark ----------------- Sheet for Warning Window ----------------------

- (IBAction)showWarningWindow: (id) sender
{
	// We can't use NSBeginInformationalAlertSheet because the disclaimer text will generally be too long
	
	[self startWarningDismissTimer];
    [NSApp beginSheet:warningWindow modalForWindow:[self window] modalDelegate:self
        didEndSelector:@selector(warningSheetDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

- (IBAction)hideWarningWindow: (id) sender
{
    [warningWindow orderOut:sender];
    [NSApp endSheet:warningWindow returnCode:1];
}

- (IBAction)agreementAccepted: (id) sender
{
    [warningWindow orderOut:sender];
    [NSApp endSheet:warningWindow returnCode:1];
	[self dismissBanner];
}

- (void)warningSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    [sheet orderOut:nil];
}

- (void)startWarningDismissTimer
{
	[NSTimer
		scheduledTimerWithTimeInterval: kInactivityDelay
								target: self
							  selector: @selector(hideWarningWindow:)
							  userInfo: nil
							   repeats: NO];
}

@end

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation,
 modification or redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject to these
 terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
 this original Apple software (the "Apple Software"), to use, reproduce, modify and
 redistribute the Apple Software, with or without modifications, in source and/or binary
 forms; provided that if you redistribute the Apple Software in its entirety and without
 modifications, you must retain this notice and the following text and disclaimers in all
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

