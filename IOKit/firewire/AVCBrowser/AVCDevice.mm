/*
	File:		AVCBrowser/AVCDevice.m

	Copyright: 	© Copyright 2001-2002 Apple Computer, Inc. All rights reserved.

	Written by: wgulland
	
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

	$Log: AVCDevice.mm,v $
	Revision 1.7  2005/10/12 16:03:56  ayanowit
	Updated for Tumeric Dev Tools release.
	
	Revision 1.6  2005/02/03 00:39:04  firewire
	*** empty log message ***
	
	Revision 1.5  2004/07/12 18:23:08  ayanowit
	Added support for EIA-775 descriptor parsing on supported devices.
	
	Revision 1.4  2004/05/18 22:06:14  niels
	*** empty log message ***
	
	Revision 1.3  2003/04/25 20:57:30  firewire
	Added new features to PlugBrowserController
	
	Revision 1.2  2003/03/20 22:49:48  firewire
	No log message.
	
	Revision 1.1  2003/03/18 00:25:01  firewire
	SDK15 fixes; nib updates, device unplug
	
	Revision 1.3  2002/11/14 22:22:47  noggin
	fix minor crasher
	
	Revision 1.2  2002/11/08 22:46:47  firewire
	first checkin of install project
	
*/

#import "AVCDevice.h"
#import "PlugBrowserController.h"
#import <IOKit/avc/IOFireWireAVCConsts.h>
#import <IOKit/avc/IOFireWireAVCLib.h>

#import "AVCBrowserGlobals.h"

const NSString* AVCMessageName = @"avc message" ;

static IOCFPlugInInterface** findPluginForDevice(CFNumberRef guid, CFStringRef serviceType, CFUUIDRef pluginType)
{
    // Find device via GUID
    IOReturn result = kIOReturnSuccess ;
    io_object_t device = 0;
    CFMutableDictionaryRef	dict = 0;
    io_iterator_t	enumerator = 0;
    IOCFPlugInInterface** theCFPlugInInterface = NULL;
    do {
        dict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
    
        if(!dict)
            break;
    
        CFDictionarySetValue( dict, CFSTR(kIOProviderClassKey), serviceType );
        CFDictionarySetValue( dict, CFSTR("GUID"), guid);
        
        result = IOServiceGetMatchingServices(
                    gMasterDevicePort,
                    dict,
                    & enumerator );
    
        if( kIOReturnSuccess != result ) {
            NSLog(@"IOServiceGetMatchingServices = %x\n", result);
            break;
        }
        
        // Should be just one device with a matching GUID!!
        device = IOIteratorNext(enumerator);
#ifdef DEBUG		
        NSLog(@"Matching device is %x\n", device);
#endif
        if(device) {
            SInt32					theScore ;

            result = IOCreatePlugInInterfaceForService(
                            device,
                            pluginType,
                            kIOCFPlugInInterfaceID,		//interfaceType,
                            & theCFPlugInInterface, 
                            & theScore);
        }

    } while ( false ) ;
	
    // we're done with the enumerator, so release it.
    if(result != kIOReturnSuccess)
        NSLog(@"Error %x connecting to device\n", result);
    if (enumerator)
        IOObjectRelease(enumerator) ;
    return theCFPlugInInterface;
}

static void avcMessage(void * refcon, UInt32 messageType, void * messageArgument)
{
    printf("Got AVC message: refcon %p, type 0x%lx arg %p\n",
        refcon, messageType, messageArgument);
	
	NSDictionary* info = [ NSDictionary dictionaryWithObjectsAndKeys:
			[ NSNumber numberWithUnsignedLong:messageType ], @"messageType"
			,[ NSNumber numberWithUnsignedLong:(unsigned long)messageArgument ], @"messageArgment"
			,nil ] ;
	[ (NSNotificationCenter*)[ NSNotificationCenter defaultCenter ] postNotificationName:[ (NSString*)AVCMessageName mutableCopy ] 
			object:(id)refcon userInfo:info ] ;
}

