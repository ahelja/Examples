/*

File: GMConnectionController.m

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright 2004 Apple Computer, Inc., All Rights Reserved

*/ 

#import "GMConnectionController.h"
#import "GMApplicationDelegate.h"

@interface GMConnectionController (Private)

- (void)setConnection:(XGConnection *)connection;
- (void)setController:(XGController *)controller;

- (void)notifyDelegateConnectionControllerDidSucceed;
- (void)notifyDelegateConnectionControllerAuthenticationNeeded;
- (void)notifyDelegateConnectionControllerAuthenticationDidFail;
- (void)notifyDelegateConnectionControllerConnectionDidFail;

@end

@implementation GMConnectionController

- (id)initWithNetServiceDomain:(NSString *)domain type:(NSString *)type name:(NSString *)name authenticator:(XGAuthenticator *)authenticator;
{
    self = [super init];
    
    if (self != nil) {

		_type = GMConnectionControllerNetServiceType;
		
		NSNetService *netService = [[[NSNetService alloc] initWithDomain:domain type:type name:name] autorelease];
		
		XGConnection *connection = [[[XGConnection alloc] initWithNetService:netService] autorelease];
		
		[connection setAuthenticator:authenticator];
		
		XGController *controller = [[[XGController alloc] initWithConnection:connection] autorelease];
		
		[controller setJobsPredicateString:[NSString stringWithFormat:@"%@ == \"%@\"", XGJobSpecificationApplicationIdentifierKey, GMApplicationIdentifier]];
		
		[self setConnection:connection];
		[self setController:controller];
	}
	
	return self;
}
		
- (id)initWithHostname:(NSString *)hostname portnumber:(int)portnumber authenticator:(XGAuthenticator *)authenticator;
{
    self = [super init];
    
    if (self != nil) {

		_type = GMConnectionControllerHostnameType;
		
		XGConnection *connection = [[[XGConnection alloc] initWithHostname:hostname portnumber:portnumber] autorelease];
		
		[connection setAuthenticator:authenticator];
		
		XGController *controller = [[[XGController alloc] initWithConnection:connection] autorelease];
		
		[self setConnection:connection];
		[self setController:controller];
	}
	
	return self;
}
		
- (void)dealloc;
{
	// [_connection close];
	
    [self setConnection:nil];
    [self setController:nil];
    
    [super dealloc];
}

- (GMConnectionControllerType)type;
{
	return _type;
}

- (NSString *)name;
{
	return [[self connection] name];
}

- (NSString *)servicePrincipal;
{
	NSString *servicePrincipal = [[self connection] servicePrincipal];
	
	if (servicePrincipal == nil) [NSString stringWithFormat:@"xgrid/%@", [self name]];
	
	return servicePrincipal;
}

- (XGConnection *)connection;
{
    return _connection;
}

- (XGController *)controller;
{
    return _controller;
}

- (void)setDelegate:(id)delegate;
{
    _delegate = delegate;
}

- (id)delegate;
{
    return _delegate;
}

#pragma mark *** Private methods ***

- (void)setConnection:(XGConnection *)connection;
{
    if (_connection != connection) {
        
        if ([_connection delegate] == self) {
            
            [_connection setDelegate:nil];
        }
                
        [_connection release];
        
        _connection = [connection retain];
        
        [_connection setDelegate:self];
    }
}

- (void)setController:(XGController *)controller;
{
    if (_controller != controller) {
        
        [_controller release];

        _controller = [controller retain];
    }
}

- (void)notifyDelegateConnectionControllerDidSucceed;
{
    if ([_delegate respondsToSelector:@selector(connectionControllerDidSucceed:)] == YES) {
        
        [_delegate connectionControllerDidSucceed:self];
    }
}

- (void)notifyDelegateConnectionControllerAuthenticationNeeded;
{
    if ([_delegate respondsToSelector:@selector(connectionControllerAuthenticationNeeded:)] == YES) {
        
        [_delegate connectionControllerAuthenticationNeeded:self];
    }
}

- (void)notifyDelegateConnectionControllerAuthenticationDidFail;
{
    if ([_delegate respondsToSelector:@selector(connectionControllerAuthenticationDidFail:)] == YES) {
        
        [_delegate connectionControllerAuthenticationDidFail:self];
    }
}

- (void)notifyDelegateConnectionControllerConnectionDidFail;
{
    if ([_delegate respondsToSelector:@selector(connectionControllerConnectionDidFail:)] == YES) {
        
        [_delegate connectionControllerConnectionDidFail:self];
    }
}

#pragma mark *** GMConnection delegate methods ***

- (void)connectionDidOpen:(XGConnection *)connection;
{
	[self notifyDelegateConnectionControllerDidSucceed];
}

- (void)connectionDidClose:(XGConnection *)connection;
{
    if ([connection error] != nil && [[connection error] code] != 200) {
        
		if ([[connection error] code] == 530) {
			
			if ([connection authenticator] == nil) {
				
				[self notifyDelegateConnectionControllerAuthenticationNeeded];
			}
			else {
				
				[self notifyDelegateConnectionControllerAuthenticationDidFail];
			}
		}
		else if ([[connection error] code] == 535) {
			
			[self notifyDelegateConnectionControllerAuthenticationDidFail];
		}
		else {
			
			[self notifyDelegateConnectionControllerConnectionDidFail];
		}
    }
}

@end
