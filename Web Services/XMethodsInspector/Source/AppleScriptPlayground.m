#import "AppleScriptPlayground.h"
#include <Carbon/Carbon.h>

/*
 * Ripped off code from AEXMLTutor.  It lives!
 * 
 * However, I ran out of time to make this truly nice.
 * Enhancments would be to use the parent property of the editable
 * script to to make the child script have the stub script as a parent.
 * then we would be able to decode error messages better.
 */

static ComponentInstance gGenericComponent;

static void failErr(const char* s)
{
	NSLog(@"%s", s);
	exit(-1);
}

static ComponentInstance getGenericComponent()
{
	if (gGenericComponent == NULL) {
		gGenericComponent = OpenDefaultComponent(kOSAComponentType, kOSAGenericScriptingComponentSubtype);

		if (gGenericComponent == NULL)
			failErr("No such component.");
		else if (gGenericComponent == (ComponentInstance) badComponentInstance)
			failErr("Bad component instance.");
		else if (gGenericComponent == (ComponentInstance) badComponentSelector)
			failErr("Bad component selector.");
	}

	return gGenericComponent;
}

NSString* descToString(AEDesc* desc)
{
	char* cString = NULL;
	AEDesc* toCoerce = desc;
	AEDesc coercer;

	if (desc->descriptorType != typeChar) {
		toCoerce = &coercer;

		if (AECoerceDesc(desc, typeChar, toCoerce) != noErr)
			toCoerce = NULL;
	}

	if (toCoerce != NULL) {
		Size size = AEGetDescDataSize(toCoerce);
		cString = (char*) malloc(size + 1);
		if (cString != NULL) {
			cString[size] = 0;
			if (AEGetDescData(toCoerce, cString, size) != noErr) {
				free(cString);
				cString = NULL;
			}
		}
	}

	if (toCoerce == &coercer)
		AEDisposeDesc(&coercer);

	if (cString == NULL)
		return @"";

	NSString* result = [NSString stringWithCString: cString];
	free(cString);
	return result;
}

static NSString* getOSAValue(OSAID osaid)
{
	NSString* result;
	AEDesc outText;

	if (OSADisplay(getGenericComponent(), osaid, typeChar, 0L, &outText) != noErr)
		result = @"(cannot display)";
	else {
		result = descToString(&outText);
		AEDisposeDesc(&outText);
		if (result == NULL)
			result = @"Can't coerce OSAID to text";
	}

	return result;
}

static NSString* getErrorSpecifics(OSErr err, int* outRangeStart, int* outRangeLength)
{
	NSString* result = NULL;
	AEDesc errDesc;
	OSErr newErr;
	
	*outRangeStart = 0;
	*outRangeLength = -1;

	// Message
	newErr = OSAScriptError(getGenericComponent(), kOSAErrorMessage, typeChar, &errDesc);
	if (newErr == noErr) {
		result = descToString(&errDesc);
		AEDisposeDesc(&errDesc);
	} else {
		result = [[NSString stringWithFormat: @"Execution error (%d) message not available (%d)", err, newErr] retain];
	}

	// See if there was an error range
	{
		newErr = OSAScriptError(getGenericComponent(), kOSAErrorRange, typeAERecord, &errDesc);

		if (newErr == noErr) {
			short startPos, endPos;
			Size actualSizeToss;
			DescType actualTypeToss;

			newErr = AEGetKeyPtr(&errDesc, keyOSASourceStart, typeShortInteger, &actualTypeToss, &startPos, sizeof(startPos), &actualSizeToss);

			if (newErr == noErr) {
				*outRangeStart = startPos;

				newErr = AEGetKeyPtr(&errDesc, keyOSASourceEnd, typeShortInteger, &actualTypeToss, &endPos, sizeof(endPos), &actualSizeToss);
				if (newErr == noErr)
					*outRangeLength = endPos - startPos;
			}

			AEDisposeDesc(&errDesc);
		}
	}

	return result;
}

static NSString* getDebugData(const AEDesc* record, OSType field)
{
	NSString* r = NULL;
	AEDesc paramDesc;
	OSErr err = AEGetParamDesc(record, field, typeChar, &paramDesc);
	if (err != noErr)
		r = @"";
	else {
		r = descToString(&paramDesc);
		AEDisposeDesc(&paramDesc);
	}
	return r;
}

