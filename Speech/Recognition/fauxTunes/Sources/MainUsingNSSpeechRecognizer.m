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


	MainUsingNSSpeechRecognizer.m
	fauxTunes
	
	Copyright (c) 2001-2005 Apple Computer, Inc. All rights reserved.
*/

#import <Cocoa/Cocoa.h>

#import "FauxTunes.h"
#import "Main.h"
#import "SpeechRoutines.h"

// Prototypes
NSDictionary * CreateCommandsDictionary();

// Globals
NSDictionary * 			gCommandsDictionary = NULL;
NSSpeechRecognizer *	gSpeechRecognizer	= NULL;

@implementation SpeechEnabledAppDelegate

/* 
    applicationDidFinishLaunching:
    
    This method is called by Cocoa when our application has been loaded and is ready to run.
    
*/

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
    
    // Create a dictionary containing keys as command phrases and values as selectors.
	gCommandsDictionary = CreateCommandsDictionary();

	// Instaniate the speech recognizer object, and set ourselves as the delegate.
	gSpeechRecognizer = [NSSpeechRecognizer new];
	[gSpeechRecognizer setDelegate:self];
    
	// Add commands
	[gSpeechRecognizer setCommands:[gCommandsDictionary allKeys]];

	// Start listening
	[gSpeechRecognizer startListening];
    
}

- (void)speechRecognizer:(NSSpeechRecognizer *)sender didRecognizeCommand:(id)command
{
	[_fauxTunes performSelector:(SEL)[[gCommandsDictionary objectForKey:command] intValue]];
}

@end

/*
    SetupCommandNameTable
    
*/

NSDictionary * CreateCommandsDictionary()
{

	return [[NSDictionary alloc] initWithObjectsAndKeys:
				// File Menu
				[NSNumber numberWithInt:(int)@selector(fileMenuNewPlaylist:)], @"Make new playlist",
				[NSNumber numberWithInt:(int)@selector(fileMenuNewPlaylistFromSelection:)], @"New playlist from selection",
				[NSNumber numberWithInt:(int)@selector(fileMenuAddToLibrary:)], @"Add to library",
				[NSNumber numberWithInt:(int)@selector(fileMenuHideOrShowWindow:)], @"Show fauxTunes Window",
				[NSNumber numberWithInt:(int)@selector(fileMenuHideOrShowWindow:)], @"Hide fauxTunes Window",
				[NSNumber numberWithInt:(int)@selector(fileMenuCloseWindow:)], @"Close this window",
				[NSNumber numberWithInt:(int)@selector(fileMenuGetInfo:)], @"Get song info",
				[NSNumber numberWithInt:(int)@selector(fileMenuShowSongFile:)], @"Show song file",
				[NSNumber numberWithInt:(int)@selector(fileMenuShopForProducts:)], @"Shop for fauxTunes products",
				// Edit Menu
				[NSNumber numberWithInt:(int)@selector(editMenuUndo:)], @"Cancel last command",
				[NSNumber numberWithInt:(int)@selector(editMenuCopy:)], @"Copy this to the clipboard",
				[NSNumber numberWithInt:(int)@selector(editMenuPaste:)], @"Paste the clipboard here",
				[NSNumber numberWithInt:(int)@selector(editMenuSelectAll:)], @"Select all songs",
				[NSNumber numberWithInt:(int)@selector(editMenuSelectNone:)], @"Select none",
				[NSNumber numberWithInt:(int)@selector(editMenuShowCurrentSong:)], @"Show current song",
				[NSNumber numberWithInt:(int)@selector(editMenuViewOptions:)], @"Show view options",
				// Controls Menu
				[NSNumber numberWithInt:(int)@selector(controlsMenuPlayOrStopSong:)], @"Play this song",
				[NSNumber numberWithInt:(int)@selector(controlsMenuPlayOrStopSong:)], @"Stop playing song",
				[NSNumber numberWithInt:(int)@selector(controlsMenuNextSong:)], @"Go to next song",
				[NSNumber numberWithInt:(int)@selector(controlsMenuPreviousSong:)], @"Go to previous song",
				[NSNumber numberWithInt:(int)@selector(controlsMenuShuffle:)], @"Shuffle songs",
				[NSNumber numberWithInt:(int)@selector(controlsMenuRepeatOff:)], @"Turn repeating off",
				[NSNumber numberWithInt:(int)@selector(controlsMenuRepeatAll:)], @"Repeat all songs",
				[NSNumber numberWithInt:(int)@selector(controlsMenuRepeatOne:)], @"Repeat one song",
				[NSNumber numberWithInt:(int)@selector(controlsMenuVolumeUp:)],  @"Turn volume up",
				[NSNumber numberWithInt:(int)@selector(controlsMenuVolumeDown:)], @"Turn volume down",
				[NSNumber numberWithInt:(int)@selector(controlsMenuMute:)], @"Mute volume",
				[NSNumber numberWithInt:(int)@selector(controlsMenuEjectCD:)], @"Eject the CD",
				// Visuals Menu
				[NSNumber numberWithInt:(int)@selector(visualsMenuToggleVisuals:)], @"Display visual",
				[NSNumber numberWithInt:(int)@selector(visualsMenuToggleVisuals:)], @"Turn visual off",
				[NSNumber numberWithInt:(int)@selector(visualsMenuSmall:)], @"Show visual at small size",
				[NSNumber numberWithInt:(int)@selector(visualsMenuMedium:)], @"Show visual at meduim size",
				[NSNumber numberWithInt:(int)@selector(visualsMenuLarge:)], @"Show visual at large size",
				[NSNumber numberWithInt:(int)@selector(visualsMenuFullScreen:)], @"Display full screen",
				[NSNumber numberWithInt:(int)@selector(visualsMenuFullScreen:)], @"Turn full screen off",
				// Advanced Menu
				[NSNumber numberWithInt:(int)@selector(advancedMenuOpenStream:)], @"Open stream",
				[NSNumber numberWithInt:(int)@selector(advancedMenuConvertToMP3:)], @"Convert to MP3",
				[NSNumber numberWithInt:(int)@selector(advancedMenuExportSongList:)], @"Export song list",
				[NSNumber numberWithInt:(int)@selector(advancedMenuGetCDTrackNames:)], @"Get CD track names",
				[NSNumber numberWithInt:(int)@selector(advancedMenuSubmitCDTrackNames:)], @"Submit CD track names",
				[NSNumber numberWithInt:(int)@selector(visualsMenuFullScreen:)], @"Convert ID3 tags",
				// Help Menu
				[NSNumber numberWithInt:(int)@selector(helpMenuGetHelp:)], @"Get help",
		NULL];
}


/*
    main
    
    This is the standard main routine for Cocoa applications
*/

int main(int argc, const char *argv[])
{
    return NSApplicationMain(argc, argv);
}

