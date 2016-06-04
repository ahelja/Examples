/*
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


	RejectableListener.m
	NewRejectableModel
	
	2001-2005 (c) 2001-2005 Apple Computer. All rights reserved.


	The rejectable listener is a class that demonstrates the power of using the rejectable
	property of a language model object to guide a correction dialog.  It makes a speech interaction
	more fluid by informing the user of the language model object not understood, and allows the user
	to then supply the correct value.  Infinitely more intuitive than simply replying "huh?"
	
	This class when it initializes, sets up the recogniion system, bulids a language model then starts
	listening.  When it hears something it displays the recognition result in the window.  Changing the 
	value of the rejectable radio buttons resets this property on both models.
*/

#import <Carbon/Carbon.h>
#import <Foundation/Foundation.h>

#import "RejectableController.h"
#import "RejectableListener.h"

//  Prototypes
pascal OSErr HandleSpeechDoneAppleEvent ( const AppleEvent *theAEevt, AppleEvent* reply, long refcon);
OSErr	SRAddTextFromCFString(SRLanguageObject base , CFStringRef theText , long refCon);
OSErr	SRNewPhraseFromCFString(SRRecognitionSystem inSystem , SRPhrase * outPhrase , CFStringRef theText);
Boolean AddCommands( SRRecognizer inSpeechRecognizer, CFArrayRef inCommandNamesArray );

//  Constants
const long 		kNamesRefCon		=	'name';
const long 		kDatesRefCon		=	'date';
const long		kTopLMRefcon		= 	'top ';
const long 		kRejectedWordRefCon = 	'rejc';

//  Classes
@implementation RejectableListener

//  ------------------------------------------------------------------------------
//  setController - sets the instance variable to the controller object for the view
- (void)setController: (id)theController
{
	_myController = theController;
}

//  ------------------------------------------------------------------------------
//  setRejectable - set the boolean value for the rejectable state on our language
//  model.  Note, we will use this value for the rejectable property on both the 
//  name and the day of the week language objects.
- (OSErr)setRejectable: (BOOL)theState
{
	_rejectableState = theState;
	//  if we have any models we set the rejection property on those models to whatever the new state is
	return [self setRejectionStateOnModels];
}

//  ------------------------------------------------------------------------------
//  initSR - initialize the speech recognition objects
- (OSErr)initSR
{
	Boolean		blockModallyFlg, foregroundOnlyFlg = TRUE;
	SInt32		notifFlags = kSRNotifyRecognitionDone;
	Size 		myLen;
	SRWord 		myRejectedWord = 0;
	char		topLM[] = "<topLM>", nameLM[] = "<name>", dayLM[] = "<dayName>";
	OSErr		theErr = noErr;
	[self getLMPlist];	//  this has our language model data.  Better have it before proceeding.
	if (_theLMPlist != NULL) {
		//  install our speech apple event handler (the refcon will point back to this object)
		if (AEInstallEventHandler( kAESpeechSuite, kAESpeechDone, NewAEEventHandlerUPP(HandleSpeechDoneAppleEvent), (SInt32)self, false) == noErr) {
		//  open a recognition system
		if (SROpenRecognitionSystem( &_theRecognitionSystem, kSRDefaultRecognitionSystemID ) == noErr) {
			// Use the Speech Feedback window
			Byte	useFeedback = kSRHasFeedbackHasListenModes;
			if (SRSetProperty( _theRecognitionSystem, kSRFeedbackAndListeningModes, &useFeedback, sizeof(useFeedback) ) == noErr) {
				//  create a recognizer
				if (SRNewRecognizer( _theRecognitionSystem, &_theRecognizer, kSRDefaultSpeechSource ) == noErr) {
					//  lets block all other recognizers if there are any and listen only in the foreground
					SRSetProperty(_theRecognizer , kSRBlockModally , &blockModallyFlg , sizeof(Boolean));
					SRSetProperty (_theRecognizer, kSRForegroundOnly, &foregroundOnlyFlg, sizeof(foregroundOnlyFlg));
                                        //  we only want to be notified of speech done events
					SRSetProperty (_theRecognizer, kSRNotificationParam, &notifFlags, sizeof(notifFlags));
					//  Here we want to set the refcon for the rejection word
					myLen = sizeof(myRejectedWord);
					// Get a reference to the rejected word.
					theErr = SRGetProperty(_theRecognitionSystem, kSRRejectedWord, &myRejectedWord, &myLen);
					// Set the refcon of the rejected word, so we can use it later when processing the search result.
					if (!theErr)
						theErr = SRSetProperty(myRejectedWord, kSRRefCon, &kRejectedWordRefCon, sizeof(kRejectedWordRefCon));
					if (myRejectedWord)
						SRReleaseObject(myRejectedWord); 
					
                                        //  create 3 empty language models for later
					
					if ((SRNewLanguageModel( _theRecognitionSystem, &_topLanguageModel, topLM, sizeof(topLM)) == noErr)
						&& (SRNewLanguageModel( _theRecognitionSystem, &_namesModel, nameLM, sizeof(nameLM)) == noErr)
						&& (SRNewLanguageModel( _theRecognitionSystem, &_dayModel, dayLM, sizeof(dayLM)) == noErr)) {
						
						//  set the refcon's for the various models
						SRSetProperty(_topLanguageModel, kSRRefCon , &kTopLMRefcon , sizeof(kTopLMRefcon));
						SRSetProperty(_namesModel, kSRRefCon , &kNamesRefCon , sizeof(kNamesRefCon));
						SRSetProperty(_dayModel, kSRRefCon , &kDatesRefCon , sizeof(kDatesRefCon));						
						
						//  fill in the language models
						if ([self buildLanguageModels] == noErr) 
							theErr = [self startListening];														
						}
					}
				}
			}
		}
	}
	return theErr;
}

