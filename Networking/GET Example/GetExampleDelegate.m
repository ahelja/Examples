/*
	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.

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


#pragma mark Includes
#import "GetExampleDelegate.h"

/*
	The following is some pseudo code which roughly indicates the correct usage of
	CFHTTPAuthentication objects.  This sample has taken certain liberties because
	of its asynchronous implementation in comparison to the synchronous pseudo code.
	The sample also does not deal with proxy authentication issues (407 responses),
	nor does it choose to save usernames and passwords beyond a single run.
	
		CFHTTPMessage request = // Make request from url
		CFHTTPAuth auth = findPreviousAuthToUseForThisRequest();
		CFHTTPMessage response = NULL
	 
		do
			// Make a new Auth object if we've failed once and aren't working on one
			if (response && !auth)
				if (timeToGiveUpForWhateverReason())
					break - we have the final response
				else if (rightTimeToAskForCreds())
					prompt for creds
					if (user cancelled)
						break - final response will be the last 401
				create auth
	 
			// Apply the auth if needed
			if (auth)
				authError = TRUE
				if (auth is valid)
					make new request if needed
					if (apply auth to request)
						// now OK to send
						authError = FALSE
					if (authError)
						// failure for this auth attempt
						auth == NULL
						continue
	 
			send request
			get response
	 
		while (!streamsError && response code == 401)
	 
		now we have the final response (200, 300, 401, ...) or a streamsError
*/

#pragma mark -
#pragma mark Constants & Globals
static const CFOptionFlags kNetworkEvents = kCFStreamEventOpenCompleted |
                                            kCFStreamEventHasBytesAvailable |
                                            kCFStreamEventEndEncountered |
											kCFStreamEventErrorOccurred;


static NSMutableArray* credentials = nil;


#pragma mark -
@interface Authentication : NSObject
{
	CFHTTPAuthenticationRef		_auth;
	NSString*					_user;
	NSString*					_pass;
	NSString*					_domain;
}

+(Authentication*)findAuthenticationForRequest:(CFHTTPMessageRef)request;
+(void)removeAuthentication:(Authentication*)auth;
+(void)addAuthentication:(Authentication*)auth;

-(void)dealloc;

-(id)initWithHTTPResponse:(CFHTTPMessageRef)response;

-(NSString*)username;
-(NSString*)password;
-(NSString*)realm;

-(void)setUsername:(NSString*)username;
-(void)setPassword:(NSString*)password;
-(void)setDomain:(NSString*)domain;

-(BOOL)requiresPrompt;
-(BOOL)requiresDomain;

-(BOOL)applyCredentialsToRequest:(CFHTTPMessageRef)request;

@end


#pragma mark -
#pragma mark Static Functions
static void
ReadStreamClientCallBack(CFReadStreamRef stream, CFStreamEventType type, void *clientCallBackInfo) {
    // Pass off to the object to handle.
    [((GetExampleDelegate*)clientCallBackInfo) handleNetworkEvent: type];
}


#pragma mark -
@implementation GetExampleDelegate


- (IBAction)fetchUrl:(id)sender
{
    CFHTTPMessageRef request;
    NSString* url_string = [urlTextField stringValue];
	Authentication* auth;
    
    // Make sure there is a string in the text field
    if (!url_string || ![url_string length]) {
        [statusTextField setStringValue: @"Enter a valid URL."];
        return;
    }
    
    // If there is no scheme of http or https, slap "http://" on the front.
    if (![url_string hasPrefix: @"http://"] && ![url_string hasPrefix: @"https://"])
        url_string = [NSString stringWithFormat: @"http://%@", url_string];
    
    // Release the old url
    if (_url) {
        [_url release];
        _url = NULL;
    }
    
    // Create a new url based upon the user entered string
    _url = [NSURL URLWithString: url_string];
    
    // Make sure it succeeded
    if (!_url) {
        [statusTextField setStringValue: @"Enter a valid URL."];
        return;
    }
    
    // Hold it around so that the authorization code can get to it
    // in order to display the host name in the prompt.
    [_url retain];

    // Create a new HTTP request.
    request = CFHTTPMessageCreateRequest(kCFAllocatorDefault, CFSTR("GET"), (CFURLRef)_url, kCFHTTPVersion1_1);
    if (!request) {
        [statusTextField setStringValue: @"Creating the request failed."];
        return;
    }
    
	auth = [Authentication findAuthenticationForRequest: request];
	if (auth && ![auth applyCredentialsToRequest: request])
		[Authentication removeAuthentication: auth];
	
    // Start the fetch.
    [self fetch: request];
    
    // Release the request.  The fetch should've retained it if it
    // is performing the fetch.
    CFRelease(request);
}


