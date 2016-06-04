#!/usr/bin/python

# Usage: picttopdf pict-data-file
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

import sys
import os
import string
from CoreGraphics import *

# we need two arguments, the zeroth argument is the name of the executable
# and the first is the data file to convert
if len(sys.argv) != 2:
	sys.stderr.write("Usage: picttopdf pict_data_file\n")
	sys.stderr.write("produces pict_data_file.pdf\n")
	sys.stderr.write("if the file suffix is .pict or .pic then it is replaced with .pdf\n")
	sys.exit()
	
inFile = sys.argv[1];

if os.path.isfile(inFile):
	qdPict = QDPictCreateWithFilename(inFile)
else:
	sys.stderr.write('ERROR: "'+inFile+'"' + " is not a file\n")
	sys.exit()
	
if not qdPict:
	sys.stderr.write('ERROR: "'+inFile+'"' + " is not a PICT file!\n")
	sys.exit()	

pictRect = qdPict.getBounds();

if pictRect.size.width <= 0 or pictRect.size.height <= 0 :
	sys.stderr.write("Width or height of PICT bounds is zero! Can't use this picture!\n")
	sys.exit()
	
result = os.path.splitext(inFile)
extension = string.upper(result[1])
if (extension == ".PICT") or (extension == ".PIC"):
	outFile = result[0] +".pdf"
else:
	outFile = inFile +".pdf"
	
ctx = CGPDFContextCreateWithFilename(outFile, pictRect);
if ctx:
	ctx.beginPage(pictRect);
	success = qdPict.drawToCGContext(ctx, pictRect)
	ctx.endPage();
	ctx.finish()
else:
	sys.stderr.write("Couldn't create PDF context for output file " + '"' + outFile + '"\n') 
	


    
    
  