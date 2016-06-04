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
	MusicDeviceBase.h
	
=============================================================================*/

#ifndef __MusicDeviceBase_h__
#define __MusicDeviceBase_h__

#include "AUMIDIBase.h"

// ________________________________________________________________________
//	MusicDeviceBase
//

/*! @class MusicDeviceBase */
class MusicDeviceBase : public AUBase, public AUMIDIBase {
public:
	/*! @ctor MusicDeviceBase */
								MusicDeviceBase(		ComponentInstance				inInstance, 
														UInt32							numInputs,
														UInt32							numOutputs,
														UInt32							numGroups = 0,
														UInt32							numParts = 0);

	/*! @method PrepareInstrument */
	virtual ComponentResult		PrepareInstrument(MusicDeviceInstrumentID inInstrument) { return noErr; }

	/*! @method ReleaseInstrument */
	virtual ComponentResult		ReleaseInstrument(MusicDeviceInstrumentID inInstrument) { return noErr; }

	/*! @method StartNote */
	virtual ComponentResult		StartNote(		MusicDeviceInstrumentID 	inInstrument, 
												MusicDeviceGroupID 			inGroupID, 
												NoteInstanceID 				&outNoteInstanceID, 
												UInt32 						inOffsetSampleFrame, 
												const MusicDeviceNoteParams &inParams) = 0;

	/*! @method StopNote */
	virtual ComponentResult		StopNote(		MusicDeviceGroupID 			inGroupID, 
												NoteInstanceID 				inNoteInstanceID, 
												UInt32 						inOffsetSampleFrame) = 0;

	/*! @method GetPropertyInfo */
	virtual ComponentResult		GetPropertyInfo(AudioUnitPropertyID			inID,
												AudioUnitScope				inScope,
												AudioUnitElement			inElement,
												UInt32 &					outDataSize,
												Boolean &					outWritable);

	/*! @method GetProperty */
	virtual ComponentResult		GetProperty(	AudioUnitPropertyID 		inID,
												AudioUnitScope 				inScope,
												AudioUnitElement		 	inElement,
												void *						outData);
												
	/*! @method SetProperty */
	virtual ComponentResult		SetProperty(			AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														const void *					inData,
														UInt32 							inDataSize);
												
	virtual void		HandleNoteOn(			int 	inChannel,
												UInt8 	inNoteNumber,
												UInt8 	inVelocity,
												long 	inStartFrame);
												
	/*! @method HandleNoteOff */
	virtual void		HandleNoteOff(			int 	inChannel,
												UInt8 	inNoteNumber,
												UInt8 	inVelocity,
												long 	inStartFrame);

	/*! @method GetInstrumentCount */
	virtual OSStatus			GetInstrumentCount (	UInt32 &outInstCount) const;

	// component dispatcher
	/*! @method ComponentEntryDispatch */
	static ComponentResult		ComponentEntryDispatch(	ComponentParameters *			params,
														MusicDeviceBase *				This);
private:
	virtual ComponentResult		HandleStartNoteMessage (MusicDeviceInstrumentID inInstrument, MusicDeviceGroupID inGroupID, NoteInstanceID *outNoteInstanceID, UInt32 inOffsetSampleFrame, const MusicDeviceNoteParams *inParams);
};

#endif // __MusicDeviceBase_h__
