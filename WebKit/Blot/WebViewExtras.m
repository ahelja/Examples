//
//  WebViewExtras.m
//  Blot
//
//  Created by Ken Kocienda on Mon Jan 12 2004.
//  Copyright (c) 2004 Apple Computer. All rights reserved.
//

#import "WebViewExtras.h"
#import "BlotPreferences.h"

#define BlotDefaultEditingFilePath @"BlotDefaultEditingFilePath"

@implementation WebView (BlotExtras)

-(NSString *)defaultEditingSource
{
    NSString *dataPath = nil;
    NSData *data = nil;
    NSString *source = nil;
    
    // look in user defaults
    dataPath = [[NSUserDefaults standardUserDefaults] stringForKey:BlotDefaultEditingFilePath];
    if (dataPath) {
        data = [NSData dataWithContentsOfFile:dataPath];
        if (data)
            source = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
    }

    // look in app bundle
    if (!source) {
        NSBundle *thisBundle = [NSBundle mainBundle];
        if ((dataPath = [thisBundle pathForResource:@"default" ofType:@"html"])) {
            data = [NSData dataWithContentsOfFile:dataPath];
            source = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
        }
    }
    
    return source;
}

@end
