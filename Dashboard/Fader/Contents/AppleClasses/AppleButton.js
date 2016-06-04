/*
 Copyright © 2005, Apple Computer, Inc.  All rights reserved.
 NOTE:  Use of this source code is subject to the terms of the Software
 License Agreement for Mac OS X, which accompanies the code.  Your use
 of this source code signifies your agreement to such license terms and
 conditions.  Except as expressly granted in the Software License Agreement
 for Mac OS X, no other copyright, patent, or other intellectual property
 license or right is granted, either expressly or by implication, by Apple.
 */

function AppleButton(button, text, height,
					 imgLeft, imgLeftClicked, imgLeftWidth,
					 imgMiddle, imgMiddleClicked,
					 imgRight, imgRightClicked, imgRightWidth,
					 onclick)
{	
	if (button == null)
		return;
	
	/* Objects */
	this.textElement = null;
	
	/* Read-write properties */
	this.onclick = onclick;
	
	/* Read-only properties */
	this.enabled = true;
	
	
	this._init(button, text, height, 
			   imgLeft, imgLeftClicked, imgLeftWidth,
			   imgMiddle, imgMiddleClicked,
			   imgRight, imgRightClicked, imgRightWidth);
	
	this.textElement.innerHTML = text;
}

AppleButton.prototype._init = function(button, text, height,
									   imgLeft, imgLeftClicked, imgLeftWidth,
									   imgMiddle, imgMiddleClicked,
									   imgRight, imgRightClicked, imgRightWidth)
{
	this._imgLeftPath = imgLeft;
	this._imgLeftClickedPath = imgLeftClicked;
	this._imgMiddlePath = imgMiddle;
	this._imgMiddleClickedPath = imgMiddleClicked;
	this._imgRightPath = imgRight;
	this._imgRightClickedPath = imgRightClicked;
	
	var container = document.createElement("div");
	this._container = container;

	button.appendChild(container);

	// For JavaScript event handlers
	var _self = this;
	this._mousedownHandler = function(event) { _self._mousedown(event); }
	this._mousemoveHandler = function(event)
	{
		event.stopPropagation();
		event.preventDefault();
	}
	this._mouseoverHandler = function(event) { _self._mouseover(event); }
	this._mouseoutHandler = function(event) { _self._mouseout(event); }
	this._mouseupHandler = function(event) { _self._mouseup(event); }
	
	// Create the inner elements	
	var element = document.createElement("div");
	var style = element.style;
	style.display = "inline-block";
	style.background = "url(" + this._imgLeftPath + ") no-repeat top left";
	style.height = height + "px";
	style.width = imgLeftWidth + "px";
	container.appendChild(element);
	
	element = document.createElement("div");
	element.innerText = text;
	style = element.style;
	style.display = "inline-block";
	style.backgroundRepeat = "repeat-x";
	style.backgroundImage = "url(" + this._imgMiddlePath + ")";
	style.lineHeight = height + "px";
	style.height = height + "px";
	style.overflow = "hidden";
	style.whiteSpace = "nowrap";
	
	container.appendChild(element);
	this.textElement = element;
	
	element = document.createElement("div");
	style = element.style;
	style.display = "inline-block";
	style.background = "url(" + this._imgRightPath + ") no-repeat top left";
	style.height = height + "px";
	style.width = imgLeftWidth + "px";
	container.appendChild(element);
	style = container.style;
	style.appleDashboardRegion = "dashboard-region(control rectangle)";
	style.height = height + "px";
	
	// preload clicked images
	var img = new Image(imgLeftWidth, height);
	img.src = this._imgLeftClickedPath;
	img = new Image();
	img.src = this._imgMiddleClickedPath;
	img = new Image(imgRightWidth, height);
	img.src = this._imgRightClickedPath;
	
	container.addEventListener("mousedown", this._mousedownHandler, true);
}

AppleButton.prototype.remove = function()
{	
	var parent = this._container.parentNode;
	parent.removeChild(this._container);
}

AppleButton.prototype.setDisabledImages = function(imgLeftDisabled, imgMiddleDisabled, imgRightDisabled)
{
	this._imgLeftDisabledPath = imgLeftDisabled;
	this._imgMiddleDisabledPath = imgMiddleDisabled;
	this._imgRightDisabledPath = imgRightDisabled;
}

