/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*=============================================================================
	HLStripView.mm

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "HLStripView.h"

//	Standard Library Includes
#include <algorithm>

#include "CADebugMacros.h"

//=============================================================================
//	HLStripView
//=============================================================================

@implementation HLStripView

-(id)	initWithFrame:	(NSRect)inFrame
{
	mPopUpMatricesMade = false;
	mIsHorizontal = true;
	mNumberStrips = 1;
	return [super initWithFrame: inFrame];
}

-(void)	awakeFromNib
{
	//	get the frame
	NSRect theFrame = [self frame];
	
	//	figure out if strips are horizontally oriented
	mIsHorizontal = theFrame.size.width > theFrame.size.height;
	
	//	go through and convert the pop-up buttons to matrices of pop-ups
	if(!mPopUpMatricesMade)
	{
		[self MakePopUpMatrices];
	}
}

-(void)	MakePopUpMatrices
{
	//	need to go through all the subviews and replace each NSPopUpButton with an NSMatrix that
	//	uses the cell from the button as it's prototype
	if(!mPopUpMatricesMade)
	{
		mPopUpMatricesMade = true;
		
		//	iterate through the subviews, and set them to be the proper size
		NSArray* theSubviews = [self subviews];
		UInt32 theNumberSubviews = [theSubviews count];
		for(UInt32 theSubviewIndex = 0; theSubviewIndex < theNumberSubviews; ++theSubviewIndex)
		{
			//	get the view
			NSView* theView = [theSubviews objectAtIndex: theSubviewIndex];
			
			//	see if it is an NSPopUpButton
			id thePUBClass = [NSPopUpButton class];
			id theViewClass = [theView class];
			BOOL isClass = [theView isKindOfClass: [NSPopUpButton class]];
			if((isClass == YES) && (theViewClass == thePUBClass))
			{
				//	it is
				NSPopUpButton* thePopUpView = (NSPopUpButton*)theView;
				
				//	get the frame
				NSRect theFrame = [thePopUpView frame];
				
				//	get the tag
				UInt32 theTag = [thePopUpView tag];
				
				//	get the frame autosizing behavior
				UInt32 theAutoSizingMask = [thePopUpView autoresizingMask];
				
				//	make the pop-up cell prototye and configure it
				NSPopUpButtonCell* thePopUpCell = [[NSPopUpButtonCell alloc] initTextCell: @"Item" pullsDown: NO];
				[thePopUpCell setControlSize: NSSmallControlSize];
				[thePopUpCell setFont: [NSFont systemFontOfSize: [NSFont smallSystemFontSize]]];
				
				//	create a new NSMatrix
				NSMatrix* theMatrix = [[NSMatrix alloc] initWithFrame: theFrame mode: NSTrackModeMatrix prototype: thePopUpCell numberOfRows: 1 numberOfColumns: 1];
				
				//	release the cell now that we're done with it
				[thePopUpCell release];
				
				//	configure the matrix
				[theMatrix setTag: theTag];
				[theMatrix setAutosizesCells: YES];
				[theMatrix setAutoresizingMask: theAutoSizingMask];
				[theMatrix setCellSize: theFrame.size];
				NSSize theCellSpacing = { 0.0, 0.0 };
				[theMatrix setIntercellSpacing: theCellSpacing];
				
				//	swap the matrix for the button
				[self replaceSubview: thePopUpView with: theMatrix];
			}
		}
	}
}

-(void)	dealloc
{
	[super dealloc];
}

-(BOOL)	isFlipped
{
	return YES;
}

-(UInt32)	GetNumberStrips
{
	//	go through and convert the pop-up buttons to matrices of pop-ups
	if(!mPopUpMatricesMade)
	{
		[self MakePopUpMatrices];
	}

	return mNumberStrips;
}

