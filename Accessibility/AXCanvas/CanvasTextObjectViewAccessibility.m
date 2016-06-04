/*
 
 File: CanvasTextObjectViewAccessibility.m
 
 Abstract: Canvas text object view accessibility helper
 
 Version: 1.0
 
 © Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to 
 you by Apple Computer, Inc. ("Apple") in 
 consideration of your agreement to the following 
 terms, and your use, installation, modification 
 or redistribution of this Apple software 
 constitutes acceptance of these terms.  If you do 
 not agree with these terms, please do not use, 
 install, modify or redistribute this Apple 
 software.
 
 In consideration of your agreement to abide by 
 the following terms, and subject to these terms, 
 Apple grants you a personal, non-exclusive 
 license, under Apple's copyrights in this 
 original Apple software (the "Apple Software"), 
 to use, reproduce, modify and redistribute the 
 Apple Software, with or without modifications, in 
 source and/or binary forms; provided that if you 
 redistribute the Apple Software in its entirety 
 and without modifications, you must retain this 
 notice and the following text and disclaimers in 
 all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or 
 logos of Apple Computer, Inc. may be used to 
 endorse or promote products derived from the 
 Apple Software without specific prior written 
 permission from Apple.  Except as expressly 
 stated in this notice, no other rights or 
 licenses, express or implied, are granted by 
 Apple herein, including but not limited to any 
 patent rights that may be infringed by your 
 derivative works or by other works in which the 
 Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS 
 IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
 THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
 OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
 SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
 THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
 UNDER THEORY OF CONTRACT, TORT (INCLUDING 
 NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
 IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.
 
 */ 


#import "CanvasTextObjectViewAccessibility.h"
#import "CanvasTextObject.h"
#import <ApplicationServices/ApplicationServices.h>

// Provides accessibility implementation for CanvasTextObjectViews,
// building on CanvasObjectView. 
@implementation CanvasTextObjectView (CanvasAccessibility)

#pragma mark AccessibleView methods
- (BOOL)accessibilityIsIgnored {
	return NO;
}

- (NSArray *)accessibilityAttributeNames {
	static NSArray *attributes = nil;
	if(attributes == nil) {
		attributes = [[[super accessibilityAttributeNames] arrayByAddingObjectsFromArray:[NSArray arrayWithObjects:
			NSAccessibilityValueAttribute,
			NSAccessibilityNumberOfCharactersAttribute,
			NSAccessibilityVisibleCharacterRangeAttribute,
			NSAccessibilitySelectedTextAttribute,
			NSAccessibilitySelectedTextRangeAttribute,
			NSAccessibilityDescriptionAttribute,
			nil]] retain];
	}
	
	return attributes;
}

- (NSArray *)accessibilityParameterizedAttributeNames {
	static NSArray *attributes = nil;
	if(attributes == nil) {
		attributes = [[NSArray alloc] initWithObjects:
			NSAccessibilityLineForIndexParameterizedAttribute,
			NSAccessibilityRangeForLineParameterizedAttribute,
			NSAccessibilityRTFForRangeParameterizedAttribute,
			NSAccessibilityStringForRangeParameterizedAttribute,
			NSAccessibilityRangeForPositionParameterizedAttribute,
			NSAccessibilityRangeForIndexParameterizedAttribute,
			NSAccessibilityBoundsForRangeParameterizedAttribute,
			NSAccessibilityAttributedStringForRangeParameterizedAttribute,
			nil];
	}
	
	return attributes;
}

#pragma mark Accessibility attribute accessors
- (NSString *)caxRoleAttribute {
	// While the receiver is really an NSAccessibilityUnknownRole, we need to make
	// it something text-related to have VoiceOver pay any attention to it.
	return NSAccessibilityStaticTextRole;
}

- (BOOL)caxIsRoleAttributeSettable {
	return NO;
}

- (NSString *)caxRoleDescriptionAttribute {
	return NSLocalizedStringFromTable(@"canvas text object", @"Accessibility", @"CanvasTextObjectView role description");
}

- (BOOL)caxIsRoleDescriptionAttributeSettable {
	return NO;
}

- (NSString *)caxValueAttribute {
	return [(CanvasTextObject *)representedObject textContent];
}

- (BOOL)caxIsValueAttributeSettable {
	return NO;
}

- (NSString *)caxSelectedTextAttribute {
	return @"";
}

- (BOOL)caxIsSelectedTextAttributeSettable {
	return NO;
}

