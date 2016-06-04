/* NestleView.h
 *
 * by Nik Gervae, Technical Publications, NeXT Software Inc.
 *
 * You may freely copy, distribute, and reuse the code in this example.
 * NeXT disclaims any warranty of any kind, expressed or  implied, as to
 * its fitness for any particular use.
 */


#import <AppKit/AppKit.h>


/* NestleView's job is to provide a buffer zone around the ruler's
 * client, an instance of RectsView. I did this to test what happens
 * to the coordinate mapping when the client isn't the scroll view's
 * document view. */

@interface NestleView : NSView
{
    id nestledView;
}

@end
