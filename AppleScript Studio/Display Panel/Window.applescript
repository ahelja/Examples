(* Window.applescript *)

(* This script demonstrates the "display panel" command which allows you to create your own dialogs and have them displayed either as a dialog or attached to a window as a sheet. *)

(* ==== Properties ==== *)

property panelWIndow : missing value

(* ==== Event Handlers ==== *)

on clicked theObject
	set theName to contents of text field "name" of window "main"
	set theType to contents of text field "type" of window "main"
	
	-- Load the panel. We do this by loading the nib that contains the panel window, and then setting our property to the loaded window. Only do this once, as every time the nib is loaded, it will create new copies of all of the top level objects in the nib.
	if panelWIndow is equal to missing value then
		load nib "SettingsPanel"
		set panelWIndow to window "settings"
	end if
	
	-- Set the state of the items in the panel
	tell panelWIndow
		set contents of text field "name" to theName
		if theType is "Button" then
			set current row of matrix "type" to 1
		else if theType is "Popup Button" then
			set current row of matrix "type" to 2
		else if theType is "Radio" then
			set current row of matrix "type" to 3
		else if theType is "Switch" then
			set current row of matrix "type" to 4
		end if
	end tell
	
	-- Display the panel
	if state of button "as sheet" of window "main" is 1 then
		display panel panelWIndow attached to window "main"
	else
		if (display panel panelWIndow) is 1 then
			local theName
			local theType
			
			tell panelWIndow
				set theName to contents of text field "name"
				set selectedRow to current row of matrix "type"
				
				if selectedRow is 1 then
					set theType to "Button"
				else if selectedRow is 2 then
					set theType to "Popup Button"
				else if selectedRow is 3 then
					set theType to "Radio"
				else if selectedRow is 4 then
					set theType to "Switch"
				end if
			end tell
			
			set contents of text field "name" of window "main" to theName
			set contents of text field "type" of window "main" to theType
		end if
	end if
	
end clicked

on panel ended thePanel with result theResult
	if theResult is 1 then
		local theName
		local theType
		
		tell thePanel
			set theName to contents of text field "name"
			set selectedRow to current row of matrix "type"
			
			if selectedRow is 1 then
				set theType to "Button"
			else if selectedRow is 2 then
				set theType to "Popup Button"
			else if selectedRow is 3 then
				set theType to "Radio"
			else if selectedRow is 4 then
				set theType to "Switch"
			end if
		end tell
		
		set contents of text field "name" of window "main" to theName
		set contents of text field "type" of window "main" to theType
	end if
end panel ended


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)