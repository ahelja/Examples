/************************************************************

 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple’s copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.

	
	CocoaSpeechAppleEvents.m
	ExpensePad
	
	Copyright (c) 2000-2005 Apple Computer. All rights reserved.

************************************************************/

#import "CocoaSpeechAppleEvents.h"

static SRRecognitionSystem	sRecSystem;

static void ProcessResult (OSErr origStatus, SRRecognitionResult recResult)
{
	OSErr			status = origStatus;
	Size			len;
	SRLanguageModel	resultLM, subLM;
	long			refCon;

	if (!status && recResult) {
		len = sizeof(resultLM);
		status = SRGetProperty (recResult, kSRLanguageModelFormat, &resultLM, &len);
		if (!status) {
			status = SRGetIndexedItem (resultLM, &subLM, 0);
			if (!status) {
				len = 4;
				status = SRGetProperty (subLM, kSRRefCon, &refCon, &len);
				if (!status) {
					[(Utterance *)refCon perform];
				}
					
				//	release subelement when done with it
				SRReleaseObject (subLM);
			}
			
			//	release resultLM fetched above when done with it
			SRReleaseObject (resultLM);
		}
	}

	if (!origStatus) SRReleaseObject (recResult);
}

pascal OSErr HandleSpeechDoneAppleEvent (const AppleEvent *theAEevt, AppleEvent* reply, long refcon)
{
	long				actualSize;
	DescType			actualType;
	OSErr				status = 0;
	OSErr				recStatus = 0;
	SRRecognitionResult	recResult = 0;
	
		/* Get status */
	status = AEGetParamPtr(theAEevt,keySRSpeechStatus,typeShortInteger,
					&actualType, (Ptr)&recStatus, sizeof(status), &actualSize);

		/* Get result */
	if (!status && !recStatus)
		status = AEGetParamPtr(theAEevt,keySRSpeechResult,typeSRSpeechResult,
					&actualType, (Ptr)&recResult, sizeof(SRRecognitionResult), &actualSize);
					
		/* Process result */
	if (!status)
		status = recStatus;
	ProcessResult (status, recResult);

	return status;
}

@implementation Utterance 

+ (SRRecognitionSystem)getRecognitionSystem
{
    if (!sRecSystem) 
        if (!SROpenRecognitionSystem(&sRecSystem, kSRDefaultRecognitionSystemID)) {
            short myModes = kSRHasFeedbackHasListenModes;
            SRSetProperty(sRecSystem, kSRFeedbackAndListeningModes, &myModes, sizeof(myModes));
            AEInstallEventHandler(kAESpeechSuite, kAESpeechDone, 
                NewAEEventHandlerUPP(HandleSpeechDoneAppleEvent), 0, false);
        } else 
            sRecSystem = NULL;
    
    return sRecSystem;
}

-(Utterance *)initWithTarget:(id)target selector:(SEL)selector withObject:(id)object
{
    _target		= [target retain];
	_selector	= selector;
    _object		= [object retain];
    
    return self;
}

+(Utterance *)utteranceWithTarget:(id)target selector:(SEL)selector withObject:(id)object
{
    return [[[Utterance alloc] 
                initWithTarget:target selector:selector withObject:object] 
                autorelease];
}

- (void) dealloc
{
    [_target release];
    [_object release];
	[super dealloc];
}

-(id)perform
{
    return [_target performSelector:_selector withObject:_object];
}

@end
