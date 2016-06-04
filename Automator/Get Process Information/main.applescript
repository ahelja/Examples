-- main.applescript
-- Get Process Information

on run {input, parameters}
	
	if input is in {{}, {""}, ""} or |ignoresInput| of parameters is true then -- ignoresInput is the value of the type setting in the action's title bar
		-- if input is empty or being ignored then get process from popup button selection
		set processNameVal to (|processName| of parameters) as Unicode text
	else
		-- if input is not empty then use input as process name
		if class of input is list then
			set processNameVal to item 1 of input
			if processNameVal is in {{}, ""} then
				set processNameVal to (|processName| of parameters) as Unicode text
			end if
		else
			set processNameVal to input
		end if
	end if
	
	set ReturnAppleScriptRecordVal to (|ReturnAppleScriptRecord| of parameters)
	
	if ReturnAppleScriptRecordVal = 1 then
		(*
		Since this is a compiled script, we use the 'run script' command to load in System Events' AppleScript terminology. 
		Otherwise the resulting AppleScript record will appear in chevron syntax (four byte character codes) in the View Results action.
		*)
		set theScript to "tell application \"System Events\" to properties of process \"" & processNameVal & "\""
		set processInfo to run script theScript
	else
		tell application "System Events"
			set processInfo to properties of process processNameVal
		end tell
		set counter to count of processInfo
		-- Get localized strings for properties labels
		set accepts_high_level_events to localized_string("accepts high level events")
		set accepts_remote_events to localized_string("accepts remote events")
		set background_only to localized_string("background only")
		set ClassicVal to localized_string("Classic")
		set creator_type to localized_string("creator type")
		set displayed_name to localized_string("displayed name")
		set fileVal to localized_string("file")
		set file_type to localized_string("file type")
		set frontmostVal to localized_string("frontmost")
		set has_scripting_terminology to localized_string("has scripting terminology")
		set idVal to localized_string("id")
		set nameVal to localized_string("name")
		set partition_space_used to localized_string("partition space used")
		set total_partition_size to localized_string("total partition size")
		set unix_id to localized_string("unix id")
		set visibleVal to localized_string("visible")
		-- Create text report
		tell application "System Events"
			set theTextData to accepts_high_level_events & ":" & tab & accepts high level events of processInfo & return & Â
				accepts_remote_events & ":" & tab & accepts remote events of processInfo & return & Â
				background_only & ":" & tab & background only of processInfo & return & Â
				ClassicVal & ":" & tab & Classic of processInfo & return & Â
				creator_type & ":" & tab & creator type of processInfo & return & Â
				displayed_name & ":" & tab & displayed name of processInfo & return & Â
				fileVal & ":" & tab & file of processInfo & return & Â
				file_type & ":" & tab & file type of processInfo & return & Â
				frontmostVal & ":" & tab & frontmost of processInfo & return & Â
				has_scripting_terminology & ":" & tab & has scripting terminology of processInfo & return & Â
				idVal & ":" & tab & id of processInfo & return & Â
				nameVal & ":" & tab & name of processInfo & return & Â
				partition_space_used & ":" & tab & partition space used of processInfo & return & Â
				total_partition_size & ":" & tab & total partition size of processInfo & return & Â
				unix_id & ":" & tab & unix id of processInfo & return & Â
				visibleVal & ":" & tab & visible of processInfo & return
		end tell
		set processInfo to theTextData
	end if
	return processInfo
end run

on localized_string(key_string)
	return localized string key_string in bundle with identifier "com.apple.AutomatorExamples.GetProcessInformation"
end localized_string

(* © Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (ÒAppleÓ) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in this original Apple software (the ÒApple SoftwareÓ), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)

