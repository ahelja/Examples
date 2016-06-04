/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
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
/* UtilityWindowController.m */

#import "AppController.h"
#import "UtilityWindowController.h"

@implementation UtilityWindowController

-(void)windowDidLoad
{
    [super windowDidLoad];

	NSNotificationCenter *defaultCenter = [NSNotificationCenter defaultCenter];
    [defaultCenter addObserver:self selector:@selector(diskCacheObjectDidChange:) name:DiskCacheObjectChangedNotification object:nil];
    [self handleDiskCacheObjectDidChange:[[AppController sharedAppController] currentDiskCacheObject]];

    [defaultCenter addObserver:self selector:@selector(windowWillClose:)
        name:NSWindowWillCloseNotification object:nil];

    [[self window] setFrameAutosaveName:[self windowNibName]];
}

-(void)reopenWindowIfNeeded
{
    NSString *key = [NSString stringWithFormat:@"Open-%@", [self windowNibName]];
    NSString *reopen = [[NSUserDefaults standardUserDefaults] stringForKey:key];
    if ([reopen isEqualToString:@"YES"]) {
        [self showWindow];
    }
}

-(void)setReopenWindowAtLaunchPreference:(BOOL)reopen
{
    NSString *key = [NSString stringWithFormat:@"Open-%@", [self windowNibName]];
    NSString *value = reopen ? @"YES" : @"NO";
    [[NSUserDefaults standardUserDefaults] setObject:value forKey:key];
}

-(BOOL)showingWindow
{
    return showingWindow;
}

-(void)toggleHideShowWindow
{
    if (showingWindow) {
        [self hideWindow];
        showingWindow = NO;
    }
    else {
        [self showWindow];
        showingWindow = YES;
    }
}

-(void)showWindow
{
    [self showWindow:self];
    [self setReopenWindowAtLaunchPreference:YES];
}

-(void)hideWindow
{
    [[self window] close];
    [self setReopenWindowAtLaunchPreference:NO];
}

- (void)windowWillClose:(NSNotification *)aNotification
{
    if ([aNotification object] == [self window]) {
        [self setReopenWindowAtLaunchPreference:NO];
    }
}

- (void)diskCacheObjectDidChange:(NSNotification *)aNotification
{
    DiskCacheObject *diskCacheObject = [aNotification object];
    [self handleDiskCacheObjectDidChange:diskCacheObject];
}

-(void)handleDiskCacheObjectDidChange:(DiskCacheObject *)diskCacheObject
{
}

@end
