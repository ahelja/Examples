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
	This is a more complex version of the ConvertFile call - see UseExtAF first
	
	The version below shows you what is in the implementation of the ExtendedAudioFile as
	it uses the AudioConverter and AudioFile APIs directly. Its alot more code to write, but 
	it is equivalent and will produce the same results.
	
	This is provided more for educational purposes as going forward we would recommend
	developers use the Extended Audio File API directly.
*/


#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>

#include "CAStreamBasicDescription.h"
#include "CAXException.h"

int ConvertFile (FSRef &inputFSRef, OSType format, Float64 sampleRate, OSType fileType, FSRef &dirFSRef, char* fname);

// a struct to hold info for the input data proc

struct AudioFileIO
{
	AudioFileID afid;
	SInt64 pos;
	char *srcBuffer;
	UInt32 srcBufferSize;
	CAStreamBasicDescription srcFormat;
};

// input data proc callback

OSStatus EncoderDataProc(		AudioConverterRef				inAudioConverter, 
								UInt32*							ioNumberDataPackets,
								AudioBufferList*				ioData,
								AudioStreamPacketDescription**	outDataPacketDescription,
								void*							inUserData)
{
	AudioFileIO* afio = (AudioFileIO*)inUserData;
	
// figure out how much to read

	UInt32 maxPackets = afio->srcBufferSize / afio->srcFormat.mBytesPerPacket;
	if (*ioNumberDataPackets > maxPackets) *ioNumberDataPackets = maxPackets;

// read from the file

	UInt32 outNumBytes;
	OSStatus err = AudioFileReadPackets(afio->afid, false, &outNumBytes, NULL, afio->pos, ioNumberDataPackets, afio->srcBuffer);
	if (err == eofErr) err = noErr;
	
// advance input file packet position

	afio->pos += *ioNumberDataPackets;

// put the data pointer into the buffer list

	if (outDataPacketDescription) *outDataPacketDescription = 0;
	ioData->mBuffers[0].mData = afio->srcBuffer;
	ioData->mBuffers[0].mDataByteSize = outNumBytes;
	ioData->mBuffers[0].mNumberChannels = 2;

	return err;
}

int ConvertFile (FSRef &inputFSRef, OSType format, Float64 sampleRate, OSType fileType, FSRef &dirFSRef, char* fname)
{
	AudioFileID infile, outfile;
	
	OSStatus err = AudioFileOpen(&inputFSRef, fsRdPerm, 0, &infile);
	XThrowIfError (err, "AudioFileOpen");
	
// get the input file format
	CAStreamBasicDescription inputFormat;
	UInt32 size = sizeof(inputFormat);
	err = AudioFileGetProperty(infile, kAudioFilePropertyDataFormat, &size, &inputFormat);
	XThrowIfError (err, "AudioFileGetProperty kAudioFilePropertyDataFormat");
	printf ("Source File format: "); inputFormat.Print();

	CAStreamBasicDescription outputFormat;	
// set up the output file format
	if (format) {
		// need to set at least these fields for kAudioFormatProperty_FormatInfo
		outputFormat.mFormatID = format;
		outputFormat.mSampleRate = inputFormat.mSampleRate;
		outputFormat.mChannelsPerFrame = inputFormat.mChannelsPerFrame;
		
	// use AudioFormat API to fill out the rest.
		size = sizeof(outputFormat);
		err = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &outputFormat);
	} else {
		outputFormat = inputFormat;
		outputFormat.mSampleRate = sampleRate;
	}
	
// create the AudioConverter

	AudioConverterRef converter;
	err = AudioConverterNew(&inputFormat, &outputFormat, &converter);
	//printf("AudioConverterNew err %4.4s %08X %d\n", &err, err, err);
	XThrowIfError (err, "AudioConverterNew");

// get the actual output format
	size = sizeof(outputFormat);
	err = AudioConverterGetProperty(converter, kAudioConverterCurrentOutputStreamDescription, &size, &outputFormat);
	XThrowIfError (err, "get kAudioConverterCurrentOutputStreamDescription");

	printf ("Dest File format: "); outputFormat.Print();

	CFStringRef cfName = CFStringCreateWithCString (NULL, fname, kCFStringEncodingUTF8);
	FSRef fsRef;
	err = AudioFileCreate(&dirFSRef, cfName, fileType, &outputFormat, 0, &fsRef, &outfile);
	CFRelease (cfName);
	XThrowIfError (err, "AudioFileCreate");

// set up buffers and data proc info struct
	UInt32 kSrcBufSize = 32768;
	char srcBuffer[kSrcBufSize];
	SInt64 pos = 0;
	AudioFileIO afio;
	afio.afid = infile;
	afio.srcBuffer = srcBuffer;
	afio.srcBufferSize = kSrcBufSize;
	afio.pos = 0;
	afio.srcFormat = inputFormat;

// grab the cookie from the converter and write it to the file
	UInt32 cookieSize = 0;
	err = AudioConverterGetPropertyInfo(converter, kAudioConverterCompressionMagicCookie, &cookieSize, NULL);
		// if there is an error here, then the format doesn't have a cookie, so on we go
	if (!err && cookieSize) {
		char* cookie = new char [cookieSize];
		
		err = AudioConverterGetProperty(converter, kAudioConverterCompressionMagicCookie, &cookieSize, cookie);
		XThrowIfError (err, "Get Cookie From AudioConverter");
	
		err = AudioFileSetProperty (outfile, kAudioFilePropertyMagicCookieData, cookieSize, cookie);
			// even though some formats have cookies, some files don't take them
		if (!err)
			printf ("write cookie to file: %ld\n", cookieSize);
		
		delete [] cookie;
	}

