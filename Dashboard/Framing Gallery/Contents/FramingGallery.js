/*

File: FramingGallery.js

Abstract: JavaScript for Framing Gallery sample widget

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

/*  JavaScript
 *  The implementation of your widget lies in the <script> portion of the HTML
 *  file. The widget object is provided by Dashboard for Dashbaord-specific 
 *  interaction, like working with preferences and certain event handlers. See
 *  the Dashboard documentation for more on these.
 */

// Global variables used throughout the JavaScript functions

var height = 0;
var width = 0;
var glassDoneButton;	// a variable to reference the glass button
var infoButton;			// a variable to reference the info button


// Resizes the "picture" and widget window when a new picture is added.

function resize(x,y)
{
	if(x >= 400) {			// the maximum image width is 400px, so if the image
		y = y * (400/x);	// is larger, we need to scale it down
		x = 400;
	}
	
	if(y >= 400) {			// the maximum image height is also 400px
		x = x * (400/y);
		y = 400;
	}

	if(x < 180) {			// the minimum image width is 180px, so if the image
		y = y * (180/x);	// is smaller, then it needs to be scaled up
		x = 180;
	}

	if(y < 100) {			// the minimum image height is 100px
		x = x * (100/y);
		y = 100;
	}
	
	document.picture.width = x;		// once the dimensions are determined, the
	document.picture.height = y;	// "picture" <img> is sized accordingly

	height = y + 52;		// the frame and mat are 52px, so the widget window
	width = x + 52;			// needs to be 52px larger than the image
	
	if(window.widget) 								// you check to see if the widget is
	{										// running in Dashboard, and then
		window.resizeTo(width,height);		// resize the widget window
	}
	
	back.style.height = height - 4;			// the preferences layer is also resized
	back.style.width = width - 6;			// to fit the new dimensions

	hanger.style.left = ( (width/2) - 50 );			// when the widget is resized,
	framePopup.style.left = ( (width/2) - 50 );		// various preference elements
	frameText.style.left = ( (width/2) - 92); 		// need to be repositioned
	matPopup.style.left = ( (width/2) - 50 );
	matText.style.left = ( (width/2) - 80); 
	doneButton.style.left = ( (width/2) - 29);

	dndindicator.style.left = ( (width/2) - 103 );	// the drag and drop indicator needs
	dndindicator.style.bottom = 30;					// to be repositioned as well.  It's
													// located at the bottom middle.
}


// The resize event handler function.

function preResize()
{
	// calls the resize function with the size of the new picture
	
	resize(parseInt(document.hiddenPic.width), parseInt(document.hiddenPic.height));

}

// Since this widget allows for multiple instances of itself, each needs
// to retain its own preferences.  Dashboard provides each widget with a 
// unique identifier that persists between logins.  This wrapper provides
// a property tied with the unique identifier.

function makeKey(key)
{
	return widget.identifier + "-" + key;
}


// The preference arrow that appears at the bottom-right of the mat may be white
// or black, depending on the mat color.  This function is a wrapper that adjusts
// the image to what it should be, and records the preference.

function prefArrowColor(color)
{
	if(color == "white")
	{
		infoButton.setStyle("white", "white");
		widget.setPreferenceForKey("white",makeKey("flipitColor"));
	}
	else {
		infoButton.setStyle("black", "black");
		widget.setPreferenceForKey("black",makeKey("flipitColor"));
	}
}


// The setup function is called upon the loading of the widget.  It initializes
// the various elements of the widget if they have preferences.  If not, the default
// settings are used. It also calls constructors for the AppleInfoButton and 
// AppleGlassButton classes that populate existing divs with their respective controls.

