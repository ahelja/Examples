/*
  cc -g -o xmlrpc xmlrpc.cp -framework ApplicationServices

  Example of calling XML-RPC methods via the AppleEvent Manager.

  Originally sample code for Mac OS X 10.1.

  By Steve Zellers
*/

#include <Carbon/Carbon.h>

#define checkErr(err) \
	while (err != noErr) { fprintf(stderr, "Failed at line %d, error %d\n", __LINE__, err); exit(-1); }

static const char* url = "http://betty.userland.com/RPC2";
static const char* methodName = "examples.getStateName";

static void dumpDebug(const char* msg, const AppleEvent* event, OSType parameter);

int main()
{
	OSErr err;
	AEDesc targetAddress;

	// Create the target address
	err = AECreateDesc(typeApplicationURL, url, strlen(url), &targetAddress);
	checkErr(err);

	// Create the event
	AppleEvent event;
	err = AECreateAppleEvent(kAERPCClass, kAEXMLRPCScheme, &targetAddress, kAutoGenerateReturnID, kAnyTransactionID, &event);
	checkErr(err);
	AEDisposeDesc(&targetAddress);

	// Create the parameters for the event - the direct object is a record that contains
	// the method name, and a list of parameters
	AEDesc directObject;
	err = AECreateList(NULL, 0, true, &directObject);
	checkErr(err);

	// Put the method name
	err = AEPutParamPtr(&directObject, keyRPCMethodName, typeChar, methodName, strlen(methodName));
	checkErr(err);

	// Create the list for the actual parameters
	AEDesc paramList;
	err = AECreateList(NULL, 0, false, &paramList);
	checkErr(err);

	// Put the state index into the parameter array
	SInt32 stateIndex = 41;
	err = AEPutPtr(&paramList, 0, typeSInt32, &stateIndex, sizeof(stateIndex));
	checkErr(err);

	// Put the parameter list into the direct object
	err = AEPutParamDesc(&directObject, keyRPCMethodParam, &paramList);
	checkErr(err);
	AEDisposeDesc(&paramList);

	// Put the direct object into the event
	err = AEPutParamDesc(&event, keyDirectObject, &directObject);
	checkErr(err);
	AEDisposeDesc(&directObject);

	// Request debugging information
	SInt32 debugAttr = kAEDebugXMLDebugAll;
	err = AEPutAttributePtr(&event, keyXMLDebuggingAttr, typeSInt32, &debugAttr, sizeof(debugAttr));

	// Finally, send the event (we're using AESendMessage so we don't link against carbon.)
	AppleEvent reply;
	AEInitializeDescInline(&reply);
	err = AESendMessage(&event, &reply, kAEWaitReply, kAEDefaultTimeout);
	checkErr(err);

	// The direct object contains our result (the name of the state)
	char buffer[255];
	Size actualSize;
	err = AEGetParamPtr(&reply, keyDirectObject, typeChar, NULL, buffer, sizeof(buffer), &actualSize);
	checkErr(err);

	fprintf(stderr, "State at index %d is %.*s!\n\n", stateIndex, actualSize, buffer);

	// Dump debug information
	dumpDebug("HTTP POST header", &reply, keyAEPOSTHeaderData);
	dumpDebug("XML Request", &reply, keyAEXMLRequestData);
	dumpDebug("HTTP Reply header", &reply, keyAEReplyHeaderData);
	dumpDebug("XML Reply", &reply, keyAEXMLReplyData);

	return 0;
}

static void dumpDebug(const char* msg, const AppleEvent* event, OSType parameter)
{
	fprintf(stderr, "%s:\n", msg);

	AEDesc paramDesc;
	OSErr err = AEGetParamDesc(event, parameter, typeChar, &paramDesc);
	if (err != noErr)
		fprintf(stderr, "\tCan't get parameter %4.4s - %d returned\n", &parameter, err);
	else {
		int len = AEGetDescDataSize(&paramDesc);
		char* buffer = new char[len];
		AEGetDescData(&paramDesc, buffer, len);

		char* p = buffer;
		char* pEnd = buffer + len;
		
		while (p < pEnd) {
			char* pNext = strpbrk(p, "\r\n");
			if (pNext == NULL)
				pNext = pEnd;
			else {
				while (pNext < pEnd && (*pNext == '\r' || *pNext == '\n')) {
					*pNext++ = '\0';
				}
			}
			fprintf(stderr, "\t%.*s\n", pNext - p, p);
			p = pNext;
		}

		AEDisposeDesc(&paramDesc);
		delete[] buffer;
	}
	fprintf(stderr, "\n\n");
}
