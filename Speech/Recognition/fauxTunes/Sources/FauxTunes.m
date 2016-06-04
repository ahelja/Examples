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


	FauxTunes.m
	fauxTunes
	
	Copyright (c) 2001-2005 Apple Computer, Inc. All rights reserved.
*/

#import "FauxTunes.h"

/*
    FauxTunes
    
    This class implements an application that imitates the menus and general window layout of Apple's popular
    iTunes application in order to show how speech recognition can easily be added to a shipping project.  No
    speech recognition is actually contained in this file, but is limited to the application delegate
    object.  Menu and buttons do nothing except display a description of the selected control at the bottom
    of the window.
        
*/

@implementation FauxTunes

//
// Methods for handling menu items
//

// File Menu
- (IBAction)fileMenuNewPlaylist:(id)sender{ [self actionOccurredWithName:@"New Playlist command received"]; }
- (IBAction)fileMenuNewPlaylistFromSelection:(id)sender{ [self actionOccurredWithName:@"New Playlist From Selection command received"]; }
- (IBAction)fileMenuAddToLibrary:(id)sender{ [self actionOccurredWithName:@"Add To Library command received"]; }
- (IBAction)fileMenuHideOrShowWindow:(id)sender{ [self actionOccurredWithName:@"Hide/Show Window command received"]; }
- (IBAction)fileMenuCloseWindow:(id)sender{ [self actionOccurredWithName:@"Close Window command received"]; }
- (IBAction)fileMenuGetInfo:(id)sender{ [self actionOccurredWithName:@"Get Info command received"]; }
- (IBAction)fileMenuShowSongFile:(id)sender{ [self actionOccurredWithName:@"Show Song File command received"]; }
- (IBAction)fileMenuShopForProducts:(id)sender{ [self actionOccurredWithName:@"Shop for Products command received"]; }

// Edit Menu
- (IBAction)editMenuUndo:(id)sender{ [self actionOccurredWithName:@"Undo command received"]; }
- (IBAction)editMenuCut:(id)sender{ [self actionOccurredWithName:@"Cut command received"]; }
- (IBAction)editMenuCopy:(id)sender{ [self actionOccurredWithName:@"Copy command received"]; }
- (IBAction)editMenuPaste:(id)sender{ [self actionOccurredWithName:@"Paste command received"]; }
- (IBAction)editMenuClear:(id)sender{ [self actionOccurredWithName:@"Clear command received"]; }
- (IBAction)editMenuSelectAll:(id)sender{ [self actionOccurredWithName:@"Select All command received"]; }
- (IBAction)editMenuSelectNone:(id)sender{ [self actionOccurredWithName:@"Select None command received"]; }
- (IBAction)editMenuShowCurrentSong:(id)sender{ [self actionOccurredWithName:@"Show Current Song command received"]; }
- (IBAction)editMenuViewOptions:(id)sender{ [self actionOccurredWithName:@"View Options command received"]; }

// Controls Menu
- (IBAction)controlsMenuPlayOrStopSong:(id)sender{ [self actionOccurredWithName:@"Play/Stop Song command received"]; }
- (IBAction)controlsMenuNextSong:(id)sender{ [self actionOccurredWithName:@"Next Song command received"]; }
- (IBAction)controlsMenuPreviousSong:(id)sender{ [self actionOccurredWithName:@"Previous Song command received"]; }
- (IBAction)controlsMenuShuffle:(id)sender{ [self actionOccurredWithName:@"Shuffle command received"]; }
- (IBAction)controlsMenuRepeatOff:(id)sender{ [self actionOccurredWithName:@"Repeat Off command received"]; }
- (IBAction)controlsMenuRepeatAll:(id)sender{ [self actionOccurredWithName:@"Repeat All command received"]; }
- (IBAction)controlsMenuRepeatOne:(id)sender{ [self actionOccurredWithName:@"Repeat One command received"]; }
- (IBAction)controlsMenuVolumeUp:(id)sender{ [self actionOccurredWithName:@"Volume Up command received"]; }
- (IBAction)controlsMenuVolumeDown:(id)sender{ [self actionOccurredWithName:@"Volume Down command received"]; }
- (IBAction)controlsMenuMute:(id)sender{ [self actionOccurredWithName:@"Mute command received"]; }
- (IBAction)controlsMenuEjectCD:(id)sender{ [self actionOccurredWithName:@"Eject CD command received"]; }

// Visuals Menu
- (IBAction)visualsMenuToggleVisuals:(id)sender{ [self actionOccurredWithName:@"Turn Visuals On/Off command received"]; }
- (IBAction)visualsMenuSmall:(id)sender{ [self actionOccurredWithName:@"Small Visuals command received"]; }
- (IBAction)visualsMenuMedium:(id)sender{ [self actionOccurredWithName:@"Mediam Visuals command received"]; }
- (IBAction)visualsMenuLarge:(id)sender{ [self actionOccurredWithName:@"Large Visuals command received"]; }
- (IBAction)visualsMenuFullScreen:(id)sender{ [self actionOccurredWithName:@"Toggle Full Screen command receivied"]; }

// Advanced Menu
- (IBAction)advancedMenuOpenStream:(id)sender{ [self actionOccurredWithName:@"Open Stream command received"]; }
- (IBAction)advancedMenuConvertToMP3:(id)sender{ [self actionOccurredWithName:@"Convert to MP3 command received"]; }
- (IBAction)advancedMenuExportSongList:(id)sender{ [self actionOccurredWithName:@"Export Song List command received"]; }
- (IBAction)advancedMenuGetCDTrackNames:(id)sender{ [self actionOccurredWithName:@"Get CD Track Names command received"]; }
- (IBAction)advancedMenuSubmitCDTrackNames:(id)sender{ [self actionOccurredWithName:@"Submit CD Track Names command received"]; }
- (IBAction)advancedMenuConvertID3Tags:(id)sender{ [self actionOccurredWithName:@"Convert ID3 Tags command received"]; }

// Help Menu
- (IBAction)helpMenuGetHelp:(id)sender{ [self actionOccurredWithName:@"Get Help command received"]; }


//
// Methods for handling window controls that don't map to menu items
//
- (IBAction)volumeSliderWasMoved:(id)sender { [self actionOccurredWithName:@"Volume was changed"]; }
- (IBAction)reverseButtonWasClicked:(id)sender { [self actionOccurredWithName:@"Reverse button was clicked"]; }
- (IBAction)forwardButtonWasClicked:(id)sender { [self actionOccurredWithName:@"Forward button was clicked"]; }


//
// Misc. methods
//
- (void)actionOccurredWithName:(NSString *)name
{
    [_infoTextField setStringValue:name];
}

-(void)awakeFromNib
{ 
    [(SongTableView *)_songTableView setFauxTunes:self];
    [_window makeFirstResponder:_songTableView];
}

@end


// This custom class for the song table view captures the space key
// to start and stop playing the song since this isn't easily done
// as a menu keyboard shortcut.
@implementation SongTableView : NSTableView
- (void)setFauxTunes:(FauxTunes *)inFauxTunes {  _fauxTunes = inFauxTunes; }
- (BOOL)acceptsFirstResponder { return true; }
- (BOOL)resignFirstResponder { return false; }
- (void)keyDown:(NSEvent *)theEvent { ([theEvent keyCode] == 49)?[_fauxTunes controlsMenuPlayOrStopSong:NULL]:[super keyDown:theEvent]; }
@end


