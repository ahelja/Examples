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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// AudioFileReaderThread.cpp
//
#include "AudioFilePlayer.h"
#include <mach/mach.h> //used for setting policy of thread
#include "CAGuard.h"
#include <pthread.h>

#include <vector>

#if DEBUG
    #define	LOG_DATA_FLOW 0
#endif

class FileReaderThread {
public:
	FileReaderThread ();

	CAGuard&					GetGuard() { return mGuard; }
    
	void						AddReader();
	
	void						RemoveReader (const FileThreadVariables* inItem);
		
		// returns true if succeeded
	bool						TryNextRead (FileThreadVariables* inItem)
	{
		bool didLock = false;
		bool succeeded = false;
		if (mGuard.Try (didLock))
		{
			mFileData.push_back (inItem);
			mGuard.Notify();
			succeeded = true;
		}
		
		if (didLock)
			mGuard.Unlock();
				
			return succeeded;
	}	
	
private:
	typedef	std::vector<FileThreadVariables*> FileData;

	CAGuard				mGuard;
	UInt32				mThreadPriority;
	bool				mThreadShouldDie;
	int					mNumReaders;	
	FileData			mFileData;


	void 						ReadNextChunk ();
	
	void 						StartFixedPriorityThread ();
    static UInt32				GetThreadBasePriority (pthread_t inThread);
    
	static void*				DiskReaderEntry (void *inRefCon);
};

FileReaderThread::FileReaderThread ()
	  : mGuard ("AudioFileReaderThread"),
	    mThreadPriority (62),
		mNumReaders (0)
{
	mFileData.reserve (48);
}

void	FileReaderThread::AddReader()
{
	if (mNumReaders == 0)
	{
		mThreadShouldDie = false;
	
		StartFixedPriorityThread ();
	}
	mNumReaders++;
}

void	FileReaderThread::RemoveReader (const FileThreadVariables* inItem)
{
	if (mNumReaders > 0)
	{
		CAGuard::Locker fileReadLock (mGuard);

		for (FileData::iterator iter = mFileData.begin(); iter != mFileData.end(); ++iter)
		{
			if ((*iter) == inItem) {	
				mFileData.erase (iter);
			}
		}
		
		if (--mNumReaders == 0) {
			mThreadShouldDie = true;
			mGuard.Notify();
		}
	}	
}

void 	FileReaderThread::StartFixedPriorityThread ()
{
	pthread_attr_t		theThreadAttrs;
	pthread_t			pThread;
	
	OSStatus result = pthread_attr_init(&theThreadAttrs);
		THROW_RESULT("pthread_attr_init - Thread attributes could not be created.")
	
	result = pthread_attr_setdetachstate(&theThreadAttrs, PTHREAD_CREATE_DETACHED);
		THROW_RESULT("pthread_attr_setdetachstate - Thread attributes could not be detached.")
	
	result = pthread_create (&pThread, &theThreadAttrs, DiskReaderEntry, this);
		THROW_RESULT("pthread_create - Create and start the thread.")
	
	pthread_attr_destroy(&theThreadAttrs);
    
	// we've now created the thread and started it
	// we'll now set the priority of the thread to the nominated priority
	// and we'll also make the thread fixed
    thread_extended_policy_data_t		theFixedPolicy;
    thread_precedence_policy_data_t		thePrecedencePolicy;
    SInt32								relativePriority;
    
    // make thread fixed
    theFixedPolicy.timeshare = false;	// set to true for a non-fixed thread
    result = thread_policy_set (pthread_mach_thread_np(pThread), THREAD_EXTENDED_POLICY, (thread_policy_t)&theFixedPolicy, THREAD_EXTENDED_POLICY_COUNT);
        THROW_RESULT("thread_policy - Couldn't set thread as fixed priority.")
    // set priority
    // precedency policy's "importance" value is relative to spawning thread's priority
    relativePriority = mThreadPriority - FileReaderThread::GetThreadBasePriority (pthread_self());
        
    thePrecedencePolicy.importance = relativePriority;
    result = thread_policy_set (pthread_mach_thread_np(pThread), THREAD_PRECEDENCE_POLICY, (thread_policy_t)&thePrecedencePolicy, THREAD_PRECEDENCE_POLICY_COUNT);
        THROW_RESULT("thread_policy - Couldn't set thread priority.")
}

