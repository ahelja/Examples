(* Mail Search.applescript *)

(* ==== Globals ==== *)

global controllers


(* ==== Properties ==== *)

property windowCount : 0
property statusPanelNibLoaded : false


(* ==== Event Handlers ==== *)

on clicked theObject
	set theController to controllerForWindow(window of theObject)
	if theController is not equal to null then
		tell theController to find()
	end if
end clicked

on double clicked theObject
	set theController to controllerForWindow(window of theObject)
	if theController is not equal to null then
		tell theController to openMessages()
	end if
end double clicked

on action theObject
	set theController to controllerForWindow(window of theObject)
	if theController is not equal to null then
		tell theController to find()
	end if
end action

on will open theObject
	set theController to makeController(theObject)
	if theController is not equal to null then
		addController(theController)
		tell theController to initialize()
	end if
end will open

on opened theObject
	set theController to controllerForWindow(theObject)
	if theController is not equal to null then
		tell theController to loadMailboxes()
	end if
end opened

on will finish launching theObject
	set controllers to {}
end will finish launching


(* ==== Controller Handlers ==== *)

on makeController(forWindow)
	script
		property theWindow : forWindow
		property theStatusPanel : null
		property foundMessages : {}
		property mailboxesLoaded : false
		
		-- Handlers
		
		on initialize()
			-- Add a column to the mailboxes data source
			tell scroll view "mailboxes" of split view 1 of theWindow
				make new data column at the end of the data columns of data source of outline view "mailboxes" with properties {name:"mailboxes"}
			end tell
			
			-- Add the columns to the messages data source
			tell scroll view "messages" of split view 1 of theWindow
				make new data column at the end of the data columns of data source of table view "messages" with properties {name:"from"}
				make new data column at the end of the data columns of data source of table view "messages" with properties {name:"subject"}
				make new data column at the end of the data columns of data source of table view "messages" with properties {name:"mailbox"}
			end tell
			
			set windowCount to windowCount + 1
		end initialize
		
		on loadMailboxes()
			if not mailboxesLoaded then
				-- Open the status panel
				set theStatusPanel to makeStatusPanel(theWindow)
				tell theStatusPanel to openPanel("Looking for Mailboxes...")
				
				-- Add the mailboxes
				addMailboxes()
				
				-- Close the status panel
				tell theStatusPanel to closePanel()
				
				set mailboxesLoaded to true
			end if
		end loadMailboxes
		
		on find()
			-- Get what and where to find
			set whatToFind to contents of text field "what" of theWindow
			set whereToFind to title of current menu item of popup button "where" of theWindow
			
			-- Make sure that we have something to find
			if (count of whatToFind) is greater than 0 then
				-- Clear any previously found messages
				clearMessages()
				
				-- Setup a status panel
				set theStatusPanel to makeStatusPanel(theWindow)
				tell theStatusPanel to openPanel("Determining the number of messages...")
				
				try
					-- Determine the mailboxes to search
					set mailboxesToSearch to selectedMailboxes()
					
					-- Determine the total number of messages to search
					set totalCount of theStatusPanel to countMessages(mailboxesToSearch)
					
					-- Adjust the status panel
					tell theStatusPanel to adjustPanel()
					
					-- Find the messages
					set foundMessages to findMessages(mailboxesToSearch, whereToFind, whatToFind)
					
					-- Change the status panel
					tell theStatusPanel to changePanel("Adding found messages...")
					
					-- Add the found messages to the result table
					addMessages(foundMessages)
					
					-- Close the status panel
					tell theStatusPanel to closePanel()
				on error errorText
					tell theStatusPanel to closePanel()
					display alert "AppleScript Error" as critical attached to theWindow message errorText
				end try
			else
				display alert "Missing Value" as critical attached to theWindow message "You need to enter a value to search for."
			end if
		end find
		
		on addMailbox(accountItem, accountName, mailboxIndex, mailboxName)
			-- Add a new item
			set mailboxItem to make new data item at the end of the data items of accountItem
			set name of data cell 1 of mailboxItem to "mailboxes"
			set contents of data cell 1 of mailboxItem to mailboxName
			set associated object of mailboxItem to mailboxIndex
		end addMailbox
		
		on addAccount(a, accountIndex, accountName)
			-- Add a new item
			set accountItem to make new data item at the end of the data items of data source of outline view "mailboxes" of scroll view "mailboxes" of split view 1 of theWindow
			set name of data cell 1 of accountItem to "mailboxes"
			set contents of data cell 1 of accountItem to accountName
			set associated object of accountItem to accountIndex
			
			-- Add the mail boxes
			tell application "Mail"
				set mailboxIndex to 0
				repeat with m in (get mailboxes of a)
					try
						set mailboxIndex to mailboxIndex + 1
						my addMailbox(accountItem, accountName, mailboxIndex, name of m)
					end try
				end repeat
			end tell
		end addAccount
		
		on addMailboxes()
			tell application "Mail"
				set accountIndex to 0
				repeat with a in (get accounts whose enabled is not equal to false)
					try
						set accountIndex to accountIndex + 1
						my addAccount(a, accountIndex, name of a)
					end try
				end repeat
			end tell
		end addMailboxes
		
		on mailboxesForIndex(mailboxIndex)
			-- Initiialize the result
			set theMailboxes to {}
			
			set theIndex to 0
			set theAccountIndex to 0
			
			-- Determine if the selected item is an account or a mailbox
			tell outline view "mailboxes" of scroll view "mailboxes" of split view 1 of theWindow
				set theItem to item for row mailboxIndex
				set theName to contents of data cell 1 of theItem
				set theIndex to associated object of theItem
				if has parent data item of theItem then
					set theAccountIndex to the associated object of the parent data item of theItem
				end if
			end tell
			
			tell application "Mail"
				if theAccountIndex > 0 then
					set theMailboxes to {mailbox theIndex of account theAccountIndex}
				else
					set theMailboxes to theMailboxes & every mailbox of account theIndex
				end if
			end tell
			
			-- Return the result
			return theMailboxes
		end mailboxesForIndex
		
		on selectedMailboxes()
			-- Initialize the result
			set mailboxesSelected to {}
			
			-- Get the currently selected mailboxes in the outline view
			set mailboxIndicies to selected rows of outline view "mailboxes" of scroll view "mailboxes" of split view 1 of theWindow
			
			-- Get the actual mailboxes from Mail
			tell application "Mail"
				if (count of mailboxIndicies) is equal to 0 then
					repeat with a in (get accounts)
						set mailboxesSelected to mailboxesSelected & every mailbox of a
					end repeat
				else
					repeat with i in mailboxIndicies
						set mailboxesSelected to mailboxesSelected & my mailboxesForIndex(i)
					end repeat
				end if
			end tell
			
			-- Return the result
			return mailboxesSelected
		end selectedMailboxes
		
		on addMessage(messageFrom, messageSubject, messageMailbox)
			-- Add a new row
			set theRow to make new data row at the end of the data rows of data source of table view "messages" of scroll view "messages" of split view 1 of theWindow
			
			-- Add "From" cell
			set name of data cell 1 of theRow to "from"
			set contents of data cell 1 of theRow to messageFrom
			
			-- Add "Subject" cell
			set name of data cell 2 of theRow to "subject"
			set contents of data cell 2 of theRow to messageSubject
			
			-- Add "Mailbox" cell
			set name of data cell 3 of theRow to "mailbox"
			set contents of data cell 3 of theRow to messageMailbox
			
			-- set the associated object of theRow to m
		end addMessage
		
		on addMessages(foundMessages)
			set update views of data source of table view "messages" of scroll view "messages" of split view 1 of theWindow to false
			
			tell application "Mail"
				repeat with m in foundMessages
					try
						set messageMailbox to name of account 1 of mailbox of m & "/" & name of mailbox of m
						my addMessage(sender of m, subject of m, messageMailbox)
					end try
				end repeat
			end tell
			
			set update views of data source of table view "messages" of scroll view "messages" of split view 1 of theWindow to true
		end addMessages
		
		on findMessages(mailboxesToSearch, whereToFind, whatToFind)
			-- Initialize the result
			set messagesFound to {}
			
			tell application "Mail"
				-- Search through each of the mail boxes 
				repeat with b in (get mailboxesToSearch)
					try
						-- Search through each of the messages of the mail box
						repeat with m in (get messages of b)
							try
								if whereToFind is equal to "Subject" then
									if whatToFind is in the subject of m then
										copy m to end of messagesFound
									end if
								else if whereToFind is equal to "From" then
									if whatToFind is in sender of m then
										copy m to end of messagesFound
									end if
								else if whereToFind is equal to "To" then
									set foundRecipient to false
									
									-- Recipients
									repeat with r in (get recipients of m)
										if whatToFind is in address of r or whatToFind is in name of r then
											set foundRecipient to true
										end if
									end repeat
									
									-- To Recipients
									if not foundRecipient then
										repeat with r in (get to recipients of m)
											if whatToFind is in address of r or whatToFind is in name of r then
												set foundRecipient to true
											end if
										end repeat
									end if
									
									-- cc Recipients
									if not foundRecipient then
										repeat with r in (get cc recipients of m)
											if whatToFind is in address of r or whatToFind is in name of r then
												set foundRecipient to true
											end if
										end repeat
									end if
									
									-- bcc Recipients
									if not foundRecipient then
										repeat with r in (get bcc recipients of m)
											if whatToFind is in address of r or whatToFind is in name of r then
												set foundRecipient to true
											end if
										end repeat
									end if
									
									if foundRecipient then
										copy m to end of messagesFound
									end if
								else if whereToFind is equal to "Contents" then
									if whatToFind is in the content of m then
										copy m to end of messagesFound
									end if
								end if
								
								-- Update the status panel
								tell theStatusPanel to incrementPanel()
							end try
						end repeat
					end try
				end repeat
			end tell
			
			-- Return the result
			return messagesFound
		end findMessages
		
		on clearMessages()
			tell scroll view "messages" of split view 1 of theWindow
				tell data source of table view "messages" to delete every data row
			end tell
		end clearMessages
		
		on countMessages(mailboxesToSearch)
			set messageCount to 0
			
			tell application "Mail"
				repeat with b in (get mailboxesToSearch)
					try
						set messageCount to messageCount + (count of every message of b)
					end try
				end repeat
			end tell
			
			return messageCount
		end countMessages
		
		on openMessages()
			-- Since Mail.app currently can't open a selected message then we will just open it in our own window
			openMessageWindow()
		end openMessages
		
		on openMessageWindow()
			set clickedRow to clicked row of table view "messages" of scroll view "messages" of split view 1 of theWindow
			if clickedRow is greater than or equal to 0 then
				set theAccount to ""
				set theMailbox to ""
				set theSubject to ""
				set theDateReceived to ""
				set theContents to ""
				set theSender to ""
				set theRecipients to ""
				set theCCRecipients to ""
				set theReplyTo to ""
				
				tell application "Mail"
					set theMessage to item clickedRow of foundMessages
					
					set theAccount to name of account of mailbox of theMessage
					set theMailbox to name of mailbox of theMessage
					set theSubject to subject of theMessage
					-- set theDateReceived to date received of theMessage
					set theContents to content of theMessage
					set theSender to sender of theMessage
					set theRecipients to address of every recipient of theMessage
					set theCCRecipients to address of every cc recipient of theMessage
					set theReplyTo to reply to of theMessage
				end tell
				
				set messageWindow to makeMessageWindow()
				tell messageWindow
					set messageContents to "Account: " & theAccount & return
					set messageContents to messageContents & "Mailbox: " & theMailbox & return
					if length of theSender > 0 then
						set messageContents to messageContents & "From: " & theSender & return
					end if
					if length of theDateReceived as string > 0 then
						set messageContents to messageContents & "Date: " & (theDateReceived as string) & return
					end if
					if length of theRecipients > 0 then
						set messageContents to messageContents & "To: " & theRecipients & return
					end if
					if length of theCCRecipients > 0 then
						set messageContents to messageContents & "Cc: " & theCCRecipients & return
					end if
					if length of theSubject > 0 then
						set messageContents to messageContents & "Subject: " & theSubject & return
					end if
					if length of theReplyTo > 0 then
						set messageContents to messageContents & "Reply-To: " & theReplyTo & return & return
					end if
					set messageContents to messageContents & theContents
					set contents of text view "message" of scroll view "message" to messageContents
					set title to theSubject
					set visible to true
				end tell
			end if
		end openMessageWindow
	end script
