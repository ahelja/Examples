/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   AUPulseDetector.r
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <AudioUnit/AudioUnit.r>

#include "AUPulseDetectorVersion.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_AUPulseDetector				10000

// So you need to define these appropriately for your audio unit.
// For the name the convention is to provide your company name and end it with a ':',
// then provide the name of the AudioUnit.
// The Description can be whatever you want.
// For an effect unit the Type and SubType should be left the way they are defined here...
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SampleEffectUnit
#define RES_ID			kAudioUnitResID_AUPulseDetector
#define COMP_TYPE		kAudioUnitType_Effect
#define COMP_SUBTYPE	'puls'
#define COMP_MANUF		'appl'
#define VERSION			kAUPulseDetectorVersion
#define NAME			"Apple_DEBUG: AUPulseDetector"
#define DESCRIPTION		"Pass through AU that uses a pulse to detect latency"
#define ENTRY_POINT		"AUPulseDetectorEntry"

#include "/Developer/Examples/CoreAudio/AudioUnits/AUPublic/AUBase/AUResources.r"

#define RES_ID			1000
#define COMP_TYPE		kAudioUnitCarbonViewComponentType
#define COMP_SUBTYPE	'puls'
#define COMP_MANUF		'appl'
#define VERSION			0x00010000
#define NAME			"Apple_DEBUG: AUPulse Detector"
#define DESCRIPTION		"View for the AUPulse Detector"
#define ENTRY_POINT		"AUPulseDetectorViewEntry"

#include "/Developer/Examples/CoreAudio/AudioUnits/AUPublic/AUBase/AUResources.r"
