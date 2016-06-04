#!/usr/bin/env python

# doc2pdf.py
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
from fnmatch import fnmatch
import os, sys, getopt

def usage():
  print '''
usage: python doc2pdf.py [OPTION] DOCUMENT[S]

Convert any of several document types to PDF:
	Text: .txt .? .??
	RTF:  .rtf
	HTML: .htm .html .php
	Word: .doc .xml  .wordml

Options:
  -f, --font-size=SIZE
'''

def drawDocument(input_file, font_size):
  '''Convert input file into PDF, using font_size'''
  text = CGDataProviderCreateWithFilename(input_file)
  if not text:
    return "Error: document '%s' not found"%(input_file)
  (root, ext) = os.path.splitext(input_file)
  output_file = root + ".pdf"

  pageRect = CGRectMake(0, 0, 612, 792)
  c = CGPDFContextCreateWithFilename(output_file, pageRect)
  c.beginPage(pageRect)

  if fnmatch(ext,".txt") or fnmatch(ext,".?") or fnmatch(ext,".??"):
    tr = c.drawPlainTextInRect(text, pageRect, font_size)
  elif fnmatch(ext,".rtf"):
    tr = c.drawRTFTextInRect(text, pageRect, font_size)
  elif fnmatch(ext,".htm*") or fnmatch(ext,".php"):
    tr = c.drawHTMLTextInRect(text, pageRect, font_size)
  elif fnmatch(ext,".doc"):
    tr = c.drawDocFormatTextInRect(text, pageRect, font_size)
  elif fnmatch(ext,"*ml"):
    tr = c.drawWordMLFormatTextInRect(text, pageRect, font_size)
  else:
    return "Error: unknown type '%s' for '%s'"%(ext, input_file)

  c.endPage()
  c.finish()
  return output_file


def main():
  '''Parse font_size option, then convert each argument'''
  try:
    opts,args = getopt.getopt(sys.argv[1:], 'f:', ['font-size='])
  except getopt.GetoptError:
    usage()
    sys.exit(1)

  font_size = 12.0
  for(o,a) in opts:
    if o in('-f', '--font-size'):
      font_size = float(a)

  if len(args) < 1:
    usage()
    exit()

  for arg in args:
    print arg, "->", drawDocument(arg, font_size)


# Run as main

if __name__ == '__main__':
  main()
