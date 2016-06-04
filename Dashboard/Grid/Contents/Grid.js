/*

File: Grid.js

Abstract: JavaScript for Grid sample widget
	Contains the code needed to perform a live resize

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

var growboxInset;		// tracks where the last mouse position was throughout the drag

var edgeOffset = 12;	// the right & bottom offset of the grow thumb vs. the edge of widget window


// called when the mouse first clicks upon the growbox 
function mouseDown(event)
{
	
	document.addEventListener("mousemove", mouseMove, true);  	// begin tracking the move
	document.addEventListener("mouseup", mouseUp, true);		// and notify when the drag ends

	// growboxInset tracks where the actual mouse click happened vs. the right and 
	// bottom edges of the widget
	growboxInset = {x:(window.innerWidth - event.x), y:(window.innerHeight - event.y)};

	event.stopPropagation();
	event.preventDefault();
}

var x,y;

// called as the mouse button is down and the mouse moves
function mouseMove(event)
{
	
	// checks if the reported event data is legit or not
	if((event.x == -1) && ( Math.abs( Math.abs(y-growboxInset.y) - Math.abs(event.y) ) > 2 ) ) { break; }

	// x and y track where bottom-right corner of the widget should be, with relation
	// to the event.
	x = event.x + growboxInset.x;
	y = event.y + growboxInset.y;
		
	if(x < 70) 		// an arbitrary minimum width
		x = 70;

	if(y < 25) 		// an arbitrary minimum height
		y = 25;
		
	// Moves the coordinates div accordingly
	document.getElementById("bottom").style.top = (y-edgeOffset);
 
	window.resizeTo(x,y);		// resizes the widget's window

	updateCoordinates();		// updates the coordinate display in the bottom-right
	
	event.stopPropagation();
	event.preventDefault();
}

// called after teh mouse button is released 
function mouseUp(event)
{
	document.removeEventListener("mousemove", mouseMove, true); // stop tracking the move
	document.removeEventListener("mouseup", mouseUp, true);		// and notify if the mouse goes down again
		
	event.stopPropagation();
	event.preventDefault();
}


/**************************/
// Grid-specific code
// Updates the coordinate display with the current widget size
/**************************/

function updateCoordinates()
{
	coordinates.innerText = "(" + window.innerWidth +"," + window.innerHeight + ")";
	
	event.stopPropagation();
	event.preventDefault();
}
