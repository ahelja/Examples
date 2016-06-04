/*
	File:		GXFile.h

	Contains:	GX print file support for simple text application

	Version:	SimpleText 1.4 or later

	Written by:	Tom Dowdy

	Copyright:	© 1993, 1995 by Apple Computer, Inc., all rights reserved.

	File Ownership:

		DRI:				Tom Dowdy

		Other Contact:		Jim Negrette

		Technology:			Macintosh Graphics Group

	Writers:

		(ted)	Tom Dowdy
		(TD)	Tom Dowdy

	Change History (most recent first):

	$Log: GXFile.h,v $
	Revision 1.1  2005/02/16 03:33:19  mig
	[3943574] add SimpleText project from ~cvs/repository/pure and don't install binary anymore
	
	Revision 1.1.1.1  1998/03/18 22:56:10  ivory
	Initial checkin of SimpleText.
	
		
		1     7/28/97 11:18 AM Duane Byram
		first added to Source Safe project

		 <5>	 10/5/95	ted		fixing hilight for rotated shapes
		 <4>	 10/2/95	TD		adding in other selections for editing
		 <3>	 9/11/95	TD		adding more markup graphics
		 <2>	  9/8/95	TD		started annotation
		 <1>	 8/21/95	TD		First checked in.

*/

#include "SimpleText.h"

#define kGotoPageDialogID	 	kGXBaseID

#define kLabelString			kGXBaseID
#define kPageControlStrings		kGXBaseID+1
	#define iGoToPageString			1
	
#define kPageControlPlain		kGXBaseID
#define kPageControlRight		kPageControlPlain+1
#define kPageControlLeft		kPageControlPlain+2

#define kZoomControlPlain		kGXBaseID+3
#define kZoomControlRight		kZoomControlPlain+1
#define kZoomControlLeft		kZoomControlPlain+2

#define kGXPopUpMenu			kGXBaseID
	#define i50						1
	#define i100					2
	#define i112					3
	#define i150					4
	#define i200					5
	#define i400					6
	#define iScaleToFit				8
	#define iDontShowMargins		10
	
#define kGXToolMenu				kGXBaseID+1
	#define kIconBase				256
	#define kSelectionTool			1
	#define kRedMarkerTool			2

#ifndef REZ
	
	struct GXDataRecord
		{
		WindowDataRecord		w;
		
		gxViewPort				parentViewPort;		// viewPort in the window
		gxViewPort				childViewPort;		// viewPort inset from the other
		
		gxPrintFile				thePrintFile;
		short					printFileRefNum;
		
		long					numberOfPages;		// # of pages in this document
		long					currentPage;		// currently visible page #
		Fixed					zoomFactor;			// current zoom factor
		Boolean					dontShowMargins;	// don't show page margins

		// mode when user clicks in content
		short					contentClickMode;

		// current selection information
		Rect					selectionRectangle;
		short					patternPhase;
		
		// filled in during drag to avoid multiple shape traversals
		gxShape					tempDragShape;
		
		// current page and format information
		gxShape					currentPageShape;
		gxFormat				currentPageFormat;

		// filled in during shape traverse to locate the desired starting "index"
		long					tempSearchIndex;
		
		// fields for currently selected item in the search
		long 					currentShapeIndex;
		long					currentShapeStart;
		long					currentShapeEnd;
		gxShape 				currentSelectionShape;
		gxMapping				currentSelectionMapping;
		
		// fields for annotation pictures
		gxShape					** pageAnnotations;
		};
	typedef struct GXDataRecord GXDataRecord, *GXDataPtr;	
#endif
