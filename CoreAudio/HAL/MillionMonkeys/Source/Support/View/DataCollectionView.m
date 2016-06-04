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
/* DataCollectionView.m */

#define ALIGNMENT_OFFSET	0.5
//#define DEBUGPRINT

#import "DataCollectionView.h"


@implementation DataCollectionView

#pragma mark ____DATACOLLECTIONVIEW
- (id) initWithFrame:(NSRect) frame
{
    UInt32 i;
	
	self = [super initWithFrame:frame];
    
    _xPos = 0;
    _selectionLocation = -1;
    
    _backDropColor = [NSColor colorWithCalibratedWhite:1.00f alpha:1.00f];
    
    _minTraceColor = [NSColor colorWithCalibratedRed:0.00f green:0.00f blue:1.00f alpha:1.00f];		// min is dark blue
    _meanTraceColor = [NSColor colorWithCalibratedRed:0.00f green:0.50f blue:1.00f alpha:1.00f];	// mean is turquoise
    _maxTraceColor = [NSColor colorWithCalibratedRed:0.00f green:1.00f blue:1.00f alpha:1.00f];		// max is cyan
	
    _minTraceColor2 = [NSColor colorWithCalibratedRed:1.00f green:0.00f blue:0.00f alpha:1.00f];	// min is red
    _meanTraceColor2 = [NSColor colorWithCalibratedRed:1.00f green:0.50f blue:0.00f alpha:1.00f];	// mean is orange
    _maxTraceColor2 = [NSColor colorWithCalibratedRed:1.00f green:1.00f blue:0.00f alpha:1.00f];	// max is yellow
    _HALOverloadColor = [NSColor colorWithCalibratedRed:1.00f green:0.00f blue:0.00f alpha:0.50f];	// HAL overload is 50% red
    
    _minTraceColor3 = [NSColor colorWithCalibratedRed:0.00f green:0.50f blue:0.00f alpha:1.00f];	// min is dark green
    _meanTraceColor3 = [NSColor colorWithCalibratedRed:0.00f green:1.00f blue:0.00f alpha:1.00f];	// mean is green
    _maxTraceColor3 = [NSColor colorWithCalibratedRed:0.50f green:1.00f blue:0.00f alpha:1.00f];	// max is yellow-green
    _FTOverloadColor = [NSColor colorWithCalibratedRed:0.66f green:0.00f blue:1.00f alpha:0.50f];	// FT overload is 50% purple
    _FTBlockedColor = [NSColor colorWithCalibratedRed:1.00f green:0.66f blue:0.33f alpha:0.50f];	// FT overload is 50% peach
    
    _cursorColor = [NSColor colorWithCalibratedRed:0.50f green:0.50f blue:0.50f alpha:1.00f];		// cursor is grey
    _midlineColor = [NSColor colorWithCalibratedRed:0.50f green:0.50f blue:0.50f alpha:1.00f];		// midline is grey
    
    _selectionColor = [NSColor colorWithCalibratedRed:0.00f green:0.00f blue:0.00f alpha:1.00f];	// selection is black
    
    [_backDropColor retain];
    [_minTraceColor retain];
    [_meanTraceColor retain];
    [_maxTraceColor retain];
    [_FTOverloadColor retain];
    [_FTBlockedColor retain];
	
    [_minTraceColor2 retain];
    [_meanTraceColor2 retain];
    [_maxTraceColor2 retain];
    [_HALOverloadColor retain];
	
    [_minTraceColor3 retain];
    [_meanTraceColor3 retain];
    [_maxTraceColor3 retain];
    
    [_cursorColor retain];
    [_midlineColor retain];
    [_selectionColor retain];
    
    _verticalMidline = NSHeight(frame) * 0.5;
	
	_lastDrawingAtPoint = 0;
	_completedFirstPass = FALSE;
	
	_graphData = (DisplayPoint**) malloc (NSWidth(frame) * sizeof(DisplayPoint*));
	for (i = 0; i < NSWidth(frame); i++) {
		_graphData[i] = NULL;
	}
	
    return self;
}

