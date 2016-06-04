(* WithoutDataSource.applescript *)

(* This script is used to demonstrate the scripting of a table view without using a data source. The important part of supplying the table with information is in the "cell value" and "number of rows" event handlers. The table will query the script asking it for the number of rows, and then for every row of every column the "number of rows" event handler will be called, returning the contents of the cell for the table to display. *)

(* ==== Properties ==== *)

property contacts : {}
property contactIndex : 0


(* ==== Event Handlers ==== *)

on clicked theObject
	if name of theObject is "add" then
		-- Add a new contact
		tell window of theObject
			-- Create a contact record from the values in the text fields and add it to the list of contacts 
			set contacts to contacts & {my getContactInfo(window of theObject)}
			
			-- Tell the table view to update it's values
			tell table view "contacts" of scroll view "contacts" to update
			
			-- Clear out the contact information
			my clearContactInfo(window of theObject)
		end tell
		
	else if name of theObject is "update" then
		-- Update the contact
		tell window of theObject
			-- Update the contact information
			set item contactIndex of contacts to my getContactInfo(window of theObject)
			
			-- Tell the table view to update it's values
			tell table view "contacts" of scroll view "contacts" to update
		end tell
	else if name of theObject is "remove" then
		-- Remove the contact
		if contactIndex > 0 and contactIndex ≤ (count of contacts) then
			tell window of theObject
				-- Remove the contact form the list
				set contacts to my deleteItemInList(contactIndex, contacts)
				
				-- Tell the table view to update it's values
				tell table view "contacts" of scroll view "contacts" to update
				
				-- Clear out the contact information
				my clearContactInfo(window of theObject)
			end tell
		end if
	end if
end clicked

-- Return the value of the specified column for the given row
-- 
on cell value theObject row theRow table column theColumn
	-- Set the value to an empty string for now
	set theValue to ""
	
	-- Make sure that we aren't being asked for a row that is greater than the number of contacts
	if (count of contacts) ≥ theRow then
		set theContact to item theRow of contacts
		
		-- Get the identifier of the column so that we can determine which field of the record to return
		set theID to identifier of theColumn
		if the theID is "name" then
			set theValue to name of theContact
		else if theID is "address" then
			set theValue to address of theContact
		else if theID is "city" then
			set theValue to city of theContact
		else if theID is "state" then
			set theValue to state of theContact
		else if theID is "zip" then
			set theValue to zip of theContact
		end if
	end if
	
	-- Now return the value that we set
	return theValue
end cell value

-- Return the number of contacts
--
on number of rows theObject
	return count of contacts
end number of rows

on selection changed theObject
	if name of theObject is "contacts" then
		set theWindow to window of theObject
		
		-- Set the contact index to the current row, so that we can use it to update the right contact later
		set contactIndex to selected row of theObject
		
		if contactIndex = 0 then
			-- There wasn't any selected so clear the contact information
			my clearContactInfo(theWindow)
			
			-- Disable the "Update" and "Remove" buttons
			set enabled of button "update" of theWindow to false
			set enabled of button "remove" of theWindow to false
		else
			-- A contact was selected, so show the contact information
			my setContactInfo(theWindow, item contactIndex of contacts)
			
			-- Enable the "Update" and "Remove" buttons
			set enabled of button "update" of theWindow to true
			set enabled of button "remove" of theWindow to true
		end if
	end if
end selection changed


(* ==== Contact Handlers ==== *)

-- Empty all of the text fields
--
on clearContactInfo(theWindow)
	tell theWindow
		set contents of text field "name" to ""
		set contents of text field "address" to ""
		set contents of text field "city" to ""
		set contents of text field "state" to ""
		set contents of text field "zip" to ""
		set first responder to text field "name"
	end tell
end clearContactInfo

-- Get the values from the text fields and return a contact record
--
on getContactInfo(theWindow)
	tell theWindow
		return {name:contents of text field "name", address:contents of text field "address", city:contents of text field "city", state:contents of text field "state", zip:contents of text field "zip"}
	end tell
end getContactInfo

-- Set the text fields with the values from the contact
-- 
on setContactInfo(theWindow, theContact)
	tell theWindow
		set contents of text field "name" to name of theContact
		set contents of text field "address" to address of theContact
		set contents of text field "city" to city of theContact
		set contents of text field "state" to state of theContact
		set contents of text field "zip" to zip of theContact
	end tell
end setContactInfo

(* ==== Utilities ==== *)

on deleteItemInList(x, theList)
	set x to (x as number)
	if x < 1 then return theList
	set numItems to count of items in theList
	if numItems is 1 then return {}
	if x > numItems then return theList
	if x = 1 then
		set newList to (items 2 thru -1 of theList)
	else if x = numItems then
		set newList to (items 1 thru -2 of theList)
	else
		set newList to (items 1 thru (x - 1) of theList) & (items (x + 1) thru -1 of theList)
	end if
	return newList
end deleteItemInList


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)