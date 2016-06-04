(* Simple Outline.applescript *)

(* This is a very simple example of how to populate an outline view using a data source. It will create a data source with data items representing the following outline:

	- Things to do
		- Work on outline example
			- Make it plain and simple
			- Put it all in a "on launched" event handler
		- Put it in my iDisk when done
		
	It has been enhanced to add drag and drop support and uses the new "content" property.
*)


(* ===== Event Handlers ===== *)

-- This event handler is attached to the table view in our nib. It is a good place to set the contents of the table view and to setup any drag types that we might desire.
--
on awake from nib theObject
	-- Create the data source, data items and data cells by simply setting the "content" property of the outline view
	set content of theObject to {{completed:"--", task:"Things to do", |items|:{{completed:"Yes", task:"Work on outline example", |items|:{{completed:"Yes", task:"Make it plain and simple"}, {completed:"Yes", task:"Put it all in an \"on launched'\" event handler"}}}, {completed:"Yes", task:"Put it in my iDisk when done"}}}}
	
	tell theObject to register drag types {"items", "file names"}
end awake from nib

-- The launched handler is generally the  last event handler to be called in the launch sequence. It's a good place for us to show our window.
--
on launched theObject
	show window "main"
end launched

-- This event handler is called whenever the user is done editing a cell in the outline view.
--
on change item value theObject table column tableColumn outline item outlineItem value theValue
	return "maybe"
end change item value

-- This event handler is called that the beginning of a drag operation in our outline view
--
on prepare outline drag theObject drag items dragItems pasteboard thePasteboard
	-- We are about to start a drag from within our outline view, so set the preferred type of the pasteboard to be "items" and then set the content of the pasteboard to be the items being dragged
	set preferred type of thePasteboard to "items"
	set content of thePasteboard to dragItems
	
	-- Since it isn't convenient to get items on to the pasteboard, we just save the list of dragged items to be used later
	set dragged items of theObject to dragItems
	
	return true
end prepare outline drag

-- This event handler is called while the drag and drop operation is ongoing. We can decide whether or not we want to accept the drop, or where to allow the drop.
--
on prepare outline drop theObject data item dataItem drag info dragInfo child index childIndex
	-- By default we will set the drag operation to not be a drag operation
	set dragOperation to no drag operation
	
	-- Get the list of data types on the pasteboard
	set dataTypes to types of pasteboard of dragInfo
	
	-- Set the type of drag operation based on the drop operation and the state of the option key
	if "items" is in dataTypes then
		if option key down of event 1 then
			set dragOperation to copy drag operation
		else
			set dragOperation to move drag operation
		end if
	else if "file names" is in dataTypes then
		set dragOperation to copy drag operation
	end if
	
	-- Return the desired drag operation
	return dragOperation
end prepare outline drop

-- This event handler is called when the drop happens. 
--
on accept outline drop theObject data item dataItem drag info dragInfo child index childIndex
	-- Get the list of data types on the pasteboard
	set dataTypes to types of pasteboard of dragInfo
	set dataSource to data source of theObject
	
	-- Turn off the updating of the views
	set update views of dataSource to false
	
	-- Set up the target data item (where we'll be placing the dropped items)
	if dataItem is missing value or childIndex = 0 or childIndex > (count of data items of dataItem) then
		set targetDataItem to missing value
	else
		set targetDataItem to data item childIndex of dataItem
	end if
	
	-- See if we are receiving our "items" in the drop
	if "items" is in dataTypes then
		-- We'll just use the list of dragged items we saved earlier, as it  is easier than getting them from the pasteboard
		set draggedItems to dragged items of theObject
		
		-- Now move or duplicate the data items based on the option key
		if option key down of event 1 then
			repeat with i in draggedItems
				if dataItem is not missing value then
					if childIndex = 0 or childIndex > (count of data items of dataItem) then
						duplicate i to end of data items of dataItem
					else
						duplicate i to before dataItem
					end if
				else
					duplicate i to end of data items of dataSource
				end if
			end repeat
		else
			repeat with i in draggedItems
				if dataItem is not missing value then
					if childIndex = 0 or childIndex > (count of data items of dataItem) then
						move i to end of data items of dataItem
					else
						move i to before dataItem
					end if
				else
					move i to end of data items of dataSource
				end if
			end repeat
		end if
	else if "file names" is in dataTypes then
		-- Initialize the list of files to an empty list
		set theFiles to {}
		
		-- We want the data as a list of file names, so set the preferred type to "file names"
		set preferred type of pasteboard of dragInfo to "file names"
		
		-- Get the list of files from the pasteboard
		set theFiles to contents of pasteboard of dragInfo
		
		-- Make sure we have at least one item
		if (count of theFiles) > 0 then
			repeat with theItem in theFiles
				if targetDataItem is not missing value then
					set dataItem to make new data item at before targetDataItem
				else
					set dataItem to make new data item at end of data items of dataSource
				end if
				
				set contents of data cell "task" of dataItem to theItem
			end repeat
		end if
	end if
	
	-- Turn back on the updating of the views
	set update views of dataSource to true
	
	-- Make sure to return true, otherwise the drop will be cancelled.
	return true
end accept outline drop


(* © Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)