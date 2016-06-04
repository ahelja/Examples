(* Coundown Timer.applescript *)

(* This is a simple example the demonstrates how to idle event to do a countdown timer. When the application is launched it will display the countdown window with a sheet asking for the amount of time for the countdown, after which the countdown begins and when the specified time has elapsed, it displays an alert. *)

(* ===== Properties ===== *)

property countdown : false
property currentDate : 0
property startDate : 0
property endDate : 0


(* ===== Event Handlers ===== *)

on launched theObject
	-- Show the window
	set visible of window "main" to true
	
	-- Display an alert (as a sheet) asking for the amount of time in the HH:MM:SS format
	display dialog "Enter the amount of time for the countdown timer:" default answer "00:00:05" attached to window "main"
end launched

on dialog ended theObject with reply withReply
	-- See if the "OK" button has been clicked
	if button returned of withReply is "OK" then
		-- Save the current date for display purposes
		set currentDate to date (text returned of withReply)
		
		-- Save the start date
		set startDate to current date
		
		-- And determine the end date (start date + the countdown timer)
		set endDate to startDate + (time of currentDate)
		
		-- Update the contents of the text field
		set contents of text field "display" of window "main" to currentDate
		
		-- And let the processing in the idle event handler begin
		set countdown to true
	end if
end dialog ended

on idle theObject
	-- See if we are ready to start counting down
	if countdown then
		-- If the required amount of time has elapsed then display our dialog 
		if (current date) is greater than endDate then
			set countdown to false
			display alert "Time's Up!"
		else
			-- Otherwise determine how much time has elapsed (for display purposes)
			set elapsedTime to (current date) - startDate
			
			-- Update the display
			set contents of text field "display" of window "main" to currentDate - elapsedTime
		end if
	end if
	
	-- We want to update the idle event every second, so we return 1
	return 1
end idle


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)