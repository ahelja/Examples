/*
 
 File: AccessibleViewKVCUtilities
 
 Abstract: KVC accessibility helper functions
 
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


#import "AccessibleViewKVCUtilities.h"

// These functions form the core of accessibility support in AXCanvas. They are used by
// AccessibleView and AccessibleProxy on top of the NSAccessibility protocol
// to implement key value coding for accessibility attributes.
// For more information about the NSAccessibility protocol, see NSAccessibility.h and
// the following URL:
//  file:///Developer/ADC%20Reference%20Library/documentation/Cocoa/Reference/ApplicationKit/ObjC_classic/Protocols/NSAccessibility.html
// 
// To create an object representing an AXUIElement, create a subclass of either
// AccessibleView or AccessibleProxy, depending on whether you are adding accessibility
// to an existing view or creating a proxy UI element to represent elements which don't
// exist in the view hierarchy. If your view class is not a direct subclass of NSView,
// you may need to copy the implementation of AccessibleView into your own class.
//
// For each accessibility attribute, you should implement two methods:
//  -(id)cax(AttributeName)Attribute, and
//  -(BOOL)caxIs(AttributeBane)Settable.
// where (AttributeName) is the attribute name without the leading AX. For example, when
// implementing accessors for AXDescription, you would implement
//  -caxDescriptionAttribute and -caxIsDescriptionAttributeSettable.
//
// Parameterized attributes follow a similar convention, but their getter takes the form
// - (id)cax(AttributeName)AttributeForParameter:(id)parameter
//
// If the attribute is settable, a setter should be implemented with the form
// - (void)caxSet(AttributeName)Attribute:(id)value
//
// See CanvasObjectViewAccessibility.m, CanvasTextObjectViewAccessibility.m and
// CanvasProxyTabView.m in this project for example implementations.

// Uncomment the following line to receive a full trace of accessibility attribute
// accesses.
//#define CANVAS_AX_DEBUG

#ifdef CANVAS_AX_DEBUG
#define CAXLog(...) NSLog(__VA_ARGS__)
#else
#define CAXLog(...) 
#endif

// Returns a string with the AX prefix removed for getter/setter naming
NSString* CAXAttributeWithoutAXPrefix(NSString *attribute) {
	return [attribute hasPrefix:@"AX"] ? [attribute substringFromIndex:2] : attribute;
}

// Returns whether an AXElement supports a specified attribute
BOOL CAXIsSupportedAttribute(id element, NSString *attribute) {
	return [[element accessibilityAttributeNames] indexOfObject:attribute] != NSNotFound;
}

// Returns whether an AXElement supports a specified parameterized attribute
BOOL CAXIsSupportedParameterizedAttribute(id element, NSString *attribute) {
	return [[element accessibilityParameterizedAttributeNames] indexOfObject:attribute] != NSNotFound;
}

// Helper for CAXIsSelfSettableAttribute
BOOL CAXIsPossiblySelfSettable(id element, NSString *attribute) {
	SEL getter = CAXAttributeSettableGetter(attribute);
	return [element respondsToSelector:getter];
}

// Returns whether an object can set a specified attribute on its own
BOOL CAXIsSelfSettableAttribute(id element, NSString *attribute) {
	SEL getter = CAXAttributeSettableGetter(attribute);
	if(CAXIsPossiblySelfSettable(element, attribute)) {
		return [element performSelector:getter] != nil;		
	} else {
		return NO;
	}
}

// Returns a selector for accessing the specified attribute
SEL CAXAttributeGetter(NSString *attribute) {
	return NSSelectorFromString([NSString stringWithFormat:@"cax%@Attribute", CAXAttributeWithoutAXPrefix(attribute)]);
}

// Returns whether an object has its own implementation of the attribute getter.
BOOL CAXIsSelfGettableAttribute(id element, NSString *attribute) {
	SEL getter = CAXAttributeGetter(attribute);
	return [element respondsToSelector:getter];
}

// Returns a selector for accessing the specified parameterized attribute
SEL CAXParameterizedAttributeGetter(NSString *attribute) {
	return NSSelectorFromString([NSString stringWithFormat:@"cax%@AttributeForParameter:", CAXAttributeWithoutAXPrefix(attribute)]);
}

// Returns whether an object has its own implementation of the parameterized attribute getter.
BOOL CAXIsSelfGettableParameterizedAttribute(id element, NSString *attribute) {
	SEL getter = CAXParameterizedAttributeGetter(attribute);
	return [element respondsToSelector:getter];
}

// Returns a selector for checking if the specified attribute is settable
SEL CAXAttributeSettableGetter(NSString *attribute) {
	return NSSelectorFromString([NSString stringWithFormat:@"caxIs%@AttributeSettable", CAXAttributeWithoutAXPrefix(attribute)]);
}

// Return a selector for setting the specified attribute
SEL CAXAttributeSetter(NSString *attribute) {
	return NSSelectorFromString([NSString stringWithFormat:@"caxSet%@Attribute:", CAXAttributeWithoutAXPrefix(attribute)]);
}

// Returns the value of the specified attribute
id CAXAttributeValue(id element, NSString *attribute) {
	CAXLog(@"Getting %@ from %@", attribute, element);
	id result = nil;
	if(CAXIsSupportedAttribute(element, attribute)) {
		SEL getter = CAXAttributeGetter(attribute);
		if([element respondsToSelector:getter]) {
			result = [element performSelector:getter];
		}
	} else
		result = nil;
	
	CAXLog(@"- Value: %@", result);
	return result;
}

// Returns the value of the specified parameterized attribute
id CAXParameterizedAttributeValue(id element, NSString *attribute, id parameter) {
	CAXLog(@"Getting parameter %@=%@ from %@", attribute, parameter, element);
	id result = nil;
	if(CAXIsSupportedParameterizedAttribute(element, attribute)) {
		SEL getter = CAXParameterizedAttributeGetter(attribute);
		if([element respondsToSelector:getter]) {
			result = [element performSelector:getter withObject:parameter];
		}
	} else
		result = nil;
	CAXLog(@"- Value: %@", result);
	return result;
}

// Sets the value of the specified attribute
void CAXSetAttributeValue(id element, NSString *attribute, id value) {
	CAXLog(@"Setting %@ to %@ on %@", attribute, value, element);
	if([element accessibilityIsAttributeSettable:attribute]) {
		SEL setter = CAXAttributeSetter(attribute);
		if([element respondsToSelector:setter]) {
			[element performSelector:setter withObject:value];
		}
	}
}
