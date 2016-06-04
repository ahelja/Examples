/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
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
/* NSExtras.m */

#import "NSExtras.h"
#import "KeyValuePair.h"

@implementation NSString (DiskCacheInspectorExtras)

-(NSArray *)splitAtCommas
{
    NSRange commaPos;
    NSRange realLastPos = NSMakeRange(0, [self length]);
    NSRange lastPos = realLastPos;
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:1];

    for (commaPos = [self rangeOfString:@", "]; 
	 commaPos.location != NSNotFound; 
	 commaPos = [self rangeOfString:@", " options:NSLiteralSearch range:NSMakeRange(commaPos.location + 1, [self length] - commaPos.location - 1)]) {
	[result addObject:[self substringWithRange:NSMakeRange(lastPos.location, commaPos.location - lastPos.location)]];
	lastPos = realLastPos;
    }
    
    [result addObject:[self substringWithRange:NSMakeRange(lastPos.location, ([self length] - lastPos.location))]];

    return result;
}

-(KeyValuePair *)parseAsKeyValuePairHandleQuotes:(BOOL)handleQuotes
{
    KeyValuePair *pair;
    NSString *trimmed = [self stringByTrimmingWhitespace];
    NSRange equalPos = [trimmed rangeOfString:@"="];

    pair = [KeyValuePair pair];

    if (equalPos.location == NSNotFound) {
	[pair setKey:trimmed];
    } 
    else {
	[pair setKey:[trimmed substringToIndex:equalPos.location]];
	// strip quotes if appropriate
	if ([trimmed length] > (equalPos.location + 1) && [trimmed characterAtIndex:(equalPos.location + 1)] == '"' && handleQuotes) {
	    [pair setValue:[trimmed substringWithRange:NSMakeRange(equalPos.location+2, [trimmed length] - equalPos.location -3)]];
	} 
        else {
	    [pair setValue:[trimmed substringFromIndex:equalPos.location+1]];
	}
    }

    return pair;
}

-(NSString *)stringByTrimmingWhitespace
{
    NSMutableString *trimmed = [[self mutableCopy] autorelease];
    CFStringTrimWhitespace((CFMutableStringRef)trimmed);
    return trimmed;
}

@end

@implementation NSURLResponse (DiskCacheInspectorExtras)

-(NSDate *)dateFromHTTPHeaderValue:(NSString *)string
{
    if (!string || [string length] == 0) {
        return nil;
    }
    
    NSDate *result = nil;
    result = [[NSCalendarDate alloc] initWithString:string calendarFormat:@"%a, %d %b %Y %H:%M:%S %Z"];
    if (!result) {
        result = [[NSCalendarDate alloc] initWithString:string calendarFormat:@"%A, %d-%b-%y %H:%M:%S %Z"];
    }
    if (!result) {
        result = [[NSCalendarDate alloc] initWithString:string calendarFormat:@"%a %b %d %H:%M:%S %Y"];
    }
    return [result autorelease];
}

-(NSDate *)createdDate
{
    if (![self isKindOfClass:[NSHTTPURLResponse class]]) {
        return nil;
    }

    NSDate *result = nil;
    id value = [[(NSHTTPURLResponse *)self allHeaderFields] objectForKey:@"Date"];
    if (value) {
        result = [self dateFromHTTPHeaderValue:value];
    }
    return result;
}

-(NSDate *)expiresDate
{
    NSDate *result = nil;
    id value = [[(NSHTTPURLResponse *)self allHeaderFields] objectForKey:@"Expires"];
    if (value) {
        result = [self dateFromHTTPHeaderValue:value];
    }
    return result;
}

-(NSDate *)lastModifiedDate
{
    NSDate *result = nil;
    id value = [[(NSHTTPURLResponse *)self allHeaderFields] objectForKey:@"Last-Modified"];
    if (value) {
        result = [self dateFromHTTPHeaderValue:value];
    }
    return result;
}

