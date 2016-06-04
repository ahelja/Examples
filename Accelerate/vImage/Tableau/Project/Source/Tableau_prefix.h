/*
 *  Tableau_prefix.h
 *  Tableau
 *
 *  Created by Ian O on Thu Jan 02 2003.
 *  Copyright (c) 2003 Apple. All rights reserved.
 *
 */

#if defined( __OBJC__ )

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

#endif

#ifdef USING_ACCELERATE
    #include <Accelerate/Accelerate.h>
#else
    #import <vImage/vImage.h>
    #warning compiling to use vImage in /Library/Frameworks
#endif

