(* Coordinate System.applescript *)

(* This is an example of how to use the new coordinate system support. *)

(* ===== Event Handlers ===== *)

on launched theObject
	show window "main"
end launched

-- This event handler is attached to the window and will be called when it is loaded. It is a good time to update the display in the window to show the current coordinates.
--
on awake from nib theObject
	updateDisplay(theObject)
end awake from nib

-- This event handler is called when the button in our document window is clicked. It will test the various settings of the coordinate system by moving the window and then by moving the button.
--
on clicked theObject
	set theWindow to window of theObject
	set testObject to theWindow
	
	-- Test the Cocoa coordinate system on a window. This system uses {x, y, width, height}, with the origin of a window or view in the lower left corner being 0, 0
	set coordinate system to Cocoa coordinate system
	set testBounds to bounds of testObject
	set testPosition to position of testObject
	set bounds of testObject to {50, 50, 500, 500}
	delay 1
	set position of testObject to {150, 150}
	updateDisplay(theWindow)
	delay 1
	
	-- Test the old (classic) coordinate system on a window. This system uses {left, bottom, right, top}, with the origin of a window or view in the bottom left corner being 0, 0
	set coordinate system to classic coordinate system
	set testBounds to bounds of testObject
	set testPosition to position of testObject
	set bounds of testObject to {50, 50, 500, 500}
	delay 1
	set position of testObject to {150, 150}
	updateDisplay(theWindow)
	delay 1
	
	-- Test the AppleScript coordinate system on a window. This system uses {left, top, right, bottom}, with the origin of a window or view in the top left corner being 0, 0
	set coordinate system to AppleScript coordinate system
	set testBounds to bounds of testObject
	set testPosition to position of testObject
	set bounds of testObject to {50, 50, 500, 500}
	delay 1
	set position of testObject to {150, 150}
	updateDisplay(theWindow)
	delay 1
	
	set testObject to theObject
	
	-- Test the Cocoa coordinate system on our button. This system uses {x, y, width, height}, with the origin of a window or view in the lower left corner being 0, 0
	set coordinate system to Cocoa coordinate system
	set testBounds to bounds of testObject
	set testPosition to position of testObject
	set bounds of testObject to {0, 0, 82, 30}
	delay 1
	set position of testObject to {10, 10}
	updateDisplay(theWindow)
	delay 1
	
	-- Test the old (classic) coordinate system on our button. This system uses {left, bottom, right, top}, with the origin of a window or view in the bottom left corner being 0, 0
	set coordinate system to classic coordinate system
	set testBounds to bounds of testObject
	set testPosition to position of testObject
	set bounds of testObject to {0, 0, 82, 30}
	delay 1
	set position of testObject to {10, 10}
	updateDisplay(theWindow)
	delay 1
	
	-- Test the AppleScript coordinate system on our button. This system uses {left, top, right, bottom}, with the origin of a window or view in the top left corner being 0, 0
	set coordinate system to AppleScript coordinate system
	set testBounds to bounds of testObject
	set testPosition to position of testObject
	set bounds of testObject to {0, 0, 82, 30}
	delay 1
	set position of testObject to {10, 10}
	updateDisplay(theWindow)
end clicked

-- This event handler is called when the coordinate system popup button is changed. It will change the coordinate system and update the display.
--
on action theObject
	set popupChoice to content of theObject
	
	if popupChoice is 0 then
		set coordinate system to Cocoa coordinate system
	else if popupChoice is 1 then
		set coordinate system to classic coordinate system
	else if popupChoice is 2 then
		set coordinate system to AppleScript coordinate system
	end if
	
	updateDisplay(window of theObject)
end action

-- This event handler is called when the window moves. It will update the display to show the current coordinates.
--
on moved theObject
	updateDisplay(theObject)
end moved

-- This event handler is called when the window resizes. It will update the display to show the current coordinates.
--
on resized theObject
	updateDisplay(theObject)
end resized

(* =====  Handlers ===== *)

-- This handler is used to get the coordinates of the window and button and display a description in the window.
--
on updateDisplay(theWindow)
	set theButton to button "button" of theWindow
	
	set windowBounds to bounds of theWindow
	set windowPosition to position of theWindow
	set buttonBounds to bounds of theButton
	set buttonPosition to position of theButton
	
	if coordinate system is Cocoa coordinate system then
		set coordinateSystemDescription to 0
		set windowBoundsDescription to "{x: " & item 1 of windowBounds & ", y: " & item 2 of windowBounds & ", w: " & item 3 of windowBounds & ", h: " & item 4 of windowBounds & "}"
		set windowPositionDescription to "{x: " & item 1 of windowPosition & ", y: " & item 2 of windowPosition & "}"
		set buttonBoundsDescription to "{x: " & item 1 of buttonBounds & ", y: " & item 2 of buttonBounds & ", w: " & item 3 of buttonBounds & ", h: " & item 4 of buttonBounds & "}"
		set buttonPositionDescription to "{x: " & item 1 of buttonPosition & ", y: " & item 2 of buttonPosition & "}"
	else if coordinate system is classic coordinate system then
		set coordinateSystemDescription to 1
		set windowBoundsDescription to "{l: " & item 1 of windowBounds & ", b: " & item 2 of windowBounds & ", r: " & item 3 of windowBounds & ", t: " & item 4 of windowBounds & "}"
		set windowPositionDescription to "{l: " & item 1 of windowPosition & ", b: " & item 2 of windowPosition & "}"
		set buttonBoundsDescription to "{l: " & item 1 of buttonBounds & ", b: " & item 2 of buttonBounds & ", r: " & item 3 of buttonBounds & ", t: " & item 4 of buttonBounds & "}"
		set buttonPositionDescription to "{l: " & item 1 of buttonPosition & ", b: " & item 2 of buttonPosition & "}"
	else if coordinate system is AppleScript coordinate system then
		set coordinateSystemDescription to 2
		set windowBoundsDescription to "{l: " & item 1 of windowBounds & ", t: " & item 2 of windowBounds & ", r: " & item 3 of windowBounds & ", b: " & item 4 of windowBounds & "}"
		set windowPositionDescription to "{l: " & item 1 of windowPosition & ", t: " & item 2 of windowPosition & "}"
		set buttonBoundsDescription to "{l: " & item 1 of buttonBounds & ", t: " & item 2 of buttonBounds & ", r: " & item 3 of buttonBounds & ", b: " & item 4 of buttonBounds & "}"
		set buttonPositionDescription to "{l: " & item 1 of buttonPosition & ", t: " & item 2 of buttonPosition & "}"
	end if
	
	tell theWindow
		set content of popup button "coordinate system" to coordinateSystemDescription
		set content of text field "window bounds" to windowBoundsDescription
		set content of text field "window position" to windowPositionDescription
		set content of text field "button bounds" to buttonBoundsDescription
		set content of text field "button position" to buttonPositionDescription
	end tell
end updateDisplay

(* © Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
