/*
	File:		AVCBrowser/PlugBrowserController.h

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.

	Written by: wgulland
	
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

	$Log: PlugBrowserController.h,v $
	Revision 1.5  2004/07/12 18:23:08  ayanowit
	Added support for EIA-775 descriptor parsing on supported devices.
	
	Revision 1.4  2003/04/25 20:57:30  firewire
	Added new features to PlugBrowserController
	
	Revision 1.3  2002/11/14 22:22:47  noggin
	fix minor crasher
	
	Revision 1.2  2002/11/08 22:46:48  firewire
	first checkin of install project
	
*/

#import <Cocoa/Cocoa.h>
#import "AVCDevice.h"

@interface PlugBrowserController : NSObject
{
    IBOutlet id 			_inPlugCount;
    IBOutlet id 			_inPlugList;
    NSMutableArray *		_inPlugs;
    IBOutlet id 			_outPlugCount;
    IBOutlet id 			_outPlugList;
    NSMutableArray *		_outPlugs;

	AVCDevice *avcDevice;
	
	IBOutlet NSTextField *subunitInfoPage;
	IBOutlet NSTextField *outputPlugSigFmtPlugNum;
	IBOutlet NSTextField *inputPlugSigFmtPlugNum;
	IBOutlet NSTextField *avcCommandBytes;
	IBOutlet NSPopUpButton *inputPlugChannel;
	IBOutlet NSPopUpButton *outputPlugChannel;
	IBOutlet NSPopUpButton *outputPlugRate;
	IBOutlet NSTextView *avcLog;
	IBOutlet NSWindow *deviceControllerWindow;
	IBOutlet NSButton *eia775Info;
		
	NSMutableString *avcLogString;
	IOFireWireAVCLibUnitInterface ** avcInterface;
}

+(PlugBrowserController *)withDevice:(AVCDevice *)device;

- (IBAction)makeInputPlugConnection:(id)sender;
- (IBAction)breakInputPlugConnection:(id)sender;
- (IBAction)makeOutputPlugConnection:(id)sender;
- (IBAction)breakOutputPlugConnection:(id)sender;
- (IBAction)rereadPlugs:(id)sender;
- (IBAction)UnitInfoCommand:(id)sender;
- (IBAction)PlugInfoCommand:(id)sender;
- (IBAction)SubunitInfoCommand:(id)sender;
- (IBAction)outputPlugSigFmtCommand:(id)sender;
- (IBAction)inputPlugSigFmtCommand:(id)sender;
- (IBAction)sendAVCCommandBytes:(id)sender;
- (IBAction)clearLog:(id)sender;
- (IBAction)getEIA775Info:(id)sender;

- (void) addToAVCLog:(NSString*)string;

@end
