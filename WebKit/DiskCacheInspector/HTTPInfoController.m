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
/* HTTPInfoController.m */

#import "HTTPInfoController.h"

#import "AppController.h"
#import "DiskCacheObject.h"

#import <Foundation/NSURLCache.h>
#import <Foundation/NSURLResponse.h>

static HTTPInfoController *sharedController;

@implementation HTTPInfoController

-(NSString *)windowNibName
{
    return @"HTTPInfo";
}

+(HTTPInfoController *)sharedController
{
    if (sharedController == nil) {
        sharedController = [[HTTPInfoController alloc] init];
    }

    return sharedController;
}

-(void)handleDiskCacheObjectDidChange:(DiskCacheObject *)diskCacheObject
{
    [tableView reloadData];
}

-(NSHTTPURLResponse *)currentHTTPResponse
{
    DiskCacheObject *currentDiskCacheObject = [[AppController sharedAppController] currentDiskCacheObject];
    NSCachedURLResponse *cachedResponse = [currentDiskCacheObject cachedResponse];
    NSURLResponse *response = [cachedResponse response];
    
    if (![response isKindOfClass:[NSHTTPURLResponse class]]) {
        return nil;
    }
    return (NSHTTPURLResponse *)response;
}

-(id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
    NSHTTPURLResponse *response = [self currentHTTPResponse];
    if (!response) {
        return nil;
    }

    id result = nil;
    NSDictionary *allHeaderFields = [response allHeaderFields];
    NSArray *allKeys = [allHeaderFields allKeys];
    NSArray *sortedKeys = [allKeys sortedArrayUsingSelector:@selector(compare:)];
    NSString *key = [sortedKeys objectAtIndex:rowIndex];
    NSString *identifier = [aTableColumn identifier];
    
    if ([identifier isEqualToString:@"Field"]) {
        result = key;
    } 
    else {
        // identifier is Value
        result = [allHeaderFields objectForKey:key];
    }
    
    return result;
}


-(int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    NSHTTPURLResponse *response = [self currentHTTPResponse];
    if (!response) {
        return 0;
    }
    return [[response allHeaderFields] count];
}

@end
