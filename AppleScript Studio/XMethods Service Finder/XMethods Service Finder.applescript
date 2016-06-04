(* XMethods Service Finder.applescript *)

(* This is an example that demonstrates how to use Web Services. It utilizes a service from XMethods.org that provides information about all of the services available at their site. It also demonstrates how to create and use data sources for a table view. *)

(* The strategy used in this script is to populate the "services" list with all of the services avialable (which is also used to populate the "all services" data source. Then whenever a "find" is requested, the "found services" list is filled out the listing found in the "services" list, and then a new temporary data source is created, using the "found services" list to populate it. That data source is then set as the current data source of the table. If the user deletes the search text field, then the data rows of the "found services" data source are removed and the "all services" data source is set to be the current data source of the table view. In essence, it's just a matter of switching in and out the "all services" and "found services" data sources according to the actions of the user. *)

(* ==== Properties ==== *)

property services : {}
property foundServices : {}
property servicesTableView : missing value
property detailWindow : missing value


(* ==== Event Handlers ==== *)

-- The "launched" event handler is called near the end of the launch sequence. This is a good place to show our main window.
--
on launched theObject
	show window "main"
end launched


-- The "idle" event handler is called on a periodic basis. For our purposes, we are using it to do the initial work of getting all of the services. This is done so that the window will have already been opened and made the active window.
--
on idle theObject
	-- Only do this once (hopefully)
	if (count of services) is 0 then
		-- Show the status items in the main window with a message
		showStatus(window "main")
		updateStatusMessage(window "main", "Getting Services...")
		
		-- Get the services from the xmethods server
		set services to getServices()
		
		-- Update the status message
		updateStatusMessage(window "main", "Adding Services...")
		
		-- Add the services to our data source
		addServicesToDataSource(services, data source "all services")
		
		-- Hide the status items
		hideStatus(window "main")
	end if
	
	return 6000
end idle


-- The "awake from nib" event handler is called whenever the object attached to this handler is loaded from a nib. It's a great place to do any initialization for a particular object, as it's not necessary to locate the object within it's hierarchy.
--
on awake from nib theObject
	if name of theObject is "services" then
		-- Save a reference to the table view
		set servicesTableView to theObject
		
		-- Create a data source that will always contain all of the services, and one that will contain the currently found service
		makeDataSourceWithColumns("all services", {"publisherid", "name", "shortdescription", "id"})
		makeDataSourceWithColumns("found services", {"publisherid", "name", "shortdescription", "id"})
		
		-- Use the "all services" data source at first
		set data source of servicesTableView to data source "all services"
	else if name of theObject is "detail" then
		-- Save a reference to the new detail window
		set detailWindow to theObject
	end if
end awake from nib


-- The "double clicked" event handler is called when someone double clicks on the table view. 
--
on double clicked theObject
	if name of theObject is "services" then
		-- Show and update the message items in the main window
		showStatus(window of theObject)
		updateStatusMessage(window of theObject, "Getting Service Details...")
		
		-- Get the clicked row of the table view
		set theDataRow to clicked data row of theObject
		
		-- Get the name and id of the selected service
		set theServiceID to contents of data cell "id" of theDataRow
		set theServiceName to contents of data cell "name" of theDataRow
		
		-- See if the listing is already open
		set theWindow to findWindowWithTitle(theServiceName)
		if theWindow is not missing value then
			-- Just bring it to the front
			show theWindow
		else
			-- Load a new instance of the detail window and show it
			load nib "ServiceDetail"
			set title of detailWindow to theServiceName
			
			-- Load the service detail and update it in the window
			set theServiceDetail to getServiceDetailWithID(theServiceID as string)
			updateServiceDetailInWindow(theServiceDetail, detailWindow)
			
			-- Show the window
			show detailWindow
		end if
		
		-- Hide the status items
		hideStatus(window of theObject)
	end if
end double clicked


-- The "action" event handler is called when someone chooses a menu item from the popup button. In this case the script will just cause another "find" to happen.
--
on action theObject
	find(window of theObject)
end action

on column clicked theObject table column tableColumn
	-- Get the data source of the table view
	set theDataSource to data source of theObject
	
	-- Get the identifier of the clicked table column
	set theColumnIdentifier to identifier of tableColumn
	
	-- Get the current sort column of the data source
	try
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
	on error
		set sort column of theDataSource to data column theColumnIdentifier of theDataSource
	end try
	
	-- We need to update the table view (so it will be redrawn)
	update theObject
end column clicked


(* ==== Handlers ==== *)

-- This handler will show the status items in the main window. It also starts the animation of the progress indicator.
--
on showStatus(theWindow)
	tell theWindow
		-- Show the text field and progress indicator
		set visible of text field "status" to true
		set visible of progress indicator "progress" to true
		
		-- Make sure it's using threaded animation and start it
		set uses threaded animation of progress indicator "progress" to true
		start progress indicator "progress"
	end tell
end showStatus


-- This handler will hide the status items in the main window. It also stops the animation of the progress indicator.
--
on hideStatus(theWindow)
	tell theWindow
		-- Hide the text field and progress indicator
		set visible of text field "status" to false
		set visible of progress indicator "progress" to false
		
		-- Stop the progress indicator
		stop progress indicator "progress"
	end tell
end hideStatus


-- This handler will update the contents of the status message.
--
on updateStatusMessage(theWindow, theMessage)
	set contents of text field "status" of theWindow to theMessage
end updateStatusMessage


-- The "find" handler is used to query the data source based on the state of where, how, and what to find.
--
on find(theWindow)
	-- Show and update the status items in the window
	showStatus(theWindow)
	updateStatusMessage(theWindow, "Finding Services...")
	
	-- Get the where, how, and what to find form the UI
	tell theWindow
		set findWhere to title of popup button "where"
		set findHow to title of popup button "how"
		set findWhat to contents of text field "what"
	end tell
	
	-- If there isn't anything specified in the "what", then switch in the "all services" data source
	if findWhat is "" then
		set data source of servicesTableView to data source "all services"
		update servicesTableView
	else
		-- Otherwise, find the matching services
		set foundServices to findServices(findWhere, findHow, findWhat)
		
		-- Turn off the updating of the table view while we manipulate the data source
		set update views of data source "found services" to false
		
		-- Delete all of the data rows in the data source
		delete every data row of data source "found services"
		
		-- Make sure that we have at least one found web service and then add the services to the data source
		if (count of foundServices) > 0 then
			addServicesToDataSource(foundServices, data source "found services")
		end if
		
		-- Switch in the "found  services" data source into the table view
		set data source of servicesTableView to data source "found services"
		
		-- Turn back on the updating of the table view
		set update views of data source "found services" to true
	end if
	
	-- Hide the status items
	hideStatus(theWindow)
end find


-- This is a utility handler that will create a new data source with the given name and columns names.
--
on makeDataSourceWithColumns(theName, theColumnNames)
	-- Make the data source
	make new data source at the end of the data sources with properties {name:theName}
	
	-- Add the data columns
	repeat with columnName in theColumnNames
		make new data column at the end of the data columns of data source theName with properties {name:columnName, sort order:ascending, sort type:alphabetical, sort case sensitivity:case insensitive}
	end repeat
	
	-- Set the first column to be  the sort column
	set sort column of data source theName to data column (item 1 of theColumnNames) of data source theName
	
	-- Make the data source sorted
	set sorted of data source theName to true
	
end makeDataSourceWithColumns


-- This handler adds the records to the data source using the "append" command.
--
on addServicesToDataSource(theServices, theDataSource)
	-- Turn off updating the associated table view
	set update views of theDataSource to false
	
	-- Add the records to the data source
	append theDataSource with theServices
	
	-- Turn the updating of the table view back on
	set update views of theDataSource to true
end addServicesToDataSource


-- This is handler will do the actual searching of the "services" list based on the where, how and what parameters.
--
on findServices(findWhere, findHow, findWhat)
	-- Set the result to an empty list
	set theServices to {}
	
	-- Determine which field of the record to search based on "where"
	if findWhere is "Publisher" then
		repeat with service in services
			set theValue to publisherid of service
			if findHow is "begins with" and theValue begins with findWhat then
				copy service to the end of theServices
			else if findHow is "contains" and theValue contains findWhat then
				copy service to the end of theServices
			else if findHow is "ends with" and theValue ends with findWhat then
				copy service to the end of theServices
			else if findHow is "is" and theValue is findWhat then
				copy service to the end of theServices
			end if
		end repeat
	else if findWhere is "Service Name" then
		repeat with service in services
			set theValue to |name| of service
			if findHow is "begins with" and theValue begins with findWhat then
				copy service to the end of theServices
			else if findHow is "contains" and theValue contains findWhat then
				copy service to the end of theServices
			else if findHow is "ends with" and theValue ends with findWhat then
				copy service to the end of theServices
			else if findHow is "is" and theValue is findWhat then
				copy service to the end of theServices
			end if
		end repeat
	else if findWhere is "Description" then
		repeat with service in services
			set theValue to shortdescription of service
			if findHow is "begins with" and theValue begins with findWhat then
				copy service to the end of theServices
			else if findHow is "contains" and theValue contains findWhat then
				copy service to the end of theServices
			else if findHow is "ends with" and theValue ends with findWhat then
				copy service to the end of theServices
			else if findHow is "is" and theValue is findWhat then
				copy service to the end of theServices
			end if
		end repeat
	end if
	
	-- Return the services that were found
	return theServices
end findServices


-- This handler is called when the user has double clicked on one of the services in the table view. It will update the UI elements in the specified detail window with the given service detail record.
--
on updateServiceDetailInWindow(theServiceDetail, theWindow)
	tell theWindow
		-- Update the contents of each of the text fields with the corresponding fields from the detail record.
		set contents of text field "name" to |name| of theServiceDetail
		set contents of text field "description" to shortdescription of theServiceDetail
		set contents of text field "publisher" to publisherid of theServiceDetail
		set contents of text field "email" to email of theServiceDetail
		--set contents of text field "info url" to infourl of theServiceDetail
		set contents of text field "wsdl url" to wsdlurl of theServiceDetail
		
		-- Check to see if we actually have a "note" field.
		if notes of theServiceDetail is not "<<nil not supported>>" then
			set contents of text view "notes" of scroll view "notes" to notes of theServiceDetail
		end if
	end tell
end updateServiceDetailInWindow


(* ==== Web Services Handlers ==== *)

-- The "getServices" handler is used to get a list of records that describes all of the services available from XMethods.org.
--
on getServices()
	-- Set the result to an empty list
	set theServices to {}
	
	-- Get the list of services from the server
	try
		tell application "http://www.xmethods.net/interfaces/query"
			set theServices to call soap {method name:"getAllServiceSummaries", method namespace uri:"http://www.xmethods.net/interfaces/query", parameters:{}, SOAPAction:""}
		end tell
	end try
	
	-- Return the list of services
	return theServices
end getServices


-- The "getServiceDetailWithID" handler will return a record that contains the details about the service with the specified ID.
--
on getServiceDetailWithID(theServiceID)
	-- Set the result to a known value
	set theDetail to missing value
	
	-- We need to convert the supplied service id as plain text (as it is given as unicode text). This is a workaround for a known bug in the "call soap" command, as it can not except unicode or styled text at this time.
	set theServiceID to getPlainText(theServiceID)
	
	-- Get the detailed info from the server.
	try
		tell application "http://www.xmethods.net/interfaces/query"
			set theDetail to call soap {method name:"getServiceDetail", method namespace uri:"http://www.xmethods.net/interfaces/query", parameters:{|id|:theServiceID}, SOAPAction:""}
		end tell
	end try
	
	-- Return the requested detail information
	return theDetail
end getServiceDetailWithID


(* ==== Utility Handlers ==== *)

-- This is a utility handler that will simply find the window with the specified title.
--
on findWindowWithTitle(theTitle)
	set theWindow to missing value
	
	set theWindows to every window whose title is theTitle
	if (count of theWindows) > 0 then
		set theWindow to item 1 of theWindows
	end if
	
	return theWindow
end findWindowWithTitle

-- This is a workaround that will convert the given unicode text into plain text (not styled text)
--
on getPlainText(fromUnicodeString)
	set styledText to fromUnicodeString as string
	set styledRecord to styledText as record
	return «class ktxt» of styledRecord
end getPlainText

(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
