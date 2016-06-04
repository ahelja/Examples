(* Application.applescript *)

(* ==== Properties ==== *)

global soapresult, readFile
property SOAPEndpointURLParm : ""
property SOAPActionPram : ""
property MethodNamespaceURLPram : ""
property MethodNamesPram : ""
property ParametersPram : {}
property ParametersPramRec : {}


(* ==== Event Handlers ==== *)

on clicked theObject
	set ButtonTitle to title of theObject
	if ButtonTitle = "Run" then
		set enabled of button "stop" of window "SOAPtalk" to true
		tell progress indicator "barberpole" of window "SOAPtalk" to start
		set the contents of text view "results" of scroll view 1 of window "SOAPtalk" to ""
		updateProperties()
		soapCallHandler()
	else if ButtonTitle = "Reset" then
		display dialog "Reset field?" buttons {"Cancel", "All", "Results"} default button "Results" --with icon stop --attached to window "SOAPtalk"
		set eraseresultswindbutton to text of button returned of result
		if eraseresultswindbutton = "Results" then
			set the contents of text view "results" of scroll view 1 of window "SOAPtalk" to ""
		else if eraseresultswindbutton = "All" then
			restFields()
		end if
	else if ButtonTitle = "" then
		open location "http://www.xmethods.com"
	else if ButtonTitle = "stop" then
		tell progress indicator "barberpole" of window "SOAPtalk" to stop
		set enabled of button "stop" of window "SOAPtalk" to false
	end if
end clicked

on changed theObject
	(*Add your script here.*)
end changed

on choose menu item theObject --menu item theItem
	set menuItemTitle to title of theObject as string
	if menuItemTitle = "Open..." then
		set SOAPReadFile to choose file with prompt "Please select a previously saved SOAPTalk file"
		set xRef to open for access SOAPReadFile
		set readFile to read xRef as list
		close access xRef
		updateFieldsFromFile()
	end if
	if menuItemTitle = "Save" then
		try
			updateProperties()
			set resultsfield to contents of text view "results" of scroll view 1 of window "SOAPtalk"
			set writeRecord to {SOAPEndpointURL:SOAPEndpointURLParm, SOAPAction:SOAPActionPram, MethodNamespaceURL:MethodNamespaceURLPram, MethodNames:MethodNamesPram, parameters:ParametersPram, soapresult:resultsfield}
			set saveFile to choose file name with prompt "Save File to" default name "SOAPTalk"
			
			set fileRef to open for access saveFile with write permission
			write writeRecord to fileRef as list
			close access fileRef
		on error errMsg
			try
				get fileRef
				close access fileRef
				display dialog errMsg
			end try
		end try
	end if
end choose menu item


(* ==== Handlers ==== *)

on updateProperties()
	tell window "SOAPTalk"
		set SOAPEndpointURLParm to contents of text field "SOAPEndpointURL" --as application
		set SOAPActionPram to contents of text field "SOAPAction"
		set MethodNamespaceURLPram to contents of text field "MethodNamespaceURL"
		set MethodNamesPram to contents of text field "MethodNames"
		set ParametersPram to contents of text field "Parameters"
		set ParametersPramRec to run script ParametersPram -- convert string record into list record
		set soapresult to ""
	end tell
end updateProperties

on updateFieldsFromFile()
	tell window "SOAPTalk"
		set contents of text field "SOAPEndpointURL" to SOAPEndpointURL of item 1 of readFile
		set contents of text field "SOAPAction" to SOAPAction of item 1 of readFile
		set contents of text field "MethodNamespaceURL" to MethodNamespaceURL of item 1 of readFile
		set contents of text field "MethodNames" to MethodNames of item 1 of readFile
		set contents of text field "Parameters" to parameters of item 1 of readFile
	end tell
end updateFieldsFromFile

on restFields()
	tell window "SOAPTalk"
		set contents of text field "SOAPEndpointURL" to ""
		set contents of text field "SOAPAction" to ""
		set contents of text field "MethodNamespaceURL" to ""
		set contents of text field "MethodNames" to ""
		set contents of text field "Parameters" to ""
		set the contents of text view "results" of scroll view 1 to ""
	end tell
end restFields

on soapCallHandler()
	try
		using terms from application "http://www.apple.com"
			tell application (SOAPEndpointURLParm as string)
				set soapresult to call soap {method name:my getPlainText(MethodNamesPram), method namespace uri:my getPlainText(MethodNamespaceURLPram), parameters:ParametersPramRec, SOAPAction:my getPlainText(SOAPActionPram)}
			end tell
		end using terms from
		
	on error errMsg number errNum
		set the contents of text view "results" of scroll view 1 of window "SOAPtalk" to errMsg & " " & errNum & return & "Are you connected to the Internet?"
		tell progress indicator "barberpole" of window "SOAPtalk" to stop
		set enabled of button "stop" of window "SOAPtalk" to false
	end try
	
	set the contents of text view "results" of scroll view 1 of window "SOAPtalk" to soapresult as string
	tell progress indicator "barberpole" of window "SOAPtalk" to stop
	set enabled of button "stop" of window "SOAPtalk" to false
end soapCallHandler

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