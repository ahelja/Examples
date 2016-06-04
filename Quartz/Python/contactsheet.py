#!/usr/bin/python

# contactsheet.py -- create contact sheet from a list of jpg files
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


import sys, os, getopt, string, pwd, math, time
from CoreGraphics import *

baseCTM = 0
totSize = 0



# Generic Graphic classes
#    - sourceRect defines drawing coordinate system for graphic
#   - xform transform between this graphic and it's drawing coordinate transform
#   - boundingRect: computed axis-aligned bbox result of sending sourceRect through transform

class Graphic:
    def __init__(self, sourceRect):
        self.sourceRect = sourceRect
        self.xform = CGAffineTransformIdentity
    def centerPoint(self):
        return CGPointMake(self.sourceRect.getMidX(),self.sourceRect.getMidY()).applyAffineTransform(self.xform)

    #  drawing transform is pre multipled with new transforms (opposite of CTM)
    def concat(self, xform):
        self.xform = self.xform.concat(xform)
    def scaleAboutCenter(self,sx,sy):
        currentCenter = self.centerPoint()
        self.translate(-currentCenter.x, -currentCenter.y)
        self.xform = self.xform.concat(CGAffineTransformMakeScale(sx,sy))
        self.translate(currentCenter.x, currentCenter.y)
    def rotateAboutCenter(self, angle):
        currentCenter = self.centerPoint()
        self.translate(-currentCenter.x, -currentCenter.y)
        self.xform = self.xform.concat(CGAffineTransformMakeRotation(angle*math.pi/180.0))
        self.translate(currentCenter.x, currentCenter.y)
    def translate(self, tx, ty):
        self.xform = self.xform.concat(CGAffineTransformMakeTranslation(tx,ty))
    def boundingRect(self):
        # compute bounding rect
        lowerLeft = CGPointMake(self.sourceRect.origin.x, self.sourceRect.origin.y)
        lowerLeft = lowerLeft.applyAffineTransform(self.xform)
        upperRight = CGPointMake(self.sourceRect.origin.x + self.sourceRect.size.width, 
                                    self.sourceRect.origin.y + self.sourceRect.size.height)
        upperRight = upperRight.applyAffineTransform(self.xform)
        return CGRectMake(lowerLeft.x, lowerLeft.y, upperRight.x - lowerLeft.x, 
                                    upperRight.y - lowerLeft.y).standardize()
    def render(self, c):
        c.saveGState()
        c.concatCTM(self.xform)
        self.draw(c)
        c.restoreGState()
    def centerInRect(self, rect):
        cx = rect.getMidX()
        cy = rect.getMidY()
        currentCenter = self.centerPoint()
        self.translate(cx - currentCenter.x, cy - currentCenter.y)
    def fitInRect(self, rect, preserveAspectRatio):
        self.centerInRect(rect)
        bbox = self.boundingRect()
        if(preserveAspectRatio):
            scale = rect.size.width/bbox.size.width;
            if (scale * bbox.size.height > rect.size.height): 
                scale = rect.size.height/bbox.size.height
            self.scaleAboutCenter(scale,scale)
        else:
            self.scaleAboutCenter(rect.size.width/bbox.size.width, rect.size.height/bbox.size.height)
            
        

class Rectangle(Graphic):
    def draw(self,c):
        c.addRect(self.sourceRect)
        
        ## return to "base space" for consistent linewidth
        c.saveGState()
        c.concatCTM(baseCTM.concat(c.getCTM().invert()))
        c.setLineWidth(1)
        c.strokePath()
        c.restoreGState()
        
class Image(Graphic):
    def __init__(self,imageRef):
        self.i = imageRef
        Graphic.__init__(self, CGRectMake(0,0,imageRef.getWidth(), imageRef.getHeight()))
    def draw(self,c):
        c.drawImage(self.sourceRect, self.i)

class TextString(Graphic):
    def __init__(self, rect, string, font, size, sizeToFit):
#        self.html = '<html><p style="font-size:%spx; font-family:%s;"> %s/p></html>\n' % (size, font, string)
        self.html = '<html><p <FONT="font-size:%spx; font-family:%s;"> %s/p></html>\n' % (size, font, string)
        self.rtf = '{\\rtf1\\mac\\ansicpg10000\\cocoartf102{\\fonttbl\\f0\\fnil\\fcharset77 %s;}\\f0\\fs%s %s}\n' % (font, size, string)
        if (sizeToFit) :
            rect = CGContextMeasureRTFTextInRect(CGDataProviderCreateWithString (self.rtf), rect)
        Graphic.__init__(self, rect)
    def draw(self,c):
