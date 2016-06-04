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
/*==================================================================================================
	ARDevice.cpp

==================================================================================================*/

//==================================================================================================
//	Includes
//==================================================================================================

//	Self Include
#include "ARDevice.h"

//	Local Includes
#include "ARDebug.h"
#include "AREngine.h"

//	System Includes
#include <IOKit/audio/IOAudioDefines.h>

//==================================================================================================
//	Constants
//==================================================================================================

#define AUDIO_ENGINE_KEY	"AudioEngine"

//==================================================================================================
//	ARDevice
//==================================================================================================

OSDefineMetaClassAndStructors(ARDevice, IOAudioDevice)

bool ARDevice::initHardware(IOService* inProvider)
{
	bool theAnswer = false;
	
	if(IOAudioDevice::initHardware(inProvider))
	{
		//	set up some basic stuff about the device
		setDeviceName("DeviceName");
		setDeviceShortName("DeviceShortName");
		setManufacturerName("ManufacturerName");
		setDeviceModelName("Audio_Reflector");
		setDeviceTransportType('virt');
		setDeviceCanBeDefault(0);
		
		// Setting this flag causes the HAL to use the strings passed to it as keys into the Localizable.strings file.
#if	AR_Debug
		//	for debugging, use the location in the build results
		setProperty (kIOAudioDeviceLocalizedBundleKey, "../../../Path/To/Your/BuildProducts/AudioReflectorDriver.kext");
#else
		setProperty (kIOAudioDeviceLocalizedBundleKey, "AudioReflectorDriver.kext");
#endif
		
		theAnswer = CreateAudioEngine();
	}
	
	return theAnswer;
}

bool ARDevice::CreateAudioEngine()
{
	//	initialize some locals
	AREngine* theEngine = NULL;
	
	//	set the return value
	bool theAnswer = true;
	
	//	get the description of th engine to build
	OSDictionary* theEngineDictionary = OSDynamicCast(OSDictionary, getProperty(AUDIO_ENGINE_KEY));
	FailIfNULLWithAction(theEngineDictionary, theAnswer = false, Done, "ARDevice::CreateAudioEngines: couldn't get the engine description");
	
	//	create the engine
	theEngine = new AREngine;
	if(theEngine != NULL)
	{
		//	initialize the engine
		if(theEngine->init(theEngineDictionary))
		{
			//	activate the new engine, if initialization was successful
			activateAudioEngine(theEngine);
		}
		
		//	release the engine
		theEngine->release();
	}
	
Done:
	return theAnswer;
}
