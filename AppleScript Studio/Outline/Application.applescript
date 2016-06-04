(* Application.applescript *)

(* This example illustrates how to script an outline view. *)

(* ==== Properties ==== *)

property diskNames : {}


(* ==== Event Handlers ==== *)

on launched theObject
	try
		tell application "Finder"
			set diskNames to name of every disk
		end tell
	end try
	tell outline view "outline" of scroll view "scroll" of window "main" to update
end launched

on number of items theObject outline item theItem
	set itemCount to 0
	try
		tell application "Finder"
			if (count of diskNames) > 0 then
				if theItem is 0 then
					set itemCount to count of diskNames
				else
					set itemCount to count of items of (get item theItem)
				end if
			end if
		end tell
	end try
	return itemCount
end number of items

on child of item theObject outline item theItem child theChild
	set childItem to ""
	try
		tell application "Finder"
			if theItem is 0 then
				set childItem to disk (get item theChild of diskNames as string) as string
			else
				set childItem to item theChild of (get item theItem) as string
			end if
		end tell
	end try
	return childItem
end child of item

on item expandable theObject outline item theItem
	set isExpandable to false
	try
		if theItem is 0 then
			if (count of diskNames) is greater than 1 then
				set isExpandable to true
			end if
		else
			tell application "Finder"
				if (count of items of (get item theItem)) is greater than 1 then
					set isExpandable to true
				end if
			end tell
		end if
	end try
	return isExpandable
end item expandable

on item value theObject outline item theItem table column theColumn
	set itemValue to ""
	try
		if the identifier of theColumn is "name" then
			tell application "Finder"
				set itemValue to displayed name of (get item theItem) as string
			end tell
		else if the identifier of theColumn is "date" then
			tell application "Finder"
				set itemValue to modification date of (get item theItem) as string
			end tell
		else if the identifier of theColumn is "kind" then
			tell application "Finder"
				set itemValue to kind of (get item theItem) as string
			end tell
		end if
	end try
	return itemValue
end item value

on will open theObject
	try
		tell application "Finder"
			set diskNames to name of every disk
		end tell
	end try
	tell outline view "outline" of scroll view "scroll" of window "main" to update
end will open


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
