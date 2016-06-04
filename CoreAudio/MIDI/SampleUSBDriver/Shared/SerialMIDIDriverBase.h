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
	SerialMIDIDriverBase.cpp
	
=============================================================================*/

#ifndef __SerialMIDIDriverBase_h__
#define __SerialMIDIDriverBase_h__

#include "MIDIDriverClass.h"	
#include "MIDIDriverUtils.h"

#include <vector>
#include <list>

// _________________________________________________________________________________________

#define kSerDevProperty_SerialPort		CFSTR("serial.port")
#define kSerDevProperty_ExtClockDivide	CFSTR("serial.clockDivide")
#define kSerDevProperty_NumberF5Ports	CFSTR("serial.numberF5Ports")

// _________________________________________________________________________________________
// from IOKit/serial/ioss.h
/*
 * External clock baud rates, for use with cfsetospeed
 */
#define _MAKE_EXT(x)	(((x) << 1) | 1)
#define BEXT1	    	_MAKE_EXT(1)
#define BEXT2	    	_MAKE_EXT(2)
#define BEXT4	    	_MAKE_EXT(4)
#define BEXT8	    	_MAKE_EXT(8)
#define BEXT16	    	_MAKE_EXT(16)
#define BEXT32	    	_MAKE_EXT(32)
#define BEXT64	    	_MAKE_EXT(64)
#define BEXT128	    	_MAKE_EXT(128)
#define BEXT256	    	_MAKE_EXT(256)

// AppleSCCSerial.kext uses a different set of external baud rate constants
#define k1XClock_ForAppleKext	1
#define k32XClock_ForAppleKext	51


/*
 * Sets the receive latency (in microseconds) with the default
 * value of 0 meaning a 256 / 3 character delay latency.
 */
#define IOSSDATALAT    _IOW('T', 0, unsigned long)

// _________________________________________________________________________________________

class SerialDevice;
class SerialMIDIDeviceManager;
class SerialMIDIDevice;

// _________________________________________________________________________________________
// SerialMIDIDriverBase
//
// MIDIDriver subclass, derive your Serial MIDI driver from this
class SerialMIDIDriverBase : public MIDIDriver {
public:
						SerialMIDIDriverBase(CFUUIDRef factoryID, CFStringRef driverIdentifier);
	virtual				~SerialMIDIDriverBase();
	
	// overrides of MIDIDriver virtual methods
	virtual OSStatus	Start(				MIDIDeviceListRef devices);
	virtual OSStatus	Stop();

	virtual OSStatus	Send(				const MIDIPacketList *pktlist,
											void *			endptRef1,
											void *			endptRef2);

	// our own abstract methods - required overrides

	virtual bool			MatchDevice(	SerialDevice *	inSerialDevice) = 0;
								// given a serial device return a boolean saying 
								// whether this driver wants to control this device

	virtual MIDIDeviceRef	CreateDevice (	SerialDevice *	inSerialDevice) = 0;

	virtual void			PreExistingDeviceFound(	MIDIDeviceRef	inMIDIDevice,
													SerialDevice *	inSerialDevice) { }

	virtual void			StartInterface(		SerialMIDIDevice *intf) = 0;
								// Do any extra initialization (send config msgs etc)
							
	virtual void			StopInterface(		SerialMIDIDevice *intf) = 0;
								// Do any preliminary cleanup
							
	virtual SerialMIDIDevice *	CreateSerialMIDIDevice(	SerialDevice *	inUSBDevice,
														MIDIDeviceRef	inMIDIDevice);
								// may override to create a subclass
	
	CFStringRef				GetIdentifier() { return mDriverIdentifier; }

private:
	CFStringRef						mDriverIdentifier;
	SerialMIDIDeviceManager *		mDeviceManager;
};

#endif // __SerialMIDIDriverBase_h__
