/*

File: Scroller.js

Abstract: Basic support code for Scroller example widget.  Provides three 
	divs with varying amounts of content to demonstrate proportional sizing of
	the scroll thumb.

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

/*
 ****************************************************
 *	Main content (onload, content switching, etc	*
 ****************************************************
 */

var scrollbar, scrollArea;

function loaded() {
	scrollbar = new AppleVerticalScrollbar(document.getElementById("myScrollBar"));
	scrollArea = new AppleScrollArea(document.getElementById("parentDiv"), scrollbar);
	scrollArea.scrollsHorizontally = false;
	scrollArea.singlepressScrollPixels = 25;
	scrollArea.focus(); // for key control when first loading in Safari
	
	window.onfocus = function () { scrollArea.focus(); }
	window.onblur = function () { scrollArea.blur(); }

	showConstitution();
}

var oldContent;
var oldControl;

// Switches between the content DIVs.
function showContent(newContent, newControl) {
	if (oldContent != null) {
		oldContent.style.display = 'none';
		oldControl.style.color = 'gray';
	}
	
	newControl.style.color = 'black';
	newContent.style.display = 'block';
	
	// Resize, reposition the scrollthumb
	scrollArea.reveal(newContent);
	scrollArea.refresh();

	oldContent = newContent;
	oldControl = newControl;
}	

function showGettysburg() {
	showContent(document.getElementById('gettysburg'), document.getElementById('gbaControl'));
}

function showConstitution() {
	showContent(document.getElementById('constitution'), document.getElementById('uscControl')); 
}

function showFear() {
	showContent(document.getElementById('fear'), document.getElementById('fearControl')); 
}