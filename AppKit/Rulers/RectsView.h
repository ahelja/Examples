/* RectsView.h
 *
 * by Nik Gervae, Technical Publications, NeXT Software Inc.
 *
 * You may freely copy, distribute, and reuse the code in this example.
 * NeXT disclaims any warranty of any kind, expressed or  implied, as to
 * its fitness for any particular use.
 */


#import <AppKit/AppKit.h>

@class Edge, ColorRect;


/* RectsView is the ruler view's client in this test app. It tries to handle
 * most ruler operations. */

@interface RectsView : NSView
{
    NSMutableArray *rects;
    ColorRect *selectedItem;
}

+ (void)initialize;
- (id)initWithFrame:(NSRect)frameRect;
- (void)awakeFromNib;
- (BOOL)acceptsFirstResponder;
- (void)drawRect:(NSRect)aRect;
- (void)mouseDown:(NSEvent *)theEvent;

- (void)selectRect:(ColorRect *)aColorRect;
- (void)lock:(id)sender;
- (void)zoomIn:(id)sender;
- (void)zoomOut:(id)sender;
- (void)nestle:(id)sender;

- (void)drawRulerlinesWithRect:(NSRect)aRect;
- (void)updateRulerlinesWithOldRect:(NSRect)oldRect newRect:(NSRect)newRect;
- (void)eraseRulerlinesWithRect:(NSRect)aRect;
- (void)updateHorizontalRuler;
- (void)updateVerticalRuler;
- (void)updateRulers;
- (void)updateSelectedRectFromRulers;


- (BOOL)rulerView:(NSRulerView *)aRulerView
    shouldMoveMarker:(NSRulerMarker *)aMarker;
- (float)rulerView:(NSRulerView *)aRulerView
    willMoveMarker:(NSRulerMarker *)aMarker
    toLocation:(float)location;
- (void)rulerView:(NSRulerView *)aRulerView
    didMoveMarker:(NSRulerMarker *)aMarker;
- (BOOL)rulerView:(NSRulerView *)aRulerView
    shouldRemoveMarker:(NSRulerMarker *)aMarker;
- (void)rulerView:(NSRulerView *)aRulerView
    didRemoveMarker:(NSRulerMarker *)aMarker;
- (BOOL)rulerView:(NSRulerView *)aRulerView
    shouldAddMarker:(NSRulerMarker *)aMarker;
- (float)rulerView:(NSRulerView *)aRulerView
    willAddMarker:(NSRulerMarker *)aMarker
    atLocation:(float)location;
- (void)rulerView:(NSRulerView *)aRulerView
    didAddMarker:(NSRulerMarker *)aMarker;
- (void)rulerView:(NSRulerView *)aRulerView
    handleMouseDown:(NSEvent *)theEvent;
- (void)rulerView:(NSRulerView *)aRulerView
    willSetClientView:(NSView *)newClient;


@end
