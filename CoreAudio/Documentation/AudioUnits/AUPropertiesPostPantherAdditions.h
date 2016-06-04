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
/*	Copyright: 	© Copyright 2004 Apple Computer, Inc. All rights reserved.

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



// THESE DEFINITIONS ARE PROVIDED AS A SUPPLEMENT TO THE PANTHER HEADER
// <AudioUnit/AudioUnitProperties.h>
// THESE WILL BE DEFINED IN A FUTURE RELEASE OF THAT HEADER

#include <CoreServices/CoreServices.h>


#ifndef __AUPropertiesPostPantherAdditions__H__
#define __AUPropertiesPostPantherAdditions__H__


#pragma mark -
#pragma mark Translations of parameter values to/from strings
#pragma mark -

// ADDITIONAL PROPERTIES
enum {
		// rename existing property ID name to be clearer on usage
		// it is provided to translate a parameter value to a name string that can be presented to the user
	kAudioUnitProperty_ParameterStringFromValue			= kAudioUnitProperty_ParameterValueName,
		
		// this is the new property to retrieve the actual parameter value from a parameter name
	kAudioUnitProperty_ParameterValueFromString			= 38,
        // retrieve AU's icon location
    kAudioUnitProperty_IconLocation						= 39,
};

	
#if 0
// The existing struct:
typedef struct AudioUnitParameterValueName {
	AudioUnitParameterID	inParamID;
	Float32					*inValue;	
	CFStringRef				outName;  	
} AudioUnitParameterValueName;

// is redefined as follows to be consistent with the naming conventions in use now. The structs are equivalent otherwise
#endif

typedef struct AudioUnitParameterStringFromValue {
	AudioUnitParameterID	inParamID;
	const Float32			*inValue;	
	CFStringRef				outString;  	
} AudioUnitParameterStringFromValue;


		// used with kAudioUnitProperty_ParameterValueFromString property
typedef struct AudioUnitParameterValueFromString {
	AudioUnitParameterID	inParamID;
	CFStringRef				inString;
	Float32					outValue;
} AudioUnitParameterValueFromString;


#if 0
// This is how the modified AudioUnitParameterInfo structure will look - with the additional field
// As this can't be defined here without causing conflicts use the Temporary Macros below to 
// set the unitName field
typedef struct AudioUnitParameterInfo
{
	char 					name[52];			// UTF8 encoded C string, may be treated as 52 characters
												// if kAudioUnitParameterFlag_HasCFNameString not set
	CFStringRef				unitName;			// only valid if kAudioUnitParameterFlag_HasUnitName
	UInt32					clumpID;			// only valid if kAudioUnitParameterFlag_HasClump
	CFStringRef				cfNameString;		// only valid if kAudioUnitParameterFlag_HasCFNameString

	AudioUnitParameterUnit	unit;						
	Float32					minValue;			
	Float32					maxValue;			
	Float32					defaultValue;		
	UInt32					flags;				
} AudioUnitParameterInfo;
#endif




// Here are some temporary macros that will get and set the unitName into an existing 
// Panther or earlier baased AudioUnitParameterInfo struct
#define TEMP_SET_PARAMETER_UNIT_NAME(paramInfo, unitName)	\
	*(CFStringRef*)(&(paramInfo.name[52])) = unitName


#define TEMP_GET_PARAMETER_UNIT_NAME(paramInfo)	\
	*((CFStringRef*)(&(paramInfo.name[52])))





// ADDITIONAL PARAMETER INFO Parameter Type
	// this is the parameter unit type for parameters that present a custom unit name
enum {
	kAudioUnitParameterUnit_CustomUnit			= 26
};






// ADDITIONAL PARAMETER INFO FLAGS
	// some standard additional display curves that can be applied to parameters
		// DisplayLogarithmic is defined in Panther.
		// additional display flags will be defined with macros and a bit field of the
		// param info flags field.
	
	// The following macros will eventually appear in AudioUnitProperties.h for getting and setting one of these display flags

	// bit positions 18,17,16 are set aside for display scales. bit 19 is reserved.
enum {
	kAudioUnitParameterFlag_DisplayMask			= (7L << 16) | (1L << 22),
	kAudioUnitParameterFlag_DisplaySquareRoot	= (1L << 16),
	kAudioUnitParameterFlag_DisplaySquared		= (2L << 16),
	kAudioUnitParameterFlag_DisplayCubed		= (3L << 16),
	kAudioUnitParameterFlag_DisplayCubeRoot		= (4L << 16),
	kAudioUnitParameterFlag_DisplayExponential	= (5L << 16),


	// this renames the existing parameter flag _HasName, to _ValuesHaveStrings to be clearer on its intended usage.
	kAudioUnitParameterFlag_ValuesHaveStrings = kAudioUnitParameterFlag_HasName 
};


#define GetAudioUnitParameterDisplayType(flags) \
	((flags) & kAudioUnitParameterFlag_DisplayMask)

#define AudioUnitDisplayTypeIsLogarithmic(flags) \
	(GetAudioUnitParameterDisplayType(flags) == kAudioUnitParameterFlag_DisplayLogarithmic)
	
#define AudioUnitDisplayTypeIsSquareRoot(flags) \
	(GetAudioUnitParameterDisplayType(flags) == kAudioUnitParameterFlag_DisplaySquareRoot)
	
#define AudioUnitDisplayTypeIsSquared(flags) \
	(GetAudioUnitParameterDisplayType(flags) == kAudioUnitParameterFlag_DisplaySquared)
	
#define AudioUnitDisplayTypeIsCubed(flags) \
	(GetAudioUnitParameterDisplayType(flags) == kAudioUnitParameterFlag_DisplayCubed)

#define AudioUnitDisplayTypeIsCubeRoot(flags) \
	(GetAudioUnitParameterDisplayType(flags) == kAudioUnitParameterFlag_DisplayCubeRoot)
	
#define AudioUnitDisplayTypeIsExponential(flags) \
	(GetAudioUnitParameterDisplayType(flags) == kAudioUnitParameterFlag_DisplayExponential)

#define SetAudioUnitParameterDisplayType(flags, displayType) \
	(((flags) & ~kAudioUnitParameterFlag_DisplayMask) | (displayType))




#pragma mark -
#pragma mark A new property used to tell an AU that it is rendering in an offline context
#pragma mark -
/*
	Both AU Instruments and Effects are by design expected to be used in a real-time context.
	However, in some usage scenarios - Logic's "Freeze Track" functionality or exporting to a file 
	- some AU's may need to adapt their normal or RT modus operandi.

	As an example - the DLS Synth currently has two features that can't be "on" when rendering 
	in a non-real time (lets call this Offline Rendering). 
	One is disk streaming, where the disk streaming timing is based around the semantic of an AU 
	rendering in a real-time situation. The other is the application of note-dropping, which 
	relies on an estimation of time passed with no preemption, etc.

	Thus, the introduction of a new property to describe to an AU that it is rendering in
	an offline context, so that its behaviour can be adjusted accordingly if needed.
	
	When set to 1, the AU is expected to set itself in state where it can render 
	in an offline situation (thus for the DLS Synth we'd turn off disk streaming 
	if it was on, and *not* do note-dropping if that were on)
	
	When set back to 0, the AU is expected to restore any previous settings it had 
	changed to go into the offline case (if these haven't been changed in the interim of course).
	
	Whilst offline rendering can change some state within an AU that a user may set, 
	we *don't* expect that state change to be reflected to the user - thus for the DLS synth, 
	"disk streaming" would still be considered to be on (even though we temporarily turn it off), 
	and when the AU's render state is reset, disk streaming would again be active.
	
	The default value of this property is 0 (ie. OfflineRendering is off) and must be explicitly enabled.
*/

