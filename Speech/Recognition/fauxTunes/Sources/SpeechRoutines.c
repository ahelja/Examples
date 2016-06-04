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


	SpeechRoutines.c
	
	Copyright (c) 2001-2005 Apple Computer, Inc. All rights reserved.
*/


#include <Carbon/Carbon.h>


/*
    SetupSpeechRecognition
    
    This routine performs basic setup of the recognition system, recognizer, and top-level language model
    that nearly all clients of Speech Recognition must do before beginning to listen and handle commands.
    
    This routine returns true if the setup was successful, and passes back through parameters a reference 
    to the recognition system, recognizer, and top-level (commands) language model objects.
*/

Boolean SetupSpeechRecognition( SRRecognitionSystem	 * outSpeechRecognitionSystem, SRRecognizer * outSpeechRecognizer, SRLanguageModel * outCommandsLangaugeModel )
{

    Boolean	wasSuccessfullySetup = false;

    // Open the global SRRecognitionSystem
    if (SROpenRecognitionSystem( outSpeechRecognitionSystem, kSRDefaultRecognitionSystemID ) == noErr) {

        // Use the Speech Feedback window
        Byte useFeedback = kSRHasFeedbackHasListenModes; 
        if (SRSetProperty( *outSpeechRecognitionSystem, kSRFeedbackAndListeningModes, &useFeedback, sizeof(useFeedback) ) == noErr) {
        
            // Create a recognizer with default speech source -- e.g. the desktop microphone 
            if (SRNewRecognizer( *outSpeechRecognitionSystem, outSpeechRecognizer, kSRDefaultSpeechSource ) == noErr) {
    
                // Create a language model object to hold our list of command names
                if (SRNewLanguageModel( *outSpeechRecognitionSystem, outCommandsLangaugeModel, "", 0) == noErr) {
                
                    // Tell the recognizer to use this language model
                    if (SRSetLanguageModel( *outSpeechRecognizer, *outCommandsLangaugeModel ) == noErr)
                        wasSuccessfullySetup = true;
                }
            }
        }
    }
    
    return wasSuccessfullySetup;
}


/*
    AddCommands
    
    This routine adds the commands names specified in an array of strings to the given language 
    model.  Any existing commands in the language model are erased before adding the new commands.  Much
    of the work of this routine is converting the command names into the proper format to give to the
    recognizer object to be displayed in the Speech Commands window.
    
    If you have a single, static list of commands then call this routine once after calling 
    SetupSpeechRecognition. If instead you wish to have the commands change depending on what the user is
    doing, then call this routine as often as necessary with a list of only the commands that are 
    valid at that time.
    
    Note:  The commands are automatically assigned IDs beginning with zero in the order the commands
    occur in the array.
*/

Boolean AddCommands( SRRecognizer inSpeechRecognizer, SRLanguageModel inCommandsLangaugeModel, CFArrayRef inCommandNamesArray )
{

    // Dictionary keys for Commands Data property list
    #define	kCommandsDataPlacementHintKey			"PlacementHint"			// value type is: CFNumberRef
        #define	kPlacementHintWhereverNumValue			0
        #define	kPlacementHintProcessNumValue			1					// you must also provide the PSN using kCommandsDataProcessPSNHighKey & kCommandsDataProcessPSNLowKey
        #define	kPlacementHintFirstNumValue				2
        #define	kPlacementHintMiddleNumValue			3
        #define	kPlacementHintLastNumValue				4
    #define	kCommandsDataProcessPSNHighKey			"ProcessPSNHigh"		// value type is: CFNumberRef
    #define	kCommandsDataProcessPSNLowKey			"ProcessPSNLow"			// value type is: CFNumberRef
    #define	kCommandsDataDisplayOrderKey				"DisplayOrder"			// value type is: CFNumberRef  - the order in which recognizers from the same client process should be displayed.
    #define	kCommandsDataCmdInfoArrayKey				"CommandInfoArray"		// value type is: CFArrayRef of CFDictionaryRef values
        #define	kCommandInfoNameKey						"Text"
        #define	kCommandInfoChildrenKey					"Children"

    Boolean	successfullyAddCommands = false;
    
    if (inCommandNamesArray) {
    
        CFIndex	numOfCommands = CFArrayGetCount( inCommandNamesArray );
        CFMutableArrayRef	theSectionArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
        
        // Erase any existing commands in the language model before we begin adding the given list of commands
        if (SREmptyLanguageObject( inCommandsLangaugeModel ) == noErr && theSectionArray) {
        
            CFIndex	i;
            char 	theCSringBuffer[100];

            successfullyAddCommands = true;
            for (i = 0; i < numOfCommands; i++) {
               
                 CFStringRef	theCommandName = CFArrayGetValueAtIndex(inCommandNamesArray, i);
                if (theCommandName && CFStringGetCString( theCommandName, theCSringBuffer, 100, kCFStringEncodingMacRoman )) {
                    
                    if (SRAddText( inCommandsLangaugeModel, theCSringBuffer, strlen(theCSringBuffer), i) == noErr) {
    
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
            }
    
            //
            // Create the XML data that contains the commands to display and give this data to the
            // recognizer object to display in the Speech Commands window.
            //
            {
                CFIndex				numOfParams = 4;
                CFStringRef 		keys[numOfParams];
                CFTypeRef			values[numOfParams];
                CFDictionaryRef 	theItemDict;
                SInt32				placementHint = 1;	// 1 = show only when this app is front process
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


/*
    ConvertAppleEventResultIntoCommandID
    
    This routine converts the Apple event data received from a speech done (kAESpeechSuite, kAESpeechDone) 
    Apple event into a command ID when using the AddCommands routine to add your commands to the top-level
    language model.
    
    This routine returns true if the spoken utterance was recognized as a valid command.
*/

Boolean ConvertAppleEventResultIntoCommandID( const AppleEvent *inAppleEvent, UInt32 * outCommandID )
{
    Boolean			successfullyFound = false;
    SInt32			actualSize;
    DescType		actualType;
    OSErr			recStatus = noErr;
    
    //	Get recognition status from AE, and check that it is equal to 0.
    if (AEGetParamPtr( inAppleEvent, keySRSpeechStatus, typeShortInteger, &actualType, (Ptr)&recStatus, sizeof(recStatus), &actualSize) == noErr && recStatus == noErr) {
    
        // Get recognition result from AE, and continue if not NULL.
        SRRecognitionResult recResult = NULL;
        if (AEGetParamPtr( inAppleEvent, keySRSpeechResult, typeSRSpeechResult, &actualType, (Ptr)&recResult, sizeof(recResult), &actualSize) == noErr && recResult) {
    
            // Get language model result from recognition result
            SInt32 				theLen = sizeof(SRLanguageModel);
            SRLanguageModel		resultLanguageModel	= 0;  
            if (SRGetProperty( recResult, kSRLanguageModelFormat, &resultLanguageModel, &theLen ) == noErr) {
            
                // Get the recognized command object from the language model
                // Since there will only be one object in the langauge model, we just grab the first one.
                SRLanguageObject	resultingCommandObj = 0;
                if (SRGetIndexedItem( resultLanguageModel, &resultingCommandObj, 0 ) == noErr) {
                
                    // Get the command ID from the refCon property of the recognized command object.
                    theLen = sizeof(UInt32);
                    if (SRGetProperty( resultingCommandObj, kSRRefCon, outCommandID, &theLen ) == noErr)
                        successfullyFound = true;
                }
            }
        }
    }
    
    return successfullyFound;
}