-(NSNumber *)maxAge
{
    if (![self isKindOfClass:[NSHTTPURLResponse class]]) {
        return nil;
    }

    // look at header for cache control information
    NSNumber *result = nil;
    NSString *cacheControlHeader = [[(NSHTTPURLResponse *)self allHeaderFields] objectForKey:@"Cache-Control"];
    if (cacheControlHeader) {
        cacheControlHeader = [cacheControlHeader lowercaseString];
        
        // parse out max-age value
        NSArray *array = [cacheControlHeader splitAtCommas];
        unsigned count = [array count];
        unsigned i;
        for (i = 0; i < count; i++) {
            NSString *item = [array objectAtIndex:i];
            KeyValuePair *kv = [item parseAsKeyValuePairHandleQuotes:YES];
            id key = [kv key];
            id value = [kv value];
            if (value) {
                if ([key isEqualToString:@"max-age"]) {
                    result = [NSNumber numberWithInt:[value intValue]];
                }
            }
        }
    }
    
    return result;
}

-(BOOL)mustRevalidate
{
    if (![self isKindOfClass:[NSHTTPURLResponse class]]) {
        return NO;
    }

    // look at header for cache control information
    BOOL result = NO;
    NSDictionary *headerFields = [(NSHTTPURLResponse *)self allHeaderFields];
    NSString *cacheControlHeader = [headerFields objectForKey:@"Cache-Control"];
    if (cacheControlHeader) {
        cacheControlHeader = [cacheControlHeader lowercaseString];
        
        // search for strings that trigger must-revalidate
        if ([cacheControlHeader rangeOfString:@"no-cache"].location != NSNotFound ||
            [cacheControlHeader rangeOfString:@"must-revalidate"].location != NSNotFound ||
            [cacheControlHeader rangeOfString:@"proxy-revalidate"].location != NSNotFound) {
            result = YES;
        }
        else {
            // look at the legacy Pragma header
            NSString *pragmaHeader = [headerFields objectForKey:@"Pragma"];
            if (pragmaHeader) {
                pragmaHeader = [[pragmaHeader stringByTrimmingWhitespace] lowercaseString];
                if ([pragmaHeader rangeOfString:@"no-cache"].location != NSNotFound ||
                    [pragmaHeader rangeOfString:@"must-revalidate"].location != NSNotFound ||
                    [pragmaHeader rangeOfString:@"proxy-revalidate"].location != NSNotFound) {
                    result = YES;
                }
            }
        }
    }
    
    return result;
}

-(struct FreshnessLifetimeResult)freshness
{
    // If the freshness lifetime is <= the age of the response, then the response has expired.
    // We calculate the freshness lifetime per section 13.2.4 of RFC 2616:
    //
    //     freshness.lifetime = maxAgeValue
    // <or>
    //     freshness.lifetime = expiresTime - createdDate
    // <or>
    //     freshness.lifetime = (createdDate - lastModifiedTime) * 0.10
    // <or>
    //     freshness.lifetime = 0
    //
    // Note that we add an extra check which trumps all of these. If the mustRevalidate
    // flag is set, freshness is zero. 
    struct FreshnessLifetimeResult result = {FreshnessLifetimeDeterminantNone, 0.0};

    if (![self mustRevalidate]) {
        NSNumber *maxAge = [self maxAge];
        if (maxAge) {  
            result.determinant = FreshnessLifetimeDeterminantMustRevalidate;
            result.lifetime = [maxAge doubleValue];
        }
        else {
            NSDate *expiresDate = [self expiresDate];
            if (expiresDate) {
                result.determinant = FreshnessLifetimeDeterminantExpiresDate;
                NSTimeInterval expiresTime = [expiresDate timeIntervalSinceReferenceDate];
                NSTimeInterval createdTime = [[self createdDate] timeIntervalSinceReferenceDate];
                result.lifetime = expiresTime - createdTime;
            }
            else {
                result.determinant = FreshnessLifetimeDeterminantLastModifiedDate;
                NSDate *lastModifiedDate = [self lastModifiedDate];
                if (lastModifiedDate) {
                    NSTimeInterval lastModifiedTime = [lastModifiedDate timeIntervalSinceReferenceDate];
                    NSTimeInterval createdTime = [[self createdDate] timeIntervalSinceReferenceDate];
                    result.lifetime = (createdTime - lastModifiedTime) * 0.10;
                }
            }
        }
    }

    return result;
}

@end
