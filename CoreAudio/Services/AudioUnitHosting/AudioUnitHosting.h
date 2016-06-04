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
/*=============================================================================
	AudioUnitHosting.h
	
=============================================================================*/

#ifndef __AUViewTest_h__
#define __AUViewTest_h__

#include "XApp.h"
#include "XControl.h"
#include "XDebugging.h"
#include "AUEditWindow.h"
#include "AudioFileChooser.h"

#include "AudioFilePlay.h"

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreFoundation/CoreFoundation.h>

class AudioUnitHosting : public XApp, public ChooserAction {
public:
	AudioUnitHosting();
	virtual ~AudioUnitHosting() {}
	
	virtual bool	DoCommand(UInt32 command);

protected:
	void 		CompleteInit ();

	virtual void 	TargetUnitEngage () = 0;

	virtual void 	TargetUnitDisengage ()  = 0;

	virtual void 	SelectedFile (const FSRef& inFSRef) = 0;

	virtual int  	GetNumAUTypesToFind () = 0;

	virtual OSType  GetNthAUType (int index) = 0;

	virtual void	StopPlayer () = 0;

	virtual void	StartPlayer () = 0;

	virtual bool	HasPlayer () = 0;

	virtual bool	IsPlaying () = 0;  
		
	virtual CAFileChooser* GetFileChooser (AUEditWindow* inParentWindow) = 0; 

	AUGraph					mGraph;
	AudioUnit				mTargetUnit;
	AUNode					mTargetNode, mOutputNode;

	enum {
		kPopupAudioUnitListID = 100,
        kBypassCheckBoxID = 101,
		kAudioFileListBox = 1
	};

private:
	bool 		SetTargetUnit (ComponentDescription& 	inDesc,
							const CFPropertyListRef 	inClassData = NULL);
	void		BuildGraph ();
							
	void 		GetListOfAudioUnits ();
	void		BuildMenuList (XControl& theController);
	void		SavePreset ();
	bool		RestorePreset ();
	void		SetBypass (bool inShouldBypass);

	// prefs stuff
	int RestoreAUSelectionFromPreferences();
	void UpdatePreferencesAUSelection();

	
	ComponentDescription*	mCompDescs;
	int						mNumUnits;
	ComponentDescription*	mCurrentTargetDesc;
	
	XControl* 					mAUMenu;
    XControl*					mBypassEffect;
	AUEditWindowController*		mEditWindow;
	CAFileChooser*				mFileChooser;
	
	CFPropertyListRef			mClassData;
	
	AUMIDIControllerRef mMIDIController;

};

class AudioEffectHosting : public AudioUnitHosting 
{
public:
	AudioEffectHosting ();

protected:
	virtual void 	TargetUnitEngage ();
							
	virtual void 	TargetUnitDisengage();

	virtual void 	SelectedFile (const FSRef& inFSRef);

	virtual int 	GetNumAUTypesToFind () { return 2; }

	virtual OSType 	GetNthAUType (int index)
	{
		return (index == 1 ? OSType(kAudioUnitType_MusicEffect_Corrected) : kAudioUnitType_Effect);
	}

	virtual void	StopPlayer () { RequireNoErr(AUGraphStop(mGraph)); }

	virtual void	StartPlayer () { AudioUnitReset (mTargetUnit, kAudioUnitScope_Global, 0); RequireNoErr(AUGraphStart(mGraph)); }

	virtual bool	HasPlayer () { return mPlayer != 0; }

	virtual bool	IsPlaying ()  
	{ 	
		Boolean isPlaying;
		if (AUGraphIsRunning (mGraph, &isPlaying)) return false;
		return isPlaying;
	}

	virtual CAFileChooser* GetFileChooser (AUEditWindow* inParentWindow) 
	{
		return new AudioFileChooser (inParentWindow, kDataBrowserListView, kAudioFileListBox, this);
	}
		
private:

	AudioFilePlayID			mPlayer;
	
	// This is ***WRONG*** in the header file (AudioUnit/AUComponent.h) in 10.2 
	// The kAudioUnitType_MusicEffect constant should be 'aumf' NOT 'aumx' 
	// 'aumx' is the type of a V2 mixer audio unit
	enum { kAudioUnitType_MusicEffect_Corrected = 'aumf' };

	static void FilePlayNotificationHandler (void *				inRefCon,
									OSStatus					inStatus);
};

class MusicDeviceHosting : public AudioUnitHosting 
{
public:
	MusicDeviceHosting ();

protected:
	virtual void 	TargetUnitEngage ();
							
	virtual void 	TargetUnitDisengage();

	virtual void 	SelectedFile (const FSRef& inFSRef);

	virtual int 	GetNumAUTypesToFind () { return 1; }

	virtual OSType 	GetNthAUType (int index)
	{
		return kAudioUnitType_MusicDevice;
	}

	virtual void	StopPlayer () 	{ if (mPlayer) MusicPlayerStop (mPlayer); }

	virtual void	StartPlayer () 	{ if (mPlayer) MusicPlayerStart (mPlayer); }

	virtual bool	HasPlayer ()  { return mPlayer != 0; }

	virtual bool	IsPlaying ()  
	{ 	
		if (!mPlayer) return false;

		Boolean isPlaying;
			//if we get an error back here, we return false
		if (MusicPlayerIsPlaying (mPlayer, &isPlaying)) return false;

		return isPlaying;
	}

	virtual CAFileChooser* GetFileChooser (AUEditWindow* inParentWindow) 
	{
		return new MIDIFileChooser (inParentWindow, kDataBrowserListView, kAudioFileListBox, this);
	}
		
private:

	MusicSequence			mSequence;
	MusicPlayer				mPlayer;
};

#endif // __AUViewTest_h__
