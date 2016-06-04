#define UseExtendedThingResource 1
#include <CoreServices/CoreServices.r>
#include <AudioUnit/AudioUnitCarbonView.r>

#define RES_ID			1000
#define COMP_TYPE		kAudioUnitCarbonViewComponentType
#define COMP_SUBTYPE	'gnrc'
#define COMP_MANUF		'appl'
#define VERSION			0x00010000
#define NAME			"Generic AUView"
#define DESCRIPTION		"Generic AUView"
#define ENTRY_POINT		"GenericAUViewEntryShim"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

resource 'STR ' (RES_ID, purgeable) {
	NAME
};

resource 'STR ' (RES_ID + 1, purgeable) {
	DESCRIPTION
};

resource 'dlle' (RES_ID) {
	ENTRY_POINT
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

resource 'thng' (RES_ID, NAME) {
	COMP_TYPE,
	COMP_SUBTYPE,
	COMP_MANUF,
	0,		0,		// Flags, Mask
	0,		0,		// Code
	'STR ',	RES_ID,
	'STR ',	RES_ID + 1,
	0,	0,			/* icon */
	VERSION,
	componentHasMultiplePlatforms | componentDoAutoVersion,
	0,
	{
		0, 
		'dlle', RES_ID,
		platformPowerPC
	},
};
