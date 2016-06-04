/*
	Copyright: 	© Copyright 2004 Apple Computer, Inc. All rights reserved.

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

#include <CoreServices/CoreServices.h>

#import "diagnosticWindowController.h"

@implementation diagnosticWindowController

	IBOutlet id urlField;
    IBOutlet id statusField;

- (IBAction)diagnoseFromStreams:(id)sender {
	CFURLRef url;
	CFHTTPMessageRef message;
	CFReadStreamRef readStream;
	CFNetDiagnosticRef myDiagnostics;
	CFNetDiagnosticStatus myStatus;
	
	//First get the CFURLRef
	url = CFURLCreateWithString(kCFAllocatorDefault, (CFStringRef)[urlField stringValue], NULL);
	
	if(url) {
		//Now create the stream
		message = CFHTTPMessageCreateRequest(kCFAllocatorDefault, (CFStringRef)@"GET", url, kCFHTTPVersion1_1);
		CFRelease(url);
		
		if(message) {
			//Create the steam
			readStream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, message);
			CFRelease(message);
			
			if(readStream) {
				//Now create the CFNetDiagnosticRef then release url since we are done with it
				myDiagnostics = CFNetDiagnosticCreateWithStreams(kCFAllocatorDefault, readStream, NULL);
				CFRelease(message);
		
				if(myDiagnostics) {
					//Call the interactive diagnose call
					
					//Normally you would not call this until a readStream failure had occured
					//It is called here to demonstrate the calling behaviour, since the stream
					//has not been initiated the CFNetDiagnosticRef only has limited info about
					//the stream
					myStatus = CFNetDiagnosticDiagnoseProblemInteractively(myDiagnostics);
					CFRelease(myDiagnostics);
				}
			}
		}
	}
}

- (IBAction)diagnoseFromURL:(id)sender {
	CFURLRef url;
	CFNetDiagnosticRef myDiagnostics;
	CFNetDiagnosticStatus myStatus;
	
	//First get the CFURLRef
	url = CFURLCreateWithString(kCFAllocatorDefault, (CFStringRef)[urlField stringValue], NULL);
	
	if(url) {
		//Now create the CFNetDiagnosticRef then release url since we are done with it
		myDiagnostics = CFNetDiagnosticCreateWithURL(kCFAllocatorDefault, url);
		CFRelease(url);
		
		if(myDiagnostics) {
			//Call the interactive diagnose call
			myStatus = CFNetDiagnosticDiagnoseProblemInteractively(myDiagnostics);
			CFRelease(myDiagnostics);
		}
	}
}


- (IBAction)statusFromStreams:(id)sender {
	CFURLRef url;
	CFHTTPMessageRef message;
	CFReadStreamRef readStream;
	CFNetDiagnosticRef myDiagnostics;
	CFNetDiagnosticStatus myStatus;
	CFStringRef errorString;
	
	//First get the CFURLRef
	url = CFURLCreateWithString(kCFAllocatorDefault, (CFStringRef)[urlField stringValue], NULL);
	
	if(url) {
		//Now create the stream
		message = CFHTTPMessageCreateRequest(kCFAllocatorDefault, (CFStringRef)@"GET", url, kCFHTTPVersion1_1);
		CFRelease(url);
		
		if(message) {
			//Create the steam
			readStream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, message);
			CFRelease(message);
			
			if(readStream) {
				//Now create the CFNetDiagnosticRef then release url since we are done with it
				myDiagnostics = CFNetDiagnosticCreateWithStreams(kCFAllocatorDefault, readStream, NULL);
				CFRelease(message);
		
				if(myDiagnostics) {
					//Get the status and description
					myStatus = CFNetDiagnosticCopyNetworkStatusPassively(myDiagnostics, &errorString);
	
					//Display the error string
					[statusField setObjectValue:(NSString *)errorString];
	
					//Set the status light color
					[self setStatusGraphic:myStatus];
			
					//Release the errorString and myDiagnostics
					CFRelease(errorString);
					CFRelease(myDiagnostics);
				}
			}
		}
	}
}

//This method demonstrates how you get status from a CFURLRef
- (IBAction)statusFromURL:(id)sender {
	CFURLRef url;
	CFNetDiagnosticRef myDiagnostics;
	CFNetDiagnosticStatus myStatus;
	CFStringRef errorString;
	
	//First get the CFURLRef
	url = CFURLCreateWithString(kCFAllocatorDefault, (CFStringRef)[urlField stringValue], NULL);
	
	if(url) {
		//Now create the CFNetDiagnosticRef then release url since we are done with it
		myDiagnostics = CFNetDiagnosticCreateWithURL(kCFAllocatorDefault, url);
		CFRelease(url);
		
		if(myDiagnostics) {
			//Get the status and description
			myStatus = CFNetDiagnosticCopyNetworkStatusPassively(myDiagnostics, &errorString);
	
			//Display the error string
			[statusField setObjectValue:(NSString *)errorString];
	
			//Set the status light color
			[self setStatusGraphic:myStatus];
			
			//Release the errorString and myDiagnostics
			CFRelease(errorString);
			CFRelease(myDiagnostics);
		}
	}
}

//Lets add a little flourish. We can use the status code returned from
//CFNetDiagnosticCopyNetworkStatusPassively() to set a status light,
//just like the network pref pane.

- (void) setStatusGraphic:(CFNetDiagnosticStatus)status {
	NSImage *imageFromBundle = NULL;
	
	switch (status) {
		case kCFNetDiagnosticConnectionDown:
			imageFromBundle = [NSImage imageNamed:@"red.tiff"];
			break;
		case kCFNetDiagnosticConnectionIndeterminate:
			imageFromBundle = [NSImage imageNamed:@"yellow.tiff"];
			break;
		case kCFNetDiagnosticConnectionUp:
			imageFromBundle = [NSImage imageNamed:@"green.tiff"];
			break;
		default:
			//NOT REACHED
			NSLog((NSString *)"@This should no be reached\n");
	}
	[statusLight setImage: imageFromBundle];
}

@end