//  ------------------------------------------------------------------------------
//
- (OSErr)startListening
{
	OSErr	theErr = SRStartListening(_theRecognizer);
	return theErr;
}

//  ------------------------------------------------------------------------------
//	getLMPlist  - fetch our plist containing the data for our language models
//
- (void)getLMPlist
{
	_bundle = CFBundleGetMainBundle();
	if (_bundle != NULL) {
		
		CFURLRef	theLMURL = CFBundleCopyResourceURL ( _bundle, CFSTR( "LMPropertyList.plist" ), NULL, NULL );
	
		if (theLMURL) {
			
			CFDataRef	theLMCFDataRef = NULL;
			SInt32 		theURLAccessErrorCode;
							
			if( CFURLCreateDataAndPropertiesFromResource( NULL, theLMURL, &theLMCFDataRef, NULL, NULL, &theURLAccessErrorCode )) {
				CFStringRef		errorString;
				if (theLMCFDataRef) _theLMPlist = CFPropertyListCreateFromXMLData( NULL, theLMCFDataRef, kCFPropertyListImmutable, &errorString );
				CFRelease( theLMCFDataRef );
				CFRelease(theLMURL);	
			}
		}
	}
}

//  ------------------------------------------------------------------------------
//	setRejectionStateOnModels - sets the rejectable property of the models
- (OSErr)setRejectionStateOnModels
{
	OSErr	theErr = noErr;
	
	if (_namesModel != NULL) theErr = SRSetProperty( _namesModel , kSRRejectable , &_rejectableState , sizeof( _rejectableState ));
	if (!theErr && (_dayModel != NULL)) theErr = SRSetProperty( _dayModel , kSRRejectable , &_rejectableState , sizeof( _rejectableState ));
	
	return theErr;
}

//  ------------------------------------------------------------------------------
//  buildLanguageModels - build up our language models
//  
- (OSErr)buildLanguageModels
{
	OSErr		theErr = noErr;
	CFArrayRef	helpArray;
	theErr = [self buildNamesLM];
	if (!theErr) theErr = [self buildDateLM];
	if (!theErr) theErr = [self setRejectionStateOnModels];
	if (!theErr) theErr = [self associateLMs];
        if (!theErr) {
            //  get the static item from our plist
            helpArray = CFArrayGetValueAtIndex(_theLMPlist , 2);
            //  create an array with this one item and update the help list
            AddCommands( _theRecognizer, helpArray );
            CFRelease(helpArray);
        }
	return theErr;
}

