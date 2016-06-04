//
//  main.m
//
//  Created by Ian Ollmann on Thu Oct 03 2002.
//  Copyright (c) 2002 Apple. All rights reserved.
//
//	
//  Please accept our apologies for the OO design model used here.
//  It is probably not the best design under the sun. We hope you
//  will charitably believe it is the twisted remnant of a lot of
//  code changes.
//
//  The FunctionMenu is a menu that dynamically creates its own menu items,
//  in response to the contents of the filterLists[] array to be found in 
//  Filters.m. This made it easy for us to add new filters as we wrote them
//  without wasting a lot of time changing interface elements. 
//
//  User driven data format changes are routed through the MyImageFilterView (yes, yes we know)
//  which tracks the starting and current image, and some details that it probably
//  shouldn't like what type of filter to use ( AltiVec or no, Interleaved data or no,
//  8 bit or 32 bit FP). These really should have been in the controller object.
//
//  A FilterTest widget is created to convert the image data from the imageFilterView
//  into a format recognized by the various filters. It then runs some timing tests
//  returns the elapsed function time and dies shortly thereafter.
//
//  The image data is stored in a NSImageRep owned by the MyImageFilterView.
//  When a filter is applied, it is copied from the NSImageRep, converted to 
//  a usable data format, operated on, and written/converted back to the 
//  NSImageRep. This lets us apply several filters in series to the image. 
//

#import <Cocoa/Cocoa.h>

int main(int argc, const char *argv[])
{
    int result;

    result = NSApplicationMain(argc, argv);

    return result;
}
