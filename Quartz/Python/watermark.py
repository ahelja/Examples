#!/usr/bin/python

# watermark.py -- add a "watermark" to each page of a pdf document
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
import sys, os, math, getopt, string

def usage ():
  print '''
usage: python watermark.py [OPTION]... INPUT-PDF OUTPUT-PDF

Add a "watermark" to a PDF document.

  -t, --text=STRING
  -f, --font-name=FONTNAME
  -F, --font-size=SIZE
  -c, --color=R,G,B
'''

def main ():

  text = 'CONFIDENTIAL'
  color = (1, 0, 0)
  font_name = 'Gill Sans Bold'
  font_size = 36
  page_rect = CGRectMake (0, 0, 612, 792)

  try:
    opts,args = getopt.getopt (sys.argv[1:], 't:f:F:c:',
			       ['text=', 'font-name=', 'font-size=', 'color='])
  except getopt.GetoptError:
    usage ()
    sys.exit (1)

  if len (args) != 2:
    usage ()
    sys.exit (1)

  for o,a in opts:
    if o in ('-t', '--text'):
      text = a
    elif o in ('-f', '--font-name'):
      font_name = a
    elif o in ('-F', '--font-size'):
      font_size = float (a)
    elif o in ('-c', '--color'):
      color = map (float, string.split (a, ','))

  c = CGPDFContextCreateWithFilename (args[1], page_rect)
  pdf = CGPDFDocumentCreateWithProvider (CGDataProviderCreateWithFilename (args[0]))

  for p in range (1, pdf.getNumberOfPages () + 1):
    r = pdf.getMediaBox (p)
    c.beginPage (r)
    c.saveGState ()
    c.drawPDFDocument (r, pdf, p)
    c.restoreGState ()
    c.saveGState ()
    c.setRGBFillColor (color[0], color[1], color[2], 1)
    c.setTextDrawingMode (kCGTextFill)
    c.setTextMatrix (CGAffineTransformIdentity)
    c.selectFont (font_name, font_size, kCGEncodingMacRoman)
    c.translateCTM (r.size.width - font_size, r.size.height - font_size)
    c.rotateCTM (-90.0 / 180 * math.pi)
    c.showTextAtPoint (0, 0, text, len (text))
    c.restoreGState ()
    c.endPage ()
  c.finish ()


if __name__ == '__main__':
  main ()
