/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
	HLAudioFile.cpp

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

#include "HLAudioFile.h"
#include "CADebugMacros.h"
#include "CAException.h"

//=============================================================================
//	HLAudioFile
//=============================================================================

HLAudioFile::HLAudioFile(const FSRef& inFSRef, UInt32 inATFileType, CFStringRef inNameExtension)
:
	HLFile(inFSRef),
	mAudioFileID(0),
	mATFileType(inATFileType),
	mNameExtension(inNameExtension)
{
	if(mNameExtension != NULL)
	{
		CFRetain(mNameExtension);
	}
}

HLAudioFile::~HLAudioFile()
{
	if(mNameExtension != NULL)
	{
		CFRelease(mNameExtension);
	}
}

void	HLAudioFile::SetType()
{
	HLFileSystemObject::SetTypeOnFile(mATFileType);
}

void	HLAudioFile::SetNameExtension()
{
	HLFileSystemObject::SetNameExtensionOnFile(mNameExtension);
}

void	HLAudioFile::GetFormat(AudioStreamBasicDescription& outFormat) const
{
	ThrowIf(!mPrepared, CAException(fnOpnErr), "HLAudioFile::GetFormat: file hasn't been prepared yet");

	outFormat = mFormat;
}

void	HLAudioFile::Open(bool inForReading, bool inForWriting)
{
	if(mOpenCount == 0)
	{
		//	only actully open the file the first time
		
		//	save off the permissions
		mOpenForReading = inForReading;
		mOpenForWriting = inForWriting;
		
		//	open the file
		SInt8 thePermissions = 0;
		if(mOpenForReading)
		{
			thePermissions += fsRdPerm;
		}
		if(mOpenForWriting)
		{
			thePermissions += fsWrPerm;
		}
		OSStatus theError = AudioFileOpen(&mFSRef, thePermissions, 0, &mAudioFileID);
		ThrowIfError(theError, CAException(theError), "HLAudioFile::Open: couldn't open the file");
	}
	else
	{
		//	file is already open, so it's an error if someone tries to add permissions
		ThrowIf((mOpenForReading && !mOpenForWriting) && inForWriting, CAException(fBsyErr), "HLAudioFile::Open: can't add write permissions");
		ThrowIf((!mOpenForReading && mOpenForWriting) && inForReading, CAException(fBsyErr), "HLAudioFile::Open: can't add read permissions");
	}

	//	increment the open count
	++mOpenCount;
}

void	HLAudioFile::Close()
{
	if(mOpenCount > 0)
	{
		//	only close the file if it is open
		
		//	decrement the open count
		--mOpenCount;
		
		if(mOpenCount == 0)
		{
			//	no one wants the file open, so really close it
			OSStatus theError = 0;
			if(mAudioFileID != 0)
			{
				theError = AudioFileClose(mAudioFileID);
				mAudioFileID = 0;
			}
			
			//	clear the permissions
			mOpenForReading = false;
			mOpenForWriting = false;
			
			//	check for errors
			ThrowIfError(theError, CAException(theError), "HLAudioFile::Close: couldn't close the fork");
		}
	}
}

void	HLAudioFile::Prepare()
{
	//	open the file
	AudioFileID theAudioFileID = 0;
	OSStatus theError = AudioFileOpen(&mFSRef, fsRdPerm, 0, &theAudioFileID);
	ThrowIfError(theError, CAException(theError), "HLAudioFile::Prepare: couldn't open the file");
	
	//	get the format
	UInt32 theSize = sizeof(AudioStreamBasicDescription);
	theError = AudioFileGetProperty(theAudioFileID, kAudioFilePropertyDataFormat, &theSize, &mFormat);
	
	//	close the file before we might throw an exception
	AudioFileClose(theAudioFileID);
	
	//	check the error
	ThrowIfError(theError, CAException(theError), "HLAudioFile::Prepare: couldn't get the format");
	
	HLFile::Prepare();
}

void	HLAudioFile::PrepareNew(const void* inData, UInt32 inDataSize)
{
	//	the data for an HLAudioFile is a stream descption
	ThrowIf(inDataSize != sizeof(AudioStreamBasicDescription), CAException(paramErr), "HLAudioFile::PrepareNew: the data is supposed to be an AudioStreamBasicDescription");

	const AudioStreamBasicDescription* theFormat = reinterpret_cast<const AudioStreamBasicDescription*>(inData);

	//	initialize the file
	AudioFileID theAudioFileID = 0;
	OSStatus theError = AudioFileInitialize(&mFSRef, mATFileType, theFormat, 0, &theAudioFileID);
	ThrowIfError(theError, CAException(theError), "HLAudioFile::Prepare: couldn't initialize the file");
	
	//	get the format
	UInt32 theSize = sizeof(AudioStreamBasicDescription);
	theError = AudioFileGetProperty(theAudioFileID, kAudioFilePropertyDataFormat, &theSize, &mFormat);

	//	close the file before we might throw an exception
	AudioFileClose(theAudioFileID);
	
	//	check the error
	ThrowIfError(theError, CAException(theError), "HLAudioFile::Prepare: couldn't get the format");
	
	HLFile::PrepareNew(inData, inDataSize);
}

