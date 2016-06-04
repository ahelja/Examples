#include <AudioUnit/AudioUnit.r>
#include "SinSynthVersion.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_SinSynth				13470

// So you need to define these appropriately for your audio unit.
// For the name the convention is to provide your company name and end it with a ':',
// then provide the name of the AudioUnit.
// The Description can be whatever you want.
// For an effect unit the Type and SubType should be left the way they are defined here...
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// AnalogSynth AudioUnit
#define RES_ID			kAudioUnitResID_SinSynth
#define COMP_TYPE		kAudioUnitType_MusicDevice
#define COMP_SUBTYPE	kSinSynthSubtype
#define COMP_MANUF		'Acme'
#define VERSION			kSinSynthVersion
#define NAME			"Acme: SinSynth"
#define DESCRIPTION		"SinSynth AudioUnit"
#define ENTRY_POINT		"SinSynthEntry"

#include "AUResources.r"
