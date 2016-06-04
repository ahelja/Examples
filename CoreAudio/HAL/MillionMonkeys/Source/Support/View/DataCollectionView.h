/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* DataCollectionView.h */

#import <Cocoa/Cocoa.h>
#import "DisplayPoint.h"

@interface DataCollectionView : NSView
{
	NSColor					*_backDropColor;
	NSColor					*_minTraceColor;
	NSColor					*_meanTraceColor;
	NSColor					*_maxTraceColor;
	NSColor					*_HALOverloadColor;
	NSColor					*_FTOverloadColor;
	NSColor					*_FTBlockedColor;
	NSColor					*_minTraceColor2;
	NSColor					*_meanTraceColor2;
	NSColor					*_maxTraceColor2;
	NSColor					*_minTraceColor3;
	NSColor					*_meanTraceColor3;
	NSColor					*_maxTraceColor3;
	NSColor					*_cursorColor;
	NSColor					*_midlineColor;
	NSColor					*_selectionColor;
	
	SInt32					_xPos;
	Boolean					_completedFirstPass;
	SInt32					_lastDrawingAtPoint;
	Float32					_HALScalar;
	Float32					_FTScalar;
	Float32					_workScalar;
	
	
	DisplayPoint**			_graphData;
	
	Boolean					*_displayMask;
	Boolean					_displayOverload;
	
	Float32					_verticalMidline;
	Float32					_selectionLocation;
}

- (id) initWithFrame:(NSRect)frame;
- (void) dealloc;
- (void) resetData;
- (void) addDisplayPoint:(DisplayPoint*)pointIn;
- (void) setDisplayMask:(Boolean *)displayMaskIn overload:(Boolean)displayOverloadIn;

- (void) setHALScalar:(Float32)scalarIn;
- (void) setFTScalar:(Float32)scalarIn;
- (void) setWorkScalar:(Float32)scalarIn;

- (UInt32) getNumberOfDisplayablePoints;
- (DisplayPoint*) getDisplayPointAtIndex:(int)inIndex;

- (UInt32) getLastDrawingPoint;
- (void) drawNextSlice;
- (void) drawRect:(NSRect)rect;
- (int) getDisplayIndex;
- (BOOL) didWrap;
- (void) setDisplayIndex:(int)newIndex;

@end

/*
 */