//  ------------------------------------------------------------------------------
//  buildNamesLM - using the LMPropertyList create a language model for names.  The LMPlist
//  is just an array of 3 string arrays.  The first array contains people's names the second
//  contains the days of the week (except sunday! which is the intended omission).  The third is
//  simply an array of 1 element - which is the text to be shown in the speech help window.  In order
//  to really do the right thing, I should show the top languge model and the embedded
//  language models. I use a static string to simplify the code.
- (OSErr)buildNamesLM
{
	//  these names are the strings in the array associated with the peopleNames dictionary
	//  which is the first element of the property list
	OSErr		theErr = noErr;
	CFArrayRef	namesArray = CFArrayGetValueAtIndex(_theLMPlist , 0);
	CFStringRef theName;
	CFIndex		numberNames = CFArrayGetCount( namesArray );
	int			i;
	for ( i = 0 ; i < numberNames; i++ ) {
		theName= CFArrayGetValueAtIndex(namesArray , i);
		theErr = SRAddTextFromCFString(_namesModel , theName , 0);
		if (theErr) return theErr;
	}
	CFRelease(namesArray);
	return theErr;
}

//  ------------------------------------------------------------------------------
// 	buildDateLM  - like the above
- (OSErr)buildDateLM
{
	//  these day names are the strings in the array associated with the second
	//  element of the property list
	OSErr		theErr = noErr;
	CFArrayRef	dayNameArray = CFArrayGetValueAtIndex(_theLMPlist , 1);
	CFStringRef theDayName;
	CFIndex		numberDayNames = CFArrayGetCount( dayNameArray );
	int			i;
	for ( i = 0 ; i < numberDayNames; i++ ) {
		theDayName = CFArrayGetValueAtIndex(dayNameArray , i);
		theErr = SRAddTextFromCFString(_dayModel , theDayName , 0);
		if (theErr) return theErr;
	}
	CFRelease(dayNameArray);
	return theErr;
}

//  ------------------------------------------------------------------------------
//  associateLMs  -  add the language models to one another building up the top model 
//  and associate them with the recognizer.
- (OSErr)associateLMs
{
	OSErr	theErr = noErr;
	SRPath	path = 0;
	Str255	meetWithPhrase	= "\pmeet with ";
	Str255	onPhrase	= "\pon";
	
	//  create a path to collect up all this stuff to add to the gDefaultLM
	theErr = SRNewPath(_theRecognitionSystem , &path);
	//  Now build up the path
	if (!theErr) theErr = SRAddText(path , &meetWithPhrase[1] , meetWithPhrase[0] , 0);	//"meet with …"
	if (!theErr) theErr = SRAddLanguageObject(path , _namesModel);				// "meet with <person>…
	if (!theErr) theErr = SRAddText(path , &onPhrase[1] , onPhrase[0] , 0);		//"meet with <person> on…"
	if (!theErr) theErr = SRAddLanguageObject(path , _dayModel);				// "meet with <person> on <date>"
		//  add it to the top LM
	if (!theErr) theErr = SRAddLanguageObject(_topLanguageModel , path);	// "meet with <person> on <date>"	
		//  associate the top language model with the recognizer
	if (!theErr) theErr = SRSetLanguageModel( _theRecognizer, _topLanguageModel );
	
	SRReleaseObject( path );	
	return theErr;
}

//  ------------------------------------------------------------------------------
//  handleSpeechDoneEvent
- (OSErr)handleSpeechDoneEvent: (const AppleEvent *)theAEevt
{
	long			actualSize = 0;
	DescType		actualType;
	OSErr			theErr = 0, recStatus = 0;
	SRRecognitionResult	recResult = 0;
	SRRecognizer		theRecognizer = 0;
		//get status
	theErr = AEGetParamPtr(theAEevt, keySRSpeechStatus, typeShortInteger, &actualType, (Ptr)&recStatus, sizeof(theErr), &actualSize);
		//  get the recognizer
	if (!theErr && !recStatus)
		theErr = AEGetParamPtr(theAEevt, keySRRecognizer, typeSRRecognizer, &actualType, (Ptr)&theRecognizer, sizeof(theRecognizer), &actualSize);
	
	//get result
	if (!theErr && !recStatus) 
		theErr = AEGetParamPtr(theAEevt, keySRSpeechResult, typeSRSpeechResult, &actualType, &recResult, sizeof(SRRecognitionResult), &actualSize);
	if (!theErr  && !recStatus) theErr = [self processRecognitionResult:recResult];

    return theErr;
}

