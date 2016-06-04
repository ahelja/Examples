(* Converter.applescript *)

(* ==== Globals ==== *)

global LogController


(* ==== Scripts ==== *)

script Converter
	property measureType : ""
	property fromMeasure : ""
	property toMeasure : ""
	property fromUnits : 0
	property resultUnits : 0
	property unitTypes : {}
	
	on initializeUnitTypes()
	end initializeUnitTypes
	
	on convert()
		tell box 1 of window "Main"
			set measureType to title of popup button "Type"
			set fromMeasure to title of popup button "From"
			set toMeasure to title of popup button "To"
			set fromUnits to contents of text field "Value" as real
			set resultUnits to 0
		end tell
	end convert
	
	on updateUnitTypes()
		tell box 1 of window "Main"
			-- Delete all of the menu items from the popups
			delete every menu item of menu of popup button "From"
			delete every menu item of menu of popup button "To"
			
			-- Add each of the unit types as menu items to both of the popups
			repeat with i in my unitTypes
				make new menu item at the end of menu items of menu of popup button "From" with properties {title:i, enabled:true}
				make new menu item at the end of menu items of menu of popup button "To" with properties {title:i, enabled:true}
			end repeat
		end tell
	end updateUnitTypes
	
	on updateObject(theObject)
	end updateObject
	
	on updateResult(theResult)
		set contents of text field "Result" of box 1 of window "Main" to (theResult as text)
		tell LogController to addResultToLog((fromUnits as text) & " " & fromMeasure & " equals " & (theResult as text) & " " & toMeasure)
	end updateResult
end script

script LengthConverter
	property parent : Converter
	property unitTypes : {"kilometers", "meters", "centimeters", "miles", "yards", "feet", "inches"}
	
	on convert()
		continue convert()
		
		if my fromMeasure = "kilometers" then
			set my resultUnits to my fromUnits as kilometers
		else if my fromMeasure = "meters" then
			set my resultUnits to my fromUnits as meters
		else if my fromMeasure = "centimeters" then
			set my resultUnits to my fromUnits as centimeters
		else if my fromMeasure = "miles" then
			set my resultUnits to my fromUnits as miles
		else if my fromMeasure = "yards" then
			set my resultUnits to my fromUnits as yards
		else if my fromMeasure = "feet" then
			set my resultUnits to my fromUnits as feet
		else if my fromMeasure = "inches" then
			set my resultUnits to my fromUnits as inches
		end if
		
		if my toMeasure = "kilometers" then
			set my resultUnits to my resultUnits as kilometers
		else if my toMeasure = "meters" then
			set my resultUnits to my resultUnits as meters
		else if my toMeasure = "centimeters" then
			set my resultUnits to my resultUnits as centimeters
		else if my toMeasure = "miles" then
			set my resultUnits to my resultUnits as miles
		else if my toMeasure = "yards" then
			set my resultUnits to my resultUnits as yards
		else if my toMeasure = "feet" then
			set my resultUnits to my resultUnits as feet
		else if my toMeasure = "inches" then
			set my resultUnits to my resultUnits as inches
		end if
		
		my updateResult(my resultUnits)
	end convert
end script

script WeightConverter
	property parent : Converter
	property unitTypes : {"kilograms", "grams", "pounds", "ounces"}
	
	on convert()
		continue convert()
		
		if my fromMeasure = "kilograms" then
			set my resultUnits to my fromUnits as kilograms
		else if my fromMeasure = "grams" then
			set my resultUnits to my fromUnits as grams
		else if my fromMeasure = "pounds" then
			set my resultUnits to my fromUnits as pounds
		else if my fromMeasure = "ounces" then
			set my resultUnits to my fromUnits as ounces
		end if
		
		if my toMeasure = "kilograms" then
			set my resultUnits to my resultUnits as kilograms
		else if my toMeasure = "grams" then
			set my resultUnits to my resultUnits as grams
		else if my toMeasure = "pounds" then
			set my resultUnits to my resultUnits as pounds
		else if my toMeasure = "ounces" then
			set my resultUnits to my resultUnits as ounces
		end if
		
		my updateResult(my resultUnits)
	end convert
end script

