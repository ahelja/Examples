/*
        FieldApsect.h
        TextSizingExample

        Author: Mike Ferris
*/

#import "Aspect.h"

@interface FieldAspect : Aspect {
    IBOutlet NSBox *leftAlignedBox;
    IBOutlet NSBox *centerAlignedBox;
    IBOutlet NSBox *rightAlignedBox;
    IBOutlet NSBox *scrollingLeftAlignedBox;
    IBOutlet NSBox *scrollingCenterAlignedBox;
    IBOutlet NSBox *scrollingRightAlignedBox;

    NSTextView *leftAlignedTextView;
    NSTextView *centerAlignedTextView;
    NSTextView *rightAlignedTextView;
    NSTextView *scrollingLeftAlignedTextView;
    NSTextView *scrollingCenterAlignedTextView;
    NSTextView *scrollingRightAlignedTextView;

    NSRect leftTVKnownFrame;
    NSRect centerTVKnownFrame;
    NSRect rightTVKnownFrame;
}

@end
