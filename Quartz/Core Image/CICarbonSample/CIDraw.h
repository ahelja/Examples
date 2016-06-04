/*
 *  CIDraw.h
 *  CICarbonSample
 *
 *  Created by frank on 2/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef CIDraw_H_
#define CIDraw_H_

#include <ApplicationServices/ApplicationServices.h>

extern SInt32	    gGammaValue;

void DoDraw(CGContextRef inContext, CGRect bounds);

#endif