//  ------------------------------------------------------------------------------
//	
- (OSErr)processRecognitionResult: (SRRecognitionResult) recResult
{
	OSErr				theErr = noErr;
	SInt32				len = 0;
	SRLanguageModel		resultLM = 0;
	SRLanguageObject	theItem , theDateItem , theNameItem;
	long				refCon = 0, itemCount = 0;
	Str255				theRecognizedText = "\p";
	Boolean				noRecognizedName = false , noRecognizedDate = false;
	
	if (recResult) {
		len = sizeof(SRLanguageModel);
		theErr = SRGetProperty(recResult, kSRLanguageModelFormat, &resultLM, &len);
		if (!theErr) {
			len = sizeof( theRecognizedText );
			SRGetProperty( recResult , kSRTEXTFormat , &theRecognizedText , &len);
			[_myController displayRecognitionResult:(char *)theRecognizedText];
			
			len = sizeof( refCon );
			theErr = SRGetProperty( resultLM , kSRRefCon , &refCon , &len);
			if (!theErr) {
				if (refCon == kTopLMRefcon) {
					SRCountItems(resultLM , &itemCount);
					if (itemCount == 1) {
					//  we got this far so we've got our utterance,  now we want to see if
					//  the embedded language models are 0.  The first item is a path
						SRGetIndexedItem(resultLM, &theItem, 0);
						//  get the first item in the path which sould be the name
						SRCountItems(theItem , &itemCount);  //  should be 4 items
						if (itemCount == 4) {
							SRGetIndexedItem( theItem , &theNameItem , 1);
							SRGetProperty( theNameItem , kSRRefCon , &refCon , &len);
							//  if the refcon here is kRejectedWordRefCon then we can set the noRecognizedName flag since it indicates that 
							//  it was rejected.  Should have been 'name'
							if (refCon == kRejectedWordRefCon) noRecognizedName = TRUE;
							//  Get the last item which should be the date
							SRGetIndexedItem( theItem , &theDateItem , 3);
							SRGetProperty( theDateItem , kSRRefCon , &refCon , &len);
							//  if the refcon here is kRejectedWordRefCon then we can set the kNoRecognizedDate flag since if it were recognized
							//  it should have been 'date'
							if (refCon == kRejectedWordRefCon) noRecognizedDate = TRUE;
						
							//  now, we have to decide what to do if we don't recognize something - ideally, we would save our
							//  current state and try to recover using some kind of error correcting dialog.
								
							if ((noRecognizedDate == TRUE) && (noRecognizedName == TRUE))
								[self whatAreYouTalkingAboutDialog];
							else if (noRecognizedDate == TRUE)
								[self clarifyDateDialog];
							else if (noRecognizedName == TRUE)
								[self clarifyNameDialog];
							}
						}
					}
					//  the top level language model is ??? which is bad - didn't even come close to anything
					else if (refCon == kRejectedWordRefCon)
							[self beAdummy];
					}
				}
		SRReleaseObject( theItem );
		SRReleaseObject( theNameItem );
		SRReleaseObject( theDateItem );
		SRReleaseObject (resultLM);
	}
    return theErr;
}

//  ------------------------------------------------------------------------------
//	clarifyNameDialog - here you would do something special to figure out what the
//  name sublanguage model is.  Perhaps you would create a whole new recognizer with
//  a specific name recognition language model and block others.  For now we just 
//  speak something to the user
-(void)clarifyNameDialog
{
    char	thePrompt[] = "Did not get the name.";
    SRSpeakAndDrawText(_theRecognizer , &thePrompt, sizeof(thePrompt));

}