// Override send proc to add debug data
static OSErr _mySendProc(  const AppleEvent*		theAppleEvent,
						   AppleEvent*				reply,
						   AESendMode 				sendMode,
						   AESendPriority 			sendPriority,
						   long 					timeOutInTicks,
						   AEIdleUPP				idleProc,
						   AEFilterUPP				filterProc,
						   long 					refCon)
{
	OSErr err;

	AEDesc toSend;
	err = AEDuplicateDesc(theAppleEvent, &toSend);

	if (err == noErr) {
		SInt32 debugAttr = kAEDebugXMLDebugAll;
		err = AEPutAttributePtr(&toSend, keyXMLDebuggingAttr, typeSInt32, &debugAttr, sizeof(debugAttr));

		if (err == noErr)
			err = AESend(&toSend, reply, sendMode, sendPriority, timeOutInTicks, idleProc, filterProc);

		if (err == noErr) {
			NSString* postHeader 	= getDebugData(reply, keyAEPOSTHeaderData);
			NSString* postData		= getDebugData(reply, keyAEXMLRequestData);
			NSString* replyHeader 	= getDebugData(reply, keyAEReplyHeaderData);
			NSString* replyData 	= getDebugData(reply, keyAEXMLReplyData);
			NSString* s = NULL;

			if (postHeader && [postHeader length] != 0) {
				s = [NSString stringWithFormat: @"\n\nHTTP Debugging Information:\n\n%@\n\n%@\n\n%@\n%@\n\n",
					postHeader, postData,
					replyHeader, replyData];
			}

			if (s) {
				NSString** pOut = (NSString**) refCon;

				if (*pOut == NULL)
					*pOut = s;
				else {
					*pOut = [*pOut stringByAppendingString: s];
				}
			}
		}

		AEDisposeDesc(&toSend);
	}

	return err;
}



static NSString* eval(OSAID scriptID)
{
	NSString* result = NULL;
	OSAID resultID = 0;
	NSString* debugData = NULL;
	
	OSASetSendProc(getGenericComponent(), NewOSASendUPP(_mySendProc), (long) &debugData);
	
	OSErr err = OSAExecute(getGenericComponent(), scriptID, kOSANullScript, 0L, &resultID);

	if (err == noErr) {
		result = getOSAValue(resultID);
		OSADispose(getGenericComponent(), resultID);
	} else {
		int start, end;
		result = getErrorSpecifics(err, &start, &end);
	}

	return [result stringByAppendingFormat: @"\n%@\n", debugData];
}

OSAID compile(NSString* srcString, NSString** outError)
{
	OSAID result = 0;

	*outError = NULL;

	const char* src = [srcString cString];
	if (src == NULL) {
		*outError = @"Can't get source text";
	} else {
		AEDesc srcDesc;

		AECreateDesc(typeChar, src, strlen(src), &srcDesc);
		
		OSErr err = OSACompile(getGenericComponent(), &srcDesc, 0, &result);
		if (err != noErr) {
			int start, end;
			*outError = getErrorSpecifics(err, &start, &end);
		}
	}
	
	return result;
}

@implementation AppleScriptPlayground

- (IBAction)runApplescript:(id) sender
{
	NSLog(@"run applescript...");

	[fProgressIndicator startAnimation: self];
	
	OSAID scriptID = 0;

	NSString* stubString = [fStubsPanel string];
	NSString* combinedString = [stubString stringByAppendingString: [fEditPanel string]];

	NSString* errorString = NULL;
	scriptID = compile(combinedString, &errorString);

	if (scriptID == 0) {
		if (errorString == NULL)
			[fResultPanel setString: @"Compilation errors abound"];
		else
			[fResultPanel setString: errorString];
	} else {
		NSString* result = eval(scriptID);

		if (result == NULL)
			[fResultPanel setString: @"Couldn't eval the script for some reason"];
		else {
			[fResultPanel setString: result];
		}
		OSADispose(getGenericComponent(), scriptID);
	}

	[fProgressIndicator stopAnimation: self];
}

@end
