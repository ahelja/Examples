/*

File: UIController.m

Abstract: Main user interface control class

Author: James A. McCombe

© Copyright 2004 Apple Computer, Inc. All rights reserved.

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

#import "UIController.h"

@implementation UIController

- (id) init
{
	[super init];
	
	/* This is the list of supported exhibits */
	exhibit_dictionary = [NSDictionary dictionaryWithObjectsAndKeys: [[Mandelbrot alloc] init],  @"Mandelbrot",
																						  [[BrickShader alloc] init], @"BrickShader",
																						  [[WoodShader alloc] init],  @"WoodShader",
																						  [[Marble alloc] init],		@"Marble",
																						  nil];
	[exhibit_dictionary retain];
	
	return self;
}

- (void) dealloc
{
	[exhibit_dictionary release];
		
	[super dealloc];
}

- (void) awakeFromNib
{
	Exhibit *exhibit;
	
	exhibit = [[exhibit_dictionary allValues] objectAtIndex: 0];
	
	[openglview setExhibit: exhibit];
	[text_view  readRTFDFromFile: [exhibit descriptionFilename]];
}

/* Following methods are for compliency with the NSTableDataSource protocol */
- (int) numberOfRowsInTableView: (NSTableView *) aTableView
{
	return [exhibit_dictionary count];
}

- (id) tableView: (NSTableView *) aTableView
       objectValueForTableColumn: (NSTableColumn *) aTableColumn                                                                 
       row: (int) rowIndex
{
	return [[exhibit_dictionary allKeys] objectAtIndex: rowIndex];
}

- (void) tableViewSelectionDidChange: (NSNotification *) aNotification
{
	unsigned int rowindex;
	NSString *exhibit_name;
	Exhibit  *exhibit;
	
   /* Get the symbol that was selected */
   rowindex = [table_view selectedRow];
   if ((rowindex < 0) || (rowindex >= [exhibit_dictionary count]))
      return;
	exhibit_name = [[exhibit_dictionary allKeys] objectAtIndex: rowindex];
	
	exhibit      = [exhibit_dictionary objectForKey: exhibit_name];
	[openglview setExhibit: exhibit];
		
	if ([exhibit descriptionFilename])
		[text_view readRTFDFromFile: [exhibit descriptionFilename]];
	else
		[text_view setString: @""];
}

@end
