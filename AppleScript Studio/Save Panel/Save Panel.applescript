(*Save Panel.applescript *)

(* This example demonstrates how to use the 'save-panel' class, either as a modal panel or as a panel attached to a window. The 'save panel' is a property of the application object. *)

(* ==== Event Handlers ==== *)

-- This event handler is called when the "Display Save Panel" button is clicked, which when clicked the various parameter values pulled from the text fields to be sent to "display". 
--
on clicked theObject
	-- Get the values from the UI
	tell window of theObject
		set theTitle to contents of text field "title"
		set thePrompt to contents of text field "prompt"
		set theFileType to contents of text field "file type"
		set theDirectory to contents of text field "directory"
		set theFileName to contents of text field "file name"
		set treatPackages to contents of button "treat packages" as boolean
		set asSheet to contents of button "sheet" as boolean
	end tell
	
	-- Setup the properties in the 'save panel'
	tell save panel
		set title to theTitle
		set prompt to thePrompt
		set required file type to theFileType
		set treat packages as directories to treatPackages
	end tell
	
	-- Determine which way to display the panel
	if asSheet then
		-- Display the panel as sheet (in which case the result will happen in 'on panel ended').
		-- One thing to note is that the script will not stop processing until the panel is presented but continues on. You must use the 'on panel ended' event handler to get notified when the panel has finished.
		-- The 'in directory' and 'with file name' parameters are optional.
		display save panel in directory theDirectory with file name theFileName attached to window of theObject
	else
		-- Display the panel.
		-- Unlike the 'attached to' variant, the script does stop processing until the panel is finished.
		-- The 'in directory' and 'with file name' parameters are optional
		set theResult to display save panel in directory theDirectory with file name theFileName
		if theResult is 1 then
			set contents of text field "path name" of window "main" to path name of save panel
		else
			set contents of text field "path name" of window "main" to ""
		end if
	end if
end clicked

-- This event handler is called when the panel presented with the 'display attached to' command is finished.
--
on panel ended theObject with result withResult
	if withResult is 1 then
		set contents of text field "path name" of window "main" to path name of save panel
	else
		set contents of text field "path name" of window "main" to ""
	end if
end panel ended

(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)