- (NSValue *)caxSelectedTextRangeAttribute {
	// CanvasTextObjects can't have any selected text unless they are being
	// edited, which is taken care of by the accessibility query method stubs
	// above.
	return [NSValue valueWithRange:NSMakeRange(0,0)];
}

- (BOOL)caxIsSelectedTextRangeAttributeSettable {
	return NO;
}

- (NSNumber *)caxNumberOfCharactersAttribute {
	return [NSNumber numberWithInt:[[(CanvasTextObject *)representedObject textContent] length]];
}

- (BOOL)caxIsNumberOfCharactersAttributeSettable {
	return NO;
}

- (NSValue *)caxVisibleCharacterRangeAttribute {
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;

	NSLayoutManager *layoutManager = [CanvasTextObjectView sharedLayoutManager];
	NSTextContainer *textContainer = [[layoutManager textContainers] objectAtIndex:0];
	
	[textContainer setContainerSize:NSInsetRect([self bounds], 1, 1).size];
	
	NSTextStorage *storage = [representedTextObject storage];
	[storage addLayoutManager:layoutManager];
	unsigned unlaidIndex = [layoutManager firstUnlaidCharacterIndex];
	[storage removeLayoutManager:layoutManager];
	
	// Since layout always starts out at character 0, the index of the first
	// unlaid character is the length of the displayed range.
	return [NSValue valueWithRange:NSMakeRange(0, unlaidIndex)];
}

- (BOOL)caxIsVisibleCharacterRangeAttributeSettable {
	return NO;
}

- (NSNumber *)caxLineForIndexAttributeForParameter:(NSNumber *)indexValue {
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;
	
	// Range check
	if(([indexValue intValue] < 0) || ([indexValue intValue]) >= [[representedTextObject storage] length]) {
		NSAccessibilityRaiseBadArgumentException(self, NSAccessibilityLineForIndexParameterizedAttribute, indexValue);
		return;
	}
		

	NSLayoutManager *layoutManager = [CanvasTextObjectView sharedLayoutManager];
	NSTextContainer *textContainer = [[layoutManager textContainers] objectAtIndex:0];
	
	[textContainer setContainerSize:NSInsetRect([self bounds], 1, 1).size];
	
	NSTextStorage *storage = [representedTextObject storage];
	[storage addLayoutManager:layoutManager];
	
	// From Cocoa: Text Layout, Counting Lines of Text, Listing 2
	// Sequentially ask for line fragments from the layout manager and count them.
	unsigned targetIndex = [indexValue unsignedIntValue];
	unsigned lineNumber, index, numberOfGlyphs = [layoutManager numberOfGlyphs];
	NSRange lineRange;
	for(lineNumber = 0, index = 0; (index <= targetIndex) && (index < numberOfGlyphs); lineNumber++) {
		[layoutManager lineFragmentRectForGlyphAtIndex:index effectiveRange:&lineRange];
		index = NSMaxRange(lineRange);
	}
	
	[storage removeLayoutManager:layoutManager];	
	
	// Subtract one line, as the incrementing in the for loop advances past our line.
	return [NSNumber numberWithUnsignedInt:--lineNumber];
}

- (NSValue *)caxRangeForLineAttributeForParameter:(NSNumber *)lineValue {
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;

	// Range check
	if([lineValue intValue] < 0) {
		NSAccessibilityRaiseBadArgumentException(self, NSAccessibilityRangeForLineParameterizedAttribute, lineValue);
		return nil;
	}
	
	NSLayoutManager *layoutManager = [CanvasTextObjectView sharedLayoutManager];
	NSTextContainer *textContainer = [[layoutManager textContainers] objectAtIndex:0];
	
	[textContainer setContainerSize:NSInsetRect([self bounds], 1, 1).size];
	
	NSTextStorage *storage = [representedTextObject storage];
	[storage addLayoutManager:layoutManager];
	
	// From Cocoa: Text Layout, Counting Lines of Text, Listing 2
	// Sequentially ask for line fragments from the layout manager,
	// keeping track of when the line containing the target line is found and
	// storing its range.
	unsigned targetLine = [lineValue unsignedIntValue];
	unsigned lineNumber, index, numberOfGlyphs = [layoutManager numberOfGlyphs];
	NSRange lineRange;
	for(lineNumber = 0, index = 0; (lineNumber <= targetLine) && (index < numberOfGlyphs); lineNumber++) {
		[layoutManager lineFragmentRectForGlyphAtIndex:index effectiveRange:&lineRange];
		index = NSMaxRange(lineRange);
	}
	
	// Range check
	if(targetLine > lineNumber) {
		// Line overrun.
		NSAccessibilityRaiseBadArgumentException(self, NSAccessibilityRangeForLineParameterizedAttribute, lineValue);
		return nil;
	}
	
	[storage removeLayoutManager:layoutManager];
	
	return [NSValue valueWithRange:lineRange];
}

