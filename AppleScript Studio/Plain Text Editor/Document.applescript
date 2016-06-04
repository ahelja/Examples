(* Document.applescript *)

(* This is a very simple example of how to write a document based plain text editor. It takes advantage of the lower level handlers for document handling, namely "read from file" and "write to file". It does this so that it can read text documents created by other applications. The two higher level handlers "data representation" and "load data representation" allow you to return and set any type of data, but then it will only be readable by your application, as it utilizes Cocoa's NSData object to store and retrieve your data. *)

(* ==== Event Handlers ==== *)

-- The "read from file" handler is called when the document needs to the data to be read from disk. "theObject" is the document object, "pathName" contains the POSIX style path of the file to read and "ofType" contains the type of document to read (which by default this value will be "DocumentType" as set up in the documents section of the target editor for document based Studio applications).
--
on read from file theObject path name pathName of type ofType
	-- Open the file so that we can read it in
	set theFile to open for access (pathName as POSIX file)
	
	-- Read the data in
	set theData to read theFile as string
	
	-- Close the file
	close access theFile
	
	-- Put the data that we read into the text view of our document
	set contents of text view "editor" of scroll view "editor" of window of theObject to theData
	
	-- We need to return true (if everything went well) or false (if something failed). For the purposes of this example we'll signal that everything went well.
	return true
end read from file


-- The "write to file" handler is called when the document needs to be saved to disk. "theObject" is the document object, "pathName" contains the POSIX style path of the file to write and "ofType" contains the type of document to read (which by default this value will be "DocumentType" as set up in the documents section of the target editor for document based Studio applications).
--
on write to file theObject path name pathName of type ofType
	-- Get the data from the text view of the document
	set theData to contents of text view "editor" of scroll view "editor" of window of theObject
	
	-- Open the file for writing
	set theFile to open for access (pathName as POSIX file) with write permission
	
	-- Write the data
	write theData to theFile as string
	
	-- Close the file
	close access theFile
	
	-- We need to return true (if everything went well) or false (if something failed). For the purposes of this example we'll signal that everything went well.
	return true
end write to file


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
