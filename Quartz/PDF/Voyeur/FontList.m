/* Voyeur - FontList.h
 *
 * Author: Joel Kraut
 * Created 4 August 2004
 *
 * Copyright (c) 2003-2005 Apple Computer, Inc.
 * All rights reserved.
 */

/* IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.

 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE. */

#import "FontList.h"
#import "VoyeurAppKitExtras.h"

static void
parseFont(const char *key, CGPDFObjectRef object, void *info)
{
    bool isEmbedded;
    const char *name;
    VoyeurFont *font;
    NSMutableDictionary *fonts;
    CGPDFDictionaryRef dict, descriptor;
    NSString *baseFont, *fontType, *encoding;

    fonts = info;

    if (!CGPDFObjectGetValue(object, kCGPDFObjectTypeDictionary, &dict))
	return;

    baseFont = @"<< none >>";
    if (CGPDFDictionaryGetName(dict, "BaseFont", &name))
	baseFont = [NSString stringWithCString:name];
    fontType = @"<< unknown >>";
    if (CGPDFDictionaryGetName(dict, "Subtype", &name))
	fontType = [NSString stringWithCString: name];
	
    isEmbedded = false;
    if (CGPDFDictionaryGetDictionary(dict, "FontDescriptor", &descriptor)) {
	if (CGPDFDictionaryGetObject(descriptor, "FontFile", &object)
	    || CGPDFDictionaryGetObject(descriptor, "FontFile2", &object)
	    || CGPDFDictionaryGetObject(descriptor, "FontFile3", &object)) {
	    isEmbedded = true;
	}
    }

    encoding = @"<< font-specific >>";
    if (CGPDFDictionaryGetName(dict, "Encoding", &name))
	encoding = [NSString stringWithCString:name];

    font = [[VoyeurFont alloc] initWithBaseFont:baseFont type:fontType
			       encoding:encoding isEmbedded:isEmbedded];
    [fonts setObject:font forKey:baseFont];
    [font release];
}

@implementation FontList

- (id)init
{
    self = [super init];
    if (self == nil)
	return nil;

    fonts = [[NSMutableDictionary alloc] init];

    return self;
}

- (void)dealloc
{
    [fonts release];
    [super dealloc];
}

- (void)addFontsFromPage:(CGPDFPageRef)page
{
    CGPDFDictionaryRef dict, resources, pageFonts;

    dict = CGPDFPageGetDictionary(page);
    if (!CGPDFDictionaryGetDictionary(dict, "Resources", &resources))
	return;
    if (!CGPDFDictionaryGetDictionary(resources, "Font", &pageFonts))
	return;
    CGPDFDictionaryApplyFunction(pageFonts, &parseFont, fonts);
}

- (NSMutableAttributedString *)info
{
    int k, count;
    NSString *key;
    NSArray *array;
    NSAttributedString *fontString;
    NSMutableAttributedString *string;

    string = [[NSMutableAttributedString alloc] init];

    array = [[fonts allKeys] sortedArrayUsingSelector:@selector(compare:)];
    count = [array count];
    for (k = 0; k < count; k++) {
	key = [array objectAtIndex:k];
	fontString = [(VoyeurFont *)[fonts objectForKey:key] info];
	[string appendAttributedString: fontString];
	[string appendString:@"\n"];
    }
    return [string autorelease];
}

@end
