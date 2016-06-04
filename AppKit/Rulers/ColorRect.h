/* ColorRect.h
 *
 * by Nik Gervae, Technical Publications, NeXT Software Inc.
 *
 * You may freely copy, distribute, and reuse the code in this example.
 * NeXT disclaims any warranty of any kind, expressed or  implied, as to
 * its fitness for any particular use.
 */


#import <AppKit/AppKit.h>

/* ColorRect is a lightweight class for splatting a colored rectangle
 * into a RectsView (which actually does most of the work). A ColorRect
 * can be locked down, so that the user can't move it, in which case it
 * draws a little X in the middle. When the selected rect is locked,
 * the RectsView doesn't allow the user to move the ruler markers. */

@interface ColorRect : NSObject
{
    NSRect frame;
    NSColor *color;
    BOOL locked;
}

- (id)initWithFrame:(NSRect)aRect color:(NSColor *)aColor;
- (void)setFrame:(NSRect)aRect;
- (NSRect)frame;
- (NSColor *)color;

- (void)setLocked:(BOOL)flag;
- (BOOL)isLocked;

- (void)drawRect:(NSRect)aRect selected:(BOOL)flag;

@end