void	HLAudioFile::Flush(bool inOptimizeLayout)
{
	ThrowIf(mAudioFileID == 0, CAException(fnOpnErr), "HLAudioFile::Flush: file isn't prepared");
	
	if(inOptimizeLayout)
	{
		AudioFileOptimize(mAudioFileID);
	}
}

SInt64	HLAudioFile::GetRawByteSize() const
{
	//	can't touch this file outside of the AudioFile API
	DebugMessage("HLAudioFile::GetRawByteSize: can't do that");
	throw CAException(paramErr);
	return 0;
}

void	HLAudioFile::SetRawByteSize(SInt64 /*inSize*/)
{
	//	can't touch this file outside of the AudioFile API
	DebugMessage("HLAudioFile::SetRawByteSize: can't do that");
	throw CAException(paramErr);
}

void	HLAudioFile::ReadRawBytes(SInt64 /*inOffset*/, UInt32& /*ioNumberBytes*/, void* /*outData*/, bool /*inCache*/)
{
	//	can't touch this file outside of the AudioFile API
	DebugMessage("HLAudioFile::ReadRawBytes: can't do that");
	throw CAException(paramErr);
}

void	HLAudioFile::WriteRawBytes(SInt64 /*inOffset*/, UInt32& /*ioNumberBytes*/, void* /*inData*/, bool /*inCache*/)
{
	//	can't touch this file outside of the AudioFile API
	DebugMessage("HLAudioFile::WriteRawBytes: can't do that");
	throw CAException(paramErr);
}

SInt64	HLAudioFile::GetAudioByteSize() const
{
	ThrowIf(mAudioFileID == 0, CAException(fnOpnErr), "HLAudioFile::GetAudioByteSize: file isn't prepared");
	
	UInt32 theSize = sizeof(SInt64);
	SInt64 theAnswer = 0;
	OSStatus theError = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyAudioDataByteCount, &theSize, &theAnswer);
	ThrowIfError(theError, CAException(theError), "HLAudioFile::GetAudioByteSize: couldn't get the property");
	
	return theAnswer;
}

void	HLAudioFile::SetAudioByteSize(SInt64 inSize)
{
	ThrowIf(mAudioFileID == 0, CAException(fnOpnErr), "HLAudioFile::SetAudioByteSize: file isn't prepared");
	
	UInt32 theSize = sizeof(UInt32);
	UInt32 theByteSize = inSize;
	OSStatus theError = AudioFileSetProperty(mAudioFileID, kAudioFilePropertyAudioDataByteCount, theSize, &theByteSize);
	ThrowIfError(theError, CAException(theError), "HLAudioFile::SetAudioByteSize: couldn't set the property");
}

void	HLAudioFile::ReadAudioBytes(SInt64 inOffset, UInt32& ioNumberBytes, void* outData, bool inCache)
{
	ThrowIf(mAudioFileID == 0, CAException(fnOpnErr), "HLAudioFile::ReadAudioBytes: file isn't prepared");
	
	OSStatus theError = AudioFileReadBytes(mAudioFileID, inCache, inOffset, &ioNumberBytes, outData);
	ThrowIfError(theError, CAException(theError), "HLAudioFile::ReadAudioBytes: couldn't read the data");
}

void	HLAudioFile::WriteAudioBytes(SInt64 inOffset, UInt32& ioNumberBytes, void* inData, bool inCache)
{
	ThrowIf(mAudioFileID == 0, CAException(fnOpnErr), "HLAudioFile::WriteAudioBytes: file isn't prepared");
	
	OSStatus theError = AudioFileWriteBytes(mAudioFileID, inCache, inOffset, &ioNumberBytes, inData);
	ThrowIfError(theError, CAException(theError), "HLAudioFile::WriteAudioBytes: couldn't write the data");
}

SInt64	HLAudioFile::GetAudioFrameSize() const
{
	ThrowIf(mAudioFileID == 0, CAException(fnOpnErr), "HLAudioFile::GetAudioFrameSize: file isn't prepared");
	
	UInt32 theSize = sizeof(UInt64);
	UInt64 theAnswer = 0;
	OSStatus theError = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyAudioDataPacketCount, &theSize, &theAnswer);
	ThrowIfError(theError, CAException(theError), "HLAudioFile::GetAudioFrameSize: couldn't get the property");
	
	return theAnswer;
}

