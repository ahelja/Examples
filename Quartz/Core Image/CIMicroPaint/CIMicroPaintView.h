//
//  CIMicroPaintView.h
//  CIMicroPaint
//
//  Created by Joel Kraut on 2/6/05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import "SampleCIView.h"

@interface CIMicroPaintView : SampleCIView {
	CIImageAccumulator *imageAccumulator;
	CIFilter *brushFilter;
	CIFilter *compositeFilter;
	NSColor *color;
	float brushSize;
}

@end
