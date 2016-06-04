(* WithDataSource.applescript *)

(* This script is used to demonstrate the scripting of a table view using a data source that is connected to the table view in Interface Builder. Basically the data source has columns added in the "will open" event handler, the "data rows" are added/updated/removed as need from the data source. *)

(* ==== Properties ==== *)

property contactsDataSource : null


(* ==== Event Handlers ==== *)

on clicked theObject
	if name of theObject is equal to "add" then
		-- Add a new contact
		set theRow to make new data row at the end of the data rows of contactsDataSource
		getContactInfo(window of theObject, theRow)
		
		-- Clear out the contact information
		clearContactInfo(window of theObject)
	else if name of theObject is "update" then
		set tableView to table view "contacts" of scroll view "contacts" of window of theObject
		set selectedDataRows to selected data rows of tableView
		if (count of selectedDataRows) > 0 then
			-- Update the contact
			getContactInfo(window of theObject, item 1 of selectedDataRows)
			
			-- Tell the table view to update it's values
			tell tableView to update
		end if
	else if name of theObject is "remove" then
		set tableView to table view "contacts" of scroll view "contacts" of window of theObject
		set selectedDataRows to selected data rows of tableView
		if (count of selectedDataRows) > 0 then
			tell window of theObject
				-- Remove the contact form the data source
				delete (item 1 of selectedDataRows)
				
				-- Clear out the contact information
				my clearContactInfo(window of theObject)
			end tell
		end if
	end if
end clicked

on will open theObject
	-- Set up the contactDataSource so that the rest will be simpler
	set contactsDataSource to data source of table view "contacts" of scroll view "contacts" of theObject
	
	-- Here we will add the data columns to the data source of the contacts table view
	tell contactsDataSource
		make new data column at the end of the data columns with properties {name:"name"}
		make new data column at the end of the data columns with properties {name:"address"}
		make new data column at the end of the data columns with properties {name:"city"}
		make new data column at the end of the data columns with properties {name:"state"}
		make new data column at the end of the data columns with properties {name:"zip"}
	end tell
end will open

on selection changed theObject
	if name of theObject is "contacts" then
		set theWindow to window of theObject
		
		-- Set the contact index to the current row, so that we can use it to update the right contact later
		set selectedDataRows to selected data rows of theObject
		
		if (count of selectedDataRows) = 0 then
			-- There wasn't any selected so clear the contact information
			my clearContactInfo(theWindow)
			
			-- Disable the "Update" and "Remove" buttons
			set enabled of button "update" of theWindow to false
			set enabled of button "remove" of theWindow to false
		else
			-- A contact was selected, so show the contact information
			my setContactInfo(theWindow, item 1 of selectedDataRows)
			
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

-- Get the values from the text fields and set the cells in the the data row
--
on getContactInfo(theWindow, theRow)
	tell theWindow
		set contents of data cell "name" of theRow to contents of text field "name"
		set contents of data cell "address" of theRow to contents of text field "address"
		set contents of data cell "city" of theRow to contents of text field "city"
		set contents of data cell "state" of theRow to contents of text field "state"
		set contents of data cell "zip" of theRow to contents of text field "zip"
	end tell
end getContactInfo

-- Set the text fields with the values from the contact
-- 
on setContactInfo(theWindow, theRow)
	tell theWindow
		set contents of text field "name" to contents of data cell "name" of theRow
		set contents of text field "address" to contents of data cell "address" of theRow
		set contents of text field "city" to contents of data cell "city" of theRow
		set contents of text field "state" to contents of data cell "state" of theRow
		set contents of text field "zip" to contents of data cell "zip" of theRow
	end tell
end setContactInfo


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)