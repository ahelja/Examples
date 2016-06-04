/*
 
 File: CanvasZoomingScrollView.m
 
 Abstract: Scroll view with support for document zooming
 
 Version: 1.0
 
 © Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to 
 you by Apple Computer, Inc. ("Apple") in 
 consideration of your agreement to the following 
 terms, and your use, installation, modification 
 or redistribution of this Apple software 
 constitutes acceptance of these terms.  If you do 
 not agree with these terms, please do not use, 
 install, modify or redistribute this Apple 
 software.
 
 In consideration of your agreement to abide by 
 the following terms, and subject to these terms, 
 Apple grants you a personal, non-exclusive 
 license, under Apple's copyrights in this 
 original Apple software (the "Apple Software"), 
 to use, reproduce, modify and redistribute the 
 Apple Software, with or without modifications, in 
 source and/or binary forms; provided that if you 
 redistribute the Apple Software in its entirety 
 and without modifications, you must retain this 
 notice and the following text and disclaimers in 
 all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or 
 logos of Apple Computer, Inc. may be used to 
 endorse or promote products derived from the 
 Apple Software without specific prior written 
 permission from Apple.  Except as expressly 
 stated in this notice, no other rights or 
 licenses, express or implied, are granted by 
 Apple herein, including but not limited to any 
 patent rights that may be infringed by your 
 derivative works or by other works in which the 
 Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS 
 IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
 THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
 OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
 SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
 THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
 UNDER THEORY OF CONTRACT, TORT (INCLUDING 
 NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
 IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.
 
 */ 


#import "CanvasZoomingScrollView.h"
#import "CanvasView.h"

// A subclass of NSScrollView which provides for a small zooming placard
@implementation CanvasZoomingScrollView

- (void)awakeFromNib {
	// Create a zooming pop up button placard, initialize its menu and add it to the view.
	placardView = [[NSPopUpButton alloc] init];
	
	NSMenu *zoomMenu = [[NSMenu alloc] init];
	NSMenuItem *normalZoomItem;

	NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:@"50\%" action:nil keyEquivalent:@""];
	[menuItem setTag:50];
	[zoomMenu addItem:menuItem];
	[menuItem release];
	
	menuItem = [[NSMenuItem alloc] initWithTitle:@"75%" action:nil keyEquivalent:@""];
	[menuItem setTag:75];
	[zoomMenu addItem:menuItem];
	[menuItem release];
	
	normalZoomItem = menuItem = [[NSMenuItem alloc] initWithTitle:@"100%" action:nil keyEquivalent:@""];
	[menuItem setTag:100];
	[zoomMenu addItem:menuItem];
	[menuItem release];
	
	menuItem = [[NSMenuItem alloc] initWithTitle:@"150%" action:nil keyEquivalent:@""];
	[menuItem setTag:150];
	[menuItem release];
	
	menuItem = [[NSMenuItem alloc] initWithTitle:@"200%" action:nil keyEquivalent:@""];
	[menuItem setTag:200];
	[zoomMenu addItem:menuItem];
	[menuItem release];
	
	[placardView setMenu:zoomMenu];
	[placardView selectItem:normalZoomItem];
	[zoomMenu release];
	
	// Set the placard style.
	[[placardView cell] setControlSize:NSMiniControlSize];
	[[placardView cell] setAlignment:NSRightTextAlignment];
	[[placardView cell] setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSMiniControlSize]]];
	[[placardView cell] setBordered:YES];
	[[placardView cell] setBezeled:YES];
	[(NSButtonCell *)[placardView cell] setBezelStyle:NSShadowlessSquareBezelStyle];
	[placardView setAction:@selector(takeZoomValueFrom:)];
	[placardView setTarget:self];
	[self addSubview:placardView];
	
	// Always show the scrollbars to ensure that the zoom placard will always be visible.
	[self setAutohidesScrollers:NO];
}

- (void)dealloc {
	[placardView release];
	[super dealloc];
}

- (void)tile {
	// Place the placard view to the left of the horizontal scrollbar.
	NSRect placardRect, scrollerRect;
	[super tile];
	scrollerRect = [[self horizontalScroller] frame];
	NSDivideRect(scrollerRect, &placardRect, &scrollerRect, 60.0, NSMinXEdge);
	[placardView setFrame:placardRect];
	[[self horizontalScroller] setFrame:scrollerRect];
}

- (IBAction)takeZoomValueFrom:(id)sender {
	[(CanvasView *)[self documentView] setScaleFactor:(float)[[sender selectedCell] tag] / 100.0];
}

- (NSView *)placardView {
	return [[placardView retain] autorelease];
}

@end
