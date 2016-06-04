(* Application.applescript *)

(* ==== Event Handlers ==== *)

on will open theObject
	set movie of movie view "movie" of window "main" to load movie "jumps"
end will open

on choose menu item theMenuItem
	tell window "main"
		set theCommand to tag of theMenuItem
		
		if theCommand is equal to 1001 then
			set moviePath to choose file
			set movie file of movie view "movie" to moviePath
		else if theCommand is equal to 1002 then
			tell movie view "movie" to play
		else if theCommand is equal to 1003 then
			tell movie view "movie" to stop
		else if theCommand is equal to 1004 then
			tell movie view "movie" to step forward
		else if theCommand is equal to 1005 then
			tell movie view "movie" to step back
		else if theCommand is equal to 1006 then
			tell movie view "movie" to go to beginning frame
		else if theCommand is equal to 1007 then
			tell movie view "movie" to go to end frame
		else if theCommand is equal to 1008 then
			tell movie view "movie" to go to poster frame
		else if theCommand is equal to 1009 then
			set loop mode of movie view "movie" to normal playback
		else if theCommand is equal to 1010 then
			set loop mode of movie view "movie" to looping playback
		else if theCommand is equal to 1011 then
			set loop mode of movie view "movie" to looping back and forth playback
		end if
	end tell
end choose menu item

on update menu item theMenuItem
	tell window "main"
		local enableItem
		set enableItem to 1
		
		set theCommand to tag of theMenuItem
		set thePlayBack to loop mode of movie view "movie"
		
		if theCommand is equal to 1002 then
			if playing of movie view "movie" is true then set enableItem to 0
		else if theCommand is equal to 1003 then
			if playing of movie view "movie" is false then set enableItem to 0
		else if theCommand ≥ 1009 and theCommand ≤ 1011 then
			set theState to 0
			
			if thePlayBack is equal to normal playback and theCommand is equal to 1009 then set theState to 1
			if thePlayBack is equal to looping playback and theCommand is equal to 1010 then set theState to 1
			if thePlayBack is equal to looping back and forth playback and theCommand is equal to 1011 then set theState to 1
			
			set state of theMenuItem to theState
		end if
	end tell
	
	return enableItem
end update menu item


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)