(* Display Dialog.applescript *)

(* This example will demonstrate the various ways of using the "display dialog" command. The dialog can be displayed as a dialog, or attached to a window as sheet. *)

(* ==== Event Handlers ==== *)

-- This event handler is called when the "Display Dialog" button is clicked. It gets the various settings from the UI elements and passes them to "display dialog" as parameters.
--
on clicked theObject
	-- Initialize all the parameter values that will be passed to display dialog
	tell window "main"
		set dialogText to contents of text field "text"
		set dialogDefaultAnswer to contents of text field "default answer"
		set dialogButton1 to contents of text field "button 1"
		
		set dialogButton2 to contents of text field "button 2"
		set dialogButton3 to contents of text field "button 3"
		set dialogDefaultButton to contents of text field "default button"
		set dialogIcon to contents of text field "icon"
		set dialogGivingUpAfter to contents of text field "giving up" as number
	end tell
	
	-- If we want to have the display dialog presented as a sheet, then we need add the optional parameter "attached to" passing it a window object
	if state of button "as sheet" of window "main" is equal to 1 then
		if dialogDefaultAnswer is "" then
			display dialog dialogText buttons {dialogButton1, dialogButton2, dialogButton3} default button dialogDefaultButton giving up after dialogGivingUpAfter with icon dialogIcon attached to window "main"
		else
			display dialog dialogText default answer dialogDefaultAnswer buttons {dialogButton1, dialogButton2, dialogButton3} default button dialogDefaultButton giving up after dialogGivingUpAfter with icon dialogIcon attached to window "main"
		end if
	else
		-- Otherwise we do it the standard way
		try
			if dialogDefaultAnswer is "" then
				set theReply to display dialog dialogText buttons {dialogButton1, dialogButton2, dialogButton3} default button dialogDefaultButton giving up after dialogGivingUpAfter with icon dialogIcon
			else
				set theReply to display dialog dialogText default answer dialogDefaultAnswer buttons {dialogButton1, dialogButton2, dialogButton3} default button dialogDefaultButton giving up after dialogGivingUpAfter with icon dialogIcon
			end if
			
			-- Set the values returned from the dialog reply
			set contents of text field "text returned" of window "main" to text returned of theReply
			set contents of text field "button returned" of window "main" to button returned of theReply
			set state of button "gave up" of window "main" to gave up of theReply
		on error
			-- The user pressed the "Cancel" button, so display that as the result. We can't use the "theReply" value because it wasn't returned from the "display dialog" call, because of the cancel.
			set contents of text field "button returned" of window "main" to "Cancel"
		end try
	end if
end clicked

-- This handler gets called when the display dialog dialog if finished if it was called with the "attached to" optional parameter.
on dialog ended theObject with reply theReply
	-- Set the values returned in "theReply"
	set contents of text field "text returned" of window "main" to text returned of theReply
	set contents of text field "button returned" of window "main" to button returned of theReply
	set state of button "gave up" of window "main" to gave up of theReply
end dialog ended


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)