/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

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
/* MixerController */

#import <Cocoa/Cocoa.h>
#import <CoreAudio/CoreAudio.h>
#import <AudioToolbox/AudioToolbox.h>


typedef struct 
{
	AudioStreamBasicDescription asbd;
	float *data;
	UInt32 numFrames;
	UInt32 phase;
	NSString *name;
} SndBuf;

#define MAXBUFS 8

struct SynthData
{
	int numbufs;
	SndBuf bufs[MAXBUFS];
	int select;
};

@interface MixerController : NSObject
{
	AUGraph mGraph;
	AudioUnit mixer;
	//AudioUnit defaultOutputUnit;
	IBOutlet NSView* meterIn0;
	IBOutlet NSView* meterIn1;
	IBOutlet NSView* meterIn2;
	IBOutlet NSView* meterIn3;
	IBOutlet NSView* meterInPre0;
	IBOutlet NSView* meterInPre1;
	IBOutlet NSView* meterInPre2;
	IBOutlet NSView* meterInPre3;
	IBOutlet NSView* meterOut0;
	IBOutlet NSView* meterOut1;
	IBOutlet NSView* meterOut2;
	IBOutlet NSView* meterOut3;
	IBOutlet NSView* meterOut4;
	IBOutlet NSView* xpmeter11;
	IBOutlet NSView* xpmeter12;
	IBOutlet NSView* xpmeter13;
	IBOutlet NSView* xpmeter14;
	IBOutlet NSView* xpmeter15;
	IBOutlet NSView* xpmeter21;
	IBOutlet NSView* xpmeter22;
	IBOutlet NSView* xpmeter23;
	IBOutlet NSView* xpmeter24;
	IBOutlet NSView* xpmeter25;
	IBOutlet NSView* xpmeter31;
	IBOutlet NSView* xpmeter32;
	IBOutlet NSView* xpmeter33;
	IBOutlet NSView* xpmeter34;
	IBOutlet NSView* xpmeter35;
	IBOutlet NSView* xpmeter41;
	IBOutlet NSView* xpmeter42;
	IBOutlet NSView* xpmeter43;
	IBOutlet NSView* xpmeter44;
	IBOutlet NSView* xpmeter45;
	
	IBOutlet NSSlider* xpslider11;
	IBOutlet NSSlider* xpslider12;
	IBOutlet NSSlider* xpslider13;
	IBOutlet NSSlider* xpslider14;
	IBOutlet NSSlider* xpslider15;
	IBOutlet NSSlider* xpslider21;
	IBOutlet NSSlider* xpslider22;
	IBOutlet NSSlider* xpslider23;
	IBOutlet NSSlider* xpslider24;
	IBOutlet NSSlider* xpslider25;
	IBOutlet NSSlider* xpslider31;
	IBOutlet NSSlider* xpslider32;
	IBOutlet NSSlider* xpslider33;
	IBOutlet NSSlider* xpslider34;
	IBOutlet NSSlider* xpslider35;
	IBOutlet NSSlider* xpslider41;
	IBOutlet NSSlider* xpslider42;
	IBOutlet NSSlider* xpslider43;
	IBOutlet NSSlider* xpslider44;
	IBOutlet NSSlider* xpslider45;
	
	NSTimer* mTimer;
	SynthData d;
	Boolean isPlaying;
	Boolean automate;
	int automatePhase;
}
- (void)awakeFromNib;
- (void)initializeGraph;
- (IBAction)play:(id)sender;
- (IBAction)setInputVolume:(id)sender;
- (IBAction)setMasterVolume:(id)sender;
- (IBAction)setMatrixVolume:(id)sender;
- (IBAction)setOutputVolume:(id)sender;
- (IBAction)stop:(id)sender;
- (IBAction)enableInput:(id)sender;
- (IBAction)enableOutput:(id)sender;
- (void)doTimer: (NSTimer*) timer;
- (IBAction)addFile:(id)sender;
- (IBAction)automateOn:(id)sender;
- (IBAction)automateOff:(id)sender;

@end