-(void)		SetNumberStrips:	(UInt32)inNumberStrips
{
	//	go through and convert the pop-up buttons to matrices of pop-ups
	if(!mPopUpMatricesMade)
	{
		[self MakePopUpMatrices];
	}

	if(inNumberStrips != mNumberStrips)
	{
		[self setNeedsDisplay: YES];
		
		//	iterate through the subviews, and set them to be the proper size
		NSArray* theSubviews = [self subviews];
		UInt32 theNumberSubviews = [theSubviews count];
		Float32 theNewSize = 0;
		for(UInt32 theSubviewIndex = 0; theSubviewIndex < theNumberSubviews; ++theSubviewIndex)
		{
			//	get the subview (which is assumed to be an NSMatrix)
			NSMatrix* theMatrix = [theSubviews objectAtIndex: theSubviewIndex];
			
			//	tell it how many cells to have
			if(inNumberStrips > mNumberStrips)
			{
				if(mIsHorizontal)
				{
					UInt32 theNumberRowsToAdd = inNumberStrips - mNumberStrips;
					while(theNumberRowsToAdd > 0)
					{
						[theMatrix addRow];
						--theNumberRowsToAdd;
					}
				}
				else
				{
					UInt32 theNumberColumnsToAdd = inNumberStrips - mNumberStrips;
					while(theNumberColumnsToAdd > 0)
					{
						[theMatrix addColumn];
						--theNumberColumnsToAdd;
					}
				}
			}
			else
			{
				if(mIsHorizontal)
				{
					UInt32 theNumberRowsToRemove = mNumberStrips - inNumberStrips;
					while(theNumberRowsToRemove > 0)
					{
						UInt32 theNumberRows = [theMatrix numberOfRows];
						[theMatrix removeRow: theNumberRows - 1];
						--theNumberRowsToRemove;
					}
				}
				else
				{
					UInt32 theNumberColumnsToRemove = mNumberStrips - inNumberStrips;
					while(theNumberColumnsToRemove > 0)
					{
						UInt32 theNumberColumns = [theMatrix numberOfColumns];
						[theMatrix removeColumn: theNumberColumns - 1];
						--theNumberColumnsToRemove;
					}
				}
			}
			
			//	make it the right size
			[theMatrix sizeToCells];
		
			//	mark it for redraw
			[theMatrix setNeedsDisplay: YES];
			
			//	get it's frame
			NSRect theMatrixFrame = [theMatrix frame];
			
			//	keep track of the biggest the view is going to need to be
			if(mIsHorizontal)
			{
				theNewSize = std::max(theNewSize, theMatrixFrame.size.height);
			}
			else
			{
				theNewSize = std::max(theNewSize, theMatrixFrame.size.width);
			}
		}
		
		//	resize the view to hold the matrices
		NSRect theFrame = [self frame];
		if(mIsHorizontal)
		{
			theFrame.size.height = theNewSize;
		}
		else
		{
			theFrame.size.width = theNewSize;
		}
		[self setFrameSize: theFrame.size];
		
		[self setNeedsDisplay: YES];
		mNumberStrips = inNumberStrips;
	}
}

-(void)	SetControl:	(UInt32)inControl
		Target:		(id)inTarget
		Action:		(SEL)inAction
{
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the prototype cell from the matrix
		NSCell* thePrototype = [theMatrix prototype];
		if(thePrototype != NULL)
		{
			//	set it's target
			[thePrototype setTarget: inTarget];
			
			//	set it's action
			[thePrototype setAction: inAction];
		}
		
		//	get the dimensions of the matrix
		int theNumberRows = 0;
		int theNumberColumns = 0;
		[theMatrix getNumberOfRows: &theNumberRows columns: &theNumberColumns];
		
		//	iterate through all the cells
		for(int theRow = 0; theRow < theNumberRows; ++theRow)
		{
			for(int theColumn = 0; theColumn < theNumberColumns; ++theColumn)
			{
				NSCell* theCell = [theMatrix cellAtRow: theRow column: theColumn];
				if(theCell != NULL)
				{
					//	set it's target
					[theCell setTarget: inTarget];
					
					//	set it's action
					[theCell setAction: inAction];
				}
			}
		}
	}
}

-(UInt32)	GetSelectedStripIndex:	(UInt32)inControl
{
	UInt32 theAnswer = 0;
	
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the selected cell
		NSCell* theCell = [theMatrix selectedCell];
		
		//	get the row and column that the cell is in
		int theRow = 0;
		int theColumn = 0;
		[theMatrix getRow: &theRow column: &theColumn ofCell: theCell];
		
		//	set the return value
		if(mIsHorizontal)
		{
			theAnswer = theRow;
		}
		else
		{
			theAnswer = theColumn;
		}
	}
	
	return theAnswer;
}

-(void)		SetEnabled:				(UInt32)inControl
			ForChannel:				(UInt32)inChannel
			Value:					(BOOL)inIsEnabled
{
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		if(theCell != NULL)
		{
			//	get the return value
			[theCell setEnabled: inIsEnabled];
		}
	}
}

-(bool)		GetBoolValue:	(UInt32)inControl
			ForChannel:		(UInt32)inChannel
{
	bool theAnswer = false;
	
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		if(theCell != NULL)
		{
			//	get the return value
			theAnswer = [theCell intValue] != 0;
		}
	}
	
	return theAnswer;
}

-(void)		SetBoolValue:		(bool)inValue
			ForControl:			(UInt32)inControl
			ForChannel:			(UInt32)inChannel
{
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		if(theCell != NULL)
		{
			//	set the value
			[theCell setIntValue: (inValue ? 1 : 0)];
		}
	}
}