UInt32	FileReaderThread::GetThreadBasePriority (pthread_t inThread)
{
    thread_basic_info_data_t			threadInfo;
	policy_info_data_t					thePolicyInfo;
	unsigned int						count;
    
    // get basic info
    count = THREAD_BASIC_INFO_COUNT;
    thread_info (pthread_mach_thread_np (inThread), THREAD_BASIC_INFO, (integer_t*)&threadInfo, &count);
    
	switch (threadInfo.policy) {
		case POLICY_TIMESHARE:
			count = POLICY_TIMESHARE_INFO_COUNT;
			thread_info(pthread_mach_thread_np (inThread), THREAD_SCHED_TIMESHARE_INFO, (integer_t*)&(thePolicyInfo.ts), &count);
			return thePolicyInfo.ts.base_priority;
            break;
            
        case POLICY_FIFO:
			count = POLICY_FIFO_INFO_COUNT;
			thread_info(pthread_mach_thread_np (inThread), THREAD_SCHED_FIFO_INFO, (integer_t*)&(thePolicyInfo.fifo), &count);
            if (thePolicyInfo.fifo.depressed) {
                return thePolicyInfo.fifo.depress_priority;
            } else {
                return thePolicyInfo.fifo.base_priority;
            }
            break;
            
		case POLICY_RR:
			count = POLICY_RR_INFO_COUNT;
			thread_info(pthread_mach_thread_np (inThread), THREAD_SCHED_RR_INFO, (integer_t*)&(thePolicyInfo.rr), &count);
			if (thePolicyInfo.rr.depressed) {
                return thePolicyInfo.rr.depress_priority;
            } else {
                return thePolicyInfo.rr.base_priority;
            }
            break;
	}
    
    return 0;
}

void	*FileReaderThread::DiskReaderEntry (void *inRefCon)
{
	FileReaderThread *This = (FileReaderThread *)inRefCon;
	This->ReadNextChunk();
	#if DEBUG
	printf ("finished with reading file\n");
	#endif
	
	return 0;
}

void 	FileReaderThread::ReadNextChunk ()
{
	OSStatus 						result;
	UInt32							dataChunkSize;
	UInt32							dataChunkSizeInPackets;
    AudioStreamPacketDescription	*packetDescriptions = NULL;
	FileThreadVariables* 			theItem = 0;

	for (;;) 
	{
		{ // this is a scoped based lock
			CAGuard::Locker fileReadLock (mGuard);
			
			if (mThreadShouldDie) return;
			
			while (mFileData.empty() && !mThreadShouldDie) {
				fileReadLock.Wait();
			}
			
			// kill thread
			if (mThreadShouldDie) return;

			theItem = mFileData[0];
			mFileData.erase (mFileData.begin());
		}
	

		packetDescriptions = theItem->mPacketDescriptions;
        if (!theItem->mWriteToFirstBuffer)
            packetDescriptions += theItem->mChunkSizeInPackets;
        
        if ((theItem->mPacketCount - theItem->mReadPacketPosition) < theItem->mChunkSizeInPackets)
		{
        	dataChunkSizeInPackets = theItem->mPacketCount - theItem->mReadPacketPosition;
            if (!theItem->IsLooping()) {
                theItem->mFinishedReadingData = true;
            }
		}
        else
			dataChunkSizeInPackets = theItem->mChunkSizeInPackets;

        // this is the exit condition for the thread
		if (dataChunkSizeInPackets == 0 && !theItem->IsLooping()) {
			theItem->mFinishedReadingData = true;
			continue;
		}

        // construct pointer
        char* writePtr = const_cast<char*>(theItem->GetFileBuffer() + 
                            (theItem->mWriteToFirstBuffer ? 0 : theItem->mChunkSizeInPackets * theItem->mMaxPacketSize));
	
#if LOG_DATA_FLOW
		fprintf(stdout, "***** ReadNextChunk(1) - AFReadPackets (pkts/offset) = %ld/%qd\n", dataChunkSizeInPackets, theItem->mReadPacketPosition);
#endif
        result = AudioFileReadPackets (theItem->GetFileID(), 
                                        false,
                                        &dataChunkSize,
                                        packetDescriptions,
                                        theItem->mReadPacketPosition, 
                                        &dataChunkSizeInPackets, 
                                        writePtr);
		if (result) {
			theItem->GetParent().DoNotification(result);
			continue;
		}

		theItem->mCurrentPacketCountInBuffer = dataChunkSizeInPackets;
		theItem->mCurrentByteCountInBuffer = dataChunkSize;

		if (dataChunkSizeInPackets != theItem->mChunkSizeInPackets)
		{
			writePtr += dataChunkSize;
            packetDescriptions += dataChunkSizeInPackets;

			if (theItem->IsLooping())
			{

                packetDescriptions = theItem->mPacketDescriptions + dataChunkSizeInPackets;
				dataChunkSizeInPackets = theItem->mChunkSizeInPackets - dataChunkSizeInPackets;
				theItem->mReadPacketPosition = 0;
            
#if LOG_DATA_FLOW
                fprintf(stdout, "***** ReadNextChunk(2) - AFReadPackets (pkts/offset) = %ld/%qd\n", dataChunkSizeInPackets, theItem->mReadPacketPosition);
#endif
                result = AudioFileReadPackets (theItem->GetFileID(), 
                                                false,
                                                &dataChunkSize,
                                                packetDescriptions,
                                                theItem->mReadPacketPosition, 
                                                &dataChunkSizeInPackets, 
                                                writePtr);
                if (result) {
                    theItem->GetParent().DoNotification(result);
                    continue;
                }
                theItem->mCurrentPacketCountInBuffer += dataChunkSizeInPackets;
                theItem->mCurrentByteCountInBuffer += dataChunkSize;
			} 
            else 
            {
                // can't exit yet.. we still have to pass the partial buffer back
                memset (writePtr, 0, ((theItem->mChunkSizeInPackets - dataChunkSizeInPackets) * theItem->mMaxPacketSize));
			}
		}
		
        theItem->mWriteToFirstBuffer = !theItem->mWriteToFirstBuffer;	// switch buffers
		
		theItem->mReadPacketPosition += dataChunkSizeInPackets;		// increment count
	}
}