//  ------------------------------------------------------------------------------
//	clarifyDateDialog - like the above but we are trying to fix the date
-(void)clarifyDateDialog
{
    char	thePrompt[] = "Did not get the date.";
    SRSpeakAndDrawText(_theRecognizer , &thePrompt, sizeof(thePrompt));
}

//  ------------------------------------------------------------------------------
//	whatAreYouTalkingAboutDialog - we're really confused here.
-(void)whatAreYouTalkingAboutDialog
{
    char	thePrompt[] = "I really didn't get much of what you said.";
    SRSpeakAndDrawText(_theRecognizer , &thePrompt, sizeof(thePrompt));
}

//  ------------------------------------------------------------------------------
//	beAdummy - a really dumb reply which is spoken when the refCon of the top level
//	language model is the rejected word.
-(void)beAdummy
{
    char	thePrompt[] = "Huh?";
    SRSpeakAndDrawText(_theRecognizer , &thePrompt, sizeof(thePrompt));

}
@end


//  ------------------------------------------------------------------------------
// 		Functions

//  ------------------------------------------------------------------------------
//  Gets sent to the application by the SR Server.  Dispatch to the appropriate method
pascal OSErr HandleSpeechDoneAppleEvent ( const AppleEvent *theAEevt, AppleEvent* reply, long refcon)
{
	[(RejectableListener *)refcon handleSpeechDoneEvent: theAEevt];
	return noErr;
}

//  ------------------------------------------------------------------------------
// 	SRAddTextFromCFString  -  a utility routine for adding text as a CFString to a language model object
OSErr	SRAddTextFromCFString(SRLanguageObject base , CFStringRef theText , long refCon)
{
	//  like SRAddText but for a CFStringRef
    OSErr		theErr			= noErr;
	CFIndex		theStringLen 	= 0;
	char * 		theBuffer 		= NULL;
	
    if (theText) {
        theStringLen = CFStringGetLength(theText);
        theBuffer = (char*)CFStringGetCStringPtr(theText , kCFStringEncodingMacRoman);
    }
	
	//  really should check for NULL, naaaa.
    if (theBuffer)
        theErr = SRAddText(base , theBuffer , (Size)theStringLen , refCon);
	else
        theErr = -1;
        
	return theErr;
}

//  ------------------------------------------------------------------------------
// 	SRNewPhraseFromCFString  -  given some text as a CFString, return a phrase with that text
OSErr	SRNewPhraseFromCFString(SRRecognitionSystem inSystem , SRPhrase * outPhrase , CFStringRef theText)
{
	OSErr		status = noErr;
	CFIndex		theStringLen = 0;
	char * 		theBuffer = NULL;
	SRPhrase	thePhrase = NULL;
	
	theStringLen = CFStringGetLength(theText);
	theBuffer = (char*)CFStringGetCStringPtr(theText , kCFStringEncodingMacRoman);
	
	status = SRNewPhrase(inSystem , &thePhrase , theBuffer , theStringLen);
	*outPhrase = thePhrase;
	return status;
}

