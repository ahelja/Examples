(* Preferences.applescript *)

(* This script shows an example of using the workaround for reading and writing user defaults. You can use a number, string, or array for objects to be stored in the defaults.  *)

(* ==== Properties ==== *)

property preferencesWindow : null


(* ==== Event Handlers ==== *)

-- This event handler is called when the "preferences" menu item is chosen.
-- 
on choose menu item theObject
	-- Only load the preferences nib once
	if preferencesWindow is equal to null then
		load nib "Preferences"
		set preferencesWindow to window "preferences"
	end if
	
	-- Load in the preferences
	loadPreferences(preferencesWindow)
	
	-- Show the preferences window
	set visible of preferencesWindow to true
end choose menu item


-- This event handler is called when either the "cancel" or "done" buttons are clicked.
-- 
on clicked theObject
	if name of theObject is "done" then
		-- Save out the preferences
		storePreferences(preferencesWindow)
	end if
	
	-- Hide the preferences window
	set visible of preferencesWindow to false
end clicked


(* ==== Handlers ==== *)

-- This handler will read the preferences from the "Support.plist" in  the ~/Library/Preferences directory and then sets those values in the UI elements.
--
on loadPreferences(theWindow)
	-- Read in the preferences
	set theText to call method "defaultObjectForKey:" with parameter "text"
	set theNumber to call method "defaultObjectForKey:" with parameter "number"
	set thePopup to call method "defaultObjectForKey:" with parameter "popup"
	set theSlider to call method "defaultObjectForKey:" with parameter "slider"
	set theRadio to call method "defaultObjectForKey:" with parameter "radio"
	set theSwitches to call method "defaultObjectForKey:" with parameter "switches"
	
	-- Set the contents of the UI elements
	tell theWindow
		set contents of text field "text" to theText
		set contents of text field "number" to theNumber
		set contents of popup button "popup" to thePopup
		set contents of slider "slider" to theSlider
		set current row of matrix "radio" to theRadio
		set contents of button "show" to item 1 of theSwitches
		set contents of button "hide" to item 2 of theSwitches
	end tell
end loadPreferences

-- This handler will get the values from the UI elements and store those values in the  preferences file.
--
on storePreferences(theWindow)
	-- Get the contents of the UI elements
	tell theWindow
		set theText to contents of text field "text"
		set theNumber to contents of text field "number"
		set thePopup to contents of popup button "popup"
		set theSlider to contents of slider "slider"
		set theRadio to current row of matrix "radio"
		set theSwitches to {contents of button "show", contents of button "hide"}
	end tell
	
	-- Write out the preferences
	call method "setDefaultObject:forKey:" with parameters {theText, "text"}
	call method "setDefaultObject:forKey:" with parameters {theNumber, "number"}
	call method "setDefaultObject:forKey:" with parameters {thePopup, "popup"}
	call method "setDefaultObject:forKey:" with parameters {theSlider, "slider"}
	call method "setDefaultObject:forKey:" with parameters {theRadio, "radio"}
	call method "setDefaultObject:forKey:" with parameters {theSwitches, "switches"}
end storePreferences


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
