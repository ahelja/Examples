(* Tool Helper.applescript *)

(* This example will help to find shell commands and then provide a window containing the man page for that command. You choose how to search by choosing from several choices in a popup button: "begins with", "contains", "ends with" and "is". The strategy employed is to get a list of all of the command names at starup and then search through that list when requested, displaying the results of the ones found. *)


(* ==== Properties ==== *)

property commandsDataSource : missing value
property commandNames : {}
property manPageWindow : missing value


(* ==== Event Handlers ==== *)

-- The "will finish launching" event handler is the first event handler called in the startup sequence and is a good place to do any type of initialization work that doesn't require any UI. For this example we will get a list of all of the command names.
--
on will finish launching theObject
	-- The quickest method of getting a list of all of the command names appears to be to get the information using "ls" in a "do shell script". We want to get a list of all of the commands from the following locations: /bin, /usr/bin, /usr/sbin. We can do this by concating the commands together with the ";" character and then piping ("|") the results through the "sort" shell command passing it the "-u" option which eliminates any duplicates. We then take the result from the do shell command (which will be a string with return characters between each item) and convert it to a list of strings.
	set commandNames to every paragraph of (do shell script "ls /usr/bin ; ls /usr/sbin ; ls /bin | sort -u")
end will finish launching


-- The "awake from nib" event handler is called when the object is loaded from a nib file. It's a good place to initialize one or more items.
--
on awake from nib theObject
	if name of theObject is "main" then
		-- When the window is loaded, be sure to hide the status items
		hideStatus(theObject)
	else if name of theObject is "man page" then
		-- If the man page window is being loaded then set a reference to it
		set manPageWindow to theObject
	else if name of theObject is "commands" then
		-- Create the data source
		set commandsDataSource to make new data source at end of data sources with properties {name:"commands"}
		
		-- Create the data columns
		make new data column at end of data columns of commandsDataSource with properties {name:"command"}
		make new data column at end of data columns of commandsDataSource with properties {name:"description"}
		
		-- Assign the data source to the table view
		set data source of theObject to commandsDataSource
	end if
end awake from nib


-- The "launched" is one of the last event handlers that is called in the startup sequence. In this case we want to show our main window.
--
on launched theObject
	show window "main"
end launched


-- The "clicked" event handler is called (in this example) when the "Find" button is clicked. We then initiate our find process.
--
on clicked theObject
	if name of theObject is "find" then
		findCommands(window of theObject)
	end if
end clicked


on double clicked theObject
	if name of theObject is "commands" then
		-- Show and update the message items in the main window
		showStatus(window of theObject)
		updateStatusMessage(window of theObject, "Getting the man page...")
		
		-- Get the clicked row of the table view
		set theRow to clicked row of theObject
		set theDataRow to data row theRow of data source of theObject
		
		-- Get the name of the command
		set theCommandName to contents of data cell "command" of theDataRow
		
		-- See if the window is already open
		set theWindow to findWindowWithTitle(theCommandName)
		if theWindow is not missing value then
			-- Just bring it to the front
			show theWindow
		else
			-- Load a new instance of the man page window and show it
			load nib "ManPage"
			set title of manPageWindow to theCommandName
			
			-- Get the man page for the command, cleaning it up in the process
			set theResult to do shell script "man " & theCommandName & " | perl -pe 's/.\\x08//g'"
			
			-- Put the results into the text view of our man page window
			set contents of text view "man page" of scroll view "man page" of manPageWindow to theResult
			
			-- Show the window
			show manPageWindow
		end if
		
		-- Hide the status items
		hideStatus(window of theObject)
	end if
end double clicked


-- The "action" event handler is called (in this example) when a menu item is chosen from the popup button. We then initiate our find process.
--
on action theObject
	if name of theObject is "how" then
		findCommands(window of theObject)
	end if
end action


(* ==== Handlers ==== *)

