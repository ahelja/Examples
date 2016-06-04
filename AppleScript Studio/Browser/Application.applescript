(* Application.applescript *)

(* This example demonstrates how to script a browser object. The main parts of the script are the "number of browser rows" event handler which needs to return number of rows in the browser for the given column, and the "will display browser cell" event handler that will be called for every item in the browser. *)

(* ==== Properties ==== *)

property diskNames : {}


(* ==== Event Handlers ==== *)

-- Initialize various items here
--
on launched theObject
	tell application "Finder"
		set diskNames to name of every disk as list
	end tell
	
	set path separator of browser "browser" of window "main" to ":"
	
	tell browser "browser" of window "main" to update
end launched

-- Return the number of rows for the given column
--
on number of browser rows theObject in column theColumn
	set rowCount to 0
	
	if (count of diskNames) > 0 then
		if theColumn is 1 then
			set rowCount to count of diskNames
		else
			tell browser "browser" of window "main"
				set thePath to path for column theColumn - 1
			end tell
			
			tell application "Finder"
				set rowCount to count of items of item thePath
			end tell
		end if
	end if
	
	return rowCount
end number of browser rows

-- This is called whenever a cell in the browser needs to be displayed.
--
on will display browser cell theObject row theRow browser cell theCell in column theColumn
	if theColumn > 1 then
		tell browser "browser" of window "main"
			set thePath to path for column theColumn
		end tell
	end if
	
	tell application "Finder"
		if theColumn is 1 then
			set cellContents to displayed name of disk (item theRow of diskNames as string)
			set isLeaf to false
		else
			set theItem to item theRow of item thePath
			
			if class of theItem is folder or class of theItem is disk then
				set isLeaf to false
			else
				set isLeaf to true
			end if
			
			set cellContents to (displayed name of theItem as string)
		end if
	end tell
	
	set string value of theCell to cellContents
	set leaf of theCell to isLeaf
	
end will display browser cell


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)