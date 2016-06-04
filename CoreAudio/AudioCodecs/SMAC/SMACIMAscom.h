/*	Copyright: 	� Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
	SMACIMAscom.h

=============================================================================*/
#if !defined(__SMACIMAscom_h__)
#define __SMACIMAscom_h__

//=============================================================================
//	Includes
//=============================================================================

#include "SMACscom.h"

#define kIMA4BlockSamples		64							/* no. samples in a block */
#define kIMA4BlockFrames		32							/* no. frames in a block */
#define kIMA4BlockBytes			34							/* no. bytes in a block */
#define kIMA4BytesPerSample		2							/* no. bytes in decompressed sample */

//=============================================================================
//	SMACIMAscom
//=============================================================================

class SMACIMAscom
:
	public	SMACscom
{

//	Construction/Destruction
public:
								SMACIMAscom(ComponentInstance inSelf);
	virtual						~SMACIMAscom();

//	Sound Component Methods
public:
	virtual void				GetInfo(SoundSource inSourceID, OSType inSelector, void* outData);
	virtual void				SetInfo(SoundSource inSourceID, OSType inSelector, void* inData);

	virtual void				SetOutput(SoundComponentData* inRequested, SoundComponentData** inActual);

	virtual void				GetSourceData(SoundComponentData** outData);

//	Implementation
protected:
	virtual void				GetCompressionInfo(CompressionInfo& outCompressionInfo);
	virtual void				GetCompressionParams(void* outData);
	virtual ComponentInstance	SetCompressionParams(const void* inData);

	void	CreateIMAEncoder();
	
//	Utility routines
protected:
	static Component			FindIMAAudioEncoderComponent(const AudioStreamBasicDescription& inFormat);
	ComponentInstance			InitializeIMAAudioEncoder(Component inEncoderComponent, const AudioStreamBasicDescription& inFormat);
	UInt32	mNumChannels;
	UInt32	mSampleRate;

};

#endif
