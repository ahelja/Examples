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
//	AudioFilePlayer.h
//
#ifndef __AudioFilePlayer_H__
#define __AudioFilePlayer_H__

#include <CoreServices/CoreServices.h>

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

#include "AudioFilePlay.h"

#define THROW_RESULT(str) 										\
	if (result) {												\
		printf ("Error:%s=0x%lX,%ld,%s\n\n",		 			\
						str,result, result, (char*)&result);	\
		throw result;											\
	}

typedef void (*AudioFileManagerErrorNotification)(void *				inRefCon,
											OSStatus					inErrorCode);

class AudioFileManager;

#pragma mark __________ AudioFilePlayer
class AudioFilePlayer
{
public:
	AudioFilePlayer (const FSRef	&inFileRef);
	
	~AudioFilePlayer();

	void			SetDestination (AudioUnit					&inDestUnit, 
								int			 					inBusNumber);
	
	void			SetNotifier (AudioFilePlayNotifier inNotifier, void *inRefCon)
	{
		mNotifier = inNotifier;
		mRefCon = inRefCon;
	}
	
	void			Connect();
	
	void 			Disconnect();

	void 			DoNotification (OSStatus inError) const;
	
	bool			IsConnected () const { return mConnected; }

	void			SetLooping (bool inLoop);
		
	bool			IsLooping () const;
	
	UInt32			GetBusNumber () const { return mBusNumber; }
	
	AudioUnit		GetDestUnit () const { return mPlayUnit; }
	
	AudioConverterRef	GetAudioConverter() const { return mConverter; }
	
	void			Print() const 
	{
		CAShow (mAudioFileID);
		printf ("Destination Bus:%ld\n", GetBusNumber());
		printf ("Is Connected:%s\n", (IsConnected() ? "true" : "false"));
		printf ("Using Reader Thread:%s\n", (mUsingReaderThread ? "true" : "false"));
		if (mConverter) CAShow (mConverter);
		printf ("- - - - - - - - - - - - - - \n");
	}
	
	const AudioStreamBasicDescription& 		GetFileFormat() const { return mFileDescription; }
	
private:
	AudioUnit		 				mPlayUnit;
	UInt32							mBusNumber;
	AudioFileID						mAudioFileID;
	
	AudioUnitInputCallback 			mInputCallback;
	AURenderCallbackStruct			mRenderCallback;

	AudioStreamBasicDescription 	mFileDescription;
	
	bool							mConnected;
	bool							mUsingReaderThread;
	
	AudioFileManager*				mAudioFileManager;
	AudioConverterRef				mConverter;
	
	AudioFilePlayNotifier 			mNotifier;
	void*							mRefCon;
    	
#pragma mark __________ Private_Methods
	
	void		OpenFile (const FSRef& inRef, SInt64& outFileSize, SInt64& outPacketCount, UInt32& outMaxPacketSize);
};

#pragma mark __________ AudioFileManager
class AudioFileManager
{
public:
	AudioFileManager (AudioFilePlayer& inParent, AudioFileID inFile)
		: mParent (inParent),
		  mAudioFileID (inFile),
		  mFileBuffer (0),
		  mIsLooping (false),
          mCurrentByteCountInBuffer(0),
          mCurrentPacketCountInBuffer(0),
          mPacketDescriptions(NULL)
		{}

	
	virtual ~AudioFileManager();
	
	
	void				Connect (AudioConverterRef inConverter) 
	{
		mParentConverter = inConverter;
		DoConnect();
	}

		// this method should NOT be called by an object of this class
		// as it is called by the parent's Disconnect() method
	virtual void		Disconnect () {}

	void				SetLooping (bool inLooping) { mIsLooping = inLooping; }
			
	bool				IsLooping () const { return mIsLooping; }

	const AudioFileID&	GetFileID() const { return mAudioFileID; }

	const char*			GetFileBuffer () { return mFileBuffer; }
	
	const AudioFilePlayer& 	GetParent () const { return mParent; }
	
protected:
	AudioFilePlayer& 				mParent;
	AudioConverterRef				mParentConverter;
	const AudioFileID				mAudioFileID;
	char*							mFileBuffer;
	bool							mIsLooping;
	
