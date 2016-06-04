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
/*
 *  AUInstrumentBase.h
 *  TestSynth
 *
 *  Created by James McCartney on Mon Mar 29 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 *
=============================================================================*/

#ifndef __AUInstrumentBase__
#define __AUInstrumentBase__

#include <vector>
#include <stdexcept>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include "MusicDeviceBase.h"
#include "LockFreeFIFO.h"
#include "SynthEvent.h"
#include "SynthNote.h"
#include "SynthElement.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef LockFreeFIFOWithFree<SynthEvent> SynthEventQueue;

class AUInstrumentBase : public MusicDeviceBase
{
public:
			AUInstrumentBase(
							ComponentInstance				inInstance, 
							UInt32							numInputs,
							UInt32							numOutputs,
							UInt32							numGroups = 32,
							UInt32							numParts = 0);
	virtual ~AUInstrumentBase();

	virtual ComponentResult		Initialize();
	
	virtual void				Cleanup();
	
	virtual AUElement*			CreateElement(			AudioUnitScope					scope,
														AudioUnitElement				element);

	virtual ComponentResult		Reset(					AudioUnitScope 					inScope,
														AudioUnitElement 				inElement);

	virtual ComponentResult		GetPropertyInfo(		AudioUnitPropertyID				inID,
														AudioUnitScope					inScope,
														AudioUnitElement				inElement,
														UInt32 &						outDataSize,
														Boolean &						outWritable);

	virtual ComponentResult		GetProperty(			AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement			 	inElement,
														void *							outData);

	virtual ComponentResult		SetProperty(			AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														const void *					inData,
														UInt32 							inDataSize);
														
	virtual bool				ValidFormat(			AudioUnitScope					inScope,
														AudioUnitElement				inElement,
														const CAStreamBasicDescription  & inNewFormat);

	virtual bool				StreamFormatWritable(	AudioUnitScope					scope,
														AudioUnitElement				element);

	virtual ComponentResult		Render(					AudioUnitRenderActionFlags &	ioActionFlags,
														const AudioTimeStamp &			inTimeStamp,
														UInt32							inNumberFrames);

	virtual ComponentResult		StartNote(		MusicDeviceInstrumentID 	inInstrument, 
												MusicDeviceGroupID 			inGroupID, 
												NoteInstanceID 				&outNoteInstanceID, 
												UInt32 						inOffsetSampleFrame, 
												const MusicDeviceNoteParams &inParams);

	virtual ComponentResult		StopNote(		MusicDeviceGroupID 			inGroupID, 
												NoteInstanceID 				inNoteInstanceID, 
												UInt32 						inOffsetSampleFrame);

	virtual ComponentResult		RealTimeStartNote(		SynthGroupElement 			*inGroup,
														NoteInstanceID 				inNoteInstanceID, 
														UInt32 						inOffsetSampleFrame, 
														const MusicDeviceNoteParams &inParams);
	
	virtual ComponentResult		RealTimeStopNote(		SynthGroupElement 			*inGroup, 
														NoteInstanceID 				inNoteInstanceID, 
														UInt32 						inOffsetSampleFrame);
																								
	virtual void		HandleControlChange(	int 	inChannel,
												UInt8 	inController,
												UInt8 	inValue,
												long	inStartFrame);
												
	virtual void		HandlePitchWheel(		int 	inChannel,
												UInt8 	inPitch1,
												UInt8 	inPitch2,
												long	inStartFrame);
												
	virtual void		HandleChannelPressure(	int 	inChannel,
												UInt8 	inValue,
												long	inStartFrame);

	virtual void		HandleProgramChange(	int 	inChannel,
												UInt8 	inValue);

	virtual void		HandlePolyPressure(		int 	inChannel,
												UInt8 	inKey,
												UInt8	inValue,
												long	inStartFrame);

	virtual void		HandleResetAllControllers(		int 	inChannel);
	
	virtual void		HandleAllNotesOff(				int 	inChannel);
	
	virtual void		HandleAllSoundOff(				int 	inChannel);

	SynthNote*			GetNote(UInt32 inIndex) 
						{ 
							if (!mNotes) throw std::runtime_error("no notes");
							return (SynthNote*)((char*)mNotes + inIndex * mNoteSize); 
						}
	
	SynthNote*			GetAFreeNote(UInt32 inFrame);
	void				AddFreeNote(SynthNote* inNote);
	
	MidiControls*		GetControls( int inChannel)
						{
							SynthGroupElement *group = GetElForGroupID(inChannel);
							return &(group->mMidiControls);
						}


protected:

	UInt32				NextNoteID() { return IncrementAtomic(&mNoteIDCounter); }
	
	
	// call SetNotes in your Initialize() method to give the base class your note structures and to set the maximum 
	// number of active notes. inNoteData should be an array of size inMaxActiveNotes.
	void				SetNotes(UInt32 inNumNotes, UInt32 inMaxActiveNotes, SynthNote* inNotes, UInt32 inNoteSize);
	
	void				PerformEvents(   const AudioTimeStamp &			inTimeStamp);
	OSStatus			SendPedalEvent(MusicDeviceGroupID inGroupID, UInt32 inEventType, UInt32 inOffsetSampleFrame);
	virtual SynthNote*  VoiceStealing(UInt32 inFrame, bool inKillIt);
	UInt32				MaxActiveNotes() const { return mMaxActiveNotes; }
	UInt32				NumActiveNotes() const { return mNumActiveNotes; }
	void				IncNumActiveNotes() { mNumActiveNotes ++; }
	void				DecNumActiveNotes() { mNumActiveNotes --; }
	UInt32				CountActiveNotes();
		// this call throws if there's no assigned element for the group ID
	SynthGroupElement *	GetElForGroupID (MusicDeviceGroupID	inGroupID);
	
	SInt64 mAbsoluteSampleFrame;

	
private:
				
	SInt32 mNoteIDCounter;
	
	SynthEventQueue mEventQueue;
	
	UInt32 mNumNotes;
	UInt32 mNumActiveNotes;
	UInt32 mMaxActiveNotes;
	SynthNote* mNotes;	
	SynthNoteList mFreeNotes;
	UInt32 mNoteSize;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AUMonotimbralInstrumentBase : public AUInstrumentBase
{
public:
	AUMonotimbralInstrumentBase(
							ComponentInstance				inInstance, 
							UInt32							numInputs,
							UInt32							numOutputs,
							UInt32							numGroups = 32,
							UInt32							numParts = 0);
										
	virtual ComponentResult		RealTimeStartNote(			SynthGroupElement 			*inGroup, 
															NoteInstanceID 				inNoteInstanceID, 
															UInt32 						inOffsetSampleFrame, 
															const MusicDeviceNoteParams &inParams);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

// this is a work in progress! The mono-timbral one is finished though!
class AUMultitimbralInstrumentBase : public AUInstrumentBase
{
public:
	AUMultitimbralInstrumentBase(
							ComponentInstance				inInstance, 
							UInt32							numInputs,
							UInt32							numOutputs,
							UInt32							numGroups,
							UInt32							numParts);
							
	virtual ComponentResult		GetPropertyInfo(		AudioUnitPropertyID				inID,
														AudioUnitScope					inScope,
														AudioUnitElement				inElement,
														UInt32 &						outDataSize,
														Boolean &						outWritable);

	virtual ComponentResult		GetProperty(			AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement			 	inElement,
														void *							outData);

	virtual ComponentResult		SetProperty(			AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														const void *					inData,
														UInt32 							inDataSize);

private:

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

