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


	InstallSpeakableItems.h
	
	Copyright (c) 2001-2005 Apple Computer, Inc. All rights reserved.
*/

#import <Carbon/Carbon.h>

Boolean InstallSpeakableItemsForThisApplication( CFStringRef commandFilesResourcePath, CFStringRef compliedASDataFilesResourcePath, Boolean inForceReinstallation );

/* 

    CREATING AND TESTING YOUR SPEAKABLE ITEMS
    
    This easiest way to create and test your items is to add items to your application just
    like a user would.  With Speakable Items running and your application front most, say
    "Make this application speakable".  In your home directory you'll find a new folder at
    the location: "~/Library/Speech/Speakable Items/Application Speakable Items/" that contains
    the name of your application.  Before you turn Speakable Items off, begin filling this folder
    with items for your application.
    
    Command files that execute key press events are best created by duplicating an existing command file,
    such as the "Cancel last command" item, into your application's speakable items folder.  To edit 
    the command file, drop the file on the Project Builder icon to open it.  Near the bottom of the 
    file you'll see a key value of "VirtualKeyCode" and a number value on the following line (the
    number 6 for the letter 'Z' in the case of "Cancel last command").  As you create your command
    files you replace this number with the virtual key code associated with the key to press by
    referring to the file "Virtual Key Code Mappings" included with the fauxTunes example.
    NOTE:  SpeakableItems expects the type of command files to be 'sicf', so make sure to use
    an editor that preserves the file type when saving your command file.  Project Builder
    preserves this, but TextEdit and PropertyListEditor do not.
    
    You can easily create compiled AppleScript files using the Script Editor application 
    that will send an Apple event to your application to perform an action.  For an example,
    look at the "Open Sherlock" item that comes with Mac OS X, or the "Quit fauxTunes" item that
    comes with the fauxTunes example.  Just make sure you save your AppleScript as "compiled".

    Test each of you items for recognizability and confusability with your other items and the global
    items that ship with Mac OS X.


    ADDING SPEAKABLE ITEMS TO YOUR APPLICATION'S BUNDLE
    
    Now that you've created and tested your items, you'll need to add them to your project so
    they'll be included inside your application bundle the next time you build.  
    
    When using Project Builder, follow these steps to add your items to your application:
        
        • Since you'll need to a separate your command files from your compiled AppleScript data files,
        so create two folders inside your project folder and name one "CommandFiles" and the other 
        "AppleScriptDataFiles".
        
        • You can simply copy the commands file you created early into folder named "CommandFiles" by
        dragging them using the Finder.  For any compiled AppleScript files you've created, you'll need
        to copy the information stored in the resource fork of the AppleScript into the data fork of the
        file you include in your application bundle.  To perform this conversion you'll need to execute 
        the following commands from within the Terminal application for each AppleScript file.
        
            /Developer/Tools/Derez ORIGINAL_COMPILED_AS_FILE_PATH  >  /tmp/derezzedfile.r
            /Developer/Tools/Rez /tmp/derezzedfile.r -useDF -o CONVERTED_AS_DATA_FILE_PATH
            
        You'll replace ORIGINAL_COMPILED_AS_FILE_PATH with the path to your compiled AppleScript file, such as:
        '~/Library/Speech/Speakable Items/Application Speakable Items/fauxTunes (SI)/Quit fauxTunes'
        
        And replace ORIGINAL_COMPILED_AS_FILE_PATH with the location where you want the converted file to be
        created.
        
        HINT: Dragging the file you want the path for into the Terminal window will paste the full path into
        the window.
        
        If your converted AS files aren't already inside the folder "AppleScriptDataFiles" you created earlier,
        drag them into this folder now.
        
        • Select "Edit Active Target" from the Project menu, and click the "Files & Build Phases" 
        tab, if not already showing.
        
        • Click the last section title, usually "ResourceManager Resources".  When selected it should show
        a border around the section.
        
        • Locate the "New Build Phases" submenu in the Project menu, then select "New Copy Files Build Phase".
        A new section will appear with the title "Copy Files".  Make sure that "Resources" is selected from the
        Where pop-up menu, and type "CommandFiles" into the Subpath field.
        
        • Select "Add Files" from the Project menu.  When the sheet appears, navigate to the "CommandFiles" folder
        you created inside your project's folder.  Select all the commands files you copied into this folder, then
        click Open.  Unfortunately, these files will be default be placed in the section titled "Bundle Resources",
        but don't despair, you just need to select the files and drag them to the newly create "Copy Files" section.
        HINT: You might need to collapse the sections in between in order to drag the files since the window doesn't auto
        scroll in older versions of Project Builder.
        
        • If you have Compiled AS Data files you want to copy, create another "Copy Files" build phase 
        with a subpath named "AppleScriptDataFiles" and add the AS data files using the same steps as you did with
        the command files.
        
        • Build your application and double check that the files actually got included inside your application bundle.
        This is best done with the Terminal application.  For example, a recursive listing of the fauxTunes Resources
        directory is performed as follows:
        
            ls -R "/Applications/fauxTunes (SI).app/Contents/Resources/"
            

    INSTALLING YOUR SPEAKABLE ITEMS AT RUN TIME

    Now that your items are packaged inside your application bundle, we need to install them at run time to allow
    the SpeakableItems application to find, load, and display them items as spoken commands your users can use.  All
    you need to do it call the provided utility routine InstallSpeakableItemsForThisApplication(...) to perform this
    action.  
    
    There's addition information in the routine header comment in the "InstallSpeakableItems.c" file, but for the 
    example above you would call this routine with the following parameters:
    
        InstallSpeakableItemsForThisApplication( CFSTR("CommandFiles"), CFSTR("AppleScriptDataFiles"), false );


*/
