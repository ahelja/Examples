/*
	File:		LUNController.h

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "../Common/FWDebugging.h"

#import <Cocoa/Cocoa.h>
#import <IOKit/IOKitLib.h> 
#import <IOKit/IOCFPlugIn.h>
#import <sys/time.h>
#import "../Common/SBP2SampleDriverInterface.h"

@interface LUNController : NSObject 
{
	io_object_t 					fLUNReference;
	IBOutlet id						fWindow;
	IBOutlet id						fLoginButton;
	IBOutlet id						fLogoutButton;
	IBOutlet id						fReadButton;
	IBOutlet id						fWriteButton;
	IBOutlet id						fWRCmpButton;
	IBOutlet id						fCancelButton;
	IBOutlet id						fTextView;

	IBOutlet id 					fReadStartBlocks;
	IBOutlet id 					fReadEndBlocks;
	IBOutlet id 					fReadLoop;
	
	IBOutlet id 					fWriteStartBlocks;
	IBOutlet id 					fWriteEndBlocks;
	IBOutlet id 					fWriteLoop;

	IBOutlet id 					fWRCmpStartBlocks;  // Write/Read compare blocks
	IBOutlet id 					fWRCmpEndBlocks;
	IBOutlet id 					fWRCLoop;

	IBOutlet id 					fReadProgressIndicator;
	IBOutlet id 					fWriteProgressIndicator;
	IBOutlet id 					fWRCmpProgressIndicator;
	NSString *						fName;
	Boolean							fInstantiated;
	Boolean							fCancel;
	
	Boolean							fLoopTheRead;
	Boolean							fLoopTheWrite;
	Boolean							fLoopTheWRC;
	
	IOCFPlugInInterface 	**		fCFPlugInInterface;
	SBP2SampleDriverInterface **	fDriverInterface;
	NSLock *						fGlobalLock;
	NSConditionLock *				fReadWorkingThreadCountLock;
	NSConditionLock *				fWriteWorkingThreadCountLock;
	NSConditionLock *				fWRCmpWorkingThreadCountLock;
}

- (id)initWithLUNReference:(io_object_t)lun;
- (void)dealloc;

- (io_object_t)getLUNReference;
- (NSString*)getName;

- (void)instantiateLUNConnection;
- (void)windowWillClose:(NSNotification *)aNotification;

- (void)loginToDevice:(id)sender;
- (void)logoutOfDevice:(id)sender;
- (void)performRead:(id)sender;
- (void)performWrite:(id)sender;
- (void)performWRCmp:(id)sender;
- (void)cancel:(id)sender;

- (void)runReadTransaction:(id)object;
- (void)runReadWorker:(id)object;
- (void)readLoop:(id)sender;

- (void)runWriteTransaction:(id)object;
- (void)runWriteWorker:(id)object;
- (void)writeLoop:(id)sender;

- (void)runWRCmpTransaction:(id)object;
- (void)runWRCmpWorker:(id)object;
- (void)wrcLoop:(id)sender;

- (void)logString:(NSString *)string;
- (void)fillBuf:(UInt32)size:(UInt32)stuff:(UInt8*)buf;
- (float)elapsed:(struct timeval*)t1:(struct timeval*)t2;

@end