#      tr = c.drawHTMLTextInRect (CGDataProviderCreateWithString (self.html), self.sourceRect)
      tr = c.drawRTFTextInRect (CGDataProviderCreateWithString (self.rtf), self.sourceRect)
        

# helper to create an image from a filename
def ImageFromFile(filename):
    # special case JPG files and render directly with CG -- this keeps created PDFs smaller
    if (os.path.splitext(filename)[1].upper() == '.JPG') :
        return Image(CGImageCreateWithJPEGDataProvider(CGDataProviderCreateWithFilename (filename), [0,1,0,1,0,1], 1, kCGRenderingIntentDefault))
    return Image(CGImageImport (CGDataProviderCreateWithFilename (filename)))
    
#
# Contact Sheet specific classes   
#

#
# ContactSheetCell object
#

class ContactSheetCell(Graphic):
    def __init__(self, jpgFile):
        global totSize
        str = os.path.basename(jpgFile)
        str +=  "\\\n" + time.strftime('%x', time.localtime(os.path.getmtime(jpgFile)))
        str += "\\\n" + ('%dK' % (os.path.getsize(jpgFile)/1024))
        totSize += os.path.getsize(jpgFile)
        self.label = TextString(CGRectMake(0,0,100,30), str , "LucidaGrande", 12, 0)
        
        self.image = ImageFromFile(jpgFile)
        self.image.fitInRect(CGRectMake(0,30,100,70), 1)
        self.border = Rectangle(CGRectMake(0,0,100,100))
        Graphic.__init__(self, CGRectMake(0,0,100,100))
    def draw(self,c):
        self.label.render(c)
        self.image.render(c)
        self.border.render(c)

class ContactSheetPage(Graphic):
    def __init__(self, imageFiles, rect, cellSize):
        # create the array of cells
        self.cells = [ ContactSheetCell(i) for i in imageFiles]
        # position each cell
        row,col = 0,0
        for cell in self.cells:
            x = rect.origin.x + col*cellSize
            if ( x + cellSize > rect.origin.x + rect.size.width) :
                col = 0
                x = rect.origin.x
                row += 1
            cell.fitInRect(CGRectMake(x, rect.origin.y + rect.size.height - (row + 1)*cellSize, cellSize, cellSize), 1)
            col += 1
        Graphic.__init__(self, rect)
    def draw(self,c):
        for cell in self.cells:
            cell.render(c)

def LayoutContactSheets(imageFiles, pageRect):
    rect = pageRect.inset(36,36)  # use .5 inch margins
    nCols = int(math.ceil(math.sqrt(len(imageFiles)*rect.size.width/rect.size.height)))
    step = rect.size.width/nCols
    while (step < 72):  # constrain tiles to be at least 1 inch
        nCols -= 1
        step = rect.size.width/nCols
    nRows = int(math.floor(rect.size.height/step))
    cellsPerPage = nRows*nCols
    base = 0
    pages = []
    while(base < len(imageFiles)):
        slice = imageFiles[base:base+cellsPerPage]
        pages.append(ContactSheetPage(slice, rect, step))
        base += cellsPerPage 
    return pages

        
    
        
                

def usage ():
  print '''
usage: python contactsheet.py [OPTION]... JPG-FILES...

Create a pdf contact sheet of JPGs.

  -o, --output=FILENAME
'''

def main ():
	global baseCTM
	try:
		opts,args = getopt.getopt (sys.argv[1:],
						'o:',
			      			 ['output='])
	except getopt.GetoptError:
		usage ()
		sys.exit (1)
	output_file = ""
	page_rect = CGRectMake (0, 0, 612, 792)
	for o,a in opts:
    		if o in ('-o', '--output'):
			output_file = a

	# need at least an output file
	if output_file == "":
		print "error: missing output filename"
		usage ()
		sys.exit (1)
	
	c = CGPDFContextCreateWithFilename (output_file, page_rect)
        print "%d image files" % len(args)
        pages = LayoutContactSheets(args, page_rect)
        for page in pages:
            c.beginPage (page_rect)
            baseCTM = c.getCTM()
            page.render(c)
            c.endPage ()
	# serialized the constructed PDF document to its file
	c.finish ()
        print "Wrote output: \'%s\', %d pages,  %dK bytes" % (output_file, len(pages), os.path.getsize(output_file)/1024)

		




if __name__ == '__main__':
  main ()