enum {
		// The property is set in Global Scope, its value is a UInt32 (0 or 1) and is both read/write
	kAudioUnitProperty_OfflineRender                                = 37
};

#pragma mark -
#pragma mark Transport State Host Callback Definition
#pragma mark -

typedef OSStatus (*HostCallback_GetTransportState) (void 	*inHostUserData,
										Boolean 			*outIsPlaying,
										Boolean 			*outTransportStateChanged,
										Float64 			*outCurrentSampleInTimeLine,
										Boolean 			*outIsCycling,
										Float64 			*outCycleStartBeat,
										Float64 			*outCycleEndBeat);

#if 0
// this struct definition will conflict with existing definition in AudioUnitProperties.h
// However, this is how this will look in a future version of that header.
typedef struct HostCallbackInfo {
	void *									hostUserData;
	HostCallback_GetBeatAndTempo 			beatAndTempoProc;
    HostCallback_GetMusicalTimeLocation     musicalTimeLocationProc;
	HostCallback_GetTransportState			transportStateProc;
} HostCallbackInfo;
#endif

// in the meantime use this temporary definition
typedef struct TempHostCallbackInfo {
	void *									hostUserData;
	HostCallback_GetBeatAndTempo 			beatAndTempoProc;
    HostCallback_GetMusicalTimeLocation     musicalTimeLocationProc;
	HostCallback_GetTransportState			transportStateProc;
} TempHostCallbackInfo;

