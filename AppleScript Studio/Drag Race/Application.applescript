(* Application.applescript *)

(* ==== Properties ==== *)

property endRace : false
property finishLine : 627
property betAmount : 5
property holdingsAmount : 1000

global carOneOrgbounds, carTwoOrgbounds, carThreeOrgbounds, carFourOrgbounds, carFiveOrgbounds, pickedCar, winner, raceSpeedval


(* ==== Handlers ==== *)

on resetRace()
	set the title of button "Car 1" of window "Drag Race" to "Car 1"
	set the title of button "Car 2" of window "Drag Race" to "Car 2"
	set the title of button "Car 3" of window "Drag Race" to "Car 3"
	set the title of button "Car 4" of window "Drag Race" to "Car 4"
	set the title of button "Car 5" of window "Drag Race" to "Car 5"
	set the enabled of button "Start Race" of window "Drag Race" to false
	set the bounds of button "Car 1" of window "Drag Race" to carOneOrgbounds
	set the bounds of button "Car 2" of window "Drag Race" to carTwoOrgbounds
	set the bounds of button "Car 3" of window "Drag Race" to carThreeOrgbounds
	set the bounds of button "Car 4" of window "Drag Race" to carFourOrgbounds
	set the bounds of button "Car 5" of window "Drag Race" to carFiveOrgbounds
end resetRace


on moveCar1()
	tell window "Drag Race"
		set carOneOrgPos to the bounds of button "Car 1"
		set stepVal to random number from 1 to raceSpeedval
		set bounds of button "Car 1" to {((item 1 of carOneOrgPos) + stepVal), item 2 of carOneOrgbounds, ((item 3 of carOneOrgPos) + stepVal), item 4 of carOneOrgbounds}
		
		set carOneOrgPos to the bounds of button "Car 1"
		if item 3 of carOneOrgPos > 630 then
			set winner to "Car 1"
			set endRace to true
			set the enabled of button "Start Race" to false
			if pickedCar = "Car 1" then
				set the contents of text field "results" to "Car 1, you won!"
				tell progress indicator "ProgressBar" to stop
				set visible of progress indicator "ProgressBar" to false
				set contents of text field "holdings" to (betAmount + holdingsAmount)
			else
				set the contents of text field "results" to winner & " won, you lost!"
				tell progress indicator "ProgressBar" to stop
				set visible of progress indicator "ProgressBar" to false
				set contents of text field "holdings" to (holdingsAmount - betAmount)
			end if
		end if
	end tell
end moveCar1

on moveCar2()
	tell window "Drag Race"
		set cartwoOrgPos to the bounds of button "Car 2"
		set stepVal to random number from 1 to raceSpeedval
		set bounds of button "Car 2" to {((item 1 of cartwoOrgPos) + stepVal), item 2 of carTwoOrgbounds, ((item 3 of cartwoOrgPos) + stepVal), item 4 of carTwoOrgbounds}
		
		set cartwoOrgPos to the bounds of button "Car 2"
		if item 3 of cartwoOrgPos > finishLine then
			set winner to "Car 2"
			set endRace to true
			set the enabled of button "Start Race" to false
			if pickedCar = "Car 2" then
				set the contents of text field "results" to "Car 2, you won!"
				tell progress indicator "ProgressBar" to stop
				set visible of progress indicator "ProgressBar" to false
				set contents of text field "holdings" to (betAmount + holdingsAmount)
			else
				set the contents of text field "results" to winner & " won, you lost!"
				tell progress indicator "ProgressBar" to stop
				set visible of progress indicator "ProgressBar" to false
				set contents of text field "holdings" to (holdingsAmount - betAmount)
			end if
		end if
	end tell
end moveCar2

on moveCar3()
	tell window "Drag Race"
		set carThreeOrgPos to the bounds of button "Car 3"
		set stepVal to random number from 1 to raceSpeedval
		set bounds of button "Car 3" to {((item 1 of carThreeOrgPos) + stepVal), item 2 of carThreeOrgbounds, ((item 3 of carThreeOrgPos) + stepVal), item 4 of carThreeOrgbounds}
		set carThreeOrgPos to the bounds of button "Car 3"
		if item 3 of carThreeOrgPos > finishLine then
			set winner to "Car 3"
			set endRace to true
			set the enabled of button "Start Race" to false
			if pickedCar = "Car 3" then
				set the contents of text field "results" to "Car 3, you won!"
				tell progress indicator "ProgressBar" to stop
				set visible of progress indicator "ProgressBar" to false
				set contents of text field "holdings" to (betAmount + holdingsAmount)
			else
				set the contents of text field "results" to winner & " won, you lost!"
				tell progress indicator "ProgressBar" to stop
				set visible of progress indicator "ProgressBar" to false
				set contents of text field "holdings" to (holdingsAmount - betAmount)
			end if
		end if
	end tell
end moveCar3

on moveCar4()
	tell window "Drag Race"
		set carFourOrgPos to the bounds of button "Car 4"
		set stepVal to random number from 1 to raceSpeedval
		set bounds of button "Car 4" to {((item 1 of carFourOrgPos) + stepVal), item 2 of carFourOrgbounds, ((item 3 of carFourOrgPos) + stepVal), item 4 of carFourOrgbounds}
		set carFourOrgPos to the bounds of button "Car 4"
		if item 3 of carFourOrgPos > finishLine then
			set winner to "Car 4"
			set endRace to true
			set the enabled of button "Start Race" to false
			if pickedCar = "Car 4" then
				set the contents of text field "results" to "Car 4, you won!"
				tell progress indicator "ProgressBar" to stop
				set visible of progress indicator "ProgressBar" to false
				set contents of text field "holdings" to (betAmount + holdingsAmount)
			else
				set the contents of text field "results" to winner & " won, you lost!"
				tell progress indicator "ProgressBar" to stop
				set visible of progress indicator "ProgressBar" to false
				set contents of text field "holdings" to (holdingsAmount - betAmount)
			end if
		end if
	end tell
