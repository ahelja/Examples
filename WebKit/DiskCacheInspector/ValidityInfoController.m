/*
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
#import "ValidityInfoController.h"
#import "AppController.h"
#import "DiskCacheObject.h"
#import "NSExtras.h"

#import <Foundation/NSURLCache.h>
#import <Foundation/NSURLResponse.h>

static ValidityInfoController *sharedController;

@implementation ValidityInfoController

-(NSString *)windowNibName
{
    return @"ValidityInfo";
}

+(ValidityInfoController *)sharedController
{
    if (sharedController == nil) {
        sharedController = [[ValidityInfoController alloc] init];
    }

    return sharedController;
}

-(void)handleDiskCacheObjectDidChange:(DiskCacheObject *)diskCacheObject
{
    NSCachedURLResponse *cachedResponse = [diskCacheObject cachedResponse];
    NSURLResponse *response = [cachedResponse response];

    NSTimeInterval responseAge = CFAbsoluteTimeGetCurrent() - [[response createdDate] timeIntervalSinceReferenceDate];

    // Created date
    NSDate *createdDate = [response createdDate];
    if (createdDate) {
        [createdLabel setTextColor:[NSColor blackColor]];
        [createdLabel setObjectValue:createdDate];
    }
    else {
        [createdLabel setTextColor:[NSColor grayColor]];
        [createdLabel setStringValue:@"None"];
    }

    // Last-Modified date
    NSDate *lastModifiedDate = [response lastModifiedDate];
    if (createdDate) {
        [lastModifiedLabel setTextColor:[NSColor blackColor]];
        [lastModifiedLabel setObjectValue:lastModifiedDate];
    }
    else {
        [lastModifiedLabel setTextColor:[NSColor grayColor]];
        [lastModifiedLabel setStringValue:@"None"];
    }

    // Expires date
    NSDate *expiresDate = [response expiresDate];
    if (expiresDate) {
        [expiresLabel setTextColor:[NSColor blackColor]];
        [expiresLabel setObjectValue:expiresDate];
    }
    else {
        [expiresLabel setTextColor:[NSColor grayColor]];
        [expiresLabel setStringValue:@"None"];
    } 
    
    // Max-Age
    NSNumber *maxAge = [response maxAge];
    if (maxAge) {
        [maxAgeLabel setTextColor:[NSColor blackColor]];
        [maxAgeLabel setObjectValue:maxAge];
    }
    else {
        [maxAgeLabel setTextColor:[NSColor grayColor]];
        [maxAgeLabel setStringValue:@"None"];
    } 

    // Age
    [ageLabel setStringValue:[NSString stringWithFormat:@"%.0f", responseAge]];

    // Must revalidate
    [mustRevalidateLabel setStringValue:[response mustRevalidate] ? @"No" : @"Yes"];

    // Freshness
    struct FreshnessLifetimeResult freshness = [response freshness];
    [freshnessLifetimeLabel setStringValue:[NSString stringWithFormat:@"%.0f", freshness.lifetime]];
    NSString *determinant = nil;
    switch (freshness.determinant) {
        case FreshnessLifetimeDeterminantNone:
            determinant = @"None";
            break;
        case FreshnessLifetimeDeterminantMustRevalidate:
            determinant = @"Must Revalidate";
            break;
        case FreshnessLifetimeDeterminantExpiresDate:
            determinant = @"Expiration Date";
            break;
        case FreshnessLifetimeDeterminantLastModifiedDate:
            determinant = @"Last Modified Date";
            break;
    }
    [freshnessDeterminantLabel setStringValue:determinant];
    
    // Has expired
    [hasExpiredLabel setStringValue:(freshness.lifetime > responseAge) ? @"No" : @"Yes"]; 
}

@end
