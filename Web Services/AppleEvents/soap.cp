/*
  cc -g -o soap soap.cp -framework ApplicationServices

  Example of calling SOAP methods via the AppleEvent Manager.

  Originally sample code for Mac OS X 10.1.

  By Steve Zellers
*/

#include <Carbon/Carbon.h>

#define checkErr(err) \
	while (err != noErr) { fprintf(stderr, "Failed at line %d, error %d\n", __LINE__, err); exit(-1); }

static const char* url = "http://www.soapware.org/examples";
static const char* methodName = "getStateName";
static const char* methodNameSpaceURI = "http://www.soapware.org/";

static OSStatus createUserRecord(AEDesc* desc, const char* name, OSType dataType, const void* data, UInt32 dataSize);
static OSStatus getUserRecordField(const AEDesc* record, const char* fieldName, OSType desiredType, void* buffer, Size bufferSize, Size* actualSize);
static void dumpDebug(const char* msg, const AppleEvent* reply, OSType keyword);

int main()
{
	OSErr err;
	AEDesc targetAddress;

	// Create the target address
	err = AECreateDesc(typeApplicationURL, url, strlen(url), &targetAddress);
	checkErr(err);

	// Create the event
	AppleEvent event;
	err = AECreateAppleEvent(kAERPCClass, kAESOAPScheme, &targetAddress, kAutoGenerateReturnID, kAnyTransactionID, &event);
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

	// The parameters for a SOAP request are a record.  We use the
	// "AppleScript" format for records that contain user readable
	// names, so a helper routine is used to construct this record.
	AEDesc paramRecord;
	SInt32 stateIndex = 41;
	err = createUserRecord(&paramRecord, "statenum", typeSInt32, &stateIndex, sizeof(stateIndex));
	checkErr(err);

	// Put the parameter record into the direct object
	err = AEPutParamDesc(&directObject, keyRPCMethodParam, &paramRecord);
	checkErr(err);
	AEDisposeDesc(&paramRecord);

	// Additional pieces for soap are the method namespace, namespaceURI and
	// SOAPAction.  If the SOAPAction isn't explicitly specified, it
	// will be the path part of the url (in this case, "/examples")
	// You don't need to supply the method namespace...
	err = AEPutParamPtr(&directObject, keySOAPMethodNameSpaceURI, typeChar, methodNameSpaceURI, strlen(methodNameSpaceURI));
	checkErr(err);

	// Put the direct object into the event
	err = AEPutParamDesc(&event, keyDirectObject, &directObject);
	checkErr(err);
	AEDisposeDesc(&directObject);

	// Request debugging information
	SInt32 debugAttr = kAEDebugXMLDebugAll;
	err = AEPutAttributePtr(&event, keyXMLDebuggingAttr, typeSInt32, &debugAttr, sizeof(debugAttr));

	// Finally, send the event (we're using AESendMessage so we don't link against Carbon, just ApplicationServices)
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

static OSStatus getUserRecordField(const AEDesc* record, const char* fieldName, OSType desiredType, void* buffer, Size bufferSize, Size* actualSize)
{
	// Extract the userfield list from the record.
	AEDesc userField;
	OSStatus err = AEGetParamDesc(record, keyASUserRecordFields, typeAEList, &userField);
	checkErr(err);
	
	// The userfield is an array of name,data, so we compare the name against what we're looking for,
	// and if it maches return nameIndex + 1 coerced appropriately
	long count;
	err = AECountItems(&userField, &count);
	checkErr(err);

	for (long i = 1;  i <= count;  i += 2) {
		char tmpName[255];
		Size tmpNameSize;
		OSType tossKeyword;
		OSType tossType;

		err = AEGetNthPtr(&userField, i, typeChar, &tossKeyword, &tossType, tmpName, sizeof(tmpName), &tmpNameSize);
		checkErr(err);
		
		if (strncmp(fieldName, tmpName, tmpNameSize) == 0) {
			err = AEGetNthPtr(&userField, i + 1, desiredType, &tossKeyword, &tossType, buffer, bufferSize, actualSize);
			checkErr(err);
			break;
		}
	}

	AEDisposeDesc(&userField);

	return err;
}

static OSStatus createUserRecord(AEDesc* desc, const char* name, OSType dataType, const void* data, UInt32 dataSize)
{
	OSErr err = AECreateList(NULL, 0, true, desc);
	if (err == noErr) {
		AEDesc termsList;
		err = AECreateList(NULL, 0, false, &termsList);

		if (err == noErr)
			err = AEPutPtr(&termsList, 0, typeChar, name, strlen(name));
		if (err == noErr)
			err = AEPutPtr(&termsList, 0, dataType, data, dataSize);

		if (err == noErr)
			err = AEPutParamDesc(desc, keyASUserRecordFields, &termsList);

		AEDisposeDesc(&termsList);
	}
	return err;
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