end moveCar4

on moveCar5()
	tell window "Drag Race"
		set carFiveOrgPos to the bounds of button "Car 5"
		set stepVal to random number from 1 to raceSpeedval
		set bounds of button "Car 5" to {((item 1 of carFiveOrgPos) + stepVal), item 2 of carFiveOrgbounds, ((item 3 of carFiveOrgPos) + stepVal), item 4 of carFiveOrgbounds}
		set carFiveOrgPos to the bounds of button "Car 5"
		if item 3 of carFiveOrgPos > finishLine then
			set winner to "Car 5"
			set endRace to true
			set the enabled of button "Start Race" to false
			if pickedCar = "Car 5" then
				set the contents of text field "results" to "Car 5, you won!"
				tell progress indicator "ProgressBar" to stop
				set visible of progress indicator "ProgressBar" to false
				set contents of text field "holdings" to (betAmount + holdingsAmount)
			else
				set the contents of text field "results" to winner & " won, you lost!"
				tell progress indicator "ProgressBar" to stop
				set visible of progress indicator "ProgressBar" to false
				set contents of text field "holdings" to (holdingsAmount - betAmount)
			end if
		end if
	end tell
end moveCar5


(* ==== Event Handlers ==== *)

on will open theObject
	set visible of progress indicator "ProgressBar" of window "Drag Race" to false
	set betAmount to contents of text field "bet" of window "Drag Race"
	set holdingsAmount to contents of text field "holdings" of window "Drag Race"
	set raceSpeedval to contents of slider "RaceSpeed" of window "Drag Race" as integer
	set carOneOrgbounds to the bounds of button "Car 1" of window "Drag Race"
	set carTwoOrgbounds to the bounds of button "Car 2" of window "Drag Race"
	set carThreeOrgbounds to the bounds of button "Car 3" of window "Drag Race"
	set carFourOrgbounds to the bounds of button "Car 4" of window "Drag Race"
	set carFiveOrgbounds to the bounds of button "Car 5" of window "Drag Race"
	set the contents of text field "results" of window "Drag Race" to "Pick a car!"
	set the enabled of button "Start Race" of window "Drag Race" to false
	set the enabled of button "Reset" of window "Drag Race" to false
end will open


on clicked theObject
	
	if title of theObject = "Car 1" then
		resetRace()
		set the title of button "Car 1" of window "Drag Race" to "Car 1 •"
		set contents of text field "results" of window "Drag Race" to "You picked car 1"
		set pickedCar to "Car 1"
		set the enabled of button "Start Race" of window "Drag Race" to true
		set the enabled of button "Reset" of window "Drag Race" to true
	else if title of theObject = "Car 2" then
		resetRace()
		set the title of button "Car 2" of window "Drag Race" to "Car 2 •"
		set contents of text field "results" of window "Drag Race" to "You picked car 2"
		set pickedCar to "Car 2"
		set the enabled of button "Start Race" of window "Drag Race" to true
		set the enabled of button "Reset" of window "Drag Race" to true
	else if title of theObject = "Car 3" then
		resetRace()
		set the title of button "Car 3" of window "Drag Race" to "Car 3 •"
		set contents of text field "results" of window "Drag Race" to "You picked car 3"
		set pickedCar to "Car 3"
		set the enabled of button "Start Race" of window "Drag Race" to true
		set the enabled of button "Reset" of window "Drag Race" to true
	else if title of theObject = "Car 4" then
		resetRace()
		set the title of button "Car 4" of window "Drag Race" to "Car 4 •"
		set contents of text field "results" of window "Drag Race" to "You picked car 4"
		set pickedCar to "Car 4"
		set the enabled of button "Start Race" of window "Drag Race" to true
		set the enabled of button "Reset" of window "Drag Race" to true
	else if title of theObject = "Car 5" then
		resetRace()
		set the title of button "Car 5" of window "Drag Race" to "Car 5 •"
		set contents of text field "results" of window "Drag Race" to "You picked car 5"
		set pickedCar to "Car 5"
		set the enabled of button "Start Race" of window "Drag Race" to true
		set the enabled of button "Reset" of window "Drag Race" to true
	else if title of theObject = "Reset" then
		set endRace to true
		tell progress indicator "ProgressBar" of window "Drag Race" to stop
		resetRace()
		set the contents of text field "results" of window "Drag Race" to "Pick a car!"
	end if
	
	if contents of text field "results" of window "Drag Race" ≠ "Pick a car!" then
		if title of theObject = "Start Race" then
			set endRace to false
			set betAmount to contents of text field "bet" of window "Drag Race"
			set holdingsAmount to contents of text field "holdings" of window "Drag Race"
			set visible of progress indicator "ProgressBar" of window "Drag Race" to true
			tell progress indicator "ProgressBar" of window "Drag Race" to start
			repeat while endRace = false
				moveCar1()
				moveCar2()
				moveCar3()
				moveCar4()
				moveCar5()
			end repeat
		end if
	end if
end clicked

on action theObject
	set raceSpeedval to contents of slider "RaceSpeed" of window "Drag Race" as integer
end action

(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)