	OSStatus			Render (AudioBuffer &ioData);

	OSStatus			Render (AudioBufferList &ioData, UInt32 inNumFrames);
	
	virtual OSStatus 	GetFileData (void** inOutData, UInt32 *inOutDataSize, UInt32 *outPacketCount, 															AudioStreamPacketDescription	**outPacketDescriptions) = 0;
	
	virtual void		DoConnect () = 0;
		
	virtual void		AfterRender () = 0;

public:

    UInt32							mCurrentByteCountInBuffer;
    UInt32							mCurrentPacketCountInBuffer;
    AudioStreamPacketDescription*	mPacketDescriptions;

	static OSStatus 	FileRenderProc (void 							*inRefCon, 
										AudioUnitRenderActionFlags 		*inActionFlags,
										const AudioTimeStamp 			*inTimeStamp, 
										UInt32 							inBusNumber,
										UInt32							inNumFrames, 
										AudioBufferList 				*ioData);

	static OSStatus 	ACComplexInputProc (AudioConverterRef		inAudioConverter,
								UInt32*								ioNumberDataPackets,
								AudioBufferList*					ioData,
								AudioStreamPacketDescription** 		outDataPacketDescription,
								void*								inUserData);
};

#pragma mark __________ AudioFileData
class AudioFileData : public AudioFileManager
{
public:
	AudioFileData (	AudioFilePlayer		&inParent, 
                    AudioFileID 		&inFile, 
                    SInt64 				inFileLength,
                    SInt64				inPacketCount,
                    UInt32				inMaxPacketSize,
                    UInt32				inChunkSizeInPackets);

protected:
	virtual void		DoConnect ();

	virtual OSStatus 	GetFileData (void** inOutData, UInt32 *inOutDataSize, UInt32 *outPacketCount, 															AudioStreamPacketDescription	**outPacketDescriptions);

	virtual void		AfterRender ();

private:
	bool						mFirstTime;
	bool						mShouldFinish;
	UInt32						mFileLength;
};

struct FileThreadVariables : public AudioFileManager 
{
	const UInt32					mChunkSizeInPackets;
	const SInt64					mFileLength;
    const SInt64					mPacketCount;
    const UInt32					mMaxPacketSize;
    SInt64							mReadPacketPosition;
	bool							mWriteToFirstBuffer;
	bool							mFinishedReadingData;

	FileThreadVariables (	const UInt32 					inChunkSizeInPackets,
                            const SInt64 					inFileLength,
                            const SInt64					inPacketCount,
                            UInt32							inMaxPacketSize,
                            AudioFilePlayer					&inParent, 
                            AudioFileID 					&inFile) 
		: AudioFileManager (inParent, inFile),
		  mChunkSizeInPackets (inChunkSizeInPackets),
		  mFileLength (inFileLength),
		  mPacketCount (inPacketCount),
		  mMaxPacketSize (inMaxPacketSize),
		  mReadPacketPosition (0),
          mWriteToFirstBuffer (false),
		  mFinishedReadingData (false)
		{}
	
	virtual ~FileThreadVariables() {}
};


#pragma mark __________ AudioFileReaderThread
class AudioFileReaderThread 
	: public FileThreadVariables
{
public:
	AudioFileReaderThread (AudioFilePlayer	&inParent, 
							AudioFileID 	&inFile, 
							SInt64 			inFileLength,
                            SInt64			inPacketCount,
                            UInt32			inMaxPacketSize,
                            UInt32			inChunkSizeInPackets);

	virtual void		Disconnect ();

protected:
	virtual void		DoConnect ();

	virtual OSStatus 	GetFileData (	void** inOutData, UInt32 *inOutDataSize, UInt32 *outPacketCount, 															AudioStreamPacketDescription	**outPacketDescriptions);
    
	virtual void		AfterRender ();

private:
	bool						mReadFromFirstBuffer;
	bool						mLockUnsuccessful;
	bool						mIsEngaged;
	
	int							mNumTimesAskedSinceFinished;
};


#endif
