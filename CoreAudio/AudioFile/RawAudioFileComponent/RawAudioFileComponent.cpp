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
/*
 *  RawAudioFileComponent.cpp
 *  RawAudioFileComponent
 *
 *  Created by James McCartney on Fri Oct 17 2003.
 *  Copyright (c) 2003 Apple Computer. All rights reserved.
 *
 */

#include <CoreServices/CoreServices.h>
#include "RawAudioFileComponent.h"
#include "CADebugMacros.h"

// #define VERBOSE 1

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class  RawAudioFile : public AudioFileObject
{	
public:    
  
	RawAudioFile ()
		: AudioFileObject(kAudioFileRawType)
		  {}

	virtual OSStatus Create(		const FSRef							*inFileRef,
									CFStringRef							inFileName,
									const AudioStreamBasicDescription	*inFormat,
									FSRef								*outNewFileRef);
                                
	virtual OSStatus OpenFromDataSource(SInt8 inPermissions);
						
	virtual OSStatus InitializeDataSource(const AudioStreamBasicDescription	*inFormat);
										
	OSStatus WriteHeader();
	
	OSStatus ReadHeader();
		
	virtual Boolean IsDataFormatSupported(const AudioStreamBasicDescription	*inFormat);
		
/* debug */
	//void PrintObject (FILE* inFile);	
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class RawAudioFormat : public AudioFileFormat
{
public:
	RawAudioFormat() : AudioFileFormat(kAudioFileRawType) {}
	
	// create an AudioFileObject for this format type.
	virtual AudioFileObject* New(); 
	
	// return true if file is of this format type
	virtual Boolean ExtensionIsThisFormat(CFStringRef inExtension);
	
	// return true if file is of this format type
	virtual UncertainResult DataSourceIsThisFormat(DataSource* inDataSource);

	virtual void GetExtensions(CFArrayRef *outArray);
	virtual void GetFileTypeName(CFStringRef *outName);
	virtual OSStatus GetAvailableFormatIDs(UInt32* ioDataSize, void* outPropertyData);
	virtual OSStatus GetAvailableStreamDescriptions(UInt32 inFormatID, UInt32* ioDataSize, void* outPropertyData);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AudioFileObject* RawAudioFormat::New()
{
#if VERBOSE
	printf("RawAudioFormat::New\n");
#endif

	return new RawAudioFile();
}

Boolean RawAudioFormat::ExtensionIsThisFormat(CFStringRef inExtension)
{
#if VERBOSE
	printf("RawAudioFormat::ExtensionIsThisFormat %d\n", cfstrcmpi(inExtension, CFSTR("raw")));
#endif

	return cfstrcmpi(inExtension, CFSTR("raw"));
}

UncertainResult RawAudioFormat::DataSourceIsThisFormat(DataSource* inDataSource)
{
#if VERBOSE
	printf("RawAudioFormat::DataSourceIsThisFormat\n");
#endif

	// we can read everything as audio, but we don't want to claim everything 
	// as our own or otherwise an AIFF file for example might be opened by us instead of AIFF.
	return kCantDetermine;
}

void RawAudioFormat::GetExtensions(CFArrayRef *outArray)
{
	const int size = 1;
	CFStringRef data[size];
	
	data[0] = CFSTR("raw");
	
	*outArray = CFArrayCreate(kCFAllocatorDefault, (const void**)data, size, &kCFTypeArrayCallBacks);
}

void RawAudioFormat::GetFileTypeName(CFStringRef *outName)
{
	*outName = CFCopyLocalizedString(CFSTR("RAW"), CFSTR("A file type name."));
}

OSStatus RawAudioFormat::GetAvailableFormatIDs(UInt32* ioDataSize, void* outPropertyData)
{
	const UInt32 size = 1;
	UInt32 data[size];
	data[0] = kAudioFormatLinearPCM;
	
	UInt32 numIDs = std::min(size, *ioDataSize / sizeof(UInt32));
	*ioDataSize = numIDs * sizeof(UInt32);
	if (outPropertyData) memcpy(outPropertyData, data, *ioDataSize);
	return noErr;
}

OSStatus RawAudioFormat::GetAvailableStreamDescriptions(UInt32 inFormatID, UInt32* ioDataSize, void* outPropertyData)
{
	UInt32 numIDs = 1;
	const UInt32 maxsize = 6;
	AudioStreamBasicDescription desc[maxsize];
	
	switch (inFormatID)
	{
		case kAudioFormatLinearPCM :
		{
			numIDs = 6;
			memset(desc, 0, numIDs * sizeof(AudioStreamBasicDescription));
						
			// 8 to 32 bits signed
			UInt32 numBits = 8;
			for (int i=0; i < 4; ++i) {
				
				desc[i].mFormatID = kAudioFormatLinearPCM;
				desc[i].mFormatFlags = kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
				desc[i].mBitsPerChannel = numBits;
				numBits += 8;
				desc[i].mFramesPerPacket = 1;
			}
		
			desc[4].mFormatID = kAudioFormatLinearPCM;
			desc[4].mFormatFlags = kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
			desc[4].mBitsPerChannel = 32;
			desc[4].mFramesPerPacket = 1;
		
			desc[5].mFormatID = kAudioFormatLinearPCM;
			desc[5].mFormatFlags = kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
			desc[5].mBitsPerChannel = 64;
			desc[5].mFramesPerPacket = 1;
			break;
		}
		
		default :
			*ioDataSize = 0;
			return kAudioFileUnsupportedDataFormatError;
	}
	
	numIDs = std::min(numIDs, *ioDataSize / sizeof(AudioStreamBasicDescription));
	*ioDataSize = numIDs * sizeof(AudioStreamBasicDescription);
	if (outPropertyData) memcpy(outPropertyData, desc, *ioDataSize);
	return noErr;
}


OSStatus RawAudioFile::Create(		
						const FSRef							*inFileRef,
						CFStringRef							inFileName,
						const AudioStreamBasicDescription	*inFormat,
						FSRef								*outNewFileRef)
{
#if VERBOSE
	printf("RawAudioFile::Create\n");
#endif

	OSStatus err = noErr;
	
	err = AudioFileObject::Create(inFileRef, inFileName, inFormat, outNewFileRef);
	ThrowIf(err,err,"AudioFileObject::Create failed");
	
	WriteHeader();
	return err;
}

												
OSStatus RawAudioFile::OpenFromDataSource(SInt8  			inPermissions)
{		
#if VERBOSE
	printf("RawAudioFile::OpenFromDataSource\n");
#endif

	OSStatus err = noErr;

	if (inPermissions & fsRdPerm) {
		err = ReadHeader();
		FailIf(err != noErr, Bail, "ReadHeader");
	}
	
Bail:
	return err;
}
					
OSStatus RawAudioFile::InitializeDataSource(const AudioStreamBasicDescription	*inFormat)
{
#if VERBOSE
	printf("RawAudioFile::InitializeDataSource\n");
#endif

	WriteHeader();

	return noErr;
}

OSStatus RawAudioFile::ReadHeader()
{
/* this is a raw file with no header and no stream description, so we need a default setting */
	OSStatus err = noErr;
	
	AudioStreamBasicDescription desc;
	memset(&desc, 0, sizeof(desc));
	
	desc.mFormatID = kAudioFormatLinearPCM;
	desc.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked;
	desc.mSampleRate = 44100.;
	desc.mChannelsPerFrame = 2;
	desc.mFramesPerPacket = 1;
	desc.mBytesPerPacket = 4;
	desc.mBytesPerFrame = 4;
	desc.mBitsPerChannel = 16;
	
	SetDataFormat(&desc);
	
	SInt64 size;
	err = GetDataSource()->GetSize(size);
	if (err) return err;
	
	SetNumBytes(size);
	SetNumPackets(size / desc.mBytesPerPacket);
    SetMaximumPacketSize(desc.mBytesPerPacket);

	return err;
}

OSStatus RawAudioFile::WriteHeader()
{
	/* raw files have no header */
	
	return noErr;
}


Boolean RawAudioFile::IsDataFormatSupported(const AudioStreamBasicDescription * inFormat)
{
#if VERBOSE
	printf("RawAudioFile::IsDataFormatSupported\n");
#endif

	return inFormat->mFormatID == kAudioFormatLinearPCM;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



RawAudioFormat* gRawAudioFormat = new RawAudioFormat();

AudioFileFormat* RawAudioFileComponent::GetAudioFormat() const
{
	return gRawAudioFormat;
}

RawAudioFileComponent::RawAudioFileComponent(ComponentInstance inInstance)
	: AudioFileObjectComponentBase(inInstance)
{
#if VERBOSE
	printf("RawAudioFileComponent::RawAudioFileComponent\n");
#endif

	SetAudioFileObject(new RawAudioFile());
}

COMPONENT_ENTRY(RawAudioFileComponent);