- (void) dealloc
{
	UInt32	i, width;
	
	width = NSWidth ([self frame]);
	for (i = 0; i < width; i++) {
		if (_graphData[i] != NULL) {
			free(_graphData[i]);
		}
	}
	free (_graphData);
    
    [_backDropColor release];
	[_minTraceColor release];
	[_meanTraceColor release];
	[_maxTraceColor release];
	[_HALOverloadColor release];
	[_FTOverloadColor release];
	[_FTBlockedColor release];
	[_minTraceColor2 release];
	[_meanTraceColor2 release];
	[_maxTraceColor2 release];
	[_minTraceColor3 release];
	[_meanTraceColor3 release];
	[_maxTraceColor3 release];
	[_cursorColor release];
	[_midlineColor release];
	[_selectionColor release];
	
	[super dealloc];
}

- (void) setHALScalar:(Float32)scalarIn
{
	_HALScalar = scalarIn;
	[self setNeedsDisplay:TRUE];
}

- (void) setFTScalar:(Float32)scalarIn
{
	_FTScalar = scalarIn;
	[self setNeedsDisplay:TRUE];
}

- (void) setWorkScalar:(Float32)scalarIn
{
	_workScalar = scalarIn;
	[self setNeedsDisplay:TRUE];
}

- (UInt32) getNumberOfDisplayablePoints
{
    // add one because 0 counts as a data point
	return NSWidth([self frame]) + 1;
}

- (int) getDisplayIndex
{
    return _xPos;
}

- (BOOL) didWrap
{
	if (_completedFirstPass == TRUE) {
		return YES;
	}
	
	return NO;
}

- (void) setDisplayIndex:(int)newIndex
{
    _selectionLocation = newIndex;
    [self setNeedsDisplay:TRUE];
}

- (void) resetData
{
    _lastDrawingAtPoint = _xPos = 0;
	_selectionLocation = 0;
	_completedFirstPass = FALSE;
	[self setNeedsDisplay:TRUE];
}

- (void) addDisplayPoint:(DisplayPoint*)pointIn
{
	if (_graphData[_xPos] != NULL) {
		free (_graphData[_xPos]);
	}
	_graphData[_xPos] = pointIn;
    
	++_xPos;
	if ( _xPos >= NSWidth([self frame]) ) {
		_xPos = 0;
		_completedFirstPass = TRUE;
    }
}

- (void) setDisplayMask:(Boolean *)displayMaskIn overload:(Boolean)displayOverloadIn
{
    _displayMask = displayMaskIn;
    _displayOverload = displayOverloadIn;
}

- (DisplayPoint*) getDisplayPointAtIndex:(int)inIndex
{
	return _graphData[inIndex];
}

- (UInt32) getLastDrawingPoint
{
	return _lastDrawingAtPoint;
}

- (void) drawNextSlice
{
	SInt32			start, end;
	SInt32			width, height;
	
	height = NSHeight([self frame]);
	
	start = _lastDrawingAtPoint - 2;
	if ((start < 0) || (start > NSWidth([self frame]))) start = 0;
	end = _xPos + 1;
	
	if (start < end) {
		width = end - start;
		[self displayRect:NSMakeRect(start, 0, width, height)];
	} else {
		width = NSWidth([self frame]) - start;
		[self displayRect:NSMakeRect(start, 0, width, height)];
		[self displayRect:NSMakeRect(0, 0, end, height)];
#ifdef DEBUGPRINT
printf ("_xPos:%d\n", _xPos);
printf ("[ Start:%d; End:%d\n", start, start + width);
printf ("  Start:%d; End:%d ]\n", 0, end);
#endif
	}
}

