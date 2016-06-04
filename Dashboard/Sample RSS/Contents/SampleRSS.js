/*

File: SampleRSS.js

Abstract: JavaScript logic for RSS sample widget
	Demonstrates use of XMLHttpRequest object to display
	RSS feed of recent Dashboard widget downloads

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

var feed = {title:"Dashboard Widgets", top:"Dashboard Widgets", bottom:"US", url:"http://www.apple.com/downloads/dashboard/home/recent.rss"};			
	
var scrollArea, scrollbar;

function load ()
{
	scrollbar = new AppleVerticalScrollbar(document.getElementById("myScrollBar"));
	scrollArea = new AppleScrollArea(document.getElementById("contents"), scrollbar);
	scrollArea.scrollsHorizontally = false;
	scrollArea.singlepressScrollPixels = 15;
	
	window.onfocus = function () { scrollArea.focus(); }
	window.onblur = function () { scrollArea.blur(); }
	
	if (!window.widget)
	{
		show ();
	}
}


var last_updated = 0;
var xml_request = null;

//---------------------------------------------------------------------------------------------
//
// show - post a request to get Apple RSS Hot News when showing the widget. Space requests by
//		  at least 15 minutes to avoid hitting the server too often. Also, note that you should
//		  cancel any outstanding request before posting the new one. 
//
//----------------------------------------------------------------------------------------------
function show ()
{
	var now = (new Date).getTime();
	
	// only check if 15 minutes have passed
	if ((now - last_updated) > 900000)
	{
		if (xml_request != null)
		{
			xml_request.abort();
			xml_request = null;
		}
		xml_request = new XMLHttpRequest();

		xml_request.onload = function(e) {xml_loaded(e, xml_request);}
		xml_request.overrideMimeType("text/xml");
		xml_request.open("GET", feed.url);
		xml_request.setRequestHeader("Cache-Control", "no-cache");
		xml_request.send(null);
    }
}

if (window.widget)
{
	widget.onshow = show;
}


//---------------------------------------------------------------------------------------------
//
// findChild - scan the children of a given DOM element for a node matching nodeName; much more
//				efficient than the standard DOM methods (getElementsByTagName, etc) if you know
//				what you're looking for
//
//----------------------------------------------------------------------------------------------
function findChild (element, nodeName)
{
	var child;
	
	for (child = element.firstChild; child != null; child = child.nextSibling)
	{
		if (child.nodeName == nodeName)
			return child;
	}
	
	return null;
}

//---------------------------------------------------------------------------------------------
//
// xml_loaded - extract the content of Apple RSS Hot News feed and place the items data into a
//			    results array: extract the title, link and publication date for each item. 
//
//----------------------------------------------------------------------------------------------

function xml_loaded (e, request) 
{
	xml_request = null;
	if (request.responseXML)
	{		
		var contents = document.getElementById('contents');
		while (contents.hasChildNodes())
		{
			contents.removeChild(contents.firstChild);
		}
		
		// Get the top level <rss> element 
		var rss = findChild(request.responseXML, 'rss');
		if (!rss) {
			alert("no <rss> element"); 
			document.getElementById("ohnoes").style.display = "block";
			return;
		}
		
		// Get single subordinate channel element
		var channel = findChild( rss, 'channel');
		if (!channel) {
			alert("no <channel> element");
			document.getElementById("ohnoes").style.display = "block";
			return;
		}
		
		
		document.getElementById("ohnoes").style.display = "none";

		var results = new Array;
		
		// Get all item elements subordinate to the channel element
		// For each element, get title, link and publication date. 
		// Note that all elements of an item are optional. 
		for( var item = channel.firstChild; item != null; item = item.nextSibling)
		{
			if( item.nodeName == 'item' ) 
			{
				var title = findChild (item, 'title');
			
				// we have to have the title to include the item in the list 
				if( title != null ) 
				{
					var link = findChild (item, 'link');
					var pubDate = findChild (item, 'pubDate');
					results[results.length] = {title:title.firstChild.data, 
						link:(link != null ? link.firstChild.data : null), 
						date:new Date(Date.parse(pubDate.firstChild.data))
				   };
				}
			}
		}
		
		// sort by date
		results.sort (compFunc);
		
		// copy title and date into rows for display. Store link so it can be used when user
		// clicks on title
		nItems = results.length;
		var even = true;		
				
		for (var i = 0; i < nItems; ++i)
		{
			var item = results[i];
			var row = createRow (item.title, item.link, item.date, even);
			even = !even;
			
			contents.appendChild (row);
		}

		// update the scrollbar so scrollbar matches new data
		scrollArea.refresh();
		
		// set last_updated to the current time to keep track of the last time a request was posted
		last_updated = (new Date).getTime();
	}
}

//---------------------------------------------------------------------------------------------
//
// sortFunc - compare function used for sorting dates
//
//----------------------------------------------------------------------------------------------

function compFunc (a, b)
{
	if (a.date < b.date)
		return 1;
	else if (a.date > b.date)
		return -1;
	else
		return 0;
}

//---------------------------------------------------------------------------------------------
//
// createRow - add data to the next row in the widget body. Rows have alternating (light and 
//			   dark backgound). The title and date as displayed for each item. The link is used
//			   when the user clicks on a RSS title. 
//
//----------------------------------------------------------------------------------------------


function createRow (title, link, date, even)
{
	var row = document.createElement ('div');
	row.setAttribute ('class', 'row ' + (even ? 'light' : 'dark'));
	
	var title_div = document.createElement ('div');
	title_div.innerText = title;
	title_div.setAttribute ('class', 'title');
	if (link != null)
	{
		title_div.setAttribute ('the_link', link);
		title_div.setAttribute ('onclick', 'clickOnTitle (event, this);');
	}
	row.appendChild (title_div);
	
	if (date != null)
	{
		var date_div = document.createElement ('div');
		date_div.setAttribute ('class', 'date');
		date_div.innerText = createDateStr (date);
		
		row.appendChild (date_div);
	}
	
	return row;	
}

function createDateStr (date)
{
	var month;
	switch (date.getMonth())
	{
		case 0: month = 'Jan'; break;
		case 1: month = 'Feb'; break;
		case 2: month = 'Mar'; break;
		case 3: month = 'Apr'; break;
		case 4: month = 'May'; break;
		case 5: month = 'Jun'; break;
		case 6: month = 'Jul'; break;
		case 7: month = 'Aug'; break;
		case 8: month = 'Sep'; break;
		case 9: month = 'Oct'; break;
		case 10: month = 'Nov'; break;
		case 11: month = 'Dec'; break;
	}	
	return month + ' ' + date.getDate();
}

//---------------------------------------------------------------------------------------------
//
// clickOnTitle - take user to the RSS link when she clicks on an article's title.  
//
//----------------------------------------------------------------------------------------------

function clickOnTitle (event, div)
{
	if (window.widget)
	{
		widget.openURL (div.the_link);
	} else document.location = div.the_link;
}

//---------------------------------------------------------------------------------------------
//
// clickOnFeedTitle - take user to the feed main web page when she clicks on the widget title. 
//
//----------------------------------------------------------------------------------------------

function clickOnFeedTitle(event)
{
	if (window.widget)
	{
			widget.openURL (feed.url);
	} else document.location = feed.url;
}