//  ------------------------------------------------------------------------------
//	AddCommands - This version just adds the commands to the speakable commands list and does not 
//	add or change the language model.
Boolean AddCommands( SRRecognizer inSpeechRecognizer, CFArrayRef inCommandNamesArray )
{

    // Dictionary keys for Commands Data property list
    #define	kCommandsDataPlacementHintKey	"PlacementHint"		// value type is: CFNumberRef
    #define	kPlacementHintWhereverNumValue	0
    #define	kPlacementHintProcessNumValue	1			// you must also provide the PSN using kCommandsDataProcessPSNHighKey & kCommandsDataProcessPSNLowKey
    #define	kPlacementHintFirstNumValue	2
    #define	kPlacementHintMiddleNumValue	3
    #define	kPlacementHintLastNumValue	4
    #define	kCommandsDataProcessPSNHighKey	"ProcessPSNHigh"	// value type is: CFNumberRef
    #define	kCommandsDataProcessPSNLowKey	"ProcessPSNLow"		// value type is: CFNumberRef
    #define	kCommandsDataDisplayOrderKey	"DisplayOrder"		// value type is: CFNumberRef  - the order in which recognizers from the same client process should be displayed.
    #define	kCommandsDataCmdInfoArrayKey	"CommandInfoArray"	// value type is: CFArrayRef of CFDictionaryRef values
    #define	kCommandInfoNameKey		"Text"
    #define	kCommandInfoChildrenKey		"Children"

    Boolean	successfullyAddCommands = false;
    
    if (inCommandNamesArray) {
    
        CFIndex	numOfCommands = CFArrayGetCount( inCommandNamesArray );
        CFMutableArrayRef	theSectionArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
        
        // Erase any existing commands in the language model before we begin adding the given list of commands
        if (theSectionArray) {
        
            CFIndex	i;
            char 	theCSringBuffer[100];

            successfullyAddCommands = true;
            for (i = 0; i < numOfCommands; i++) {
               
                 CFStringRef	theCommandName = CFArrayGetValueAtIndex(inCommandNamesArray, i);
                if (theCommandName && CFStringGetCString( theCommandName, theCSringBuffer, 100, kCFStringEncodingMacRoman )) {
                	
                        CFStringRef 	keys[1];
                        CFTypeRef		values[1];
                        CFDictionaryRef theItemDict;
                        
                        keys[0] 	= CFSTR( kCommandInfoNameKey );
                        values[0] 	= theCommandName;
                        
                        // Make CDDictionary our keys and values.
                        theItemDict = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                        
                        // Add to command array
                        if (theItemDict) {
                            CFArrayAppendValue( theSectionArray, theItemDict );
                            CFRelease( theItemDict );  // We release our hold on the dictionary object and let the array own it now.
                        }
                        else
                            successfullyAddCommands = false;
                    }
                    else
                        successfullyAddCommands = false;
                    
                    if (! successfullyAddCommands)
                        break;
                }
            
    
            //
            // Create the XML data that contains the commands to display and give this data to the
            // recognizer object to display in the Speech Commands window.
            //
            {
                CFIndex			numOfParams = 4;
                CFStringRef 		keys[numOfParams];
                CFTypeRef		values[numOfParams];
                CFDictionaryRef 	theItemDict;
                SInt32			placementHint = 1;	// 1 = show only when this app is front process
                ProcessSerialNumber	thisAppsPSN;
                
                thisAppsPSN.highLongOfPSN = 0;
                thisAppsPSN.lowLongOfPSN = 0;
        
                GetCurrentProcess( &thisAppsPSN );
            
                keys[0] 	= CFSTR( kCommandsDataPlacementHintKey );
                keys[1] 	= CFSTR( kCommandsDataCmdInfoArrayKey );
                keys[2] 	= CFSTR( kCommandsDataProcessPSNHighKey );
                keys[3] 	= CFSTR( kCommandsDataProcessPSNLowKey );
                values[0] 	= CFNumberCreate(NULL, kCFNumberSInt32Type, &placementHint);
                values[1] 	= theSectionArray;
                values[2] 	= CFNumberCreate(NULL, kCFNumberSInt32Type, &(thisAppsPSN.highLongOfPSN));
                values[3] 	= CFNumberCreate(NULL, kCFNumberSInt32Type, &(thisAppsPSN.lowLongOfPSN));
                    
                // Make CDDictionary of our keys and values.
                theItemDict = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, numOfParams, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                if (theItemDict) {
                
                    // Convert CFDictionary object to XML representation
                    CFDataRef dictData = CFPropertyListCreateXMLData(NULL, theItemDict);
                    if (dictData) {

                    	// Set command list in our SRRecognizer object, causing the Speech Commands window to update.
                        (void)SRSetProperty (inSpeechRecognizer, 'cdpl', CFDataGetBytePtr(dictData), CFDataGetLength(dictData));
                        CFRelease( dictData );
                    }
                }
                
                // Release our hold on the dictionary object and let the array own it now.
                if (theItemDict)
                    CFRelease( theItemDict );
                
                // Release our values
                if (values[0])
                    CFRelease( values[0] );
                
                if (values[2])
                    CFRelease( values[2] );
                    
                if (values[3])
                    CFRelease( values[3] );
                    
                CFRelease( theSectionArray );
            }
        }

    }
    
    return successfullyAddCommands;
}