- (void) drawRect:(NSRect)rect
{
    int				i;
    int				start, end;
    int				height, frameWidth;
    int				myOffset;
	int				loopEnd;
	
	_lastDrawingAtPoint = _xPos;
#ifdef DEBUGPRINT
printf ("_xPos:%d\n", _xPos);
#endif
    start = NSMinX (rect);
    end = NSMaxX (rect);
	height = NSMaxY (rect);
    
    // draw backdrop
    [_backDropColor set];
    [NSBezierPath fillRect:rect];
    
    // draw border
    [[NSColor blackColor] set];
    if (start == 0) [NSBezierPath strokeLineFromPoint:NSMakePoint(0, NSMinY(rect)) toPoint:NSMakePoint(0, height)];
    frameWidth = NSWidth ([self frame]);
    if (end == frameWidth) [NSBezierPath strokeLineFromPoint:NSMakePoint(frameWidth, NSMinY(rect)) toPoint:NSMakePoint(frameWidth, height)];
    [NSBezierPath strokeLineFromPoint:NSMakePoint(start, 0) toPoint:NSMakePoint(end, 0)];
    [NSBezierPath strokeLineFromPoint:NSMakePoint(start, height) toPoint:NSMakePoint(end, height)];
    
    // draw cursor
    if ( ((_lastDrawingAtPoint + ALIGNMENT_OFFSET) >= start) || (((_lastDrawingAtPoint + ALIGNMENT_OFFSET) <= end)) ) {
        [_cursorColor set];
        [NSBezierPath strokeRect:NSMakeRect(_lastDrawingAtPoint + ALIGNMENT_OFFSET - 1, 0, 1, height)];
    }
    
    // graph data
    if (_displayMask) {
		if ( ((_lastDrawingAtPoint - 1) < end) && !(_completedFirstPass) ) {
			loopEnd = _lastDrawingAtPoint - 1;
		} else {
			loopEnd = end;
		}
		if (loopEnd < 0) loopEnd = 0;
		for (i = start; i < loopEnd; i++) {
			// HAL Overload
            if (_graphData[i]->HALOverload == TRUE) {
                [_HALOverloadColor set];
                [NSBezierPath strokeLineFromPoint:NSMakePoint(i + ALIGNMENT_OFFSET, 0)
                                                                toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, height)];
            }
			
			// HAL Trace Overload
            if ( (_displayOverload == TRUE) && (_graphData[i]->HALTraceOverload == TRUE) ) {
                NSBezierPath*	myPath;
				float			dashing[2];
				
				[_HALOverloadColor set];
				dashing[0] = dashing[1] = 5.0;
				
				myPath = [NSBezierPath bezierPath];
				[myPath setLineDash:dashing count:2 phase:0.0];
				[myPath moveToPoint:NSMakePoint (i + ALIGNMENT_OFFSET, 0)];
				[myPath lineToPoint:NSMakePoint (i + ALIGNMENT_OFFSET, height)];
				[myPath stroke];
			}
			
			// FT Overload
            if ( (_displayOverload == TRUE) && (_graphData[i]->FeederThreadOverload == TRUE) ) {
                [_FTOverloadColor set];
                [NSBezierPath strokeLineFromPoint:NSMakePoint(i + ALIGNMENT_OFFSET, 0)
                                                                toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, height)];
            }
			
			// FT Never Ran
            if ( (_displayOverload == TRUE) && (_graphData[i]->FeederThreadNeverRan == TRUE) ) {
                [_FTBlockedColor set];
                [NSBezierPath strokeLineFromPoint:NSMakePoint(i + ALIGNMENT_OFFSET, 0)
                                                                toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, height)];
            }
						
			// FT Trace Overload
            if ( (_displayOverload == TRUE) && (_graphData[i]->FeederThreadTraceOverload == TRUE) ) {
                NSBezierPath*	myPath;
				float			dashing[2];
				
				[_FTOverloadColor set];
				dashing[0] = dashing[1] = 5.0;
				
				myPath = [NSBezierPath bezierPath];
				[myPath setLineDash:dashing count:2 phase:0.0];
				[myPath moveToPoint:NSMakePoint (i + ALIGNMENT_OFFSET, 0)];
				[myPath lineToPoint:NSMakePoint (i + ALIGNMENT_OFFSET, height)];
				[myPath stroke];
			}
			
			// Work Trace Overload
            if ( (_displayOverload == TRUE) && (_graphData[i]->WorkTraceOverload == TRUE) ) {
                NSBezierPath*	myPath;
				float			dashing[2];
				
				[_meanTraceColor set];
				dashing[0] = dashing[1] = 5.0;
				
				myPath = [NSBezierPath bezierPath];
				[myPath setLineDash:dashing count:2 phase:0.0];
				[myPath moveToPoint:NSMakePoint (i + ALIGNMENT_OFFSET, 0)];
				[myPath lineToPoint:NSMakePoint (i + ALIGNMENT_OFFSET, height)];
				[myPath stroke];
			}
			
            // offset to keep 0 at beginning of data set from screwing up graph
			myOffset = -1;
            if (i < 1) myOffset = 0;
			
            if (_displayMask[0 * 3 + 0] == TRUE) {
                [_minTraceColor set];
                [NSBezierPath strokeLineFromPoint:NSMakePoint(i - 1 + ALIGNMENT_OFFSET, _graphData[i + myOffset]->WorkDoneMin * _workScalar)
                                                                toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, _graphData[i]->WorkDoneMin * _workScalar)];
            }
            if (_displayMask[0 * 3 + 1] == TRUE) {
                [_minTraceColor2 set];
                [NSBezierPath strokeLineFromPoint:NSMakePoint(i - 1 + ALIGNMENT_OFFSET, _graphData[i + myOffset]->HALLatencyMin * _HALScalar + _verticalMidline)
                                                                toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, _graphData[i]->HALLatencyMin * _HALScalar + _verticalMidline)];
            }
            if (_displayMask[0 * 3 + 2] == TRUE) {
                [_minTraceColor3 set];
                [NSBezierPath strokeLineFromPoint:NSMakePoint(i - 1 + ALIGNMENT_OFFSET, _graphData[i + myOffset]->FTLatencyMin * _FTScalar)
                                                                toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, _graphData[i]->FTLatencyMin * _FTScalar)];
            }
            if (_displayMask[2 * 3 + 0] == TRUE) {
                [_maxTraceColor set];
                [NSBezierPath strokeLineFromPoint:NSMakePoint(i - 1 + ALIGNMENT_OFFSET, _graphData[i + myOffset]->WorkDoneMax * _workScalar)
                                                                toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, _graphData[i]->WorkDoneMax * _workScalar)];
            }
            if (_displayMask[2 * 3 + 1] == TRUE) {
                [_maxTraceColor2 set];
                [NSBezierPath strokeLineFromPoint:NSMakePoint(i - 1 + ALIGNMENT_OFFSET, _graphData[i + myOffset]->HALLatencyMax * _HALScalar + _verticalMidline)
                                                                toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, _graphData[i]->HALLatencyMax * _HALScalar + _verticalMidline)];
            }
            if (_displayMask[2 * 3 + 2] == TRUE) {
                [_maxTraceColor3 set];
                [NSBezierPath strokeLineFromPoint:NSMakePoint(i - 1 + ALIGNMENT_OFFSET, _graphData[i + myOffset]->FTLatencyMax * _FTScalar)
                                                                toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, _graphData[i]->FTLatencyMax * _FTScalar)];
            }
            if (_displayMask[1 * 3 + 0] == TRUE) {
                    [_meanTraceColor set];
                    [NSBezierPath strokeLineFromPoint:NSMakePoint(i - 1 + ALIGNMENT_OFFSET, _graphData[i + myOffset]->WorkDoneMean * _workScalar)
                                                                    toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, _graphData[i]->WorkDoneMean * _workScalar)];
            }
            if (_displayMask[1 * 3 + 1] == TRUE) {
                    [_meanTraceColor2 set];
                    [NSBezierPath strokeLineFromPoint:NSMakePoint(i - 1 + ALIGNMENT_OFFSET, _graphData[i + myOffset]->HALLatencyMean * _HALScalar + _verticalMidline)
                                                                    toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, _graphData[i]->HALLatencyMean * _HALScalar + _verticalMidline)];
            }
            if (_displayMask[1 * 3 + 2] == TRUE) {
                    [_meanTraceColor3 set];
                    [NSBezierPath strokeLineFromPoint:NSMakePoint(i - 1 + ALIGNMENT_OFFSET, _graphData[i + myOffset]->FTLatencyMean * _FTScalar)
                                                                    toPoint:NSMakePoint(i + ALIGNMENT_OFFSET, _graphData[i]->FTLatencyMean * _FTScalar)];
            }
       }
    }
        
    // draw selection if necessary
    if ( ((_selectionLocation + ALIGNMENT_OFFSET) >= start) || (((_selectionLocation + ALIGNMENT_OFFSET) <= end)) ) {
		[_selectionColor set];
		[NSBezierPath strokeLineFromPoint:NSMakePoint(_selectionLocation + ALIGNMENT_OFFSET, 0)
										toPoint:NSMakePoint(_selectionLocation + ALIGNMENT_OFFSET, height)];
	}
}

- (void) mouseDown:(NSEvent *)inEvent
{
    NSPoint translatedPoint;
    
    translatedPoint = [self convertPoint:[inEvent locationInWindow] fromView:nil];
    _selectionLocation = translatedPoint.x;
    
    [self setNeedsDisplay:TRUE];
    [(NSNotificationCenter *)[NSNotificationCenter defaultCenter] postNotificationName:@"showTrace" object:[NSNumber numberWithFloat:_selectionLocation]];
}

@end

/*
 */
