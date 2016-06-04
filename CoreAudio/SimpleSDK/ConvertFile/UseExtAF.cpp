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
	This is a simple example program that shows the usage of the ExtendedAudioFile API found in AudioToolbox.framework
	
	Its input is an audio file containing linear pcm audio, and it generates a specified compressed audio file
*/

#include <AudioToolbox/AudioToolbox.h>

#include "CAStreamBasicDescription.h"
#include "CAXException.h"


int ConvertFile (FSRef &inputFSRef, OSType format, Float64 sampleRate, OSType fileType, FSRef &dirFSRef, char* fname)
{
	ExtAudioFileRef infile, outfile;

// first open the input file
	OSStatus err = ExtAudioFileOpen (&inputFSRef, &infile);
	XThrowIfError (err, "ExtAudioFileOpen");
	
// get the input file format
	CAStreamBasicDescription inputFormat;
	UInt32 size = sizeof(inputFormat);
	err = ExtAudioFileGetProperty(infile, kExtAudioFileProperty_FileDataFormat, &size, &inputFormat);
	XThrowIfError (err, "ExtAudioFileGetProperty kExtAudioFileProperty_FileDataFormat");
	printf ("Source File format: "); inputFormat.Print();

// set up the output file format
	CAStreamBasicDescription outputFormat;	
	
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
	printf ("Dest File format: "); outputFormat.Print();

// create the output file 
	CFStringRef cfName = CFStringCreateWithCString (NULL, fname, kCFStringEncodingUTF8);
	err = ExtAudioFileCreateNew (&dirFSRef, cfName, fileType, &outputFormat, NULL, &outfile);
	CFRelease (cfName);
	XThrowIfError (err, "ExtAudioFileCreateNew");
	
// get and set the client format:
	size = sizeof(inputFormat);
	err = ExtAudioFileSetProperty(infile, kExtAudioFileProperty_ClientDataFormat, size, &inputFormat);
	XThrowIfError (err, "ExtAudioFileGetProperty kExtAudioFileProperty_ClientDataFormat");
	
	size = sizeof(inputFormat);
	err = ExtAudioFileSetProperty(outfile, kExtAudioFileProperty_ClientDataFormat, size, &inputFormat);
	XThrowIfError (err, "ExtAudioFileGetProperty kExtAudioFileProperty_ClientDataFormat");

// set up buffers
	UInt32 kSrcBufSize = 32768;
	char srcBuffer[kSrcBufSize];

// do the read and write - the conversion is done on and by the write call
	while (1) 
	{	
		AudioBufferList fillBufList;
		fillBufList.mNumberBuffers = 1;
		fillBufList.mBuffers[0].mNumberChannels = inputFormat.mChannelsPerFrame;
		fillBufList.mBuffers[0].mDataByteSize = kSrcBufSize;
		fillBufList.mBuffers[0].mData = srcBuffer;

		UInt32 numFrames = (kSrcBufSize / inputFormat.mBytesPerPacket) * inputFormat.mFramesPerPacket /* normally 1*/ ;

		err = ExtAudioFileRead (infile, &numFrames, &fillBufList);
		if (err || !numFrames) {
				// this is our termination condition
			break;
		}
		
		err = ExtAudioFileWrite(outfile, numFrames, &fillBufList);	
		XThrowIfError (err, "ExtAudioFileWrite");	
	}

// close
	ExtAudioFileDispose(outfile);
	ExtAudioFileDispose(infile);
	
	printf("done: /tmp/%s\n", fname);
    return 0;
}

