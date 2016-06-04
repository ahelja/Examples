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
	AudioUnitHosting.cpp
	
=============================================================================*/
	
	// this code will also look for the first MIDI Source it finds
	// and will direct that MIDI input to the selected unit
	
	// If hosting effect or converter units - then audio files can be used
	// If hosting music devices - then midi files

#define PRINT_DIAGNOSTICS 0

	// set this to zero to host MusicDevice units
#define HOST_EFFECT_UNIT 1

#if HOST_EFFECT_UNIT
	// this will overide the HOST_EFFECT_UNIT setting to host units of type 'aufc' - like the varispeed AU
	#define HOST_CONVERTER_UNIT 0
#else
	#define HOST_CONVERTER_UNIT 0
#endif
 
	// set this to true to skip a custom view and just use the generic view
static const bool sUseGenericView = false;
	
#include "AudioUnitHosting.h"
#include "AUEditWindow.h"


#pragma mark __AudioEffectHosting
AudioEffectHosting::AudioEffectHosting ()
	: AudioUnitHosting (),
	  mPlayer (0)
{
	CompleteInit();
}

void 	AudioEffectHosting::SelectedFile (const FSRef& inFSRef)
{
	if (mPlayer) {
		AUGraphStop(mGraph);
		TargetUnitDisengage();
		DisposeAudioFilePlayID (mPlayer);
		mPlayer = 0;
		AudioUnitReset (mTargetUnit, kAudioUnitScope_Global, 0);
	}
	
	AudioFilePlayID player;

	RequireNoErr(NewAudioFilePlayID (&inFSRef, &player));
		
	if (PRINT_DIAGNOSTICS)
		AFP_Print (player);
		
	RequireNoErr(AFP_SetLooping (player, true));
			
	RequireNoErr(AFP_SetNotifier (player, FilePlayNotificationHandler, player));

	mPlayer = player;

	TargetUnitEngage ();
}

void 	AudioEffectHosting::TargetUnitDisengage()
{
	if (mPlayer) {
		AFP_Disconnect (mPlayer);
	}
}	

void 	AudioEffectHosting::TargetUnitEngage ()
{
	if (mPlayer) {
		RequireNoErr(AFP_SetDestination (mPlayer, mTargetUnit, 0));
		RequireNoErr(AFP_Connect (mPlayer));
	}
}


// this is the default behaviour, but we'll do this anyway	
void 	AudioEffectHosting::FilePlayNotificationHandler (void *				inRefCon,
							OSStatus					inStatus)
{
		// if we receive any notification we'll just disconnect ourselves
	if (PRINT_DIAGNOSTICS)
		printf ("FilePlay notification:%ld\n", inStatus);
	AFP_Disconnect ((AudioFilePlayID)inRefCon);
	if (PRINT_DIAGNOSTICS)
		AFP_Print ((AudioFilePlayID)inRefCon);
}


#pragma mark __MusicDeviceHosting

MusicDeviceHosting::MusicDeviceHosting ()
	: AudioUnitHosting (),
	  mSequence (0),
	  mPlayer (0)
{
	NewMusicPlayer (&mPlayer);
	
	CompleteInit();
}

void 	MusicDeviceHosting::TargetUnitEngage ()
{
	if (mSequence) {
		RequireNoErr (MusicSequenceSetAUGraph (mSequence, mGraph));
		
			// we should be able to rely on the default behaviour here!!!
			// this bug has been fixed...
			// ie. when setting a graph to a sequence, the sequence 
			// will normally find the first MusicDevice and target that
			// unit to all of its tracks..
			// so we go through and do this manually
		
		UInt32 numTracks;
		
		RequireNoErr (MusicSequenceGetTrackCount (mSequence, &numTracks));
		for (UInt32 i = 0; i < numTracks; ++i) {
			MusicTrack track;
			RequireNoErr (MusicSequenceGetIndTrack (mSequence, i, &track));
			RequireNoErr (MusicTrackSetDestNode (track, mTargetNode));
		}
	}

	RequireNoErr(AUGraphStart (mGraph));
}