function setup()
{
	glassDoneButton = new AppleGlassButton(document.getElementById("doneButton"), "Done", hidePrefs);
	infoButton = new AppleInfoButton(document.getElementById("infoButton"), document.getElementById("front"), "black", "black", showPrefs);

	if(window.widget)
	{
		// The frame preferences are retrieved:
		var frameTop = widget.preferenceForKey(makeKey("frameTop"));
		var frameRight = widget.preferenceForKey(makeKey("frameRight"));
		var frameBottom = widget.preferenceForKey(makeKey("frameBottom"));
		var frameLeft = widget.preferenceForKey(makeKey("frameLeft"));

		if (frameTop && frameTop.length > 0)				// if there are preferences,
		{													// they are restored
			frame.style.borderTopColor=frameTop;
			frame.style.borderRightColor=frameRight;
			frame.style.borderBottomColor=frameBottom;
			frame.style.borderLeftColor=frameLeft;
		}
		
		// The mat preferences are retrieved:
		var matColor = widget.preferenceForKey(makeKey("matColor"));
		
		if (matColor && matColor.length > 0)				// if there was a preference
		{													// for this instance, it is
			mat.style.borderColor=matColor;					// restored
		}
		
		// The preference button preferences are rereived:
		var flipitColor = widget.preferenceForKey(makeKey("flipitColor"));
		
		if (flipitColor && flipitColor.length > 0)			// if there was a preference
		{													// for this instance, it is
			prefArrowColor(flipitColor);					// restored
		}
		
		// The image preferences are retrieved:
		var picturesrc = widget.preferenceForKey(makeKey("image"));
		
		if (picturesrc && picturesrc.length > 0)			// if there was a preference
		{													// for this instance, it is
			var img, img2;									// assigned to the "picture"
			img = document.getElementById ("picture");		// and the "hiddenPic" (used
			img2 = document.getElementById ("hiddenPic");	// to ascertain image dimensions).
			img.src = img2.src = picturesrc;				// A load handler is assigned
			img.onload = preResize;							// for when the image has loaded
		}
		else {												// if no preference was found
			preResize();									// the widget should be sized
		}													// for the default image
	}
}


// The event handler for the image drop.  This handles fetching the image URL and trying
// to place it inside of the widget.

function dragdrop (event)
{
	var uri = null;
	try {
	    uri = event.dataTransfer.getData("text/uri-list");	// attempt to load the new
	} catch (ex)											// image
	{
	}
	
	// if the acquisition is successful:
	if (uri)
	{
		if(window.widget)
		{
			widget.setPreferenceForKey(uri,makeKey("image"));	// set the preference for
		}														// the new image
		var img, img2;
		img = document.getElementById ("picture");				// then, assign the new
		img2 = document.getElementById ("hiddenPic");			// image to the "picture"
		img.src = img2.src = uri;								// and the "hiddenPic"; a
		img.onload = preResize;									// load handler is called
	}															// when the image has loaded

	event.stopPropagation();
	event.preventDefault();
}


// The dragenter, dragover, and dragleave functions are implemented but not used.  They
// can be used if you want to change the image when it enters the widget.

function dragenter (event)
{
	event.stopPropagation();
	event.preventDefault();
}

function dragover (event)
{
	event.stopPropagation();
	event.preventDefault();
}

function dragleave (event)
{
	event.stopPropagation();
	event.preventDefault();
}


// The showPrefs function is called when the preferences button is clicked.  It prepares
// the widget to show the preferences, and then flips them.  More on this can be found in
// the Dashboard documentation.

function showPrefs()
{
	var front = document.getElementById("front");		// first, you need to get the front
	var back = document.getElementById("back");			// and back layers
	
	if(window.widget)									// this freezes the currently visible
		widget.prepareForTransition("ToBack");			// layer
	
	front.style.display="none";							// the preferences layer is made
	back.style.display="block";							// visible and the picture is hidden
				
	if(window.widget)									// the flip transition is run, with the
		setTimeout ('widget.performTransition();', 0);	// frozen side on the front and the
														// preferences on the back
}

// When the "Done" button is clicked in the preferences, this function is called. It swaps
// the preference layer out with the picture and frame.

function hidePrefs()
{
	
	document.getElementById("doneButton").src = "Images/done.png";  // restore the regular
																	// "Done" button

	var front = document.getElementById("front");		// again, you need to obtain the front
	var back = document.getElementById("back");			// and back layers
	
	if (window.widget)									// then freeze the visible layer (as seen
		widget.prepareForTransition("ToFront");			// by the user
	
	front.style.display="block";						// hide the preferences layer and shot the 
	back.style.display="none";							// main picture layer
	
	if (window.widget)									// and run the flip transition; note that since
		setTimeout ('widget.performTransition();', 0);	// finishEdit() was used to freeze the UI, this 
}														// the widget in the opposite direction

