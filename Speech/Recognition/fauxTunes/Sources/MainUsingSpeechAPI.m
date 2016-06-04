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


	MainUsingSpeechAPI
	fauxTunes
	
	Copyright (c) 2001-2005 Apple Computer, Inc. All rights reserved.
*/

#import <Cocoa/Cocoa.h>

#import "FauxTunes.h"
#import "Main.h"
#import "SpeechRoutines.h"

// Prototypes
CFIndex SetupCommandNameTable();
pascal OSErr HandleSpeechDoneAppleEvent ( const AppleEvent *theAEevt, AppleEvent* reply, long refcon);

// Globals
SRRecognitionSystem		gSpeechRecognitionSystem;
SRRecognizer			gSpeechRecognizer;
SRLanguageModel			gCommandsLangaugeModel;
SEL 					gMethodSelectorTable[100];
NSString *				gCommandNameTable[100];

@implementation SpeechEnabledAppDelegate

/* 
    applicationDidFinishLaunching:
    
    This method is called by Cocoa when our application has been loaded and is ready to run.
    
    For this example, we call routines to perform the following tasks during this method:
        - set up a command name/method table by calling SetupCommandNameTable (defined in this file)
        - call AEInstallEventHandler to register our speech done Apple event handler callback routine
        - call SetupSpeechRecognition (provided in "SpeechRoutines.c") to set up Apple's speech recognition system
        - call CFArrayCreate (defined in "CFArray.h") to create a CoreFoundation array of command name strings
        - call AddCommands (provided in "SpeechRoutines.c") to add our commands to the Speech Commands window and recognizer
        - call SRStartListening (defined in "SpeechRecognition.h") to begin actively listening for commands
    
*/

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
    CFArrayRef theCommandNamesCFArray = NULL;
    CFIndex		numberOfCommands;
    BOOL		speechRecognitionSetupWasSuccessful = false;
    
    //
    // Set up command name and method dispatch table to make Apple event handler routine easy.
    //
	numberOfCommands = SetupCommandNameTable();

    //
    // Register Speech Done Apple event handler routine.
    //
	if (AEInstallEventHandler( kAESpeechSuite, kAESpeechDone, NewAEEventHandlerUPP(HandleSpeechDoneAppleEvent), (SInt32)_fauxTunes, false) == noErr) {

        //
        // Instaniate and intialize our speech recognition objects.
        //
        if (SetupSpeechRecognition( &gSpeechRecognitionSystem, &gSpeechRecognizer, &gCommandsLangaugeModel )) {
    
            //
            // Add Commands
            //
            theCommandNamesCFArray = CFArrayCreate( NULL, (const void **)gCommandNameTable, numberOfCommands, &kCFTypeArrayCallBacks );
            if (theCommandNamesCFArray) {
                if (AddCommands( gSpeechRecognizer, gCommandsLangaugeModel, theCommandNamesCFArray )) {
                
                    //
                    // Start Listening
                    //
                    if (SRStartListening( gSpeechRecognizer ) == noErr)
                        speechRecognitionSetupWasSuccessful = true;
                }
                CFRelease( theCommandNamesCFArray );
            }
        }
    }
    
    // Note that speech recognition falied for some reason.
    if (! speechRecognitionSetupWasSuccessful)
        CFShowStr( CFSTR( "An error occurred starting speech recognition." ) );
    
}

@end

/*
    HandleSpeechDoneAppleEvent
    
    This routine is called by the Apple event manager when a speech done event occurs for our
    application.  We handle the event by calling a utility routine provided in SpeechRoutines.c
    to determine the command recognized, then use the command's index to look up and execute the
    desired method in the FauxTunes object.
*/
pascal OSErr HandleSpeechDoneAppleEvent( const AppleEvent *theAEevt, AppleEvent* reply, long refcon)
{
    UInt32 	theCommandID;
    
    if (ConvertAppleEventResultIntoCommandID( theAEevt, &theCommandID ))
        [(FauxTunes *)refcon performSelector:gMethodSelectorTable[theCommandID] withObject:NULL];

    return noErr;
}


/*
    SetupCommandNameTable
    
    This routines sets up a simple table that matches the spoken command name with a method 
    in the FauxTunes object.  The strings in this table are the actual commands names
    displayed to the user.

*/

