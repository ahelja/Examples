(* Application.applescript *)

(* ==== Globals ==== *)

global converterLib
global logLib
global LogController
global Converter
global LengthConverter
global WeightConverter
global LiquidVolumeConverter
global VolumeConverter
global AreaConverter
global TemperatureConverter


(* ==== Properties ==== *)

property currentConverter : 0
property scriptsLoaded : false


(* ==== Event Handlers ==== *)

on clicked theObject
	tell window "Main"
		if theObject is equal to button "Convert" of box 1 then
			my convert()
		else if the theObject is equal to button "Drawer" then
			if state of drawer "Log" is drawer closed then
				tell drawer "Log" to open drawer on bottom edge
			else
				tell drawer "Log" to close drawer
			end if
		else if the theObject is equal to button "Clear" of box 1 of drawer "Log" then
			tell LogController to clearLog()
		else if the theObject is equal to button "Save As" of box 1 of drawer "Log" then
			set logFile to choose file name with prompt "Save Log As" default name "Conversion Results.txt"
			tell LogController to saveLogInFile(logFile)
		end if
	end tell
end clicked

on choose menu item theObject
	tell window "Main"
		if theObject is equal to popup button "Type" of box 1 then
			set currentConverter to my getConverterForType(title of popup button "Type" of box 1)
			tell currentConverter to updateUnitTypes()
		else
			my convert()
		end if
	end tell
end choose menu item

on action theObject
	if theObject is equal to text field "Value" of box 1 of window "Main" then
		my convert()
	end if
end action

on launched theObject
	my loadScripts()
	tell LogController to initialize()
	set currentConverter to my getConverterForType(title of popup button "Type" of box 1 of window "Main")
	
	set visible of window "Main" to true
end launched


(* ==== Handlers ==== *)

on convert()
	if contents of text field "Value" of box 1 of window "Main" is equal to "" then
		display alert "You must enter a value to convert." as critical attached to window "Main"
	else
		tell currentConverter to convert()
	end if
end convert

on getConverterForType(typeName)
	if typeName is equal to "length" then
		return LengthConverter
	else if typeName is equal to "weight and mass" then
		return WeightConverter
	else if typeName is equal to "liquid volume" then
		return LiquidVolumeConverter
	else if typeName is equal to "volume" then
		return VolumeConverter
	else if typeName is equal to "area" then
		return AreaConverter
	else if typeName is equal to "temperature" then
		return TemperatureConverter
	else
		return Converter
	end if
end getConverterForType

on pathToScripts()
	set appPath to (path to me from user domain) as text
	return (appPath & "Contents:Resources:Scripts:") as text
end pathToScripts

on loadScript(scriptName)
	return load script file (my pathToScripts() & scriptName & ".scpt")
end loadScript

on loadScripts()
	set logLib to my loadScript("Log Controller")
	set converterLib to my loadScript("Converter")
	
	set LogController to LogController of logLib
	set Converter to Converter of converterLib
	set LengthConverter to LengthConverter of converterLib
	set WeightConverter to WeightConverter of converterLib
	set LiquidVolumeConverter to LiquidVolumeConverter of converterLib
	set VolumeConverter to VolumeConverter of converterLib
	set AreaConverter to AreaConverter of converterLib
	set TemperatureConverter to TemperatureConverter of converterLib
end loadScripts


(* © Copyright 2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. *)
