/*
	Created by: nwg
	
	Copyright: 	© Copyright 2002-2003 Apple Computer, Inc. All rights reserved.
	
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

	$Log: OutlineView.mm,v $
	Revision 1.2  2003/05/27 17:47:00  firewire
	SDK16
	
	Revision 1.1  2002/11/07 00:36:20  noggin
	move to new repository
	
	Revision 1.3  2002/08/06 22:23:51  noggin
	we were creating extra retains on services.. now we release them properly.
	
	Revision 1.2  2002/07/23 16:36:36  noggin
	first draft of graphing classes
	
	Revision 1.1  2002/07/18 22:04:19  noggin
	added spiffy outline tree lines
	
*/

#import "OutlineView.h"
#import "OutlineItem.h"

@implementation OutlineView

	- (id)initWithFrame:(NSRect)frameRect
	{
		if ( self != [ super initWithFrame:frameRect ] )
			return nil ;
	
		mIImage = [ NSImage imageNamed:@"I" ] ;
		[ mIImage setFlipped:YES ] ;
	
		mLImage = [ NSImage imageNamed:@"L" ] ;
		[ mLImage setFlipped:YES ] ;
		
		mRightTImage = [ NSImage imageNamed:@"right T" ] ;
		[ mRightTImage setFlipped:YES ] ;
	
		mDashImage = [ NSImage imageNamed:@"dash" ] ;
		[ mDashImage setFlipped:YES ] ;
		
		return self ;
	}
	
	- (void)drawRow:(int)row clipRect:(NSRect)rect ;
	{	
		// do normal row drawing first...
		[ super drawRow:row clipRect:rect ] ;

		// adorn row with "tree lines"...
		NSRect			drawRect = [ self rectOfRow:row ] ;
		{
			NSRect tempRect = NSIntersectionRect( rect, drawRect ) ;
			if ( tempRect.size.width == 0.0f || tempRect.size.height == 0.0f )
				return ;	// nothing to draw
		}
		
		OutlineItem*	item = [ self itemAtRow:row ] ;
	
		float left = ( [ self levelForRow:row ] - 1 ) * [ self indentationPerLevel ] + 4.0f ;
		if ( left < 0.0f )
			return ;	// nothing to draw
	
		BOOL	hasParent = [ item hasParent ] ;
		if (!hasParent)
			return ;
		
		BOOL	hasSibling = ( [ [ item parent ] numberOfChildren ] ) 
					> ( [ [ [ item parent ] children ] indexOfObject:item ] + 1 ) ;
		BOOL	hasChildren = [ [ item children ] count ] > 0 ;
				
		if (!hasChildren)
		{
			// draw horz. line where turn-down triangle would be		
			NSRect frameRect ;
			{
				frameRect.origin.x = 0.0f ;
				frameRect.origin.y = 0.0f ;
				frameRect.size = [ mDashImage size ] ;
			}
	
			[ mDashImage
					drawAtPoint:NSMakePoint( left + 18.0f, drawRect.origin.y )
					fromRect:frameRect
					operation:NSCompositeSourceOver fraction:1.0f ] ;
		}
	
		// draw 'L' or right 'T'
		{			
			NSImage* image = hasSibling ? mRightTImage : mLImage ;
			
			NSRect frameRect ;
			{
				frameRect.origin.x = 0.0f ;
				frameRect.origin.y = 0.0f ;
				frameRect.size = [ image size ] ;
			}
			
			[ image
				drawAtPoint:NSMakePoint( left, drawRect.origin.y )
				fromRect:frameRect
				operation:NSCompositeSourceOver fraction:1.0f ] ;
		}
		
		// draw 'I'
		{	
			NSRect frameRect ;
			{
				frameRect.origin.x = 0.0f ;
				frameRect.origin.y = 0.0f ;
				frameRect.size = [ mIImage size ] ;
			}
			
			item = [ item parent ] ;
			left -= [ self indentationPerLevel ] ;
			
			// this seems a little expensive.. could be made better?
			while ( item && [ item parent ] )
			{
				BOOL hasSibling = ( [ [ item parent ] numberOfChildren ] ) > ( [ [ [ item parent ] children ] indexOfObject:item ] + 1 ) ;
				if ( hasSibling )
				{
					[ mIImage 
						drawAtPoint:NSMakePoint( left, drawRect.origin.y )
						fromRect:frameRect
						operation:NSCompositeSourceOver fraction:1.0f ] ;
				}
					
				item = [ item parent ] ;
				left -= [ self indentationPerLevel ] ;
			}
		}
	}

@end

