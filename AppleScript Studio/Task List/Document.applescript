(* Document.applescript *)

(* This is a good example of several different features of AppleScript Studio. The main one is to demonstrate how to write a document bases application using the higher level handlers "data representation" and "load data representation" (as opposed to the lower level handlers "write to file" and "read from file". It also demonstrates how to work with a table view (including support for sorting). Menu Item handling is also included in this example. *)

(* ==== Event Handlers ==== *)

-- The "awake from nib" handler is called (in this example) when the table view is loaded from the "Documents.nib" nib file. This is a good place to create a new data source and data columns and set various properties of said items.
--
on awake from nib theObject
	if name of theObject is "tasks" then
		-- Create the data source for our "tasks" table view
		set theDataSource to make new data source at end of data sources with properties {name:"tasks"}
		
		-- Create the data columns, "priority", "task" and "status". We also set the sort properties of each of the data columns, including the sort order, the type of data in each column and what type of sensitivity to use.
		make new data column at end of data columns of theDataSource with properties {name:"priority", sort order:ascending, sort type:numerical, sort case sensitivity:case sensitive}
		make new data column at end of data columns of theDataSource with properties {name:"task", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
		make new data column at end of data columns of theDataSource with properties {name:"status", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
		
		-- Set the data source as sorted
		set sorted of theDataSource to true
		
		-- Set the "priority" data column as the sort column
		set sort column of theDataSource to data column "priority" of theDataSource
		
		-- Finally, assign the data source of the table view to our data source
		set data source of theObject to theDataSource
	end if
end awake from nib


-- The "action" event handler is called whenever the user chooses a menu in the popup buttons or presses (in this example) the enter key in the text field.
--
on action theObject
	-- Set some local variables to various objects in the UI
	set theWindow to window of theObject
	set theTableView to table view "tasks" of scroll view "tasks" of theWindow
	set theDataSource to data source of theTableView
	
	-- The behavior from here will be determined by whether or not an task in the table view is selected
	if (count of selected rows of theTableView) is 0 then
		-- Since nothing is selected we will create a new task (but only if the enter key is pressed in the "task" text field)
		if name of theObject is "task" then
			-- Make a new data row
			set theTask to make new data row at end of data rows of theDataSource
			
			-- Populate the task using values in the UI
			setTaskValuesWithUIValues(theTask, theWindow)
			
			-- Now set the UI to default values
			setDefaultUIValues(theWindow)
			
			-- Make the "task" text field the object with the focus so that it will be ready for typing
			set first responder of theWindow to text field "task" of theWindow
		end if
	else
		-- Get the selected task from the table view
		set theTask to selected data row of theTableView
		
		-- See which object was touched
		if name of theObject is "priority" then
			set contents of data cell "priority" of theTask to title of theObject
		else if name of theObject is "task" then
			set contents of data cell "task" of theTask to content of theObject
		else if name of theObject is "status" then
			set contents of data cell "status" of theTask to title of theObject
		end if
	end if
end action


(* ==== Document Event Handlers ==== *)

-- The "data representation" event handler is called when the document needs to be saved. It is the responsiblity of the handler to return the data that is to be saved. This can be nearly any AppleScript object, whether it be a string, a list, a record, etc. In this case we are going to return a record that contains the list of tasks, the name of the current sort column and the sort order of the current sort column. 
--
on data representation theObject of type ofType
	-- Set some local variables to various objects in the UI
	set theWindow to window 1 of theObject
	set theDataSource to data source of table view "tasks" of scroll view "tasks" of theWindow
	set theTasks to contents of every data cell of every data row of theDataSource
	set theSortColumn to sort column of theDataSource
	
	-- Create our record containing the list of tasks (just a list of lists), the name of the sort column and the sort order.
	set theData to {tasks:theTasks, sortColumnName:name of theSortColumn, sortColumnOrder:sort order of theSortColumn}
	
	return theData
end data representation


-- The "load data representation" event handler is called when the document is being loaded. The data that you provided in the "data representation" event handler is passed to you in the "theData" parameter.
--
on load data representation theObject of type ofType with data theData
	-- Set some local variables to various objects in the UI
	set theWindow to window 1 of theObject
	set theDataSource to data source of table view "tasks" of scroll view "tasks" of theWindow
	
	-- Restore the sort column and sort order of the data source based on the information saved
	set sort column of theDataSource to data column (sortColumnName of theData) of theDataSource
	set sort order of sort column of theDataSource to (sortColumnOrder of theData)
	
	-- Use the "append" verb to quickly populate the data source with the list of tasks
	append the theDataSource with (tasks of theData)
	
	-- We return true, signaling that everything worked correctly. If you return "false" then the document will fail to load and an alert will be presented.
	return true
end load data representation


(* ==== Data View Event Handlers ==== *)

-- The "selection changing" event handler is called whenever the selection in the table view is changing. We will use this to update the values in the UI based on the selection.
--
on selection changing theObject
	if name of theObject is "tasks" then
		-- If there is a selection then we'll update the UI, otherwise we set the UI to default values
		if (count of selected rows of theObject) > 0 then
			-- Get the selected data row of the table view
			set theTask to selected data row of theObject
			
			-- Update the UI using the selected task
			setUIValuesWithTaskValues(window of theObject, theTask)
		else
			-- Set the UI to default values
			setDefaultUIValues(window of theObject)
		end if
	end if
end selection changing


-- The "selection changing" event handler is called whenever the selection in the table view is changing. We will use this to update the values in the UI based on the selection.
--
on selection changed theObject
	if name of theObject is "tasks" then
		-- If there is a selection then we'll update the UI, otherwise we set the UI to default values
		if (count of selected rows of theObject) > 0 then
			-- Get the selected data row of the table view
			set theTask to selected data row of theObject
			
			-- Update the UI using the selected task
			setUIValuesWithTaskValues(window of theObject, theTask)
		else
			-- Set the UI to default values
			setDefaultUIValues(window of theObject)
		end if
	end if
end selection changed


-- The "column clicked" event handler is called whenever the user clickes on a column in the table view. We will change the sort state based on the column clicked. This event handler can be used as is in most applications when utilizing the sort support built into data sources.
--
on column clicked theObject table column tableColumn
	-- Get the data source of the table view
	set theDataSource to data source of theObject
	
	-- Get the name of the clicked table column
	set theColumnName to name of tableColumn
	
	-- Get the current sort column of the data source
	set theSortColumn to sort column of theDataSource
	
	-- If the current sort column is not the same as the clicked column then switch the sort column
	if (name of theSortColumn) is not equal to theColumnName then
		set the sort column of theDataSource to data column theColumnName of theDataSource
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


(* ==== Menu Item Event Handlers ==== *)

-- The "choose menu item" is called (in this example) whenever the user chooses one of the "New Task, Duplicate Task, and Delete Task" menu items.
--
on choose menu item theObject
	-- Set some local variables to various objects in the UI
	set theWindow to front window
	set theTableView to table view "tasks" of scroll view "tasks" of theWindow
	set theDataSource to data source of theTableView
	
	if name of theObject is "new" then
		-- New Task
		set theTask to make new data row at end of data rows of theDataSource
		
		-- Set the UI to default values
		setDefaultTaskValues(theTask)
		
		-- Select the newly added task
		set selected data row of theTableView to theTask
		
		-- Make the "task" text field the object with the focus so that it will be ready for typing
		set first responder of theWindow to text field "task" of theWindow
	else if name of theObject is "duplicate" then
		-- Duplicate Task (only if there is a task selected in the table view)
		if (count of selected data rows of theTableView) > 0 then
			-- Get the selected task
			set theTask to selected data row of theTableView
			
			-- Make a new task and copy the values from the selected one to the new one. (There is a bug in the copy of a data row such that you can't simply say "copy theTask to end of data rows of theDataSource").
			set newTask to make new data row at end of data rows of theDataSource
			set contents of data cell "priority" of newTask to contents of data cell "priority" of theTask
			set contents of data cell "task" of newTask to contents of data cell "task" of theTask
			set contents of data cell "status" of newTask to contents of data cell "status" of theTask
		end if
	else if name of theObject is "delete" then
		-- Delete Task
		if (count of selected data rows of theTableView) > 0 then
			-- Get the selected task
			set theTask to selected data row of theTableView
			
			-- Delete it
			delete theTask
		end if
	end if
end choose menu item


-- The "update menu item" is called whenever the status of any the "Task" menu items need to be updated (for instance when the user clicks on the "Edit" menu where these menu items are). 
--
on update menu item theObject
	-- By default we will enable each of these items
	if front window exists then
		set shouldEnable to true
		
		-- Set some local variables to various objects in the UI
		set theWindow to front window
		set theTableView to table view "tasks" of scroll view "tasks" of theWindow
		set theDataSource to data source of theTableView
		
		if name of theObject is "duplicate" then
			-- If there isn't a task selected disable the "Duplicate Task" menu item
			if (count of selected data rows of theTableView) is 0 then
				set shouldEnable to false
			end if
		else if name of theObject is "delete" then
			-- If there isn't a task selected disable the "Delete Task" menu item
			if (count of selected data rows of theTableView) is 0 then
				set shouldEnable to false
			end if
		end if
	else
		set shouldEnable to false
	end if
	
	-- Return out enable state
	return shouldEnable
end update menu item


(* ==== Handlers ==== *)

-- This handler will set the default values of a new task
--
on setDefaultTaskValues(theTask)
	set contents of data cell "priority" of theTask to "3"
	set contents of data cell "task" of theTask to ""
	set contents of data cell "status" of theTask to "Not Started"
end setDefaultTaskValues

-- This handler will set the default values of UI
--
on setDefaultUIValues(theWindow)
	tell theWindow
		set title of popup button "priority" to "3"
		set contents of text field "task" to ""
		set title of popup button "status" to "Not Started"
	end tell
end setDefaultUIValues

-- This handler will set the values of the given task using the values in the UI
--
on setTaskValuesWithUIValues(theTask, theWindow)
	set contents of data cell "priority" of theTask to title of popup button "priority" of theWindow
	set contents of data cell "task" of theTask to contents of text field "task" of theWindow
	set contents of data cell "status" of theTask to title of popup button "status" of theWindow
end setTaskValuesWithUIValues

-- This handler will set the values of the UI using the given task
--
on setUIValuesWithTaskValues(theWindow, theTask)
	set title of popup button "priority" of theWindow to contents of data cell "priority" of theTask
	set contents of text field "task" of theWindow to contents of data cell "task" of theTask
	set title of popup button "status" of theWindow to contents of data cell "status" of theTask
end setUIValuesWithTaskValues


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)