void 	MusicDeviceHosting::TargetUnitDisengage()
{
	//nothing to really do here...
}	

void 	MusicDeviceHosting::SelectedFile (const FSRef& inFSRef)
{
	AUGraphStop(mGraph);
	
	if (mPlayer)
		MusicPlayerStop (mPlayer);
		
	if (mSequence) { DisposeMusicSequence (mSequence); mSequence = NULL; }
		
	AudioUnitReset (mTargetUnit, kAudioUnitScope_Global, 0);
	
	FSSpec theSpec;
	RequireNoErr(FSGetCatalogInfo (&inFSRef, kFSCatInfoNone, NULL, NULL, &theSpec, NULL));
	
	RequireNoErr(NewMusicSequence(&mSequence));

	RequireNoErr(MusicSequenceLoadSMF(mSequence, &theSpec));
	
	if (PRINT_DIAGNOSTICS)
		CAShow (mSequence);
	
		// this will start the graph for us....
	TargetUnitEngage ();
	
	RequireNoErr (MusicPlayerSetSequence (mPlayer, mSequence));
}



#pragma mark __General_AudioUnitHosting

AudioUnitHosting::AudioUnitHosting() 
	: XApp(CFSTR("main")),
	  mGraph (0), 
	  mTargetUnit (0),
	  mTargetNode (0),
	  mCurrentTargetDesc (0),
	  mClassData (0),
	  mMIDIController (0)
{
}

void 	AudioUnitHosting::CompleteInit ()
{
	GetListOfAudioUnits();
	
	mEditWindow = new AUEditWindowController(this, mMainNib);
	mFileChooser = GetFileChooser (mEditWindow->GetWindow());
	mEditWindow->GetWindow()->SetResizeableControl (mFileChooser);
    
	mAUMenu = new XControl (mEditWindow->GetWindow(), kControlKindPopupGroupBox, kPopupAudioUnitListID);
	mBypassEffect = new XControl (mEditWindow->GetWindow(), kControlKindCheckBox, kBypassCheckBoxID);
    BuildMenuList (*mAUMenu);

	int numMIDI = MIDIGetNumberOfSources ();

	if (numMIDI > 0) {
		//being lazy we're JUST picking up the first MIDI input
		RequireNoErr (AUMIDIControllerCreate (NULL, &mMIDIController));

		MIDIEndpointRef endpoint = MIDIGetSource (0);
	
		RequireNoErr (AUMIDIControllerConnectSource (mMIDIController, endpoint));
	}

	BuildGraph ();

	int targetUnitIndex = RestoreAUSelectionFromPreferences();
	SetTargetUnit (mCompDescs[targetUnitIndex]);	
	mAUMenu->SetValue(targetUnitIndex + 1);

	mEditWindow->SetUnitToDisplay (mTargetUnit, sUseGenericView);
	
	mEditWindow->GetWindow()->Show();
}

//_MD changed
// we're just going to tear this down and start again!!!
void	AudioUnitHosting::BuildGraph ()
{
	RequireNoErr(NewAUGraph(&mGraph));
	
	ComponentDescription	cd;
	cd.componentFlags = 0;        
	cd.componentFlagsMask = 0;     
	cd.componentType = kAudioUnitType_Output;
	cd.componentSubType = kAudioUnitSubType_DefaultOutput;       	
	cd.componentManufacturer = kAudioUnitManufacturer_Apple; 
    
	RequireNoErr(AUGraphNewNode(mGraph, &cd, 0, NULL, &mOutputNode));
    
	RequireNoErr(AUGraphOpen(mGraph));
	
	RequireNoErr(AUGraphInitialize(mGraph));
}

