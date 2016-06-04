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
/* DataCollectionManager.m */

typedef void *	(*ThreadRoutine)(void * inParameter);

#import <Foundation/Foundation.h>

#include <unistd.h>

#import "DataCollectionManager.h"
#import "AudioCycleManager.h"
#import "pThreadUtilities.h"
#import "MillionMonkeys.h"

void	dataCollectionThreadRunnable (DataCollectionManager* inDCM);


DataCollectionManager* DataCollectionManagerNew ()
{
	DataCollectionManager* retVal;
	
	retVal = (DataCollectionManager *)malloc (sizeof (DataCollectionManager));
	retVal->_DataBinQueue = NULL;
	retVal->_displayDataBin = NULL;
	retVal->_currentDataBin = NULL;
	retVal->_binGranularity = 0;
	retVal->_dataCollectionsPerDisplay = 0;
	retVal->_dataNeedsDisplayUpdate = FALSE;
	
	retVal->_dataCollectionThreadIsRunning = FALSE;
	retVal->_dataCollectionThreadShouldPause = FALSE;
	
	retVal->_AppInstance = NULL;
	
	return retVal;
}

void DataCollectionManagerDestroy (DataCollectionManager* inDCM)
{
	free (inDCM);
}

void DataCollectionManagerInitialize (DataCollectionManager* inDCM)
{
	// data collection thread
	pthread_attr_t		theThreadAttributes;
	
	notifyOnError (pthread_attr_init(&theThreadAttributes), "Error getting thread attributes.");
	notifyOnError (pthread_attr_setdetachstate(&theThreadAttributes, PTHREAD_CREATE_DETACHED), "Error setting thread detachable.");
	notifyOnError (pthread_cond_init(&inDCM->_dataCollectionThreadConditionAttributes, NULL), "Error creating condition variable for data collection thread.");
	notifyOnError (pthread_create (&inDCM->_dataCollectionThread, &theThreadAttributes, (ThreadRoutine)*dataCollectionThreadRunnable, inDCM), "Error creating data collection thread.");
	setThreadToPriority (inDCM->_dataCollectionThread, 31, FALSE, 0);
	// cleanup
	pthread_attr_destroy(&theThreadAttributes);
}

DataBin*	DataCollectionManagerGetCurrentDataBin (DataCollectionManager* inDCM)
{
	return inDCM->_currentDataBin;
}

void		DataCollectionManagerSetCurrentDataBin (DataCollectionManager* inDCM, DataBin *inDataBin)
{
	inDCM->_currentDataBin = inDataBin;
}

UInt32 DataCollectionManagerGetBinGranularity (DataCollectionManager* inDCM)
{
	return inDCM->_binGranularity;
}

DataBinCollection* DataCollectionManagerGetDataBinQueue (DataCollectionManager* inDCM)
{
	return inDCM->_DataBinQueue;
}

//   [D] Data Collection Thread Runnable
void dataCollectionThreadRunnable (DataCollectionManager* inDCM)
{
	DataBin	*				binBeingProcessed;
	AudioCycleManager *		theACM;
	int						startPoint, endPoint;
	int						delta;
	
	theACM = [inDCM->_AppInstance getACM];
	
	do {
		inDCM->_dataCollectionThreadIsRunning = TRUE;
		if ( theACM->_testIsRunning || theACM->_testJustFinished ) {
			theACM->_testJustFinished = FALSE;
			if (inDCM->_binGranularity > 1) {
				// aggregate data & place in displayBin
				binBeingProcessed = DataBinCollectionGetNextReadBin (inDCM->_DataBinQueue);
				while (binBeingProcessed != NULL) {
					[inDCM->_AppInstance aggregateNextDataBin:binBeingProcessed];
					
					DataBinReset (binBeingProcessed);
					binBeingProcessed = DataBinCollectionGetNextReadBin (inDCM->_DataBinQueue);
				}
				
				if (DataBinCollectionUnviewedBinCount(inDCM->_DataBinQueue) >= inDCM->_dataCollectionsPerDisplay)  {
					inDCM->_dataNeedsDisplayUpdate = TRUE;
					DataBinCollectionSetLastBinView (inDCM->_DataBinQueue);
				}
			} else {
				startPoint = [inDCM->_AppInstance getOwnedDCVIndex];
				endPoint = DataBinCurrentIndex (inDCM->_currentDataBin);
				delta = endPoint - startPoint;
				if ( (delta >= inDCM->_dataCollectionsPerDisplay) || (startPoint > endPoint) ) {
					if (startPoint > endPoint) {
						// case for wraparound
						[inDCM->_AppInstance _directDataWriteFromBin:inDCM->_displayDataBin startPoint:startPoint numPoints:(DataBinNumCycles (inDCM->_displayDataBin) - startPoint)];
						[inDCM->_AppInstance _directDataWriteFromBin:inDCM->_displayDataBin startPoint:0 numPoints:endPoint];
					} else {
						// normal display case
						[inDCM->_AppInstance _directDataWriteFromBin:inDCM->_displayDataBin startPoint:startPoint numPoints:delta];
					}
					inDCM->_dataNeedsDisplayUpdate = TRUE;
				}
			}
		}
		if (inDCM->_dataCollectionThreadShouldPause) dataCollectionThreadPause (inDCM);
		usleep (33000);	// runs at 30fps max
		if (inDCM->_dataCollectionThreadShouldPause) dataCollectionThreadPause (inDCM);
	} while (TRUE);
}

void dataCollectionThreadPause (DataCollectionManager* inDCM)
{
	inDCM->_dataCollectionThreadIsRunning = FALSE;
	while (inDCM->_dataCollectionThreadShouldPause == TRUE) usleep (1000);	// check for change every 1ms
}

/*
 */