CFIndex SetupCommandNameTable()
{
    UInt32 i = 0;
    
    // File Menu
    gCommandNameTable[i] 		= @"Make new playlist";
	gMethodSelectorTable[i++] 	= @selector(fileMenuNewPlaylist:);

    gCommandNameTable[i] 		= @"New playlist from selection";
	gMethodSelectorTable[i++] 	= @selector(fileMenuNewPlaylistFromSelection:);

	gCommandNameTable[i] 		= @"Add to library";
	gMethodSelectorTable[i++] 	= @selector(fileMenuAddToLibrary:);

    gCommandNameTable[i] 		= @"Show fauxTunes Window";
	gMethodSelectorTable[i++] 	= @selector(fileMenuHideOrShowWindow:);

    gCommandNameTable[i] 		= @"Hide fauxTunes Window";
	gMethodSelectorTable[i++] 	= @selector(fileMenuHideOrShowWindow:);

    gCommandNameTable[i] 		= @"Close this window";
	gMethodSelectorTable[i++] 	= @selector(fileMenuCloseWindow:);

    gCommandNameTable[i] 		= @"Get song info";
	gMethodSelectorTable[i++] 	= @selector(fileMenuGetInfo:);

    gCommandNameTable[i] 		= @"Show song file";
	gMethodSelectorTable[i++] 	= @selector(fileMenuShowSongFile:);

    gCommandNameTable[i] 		= @"Shop for fauxTunes products";
	gMethodSelectorTable[i++] 	= @selector(fileMenuShopForProducts:);
    
    // Edit Menu
    gCommandNameTable[i] 		= @"Cancel last command";
	gMethodSelectorTable[i++] 	= @selector(editMenuUndo:);

    gCommandNameTable[i] 		= @"Copy this to the clipboard";
	gMethodSelectorTable[i++] 	= @selector(editMenuCopy:);

    gCommandNameTable[i] 		= @"Paste the clipboard here";
	gMethodSelectorTable[i++] 	= @selector(editMenuPaste:);

    gCommandNameTable[i] 		= @"Select all songs";
	gMethodSelectorTable[i++] 	= @selector(editMenuSelectAll:);

    gCommandNameTable[i] 		= @"Select none";
	gMethodSelectorTable[i++] 	= @selector(editMenuSelectNone:);

    gCommandNameTable[i] 		= @"Show current song";
	gMethodSelectorTable[i++] 	= @selector(editMenuShowCurrentSong:);

    gCommandNameTable[i] 		= @"Show view options";
	gMethodSelectorTable[i++] 	= @selector(editMenuViewOptions:);

    // Controls Menu
    gCommandNameTable[i] 		= @"Play this song";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuPlayOrStopSong:);

    gCommandNameTable[i] 		= @"Stop playing song";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuPlayOrStopSong:);

    gCommandNameTable[i] 		= @"Go to next song";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuNextSong:);

    gCommandNameTable[i] 		= @"Go to previous song";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuPreviousSong:);

    gCommandNameTable[i] 		= @"Shuffle songs";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuShuffle:);

    gCommandNameTable[i] 		= @"Turn repeating off";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuRepeatOff:);

    gCommandNameTable[i] 		= @"Repeat all songs";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuRepeatAll:);

    gCommandNameTable[i] 		= @"Repeat one song";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuRepeatOne:);

    gCommandNameTable[i] 		= @"Turn volume up";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuVolumeUp:);

    gCommandNameTable[i] 		= @"Turn volume down";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuVolumeDown:);

    gCommandNameTable[i] 		= @"Mute volume";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuMute:);

    gCommandNameTable[i] 		= @"Eject the CD";
	gMethodSelectorTable[i++] 	= @selector(controlsMenuEjectCD:);

    // Visuals Menu
    gCommandNameTable[i] 		= @"Display visual";
	gMethodSelectorTable[i++] 	= @selector(visualsMenuToggleVisuals:);

    gCommandNameTable[i] 		= @"Turn visual off";
	gMethodSelectorTable[i++] 	= @selector(visualsMenuToggleVisuals:);

    gCommandNameTable[i] 		= @"Show visual at small size";
	gMethodSelectorTable[i++] 	= @selector(visualsMenuSmall:);

    gCommandNameTable[i] 		= @"Show visual at meduim size";
	gMethodSelectorTable[i++] 	= @selector(visualsMenuMedium:);

    gCommandNameTable[i] 		= @"Show visual at large size";
	gMethodSelectorTable[i++] 	= @selector(visualsMenuLarge:);

    gCommandNameTable[i] 		= @"Display full screen";
	gMethodSelectorTable[i++] 	= @selector(visualsMenuFullScreen:);

    gCommandNameTable[i] 		= @"Turn full screen off";
	gMethodSelectorTable[i++] 	= @selector(visualsMenuFullScreen:);

    // Advanced Menu
    gCommandNameTable[i] 		= @"Open stream";
	gMethodSelectorTable[i++] 	= @selector(advancedMenuOpenStream:);

    gCommandNameTable[i] 		= @"Convert to MP3";
	gMethodSelectorTable[i++] 	= @selector(advancedMenuConvertToMP3:);

    gCommandNameTable[i] 		= @"Export song list";
	gMethodSelectorTable[i++] 	= @selector(advancedMenuExportSongList:);

    gCommandNameTable[i] 		= @"Get CD track names";
	gMethodSelectorTable[i++] 	= @selector(advancedMenuGetCDTrackNames:);

    gCommandNameTable[i] 		= @"Submit CD track names";
	gMethodSelectorTable[i++] 	= @selector(advancedMenuSubmitCDTrackNames:);

    gCommandNameTable[i] 		= @"Convert ID3 tags";
	gMethodSelectorTable[i++] 	= @selector(visualsMenuFullScreen:);

    // Help Menu
    gCommandNameTable[i] 		= @"Get help";
	gMethodSelectorTable[i++] 	= @selector(helpMenuGetHelp:);

    return i - 2;
}


/*
    main
    
    This is the standard main routine for Cocoa applications
*/

int main(int argc, const char *argv[])
{
    return NSApplicationMain(argc, argv);
}