bool 	AudioUnitHosting::SetTargetUnit (ComponentDescription& 	inDesc,
									const CFPropertyListRef 	inClassData)
{
	if (!mGraph) return false;
		
	if (mCurrentTargetDesc) {
		if (mCurrentTargetDesc->componentType == inDesc.componentType
			&& mCurrentTargetDesc->componentSubType == inDesc.componentSubType
			&& mCurrentTargetDesc->componentManufacturer == inDesc.componentManufacturer) 
		{
			return false;
		}
	}
	
	if (mTargetUnit && mEditWindow)
		mEditWindow->CloseView();

				// remove midi mapping
	if (mMIDIController && mTargetUnit)
		RequireNoErr (AUMIDIControllerUnmapAudioUnit(mMIDIController, mTargetUnit));
	
	RequireNoErr(AUGraphStop (mGraph));
	
	StopPlayer();
	
	TargetUnitDisengage();	

	if (mTargetNode) {
		RequireNoErr (AUGraphRemoveNode (mGraph, mTargetNode));
		
		// lets just update the state of the graph to remove the node
		AUGraphUpdate (mGraph, NULL);
	}
	
	if (PRINT_DIAGNOSTICS) {
		OSType bigEndianValue = CFSwapInt32HostToBig(inDesc.componentType);
		printf ("%4.4s - ", (char*)&bigEndianValue);
		bigEndianValue = CFSwapInt32HostToBig(inDesc.componentSubType);
		printf ("%4.4s - ", (char*)&bigEndianValue);
		bigEndianValue = CFSwapInt32HostToBig(inDesc.componentManufacturer);
		printf ("%4.4s", (char*)&bigEndianValue);		
	}
	

	RequireNoErr(AUGraphNewNode(mGraph, &inDesc, 0, inClassData, &mTargetNode));

	RequireNoErr(AUGraphConnectNodeInput (mGraph, mTargetNode, 0, mOutputNode, 0));

	RequireNoErr(AUGraphGetNodeInfo(mGraph, mTargetNode, NULL, NULL, NULL, &mTargetUnit));

	TargetUnitEngage ();
	
	if (PRINT_DIAGNOSTICS)
		CAShow (mGraph);

	if (mMIDIController)
		RequireNoErr (AUMIDIControllerMapChannelToAU(mMIDIController, -1, mTargetUnit, -1, false));

	mCurrentTargetDesc = &inDesc;
	UpdatePreferencesAUSelection();
	
	return true;
}

bool	AudioUnitHosting::DoCommand(UInt32 command)
{
	switch (command) {
	case 'aumn':		
		if (SetTargetUnit (mCompDescs[mAUMenu->GetValue() - 1])) {
			mEditWindow->SetUnitToDisplay (mTargetUnit, sUseGenericView);
		}
		break;
	
	case 'save':
		SavePreset();
		break;
	
	case 'rest':
        // stop graph if it's running (we don't want to thrash the
        // AU state while it's playing: could lead to some ugly
        // audio...)
        if (IsPlaying()) StopPlayer();
        
        // restore preset
        if (RestorePreset()) {
			mEditWindow->SetUnitToDisplay (mTargetUnit, sUseGenericView);
		}
		break;
	
	case 'play':
		{
			if (HasPlayer()) {
				bool isPlaying = IsPlaying();
				if (!isPlaying) StartPlayer();
			}
		}
		break;
		
	case 'stop':
		{
			if (HasPlayer()) {
				bool isPlaying = IsPlaying();
				if (isPlaying) StopPlayer();	
			}
		}
		break;
	
	case 'byps':
		{
			SetBypass(mBypassEffect->GetValue() == kControlCheckBoxCheckedValue);
		}
		break;
		
	default:
		return XApp::DoCommand(command);
	}
	return true;
}

void	AudioUnitHosting::SetBypass (bool inShouldBypass)
{
	UInt32 bypass = inShouldBypass ? 1 : 0;
	AudioUnitSetProperty (	mTargetUnit, kAudioUnitProperty_BypassEffect,
                            kAudioUnitScope_Global, 0, &bypass, sizeof(bypass));
}

