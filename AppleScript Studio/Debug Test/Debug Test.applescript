(* Debug Test.applescript *)

(* The purpose of this example is to illustrate the debugging of AppleScript. Many of the properties and values are there mainly to test the debugger in it's ability to show and set the various values. It also illustrates the ability to interact with the UI while in the processing of executing a script. *)

(* ==== Properties ==== *)

property keepRunning : true
property prop1 : "Test property 1"
property prop2 : "Test property 2"
property prop3 : 0


(* ==== Event Handlers ==== *)

-- Here we handle the click on the "Start/Stop" button, toggling between states as necessary.
--
on clicked theObject
	if title of theObject is "Start" then
		set keepRunning to true
		set title of theObject to "Stop"
		set theResult to 2
		runforever()
	else if title of theObject is "Stop" then
		set title of theObject to "Start"
		set keepRunning to false
	end if
end clicked

-- This handler is called after the window is loaded, but before it is displayed.
--
on will open theObject
	set prop3 to 10
end will open

-- This event handler is called just before the window is closed. If you want to stop the window from being closed, you can use the "should close" event handler and return false.
--
on will close theObject
	set keepRunning to false
end will close


(* ==== Handlers ==== *)

-- This is a handler that is called to do a repeat loop until the keepRunning variable gets changed to false. It also animates the barber pole and set the value of the text field.
--
on runforever()
	set numberTest to 1
	set stringTest to "testing"
	
	runonce()
	
	repeat while keepRunning
		tell progress indicator "Barber Pole" of window "Main" to animate
		set prop3 to prop3 + 1
		set numberTest to numberTest + 1
		
		set contents of text field "counter" of window "Main" to numberTest as string
	end repeat
end runforever

on runonce()
	set prop3 to prop3 + 1
	set prop3 to prop3 + 1
	set prop3 to prop3 + 1
	set prop3 to prop3 + 1
	
	runonceagain()
end runonce

on runonceagain()
	set prop3 to prop3 + 1
	set prop3 to prop3 + 1
	set prop3 to prop3 + 1
	set prop3 to prop3 + 1
	
	runlasttime()
end runonceagain

on runlasttime()
	set prop3 to prop3 + 1
end runlasttime


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)