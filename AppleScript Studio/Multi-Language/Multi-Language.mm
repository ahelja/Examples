/* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#import "Multi-Language.h"
#import "Objective-C.h"
#import "C++.h"

extern "C"
{
	#import "C.h"
}


/* This file adds a category on the NSApplication class which 'call method' can then use to easily call the Objective-C methods defined here. The methods will each their respective langes to return the name of their language. Since this example uses C++ classes directly, it's necessary to give the file a .mm extension so that the Objective-C++ compiler will be used instead of the Objective-C compiler. This way we don't have to provide "C" functions to get access to the C++ classes. */

// NSApplication
// =============

@implementation NSApplication (ASKAMultiLanguage)

- (NSString *)nameForObjCLanguage
{
	// Create a Objective-C object (class ObjCLanguage) which is defined in "Objective-C.h" and implemented in "Objective-C.m".
	ObjCLanguage *language = [[[ObjCLanguage alloc] init] autorelease];
	
	// Ask the language object for it's name and return it.
	return [language name];
}

- (NSString *)nameForCLanguage
{
	// This will create a string using the c string returned from the function call 'getNameForCLanguage' which is defined in "C.h" and is implemented in "C.c".
	NSString *languageName = [NSString stringWithCString:getNameForCLanguage()];
	
	// Return the language name.
	return languageName;
}

- (NSString *)nameForCPlusPlusLanguage
{
	// Create a C++ object (class CPluPlusLanguage) which is defined in "C++.h" and implemented in "C++.cpp".
	CPlusPlusLanguage language = CPlusPlusLanguage();
	
	// Create a string based on the c string returned from the method 'name' of the language object.
	NSString *languageName = [NSString stringWithCString:language.name()];
	
	// Return the language name.
	return languageName;
}

- (NSString *)nameForJavaLanguage
{
	// This method uses the Java Bridge to find a Java class with the name "JavaLanguage" which is defined in JavaLanguage.java. It allocates an Objective-C object 'language' where the implementation is done in Java.
	id language = [[[NSClassFromString(@"JavaLanguage") alloc] init] autorelease];
	
	// Ask the language object for it's name and return it.
	return [language name];
}

- (NSString *)nameOfAllLanguages
{
	// This method is to demonstrate calling all of the languages used in this example in one method, that asks each language to append it's language name to the end of a provided string.
	
	// Allocate the string to return.
	NSMutableString *allLanguages = [NSMutableString stringWithCapacity:1];
	
	// Creata a C++ based object.
	CPlusPlusLanguage cplusplusLanguage = CPlusPlusLanguage();
	
	// Create an Objective-C based object.
	ObjCLanguage *objcLanguage = [[[ObjCLanguage alloc] init] autorelease];
	
	// Create a Java based object
	id javaLanguage = [[[NSClassFromString(@"JavaLanguage") alloc] init] autorelease];
	
	// Ask each of the objects to append the name of their language to provided string. It will execute in the following order: C, C++, Java, Objective-C and then 'AppleScript' will be appended to the result of this method.
	[allLanguages appendString:[objcLanguage appendNameToString:[javaLanguage appendNameToString:[NSString stringWithCString:cplusplusLanguage.appendNameToString(getNameForCLanguage())]]]];
	
	// Return the language names
	return allLanguages;
}

@end