-(int)	GetIntValue:		(UInt32)inControl
		ForChannel:			(UInt32)inChannel
{
	int theAnswer = 0;
	
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		if(theCell != NULL)
		{
			//	get the return value
			theAnswer = [theCell intValue];
		}
	}
	
	return theAnswer;
}

-(void)		SetIntValue:		(int)inValue
			ForControl:			(UInt32)inControl
			ForChannel:			(UInt32)inChannel
{
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		if(theCell != NULL)
		{
			//	set the value
			[theCell setIntValue: inValue];
		}
	}
}

-(Float32)	GetFloatValue:		(UInt32)inControl
			ForChannel:			(UInt32)inChannel
{
	Float32 theAnswer = false;
	
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		if(theCell != NULL)
		{
			//	get the return value
			theAnswer = [theCell floatValue];
		}
	}
	
	return theAnswer;
}

-(void)		SetFloatValue:		(Float32)inValue
			ForControl:			(UInt32)inControl
			ForChannel:			(UInt32)inChannel
{
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		if(theCell != NULL)
		{
			//	set the value
			[theCell setFloatValue: inValue];
		}
	}
}

-(NSString*)	GetStringValue:		(UInt32)inControl
				ForChannel:			(UInt32)inChannel
{
	NSString* theAnswer = NULL;
	
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		if(theCell != NULL)
		{
			//	get the return value
			theAnswer = [theCell stringValue];
		}
	}
	
	return theAnswer;
}

-(void)		SetStringValue:		(NSString*)inValue
			ForControl:			(UInt32)inControl
			ForChannel:			(UInt32)inChannel
{
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		if(theCell != NULL)
		{
			//	set the value
			[theCell setStringValue: inValue];
		}
	}
}

-(UInt32)		GetSelectedMenuItemTag:	(UInt32)inControl
				ForChannel:				(UInt32)inChannel
{
	UInt32 theAnswer = 0;
	
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		//	make sure we have a pop-up cell
		if((theCell != NULL) && [theCell isKindOfClass: [NSPopUpButtonCell class]])
		{
			NSPopUpButtonCell* thePopUp = (NSPopUpButtonCell*)theCell;
			
			//	get the selected item
			NSMenuItem* theSelectedItem = [thePopUp selectedItem];
			if(theSelectedItem != NULL)
			{
				//	return the selected item's tag
				theAnswer = [theSelectedItem tag];
			}
		}
	}
	
	return theAnswer;
}

-(void)			SetSelectedMenuItemByTag:	(UInt32)inTag
				ForControl:					(UInt32)inControl
				ForChannel:					(UInt32)inChannel
{
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		//	make sure we have a pop-up cell
		if((theCell != NULL) && [theCell isKindOfClass: [NSPopUpButtonCell class]])
		{
			NSPopUpButtonCell* thePopUp = (NSPopUpButtonCell*)theCell;
			
			//	get the index of the item with the given tag
			UInt32 theIndex = [thePopUp indexOfItemWithTag: inTag];
			
			//	set that item as current
			[thePopUp selectItemAtIndex: theIndex];
		}
	}
}

-(void)			RemoveAllMenuItems:		(UInt32)inControl
				ForChannel:				(UInt32)inChannel
{
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		//	make sure we have a pop-up cell
		if((theCell != NULL) && [theCell isKindOfClass: [NSPopUpButtonCell class]])
		{
			NSPopUpButtonCell* thePopUp = (NSPopUpButtonCell*)theCell;
			
			//	remove all the items
			[thePopUp removeAllItems];
		}
	}
}

-(void)			AppendMenuItem:			(NSString*)inItemName
				Tag:					(UInt32)inItemTag
				ForControl:				(UInt32)inControl
				ForChannel:				(UInt32)inChannel
{
	//	figure out which matrix we're talking about
	NSMatrix* theMatrix = [self viewWithTag: inControl];
	if(theMatrix != NULL)
	{
		//	get the indicated cell
		NSCell* theCell = NULL;
		if(mIsHorizontal)
		{
			theCell = [theMatrix cellAtRow: inChannel column: 0];
		}
		else
		{
			theCell = [theMatrix cellAtRow: 0 column: inChannel];
		}
		
		//	make sure we have a pop-up cell
		if((theCell != NULL) && [theCell isKindOfClass: [NSPopUpButtonCell class]])
		{
			NSPopUpButtonCell* thePopUp = (NSPopUpButtonCell*)theCell;
			
			//	add the new item to the menu
			[thePopUp addItemWithTitle: inItemName];
			
			//	set the tag of the item
			[[thePopUp lastItem] setTag: inItemTag];
		}
	}
}

@end