AppleButton.prototype.setEnabled = function(enabled)
{
	this.enabled = enabled;
	if (enabled)
	{
		this._container.children[0].style.backgroundImage = "url(" + this._imgLeftPath + ")";
		this._container.children[1].style.backgroundImage = "url(" + this._imgMiddlePath + ")";
		this._container.children[2].style.backgroundImage = "url(" + this._imgRightPath + ")";
		this._container.style.appleDashboardRegion = "dashboard-region(control rectangle)";
	}
	else if (this._imgLeftDisabledPath !== undefined)
	{
		this._container.children[0].style.backgroundImage = "url(" + this._imgLeftDisabledPath + ")";
		this._container.children[1].style.backgroundImage = "url(" + this._imgMiddleDisabledPath + ")";
		this._container.children[2].style.backgroundImage = "url(" + this._imgRightDisabledPath + ")";
		this._container.style.appleDashboardRegion = "none";
	}
}


/*********************
* Private handlers
*/

AppleButton.prototype._setPressed = function(pressed)
{
	if (pressed)
	{
		this._container.children[0].style.backgroundImage = "url(" + this._imgLeftClickedPath + ")";
		this._container.children[1].style.backgroundImage = "url(" + this._imgMiddleClickedPath + ")";
		this._container.children[2].style.backgroundImage = "url(" + this._imgRightClickedPath + ")";
	}
	else
	{
		this._container.children[0].style.backgroundImage = "url(" + this._imgLeftPath + ")";
		this._container.children[1].style.backgroundImage = "url(" + this._imgMiddlePath + ")";
		this._container.children[2].style.backgroundImage = "url(" + this._imgRightPath + ")";
	}
}

AppleButton.prototype._mousedown = function(event)
{
	// If we're disabled, don't do anything
	if (!this.enabled)
	{
		event.stopPropagation();
		event.preventDefault();
		return;
	}
	
	// Change images to clicked state
	this._setPressed(true);
	
	// add temp event listeners
	document.addEventListener("mousemove", this._mousemoveHandler, true);
	document.addEventListener("mouseup", this._mouseupHandler, true);
	this._container.addEventListener("mouseover", this._mouseoverHandler, true);
	this._container.addEventListener("mouseout", this._mouseoutHandler, true);
	
	this._inside = true;
	
	event.stopPropagation();
	event.preventDefault();
}

AppleButton.prototype._mouseover = function(event)
{
	// Change images to clicked state
	this._setPressed(true);
	
	this._inside = true;
	
	event.stopPropagation();
	event.preventDefault();		
}

AppleButton.prototype._mouseout = function(event)
{
	// Change images to regular state
	this._setPressed(false);
	
	this._inside = false;
	
	event.stopPropagation();
	event.preventDefault();	
}

AppleButton.prototype._mouseup = function(event)
{
	// Change images to regular state
	this._setPressed(false);
	
	// Remove temp event listeners
	document.removeEventListener("mousemove", this._mousemoveHandler, true);
	document.removeEventListener("mouseup", this._mouseupHandler, true);
	this._container.removeEventListener("mouseover", this._mouseoverHandler, true);
	this._container.removeEventListener("mouseout", this._mouseoutHandler, true);
	
	// Perform callback if we're inside the button
	try {
		if (this._inside && this.onclick != null)
			this.onclick(event);
	} catch(ex) {
		throw ex;
	} finally {
		event.stopPropagation();
		event.preventDefault();
		delete this._inside;
	}
}

//
// AppleGlassButton class
//

function AppleGlassButton(button, text, onclick)
{
	/* Objects */
	this.textElement = null;
	
	/* Read-write properties */
	this.onclick = onclick;
	
	/* Read-only properties */
	this.enabled = true;
	
	this._init(button, text, 23,
			   "file:///System/Library/WidgetResources/button/glassbuttonleft.png",
			   "file:///System/Library/WidgetResources/button/glassbuttonleftclicked.png",
			   10,
			   "file:///System/Library/WidgetResources/button/glassbuttonmiddle.png",
			   "file:///System/Library/WidgetResources/button/glassbuttonmiddleclicked.png",
			   "file:///System/Library/WidgetResources/button/glassbuttonright.png",
			   "file:///System/Library/WidgetResources/button/glassbuttonrightclicked.png",
			   10);
	
	var style = this.textElement.style;
	style.fontSize = "12px";
	style.fontFamily = "Helvetica Neue";
	style.color = "white";
	style.fontWeight = "bold";
}

// Inherit from AppleButton
AppleGlassButton.prototype = new AppleButton(null);

// Override regular disabled functionality
AppleGlassButton.prototype.setEnabled = function(enabled)
{
	this.enabled = enabled;
	if (enabled)
	{
		this._container.children[1].style.color = "white";
		this._container.style.appleDashboardRegion = "dashboard-region(control rectangle)";
	}
	else
	{
		this._container.children[1].style.color = "rgb(150,150,150)";
		this._container.style.appleDashboardRegion = "none";
	}
}