// changeFrame is called by the frame popup when its value is changed.  This function
// changes the border color attributes for the frame, simulating different picture
// frame styles
function changeFrame(elem)
{	
	
	var frame = document.getElementById("frame");	// the frame element is obtained
	
	// based on the value of the popup menu, the proper frame colors are applied
	switch( parseInt(elem.options[elem.selectedIndex].value) )
	{
		case 1: 									// black
			frame.style.borderTopColor="#030303";
			frame.style.borderRightColor="#1A1B1A";
			frame.style.borderBottomColor="#030303";
			frame.style.borderLeftColor="#1A1B1A";
			break;
		case 2:										// rosewood
			frame.style.borderTopColor="#660000";
			frame.style.borderRightColor="#8C0B04";
			frame.style.borderBottomColor="#660000";
			frame.style.borderLeftColor="#8C0B04";
			break;
		case 3:										// oak
			frame.style.borderTopColor="#A04400";
			frame.style.borderRightColor="#BB4E00";
			frame.style.borderBottomColor="#A04400";
			frame.style.borderLeftColor="#BB4E00";
			break;
		case 4:										// pine
			frame.style.borderTopColor="#C5A17F";
			frame.style.borderRightColor="#DBC0A5";
			frame.style.borderBottomColor="#C5A17F";
			frame.style.borderLeftColor="#DBC0A5";
			break;
		case 5:										// gold
			frame.style.borderTopColor="#94732A";
			frame.style.borderRightColor="#E5C062";
			frame.style.borderBottomColor="#94732A";
			frame.style.borderLeftColor="#E5C062";
			break;
	}
	
	// once the frame is changed, the new styles are saved as preferences
	if(window.widget)
	{
		widget.setPreferenceForKey(frame.style.borderTopColor,makeKey("frameTop"));
		widget.setPreferenceForKey(frame.style.borderRightColor,makeKey("frameRight"));
		widget.setPreferenceForKey(frame.style.borderBottomColor,makeKey("frameBottom"));
		widget.setPreferenceForKey(frame.style.borderLeftColor,makeKey("frameLeft"));
	}

} 

// changeMat is called by the mat popup when its value is changed.  This function
// changes the border color attribute for the mat, simulating different mat colors
function changeMat(elem)
{	
	
	var mat = document.getElementById("mat");	// the mat element is obtained
	
	// based on the value of the popup menu, the proper mat color is applied.  Also,
	// the correct preference button is set here as well
	switch( parseInt(elem.options[elem.selectedIndex].value) )
	{
		case 1:								// arctic white
			mat.style.borderColor="#F8F7F1";
			prefArrowColor("black");
			break;
		case 2:								// venetian red
			mat.style.borderColor="#9F1818";
			prefArrowColor("white");
			break;
		case 3:								// navy blue
			mat.style.borderColor="#183D89";
			prefArrowColor("white");
			break;
		case 4:								// hunter green
			mat.style.borderColor="#085B33";
			prefArrowColor("white");
			break;
		case 5:								// pumpkin orange
			mat.style.borderColor="#ED9711";
			prefArrowColor("black");
			break;
		case 6:								// royal purple
			mat.style.borderColor="#663292";
			prefArrowColor("white");
			break;
		case 7:								// sunrise yellow
			mat.style.borderColor="#EED10F";
			prefArrowColor("black");
			break;
		case 8:								// raven black
			mat.style.borderColor="#191919";
			prefArrowColor("white");
			break;
	}
	
	// once the mat is changed, the new style is saved as a preference
	if(window.widget)
	{
		widget.setPreferenceForKey(mat.style.borderTopColor,makeKey("matColor"));
	}
} 


// onremove clears away any preferences that this widget may have had.

function onremove()
{
		widget.setPreferenceForKey(null,makeKey("image"));

		widget.setPreferenceForKey(null,makeKey("frameTop"));
		widget.setPreferenceForKey(null,makeKey("frameRight"));
		widget.setPreferenceForKey(null,makeKey("frameBottom"));
		widget.setPreferenceForKey(null,makeKey("frameLeft"));

		widget.setPreferenceForKey(null,makeKey("matColor"));
		
		widget.setPreferenceForKey(null,makeKey("flipitColor"));

}

// this is where widget-specific handlers can be declared.  This widget only uses one;
// it is called when the widget is removed from Dashbaord

if(window.widget)
{
	widget.onremove = onremove;
}

var currentAnimator;
var currentAnimation;
var globalFrom = 0;

// fadeIn() is called when the mouse is over the widget
function fadeIn()
{
		if(currentAnimator != undefined) {
			currentAnimator.stop();
			delete currentAnimator;
		}
		
		currentAnimator = new AppleAnimator (500, 13);
		currentAnimator.oncomplete = dispose;
		currentAnimation = new AppleAnimation (globalFrom, 1.0, setOpacity)
		currentAnimator.addAnimation(currentAnimation);
	
		currentAnimator.start();
}

// fadeOut() is called when the mouse leaves the widget
function fadeOut()
{
		if(currentAnimator != undefined) {
			currentAnimator.stop();
			delete currentAnimator;
		}
		
		currentAnimator = new AppleAnimator (500, 13);
		currentAnimator.oncomplete = dispose;
		currentAnimation = new AppleAnimation (globalFrom, 0.0, setOpacity)
		currentAnimator.addAnimation(currentAnimation);
	
		currentAnimator.start();
}

function setOpacity(animation, now, first, done)
{
	document.getElementById("dndindicator").style.opacity = now;
	globalFrom = now;
}

function dispose()
{
	delete currentAnimator;
	delete currentAnimation;		
}
