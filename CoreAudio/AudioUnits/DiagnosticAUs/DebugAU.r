/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   SampleEffectUnit.r
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <AudioUnit/AudioUnit.r>

#include "DebugAUVersion.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_DebugAU				10000

// So you need to define these appropriately for your audio unit.
// For the name the convention is to provide your company name and end it with a ':',
// then provide the name of the AudioUnit.
// The Description can be whatever you want.
// For an effect unit the Type and SubType should be left the way they are defined here...
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SampleEffectUnit
#define RES_ID			kAudioUnitResID_DebugAU
#define COMP_TYPE		kAudioUnitType_Effect
#define COMP_SUBTYPE	'Dbug'	// this effect just passes audio through
#define COMP_MANUF		'Acme'	// note that Apple has reserved all all-lower-case 4-char codes for its own use.
                                // Be sure to include at least one upper-case character in each of your codes.
#define VERSION			kDebugAUVersion
#define NAME			"Acme Inc: DebugDispatcher"
#define DESCRIPTION		"Acme Inc's favorite audio effect"
#define ENTRY_POINT		"DebugAUEntry"

#include "AUResources.r"
