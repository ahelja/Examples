#!/usr/bin/python

# cmyk-bitmap.py
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

pi = 3.1415927

# Create a CYMK bitmap context with an opaque white background

cs = CGColorSpaceCreateWithName (kCGColorSpaceUserCMYK)
c = CGBitmapContextCreateWithColor (256, 256, cs, (0,0,0,0,1))

# Draw a yellow square with a red outline in the center

c.saveGState ()
c.setRGBStrokeColor (1,0,0,1)			# red
c.setRGBFillColor (1,1,0,1)			# yellow
c.setLineWidth (3)
c.setLineJoin (kCGLineJoinBevel)
c.addRect (CGRectMake (32.5, 32.5, 191, 191))
c.drawPath (kCGPathFillStroke);
c.restoreGState ()

# Draw some text at an angle

c.saveGState ()
c.translateCTM (128, 128)
c.rotateCTM ((-30.0 / 360) * (2 * pi))
c.translateCTM (-128, -128)
c.setRGBStrokeColor (0,0,0,1)
c.setRGBFillColor (1,1,1,1)
c.selectFont ("Helvetica", 36, kCGEncodingMacRoman)
c.setTextPosition (40, 118)
c.setTextDrawingMode (kCGTextFillStroke)
c.showText ("hello, world", 12)
c.restoreGState ()

# Write the bitmap to disk as a TIFF

c.writeToFile ("out.tiff", kCGImageFormatTIFF)

