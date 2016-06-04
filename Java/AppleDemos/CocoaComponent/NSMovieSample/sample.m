//
//  sample.m
//  NSMovieSample
//
//  Created by Steve Lewallen on Mon Jan 13 2003.
//  Copyright (c) 2003 Apple Computer. All rights reserved.
//
#import <JavaVM/jni.h>
#import <JavaVM/AWTCocoaComponent.h>
#import <AppKit/AppKit.h>
#include "NSMovieView.h"

// Simple utility to convert java strings to NSStrings
NSString *convertToNSString(JNIEnv *env, jstring javaString)
{
    NSString *converted = nil;
    const jchar *unichars = NULL;

    if (javaString == NULL) {
        return nil;	
    }                   
    unichars = (*env)->GetStringChars(env, javaString, nil);
    if ((*env)->ExceptionOccurred(env)) {
        return @"";
    }
    converted = [NSString stringWithCharacters:unichars length:(*env)->GetStringLength(env, javaString)]; // auto-released
    (*env)->ReleaseStringChars(env, javaString, unichars);
    return converted;
}

// Interface to our custom NSView
@interface MyNSMovieView : NSMovieView<AWTCocoaComponent> {
}
-(id)init;
-(void)awtMessage:(jint)messageID message:(jobject)message env:(JNIEnv *)env;
@end

// Implementation of our custom NSView
@implementation MyNSMovieView
-(id)init
{
    return [super init];
}

// Messaging api which is sent async from CocoaComponent's sendMessage api
-(void)awtMessage:(jint)messageID message:(jobject)message env:(JNIEnv *)env
{
    switch (messageID) {
        case NSMovieView_kSetMovieUrl:
        {
            NSMovie *movie = [[NSMovie alloc] initWithURL:[NSURL
                URLWithString:convertToNSString(env, message)] byReference:YES];
            [self setMovie:movie];
            break;
        }
        case NSMovieView_kPlayMovie:
            [self start:self];
            break;
        case NSMovieView_kStopMovie:
            [self stop:self];
            break;        
        default:
            fprintf(stderr,"MyNSMovieView Error : Unknown Message Received (%d)\n", (int)messageID);
            
    }
}

@end

// Simple JNI_OnLoad api
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    return JNI_VERSION_1_4;
}

/*
 * Class:     NSMovieView
 * Method:    createNSViewLong
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_NSMovieView_createNSViewLong
  (JNIEnv *env, jobject nsMovieView)
{
    MyNSMovieView *movieView = nil;
    NS_DURING;
    
    // Here we create our custom NSView
    movieView = [[MyNSMovieView alloc] init];
    
    // We can call any api on it from the calling thread, so long as this does not block or
    // need to message the AppKit's main thread.  This is most always safe on an NSView which
    // has not yet been added to the view hierarchy. 
    [movieView showController:TRUE adjustingSize:NO];

    NS_HANDLER;
    fprintf(stderr,"ERROR : Failed to create NSMovieView\n");
    NS_VALUERETURN(0, jlong);
    NS_ENDHANDLER;

    // Return a pointer to the custom NSView (the view must have a retain count of at least 1)
    return (long)(uintptr_t) movieView;
}
