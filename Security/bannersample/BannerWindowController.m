/*
 * Copyright (c) 2000-2001,2003-2004 Apple Computer, Inc. All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

//
// bannersample - simple example of plugin for auth api
//

#include <Security/AuthorizationPlugin.h>
#import "BannerWindowController.h"

static BannerWindowController *mBannerWindowController = nil;
static void *lastMechanismRef;

OSStatus setResult(AuthorizationMechanismRef inMechanism);
OSStatus initializeBanner(AuthorizationMechanismRef inMechanism);
OSStatus finalizeBanner(AuthorizationMechanismRef inMechanism);

OSStatus initializeBanner(AuthorizationMechanismRef inMechanism)
{
	if (!mBannerWindowController)
	{
		mBannerWindowController = [[BannerWindowController alloc] init];
		[NSApp activateIgnoringOtherApps:YES];
	//	[[mBannerWindowController window] display];
		[mBannerWindowController showWindow:nil];
	}
	[mBannerWindowController setRef:inMechanism];
	return 0;
}

OSStatus finalizeBanner(AuthorizationMechanismRef inMechanism)
{
	[mBannerWindowController release];
	return 0;
}

@implementation BannerWindowController

- (id)init
{
    NSLog(@"BannerWindowController:init");
	if ([super init])
		self = [super initWithWindowNibName:@"bannersample"];

    return self;
}

- (void)dealloc
{
    [super dealloc];
mBannerWindowController = nil;
lastMechanismRef = 0;
}

- (void)setRef:(void *)ref
{
    NSLog(@"BannerWindowController:setRef");
	mMechanismRef = ref;
lastMechanismRef = ref;
}

- (IBAction)okClicked: (id) sender
{
    // Signal to SecurityAgent that we are done
    NSLog(@"BannerWindowController:okClicked");
//	setResult(mMechanismRef);
	setResult(lastMechanismRef);
	
}

@end
