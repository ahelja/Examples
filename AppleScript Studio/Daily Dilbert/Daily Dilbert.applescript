(* Daily Dilbert.applescript *)

(* This is a simple example of how to load an image given a URL from a web service. It utilizes a couple of shell commands (date, curl) to accomplish this. *)


(* ==== Event Handlers ==== *)

-- The "awake from nib" event handler is called when the object is loaded from its nib file. In this case it will be the image view. The script will get the image from the web service and then set that image into the image view. Then the window will be resized appropriately.
--
on awake from nib theObject
	-- We need to have the date in the format "mm/dd/yy" which is actually easier to get from the "date" shell command.
	set theDate to do shell script "date +%m/%d/%y"
	
	-- Get the Dilbert image based on the date
	set theImage to getDilbertImageForDate(theDate)
	set image of theObject to theImage
	
	-- Resize the window
	set the size of (window of theObject) to call method "size" of object theImage
	
	-- Show the window
	show window of theObject
end awake from nib


(* ==== Handlers ==== *)

-- This handler will return the image for the given date. It does this by getting the URL for the image from a web service.
--
on getDilbertImageForDate(theDate)
	set theImage to missing value
	set theImage to loadImageAtURL(DailyDilbertImagePath(theDate))
	return theImage
end getDilbertImageForDate


-- With the given URL, this handler will download the image using the "curl" shell tool. It then will load the image using the "load image" command.
--
on loadImageAtURL(theURL)
	set theImage to missing value
	
	-- Get the last component of the URL. Here we'll use the "lastPathComponent" method of NSString.
	set theImagePath to "/tmp/" & (call method "lastPathComponent" of object theURL)
	
	-- Download the image using "curl"
	do shell script ("curl -o " & theImagePath & " " & theURL)
	
	-- Load the image
	set theImage to load image theImagePath
	
	return theImage
end loadImageAtURL


(* ==== Web Services Handlers ==== *)

-- This handler will return the URL that points to the Dilbert image for the given date.
--
on DailyDilbertImagePath(forDate)
	tell application "http://www.esynaps.com/WebServices/DailyDiblert.asmx"
		set mname to "DailyDilbertImagePath"
		set soapact to "http://tempuri.org/DailyDilbertImagePath"
		set namespace to "http://tempuri.org/"
		set params to {}
		set params to params & {|parameters|:forDate}
		return call soap {method name:mname, parameters:params, SOAPAction:soapact, method namespace uri:namespace}
	end tell
end DailyDilbertImagePath


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
