(* Assistant.applescript *)

(* This application is to present one possible implementation of an 'Assistant'. The strategy that is used is to use a tab view and use seperate tab view items to represent an information panel. The tab view is set without a border or visible tabs. This gives the appearance of a panel full of UI elements to being switched in and out. The design also supports the ability to easily add, remove or change the order of info panels. One thing of note is that and that is incorporated in this strategy is that UI elements of tab view items that are not the current tab view item are not accessible. The way a tab view works is by adding and removing the tab view item's view in and out of the view hierarchy. Since AppleScript needs to be able to walk that view hierarchy to get access to the UI elements in the sub views. Thus, the properties of each info panel is updated before the tab view item is switched out. *)

(* The structure of this script is as follows:
	Properties		Properties needed for the application.
	Script Objects		Model/Controller objects that are specific to each info panel.
	Event Handlers	Handlers that are called by actions in the UI.
	Handlers 			Handlers that interact with the script objects and as well as the UI.
*)


(* ==== Properties === *)

property infoPanels : {}
property currentInfoPanelIndex : 1
property statusImages : {}


(* ==== Script Objects ==== *)

-- This is the parent script object that represents an info panel. It has default implementations of all of the handlers that is used throughout this application.
-- 
script InfoPanel
	-- This handler is called when the contents of the UI elements need to be prepared
	on prepareValues(theWindow)
		-- Scripts that inherit from this script need to implement this handler
	end prepareValues
	
	-- This handler is called when the properties need to be updated from the contents of the UI elements
	on updateValues(theWindow)
		-- Scripts that inherit from this script need to implement this handler
	end updateValues
	
	-- This handler is called to allow an info panel to validate it's values, returning false if the data isn't valid (or is missing)
	on validateValues(theWindow)
		-- Scripts that inherit from this script need to implement this handler
		return true
	end validateValues
	
	-- This handler is called when a summary of the property values is needed.
	on summarizeValues()
		-- Scripts that inherit from this script need to implement this handler
	end summarizeValues
	
	-- This handler will set the focus on the UI element that has a problem and then presents an alert.
	on postValidationAlert(theMessage, theTextField, theWindow)
		-- Move to the field that is missing it's information
		set first responder of theWindow to theTextField
		
		-- Display the alert
		display alert "Missing Information" as critical message theMessage attached to theWindow
	end postValidationAlert
end script


-- This script represents the reporter info panel that contains the personal information about the person reporting the problem.
-- 
script ReporterInfoPanel
	property parent : InfoPanel
	property infoPanelName : "reporter"
	property infoPanelInstruction : "Please enter your personal information."
	
	property company : ""
	property name : ""
	property address : ""
	property city : ""
	property zip : ""
	property state : ""
	property email : ""
	
	-- This handler is called when the properties need to be updated from the contents of the UI elements
	-- 
	on updateValues(theWindow)
		tell view of tab view item infoPanelName of tab view "info panels" of box "border" of theWindow
			set my company to contents of text field "company"
			set my name to contents of text field "name"
			set my address to contents of text field "address"
			set my city to contents of text field "city"
			set my state to contents of text field "state"
			set my zip to contents of text field "zip"
			set my email to contents of text field "email"
		end tell
	end updateValues
	
	-- This handler is called to allow an info panel to validate it's values, returning false if the data isn't valid (or is missing)
	-- 
	on validateValues(theWindow)
		set isValid to true
		
		-- We need to have at least the name and email
		if name is "" then
			postValidationAlert("You must enter a name.", text field "name" of view of tab view item infoPanelName of tab view "info panels" of box "border" of theWindow, theWindow)
			set isValid to false
		else if email is "" then
			postValidationAlert("You must enter an e-mail address.", text field "email" of view of tab view item infoPanelName of tab view "info panels" of box "border" of theWindow, theWindow)
			set isValid to false
		end if
		
		return isValid
	end validateValues
	
	-- This handler is called when a summary of the property values is needed.
	-- 
	on summarizeValues()
		set theSummary to company & return
		set theSummary to theSummary & name & return
		set theSummary to theSummary & address & return
		set theSummary to theSummary & city & ", " & state & " " & zip & return
		set theSummary to theSummary & email & return
		return theSummary
	end summarizeValues
end script


-- This script represents the problem info panel that contains the information about the problem itself.
-- 
script ProblemInfoPanel
	property parent : InfoPanel
	property infoPanelName : "problem"
	property infoPanelInstruction : "Please describe your problem."
	
	property product : ""
	property version : ""
	property severity : ""
	property reproducible : ""
	property description : ""
	
	-- This handler is called when the properties need to be updated from the contents of the UI elements
	-- 
	on updateValues(theWindow)
		tell view of tab view item infoPanelName of tab view "info panels" of box "border" of theWindow
			set my product to contents of text field "product"
			set my version to contents of text field "version"
			set my severity to title of current cell of matrix "severity"
			set my reproducible to title of current menu item of popup button "reproducible"
			set my description to contents of text view "description" of scroll view "scroll"
		end tell
	end updateValues
	
	-- This handler is called to allow an info panel to validate it's values, returning false if the data isn't valid (or is missing)
	-- 
	on validateValues(theWindow)
		set isValid to true
		
		-- We need to have at the very least the product info, version info and description info
		if product is "" then
			postValidationAlert("You must enter a product name.", text field "product" of view of tab view item infoPanelName of tab view "info panels" of box "border" of theWindow, theWindow)
			set isValid to false
		else if version is "" then
			postValidationAlert("You must enter the version of the product.", text field "version" of view of tab view item infoPanelName of tab view "info panels" of box "border" of theWindow, theWindow)
			set isValid to false
		else if description is "" then
			postValidationAlert("You must enter a description of the problem.", text field "description" of view of tab view item infoPanelName of tab view "info panels" of box "border" of theWindow, theWindow)
			set isValid to false
		end if
		
		return isValid
	end validateValues
	
	-- This handler is called when a summary of the property values is needed.
	-- 
	on summarizeValues()
		set theSummary to "Product: " & tab & product & " version " & version & return
		set theSummary to theSummary & "Severity: " & tab & severity & return
		set theSummary to theSummary & "Reproducible: " & tab & reproducible & return
		set theSummary to theSummary & "Description: " & return
		set theSummary to theSummary & description & return
		return theSummary
	end summarizeValues
	
end script


-- This script represents the comments info panel that contains the comments from the reporter.
-- 
script CommentsInfoPanel
	property parent : InfoPanel
	property infoPanelName : "comments"
	property infoPanelInstruction : "Please enter any comments."
	
	property comments : ""
	
	-- This handler is called when the properties need to be updated from the contents of the UI elements
	-- 
	on updateValues(theWindow)
		tell view of tab view item infoPanelName of tab view "info panels" of box "border" of theWindow
			set my comments to contents of text view "comments" of scroll view "scroll"
		end tell
	end updateValues
	
	-- This handler is called when a summary of the property values is needed.
	-- 
	on summarizeValues()
		set theSummary to "Comments: " & return
		set theSummary to theSummary & comments & return
		return theSummary
	end summarizeValues
end script


-- This script represents the review info panel, that allows the reporter a chance to see a summary of all of the information before it will be sent.
-- 
script ReviewInfoPanel
	property parent : InfoPanel
	property infoPanelName : "review"
	property infoPanelInstruction : "Please review before sending."
	
	property reviewSummary : ""
	
	-- This handler is called when the contents of the UI elements need to be prepared
	-- 
	on prepareValues(theWindow)
		set theSummary to summarizeValues()
		tell view of tab view item "review" of tab view "info panels" of box "border" of theWindow
			set contents of text view "review" of scroll view "scroll" to theSummary
		end tell
	end prepareValues
	
	-- This handler is called when the properties need to be updated from the contents of the UI elements
	-- 
	on updateValues(theWindow)
		tell view of tab view item infoPanelName of tab view "info panels" of box "border" of theWindow
			set my reviewSummary to contents of text view "review" of scroll view "scroll"
		end tell
	end updateValues
	
	-- This handler is called when a summary of the property values is needed.
	-- 
	on summarizeValues()
		set theSummary to ""
		
		-- Since this is the review info panel, we'll get the summary from all of the other info panels and put them together
		repeat with n from 1 to ((count of infoPanels) - 1)
			set theSummary to theSummary & summarizeValues() of item n of infoPanels & return
		end repeat
		
		return theSummary
	end summarizeValues
end script


(* ==== Event Handlers ==== *)

-- This event handler is called when the application is finished launching. It's a good place to to any initialization before showing the main window.
-- 

on launched theObject
	-- Load the images
	set statusImages to {(load image "DotBlue"), (load image "DotGray")}
	
	-- Setup the info panel list. The order of the panels is established here. You can easily change the order that they are presented by changing their order here in this list. The only other thing you need to keep synchronized is the status text items in the left hand portion of the window.
	set infoPanels to {ReporterInfoPanel, ProblemInfoPanel, CommentsInfoPanel, ReviewInfoPanel}
	
	-- Switch to the first info panel
	switchToFirstInfoPanel(window "main")
	
	set visible of window "main" to true
end launched


-- This event handler is called when a button is clicked, in this case the 'go back' or 'continue' buttons.
-- 
on clicked theObject
	if name of theObject is "continue" then
		if currentInfoPanelIndex is equal to (count of infoPanels) then
			-- On the last panel, the button has changed to 'Send' so send the gathered information 
			sendInformation(window of theObject)
		else
			-- Switch to the next info panel
			switchToNextInfoPanel(window of theObject)
		end if
	else if name of theObject is "back" then
		-- Switch to the previous info panel
		switchToPreviousInfoPanel(window of theObject)
	end if
end clicked


-- This event handler is called when the tab view is about to switch tab items. You can control the result by returning 'true' to allow the selection to happen, or 'false' to cancel it. Here we will collect the information from each panel and then validate the information and make our decision based upon the validation as to whether or not we will allow the selection to change.
-- 
on should select tab view item theObject tab view item tabViewItem
	set isValid to true
	
	-- We only want to update and validate if the window is visible
	if window of theObject is visible then
		-- Update the current info panel with the contents of the UI
		updateCurrentInfoPanel(window of theObject)
		
		-- Validate the current  info panel to see if we should move on
		set isValid to validateCurrentInfoPanel(window of theObject)
	end if
	
	-- Return the validity status (true if it's ok to select the tab, false if it's not)
	return isValid
end should select tab view item


-- This event handler is called when the current tab view item has been changed. 
-- 
on selected tab view item theObject tab view item tabViewItem
	-- We will give the new info panel a chance to prepare it's data values
	prepareValues(window of theObject) of infoPanelWithName(name of tabViewItem)
end selected tab view item


(* ==== Handlers ==== *)

-- This handler will attempt to switch to the indicated info panel and change the UI to reflect that change.
-- 
on switchToInfoPanel(theIndex, theWindow)
	tell theWindow
		set theInfoPanelName to infoPanelName of item theIndex of infoPanels
		set theInfoPanelInstruction to infoPanelInstruction of item theIndex of infoPanels
		
		-- Attempt to switch to the indicated tab view item
		tell tab view "info panels" of box "border"
			set current tab view item to tab view item theInfoPanelName
			
			-- The tab may not change due to validation checking, so make sure we have changed
			if name of current tab view item is not equal to theInfoPanelName then
				return
			end if
		end tell
		
		-- Update the current index
		set currentInfoPanelIndex to theIndex
		
		-- Update the instructions
		tell box "instructions"
			set contents of text field "instructions" to theInfoPanelInstruction
		end tell
		
		-- Update the 'back' button. 
		if theIndex is 1 then
			-- Hide it on the first panel.
			set visible of button "back" to false
		else
			-- Show it on all others
			set visible of button "back" to true
		end if
		
		-- Update the 'continue' button. 
		if theIndex is (count of infoPanels) then
			-- Set the title to 'Send' if we are on the last panel.
			set title of button "continue" to "Send"
		else
			-- Otherwise set it to 'Continue'
			set title of button "continue" to "Continue"
		end if
		
		-- Update the status images
		repeat with index from 1 to count of infoPanels
			-- Get the name of the info panel
			set infoPanelName to infoPanelName of item index of infoPanels
			
			-- We will be setting the status image to blue for any info panels up to the current index, otherwise we'll set it to gray
			if index ≤ currentInfoPanelIndex then
				set image of image view infoPanelName to item 1 of statusImages
			else
				set image of image view infoPanelName to item 2 of statusImages
			end if
		end repeat
	end tell
end switchToInfoPanel


-- Switches to the the first info panel (called upon startup of the application)
-- 
on switchToFirstInfoPanel(theWindow)
	-- Switch to the first item in the info panels list
	switchToInfoPanel(1, theWindow)
end switchToFirstInfoPanel


-- Switches to the the next info panel if available
-- 
on switchToNextInfoPanel(theWindow)
	-- Make sure that we aren't already on the last panel
	if currentInfoPanelIndex is less than (count of infoPanels) then
		switchToInfoPanel(currentInfoPanelIndex + 1, theWindow)
	end if
end switchToNextInfoPanel


-- Switches to the the previous info panel if available
-- 
on switchToPreviousInfoPanel(theWindow)
	-- Make sure that we aren't already on the first panel
	if currentInfoPanelIndex is greater than 1 then
		switchToInfoPanel(currentInfoPanelIndex - 1, theWindow)
	end if
end switchToPreviousInfoPanel


-- This handler will tell the current info panel to set it's properties values from the UI objects in it's panel
-- 
on updateCurrentInfoPanel(theWindow)
	tell item currentInfoPanelIndex of infoPanels to updateValues(theWindow)
end updateCurrentInfoPanel


-- This handler will validate the current info panel, to ensure that the required data is present and valid
-- 
on validateCurrentInfoPanel(theWindow)
	return validateValues(theWindow) of item currentInfoPanelIndex of infoPanels
end validateCurrentInfoPanel


-- This event handler handles sending the gathered information to (wherever)
-- 
on sendInformation(theWindow)
	-- Get the summary information from the the Review info panel
	set theInformation to reviewSummary of ReviewInfoPanel
	
	-- Send this information
	-- *** This is left blank as it is implementation dependent and is left as an exercise ***
end sendInformation


-- This is a utility handler that is called to return the info panel with the given name
-- 
on infoPanelWithName(theName)
	set theInfoPanel to null
	
	repeat with thePanel in infoPanels
		if infoPanelName of thePanel is equal to theName then
			set theInfoPanel to thePanel
			exit repeat
		end if
	end repeat
	
	return theInfoPanel
end infoPanelWithName


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
