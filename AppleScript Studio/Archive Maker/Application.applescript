(* Application.applescript *)

(* This example employs many UI features in Cocoa, such as a 'drawer' and 'panels' as well as using the 'do shell script' to provide a UI frontend to the 'gnutar' shell tool to build tar archives. It also demonstrates how to design an application that is a droplet as well. You can also fine an example of how to use the 'user-defaults' class. *)

(* The structure of this script is as follows:
	Properties		Properties needed for the application.
	Event Handlers	Handlers that are called by actions in the UI.
	Handlers 		Handlers that are called within the script.
*)

(* ==== Properties ==== *)

-- Settings
property openWindowOnLaunch : true
property showProgress : true
property compressArchive : true
property preserveIDs : true
property followLinks : false
property verboseMode : false
property defaultLocation : ""

-- Others
property windowOpened : false
property progressPanel : missing value
property fileNames : {}
property filesDataSource : missing value


(* ==== Event Handlers ==== *)

-- This event handler is called as early in the process of launching an application as is possible. The handler is a good place to register our settings as well as read in the current set of settings. 
-- 
on will finish launching theObject
	set windowOpened to false
	
	registerSettings()
	readSettings()
end will finish launching

-- This event handler is the last handler called in the process of launching an application. If the handler is called and a window hasn't been shown yet (via the 'open' event handler) then we need to show the main window here (as well was opening the settings drawer).
-- 
on launched theObject
	if windowOpened is false then
		showWindow()
		showSettings()
	end if
end launched

-- This event handler is called when the object that is associated with it is loaded from its nib file. It's a good place to do any one-time initialization, which in this case is to create the data source for the table view.
--
on awake from nib theObject
	-- Create the data source for the table view
	set filesDataSource to make new data source at end of data sources with properties {name:"files"}
	
	-- Create the "files" data column
	make new data column at end of data columns of filesDataSource with properties {name:"files"}
	
	-- Assign the data source to the table view
	set data source of theObject to filesDataSource
	
	-- Register for the "file names" drag types
	tell theObject to register drag types {"file names", "color"}
end awake from nib

-- This event handler is called (in this example) when the user drags any finder items over the table view.
--
on drop theObject drag info dragInfo
	-- Get the list of data types on the pasteboard
	set dataTypes to types of pasteboard of dragInfo
	
	-- We are only interested in either "file names" or "color" data types
	if "file names" is in dataTypes then
		-- Initialize the list of files to an empty list
		set theFiles to {}
		
		-- We want the data as a list of file names, so set the preferred type to "file names"
		set preferred type of pasteboard of dragInfo to "file names"
		
		-- Get the list of files from the pasteboard
		set theFiles to contents of pasteboard of dragInfo
		
		-- Make sure we have at least one item
		if (count of theFiles) > 0 then
			-- Turn off the updating of the views
			set update views of filesDataSource to false
			
			-- For every item in the list, make a new data row and set it's contents
			repeat with theItem in theFiles
				set theDataRow to make new data row at end of data rows of filesDataSource
				set contents of data cell "files" of theDataRow to quoted form of theItem
				set fileNames to fileNames & {quoted form of theItem}
			end repeat
			
			-- Turn back on the updating of the views
			set update views of filesDataSource to true
		end if
	end if
	
	-- Set the preferred type back to the default
	set preferred type of pasteboard of dragInfo to ""
	
	return true
end drop

