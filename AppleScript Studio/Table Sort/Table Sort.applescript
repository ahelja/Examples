(* Table Sort.applescript *)

(* This example demonstrates how easy it is to add sorting to your tables. In the example, you can click on different columns to sort on that column. Clicking more than once in the same column changes the sort order of that column. *)

(* ==== Properties ==== *)

property tableData : {{|name|:"Bart Simpson", city:"Springfield", zip:"19542", age:12}, {|name|:"Ally McBiel", city:"Chicago", zip:"91544", age:28}, {|name|:"Joan of Ark", city:"Paris", zip:"53255", age:36}, {|name|:"King Tut", city:"Egypt", zip:"00245", age:45}, {|name|:"James Taylor", city:"Atlanta", zip:"21769", age:42}}


(* ==== Event Handlers ==== *)

-- The "awake from nib" event handler is attached to the table view. It will be called when the table view is loaded from the nib. It's a good place to create our data source and set up the data columns.
--
on awake from nib theObject
	-- Create the data source
	set theDataSource to make new data source at end of data sources with properties {name:"names"}
	
	-- Create each of the data columns, including the sort information for each column
	make new data column at end of data columns of theDataSource with properties {name:"name", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
	make new data column at end of data columns of theDataSource with properties {name:"city", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
	make new data column at end of data columns of theDataSource with properties {name:"zip", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
	make new data column at end of data columns of theDataSource with properties {name:"age", sort order:ascending, sort type:numerical, sort case sensitivity:case sensitive}
	
	-- Make this a sorted data source
	set sorted of theDataSource to true
	
	-- Set the "name" data column as the sort column
	set sort column of theDataSource to data column "name" of theDataSource
	
	-- Set the data source of the table view to the new data source
	set data source of theObject to theDataSource
	
	-- Add the table data (using the new "append" command)
	append theDataSource with tableData
end awake from nib


-- The "launched" event handler is attached to the application object ("File's Owner of MainMenu.nib"). It is called towards the end of the startup sequence.
--
on launched theObject
	-- Show the main window
	show window "main"
end launched


-- The "column clicked" event handler is called when the user clicks on a table column in the table view. We will use this handler to change the sort column of the data source as well as the sort order.
--
on column clicked theObject table column tableColumn
	-- Get the data source of the table view
	set theDataSource to data source of theObject
	
	-- Get the identifier of the clicked table column
	set theColumnIdentifier to identifier of tableColumn
	
	-- Get the current sort column of the data source
	set theSortColumn to sort column of theDataSource
	
	-- If the current sort column is not the same as the clicked column then switch the sort column
	if (name of theSortColumn) is not equal to theColumnIdentifier then
		set the sort column of theDataSource to data column theColumnIdentifier of theDataSource
	else
		-- Otherwise change the sort order
		if sort order of theSortColumn is ascending then
			set sort order of theSortColumn to descending
		else
			set sort order of theSortColumn to ascending
		end if
	end if
	
	-- We need to update the table view (so it will be redrawn)
	update theObject
end column clicked


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
