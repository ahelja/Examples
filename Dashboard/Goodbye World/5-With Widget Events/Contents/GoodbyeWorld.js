/*

File: GoodbyeWorld.js

Abstract: JavaScript for GoodbyeWorld sample widget
	This is the fifth milestone, including preference
	cleanup in the onremove handler as well as response to
	onfocus and onblur events

Version: 2.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

/***********************************/
// SAVING AND RETRIEVING PREFERENCES
/***********************************/

// Global variables for the info and done buttons; keeping these around lets us modify them later
var glassDoneButton;
var whiteInfoButton;

// setup() is the onload handler for the widget. It calls constructors for the AppleInfoButton
// and AppleGlassButton classes that populate existing divs with their respective controls.
// It checks to see if there is a preference for this widget  and if so, applies the preference 
// to the widget.

function setup()
{
	glassDoneButton = new AppleGlassButton(document.getElementById("doneButton"), "Done", hidePrefs);
	whiteInfoButton = new AppleInfoButton(document.getElementById("infoButton"), document.getElementById("front"), "white", "white", showPrefs);

	if(window.widget)		// always check to make sure that you are running in Dashboard
	{
		// The preferences are retrieved:
		var worldString = widget.preferenceForKey(makeKey("worldString"));

		if (worldString && worldString.length > 0)  // if the retrieved preferences are not empty,
		{											// they are restored.
			document.getElementById("worldText").innerText = worldString;

			if( worldString == "Goodbye, World!" )	// if Goodbye is the retreived value...
			{
				document.getElementById("worldPopup").selectedIndex = 1;	// set the popup to reflect that.
			}
		}
	}
}


// changeWorld() is called whenever a menu item is chosen in the widget's preferences.  It queries the
// menu to find out which option was chosen, applies the change to the widget, and saves the preference.

function changeWorld(elem)
{	
	var world = document.getElementById("worldText");
	
	switch( parseInt(elem.options[elem.selectedIndex].value) )					// find out which option was chosen
	{
		case 1: 									// if option #1 ("Hello, World!")	
			world.innerText="Hello, World!";		// change the front text to "Hello, World!" & adjust the style
			if(window.widget)
			{
				widget.setPreferenceForKey("Hello, World!",makeKey("worldString"));		// and save the new preference to disk
			}
			break;
		case 2:										// if option #2 ("Goodbye, World!")
			world.innerText="Goodbye, World!";		// change the front text to "Goodbye, World!" & adjust the style
			if(window.widget)
			{
				widget.setPreferenceForKey("Goodbye, World!",makeKey("worldString"));	// and save the new preference to disk
			}
			break;
	}
} 


/*********************************/
// HIDING AND SHOWING PREFERENCES
/*********************************/

// showPrefs() is called when the preferences flipper is clicked upon.  It freezes the front of the widget,
// hides the front div, unhides the back div, and then flips the widget over.

function showPrefs()
{
	var front = document.getElementById("front");
	var back = document.getElementById("back");
	
	if (window.widget)
		widget.prepareForTransition("ToBack");		// freezes the widget so that you can change it without the user noticing
	
	front.style.display="none";		// hide the front
	back.style.display="block";		// show the back
	
	if (window.widget)
		setTimeout ('widget.performTransition();', 0);		// and flip the widget over	

	document.getElementById('fliprollie').style.display = 'none';  // clean up the front side - hide the circle behind the info button

}


// hidePrefs() is called by the done button on the back side of the widget.  It performs the opposite transition
// as showPrefs() does.

function hidePrefs()
{
	var front = document.getElementById("front");
	var back = document.getElementById("back");
	
	if (window.widget)
		widget.prepareForTransition("ToFront");		// freezes the widget and prepares it for the flip back to the front
	
	back.style.display="none";			// hide the back
	front.style.display="block";		// show the front
	
	if (window.widget)
		setTimeout ('widget.performTransition();', 0);		// and flip the widget back to the front
}


// makeKey makes the widget multi-instance aware

function makeKey(key)
{
	return (widget.identifier + "-" + key);
}


/***************/
// WIDGET EVENTS
/***************/

// removed is called when the widget is removed from the Dashboard

function removed()
{

	widget.setPreferenceForKey(null,makeKey("worldString"));
}

// focused is called when the widget gets key focus

function focused()
{
	document.getElementById('worldText').style.color = 'white'; 
}

// blurred is called when the widget looses key focus

function blurred()
{
	document.getElementById('worldText').style.color = 'gray'; 
}

// Here we register for some widget events

if(window.widget)
{
	widget.onremove = removed;
	window.onfocus = focused;
	window.onblur = blurred;
}	
