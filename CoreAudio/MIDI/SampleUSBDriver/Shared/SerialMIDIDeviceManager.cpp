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
	SerialMIDIDeviceManager.cpp
	
=============================================================================*/

#include "SerialMIDIDeviceManager.h"
#include <IOKit/serial/IOSerialKeys.h>
#include "SerialDevice.h"
#include "SerialMIDIDevice.h"

// _________________________________________________________________________________________
//	SerialMIDIDeviceManager::SerialMIDIDeviceManager
//
SerialMIDIDeviceManager::SerialMIDIDeviceManager(	SerialMIDIDriverBase *	driver,
													MIDIDeviceListRef	 	devList) :
	SerialDeviceManager(CFRunLoopGetCurrent()),
	mDriver(driver)
{
	// this could be moved to a utility function
	int nDevices = MIDIDeviceListGetNumberOfDevices(devList);
	
	if (driver->mVersion >= 2) {
		// mark everything previously present as offline
		for (int iDevice = 0; iDevice < nDevices; ++iDevice) {
			MIDIDeviceRef midiDevice = MIDIDeviceListGetDevice(devList, iDevice);
			MIDIObjectSetIntegerProperty(midiDevice, kMIDIPropertyOffline, true);
		}
	}
	
	ScanServices();
}

// _________________________________________________________________________________________
//	USBMIDIDeviceManager::~USBMIDIDeviceManager
//
SerialMIDIDeviceManager::~SerialMIDIDeviceManager()
{
	for (SerialMIDIDeviceList::iterator it = mDeviceList.begin(); 
	it != mDeviceList.end(); ++it) {
		SerialMIDIDevice *device = *it;
		delete device;
	}
}


// _________________________________________________________________________________________
//	SerialMIDIDeviceManager::ServiceTerminated
//
//	Called when the device is unplugged.
void	SerialMIDIDeviceManager::ServiceTerminated(			io_service_t	terminatedService)
{
	for (SerialMIDIDeviceList::iterator it = mDeviceList.begin(); 
	it != mDeviceList.end(); ++it) {
#warning "unimplemented -- would need this for USB-to-serial adapter"
/*		
		SerialMIDIDevice *device = *it;
		if (device->mUSBDevice->GetLocationID() == terminatedLocation) {
			#if VERBOSE
				printf("shutting down removed device 0x%X\n", (int)terminatedService);
			#endif
			MIDIObjectSetIntegerProperty(device->mMIDIDevice, kMIDIPropertyOffline, true);
			delete device;
			mUSBMIDIDeviceList.erase(it);
			break;
		} */
	}
}

// _________________________________________________________________________________________
//	SerialMIDIDeviceManager::MatchDevice
//
//	Called to determine whether this driver wishes to use this device.
bool	SerialMIDIDeviceManager::MatchDevice(	SerialDevice *		serDevice)
{
	// first make sure we own this serial port
	CFStringRef portOwner;
	bool result;
	
	if (MIDIGetSerialPortOwner(serDevice->GetPortName(), &portOwner) != noErr)
		return false;
	
	result = CFEqual(portOwner, mDriver->GetIdentifier());
	
	// allow driver a chance to reject the device
	if (result)
		result = mDriver->MatchDevice(serDevice);
	
	CFRelease(portOwner);
	return result;
}

// _________________________________________________________________________________________
//	SerialMIDIDeviceManager::UseDevice
//
OSStatus	SerialMIDIDeviceManager::UseDevice(	SerialDevice *		serDevice)
{
	// Match the device that was just located with what is in the current state
	MIDIDeviceRef midiDevice = NULL;
	bool deviceInSetup = false;
	OSStatus err = noErr;
	{
		// See if it's already in the setup
		MIDIDeviceListRef curDevices = MIDIGetDriverDeviceList(mDriver->Self());
		int nDevices = MIDIDeviceListGetNumberOfDevices(curDevices);

		for (int iDevice = 0; iDevice < nDevices; ++iDevice) {
			//SInt32 prevLocation, prevVendorProduct, isOffline;
			midiDevice = MIDIDeviceListGetDevice(curDevices, iDevice);
			CFStringRef devicePort = NULL;
			if (MIDIObjectGetStringProperty(midiDevice, kSerDevProperty_SerialPort, &devicePort) == noErr) {
				deviceInSetup = CFEqual(devicePort, serDevice->GetPortName());
				CFRelease(devicePort);
				if (deviceInSetup) break;
			}
		}
		MIDIDeviceListDispose(curDevices);
	}

	if (!deviceInSetup) {
		#if VERBOSE
			printf("creating new device\n");
		#endif
		
		midiDevice = mDriver->CreateDevice(serDevice);
		require_noerr(err = MIDISetupAddDevice(midiDevice), errexit);
	} else {
		#if VERBOSE
			printf("old device found\n");
		#endif
		mDriver->PreExistingDeviceFound(midiDevice, serDevice);
	}
	
	// set device properties unconditionally
	MIDIObjectSetStringProperty(midiDevice, kSerDevProperty_SerialPort, serDevice->GetPortName());
	
	// Create a SerialMIDIDevice (or subclass), starting it for I/O
	{
		SerialMIDIDevice *ioDev = mDriver->CreateSerialMIDIDevice(serDevice, midiDevice);
		if (ioDev != NULL && ioDev->Initialize()) {
			if (mDeviceList.size() == 0)
				mDeviceList.reserve(16);
			mDeviceList.push_back(ioDev);
			MIDIObjectSetIntegerProperty(midiDevice, kMIDIPropertyOffline, false);
		}
	}
errexit:
	return err;
}
