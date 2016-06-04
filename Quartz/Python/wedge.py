#!/usr/bin/python

# wedge.py
# Copyright (c) 2004, Apple Computer, Inc., all rights reserved.

# IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
# consideration of your agreement to the following terms, and your use, installation,
# modification or redistribution of this Apple software constitutes acceptance of these
# terms.  If you do not agree with these terms, please do not use, install, modify or
# redistribute this Apple software.

# In consideration of your agreement to abide by the following terms, and subject to these
# terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
# this original Apple software (the "Apple Software"), to use, reproduce, modify and
# redistribute the Apple Software, with or without modifications, in source and/or binary
# forms; provided that if you redistribute the Apple Software in its entirety and without
# modifications, you must retain this notice and the following text and disclaimers in all
# such redistributions of the Apple Software.  Neither the name, trademarks, service marks
# or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
# the Apple Software without specific prior written permission from Apple. Except as expressly
# stated in this notice, no other rights or licenses, express or implied, are granted by Apple
# herein, including but not limited to any patent rights that may be infringed by your
# derivative works or by other works in which the Apple Software may be incorporated.

# The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
# USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

# IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
# REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
# WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
# OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from CoreGraphics import *
import math

#  For old times sake -- the PostScript cookbook wedge example in python...

def inch(x):
	return 72*x

def rad(angle):
	return angle*math.pi/180.0

def wedge(c):
	c.beginPath()
	c.moveToPoint(0,0)
	c.translateCTM(1,0)
	c.rotateCTM(rad(16))
	c.translateCTM(0, math.sin(rad(15)))
	c.addArc(0,0, math.sin(rad(15)), rad(-90), rad(90), 0)
	c.closePath()

pageRect = CGRectMake (0, 0, 612, 792)   #  landscape
c = CGPDFContextCreateWithFilename ("wedge.pdf", pageRect)
c.beginPage(pageRect)
c.saveGState()
c.translateCTM(inch(4.25), inch(4.25))
c.scaleCTM(inch(1.75), inch(1.75))
c.setLineWidth(.02)
for i in range(1,13):
	c.setGrayFillColor(i/12.0,1.0)
	c.saveGState()
	wedge(c)
	c.fillPath()
	c.restoreGState()
	c.saveGState()
	c.setGrayStrokeColor(0,1)
	wedge(c)
	c.strokePath()
	c.restoreGState()
	c.rotateCTM(rad(30))
c.restoreGState()
c.endPage()
c.finish()
