/*

File: AppleInfoButton.js

Abstract: Standard Apple class for a widget info button; included in this
	widget under an "AppleClasses" directory for compatibility with Mac OS X
	10.4.0-10.4.2.  On Mac OS X 10.4.3 and later, Dashboard will first check
	in /System/Library/WidgetResources when loading script references to the
	AppleClasses directory.

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
 Copyright Â© 2005, Apple Computer, Inc.  All rights reserved.
 NOTE:  Use of this source code is subject to the terms of the Software
 License Agreement for Mac OS X, which accompanies the code.  Your use
 of this source code signifies your agreement to such license terms and
 conditions.  Except as expressly granted in the Software License Agreement
 for Mac OS X, no other copyright, patent, or other intellectual property
 license or right is granted, either expressly or by implication, by Apple.
 */

function AppleInfoButton(flipper, front, foregroundStyle, backgroundStyle, onclick)
{
	/* Read-write properties */
    this.onclick = onclick;
    
    /* Internal */
	this._front = front;
	this._flipper = flipper;
	this._flipLabel = document.createElement("img");
	this._flipLabel.width = 13;
	this._flipLabel.height = 13;
	this._flipLabel.setAttribute("alt", "Info");
	this._flipCircle = document.createElement("div");
	flipper.appendChild(this._flipCircle);
	flipper.appendChild(this._flipLabel);
    this._labelshown = false;
	
	// For JavaScript event handlers
	var _self = this;
	
	this._updateOpacity = function(animation, now, first, done)
	{
		_self._flipLabel.style.opacity = now;
	}
	
	this._animationComplete = function()
	{
		delete _self._animation;
		delete _self._animator;
	}
	
	this._frontMove = function(event)
	{
		if (_self._outdelay !== undefined)
		{
			clearInterval(_self._outdelay);
			delete _self._outdelay;
		}
		if (_self._labelshown)
			return;
		
		var from = 0.0;
		var duration = 500;
		if (_self._animation !== undefined)
		{
			from = _self._animation.now;
			duration = (new Date).getTime() - _self._animator.startTime;
			_self._animator.stop();
		}
		
		_self._labelshown = true;
		
		var animator = new AppleAnimator(duration, 13);
		animator.oncomplete = _self._animationComplete;
		_self._animator = animator;
		
		_self._animation = new AppleAnimation(from, 1.0, _self._updateOpacity);
		animator.addAnimation(_self._animation);
		animator.start();
	}
	
	this._frontOutDelay = function(event)
	{
		_self._outdelay = setInterval(_self._frontOut, 0, _self);
	}
	
	this._frontOut = function(_self)
	{
		if (_self._outdelay !== undefined)
		{
			clearInterval(_self._outdelay);
			delete _self._outdelay;
		}
		if (!_self._labelshown)
			return;
		
		var from = 1.0;
		var duration = 500;
		if (_self._animation !== undefined)
		{
			from = _self._animation.now;
			duration = (new Date).getTime() - _self._animator.startTime;
			_self._animator.stop();
		}
		
		var animator = new AppleAnimator(duration, 13);
		animator.oncomplete = _self._animationComplete;
		_self._animator = animator;
		
		_self._animation = new AppleAnimation(from, 0.0, _self._updateOpacity);
		animator.addAnimation(_self._animation);
		animator.start();
	
		_self._labelshown = false;
	}
	
	this._labelOver = function(event)
	{
		_self._flipCircle.style.visibility = "visible";
	}
	
	this._labelOut = function(event)
	{
		_self._flipCircle.style.visibility = "hidden";
	}
	
	this._labelClicked = function(event)
	{
		_self._flipCircle.style.visibility = "hidden";
		
		try {
			if (_self.onclick != null)
				_self.onclick(event);
		} catch(ex) {
			throw ex;
		} finally {
			event.stopPropagation();
    	    event.preventDefault();
    	}
	}

	// Set up style
	var style = this._flipLabel.style;
	style.position = "absolute";
	style.top = 0;
	style.left = 0;
	style.opacity = 0;
	
	style = this._flipCircle.style;
	style.position = "absolute";
	style.top = 0;
	style.left = 0;
	style.width = "13px";
	style.height = "13px";
	this.setCircleOpacity(0.25);
	style.visibility = "hidden";
	
	this.setStyle(foregroundStyle, backgroundStyle);
	
	this._front.addEventListener("mousemove", this._frontMove, true);
	this._front.addEventListener("mouseout", this._frontOutDelay, true);
	
	this._flipper.addEventListener("click", this._labelClicked, true);
	this._flipper.addEventListener("mouseover", this._labelOver, true);
	this._flipper.addEventListener("mouseout", this._labelOut, true);
}

AppleInfoButton.prototype.remove = function()
{
	this._front.removeEventListener("mousemove", this._frontMove, true);
	this._front.removeEventListener("mouseout", this._frontOutDelay, true);
	
	this._flipper.removeEventListener("click", this._labelClicked, true);
	this._flipper.removeEventListener("mouseover", this._labelOver, true);
	this._flipper.removeEventListener("mouseout", this._labelOut, true);
	
	var parent = this._flipLabel.parentNode;
	parent.removeChild(this._flipCircle);
	parent.removeChild(this._flipLabel);
}

AppleInfoButton.prototype.setStyle = function(foregroundStyle, backgroundStyle)
{
	this._flipLabel.src = "file:///System/Library/WidgetResources/ibutton/" + foregroundStyle + "_i.png";
	this._flipCircle.style.background = "url(file:///System/Library/WidgetResources/ibutton/" + backgroundStyle + "_rollie.png) no-repeat top left";
}

AppleInfoButton.prototype.setCircleOpacity = function(opacity)
{
	this._flipCircle.style.opacity = opacity;
}
