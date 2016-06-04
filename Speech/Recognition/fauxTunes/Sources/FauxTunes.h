/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple’s copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.


	FauxTunes.h
	fauxTunes
	
	Copyright (c) 2001-2005 Apple Computer, Inc. All rights reserved.
*/

#import <Cocoa/Cocoa.h>


@interface FauxTunes : NSObject {

    // Control references
    IBOutlet NSWindow *				_window;
    IBOutlet NSSlider *				_volumeSlider;
    IBOutlet NSButton *				_newPlaylistButton;
    IBOutlet NSButton *				_shuffleButton;
    IBOutlet NSButton *				_repeatButton;
    IBOutlet NSButton *				_visualsButton;
    IBOutlet NSButton *				_ejectButton;
    IBOutlet NSButton *				_playOrStopButton;
    IBOutlet NSButton *				_forwardButton;
    IBOutlet NSButton *				_reverseButton;
    IBOutlet NSTableView *			_playlistTableView;
    IBOutlet NSTableView *			_songTableView;
    IBOutlet NSTextField *			_infoTextField;
}


//
// Methods for handling menu items
//

// File Menu
- (IBAction)fileMenuNewPlaylist:(id)sender;
- (IBAction)fileMenuNewPlaylistFromSelection:(id)sender;
- (IBAction)fileMenuAddToLibrary:(id)sender;
- (IBAction)fileMenuHideOrShowWindow:(id)sender;
- (IBAction)fileMenuCloseWindow:(id)sender;
- (IBAction)fileMenuGetInfo:(id)sender;
- (IBAction)fileMenuShowSongFile:(id)sender;
- (IBAction)fileMenuShopForProducts:(id)sender;

// Edit Menu
- (IBAction)editMenuUndo:(id)sender;
- (IBAction)editMenuCut:(id)sender;
- (IBAction)editMenuCopy:(id)sender;
- (IBAction)editMenuPaste:(id)sender;
- (IBAction)editMenuClear:(id)sender;
- (IBAction)editMenuSelectAll:(id)sender;
- (IBAction)editMenuSelectNone:(id)sender;
- (IBAction)editMenuShowCurrentSong:(id)sender;
- (IBAction)editMenuViewOptions:(id)sender;

// Controls Menu
- (IBAction)controlsMenuPlayOrStopSong:(id)sender;
- (IBAction)controlsMenuNextSong:(id)sender;
- (IBAction)controlsMenuPreviousSong:(id)sender;
- (IBAction)controlsMenuShuffle:(id)sender;
- (IBAction)controlsMenuRepeatOff:(id)sender;
- (IBAction)controlsMenuRepeatAll:(id)sender;
- (IBAction)controlsMenuRepeatOne:(id)sender;
- (IBAction)controlsMenuVolumeUp:(id)sender;
- (IBAction)controlsMenuVolumeDown:(id)sender;
- (IBAction)controlsMenuMute:(id)sender;
- (IBAction)controlsMenuEjectCD:(id)sender;

// Visuals Menu
- (IBAction)visualsMenuToggleVisuals:(id)sender;
- (IBAction)visualsMenuSmall:(id)sender;
- (IBAction)visualsMenuMedium:(id)sender;
- (IBAction)visualsMenuLarge:(id)sender;
- (IBAction)visualsMenuFullScreen:(id)sender;

// Advanced Menu
- (IBAction)advancedMenuOpenStream:(id)sender;
- (IBAction)advancedMenuConvertToMP3:(id)sender;
- (IBAction)advancedMenuExportSongList:(id)sender;
- (IBAction)advancedMenuGetCDTrackNames:(id)sender;
- (IBAction)advancedMenuSubmitCDTrackNames:(id)sender;
- (IBAction)advancedMenuConvertID3Tags:(id)sender;

// Help Menu
- (IBAction)helpMenuGetHelp:(id)sender;


//
// Methods for handling window controls that don't map to menu items
//
- (IBAction)volumeSliderWasMoved:(id)sender;
- (IBAction)reverseButtonWasClicked:(id)sender;
- (IBAction)forwardButtonWasClicked:(id)sender;


//
// Misc. methods
//
- (void)actionOccurredWithName:(NSString *)name;


@end



// Custom class for capturing space key in order to start and stop song playing.
@interface SongTableView : NSTableView
{ FauxTunes *		_fauxTunes;}
- (void)setFauxTunes:(FauxTunes *)inFauxTunes;
- (BOOL)acceptsFirstResponder;
- (BOOL)resignFirstResponder;
- (void)keyDown:(NSEvent *)theEvent;
@end