@implementation AVCDevice
+(AVCDevice *)withIOService:(io_object_t)obj
{
    IOReturn 								result;
    UInt64 									newGUID 				= 0;
    UInt32 									type;
    CFMutableDictionaryRef 					properties;
    AVCDevice *								theDevice 				= [[self alloc] init];
    NSString *								name 					= [NSString stringWithFormat:@"Device %x", obj];
    IOFireWireLibDeviceRef					resultInterface 		= 0 ;
    IOFireWireAVCLibUnitInterface **		avcInterface 			= 0;
    IOFireWireSessionRef 					session;
    
    // Get name and GUID
    result = IORegistryEntryCreateCFProperties(obj, & properties, kCFAllocatorDefault, kNilOptions);

    if(result == noErr)
	{    
        CFNumberRef 			dataDesc;
        CFNumberRef 			typeDesc;
        CFStringRef 			nameDesc;
        IOCFPlugInInterface** 	theCFPlugInInterface ;
		
        dataDesc = (CFNumberRef)CFDictionaryGetValue(properties, CFSTR("GUID"));
        CFNumberGetValue(dataDesc, kCFNumberSInt64Type, &newGUID);
		
		// Save the GUID as a 64-bit value in this object
		[theDevice setGuidVal:newGUID];

        typeDesc = (CFNumberRef)CFDictionaryGetValue(properties, CFSTR("Unit_Type"));
        CFNumberGetValue(typeDesc, kCFNumberSInt32Type, &type);
		
        nameDesc = (CFStringRef)CFDictionaryGetValue(properties, CFSTR("FireWire Product Name"));
        if(nameDesc)
		{
            name = [NSString stringWithCString:CFStringGetCStringPtr(nameDesc, kCFStringEncodingMacRoman)];
        }

        //
		// Find AVC user client
		//
		
        theCFPlugInInterface = findPluginForDevice(dataDesc, CFSTR("IOFireWireAVCUnit"), 
				kIOFireWireAVCLibUnitTypeID);
        if (theCFPlugInInterface) 
		{
            HRESULT err;
            err = (*theCFPlugInInterface)->QueryInterface(
                                                theCFPlugInInterface, 
                                                CFUUIDGetUUIDBytes(kIOFireWireAVCLibUnitInterfaceID), 
                                                (void**) & avcInterface);
            if (err == S_OK) {
                UInt32 size;
                UInt8 cmd[8],response[8];

                cmd[kAVCCommandResponse] = kAVCStatusInquiryCommand;
                cmd[kAVCAddress] = kAVCUnitAddress;
                cmd[kAVCOpcode] = kAVCUnitInfoOpcode;
                cmd[3] = cmd[4] = cmd[5] = cmd[6] = cmd[7] = 0xff;
                size = 8;
                
                result = (**avcInterface).open(avcInterface);

                if(result != kIOReturnSuccess)
                    NSLog(@"Error %x opening to device\n", result);

                (**avcInterface).addCallbackDispatcherToRunLoop( avcInterface, CFRunLoopGetCurrent() ) ;
                (**avcInterface).setMessageCallback( avcInterface, theDevice, avcMessage);

                result = (**avcInterface).AVCCommand( avcInterface, cmd, size, response, &size ) ;

                if(result != kIOReturnSuccess)
				{
                    NSLog( @"Error %x sending command to device\n", result );
				}
                else 
				{
#ifdef DEBUG
                    printf("Returned %ld:", size);
#endif
                    for( unsigned i = 0 ; i < size ; ++i)
                        printf("%x ", response[i]);
                    printf("\n");
                }

            }
            else
                result = err;
        }
        // Find device via GUID
        resultInterface = (IOFireWireLibDeviceRef)((**avcInterface).getAncestorInterface( avcInterface, "IOFireWireUnit",
            CFUUIDGetUUIDBytes( kIOFireWireLibTypeID ), CFUUIDGetUUIDBytes( kIOFireWireDeviceInterfaceID ) )) ;
        if ( resultInterface )
		{
			(**resultInterface).AddCallbackDispatcherToRunLoop( resultInterface, CFRunLoopGetCurrent() ) ;
			session = (**avcInterface).getSessionRef( avcInterface );
			result = (**resultInterface).OpenWithSessionRef( resultInterface, session ) ;
#ifdef DEBUG
			printf( "IOFireWireUnit open returned %x\n", result ) ;
#endif
        }
        
        ::CFRelease( properties ) ;
    }
    
    
    [theDevice setName:name];
    switch (type)
	{
        case kAVCTapeRecorder:
            [theDevice setType:@"VCR"];
            break;
        case kAVCVideoCamera:
            [theDevice setType:@"Camera"];
            break;
        case kAVCVendorUnique:
            [theDevice setType:@"Vendor Unique"];
            break;
        default:
            [theDevice setType:[NSString stringWithFormat:@"Type-%d", type]];
            break;
    }

    [theDevice setGuid:[NSString stringWithFormat:@"%qx", newGUID]];

    theDevice->_ioService = obj;
    theDevice->_device = (**resultInterface).GetDevice(resultInterface);
    theDevice->_interface = resultInterface;
    theDevice->_avcInterface = avcInterface;
    
    if(resultInterface) {
        FWAddress addr;
        UInt32 master;
        addr.addressHi = 0xffff;
        addr.addressLo = 0xf0000900;
        master = [theDevice readQuad:addr];
        [theDevice setOutMax: [NSNumber numberWithUnsignedInt:(1 << (master >> 30)) * 100]];
        [theDevice setOutBase: [NSNumber numberWithUnsignedInt:(master >> 24 & 0x3f)]];
        [theDevice setOutNum: [NSNumber numberWithUnsignedInt:master & 0x1f]];
        addr.addressLo = 0xf0000980;
        master = [theDevice readQuad:addr];
        [theDevice setInMax: [NSNumber numberWithUnsignedInt:(1 << (master >> 30)) * 100]];
        [theDevice setInBase: [NSNumber numberWithUnsignedInt:(master >> 24 & 0x3f)]];
        [theDevice setInNum: [NSNumber numberWithUnsignedInt:master & 0x1f]];
    }

    [theDevice autorelease];

	printf("made AVCDevice %p\n", theDevice) ;

    return theDevice;
}    