void	AudioUnitHosting::SavePreset ()
{
	if (mClassData) CFRelease (mClassData);
	mClassData = NULL;
	
	AUPreset myPreset;
	myPreset.presetNumber = -1; //should be less than zero as this is a "user preset" name we're setting
	myPreset.presetName = CFSTR("My Preset");
	
	RequireNoErr (AudioUnitSetProperty (mTargetUnit, 
								kAudioUnitProperty_CurrentPreset, kAudioUnitScope_Global, 0,
								&myPreset, sizeof(myPreset)));

		// now lets get it
	UInt32 size = sizeof(CFPropertyListRef);
	RequireNoErr (AudioUnitGetProperty (mTargetUnit, 
								kAudioUnitProperty_ClassInfo, kAudioUnitScope_Global, 0,
								&mClassData, &size));
								
		// now lets have a look at it
	if (PRINT_DIAGNOSTICS)
		CFShow (mClassData);
}

bool	AudioUnitHosting::RestorePreset ()
{
	if (!mClassData) return false; //haven't saved yet
	
	CFDictionaryRef dict = (CFDictionaryRef)mClassData;
	CFNumberRef cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, CFSTR("type")));
	ComponentDescription cd;
	CFNumberGetValue (cfnum, kCFNumberSInt32Type, &cd.componentType);

// second step -> check that this data belongs to this kind of audio unit
// by checking the component type, subtype and manuID
// they MUST all match
	cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, CFSTR("subtype")));
	CFNumberGetValue (cfnum, kCFNumberSInt32Type, &cd.componentSubType);

	cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, CFSTR("manufacturer")));
	CFNumberGetValue (cfnum, kCFNumberSInt32Type, &cd.componentManufacturer);
	
	cd.componentFlags = 0;        
	cd.componentFlagsMask = 0;     
	
// 	if we're already looking at the unit to be restored then lets stop the graph
//	and set the state of the unit manually
	bool newUnitToDisplay = false;
	if (mCurrentTargetDesc
		&& cd.componentManufacturer == mCurrentTargetDesc->componentManufacturer
		&& cd.componentSubType == mCurrentTargetDesc->componentSubType
		&& cd.componentType == mCurrentTargetDesc->componentType)
	{
			// we could stop the graph here, or temporarily disconnect the unit
			// to avoid glitching..		
		RequireNoErr (AudioUnitSetProperty (mTargetUnit, 
								kAudioUnitProperty_ClassInfo, kAudioUnitScope_Global, 0,
								&mClassData, sizeof(mClassData)));
			
			// now that we've potentially changed the values of the parameter we
			// should notify any listeners of this change:
		AudioUnitParameter changedUnit;
		changedUnit.mAudioUnit = mTargetUnit;
		changedUnit.mParameterID = kAUParameterListener_AnyParameter;
		RequireNoErr (AUParameterListenerNotify (NULL, NULL, &changedUnit));
		
	} else {
		mEditWindow->CloseView();
		SetTargetUnit (cd, mClassData);
		
		newUnitToDisplay = true;
		
		// now set up the menu to reflect our new unit
		for (int i = 0; i < mNumUnits; ++i) 
		{
			if (cd.componentManufacturer == mCompDescs[i].componentManufacturer
				&& cd.componentSubType == mCompDescs[i].componentSubType
				&& cd.componentType == mCompDescs[i].componentType)
			{
				mAUMenu->SetValue(i + 1);
				break;
			}
		}
	}
	
	return newUnitToDisplay;
}