// write dest channel layout
	if (inputFormat.mChannelsPerFrame > 2) {
		UInt32 layoutSize = 0;
		bool layoutFromConverter = true;
		err = AudioConverterGetPropertyInfo(converter, kAudioConverterOutputChannelLayout, &layoutSize, NULL);
			
			// if the converter doesn't have a layout does the input file?
		if (err || !layoutSize) {
			err = AudioFileGetPropertyInfo (infile, kAudioFilePropertyChannelLayout, &layoutSize, NULL);
			layoutFromConverter = false;
		}
		
		if (!err && layoutSize) {
			char* layout = new char[layoutSize];
			
			if (layoutFromConverter) {
				err = AudioConverterGetProperty(converter, kAudioConverterOutputChannelLayout, &layoutSize, layout);
				XThrowIfError (err, "Get Layout From AudioConverter");
			} else {
				err = AudioFileGetProperty(infile, kAudioFilePropertyChannelLayout, &layoutSize, layout);
				XThrowIfError (err, "Get Layout From AudioFile");
			}
			
			err = AudioFileSetProperty (outfile, kAudioFilePropertyChannelLayout, layoutSize, layout);
				// even though some formats have layouts, some files don't take them
			if (!err)
				printf ("write channel layout to file: %ld\n", layoutSize);
			
			delete [] layout;
		}
	}
		
// what's the size we need per packet of output data?
	int sizePerPacket = outputFormat.mBytesPerPacket; // this will be non-zero of the format is CBR
	UInt32 numPackets;
	UInt32 theOutputBufSize = 32768;
	
// if we have a VBR format, we need packet descriptions as well.
	AudioStreamPacketDescription* pktDescs = NULL;
	
	if (sizePerPacket == 0) { // we have a VBR format, what's the max packet size?
		UInt32 size = sizeof(sizePerPacket);
		err = AudioConverterGetProperty(converter, kAudioConverterPropertyMaximumOutputPacketSize, &size, &sizePerPacket);
		XThrowIfError (err, "Get Max Packet Size");
	
		if (sizePerPacket > 32768)
			theOutputBufSize = sizePerPacket;

		numPackets = theOutputBufSize / sizePerPacket;
		pktDescs = new AudioStreamPacketDescription [numPackets];

	} else {
		numPackets = theOutputBufSize / sizePerPacket;
	}

// now, set up our output buffers
	char* outputBuffer = new char[theOutputBufSize];

	
// loop to convert data
	UInt64 totalOutputFrames = 0;
	
	while (1) {

// set up output buffer list
	
		AudioBufferList fillBufList;
		fillBufList.mNumberBuffers = 1;
		fillBufList.mBuffers[0].mNumberChannels = inputFormat.mChannelsPerFrame;
		fillBufList.mBuffers[0].mDataByteSize = theOutputBufSize;
		fillBufList.mBuffers[0].mData = outputBuffer;

// convert data
		UInt32 ioOutputDataPackets = numPackets;
		err = AudioConverterFillComplexBuffer(converter, EncoderDataProc, &afio, &ioOutputDataPackets, &fillBufList, pktDescs);
		XThrowIfError (err, "AudioConverterFillComplexBuffer");	
		if (ioOutputDataPackets == 0) {
			// this is the EOF conditon
			break;
		}

// write to output file

		UInt32 inNumBytes = fillBufList.mBuffers[0].mDataByteSize;
		err = AudioFileWritePackets(outfile, false, inNumBytes, pktDescs, pos, &ioOutputDataPackets, outputBuffer);
		XThrowIfError (err, "AudioFileWritePackets");	
		
// advance output file packet position

		pos += ioOutputDataPackets;
		
		if (outputFormat.mFramesPerPacket) { 
				// this is the common case: format has constant frames per packet
			totalOutputFrames += (ioOutputDataPackets * outputFormat.mFramesPerPacket);
		} else {
				// if there are variable frames per packet, then we have to do this for each packeet
			for (unsigned int i = 0; i < ioOutputDataPackets; ++i)
				totalOutputFrames += pktDescs[i].mVariableFramesInPacket;
		}
	}

// we right out any of the leading and trailing frames for compressed formats only	
	if (outputFormat.mBitsPerChannel) {
	// last job is to make sure we write out the priming and remainder details to the file
		AudioConverterPrimeInfo primeInfo;
		UInt32 primeSize = sizeof(primeInfo);

		err = AudioConverterGetProperty(converter, kAudioConverterPrimeInfo, &primeSize, &primeInfo);
			// if there's an error we don't care
		if (err == noErr) {
				// there's priming to write out to the file
			AudioFilePacketTableInfo pti;
			pti.mPrimingFrames = primeInfo.leadingFrames;
			pti.mRemainderFrames = primeInfo.trailingFrames;
			pti.mNumberValidFrames = totalOutputFrames - pti.mPrimingFrames - pti.mRemainderFrames;
			err = AudioFileSetProperty(outfile, kAudioFilePropertyPacketTableInfo, sizeof(pti), &pti);
				// we don't care about this err, some audio files can't contain this information
		}
	}
	
// cleanup
	delete [] pktDescs;
	delete [] outputBuffer;

	AudioConverterDispose(converter);
	AudioFileClose(outfile);
	AudioFileClose(infile);
	
	printf("done: /tmp/%s\n", fname);
    return 0;
}