- (IBAction)refetchUrl:(id)sender
{	
	Authentication* auth = [Authentication findAuthenticationForRequest: _request];

	// If authPanel had been opened, need to get the user/pass information.
	if ([authPanel isVisible]) {
		
		// Get the user entered values and set in the authentication object.
		[auth setUsername: [authName stringValue]];
		[auth setPassword: [authPass stringValue]];
		
		// Save the domain too, if that was required.
		if ([auth requiresDomain]) {
			NSString* domain = [domainField stringValue];
			[auth setDomain: (domain && [domain length]) ? domain : nil];
		}
		
		// For security sake, empty out the password field.
		[authPass setStringValue: @""];
		
		// Close the panel
		[authPanel performClose: self];
	}

	if ([auth applyCredentialsToRequest: _request])
		[self fetch: _request];
	
	else {
		// Remove this particular auth object, so fetch doesn't end up
		// with bad auth every time.
		[Authentication removeAuthentication: auth];
		
		[statusTextField setStringValue: @"Could not apply authenticatiion to the request."];
	}
}


- (void)fetch:(CFHTTPMessageRef)request
{
    
    CFHTTPMessageRef old;
	CFReadStreamRef stream;
    CFStreamClientContext ctxt = {0, self, NULL, NULL, NULL};
    
    // Clear out the old results
    [resultsTextView scrollRangeToVisible: NSMakeRange(0, 0)];
    [resultsTextView replaceCharactersInRange: NSMakeRange(0, [[resultsTextView string] length])
                     withString: @""];
    
    // Swap the old request and the new request.  It is done in this
    // order since the new request could be the same as the existing
    // request.  If the old one is released first, it could be destroyed
    // before retain.
    old = _request;
    _request = (CFHTTPMessageRef)CFRetain(request);
    if (old)
        CFRelease(old);
    
    // Create the stream for the request.
    stream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, _request);

    // Make sure it succeeded.
    if (!stream) {
        [statusTextField setStringValue: @"Creating the stream failed."];
        return;
    }
    
	// Use persistent conections so connection-based authentications work correctly.
	CFReadStreamSetProperty(stream, kCFStreamPropertyHTTPAttemptPersistentConnection, kCFBooleanTrue);
	
    // Set the client
    if (!CFReadStreamSetClient(stream, kNetworkEvents, ReadStreamClientCallBack, &ctxt)) {
        CFRelease(stream);
        [statusTextField setStringValue: @"Setting the stream's client failed."];
        return;
    }
    
    // Schedule the stream
    CFReadStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    
    // Start the HTTP connection
    if (!CFReadStreamOpen(stream)) {
        CFReadStreamSetClient(stream, 0, NULL, NULL);
        CFReadStreamUnscheduleFromRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFRelease(stream);
        [statusTextField setStringValue: @"Opening the stream failed."];
        return;
    }
    
    // Don't need the old stream any more.
	if (_stream) {
		
		// This may seem odd to close the old stream at this point.
		// A persistent connection's underlying connection will be held
		// as long as there is one stream left in the "not closed"
		// state (the stream may be at the end but the actual Close
		// function has not be called).  The new stream is being opened
		// before the old one closes, so if the stream is to the same
		// server with the same properties, the same pipe will be used.
		// It's very important to use the same pipe for authentication
		// methods such as NTLM.
		CFReadStreamClose(_stream);
		CFRelease(_stream);
	}
	
	_stream = stream;
}