static FileReaderThread sReaderThread;

AudioFileReaderThread::AudioFileReaderThread (AudioFilePlayer	&inParent, 
										AudioFileID 			&inFile, 
										SInt64 					inFileLength,
                                        SInt64					inPacketCount,
                                        UInt32					inMaxPacketSize,
                                        UInt32					inChunkSizeInPackets)
	: FileThreadVariables (inChunkSizeInPackets, inFileLength, inPacketCount, inMaxPacketSize, inParent, inFile),
	  mLockUnsuccessful (false),
	  mIsEngaged (false)
{
	mFileBuffer = (char*) malloc (mChunkSizeInPackets * mMaxPacketSize * 2);
    mPacketDescriptions = (AudioStreamPacketDescription *) calloc (1, (mChunkSizeInPackets * sizeof(AudioStreamPacketDescription)) * 2);
}

void	AudioFileReaderThread::DoConnect ()
{
	if (!mIsEngaged)
	{
		mFinishedReadingData = false;

		mNumTimesAskedSinceFinished = -1;
		mLockUnsuccessful = false;
		
		UInt32 dataChunkSize = 0;

		mCurrentPacketCountInBuffer = mChunkSizeInPackets;
#if LOG_DATA_FLOW
		fprintf(stdout, "***** DoConnect - AFReadPackets from  (pkts/offset) %ld/%qd\n", mCurrentPacketCountInBuffer, mReadPacketPosition);
#endif
		OSStatus result = AudioFileReadPackets ( mAudioFileID, 
												false, 
												&dataChunkSize,
                                                mPacketDescriptions,
                                                mReadPacketPosition, 
												&mCurrentPacketCountInBuffer, 
												mFileBuffer);
			THROW_RESULT("AudioFileReadPackets")
        
        mCurrentByteCountInBuffer = dataChunkSize;
		mReadPacketPosition = mCurrentPacketCountInBuffer;

		mWriteToFirstBuffer = false;
		mReadFromFirstBuffer = true;

		sReaderThread.AddReader();
		
		mIsEngaged = true;
	}
	else
		throw static_cast<OSStatus>(-1); //thread has already been started
}

void	AudioFileReaderThread::Disconnect ()
{
	if (mIsEngaged) 
	{
		sReaderThread.RemoveReader (this);
		mIsEngaged = false;
	}
}

OSStatus AudioFileReaderThread::GetFileData (void** inOutData, UInt32 *inOutDataSize, UInt32 *outPacketCount, AudioStreamPacketDescription	**outPacketDescriptions)
{
	if (mFinishedReadingData) 
	{
		++mNumTimesAskedSinceFinished;
		*inOutDataSize = 0;
		*inOutData = 0;
		return noErr;
	}
	
	if (mReadFromFirstBuffer == mWriteToFirstBuffer) {
		#if DEBUG
		printf ("* * * * * * * Can't keep up with reading file:%ld\n", mParent.GetBusNumber());
		#endif
		
		mParent.DoNotification (kAudioFilePlayErr_FilePlayUnderrun);
		*inOutDataSize = 0;
		*inOutData = 0;
	} else {
		*inOutDataSize = mCurrentByteCountInBuffer;
		*inOutData = mReadFromFirstBuffer ? mFileBuffer : (mFileBuffer + (mChunkSizeInPackets * mMaxPacketSize));
        *outPacketCount = mCurrentPacketCountInBuffer;
		*outPacketDescriptions = mReadFromFirstBuffer ? mPacketDescriptions : (mPacketDescriptions + mChunkSizeInPackets);
	}

	mLockUnsuccessful = !sReaderThread.TryNextRead (this);
	
	mReadFromFirstBuffer = !mReadFromFirstBuffer;

	return noErr;
}

void 	AudioFileReaderThread::AfterRender ()
{
	if (mNumTimesAskedSinceFinished > 0)
	{
		bool didLock = false;
		if (sReaderThread.GetGuard().Try (didLock)) {
			mParent.DoNotification (kAudioFilePlay_FileIsFinished);
			if (didLock)
				sReaderThread.GetGuard().Unlock();
		}
	}

	if (mLockUnsuccessful)
		mLockUnsuccessful = !sReaderThread.TryNextRead (this);
}