#pragma mark -
#pragma mark Translating to an AU from an existing/other plug-in format
#pragma mark -

enum {
	kAudioUnitMigrateProperty_FromPlugin		= 4000,
	kAudioUnitMigrateProperty_OldAutomation		= 4001
};


enum {
	kOtherPluginFormat_Undefined	= 0, //we're reserving this value for future use
	kOtherPluginFormat_kMAS			= 1,
	kOtherPluginFormat_kVST			= 2
};


#include <CoreAudio/CoreAudioTypes.h>
/*
struct AudioClassDescription {
    OSType mType;
    OSType mSubType;
    OSType mManufacturer;
};
is defined in <CoreAudio/CoreAudioTypes.h>

How these fields are used varies for each format.
In general, 
	mType specifies a generic, plugin format defined descriptor, 
	mSubType is usually left to the manufacturer to use at their discretion
	mManufacturer is a registered code to identify all plugins from the same manufacturer
*/
typedef struct  AudioUnitOtherPluginDesc
{
	UInt32 						format; //kOtherPluginFormat_kMAS, etc
	AudioClassDescription		plugin;
} AudioUnitOtherPluginDesc;


// value of this AUPreset dictionary entry is discussed in docs
// "vstdata" is already defined in Panther
#define kAUPresetMASDataKey			"masdata"

#define kAUPresetExternalFileRefs	"file-references"


typedef struct AudioUnitParameterValueTranslation 
{
	AudioUnitOtherPluginDesc	otherDesc;
	UInt32						otherParamID;
	Float32						otherValue;
	AudioUnitParameterID		auParamID;
	Float32						auValue;
} AudioUnitParameterValueTranslation;


// AU-MAS Specific Structs:
typedef struct AudioUnitPresetMAS_SettingData
{
	UInt32 				isStockSetting; // zero or 1  i.e. "long bool"
	UInt32 				settingID;
	UInt32 				dataLen; //length of following data
	UInt8 				data[1];
} AudioUnitPresetMAS_SettingData;

typedef struct AudioUnitPresetMAS_Settings
{
	UInt32 							manufacturerID;
	UInt32 							effectID;
	UInt32 							variantID;
	UInt32 							settingsVersion;
	UInt32 							numberOfSettings;
	AudioUnitPresetMAS_SettingData 	settings[1];
} AudioUnitPresetMAS_Settings;


#endif // __AUPropertiesPostPantherAdditions__H__