script LiquidVolumeConverter
	property parent : Converter
	property unitTypes : {"liters", "gallons", "quarts"}
	
	on convert()
		continue convert()
		
		if my fromMeasure = "liters" then
			set my resultUnits to my fromUnits as liters
		else if my fromMeasure = "gallons" then
			set my resultUnits to my fromUnits as gallons
		else if my fromMeasure = "quarts" then
			set my resultUnits to my fromUnits as quarts
		end if
		
		if my toMeasure = "liters" then
			set my resultUnits to my resultUnits as liters
		else if my toMeasure = "gallons" then
			set my resultUnits to my resultUnits as gallons
		else if my toMeasure = "quarts" then
			set my resultUnits to my resultUnits as quarts
		end if
		
		my updateResult(my resultUnits)
	end convert
end script

script VolumeConverter
	property parent : Converter
	property unitTypes : {"cubic centimeters", "cubic meters", "cubic inches", "cubic feet", "cubic yards"}
	
	on convert()
		continue convert()
		
		if my fromMeasure = "cubic centimeters" then
			set my resultUnits to my fromUnits as cubic centimeters
		else if my fromMeasure = "cubic meters" then
			set my resultUnits to my fromUnits as cubic meters
		else if my fromMeasure = "cubic inches" then
			set my resultUnits to my fromUnits as cubic inches
		else if my fromMeasure = "cubic feet" then
			set my resultUnits to my fromUnits as cubic feet
		else if my fromMeasure = "cubic yards" then
			set my resultUnits to my fromUnits as cubic yards
		end if
		
		if my toMeasure = "cubic centimeters" then
			set my resultUnits to my resultUnits as cubic centimeters
		else if my toMeasure = "cubic meters" then
			set my resultUnits to my resultUnits as cubic meters
		else if my toMeasure = "cubic inches" then
			set my resultUnits to my resultUnits as cubic inches
		else if my toMeasure = "cubic feet" then
			set my resultUnits to my resultUnits as cubic feet
		else if my toMeasure = "cubic yards" then
			set my resultUnits to my resultUnits as cubic yards
		end if
		
		my updateResult(my resultUnits)
	end convert
end script

script AreaConverter
	property parent : Converter
	property unitTypes : {"square meters", "square kilometers", "square feet", "square yards", "square miles"}
	
	on convert()
		continue convert()
		
		if my fromMeasure = "square meters" then
			set my resultUnits to my fromUnits as square meters
		else if my fromMeasure = "square kilometers" then
			set my resultUnits to my fromUnits as square kilometers
		else if my fromMeasure = "square feet" then
			set my resultUnits to my fromUnits as square feet
		else if my fromMeasure = "square yards" then
			set my resultUnits to my fromUnits as square yards
		else if my fromMeasure = "square miles" then
			set my resultUnits to my fromUnits as square miles
		else if my fromMeasure = "feet" then
			set my resultUnits to my fromUnits as feet
		else if my fromMeasure = "inches" then
			set my resultUnits to my fromUnits as inches
		end if
		
		if my toMeasure = "square meters" then
			set my resultUnits to my resultUnits as square meters
		else if my toMeasure = "square kilometers" then
			set my resultUnits to my resultUnits as square kilometers
		else if my toMeasure = "square feet" then
			set my resultUnits to my resultUnits as square feet
		else if my toMeasure = "square yards" then
			set my resultUnits to my resultUnits as square yards
		else if my toMeasure = "square miles" then
			set my resultUnits to my resultUnits as square miles
		end if
		
		my updateResult(my resultUnits)
	end convert
end script

script TemperatureConverter
	property parent : Converter
	property unitTypes : {"degrees Fahrenheit", "degrees Celsius", "degrees Kelvin"}
	
	on convert()
		continue convert()
		
		if my fromMeasure = "degrees Fahrenheit" then
			set my resultUnits to my fromUnits as degrees Fahrenheit
		else if my fromMeasure = "degrees Celsius" then
			set my resultUnits to my fromUnits as degrees Celsius
		else if my fromMeasure = "degrees Kelvin" then
			set my resultUnits to my fromUnits as degrees Kelvin
		end if
		
		if my toMeasure = "degrees Fahrenheit" then
			set my resultUnits to my resultUnits as degrees Fahrenheit
		else if my toMeasure = "degrees Celsius" then
			set my resultUnits to my resultUnits as degrees Celsius
		else if my toMeasure = "degrees Kelvin" then
			set my resultUnits to my resultUnits as degrees Kelvin
		end if
		
		my updateResult(my resultUnits)
	end convert
end script

(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)