-- This event handler is called when you drag any file/folder items in the Finder onto the application icon (either in the Finder or in the Dock). It can be called as many times as the user drags items onto the application icon, therefore the main process here is to append the list of names the existing list of names. Then we conditionally open the window, make the archive (displaying a progress bar if requested) and then if a window hasn't been opened we simply quit. 
-- 
on open names
	-- Append the list of names to our current list
	repeat with i from 1 to count of names
		set fileNames to fileNames & {quoted form of (POSIX path of (item i of names))}
	end repeat
	
	-- Show the window if requested
	if openWindowOnLaunch then
		-- Of course, only show if it hasn't already been opened
		if not windowOpened then
			showWindow()
		end if
	end if
	
	-- If the main window wasn't opened then go ahead and process the list of files, making an archive with a determined name.
	if not windowOpened then
		set windowOpened to true
		
		-- Get the generated archive name
		set archiveFileName to getArchiveFileName()
		
		-- Show the progress panel if requested
		if showProgress then
			showProgressPanel(false, archiveFileName)
		end if
		
		-- Make the archive
		set theResult to makeArchive(archiveFileName)
		
		-- If we are in verbose mode, then show the results in the log window
		if verboseMode then
			set contents of text view "log" of scroll view "log" of window "log" to theResult
			show window "log"
		end if
		
		-- Hide the progress panel (if shown)
		if showProgress then
			hideProgressPanel(false)
		end if
		
		-- Go ahead and quit, as we are done. (This might need some rethinking, as it probably isn't the right thing to do if for instance the log window is shown, with the verbose mode on.
		quit
	else if openWindowOnLaunch then
		-- Turn off the updating of the views
		set update views of filesDataSource to false
		
		-- Add the files to the data source
		repeat with i from 1 to count of names
			set theDataRow to make new data row at end of data rows of filesDataSource
			set contents of data cell "files" of theDataRow to quoted form of (POSIX path of (item i of names))
		end repeat
		
		-- Turn back on the updating of the views
		set update views of filesDataSource to true
	end if
end open

-- This handler is the last handler to be called before the application quits. It's a good place to the get current settings from the setting drawer and write them out (but only if the window has been opened).
-- 
on will quit theObject
	if windowOpened then
		getSettingsFromUI()
		writeSettings()
	end if
end will quit

-- This event handler is called when a UI object is clicked (any object that is linked to this handler in Interface Builder that is...). 
-- 
on clicked theObject
	if name of theObject is "make" then
		-- Make sure that we have at least one item to make into an archive. 
		if (count of fileNames) is greater than 0 then
			-- Get the current settings in the UI from the settings drawer.
			getSettingsFromUI()
			
			-- Determine a good default name based on the first file item, and then ask for the archive name.
			set defaultName to last word of (item 1 of fileNames as string) & ".tar"
			if compressArchive then set defaultName to defaultName & ".gz"
			
			-- Setup the 'save panel'
			tell save panel
				set title to "Save Archive As"
				set prompt to "Make"
				set treat packages as directories to false
			end tell
			
			-- Display the save panel as a sheet (we will do the processing in the 'on panel ended' handler)
			display save panel in directory defaultLocation with file name defaultName attached to window of theObject
		else
			-- Alert the user that they need to have at least one file item.
			display alert "Missing Files/Folders" as critical message "You must add files or folders by dragging them on to the application icon in order to make an archive." attached to window "main"
		end if
	else if name of theObject is "settings" then
		-- This simply toggles the state of the 'settings' button, showing/hiding the settings drawer as needed.
		tell window "main"
			set currentState to state of drawer "settings"
			
			if currentState is drawer closed then
				my showSettings()
			else if currentState is drawer opened then
				my hideSettings()
			end if
			
		end tell
	else if name of theObject is "choose" then
		-- Choose the default location (folder) in which to store the archive when the application is used as a droplet (without the main window begin shown.)
		chooseDefaultLocation()
	end if
end clicked

-- This event handler is called when the save panel (which was shown as a sheet) has been concluded.
--
on panel ended theObject with result withResult
	if theObject is the open panel then
		if withResult is 1 then
			set theLocation to item 1 of (path names of open panel as list)
			set contents of text field "default location" of drawer "settings" of window "main" to theLocation as string
		end if
	else if theObject is the save panel and withResult is 1 then
		-- We need to hide the panel as we might be putting up a progress panel next
		set visible of save panel to false
		
		-- Show the progress panel (if requested).
		if showProgress then
			showProgressPanel(true, path name of save panel)
		end if
		
		-- The main point of this entire application. Make the archive (which expects everything to be a POSIX path.
		set theResult to makeArchive(path name of save panel)
		
		-- If requested, show the results of the make in the log window
		if verboseMode then
			set contents of text view "log" of scroll view "log" of window "log" to theResult
			show window "log"
		end if
		
		-- Hide the progres panel (if shown)
		if showProgress then
			hideProgressPanel(true)
		end if
	end if
end panel ended


(* ==== Handlers ==== *)

-- This is the bread and butter of the application. It simply creates the command to be issued to 'do shell script' and returns the result.
-- 
on makeArchive(archiveName)
	-- The 'gnutar' command in it's basic strucure.
	set scriptCommand to "gnutar " & getOptionsString() & " -f " & archiveName
	
	-- Add each of the file items to the command.
	repeat with fileName in fileNames
		set scriptCommand to scriptCommand & space & fileName
	end repeat
	
	-- Tell the shell to do it's thing.
	return do shell script scriptCommand
end makeArchive

-- Returns the various options chosen by the user in a simple string beginning with the required '-c' which is used to tell 'gnutar' to create a new archive. You can do a 'man gnutar' to see all of the options in a terminal window.
-- 
on getOptionsString()
	set optionsString to "-c"
	
	if compressArchive then
		set optionsString to optionsString & "z"
	end if
	if preserveIDs then
		set optionsString to optionsString & "p"
	end if
	if followLinks then
		set optionsString to optionsString & "h"
	end if
	if verboseMode then
		set optionsString to optionsString & "v"
	end if
	
	return optionsString
end getOptionsString

-- Returns a self determined archive name based on the first item in the file item list.
-- 
on getArchiveFileName()
	set archiveFileName to ""
	
	-- Prepend the file name with the default location
	if defaultLocation is not equal to "" then
		set archiveFileName to defaultLocation
		if archiveFileName does not end with "/" then
			set archiveFileName to archiveFileName & "/"
		end if
	end if
	
	-- Append the last word of the first item plus a '.tar'  or '.tar.gz' (which is the normal extension for tar files.
	set archiveFileName to archiveFileName & last word of (item 1 of fileNames as string) & ".tar"
	if compressArchive then set archiveFileName to archiveFileName & ".gz"
	
	return archiveFileName
end getArchiveFileName

-- Loads the progress panel (if needed) and then displays it.
-- 
on showProgressPanel(attachedToWindow, archiveFileName)
	-- Only load the progress panel once.
	if progressPanel is missing value then
		load nib "ProgressPanel"
		set progressPanel to window "progress"
	end if
	
	-- Set the status item in the progress panel
	set content of text field "status" of progressPanel to "Making Archive: " & (call method "lastPathComponent" of archiveFileName)
	
	-- Display the progress panel appropriately.
	if attachedToWindow then
		display panel progressPanel attached to window "main"
	else
		show progressPanel
	end if
	
	-- Start spinning the progress bar.
	tell progressPanel
		set uses threaded animation of progress indicator "progress" to true
		tell progress indicator "progress" to start
	end tell
end showProgressPanel

-- Hides the progress panel.
-- 
on hideProgressPanel(attachedToWindow)
	if attachedToWindow then
		tell progress indicator "progress" of progressPanel to stop
		close panel progressPanel
	else
		hide progressPanel
	end if
	
	-- Set the status item in the progress panel
	set content of text field "status" of progressPanel to ""
end hideProgressPanel

-- Shows the main window, doing any necessary setup of the drawer as necessary.
-- 
on showWindow()
	tell window "main"
		tell drawer "settings"
			-- Initialize some settings to appropriate values for the settings drawer. These will set the current, min and max contents size to be the same, which will have the effect of keeping the settings drawer size appropriate to it's contents. (In other words it can't grow or shrink.) 
			set leading offset to 20
			set trailing offset to 20
			set content size to {436, 136}
			set minimum content size to {436, 136}
			set maximum content size to {436, 136}
			
			-- Set the UI settings
			my setSettingsInUI()
		end tell
		
		set visible to true
	end tell
	
	set windowOpened to true
end showWindow

-- Shows the current list of file names as a list of strings in the text view of the main window.
-- 
on updateFileNamesInUI()
	tell window "main"
		set AppleScript's text item delimiters to return
		set contents of text view "files" of scroll view "files" to fileNames as string
		set AppleScript's text item delimiters to ""
	end tell
end updateFileNamesInUI

-- Prompts the user to select a default location for new archives.
-- 
on chooseDefaultLocation()
	-- Setup the open panel properties
	tell open panel
		set can choose directories to true
		set can choose files to false
		set prompt to "Choose"
	end tell
	
	display open panel attached to window "main"
end chooseDefaultLocation

-- Show's the settings drawer, also adjusting the title of the 'settings' button.
-- 
on showSettings()
	tell window "main"
		tell drawer "settings" to open drawer on bottom edge
		set title of button "settings" to "Hide Settings"
	end tell
end showSettings

-- Hide's the settings drawer, also adjusting the title of the 'settings' button.
-- 
on hideSettings()
	tell window "main"
		tell drawer "settings" to close drawer
		set title of button "settings" to "Show Settings"
	end tell
end hideSettings

-- Sets the settings properties based on the states of the various UI items in the settings drawer.
-- 
on getSettingsFromUI()
	tell drawer "settings" of window "main"
		set defaultLocation to contents of text field "default location"
		set openWindowOnLaunch to (state of button "open window") as boolean
		set showProgress to (state of button "show progress") as boolean
		set compressArchive to (state of button "compress archive") as boolean
		set preserveIDs to (state of button "preserve ids") as boolean
		set followLinks to (state of button "follow links") as boolean
		set verboseMode to (state of button "verbose mode") as boolean
	end tell
end getSettingsFromUI

-- Sets the state of the UI elements int he settings drawer based upon the settings properties.
-- 
on setSettingsInUI()
	tell drawer "settings" of window "main"
		set contents of text field "default location" to defaultLocation
		set state of button "open window" to openWindowOnLaunch
		set state of button "show progress" to showProgress
		set state of button "compress archive" to compressArchive
		set state of button "preserve ids" to preserveIDs
		set state of button "follow links" to followLinks
		set state of button "verbose mode" to verboseMode
	end tell
end setSettingsInUI

-- Registers the settings (application preferences) with the 'user defaults'. 
-- 
on registerSettings()
	tell user defaults
		-- Add all of the new defalt entries
		make new default entry at end of default entries with properties {name:"openWindowOnLaunch", contents:openWindowOnLaunch}
		make new default entry at end of default entries with properties {name:"showProgress", contents:showProgress}
		make new default entry at end of default entries with properties {name:"compressArchive", contents:compressArchive}
		make new default entry at end of default entries with properties {name:"preserveIDs", contents:preserveIDs}
		make new default entry at end of default entries with properties {name:"followLinks", contents:followLinks}
		make new default entry at end of default entries with properties {name:"verboseMode", contents:verboseMode}
		make new default entry at end of default entries with properties {name:"defaultLocation", contents:defaultLocation}
		
		-- Now we need to register the new entries in the user defaults
		register
	end tell
end registerSettings

-- Reads the settings (application preferences) from the 'user defaults'. 
-- 
on readSettings()
	tell user defaults
		set openWindowOnLaunch to contents of default entry "openWindowOnLaunch" as boolean
		set showProgress to contents of default entry "showProgress" as boolean
		set compressArchive to contents of default entry "compressArchive" as boolean
		set preserveIDs to contents of default entry "preserveIDs" as boolean
		set followLinks to contents of default entry "followLinks" as boolean
		set verboseMode to contents of default entry "verboseMode" as boolean
		set defaultLocation to contents of default entry "defaultLocation"
	end tell
end readSettings

-- Writes the settings (application preferences) to the 'user defaults'. 
-- 
on writeSettings()
	tell user defaults
		set contents of default entry "openWindowOnLaunch" to openWindowOnLaunch
		set contents of default entry "showProgress" to showProgress
		set contents of default entry "compressArchive" to compressArchive
		set contents of default entry "preserveIDs" to preserveIDs
		set contents of default entry "followLinks" to followLinks
		set contents of default entry "verboseMode" to verboseMode
		set contents of default entry "defaultLocation" to defaultLocation
	end tell
end writeSettings


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
