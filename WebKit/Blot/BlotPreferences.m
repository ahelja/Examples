//
//  BlotPreferences.m
//  Blot
//
//  Created by Ken Kocienda on Thu Jan 29 2004.
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#import "BlotPreferences.h"

NSString *const BlotDefaultEditingFilePathPreferenceKey = @"BlotDefaultEditingFilePath";
NSString *const BlotInlineSpellCheckingPreferenceKey = @"BlotInlineSpellChecking";
NSString *const BlotPreferencesChangedNotification = @"BlotPreferencesChanged";

static BlotPreferences *sharedPreferences = nil;

@interface BlotPreferences (BlotInternal) 
- (void)_postPreferencesChanged;
@end

@implementation BlotPreferences

+ (BlotPreferences *)sharedPreferences
{
    if (!sharedPreferences)
        sharedPreferences = [[BlotPreferences alloc] init];
        
    return sharedPreferences;
}

-(NSString *)windowNibName
{
    return @"BlotPreferences";
}

- (void)initializeFromDefaults 
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    NSString *path = [defaults stringForKey:BlotDefaultEditingFilePathPreferenceKey];
    if (path)
        [defaultEditingFilePath setStringValue:path];
}

-(void)windowDidLoad
{
    [super windowDidLoad];
    [self initializeFromDefaults];
}

- (BOOL)isResizable 
{
    return NO;
}

- (void)_postPreferencesChanged
{
    NSNotificationCenter *defaultCenter = [NSNotificationCenter defaultCenter];
    [defaultCenter postNotificationName:BlotPreferencesChangedNotification object:nil];
    [self initializeFromDefaults];
}

- (IBAction)defaultEditingFilePathChanged:sender
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setObject:[sender stringValue] forKey:BlotDefaultEditingFilePathPreferenceKey];
    [self _postPreferencesChanged];
}

@end

