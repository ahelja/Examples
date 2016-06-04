/*
 Get Buddy Info.m
 Get Buddy Info

 Copyright (c) 2005, Apple Computer, Inc., all rights reserved.

 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "Get Buddy Info.h"
#import <AddressBook/AddressBook.h>
#import <InstantMessage/IMService.h>

@implementation GetBuddyInfo

- (id)runWithInput:(id)input fromAction:(AMAction *)anAction error:(NSDictionary **)errorInfo
{
	NSMutableArray *people = [NSMutableArray array];
	ABRecord *person;
	
	// convert the input to a list of ABPerson objects
	if ([input isKindOfClass:[NSAppleEventDescriptor class]])
	{
		if ([(NSAppleEventDescriptor *)input descriptorType] == typeAEList)
		{
			int i,count = [(NSAppleEventDescriptor *)input numberOfItems];
			NSAppleEventDescriptor *personDescriptor;
			NSString *uid = nil;
			
			for (i=1;i<=count;i++)
			{
				personDescriptor = [(NSAppleEventDescriptor *)input descriptorAtIndex:i];
				if (personDescriptor)
				{
					// get the uid of the person from this descriptor
					if ([personDescriptor descriptorType] == typeObjectSpecifier)
					{ 
						NSAppleEventDescriptor* container = [personDescriptor descriptorForKeyword:keyAEContainer];
						if (container)
						{
							NSAppleEventDescriptor* uidDescriptor = [[container descriptorAtIndex:i] descriptorForKeyword:keyAEKeyData];
							if (uidDescriptor)
							{
								uid = [uidDescriptor stringValue];
								if (uid)
								{
									// get the person object from the uid
									person = [[ABAddressBook sharedAddressBook] recordForUniqueId:uid];
									if (person)
									{
										[people addObject:person];
									}
								}
							}
						}
					}
				}
			}
		}
	}

	NSEnumerator *peopleEnumerator = [people objectEnumerator];
	NSMutableArray *info = [NSMutableArray array];
	
	while (person = [peopleEnumerator nextObject])
	{
		NSEnumerator *serviceEnumerator = [[IMService allServices] objectEnumerator];
		IMService *service;
		
		// iterate through each message service
		while (service = [serviceEnumerator nextObject])
		{
			if ([person isKindOfClass:[ABPerson class]])
			{
				NSArray *screenNames = [service screenNamesForPerson:(ABPerson *)person];
				if (screenNames)
				{
					NSEnumerator *nameEnumerator = [screenNames objectEnumerator];
					NSString *screenName;
					
					// iterate through each screen name for this person
					while (screenName = [nameEnumerator nextObject])
					{
						NSDictionary *dict = [service infoForScreenName:screenName];
						if (dict)
						{
							// build the description
							NSMutableString *description = [NSMutableString stringWithString:@"\rName: "];
							
							NSString *firstName = [dict objectForKey:IMPersonFirstNameKey];
							if (firstName)
							{
								[description appendString:firstName];
								[description appendString:@" "];
							}
							
							NSString *lastName = [dict objectForKey:IMPersonLastNameKey];
							if (lastName)
							{
								[description appendString:lastName];
							}
							
							[description appendString:@"\r"];
							
							NSString *serviceName = [dict objectForKey:IMPersonServiceNameKey];
							if (serviceName)
							{
								[description appendString:@"Service: "];
								[description appendString:serviceName];
								[description appendString:@"\r"];
							}
							
							NSString *screenName = [dict objectForKey:IMPersonScreenNameKey];
							if (screenName)
							{
								[description appendString:@"Screen Name: "];
								[description appendString:screenName];
								[description appendString:@"\r"];
							}
							
							int status = [[dict objectForKey:IMPersonStatusKey] intValue];
							if (status)
							{
								[description appendString:@"Status: "];
								
								if (status == IMPersonStatusUnknown)
								{
									[description appendString:@"Unknown"];
								}
								else if (status == IMPersonStatusOffline)
								{
									[description appendString:@"Offline"];
								}
								else if (status == IMPersonStatusIdle)
								{
									[description appendString:@"Idle"];
								}
								else if (status == IMPersonStatusAway)
								{
									[description appendString:@"Away"];
								}
								else if (status == IMPersonStatusAvailable)
								{
									[description appendString:@"Available"];
								}
								
								[description appendString:@"\r"];
							}
							
							NSString *message = [dict objectForKey:IMPersonStatusMessageKey];
							if (message)
							{
								[description appendString:@"Message: "];
								[description appendString:message];
								[description appendString:@"\r"];
							}
							
							[info addObject:description];
						}
					}
				}
			}
		}
	}
	
	return [info description];
}

@end