end makeController

on addController(theController)
	set controllers to controllers & {theController}
end addController


on controllerForWindow(aWindow)
	repeat with c in controllers
		if theWindow of c is equal to aWindow then
			set theController to c
		end if
	end repeat
	return theController
end controllerForWindow


(* ==== Message Window Handlers ==== *)

on makeMessageWindow()
	load nib "Message"
	set windowCount to windowCount + 1
	set windowName to "message " & windowCount
	set name of window "message" to windowName
	return window windowName
end makeMessageWindow


(* ==== Status Panel Handlers ==== *)

on makeStatusPanel(forWindow)
	script
		property theWindow : forWindow
		property initialized : false
		property totalCount : 0
		property currentCount : 0
		
		-- Handlers
		on openPanel(statusMessage)
			if initialized is false then
				if not statusPanelNibLoaded then
					load nib "StatusPanel"
					set statusPanelNibLoaded to true
				end if
				tell window "status"
					set indeterminate of progress indicator "progress" to true
					tell progress indicator "progress" to start
					set contents of text field "statusMessage" to statusMessage
				end tell
				set initialized to true
			end if
			display panel window "status" attached to theWindow
		end openPanel
		
		on changePanel(statusMessage)
			tell window "status"
				set indeterminate of progress indicator "progress" to true
				tell progress indicator "progress" to start
				set contents of text field "statusMessage" to statusMessage
			end tell
		end changePanel
		
		on adjustPanel()
			tell progress indicator "progress" of window "status"
				set indeterminate to false
				set minimum value to currentCount
				set maximum value to totalCount
				set contents to 0
			end tell
			incrementPanel()
		end adjustPanel
		
		on incrementPanel()
			set currentCount to currentCount + 1
			if currentCount ≤ totalCount then
				tell window "status"
					tell progress indicator "progress" to increment by 1
					set contents of text field "statusMessage" to "Message " & currentCount & " of " & totalCount
				end tell
			end if
		end incrementPanel
		
		on closePanel()
			close panel window "status"
		end closePanel
	end script
end makeStatusPanel


(* © Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