- (void)release
{
	printf("release avcDevice %p, retainCount=%u\n", self, [ self retainCount ] ) ;
	assert( [ self retainCount ] > 0 ) ;
	[ super release ] ;
}

- (void)dealloc
{
	printf("AVCDevice :: dealloc self=%p\n", self) ;
	
	(**_interface).Close( _interface ) ;
	(**_interface).Release( _interface ) ;
	
	(**_avcInterface).removeCallbackDispatcherFromRunLoop( _avcInterface ) ;
	(**_avcInterface).close( _avcInterface ) ;
	(**_avcInterface).Release( _avcInterface ) ;
	
	[ super dealloc ] ;
}

-(NSString *)name
{
    return _name;
}

-(void) setName:(NSString *)val
{
    [_name autorelease];
    _name = [ val retain ] ;
}

-(NSString *)type
{
    return _type;
}

-(void) setType:(NSString *)val
{
    [_type autorelease];
    _type = [ val retain ] ;
}

-(NSString *)guid
{
    return _guid;
}

-(void) setGuid:(NSString *)val
{
    [val retain];
    [_guid release];
    _guid = val;
}

-(void) setGuidVal:(UInt64)val
{
	_guidVal = val;
}

-(UInt64) guidVal
{
	return _guidVal;
}

-(NSNumber *)outMax
{
    return _outMax;
}

-(void) setOutMax:(NSNumber *)val
{
    [val retain];
    [_outMax release];
    _outMax = val;
}

-(NSNumber *)inMax
{
    return _inMax;
}

-(void) setInMax:(NSNumber *)val
{
    [val retain];
    [_inMax release];
    _inMax = val;
}

-(NSNumber *)outNum
{
    return _outNum;
}

-(void) setOutNum:(NSNumber *)val
{
    [val retain];
    [_outNum release];
    _outNum = val;
}

-(NSNumber *)inNum
{
    return _inNum;
}

-(void) setInNum:(NSNumber *)val
{
    [val retain];
    [_inNum release];
    _inNum = val;
}


-(NSNumber *)outBase
{
    return _outBase;
}

-(void) setOutBase:(NSNumber *)val
{
    [val retain];
    [_outBase release];
    _outBase = val;
}

-(NSNumber *)inBase
{
    return _inBase;
}

-(void) setInBase:(NSNumber *)val
{
    [val retain];
    [_inBase release];
    _inBase = val;
}

-(io_object_t)ioService
{
    return _ioService;
}

-(UInt32)readQuad:(FWAddress&)addr
{
	UInt32 val = EndianU32_NtoB(0xdeadbabe);
    IOReturn status;

    status = (**_interface).ReadQuadlet(_interface, _device, &addr, &val, kFWDontFailOnReset, 0);
#ifdef DEBUG
    NSLog(@"Reading from 0x%x:0x%x = 0x%x, res %x\n", addr.addressHi, addr.addressLo, EndianU32_BtoN(val), status);
#endif
    return EndianU32_BtoN(val);
}

-(IOReturn)compareSwap:(FWAddress &)addr withOld:(UInt32)oldVal andNew:(UInt32)newVal
{
	return (**_interface).CompareSwap(_interface, _device, &addr, EndianU32_NtoB(oldVal), EndianU32_NtoB(newVal), kFWDontFailOnReset, 0);
}

-(void)open
{
    PlugBrowserController *control = [ [ PlugBrowserController withDevice:self ] retain ] ;
    [ NSBundle loadNibNamed:@"PlugBrowser" owner:control ] ;
}

-(IOFireWireAVCLibUnitInterface **) getAVCInterface
{
	return _avcInterface;
}

@end