void	AudioUnitHosting::BuildMenuList (XControl& theControl)
{
	MenuRef menu = GetControlPopupMenuHandle (theControl.MacControl());

	// clean the old items out
	DeleteMenuItems (menu, 1, GetControlMaximum (theControl.MacControl()));
	
	CFMutableArrayRef menuItemsArray = CFArrayCreateMutable(kCFAllocatorDefault, mNumUnits, &kCFTypeArrayCallBacks);
	// XXX oof, what to do now?  at least this probably shouldn't happen...
	if (menuItemsArray == NULL)
		return;

	// build a CFArray of all of the names of each AU in our ComponentDescriptions array
	for (int i = 0; i < mNumUnits; i++)
	{
		CFStringRef menuItemString = NULL;
		// Marc Poirier -style menu item
		Component auComponent = FindNextComponent(NULL, &(mCompDescs[i]));
		if (auComponent != NULL)
		{
			ComponentDescription dummydesc;
			Handle nameHandle = NewHandle(sizeof(void*));
			if (nameHandle != NULL)
			{
				OSErr err0r = GetComponentInfo(auComponent, &dummydesc, nameHandle, NULL, NULL);
				if (err0r == noErr)
				{
					ConstStr255Param nameString = (ConstStr255Param) (*nameHandle);
					if (nameString != NULL)
						menuItemString = CFStringCreateWithPascalString(kCFAllocatorDefault, nameString, CFStringGetSystemEncoding());
				}
				DisposeHandle(nameHandle);
			}
		}

		// if Marc-style fails, do the original way
		if (menuItemString == NULL)
		{
			CFStringRef compTypeString = UTCreateStringForOSType(mCompDescs[i].componentType);
			CFStringRef compSubTypeString = UTCreateStringForOSType(mCompDescs[i].componentSubType);
			CFStringRef compManufacturerString = UTCreateStringForOSType(mCompDescs[i].componentManufacturer);
			
			menuItemString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@ - %@ - %@"), 
				compTypeString, compManufacturerString, compSubTypeString);

			if (compTypeString != NULL)
				CFRelease(compTypeString);
			if (compSubTypeString != NULL)
				CFRelease(compSubTypeString);
			if (compManufacturerString != NULL)
				CFRelease(compManufacturerString);
		}

		CFArraySetValueAtIndex(menuItemsArray, i, menuItemString);	// XXX do this even if the string is null?
		if (menuItemString != NULL)
			CFRelease(menuItemString);
	}

	// sort the menu item strings so that they are in alphabetical order (using a bubble sort)
	for (CFIndex passCount=0; passCount < (mNumUnits-1); passCount++)
	{
		bool stillSorting = false;
		for (CFIndex i=0; i < (mNumUnits-1); i++)
		{
			CFStringRef item1 = (CFStringRef) CFArrayGetValueAtIndex(menuItemsArray, i);
			CFStringRef item2 = (CFStringRef) CFArrayGetValueAtIndex(menuItemsArray, i+1);
			if ( (item1 == NULL) || (item2 == NULL) )
				continue;	// XXX eh?
			// swap the neighbors if they're out of alphebetical order
			// we need to swap both the items in the ComponentDescriptions array and in the menu items array
			if ( CFStringCompare(item1, item2, kCFCompareCaseInsensitive) == kCFCompareGreaterThan )
			{
				ComponentDescription tempComp = mCompDescs[i];
				mCompDescs[i] = mCompDescs[i+1];
				mCompDescs[i+1] = tempComp;
				CFArrayExchangeValuesAtIndices(menuItemsArray, i, i+1);
				stillSorting = true;
			}
		}
		// no need to go through all mNumUnits-1 factorial iterations if the array is already fully sorted now
		if (!stillSorting)
			break;
	}

	// now that they're sorted, actually go through and add all of the menu entries
	for (CFIndex i=0; i < mNumUnits; i++)
	{
		CFStringRef itemString = (CFStringRef) CFArrayGetValueAtIndex(menuItemsArray, i);
		MenuItemAttributes itemAttributes = 0;
		// XXX not really sure what else to do about this, although it really shouldn't ever be a problem
		if (itemString == NULL)
			itemAttributes |= kMenuItemAttrSeparator;
		AppendMenuItemTextWithCFString(menu, itemString, itemAttributes, 0, NULL);
	}

	CFRelease(menuItemsArray);
	SetControlMaximum (theControl.MacControl(), mNumUnits);
}

