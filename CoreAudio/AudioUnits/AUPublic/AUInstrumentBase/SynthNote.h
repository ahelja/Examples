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
 *  SynthNote.h
 *  TestSynth
 *
 *  Created by James McCartney on Mon Mar 29 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
=============================================================================*/

#ifndef __SynthNote__
#define __SynthNote__

#include <Carbon/Carbon.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include "MusicDeviceBase.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
	kNoteState_Attacked = 0,
	kNoteState_Sostenutoed = 1,
	kNoteState_ReleasedButSostenutoed = 2,
	kNoteState_ReleasedButSustained = 3,
	kNoteState_Released = 4,
	kNoteState_FastReleased = 5,
	kNoteState_Free = 6,
	kNumberOfActiveNoteStates = 5,
	kNumberOfSoundingNoteStates = 6,
	kNumberOfNoteStates = 7
};

/*
		This table describes the state transitions for SynthNotes

        EVENT                   CURRENT STATE                                   NEW STATE
        note on                 free                                            attacked
        note off                attacked (and sustain on)                       released but sustained
        note off                attacked                                        released
        note off                sostenutoed                                     released but sostenutoed
        sustain on              -- no changes --
        sustain off             released but sustained                          released
        sostenuto on            attacked                                        sostenutoed
        sostenuto off           sostenutoed                                     attacked
        sostenuto off           released but sostenutoed (and sustain on)       released but sustained
        sostenuto off           released but sostenutoed                        released
        end of note             any state                                       free
		soft voice stealing     any state                                       fast released
		hard voice stealing     any state                                       free
		
		soft voice stealing happens when there is a note on event and NumActiveNotes > MaxActiveNotes
		hard voice stealing happens when there is a note on event and NumActiveNotes == NumNotes (no free notes)
		voice stealing removes the quietest note in the highest numbered state that has sounding notes.
*/

class SynthGroupElement;
class SynthPartElement;
class AUInstrumentBase;

struct SynthNote
{
	SynthNote() :
		mPrev(0), mNext(0), mState(kNoteState_Free), 
		mRelativeStartFrame(0),
		mRelativeReleaseFrame(-1),
		mRelativeKillFrame(-1)
	{
	}
	
	virtual					~SynthNote() {}
	
	virtual void			Reset();
	virtual void			AttackNote(
									SynthPartElement *				inPart,
									SynthGroupElement *				inGroup,
									NoteInstanceID					inNoteID, 
									SInt64							inAbsoluteSampleFrame, 
									UInt32							inOffsetSampleFrame, 
									const MusicDeviceNoteParams &inParams
							);
								
	virtual OSStatus		Render(UInt32 inNumFrames, AudioBufferList& inBufferList)=0;
	virtual void			Attack(const MusicDeviceNoteParams &inParams) = 0;
	virtual void			Kill(UInt32 inFrame); // voice is being stolen.
	virtual void			Release(UInt32 inFrame);
	virtual void			FastRelease(UInt32 inFrame);
	virtual Float32			Amplitude() = 0; // used for finding quietest note for voice stealing.

	virtual void			NoteEnded(UInt32 inFrame);

	SynthGroupElement*		GetGroup() const { return mGroup; }
	SynthPartElement*		GetPart() const { return mPart; }
	
	AUInstrumentBase*		GetAudioUnit() const;

	Float32					GetGlobalParameter(AudioUnitParameterID inParamID) const;

	NoteInstanceID			GetNoteID() const { return mNoteID; }
	UInt32					GetState() const { return mState; }
	Boolean					IsSounding() const { return mState < kNumberOfSoundingNoteStates; }
	Boolean					IsActive() const { return mState < kNumberOfActiveNoteStates; }
	SInt64					GetAbsoluteStartFrame() const { return mAbsoluteStartFrame; }
	SInt32					GetRelativeStartFrame() const { return mRelativeStartFrame; }
	SInt32					GetRelativeReleaseFrame() const { return mRelativeReleaseFrame; }
	SInt32					GetRelativeKillFrame() const { return mRelativeKillFrame; }

	void					ListRemove() { mPrev = mNext = 0; } // only use when lists will be reset.

	float					PitchBend() const;
	double					TuningA() const;
	double					MiddleC() const;
	
	virtual double			Frequency(); // returns the frequency of note + pitch bend.
	double					SampleRate();

//private:
//	friend class NoteList;
	
	// linked list pointers
	SynthNote				*mPrev;
	SynthNote				*mNext;
	
	SynthPartElement*		mPart;
	SynthGroupElement*		mGroup;
		
	NoteInstanceID			mNoteID;
	UInt32					mState;
	SInt64					mAbsoluteStartFrame;
	SInt32					mRelativeStartFrame;
	SInt32					mRelativeReleaseFrame;
	SInt32					mRelativeKillFrame;
	
	Float32					mPitch;
	Float32					mVelocity;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

