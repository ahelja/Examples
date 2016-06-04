(*Open Panel.applescript *)

(* This example demonstrates how to use the 'open-panel' class, either as a modal panel or as a panel attached to a window. The 'open panel' is a property of the application object. *)

(* ==== Event Handlers ==== *)

-- This event handler is called when the "Display Open Panel" button is clicked, which when clicked the various parameter values pulled from the text fields to be sent to "display". 
--
on clicked theObject
	-- Get the values from the UI
	tell window of theObject
		set theTitle to contents of text field "title"
		set thePrompt to contents of text field "prompt"
		set theFileTypes to contents of text field "file types"
		set theDirectory to contents of text field "directory"
		set theFileName to contents of text field "file name"
		set treatPackages to contents of button "treat packages" as boolean
		set canChooseDirectories to contents of button "choose directories" as boolean
		set canChooseFiles to contents of button "choose files" as boolean
		set allowsMultiple to contents of button "multiple selection" as boolean
		set asSheet to contents of button "sheet" as boolean
		
		-- Convert the comma separated list of file type to an actual list
		set AppleScript's text item delimiters to ", "
		set theFileTypes to text items of theFileTypes
		set AppleScript's text item delimiters to ""
	end tell
	
	-- Setup the properties in the 'open panel'
	tell open panel
		set title to theTitle
		set prompt to thePrompt
		set treat packages as directories to treatPackages
		set can choose directories to canChooseDirectories
		set can choose files to canChooseFiles
		set allows multiple selection to allowsMultiple
	end tell
	
	-- Determine which way to display the panel
	if asSheet then
		-- Display the panel as sheet (in which case the result will happen in 'on panel ended').
		-- One thing to note is that the script will not stop processing until the panel is presented but continues on. You must use the 'on panel ended' event handler to get notified when the panel has finished.
		-- The 'in directory' and 'with file name' parameters are optional.
		if (count of theFileTypes) is 0 then
			display open panel in directory theDirectory with file name theFileName attached to window of theObject
		else
			display open panel in directory theDirectory with file name theFileName for file types theFileTypes attached to window of theObject
		end if
	else
		-- Display the panel.
		-- Unlike the 'attached to' variant, the script does stop processing until the panel is finished.
		-- The 'in directory' and 'with file name' parameters are optional
		if (count of theFileTypes) is 0 then
			set theResult to display open panel in directory theDirectory with file name theFileName for file types theFileTypes
		else
			set theResult to display open panel in directory theDirectory with file name theFileName
		end if
		
		if theResult is 1 then
			-- Convert the list into a list of strings separated by return characters that we can put in the 'path names' text view
			-- For some unknown (as of yet) you must coerce the 'path names' to a list (even though it is defined as list).
			set the pathNames to (path names of open panel as list)
			set AppleScript's text item delimiters to return
			set the pathNames to pathNames as string
			set AppleScript's text item delimiters to ""
			
			set contents of text view "path names" of scroll view "path names" of window "main" to pathNames
		else
			set contents of text view "path names" of scroll view "path names" of window "main" to ""
		end if
	end if
end clicked

-- This event handler is called when the panel presented with the 'display attached to' command is finished.
--
on panel ended theObject with result withResult
	if withResult is 1 then
		-- Convert the list into a list of strings separated by return characters that we can put in the 'path names' text view
		-- For some unknown (as of yet) you must coerce the 'path names' to a list (even though it is defined as list).
		set the pathNames to (path names of open panel as list)
		set AppleScript's text item delimiters to return
		set the pathNames to pathNames as string
		set AppleScript's text item delimiters to ""
		
		set contents of text view "path names" of scroll view "path names" of window "main" to pathNames
	else
		set contents of text view "path names" of scroll view "path names" of window "main" to ""
	end if
end panel ended

(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)