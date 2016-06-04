/*

File: GMMainWindowController.h

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright 2004 Apple Computer, Inc., All Rights Reserved

*/ 

#import <Cocoa/Cocoa.h>

@class GMRectangleDraggerView;
@class GMConnectionController;

@interface GMMainWindowController : NSWindowController
{
	IBOutlet GMRectangleDraggerView *_imageView;
	IBOutlet NSWindow *_connectSheet;
	IBOutlet NSWindow *_authenticationNeededSheet;
	
	XGGrid *_grid;
	
	NSString *_connectName;
	BOOL _isConnecting;
	NSString *_connectMessage;
	int _authenticationMethodTag;
	NSString *_authenticationPassword;

	int _pixelsWide;
	int _pixelsHigh;
	
	int _patchPixelsWide;
	int _patchPixelsHigh;
	
	double _currentImageX;
	double _currentImageY;
	double _currentImageWidth;
	
	double _nextImageX;
	double _nextImageY;
	double _nextImageWidth;
	
	NSString *_toggleAutomaticTourTitle;
	
	BOOL _useAltivec;
	BOOL _canToggleAutomaticTour;
	BOOL _canShowImage;
	BOOL _animate;
	
	double _elapsedSeconds;
	
	NSArray *_tourArray;
	BOOL _doingTour;
	int _currentTourIndex;
	
	NSMutableArray *_settingsArray;
	int _currentSettingsIndex;
	
	NSBitmapImageRep *_imageRep;
	NSImage *_image;
	
	NSMutableSet *_submissionMonitors;
	NSMutableSet *_jobIdentifiers;
	NSMutableSet *_jobs;
}

- (void)connectionDidOpen;
- (void)connectionDidNotOpen;
- (void)connectionWasCanceled;
- (void)connectionDidClose;
- (void)connectionAuthenticationNeeded;

- (NSDictionary *)currentImageSettings;
- (NSDictionary *)nextImageSettings;
- (NSDictionary *)currentTourSettings;

- (void)clearImageRep;
- (void)fadeImageRep;
- (void)refreshImage;
- (void)showInitialImage;
- (void)showImageWithSettings:(NSDictionary *)settings;
- (void)submitJobs;
- (void)addSubmissionMonitor:(XGActionMonitor *)submissionMonitor;
- (void)removeSubmissionMonitor:(XGActionMonitor *)submissionMonitor;
- (void)addJob:(XGJob *)job;
- (void)removeJob:(XGJob *)job;
- (void)removeAllJobs;
- (int)outstandingJobCount;
- (void)performTour;
- (void)writeImageFromData:(NSData *)data;

- (void)showConnectSheet;
- (void)showAuthenticationNeededSheetForConnectionController:(GMConnectionController *)connectionController;
- (void)showConnectionDidCloseAlertForConnectionController:(GMConnectionController *)connectionController;

- (void)connectSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void)authenticationNeededSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void)connectionDidCloseAlertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo;

- (IBAction)toggleAutomaticTour:(id)sender;
- (IBAction)showImage:(id)sender;
- (IBAction)backImage:(id)sender;

- (IBAction)connectSheetOK:(id)sender;
- (IBAction)connectSheetCancel:(id)sender;

- (IBAction)authenticationNeededSheetOK:(id)sender;
- (IBAction)authenticationNeededSheetCancel:(id)sender;

@end