- (void)handleNetworkEvent:(CFStreamEventType)type {
    
    // Dispatch the stream events.
    switch (type) {
        case kCFStreamEventHasBytesAvailable:
            [self handleBytesAvailable];
            break;
            
        case kCFStreamEventEndEncountered:
            [self handleStreamComplete];
            break;
            
        case kCFStreamEventErrorOccurred:
            [self handleStreamError];
            break;
            
        default:
            break;
    }
}


- (void)handleBytesAvailable {

    UInt8 buffer[2048];
    CFIndex bytesRead = CFReadStreamRead(_stream, buffer, sizeof(buffer));
    
    // Less than zero is an error
    if (bytesRead < 0)
        [self handleStreamError];
    
    // If zero bytes were read, wait for the EOF to come.
    else if (bytesRead) {
        
        // This would not work for binary data!  Build a string to add
        // to the results.
        NSString* to_add = [NSString stringWithCString: (char*)buffer length: bytesRead];
        
        // Append and scroll the results field.
        [resultsTextView replaceCharactersInRange: NSMakeRange([[resultsTextView string] length], 0)
                         withString: to_add];
        
        [resultsTextView scrollRangeToVisible: NSMakeRange([[resultsTextView string] length], 0)];
    }
}


- (void)handleStreamComplete {
    
    CFHTTPMessageRef response = (CFHTTPMessageRef)CFReadStreamCopyProperty(_stream, kCFStreamPropertyHTTPResponseHeader);
	
	// Remove the stream from the run loop and such.  Don't close the connection
	// yet.  See the note in -fetch:.
	CFReadStreamSetClient(_stream, 0, NULL, NULL);
	CFReadStreamUnscheduleFromRunLoop(_stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
	
    [statusTextField setStringValue: @"Fetch is complete."];
	
    // Check to see if it is a 401 "Authorization Needed" error.  To
    // test for proxy authentication, 407 would have to be caught too.
    if (CFHTTPMessageGetResponseStatusCode(response) == 401) {
    
		Authentication* auth = [Authentication findAuthenticationForRequest: _request];

		// No authentication, so must be the first time.  Create the authentication.
		if (!auth) {
			auth = [[Authentication alloc] initWithHTTPResponse: response];
			if (auth)
				[Authentication addAuthentication: auth];
		}

		if (auth) {
			
			// No need to prompt if no user/pass required.  Go straight to load.
			if (![auth requiresPrompt])
				[self refetchUrl: self];
			
			else {
				
				[authPrompt setStringValue: [NSString stringWithFormat: @"Connect to %@ as", [_url host]]];
				
				if ([auth requiresDomain]) {
					
					[realmPrompt setStringValue: @"Domain"];
					[domainField setStringValue: @""];
					[domainField setHidden: NO];
					[authRealm setHidden: YES];
				}
				
				else {
					
					NSString* realm = [auth realm];
					[realmPrompt setStringValue: @"Realm"];
					[authRealm setStringValue: realm ? realm : @""];
					[domainField setHidden: YES];
					[authRealm setHidden: NO];
				}
                
                // Prompt the user
                [authPanel makeKeyAndOrderFront: self];
				
			}
		}
	}
	
	CFRelease(response);
}


- (void)handleStreamError {

	CFStreamError error = CFReadStreamGetError(_stream);

    CFReadStreamSetClient(_stream, 0, NULL, NULL);
    CFReadStreamUnscheduleFromRunLoop(_stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    CFReadStreamClose(_stream);
    CFRelease(_stream);
    _stream = NULL;
	
    // Lame error handling.  Simply state that an error did occur.
    [statusTextField setStringValue: [NSString stringWithFormat: @"Error occurred (%ld, %ld).", error.domain, error.error]];
}


@end


#pragma mark -
@implementation Authentication


+(Authentication*)findAuthenticationForRequest:(CFHTTPMessageRef)request {

	Authentication* auth = nil;
	
	if (!credentials)
		credentials = [[NSMutableArray alloc] init];

	if (credentials) {
		
		NSEnumerator* iter = [credentials objectEnumerator];
		
		
		// Note that this is grossly inefficient for a large number of authentication
		// objects and URLs.  It's done this way primarily to show how the relation
		// is used.  A tree based upon domains and realms would be best.
		while ((auth = [iter nextObject])) {
			
			if (CFHTTPAuthenticationAppliesToRequest(auth->_auth, request)) {
				
				// Make sure the authentication object hasn't become invalid.
				if (!CFHTTPAuthenticationIsValid(auth->_auth, NULL)) {
					
					// Remove the invalid object from the list and return nil.
					[Authentication removeAuthentication: auth];
					auth = nil;
				}
				
				return auth;
			}
		}
	}
	
	return auth;
}


+(void)removeAuthentication:(Authentication*)auth {
	
	if (!credentials)
		credentials = [[NSMutableArray alloc] init];
	
	if (credentials)
		[credentials removeObject: auth];
}


+(void)addAuthentication:(Authentication*)auth {
	
	if (!credentials)
		credentials = [[NSMutableArray alloc] init];

	if (credentials)
		[credentials addObject: auth];
}

						  
-(void)dealloc {
	
	if (_user)
		[_user release];
	
	if (_pass)
		[_pass release];
	
	if (_auth)
		CFRelease(_auth);
	
	[super dealloc];
}


-(id)initWithHTTPResponse:(CFHTTPMessageRef)response {
	
	if ((self = [self init])) {
		
		_auth = CFHTTPAuthenticationCreateFromResponse(kCFAllocatorDefault, response);
		if (!_auth || !CFHTTPAuthenticationIsValid(_auth, NULL)) {
			[self release];
			self = nil;
		}
	}
	
	return self;
}

						  
-(NSString*)username {
	
	return _user ? [NSString stringWithString: _user] : nil;
}


-(NSString*)password {
	
	return _pass ? [NSString stringWithString: _pass] : nil;
}


-(NSString*)realm {
	
	NSString* result = (NSString*)CFHTTPAuthenticationCopyRealm(_auth);
	
	if (result)
		[result autorelease];
	
	return result;
}


-(void)setUsername:(NSString*)username {
	
	NSString* old = _user;
	_user = username ? [[NSString alloc] initWithString: username] : nil;
	[old autorelease];
}


-(void)setPassword:(NSString*)password {
	
	NSString* old = _pass;
	_pass = password ? [[NSString alloc] initWithString: password] : nil;
	[old autorelease];
}


-(void)setDomain:(NSString*)domain {
	
	NSString* old = _domain;
	_domain = domain ? [[NSString alloc] initWithString: domain] : nil;
	[old autorelease];
}


-(BOOL)requiresPrompt {
	
	return CFHTTPAuthenticationRequiresUserNameAndPassword(_auth) ? YES : NO;
}


-(BOOL)requiresDomain {
	
	return CFHTTPAuthenticationRequiresAccountDomain(_auth) ? YES : NO;
}


-(BOOL)applyCredentialsToRequest:(CFHTTPMessageRef)request {

	NSMutableDictionary* dict = [NSMutableDictionary dictionary];

	if (CFHTTPAuthenticationRequiresUserNameAndPassword(_auth)) {
		
		// Get the user entered values for name and password.
		[dict setObject: _user forKey: (NSString*)kCFHTTPAuthenticationUsername];
		[dict setObject: _pass forKey: (NSString*)kCFHTTPAuthenticationPassword];
				
		// If the authentication required a domain, grab it and add to the credentials too.
		if ([self requiresDomain] && _domain)
			[dict setObject: _domain forKey: (NSString*)kCFHTTPAuthenticationAccountDomain];
	}
	
	// Try to add the authentication credentials to the request.  If it
    // succeeds, perform the fetch.  This example does not support proxy
    // authentication, but the API does support it.  Since all the
	// authentication objects are created from 401 responses, they will
	// only be applied as non-proxied.  Had the authentication object
	// been created from a 407 response, the credentials would have been
	// applied for the proxy.  In the case of a 407, only one object
	// should have to be maintained per protocol scheme.  Chained,
	// authenticated proxies is a bad thing for HTTP.
    return CFHTTPMessageApplyCredentialDictionary(request, _auth, (CFDictionaryRef)dict, NULL) ? YES : NO;
}

						  
@end
