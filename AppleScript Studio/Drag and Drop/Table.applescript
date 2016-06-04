(* Table.applescript *)

(* This script is used to register the appropriate drag types for the "table view" object and then responds to a drop on it. *)

(* ==== Event Handlers ==== *)

-- The "awake from nib" event handler is a good place to register the drag types that this object can respond to.
--
on awake from nib theObject
	-- Create the data source for the table view
	set theDataSource to make new data source at end of data sources with properties {name:"files"}
	
	-- Create the "files" data column
	make new data column at end of data columns of theDataSource with properties {name:"files"}
	
	-- Assign the data source to the table view
	set data source of theObject to theDataSource
	
	-- Register for the "color" and "file names" drag types
	tell theObject to register drag types {"file names", "color"}
end awake from nib

-- The "drop" event handler is called when the appropriate type of data is dropped onto the object. All of the pertinent information about the drop is contained in the "dragInfo" object.
--
on drop theObject drag info dragInfo
	-- Get the list of data types on the pasteboard
	set dataTypes to types of pasteboard of dragInfo
	
	-- We are only interested in either "file names" or "color" data types
	if "file names" is in dataTypes then
		-- Initialize the list of files to an empty list
		set theFiles to {}
		
		-- We want the data as a list of file names, so set the preferred type to "file names"
		set preferred type of pasteboard of dragInfo to "file names"
		
		-- Get the list of files from the pasteboard
		set theFiles to contents of pasteboard of dragInfo
		
		-- Make sure we have at least one item
		if (count of theFiles) > 0 then
			--- Get the data source from the table view
			set theDataSource to data source of theObject
			
			-- Turn off the updating of the views
			set update views of theDataSource to false
			
			-- Delete all of the data rows in the data source
			delete every data row of theDataSource
			
			-- For every item in the list, make a new data row and set it's contents
			repeat with theItem in theFiles
				set theDataRow to make new data row at end of data rows of theDataSource
				set contents of data cell "files" of theDataRow to theItem
			end repeat
			
			-- Turn back on the updating of the views
			set update views of theDataSource to true
		end if
	else if "color" is in dataTypes then
		-- We want the data as a color, so set the preferred type
		set preferred type of pasteboard of dragInfo to "color"
		
		-- Set the background color of the table view to the color on the pasteboard
		set background color of theObject to contents of pasteboard of dragInfo
		
		-- We need to update the table view (redraw it).
		update theObject
	end if
	
	-- Set the preferred type back to the default
	set preferred type of pasteboard of dragInfo to ""
	
	return true
end drop


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