void 	AudioUnitHosting::GetListOfAudioUnits ()
{
	mNumUnits = 0;

#if HOST_CONVERTER_UNIT
	int numTypes = 1; 
#else
	int numTypes = GetNumAUTypesToFind();
#endif
	
	ComponentDescription	desc;
	desc.componentFlags = 0;        
	desc.componentFlagsMask = 0;     
	desc.componentSubType = 0;       	
	desc.componentManufacturer = 0;
	
	for (int i = 0; i < numTypes; ++i) {
#if HOST_CONVERTER_UNIT
		desc.componentType = kAudioUnitType_FormatConverter;
#else
		desc.componentType = GetNthAUType (i);
#endif
	
		int n = CountComponents (&desc);
		if (n == 0)
			continue;
			
		mNumUnits += n;
	}
		
	if (mNumUnits == 0)
		throw -1; // can't find any components!!!!

	mCompDescs = (ComponentDescription*)malloc (sizeof (ComponentDescription) * mNumUnits);
	memset (mCompDescs, 0, sizeof (ComponentDescription) * mNumUnits);
	
	int n = 0;
	for (int i = 0; i < numTypes; ++i)
	{
#if HOST_CONVERTER_UNIT
		desc.componentType = kAudioUnitType_FormatConverter;
#else
		desc.componentType = GetNthAUType (i);
#endif

		Component comp = 0;

		comp = FindNextComponent (NULL, &desc);
		while (comp != NULL) {
			ComponentDescription temp;
			GetComponentInfo (comp, &temp, NULL, NULL, NULL);
			mCompDescs[n++] = temp;
			comp = FindNextComponent (comp, &desc);
		}
	}
}

int AudioUnitHosting::RestoreAUSelectionFromPreferences()
{
	OSType auType = CFPreferencesGetAppIntegerValue(CFSTR(kAUPresetTypeKey), kCFPreferencesCurrentApplication, NULL);
	OSType auSubtype = CFPreferencesGetAppIntegerValue(CFSTR(kAUPresetSubtypeKey), kCFPreferencesCurrentApplication, NULL);
	OSType auManu = CFPreferencesGetAppIntegerValue(CFSTR(kAUPresetManufacturerKey), kCFPreferencesCurrentApplication, NULL);
	for (int i=0; i < mNumUnits; i++)
	{
		if ( (mCompDescs[i].componentType == auType) 
				&& (mCompDescs[i].componentSubType == auSubtype) 
				&& (mCompDescs[i].componentManufacturer == auManu) )
			return i;
	}
	return 0;
}

void AUH_AddNumToPreferences(CFStringRef inPrefKey, SInt32 inValue)
{
	CFNumberRef number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &inValue);
	if (number != NULL)
	{
		CFPreferencesSetAppValue(inPrefKey, number, kCFPreferencesCurrentApplication);
		CFRelease(number);
	}
}

void AudioUnitHosting::UpdatePreferencesAUSelection()
{
	if (mCurrentTargetDesc != NULL)
	{
		AUH_AddNumToPreferences(CFSTR(kAUPresetTypeKey), mCurrentTargetDesc->componentType);
		AUH_AddNumToPreferences(CFSTR(kAUPresetSubtypeKey), mCurrentTargetDesc->componentSubType);
		AUH_AddNumToPreferences(CFSTR(kAUPresetManufacturerKey), mCurrentTargetDesc->componentManufacturer);
		CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
	}
}


// ______________________________________________________________________________

int main(int argc, char* argv[])
{
	AudioUnitHosting *app = NULL; 
#if HOST_EFFECT_UNIT
	app = new AudioEffectHosting;
#else
	app = new MusicDeviceHosting;
#endif
	
	app->Initialize();
	app->Run();
}

