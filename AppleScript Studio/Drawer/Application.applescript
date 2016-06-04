(* Application.applescript *)

(* This is an example that demonstrates how to show and hide a drawer, as well as change all of the various settings of a drawer, including the leading/trailing offsets and the various content sizes. *)

(* ==== Event Handlers ==== *)

-- This event handler is called when any of the attached UI elements are clicked. One thing of note in the handling of clicking on stepper objects: you need to update the value of the text fields based on the value of the stepper in order to keep them in sync.
--
on clicked theObject
	tell window "main"
		if theObject is equal to button "drawer" then
			set currentState to state of drawer "drawer"
			set openOnSide to current row of matrix "open on"
			
			-- Show/Hide the drawer as appropriate as well as updating the state text fields.
			if currentState is equal to drawer closed or currentState is equal to drawer closing then
				if openOnSide is equal to 1 then
					tell drawer "drawer" to open drawer on left edge
				else if openOnSide is equal to 2 then
					tell drawer "drawer" to open drawer on top edge
				else if openOnSide is equal to 3 then
					tell drawer "drawer" to open drawer on right edge
				else if openOnSide is equal to 4 then
					tell drawer "drawer" to open drawer on bottom edge
				end if
				set title of button "drawer" to "Close Drawer"
				set contents of text field "drawer state" to "Opened"
			else if currentState is equal to drawer opened or currentState is equal to drawer opening then
				tell drawer "drawer" to close drawer
				set title of button "drawer" to "Open Drawer"
				set contents of text field "drawer state" to "Closed"
			end if
		else if theObject is equal to stepper "leading offset" then
			set theValue to (contents of stepper "leading offset") as integer
			set leading offset of drawer "drawer" to theValue
			set contents of text field "leading offset" to theValue
		else if theObject is equal to stepper "trailing offset" then
			set theValue to (contents of stepper "trailing offset") as integer
			set trailing offset of drawer "drawer" to theValue
			set contents of text field "trailing offset" to theValue
		else if theObject is equal to stepper "content width" then
			set theValue to (contents of stepper "content width") as integer
			set contentSize to content size of drawer "drawer"
			set item 1 of contentSize to theValue
			set content size of drawer "drawer" to contentSize
			set contents of text field "content width" to theValue
		else if theObject is equal to stepper "content height" then
			set theValue to (contents of stepper "content height") as integer
			set contentSize to content size of drawer "drawer"
			set item 2 of contentSize to theValue
			set content size of drawer "drawer" to contentSize
			set contents of text field "content height" to theValue
		else if theObject is equal to stepper "minimum width" then
			set theValue to (contents of stepper "minimum width") as integer
			set minimumSize to minimum content size of drawer "drawer"
			set item 1 of minimumSize to theValue
			set minimum content size of drawer "drawer" to minimumSize
			set contents of text field "minimum width" to theValue
		else if theObject is equal to stepper "minimum height" then
			set theValue to (contents of stepper "minimum height") as integer
			set minimumSize to minimum content size of drawer "drawer"
			set item 2 of minimumSize to theValue
			set minimum content size of drawer "drawer" to minimumSize
			set contents of text field "minimum height" to theValue
		else if theObject is equal to stepper "maximum width" then
			set theValue to (contents of stepper "maximum width") as integer
			set maximumSize to maximum content size of drawer "drawer"
			set item 1 of maximumSize to theValue
			set maximum content size of drawer "drawer" to maximumSize
			set contents of text field "maximum width" to theValue
		else if theObject is equal to stepper "maximum height" then
			set theValue to (contents of stepper "maximum height") as integer
			set maximumSize to maximum content size of drawer "drawer"
			set item 2 of maximumSize to theValue
			set maximum content size of drawer "drawer" to maximumSize
			set contents of text field "maximum height" to theValue
		end if
	end tell
end clicked

