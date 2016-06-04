#import "XMethodInspector.h"

@implementation XMethodInspector

void setFieldFromDict(NSDictionary* dict, id target, id key)
{
	NSString* s = [dict objectForKey: key];
	if (s == NULL)
		s = @"";
	[target setStringValue: [s description]];
}

void setFieldFromDictTextArea(NSDictionary* dict, id target, id key)
{
	NSString* s = [dict objectForKey: key];
	if (s == NULL)
		s = @"";
	[target setString: [s description]];
}

-(void) updateFromDict:(NSDictionary*) dict
{
	setFieldFromDict(dict, fDiscussionURL, @"discussion");
	setFieldFromDict(dict, fEmail, @"email");
	setFieldFromDict(dict, fID, @"id");
	setFieldFromDict(dict, fImplementationID, @"implementationId");
	setFieldFromDict(dict, fInfoURL, @"infoURL");
	setFieldFromDict(dict, fName, @"name");
	setFieldFromDict(dict, fPublisherID, @"publisherID");
	setFieldFromDict(dict, fShortDescription, @"shortDescription");
	setFieldFromDict(dict, fTModelID, @"tmodelID");
	setFieldFromDict(dict, fUUID, @"uuid");
	setFieldFromDict(dict, fWSDLURL, @"wsdlURL");

	setFieldFromDictTextArea(dict, fDescription, @"description");
	setFieldFromDictTextArea(dict, fNotes, @"notes");
}

@end