- (NSString *)caxStringForRangeAttributeForParameter:(NSValue *)rangeValue {
	return [[(CanvasTextObject *)representedObject textContent] substringWithRange:[rangeValue rangeValue]];
}

- (NSData *)caxRTFForRangeAttributeForParameter:(NSValue *)rangeValue {
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;
	NSTextStorage *storage = [representedTextObject storage];

	// Type check
	if(strcmp([rangeValue objCType], @encode(NSRange)) != 0) {
		NSAccessibilityRaiseBadArgumentException(self, NSAccessibilityRTFForRangeParameterizedAttribute, rangeValue);
		return nil;
	}
	
	// Range check
	if(([rangeValue rangeValue].location < 0) || (NSMaxRange([rangeValue rangeValue]) > [storage length])) {
		NSAccessibilityRaiseBadArgumentException(self, NSAccessibilityRTFForRangeParameterizedAttribute, rangeValue);
		return nil;
	}
	
	return [storage RTFFromRange:[rangeValue rangeValue] documentAttributes:nil];
}

- (NSValue *)caxRangeForIndexAttributeForParameter:(NSNumber *)indexValue {
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;
	
	// Range check
	if(([indexValue intValue] < 0) || ([indexValue intValue] >= [[representedTextObject storage] length])) {
		NSAccessibilityRaiseBadArgumentException(self, NSAccessibilityRangeForIndexParameterizedAttribute, indexValue);
		return nil;
	}
	
	return [NSValue valueWithRange:NSMakeRange([indexValue intValue], 1)];
}

- (NSValue *)caxBoundsForRangeAttributeForParameter:(NSValue *)rangeValue {
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;
	NSTextStorage *storage = [representedTextObject storage];
	
	// Type check
	if(strcmp([rangeValue objCType], @encode(NSRange)) != 0) {
		NSAccessibilityRaiseBadArgumentException(self, NSAccessibilityBoundsForRangeParameterizedAttribute, rangeValue);
		return nil;
	}
	
	// Range check
	if(([rangeValue rangeValue].location < 0) || (NSMaxRange([rangeValue rangeValue]) > [storage length])) {
		NSAccessibilityRaiseBadArgumentException(self, NSAccessibilityBoundsForRangeParameterizedAttribute, rangeValue);
		return nil;
	}
	
	NSLayoutManager *layoutManager = [CanvasTextObjectView sharedLayoutManager];
	NSTextContainer *textContainer = [[layoutManager textContainers] objectAtIndex:0];
	
	[textContainer setContainerSize:NSInsetRect([self bounds], 1, 1).size];
	
	[storage addLayoutManager:layoutManager];
	
	// Retrieve the glyph range for the specified character range from the layout manager and get its bounding rect.
	NSRange glyphRange = [layoutManager glyphRangeForCharacterRange:[rangeValue rangeValue] actualCharacterRange:nil];
	NSRect boundingRect = [layoutManager boundingRectForGlyphRange:glyphRange inTextContainer:textContainer]; 
	
	[storage removeLayoutManager:layoutManager];	
	
	// Offset the rect since the text container is inset from the view edge by one pixel, and 
	// convert to screen coordinates.
	return [NSValue valueWithRect:[self convertRectToScreen:NSOffsetRect(boundingRect, 1, 1)]];
}