-- This event handler is called when the text value of the attached text fields are changed. One thing of note in the handling of text fields with stepper objects: you need to update the value of the stepper based on the value of the text field in order to keep them in sync.
--
on action theObject
	set textValue to contents of theObject
	
	tell window "main"
		if theObject is equal to text field "leading offset" then
			set leading offset of drawer "drawer" to textValue
			set contents of stepper "leading offset" to textValue
		else if theObject is equal to text field "trailing offset" then
			set trailing offset of drawer "drawer" to textValue
			set contents of stepper "trailing offset" to textValue
		else if theObject is equal to text field "content width" then
			set theValue to (contents of text field "content width") as integer
			set contentSize to content size of drawer "drawer"
			set item 1 of contentSize to theValue
			set content size of drawer "drawer" to contentSize
			set contents of stepper "content width" to theValue
		else if theObject is equal to text field "content height" then
			set theValue to (contents of text field "content height") as integer
			set contentSize to content size of drawer "drawer"
			set item 2 of contentSize to theValue
			set content size of drawer "drawer" to contentSize
			set contents of stepper "content height" to theValue
		else if theObject is equal to text field "minimum width" then
			set theValue to (contents of text field "minimum width") as integer
			set minimumSize to minimum content size of drawer "drawer"
			set item 1 of minimumSize to theValue
			set minimum content size of drawer "drawer" to minimumSize
			set contents of stepper "minimum width" to theValue
		else if theObject is equal to text field "minimum height" then
			set theValue to (contents of text field "minimum height") as integer
			set minimumSize to minimum content size of drawer "drawer"
			set item 2 of minimumSize to theValue
			set minimum content size of drawer "drawer" to minimumSize
			set contents of stepper "minimum height" to theValue
		else if theObject is equal to text field "maximum width" then
			set theValue to (contents of text field "maximum width") as integer
			set maximumSize to maximum content size of drawer "drawer"
			set item 1 of maximumSize to theValue
			set maximum content size of drawer "drawer" to maximumSize
			set contents of stepper "maximum width" to theValue
		else if theObject is equal to text field "maximum height" then
			set theValue to (contents of text field "maximum height") as integer
			set maximumSize to maximum content size of drawer "drawer"
			set item 2 of maximumSize to theValue
			set maximum content size of drawer "drawer" to maximumSize
			set contents of stepper "maximum height" to theValue
		end if
	end tell
end action

-- This event handler is called when the attached window is loaded from the nib file. It's a good place to set up the values of all of the UI elements based on the current drawer settings.
--
on awake from nib theObject
	tell theObject
		set openOnEdge to edge of drawer "drawer"
		set preferredEdge to preferred edge of drawer "drawer"
		
		-- Set the drawer up with some initial values.
		set leading offset of drawer "drawer" to 20
		set trailing offset of drawer "drawer" to 20
		
		-- Update the UI to match the settings of the drawer.
		if state of drawer "drawer" is drawer closed then
			set contents of text field "drawer state" to "Closed"
		else if state of drawer "drawer" is drawer opened then
			set contents of text field "drawer state" to "Opened"
		end if
		
		if openOnEdge is left edge then
			set current row of matrix "open on" to 1
		else if openOnEdge is top edge then
			set current row of matrix "open on" to 2
		else if openOnEdge is right edge then
			set current row of matrix "open on" to 3
		else if openOnEdge is bottom edge then
			set current row of matrix "open on" to 4
		end if
		
		if preferredEdge is left edge then
			set current row of matrix "prefer on" to 1
		else if preferredEdge is top edge then
			set current row of matrix "prefer on" to 2
		else if preferredEdge is right edge then
			set current row of matrix "prefer on" to 3
		else if preferredEdge is bottom edge then
			set current row of matrix "prefer on" to 4
		end if
		
		set leadingValue to leading offset of drawer "drawer"
		set trailingValue to trailing offset of drawer "drawer"
		set contentSize to content size of drawer "drawer"
		set minimumContentSize to minimum content size of drawer "drawer"
		set maximumContentSize to maximum content size of drawer "drawer"
		
		set contents of text field "leading offset" to leadingValue
		set contents of stepper "leading offset" to leadingValue
		set contents of text field "trailing offset" to trailingValue
		set contents of stepper "trailing offset" to trailingValue
		set contents of text field "content width" to item 1 of contentSize
		set contents of stepper "content width" to item 1 of contentSize
		set contents of text field "content height" to item 2 of contentSize
		set contents of stepper "content height" to item 2 of contentSize
		set contents of text field "minimum width" to item 1 of minimumContentSize
		set contents of stepper "minimum width" to item 1 of minimumContentSize
		set contents of text field "minimum height" to item 2 of minimumContentSize
		set contents of stepper "minimum height" to item 2 of minimumContentSize
		set contents of text field "maximum width" to item 1 of maximumContentSize
		set contents of stepper "maximum width" to item 1 of maximumContentSize
		set contents of text field "maximum height" to item 2 of maximumContentSize
		set contents of stepper "maximum height" to item 2 of maximumContentSize
	end tell
end awake from nib

on launched theObject
	show window "main"
end launched


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)