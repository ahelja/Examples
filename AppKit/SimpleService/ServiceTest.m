/*
 Services menu example. This file provides two services; one with no return value
 (open a file), and the other that takes a string and returns a capitalized version.

 Author: Ali Ozer
 Written Nov '96.

 Copyright (c) 1996-2004, Apple Computer, Inc., all rights reserved.
*/
/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple�s copyrights in 
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

#import "ServiceTest.h"

@implementation ServiceTest

/* This is an example of a service which doesn't return a value...
*/
- (void)doOpenFileService:(NSPasteboard *)pboard
 userData:(NSString *)data
 error:(NSString **)error
 {
    NSString *pboardString;
    NSArray *types;

    types = [pboard types];

    if (![types containsObject:NSStringPboardType] || !(pboardString = [pboard stringForType:NSStringPboardType])) {
        *error = NSLocalizedString(@"Error: Pasteboard doesn't contain a string.",
                   @"Pasteboard couldn't give string.");
        return;
    }


    if (![[NSWorkspace sharedWorkspace] openFile:pboardString]) {
        *error = [NSString stringWithFormat:NSLocalizedString(@"Error: Couldn't open file %@.",
                   @"Couldn't perform service operation."), pboardString];
        return;
    }

    return;
}

/* This service returns a value; simply the capitalized version of the provided string...
*/
- (void)doCapitalizeService:(NSPasteboard *)pboard
 userData:(NSString *)data
 error:(NSString **)error
 {

    NSString *pboardString;
    NSString *newString;
    NSArray *types;

    types = [pboard types];

    if (![types containsObject:NSStringPboardType] || !(pboardString = [pboard stringForType:NSStringPboardType])) {
        *error = NSLocalizedString(@"Error: Pasteboard doesn't contain a string.",
                   @"Pasteboard couldn't give string.");
        return;
    }


    newString = [pboardString capitalizedString];

    if (!newString) {
        *error = NSLocalizedString(@"Error: Couldn't capitalize string %@.",
                   @"Couldn't perform service operation.");
        return;
    }

    /* We now return the capitalized string... */
    types = [NSArray arrayWithObject:NSStringPboardType];
    [pboard declareTypes:types owner:nil];
    [pboard setString:newString forType:NSStringPboardType];

    return;
}

@end
