/*
 AdjustGamma.m
 AdjustGamma
 Cocoa Automator action showing the use of the Core Image framework in Automator actions.

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

#import "AdjustGamma.h"

@implementation AdjustGamma

- (NSData *)tiffDataForImage:(CIImage *)image fromRect:(CGRect)extent
{
    CFMutableDataRef data = CFDataCreateMutable(kCFAllocatorDefault, 0);
    CGImageDestinationRef ref = CGImageDestinationCreateWithData(data, (CFStringRef)@"public.tiff", 1, NULL);
    if (ref == NULL)
	{
		CFRelease(data);
		CFRelease(ref);
		return nil;
	}
    
	// make a CGImageRef
    CGImageRef iref = [context createCGImage:image fromRect:extent];
    
	// add image to the ImageIO destination (specify the image we want to save)
    CGImageDestinationAddImage(ref, iref, NULL);
    
	// save the image to the TIFF format as data
    if (!CGImageDestinationFinalize(ref))
	{
		CFRelease(data);
		CFRelease(ref);
		return nil;
	}
	
    CFRelease(ref);
	
    return [(NSData *)data autorelease];
}

- (id)runWithInput:(id)input fromAction:(AMAction *)anAction error:(NSDictionary **)errorInfo
{
	id output = input;
	
	if (![input isKindOfClass:[NSArray class]])
	{
		input = [NSArray arrayWithObject:input];
	}
	
	// get the graphics context
	if (context == nil)
	{
		context = [CIContext contextWithCGContext: [[NSGraphicsContext currentContext] graphicsPort] options: nil];
		[context retain];
	}
	
	// create and setup the filter
	CIFilter *gammaAdjust = [CIFilter filterWithName:@"CIGammaAdjust"];
	[gammaAdjust setDefaults]; 
	[gammaAdjust setValue: [NSNumber numberWithFloat:[[[self parameters] objectForKey:@"power"] floatValue]]  forKey: @"inputPower"];
	
	NSEnumerator *enumerator = [input objectEnumerator];
	NSString *path;
	
	while (path = [enumerator nextObject])
	{
		// create a CIImage from the file
		CIImage *image = [CIImage imageWithContentsOfURL:[NSURL fileURLWithPath:path]];
		if (image)
		{
			[gammaAdjust setValue:image forKey:@"inputImage"];
			
			CIImage *result = [gammaAdjust valueForKey:@"outputImage"]; 
			if (result)
			{
				// save the changes to the path
				NSData *data = [self tiffDataForImage:result fromRect:[image extent]];
				if (data)
				{
					if (![data writeToFile:path atomically:YES])
					{
						NSString *errorString = NSLocalizedString(@"Adjust Image Gamma could not create output file.",@"Couldn't write data to file");
						*errorInfo = [NSDictionary dictionaryWithObjectsAndKeys: [errorString autorelease], NSAppleScriptErrorMessage, nil];
					}
				}
			}
		}
	}
	
	[context release];
	context = nil;
	
	return output;
}

@end
