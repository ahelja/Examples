-- UI.applescript
-- Get Process Information

property actionview_reference : missing value
property contentview_reference : missing value
property action_parameters : missing value

on awake from nib theObject
	set contentview_reference to theObject
	set actionview_reference to the super view of contentview_reference
	set the action_parameters to (call method "parameters" of (call method "action" of the actionview_reference))
	getProcesses()
end awake from nib

on parameters updated theObject parameters theParameters
	set (the title of popup button "processesMenu" of contentview_reference) to |processName| of theParameters as Unicode text
	set (the state of button "backgroundProcessesButton" of contentview_reference) to |backgroundProcesses| of theParameters as integer
	set (the state of button "ReturnAppleScriptRecord" of contentview_reference) to |ReturnAppleScriptRecord| of theParameters as integer
	return theParameters
end parameters updated

on update parameters theObject parameters theParameters
	set |processName| of theParameters to (the title of popup button "processesMenu" of contentview_reference) as Unicode text
	set |backgroundProcesses| of theParameters to (the state of button "backgroundProcessesButton" of contentview_reference)
	set |ReturnAppleScriptRecord| of theParameters to (the state of button "ReturnAppleScriptRecord" of contentview_reference)
	return theParameters
end update parameters

on clicked theObject
	-- This handler is attached to the check box buttons
	if name of theObject is "backgroundProcessesButton" then
		getProcesses()
	else if name of theObject is "ReturnAppleScriptRecord" then
		-- There are problems targeting the Automator process when returning a record, so we don't present this process to user
		set CurrentProcessTitle to (title of popup button "processesMenu" of contentview_reference) --as Unicode text
		if CurrentProcessTitle contains "Automator" then
			getProcesses()
		end if
	end if
end clicked


on will pop up theObject
	-- This handler is attached to the popup button
	tell progress indicator "ProgressIndicator" of contentview_reference to start
	set CurrentProcessTitle to (title of theObject) --as Unicode text
	getProcesses()
	set (title of theObject) to CurrentProcessTitle
	tell progress indicator "ProgressIndicator" of contentview_reference to stop
end will pop up

on getProcesses()
	set |backgroundProcesses| of action_parameters to (the state of button "backgroundProcessesButton" of contentview_reference)
	set backgroundProcessVal to (|backgroundProcesses| of action_parameters)
	set ReturnAppleScriptRecordVar to (the state of button "ReturnAppleScriptRecord" of contentview_reference)
	
	delete every menu item of menu of popup button "processesMenu" of contentview_reference
	if backgroundProcessVal = 1 then
		tell application "System Events"
			set processList to name of every process
		end tell
	else
		tell application "System Events"
			set processList to name of every process whose background only is false
		end tell
	end if
	repeat with processTitle in processList
		-- There are problems targeting the Automator process when returning a record, so we don't present this process to user
		if ReturnAppleScriptRecordVar = 1 then
			if processTitle does not contain "Automator" then
				make new menu item at the end of menu items of menu of popup button "processesMenu" of contentview_reference with properties {title:processTitle, enabled:true}
			end if
		else
			make new menu item at the end of menu items of menu of popup button "processesMenu" of contentview_reference with properties {title:processTitle, enabled:true}
		end if
	end repeat
end getProcesses

(* © Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
