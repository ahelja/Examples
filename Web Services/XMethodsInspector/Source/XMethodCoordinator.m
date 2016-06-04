#import "XMethodCoordinator.h"
#import "XMethodInspector.h"
#import "XMethodsServiceList.h"

// Generated source:
#import "XMethodsStubs.h"

@implementation XMethodCoordinator

static void setStubs(const char* emitter, id srcTarget, const char* srcExtension, id hTarget, const char* hExtension, NSString* wsdl)
{
	char tmp[4096];
	snprintf(tmp, sizeof(tmp), "/Developer/Tools/WSMakeStubs -x %s -dir /tmp -name XMethodsCoordinator -url %s", emitter, [wsdl cString]);
	system(tmp);

	if (srcTarget != NULL) {
		[srcTarget readRTFDFromFile: [NSString stringWithFormat:@"/tmp/XMethodsCoordinator%s", srcExtension]];
		[srcTarget setRichText: false];
		[srcTarget setFont: [NSFont userFixedPitchFontOfSize: 12]];
	}

	if (hTarget != NULL) {
		[hTarget readRTFDFromFile: [NSString stringWithFormat:@"/tmp/XMethodsCoordinator%s", hExtension]];
		[hTarget setRichText: false];
		[hTarget setFont: [NSFont userFixedPitchFontOfSize: 12]];
	}
}

- (void) invocationCompleted:(getServiceDetail*) invocation
{
	NSLog(@"invocationCompleted now...");

	NSDictionary* dict = [invocation resultValue];
	NSString* wsdl = [dict objectForKey: @"wsdlURL"];
	[fInspector updateFromDict: dict];

	if (wsdl == NULL) {
		[fCPlusPlusStubView setString: @"No WSDL URL specified"];
		[fCPlusPlusStubView_h setString: @"No WSDL URL specified"];
		[fObjcStubView setString: @"No WSDL URL specified"];
		[fObjcStubView_h setString: @"No WSDL URL specified"];
		[fAppleScriptStubView setString: @"No WSDL URL specified"];
	} else {
		setStubs("c++", fCPlusPlusStubView, ".cp", fCPlusPlusStubView_h, ".h", wsdl);
		setStubs("ObjC", fObjcStubView, ".m", fObjcStubView_h, ".h", wsdl);
		setStubs("applescript", fAppleScriptStubView, ".as", NULL, NULL, wsdl);
	}

	[dict release];

	[fProgressIndicator stopAnimation: self];
}

- (IBAction)fetch:(id)sender
{
	NSLog(@"Fetching data...");

	[fProgressIndicator startAnimation: self];

	int ix = [fServiceTable selectedRow];
	if (ix >= 0) {
		NSString* itemID = [fServiceList idForItem: ix];

		getServiceDetail* obj = [[getServiceDetail alloc] init];
		[obj setParameters:itemID];
		[obj setCallBack:(id)self selector:@selector(invocationCompleted:)];
		[obj scheduleOnRunLoop:(NSRunLoop*) CFRunLoopGetCurrent() mode: (NSString*) kCFRunLoopDefaultMode];
	}
}

@end
