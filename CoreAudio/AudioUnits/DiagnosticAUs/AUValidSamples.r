/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   AUValidSamples.r
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <AudioUnit/AudioUnit.r>

#include "AUValidSamplesVersion.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_AUValidSamples				10000

// So you need to define these appropriately for your audio unit.
// For the name the convention is to provide your company name and end it with a ':',
// then provide the name of the AudioUnit.
// The Description can be whatever you want.
// For an effect unit the Type and SubType should be left the way they are defined here...

#define RES_ID			kAudioUnitResID_AUValidSamples
#define COMP_TYPE		kAudioUnitType_Effect
#define COMP_SUBTYPE	'vsmp'
#define COMP_MANUF		'appl'	
#define VERSION			kAUValidSamplesVersion
#define NAME			"Apple_DEBUG: AUValidSamples"
#define DESCRIPTION		"Validates the samples as it passes through the AU"
#define ENTRY_POINT		"AUValidSamplesEntry"

#include "/Developer/Examples/CoreAudio/AudioUnits/AUPublic/AUBase/AUResources.r"


#define RES_ID			1002
#define COMP_TYPE		kAudioUnitCarbonViewComponentType
#define COMP_SUBTYPE	'vsmp'
#define COMP_MANUF		'appl'
#define VERSION			0x00010000
#define NAME			"Apple_DEBUG: AUValidSamples"
#define DESCRIPTION		"View for the AUValidSamples"
#define ENTRY_POINT		"AUValidSamplesViewEntry"

#include "/Developer/Examples/CoreAudio/AudioUnits/AUPublic/AUBase/AUResources.r"