-- This handler is called to find any commands that meet the criteria specified in the UI (how and what). It also is responsible for providing any feedback during the find, such as showing, updating and hiding the status items in the window.
--
on findCommands(theWindow)
	-- Show the the status items
	showStatus(theWindow)
	updateStatusMessage(theWindow, "Finding commands...")
	
	-- Find the commands with what coming from the text field, and how coming from the popup button
	set theCommands to commandsWithName(contents of text field "name" of theWindow, title of popup button "how" of theWindow)
	
	-- Turn off the updating of the table view while we load the data source
	set update views of commandsDataSource to false
	
	-- Delete any existing items in the data source
	delete every data row of commandsDataSource
	
	-- Make sure that we actually found at least one command
	if (count of theCommands) > 0 then
		-- Update the status message
		updateStatusMessage(theWindow, "Adding commands...")
		
		-- Add the list of commands to the data source using the "append" command
		append commandsDataSource with theCommands
	end if
	
	-- Turn back on the updating of the table view
	set update views of commandsDataSource to true
	
	-- Hide the status items
	hideStatus(theWindow)
end findCommands


-- This handler is used to look through our list of command names, returning a list of found commands, which also includes getting and returning the description of the command
--
on commandsWithName(whatToFind, howToFind)
	-- Set our result to a known good value, in this case an empty list will do just fine
	set theCommands to {}
	
	-- Make sure that we have a value to find for
	if (count of whatToFind) > 0 then
		-- Set our found names list to an empty list
		set foundCommandNames to {}
		
		-- Based on the "howToFind" repeat through each of the command names in our commandNames list finding the appropriate items and adding it to the foundCommandNames list
		if howToFind is "begins with" then
			repeat with i in commandNames
				if i begins with whatToFind then
					copy i to end of foundCommandNames
				end if
			end repeat
		else if howToFind is "contains" then
			repeat with i in commandNames
				if i contains whatToFind then
					copy i to end of foundCommandNames
				end if
			end repeat
		else if howToFind is "ends with" then
			repeat with i in commandNames
				if i ends with whatToFind then
					copy i to end of foundCommandNames
				end if
			end repeat
		else if howToFind is "is" then
			repeat with i in commandNames
				if (i as string) is equal to whatToFind then
					copy i to end of foundCommandNames
				end if
			end repeat
		end if
		
		-- Make sure that we found at least one command name
		if (count of foundCommandNames) > 0 then
			-- Iterate through each of the found names
			repeat with i in foundCommandNames
				try
					set theDescription to ""
					
					-- We will use the "whatis" shell command to get the description of 
					set theResult to do shell script ("whatis " & (i as string))
					
					-- Unfortunately, the result will look something like "more(1), page(1)         - file perusal filter for crt viewing". We only want to get portion of the text following the " - " characters. This can be done using the following bit of script.
					set dashoffset to offset of " - " in theResult
					set firstReturn to offset of return in theResult
					set theDescription to characters (dashoffset + 2) through (firstReturn - 1) of theResult as string
					
					-- Add the command name and description as a list the end of our command list
					copy {i, theDescription} to end of theCommands
				end try
			end repeat
		end if
	end if
	
	-- Return our result
	return theCommands
end commandsWithName


(* ==== Status Handlers ==== *)

-- This handler will show the various status items in the window, along with starting the animation of the progress indicator
--
on showStatus(theWindow)
	tell theWindow
		set visible of progress indicator "progress" to true
		set visible of text field "status" to true
		set uses threaded animation of progress indicator "progress" to true
		start progress indicator "progress"
	end tell
end showStatus


-- This handler will hide all of the status items in the window, including stopping the animation of the progress indicator
--
on hideStatus(theWindow)
	tell theWindow
		set visible of progress indicator "progress" to false
		set visible of text field "status" to false
		stop progress indicator "progress"
	end tell
end hideStatus


-- This handler will update the status message in the status items of the window
--
on updateStatusMessage(theWindow, theMessage)
	set contents of text field "status" of theWindow to theMessage
end updateStatusMessage


(* ==== Utility Handlers ==== *)

-- This is a utility handler that will simply find the window with the specified title.
--
on findWindowWithTitle(theTitle)
	set theWindow to missing value
	
	set theWindows to every window whose title is theTitle
	if (count of theWindows) > 0 then
		set theWindow to item 1 of theWindows
	end if
	
	return theWindow
end findWindowWithTitle

(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