- (NSAttributedString *)caxAttributedStringForRangeAttributeForParameter:(NSValue *)rangeValue {
	// Since CanvasTextObjects only have a single attribute run for the entire content,
	// the input range can be ignored.

	// The Accessibility API expects the text attribute keys present in NSAccessibility.h to be present
	// in the attributes dictionaries of the returned attributed strings instead of the usual attributes
	// we see in Cocoa attributed strings. Therefore, a bit of conversion is necessary. More complex
	// views that have multiple style runs will need to break up the runs and convert each run of attributes
	// as appropriate.
	
	CanvasTextObject *representedTextObject = (CanvasTextObject *)representedObject;
	// Type check
	if(strcmp([rangeValue objCType], @encode(NSRange)) != 0) {
		NSAccessibilityRaiseBadArgumentException(self, NSAccessibilityAttributedStringForRangeParameterizedAttribute, rangeValue);
		return nil;
	}
	
	// Range check
	if(([rangeValue rangeValue].location < 0) || (NSMaxRange([rangeValue rangeValue]) > [[representedTextObject storage] length])) {
		NSAccessibilityRaiseBadArgumentException(self, NSAccessibilityAttributedStringForRangeParameterizedAttribute, rangeValue);
		return nil;
	}
	
	NSDictionary *textAttributes = [representedTextObject textAttributes];
	
	// Create a new attributed string for us to work with and find the range which it spans.
	NSMutableAttributedString *result = [[NSMutableAttributedString alloc] initWithString:[[representedTextObject textContent] substringWithRange:[rangeValue rangeValue]]];
	NSRange attrRange = NSMakeRange(0, [result length]);
	id obj;
	
	[result beginEditing];
	
	// Foreground color
	CGColorRef color = CAXCGColorFromNSColor([textAttributes objectForKey:NSForegroundColorAttributeName]);
	[result addAttribute:NSAccessibilityForegroundColorTextAttribute value:(id)color range:attrRange];
	CGColorRelease(color);
	
	// Background color
	if([representedObject hasFill]) {
		color = CAXCGColorFromNSColor([representedObject fillColor]);
		[result addAttribute:NSAccessibilityBackgroundColorTextAttribute value:(id)color range:attrRange];
		CGColorRelease(color);
	}

	// Underline color
	if(obj = [textAttributes objectForKey:NSUnderlineColorAttributeName]) {
		color = CAXCGColorFromNSColor(obj);
		[result addAttribute:NSAccessibilityUnderlineColorTextAttribute value:(id)color range:attrRange];
		CGColorRelease(color);
	}

	// Strikethrough color
	if(obj = [textAttributes objectForKey:NSStrikethroughColorAttributeName]) {
		color = CAXCGColorFromNSColor(obj);
		[result addAttribute:NSAccessibilityStrikethroughColorTextAttribute value:(id)color range:attrRange];
		CGColorRelease(color);
	}

	// Underline
	if(obj = [textAttributes objectForKey:NSUnderlineStyleAttributeName])
		[result addAttribute:NSAccessibilityUnderlineTextAttribute value:obj range:attrRange];
	
	// Superscript
	if(obj = [textAttributes objectForKey:NSSuperscriptAttributeName])
		[result addAttribute:NSAccessibilitySuperscriptTextAttribute value:obj range:attrRange];
	
	// Strikethrough
	// Accessibility does not currently handle strikethrough styles as it does underline styles, so
	// it is simply reported if it is present
	if(obj = [textAttributes objectForKey:NSStrikethroughStyleAttributeName])
		[result addAttribute:NSAccessibilityStrikethroughTextAttribute value:[NSNumber numberWithBool:([obj intValue] != 0)] range:attrRange];
	
	// Shadow
	// Simply report that a shadow is present.
	if(obj = [textAttributes objectForKey:NSShadowAttributeName])
		[result addAttribute:NSAccessibilityShadowTextAttribute value:[NSNumber numberWithBool:YES] range:attrRange];
	
	// Font
	// The font dictionary also uses custom keys, as specified in NSAccessibility.h.
	// The conversion here is fairly simple to do.
	NSFont *font = [textAttributes objectForKey:NSFontAttributeName];
	NSDictionary *fontDictionary = [NSDictionary dictionaryWithObjectsAndKeys:
		[font fontName], NSAccessibilityFontNameKey,
		[font familyName], NSAccessibilityFontFamilyKey,
		[font displayName], NSAccessibilityVisibleNameKey,
		[NSNumber numberWithFloat:[font pointSize]], NSAccessibilityFontSizeKey, nil];
	[result addAttribute:NSAccessibilityFontTextAttribute value:fontDictionary range:attrRange];

	// Finalize the attributed string.
	[result endEditing];
	return [result autorelease];
	
}

#pragma mark Conversion functions
static CGColorRef CAXCGColorFromNSColor(NSColor *color) {
	// Converts NSColors to CGColors in the current device color space. Returned
	// CGColors must be released when they are done.
	NSColor *rgbColor = [color colorUsingColorSpaceName:NSDeviceRGBColorSpace];
	if(!rgbColor)
		rgbColor = [NSColor blackColor];
	
	float rgbComponents[4];
	[rgbColor getRed:&rgbComponents[0] green:&rgbComponents[1] blue:&rgbComponents[2] alpha:&rgbComponents[3]];
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	CGColorRef cgColor = CGColorCreate(colorSpace, rgbComponents);
	CGColorSpaceRelease(colorSpace);
	
	return cgColor;
}

@end