void	HLAudioFile::SetAudioFrameSize(SInt64 inSize)
{
	ThrowIf(mAudioFileID == 0, CAException(fnOpnErr), "HLAudioFile::SetAudioFrameSize: file isn't prepared");
	
	UInt32 theSize = sizeof(UInt64);
	UInt64 theFrameSize = inSize;
	OSStatus theError = AudioFileSetProperty(mAudioFileID, kAudioFilePropertyAudioDataPacketCount, theSize, &theFrameSize);
	ThrowIfError(theError, CAException(theError), "HLAudioFile::SetAudioFrameSize: couldn't set the property");
}

void	HLAudioFile::ReadAudioFrames(SInt64 inOffset, UInt32& ioNumberFrames, void* outData, bool inCache)
{
	ThrowIf(mAudioFileID == 0, CAException(fnOpnErr), "HLAudioFile::ReadAudioFrames: file isn't prepared");
	
	UInt32 theNumberBytesRead = 0;
	OSStatus theError = AudioFileReadPackets(mAudioFileID, inCache, &theNumberBytesRead, NULL, inOffset, &ioNumberFrames, outData);
	ThrowIfError(theError, CAException(theError), "HLAudioFile::ReadAudioFrames: couldn't read the data");
}

void	HLAudioFile::WriteAudioFrames(SInt64 inOffset, UInt32& ioNumberFrames, void* inData, bool inCache)
{
	ThrowIf(mAudioFileID == 0, CAException(fnOpnErr), "HLAudioFile::WriteAudioFrames: file isn't prepared");
	
	UInt32 theNumberBytesToWrite = ioNumberFrames * mFormat.mBytesPerFrame;
	OSStatus theError = AudioFileWritePackets(mAudioFileID, inCache, theNumberBytesToWrite, NULL, inOffset, &ioNumberFrames, inData);
	ThrowIfError(theError, CAException(theError), "HLAudioFile::WriteAudioFrames: couldn't write the data");
}

//=============================================================================
//	HLAudioFileFactory
//=============================================================================

HLAudioFileFactory::HLAudioFileFactory(UInt32 inATFileType, CFStringRef inNameExtension)
:
	HLFileFactory(inATFileType, inNameExtension, false)
{
}

HLAudioFileFactory::~HLAudioFileFactory()
{
}

bool	HLAudioFileFactory::ObjectIsA_FSRef(const FSRef& inFSRef) const
{
	bool theAnswer = false;
	
	//	make an AT file so we can query a few things
	AudioFileID theAudioFileID;
	OSStatus theError = AudioFileOpen(const_cast<FSRef*>(&inFSRef), fsRdPerm, 0, &theAudioFileID);
	
	//	ask the AT file what kind it is
	if(theError == 0)
	{
		UInt32 theSize = sizeof(UInt32);
		UInt32 theType = 0;
		theError = AudioFileGetProperty(theAudioFileID, kAudioFilePropertyFileFormat, &theSize, &theType);
		if(theError == 0)
		{
			theAnswer = theType == mObjectType;
			AudioFileClose(theAudioFileID);
		}
	}
	
	return theAnswer;
}

HLFileSystemObject*	HLAudioFileFactory::Create_FSRef(const FSRef& inParentFSRef, CFStringRef inName, const void* inData, UInt32 inDataSize)
{
	HLFile* theFile = NULL;
	
	//	the data for an HLAudioFile is a stream descption
	ThrowIf(inDataSize != sizeof(AudioStreamBasicDescription), CAException(paramErr), "HLAudioFileFactory::Create: the data is supposed to be an AudioStreamBasicDescription");

	const AudioStreamBasicDescription* theFormat = reinterpret_cast<const AudioStreamBasicDescription*>(inData);

	//	create the file on disk
	FSRef theFSRef;
	AudioFileID theAudioFileID = 0;
	OSStatus theError = AudioFileCreate(&inParentFSRef, inName, mObjectType, theFormat, 0, &theFSRef, &theAudioFileID);
	if(theError == 0)
	{
		AudioFileClose(theAudioFileID);
		
		//	make the object
		theFile = CreateObject(theFSRef);
		theFile->Prepare();
	}
	else
	{
		ThrowIfError(theError, CAException(theError), "HLAudioFileFactory::Create: AudioFileCreate failed");
	}
	
	return theFile;
}

HLFile*	HLAudioFileFactory::CreateObject(const FSRef& inFSRef)
{
	return new HLAudioFile(inFSRef, mObjectType, mNameExtension);
}
