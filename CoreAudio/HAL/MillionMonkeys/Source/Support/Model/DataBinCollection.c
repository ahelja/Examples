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
/* DataBinCollection.c */

#include "DataBinCollection.h"
#include <stdlib.h>

//#define DEBUGPRINT

DataBinCollection*		DataBinCollectionNew (UInt32 inNumBins, UInt32 inNumCyclesPerBin, bool inBinsRequireTrace)
{
	UInt32		i;
	DataBinCollection*	DBCNew;
	
	DBCNew = (DataBinCollection*) malloc (sizeof (DataBinCollection));
	
	DBCNew->mWriteHead = -1;
	DBCNew->mHeadDisplayStamp = DBCNew->mReadHead = 0;
	
	DBCNew->mNumBins = inNumBins;
    
	DBCNew->mDataBins = (DataBin **) malloc (inNumBins * sizeof(DataBin *));
	for (i = 0; i < inNumBins; i++) {
		DBCNew->mDataBins[i] = DataBinNew (inNumCyclesPerBin, inBinsRequireTrace);
	}
	
	return DBCNew;
}

void		DataBinCollectionDealloc (DataBinCollection* DBCToDealloc)
{
	UInt32		i;
	
	for (i = 0; i < DBCToDealloc->mNumBins; i++) {
		DataBinDealloc (DBCToDealloc->mDataBins[i]);
	}
	
	free (DBCToDealloc);
}

DataBin*	DataBinCollectionGetNextWriteBin (DataBinCollection* inDBC)
{
	UInt32	indexOfBufferToReturn;
	
	if ((inDBC->mWriteHead - inDBC->mReadHead) >= (SInt32)inDBC->mNumBins) {
		// buffer overrun
		return NULL;
	}
	
	inDBC->mWriteHead = (inDBC->mWriteHead) + 1;
	indexOfBufferToReturn = inDBC->mWriteHead % inDBC->mNumBins;
	
#ifdef DEBUGPRINT
printf ("Write-head index:%d\n", inDBC->mWriteHead);
#endif
	return inDBC->mDataBins[indexOfBufferToReturn];
}

DataBin*	DataBinCollectionGetNextReadBin (DataBinCollection* inDBC)
{
	UInt32 indexOfBufferToReturn;
	
	if (inDBC->mReadHead >= inDBC->mWriteHead) {
		// buffer underrun
		return NULL;
	}
	
	indexOfBufferToReturn = inDBC->mReadHead % inDBC->mNumBins;
	inDBC->mReadHead = (inDBC->mReadHead) + 1; // increment head AFTER we've branched our return value (for read case only)
#ifdef DEBUGPRINT
printf ("Read-head index:%d, buffer idx. to return:%d\n", inDBC->mReadHead, indexOfBufferToReturn);
#endif
	return inDBC->mDataBins[indexOfBufferToReturn];
}

UInt32		DataBinCollectionUnviewedBinCount (DataBinCollection* inDBC)
{
	return inDBC->mWriteHead - inDBC->mHeadDisplayStamp;
}

void		DataBinCollectionSetLastBinView (DataBinCollection* inDBC)
{
	inDBC->mHeadDisplayStamp = inDBC->mReadHead;
}

void		DataBinCollectionReset (DataBinCollection* inDBC)
{
	inDBC->mReadHead = -1;
	inDBC->mWriteHead = -1;
}

/*
 */
