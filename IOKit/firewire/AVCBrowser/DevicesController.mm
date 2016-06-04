/*
	File:		AVCBrowser/DevicesController.m

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

	$Log: DevicesController.mm,v $
	Revision 1.4  2005/02/03 00:39:04  firewire
	*** empty log message ***
	
	Revision 1.3  2004/05/18 22:06:14  niels
	*** empty log message ***
	
	Revision 1.2  2003/03/18 01:03:11  firewire
	*** empty log message ***
	
	Revision 1.1  2003/03/18 00:25:02  firewire
	SDK15 fixes; nib updates, device unplug
	
	Revision 1.3  2002/11/14 22:22:47  noggin
	fix minor crasher
	
	Revision 1.2  2002/11/08 22:46:47  firewire
	first checkin of install project
	
*/

#import "DevicesController.h"
#import "AVCDevice.h"
#import <IOKit/avc/IOFireWireAVCConsts.h>
#import <IOKit/avc/IOFireWireAVCLib.h>
#import "AVCBrowserGlobals.h"

// Structure for stashing bus stuff
typedef struct _BusInfo {
    IOCFPlugInInterface** 					theCFPlugInInterface;	// to the AVC protocol object
    IOFireWireAVCLibProtocolInterface **	theAVCInterface;
} BusInfo;

mach_port_t gMasterDevicePort;
IONotificationPortRef gNotifyPort;
CFRunLoopSourceRef gNotifySource;

static void avcMessage(void * refcon, UInt32 messageType, void * messageArgument)
{
//#ifdef DEBUG
    printf("Got AVCProtocol message: refcon %p, type 0x%lx arg %p\n",
        refcon, messageType, messageArgument);
//#endif
}


static void serviceMatchingCallback(void * refcon,	io_iterator_t iterator)
{
    DevicesController *control = (DevicesController *)refcon;
    io_object_t		newDevice;

	// add devices on the bus to our list
    // Get devices into device array.
#ifdef DEBUG
    printf("notification %x arrived \n", iterator);
#endif
    while((newDevice = IOIteratorNext(iterator)) )
	{
#ifdef DEBUG
        printf("device %x arrived \n", newDevice);
#endif
		id descObj = [AVCDevice withIOService:newDevice];
        if(descObj)
		{
            [control addDevice:descObj];
        }
    }
}

static IOReturn avcRequest(void *refCon, UInt32 generation, UInt16 srcNodeID,
                const UInt8 * command, UInt32 cmdLen, UInt8 * response, UInt32 *responseLen)
{
#ifdef DEBUG
    UInt32 i;
    printf("got AVC request, gen %ld, source %x, len %ld\n",
        generation, srcNodeID, cmdLen);
    for(i=0; i<cmdLen; i++)
        printf("%x ", command[i]);
    printf("\n");
#endif
	
	return kIOReturnSuccess ;
}

static void writePlug(void *refcon, UInt32 generation, UInt16 nodeID, UInt32 plug,
                                                                    UInt32 oldVal, UInt32 newVal)
{
#ifdef DEBUG
    printf("got plug write, gen %ld, source %x, plug %ld, oldval %lx newval %lx\n",
        generation, nodeID, plug, oldVal, newVal);
#endif
}

@implementation DevicesController
- init
{
	io_iterator_t	enumerator;
	kern_return_t	result;
    io_object_t		newDevice;
    NSMutableArray *devs;

    [super init];

    devs = [NSMutableArray arrayWithCapacity:1];
    _devices = [devs retain];
    _busses = [NSMutableSet setWithCapacity:1];
    [_busses retain];
    
	// get mach master port
	result = IOMasterPort(bootstrap_port, & gMasterDevicePort) ;
	if ( result != kIOReturnSuccess )
	{
		NSLog(@"IOMasterPort failed: 0x", result);
        return self;
	}
	
    do {
        // get a registry enumerator for all "IOFireWireLocalNode" devices
        result = IOServiceGetMatchingServices(
                    gMasterDevicePort,
                    IOServiceMatching( "IOFireWireLocalNode" ),
                    & enumerator );
    
        if( kIOReturnSuccess != result ) {
            NSLog(@"IOServiceGetMatchingServices = %x\n", result);
            break;
        }
        
        while((newDevice = IOIteratorNext(enumerator)) ) {
            SInt32					theScore ;
            IOCFPlugInInterface** 	theCFPlugInInterface = NULL;
            // Get IOFireWireAVCLibProtocolInterface to play with local PCRs and receive AVC requests.
            result = IOCreatePlugInInterfaceForService(
                            newDevice,
                            kIOFireWireAVCLibProtocolTypeID,
                            kIOCFPlugInInterfaceID,		//interfaceType,
                            & theCFPlugInInterface, 
                            & theScore);
#ifdef DEBUG
            NSLog(@"IOCreatePlugInInterfaceForService (protocol) = %x, prot = %p\n", result, theCFPlugInInterface);
#endif
            if(theCFPlugInInterface) {
                BusInfo	busInfo;
                NSData *data;
                UInt32 inputPlug;
                UInt32 outputPlug;
                IOFireWireAVCLibProtocolInterface **	avcInterface = 0 ;
                result = (*theCFPlugInInterface)->QueryInterface(
                                                theCFPlugInInterface, 
                                                CFUUIDGetUUIDBytes(kIOFireWireAVCLibProtocolInterfaceID), 
                                                (void**) & avcInterface);
                if(avcInterface) {
                    (**avcInterface).setMessageCallback(avcInterface, self, avcMessage);
                    result = (**avcInterface).setAVCRequestCallback(avcInterface, kAVCVideoCamera, 0, self, avcRequest);
#ifdef DEBUG
                    printf("Result %x\n", result);
#endif
                    if((*avcInterface)->allocateInputPlug) {
                        result = (*avcInterface)->allocateInputPlug(avcInterface, self, writePlug, &inputPlug);
#ifdef DEBUG
                        printf("Result %x, input plug %ld\n", result, inputPlug);
#endif
                    }
                    if((*avcInterface)->readInputPlug) {
                        UInt32 val;
                        val = (*avcInterface)->readInputPlug(avcInterface, inputPlug);
#ifdef DEBUG
                        printf("plug %ld = %lx\n", inputPlug, val);
#endif
                        if((*avcInterface)->updateInputPlug)
                            (*avcInterface)->updateInputPlug(avcInterface, inputPlug, val, val+1);
                        val = (*avcInterface)->readInputPlug(avcInterface, inputPlug);
#ifdef DEBUG
                        printf("plug now %ld = %lx\n", inputPlug, val);
#endif
                    }
                    if((*avcInterface)->freeInputPlug)
                        (*avcInterface)->freeInputPlug(avcInterface, inputPlug);
                    if((*avcInterface)->allocateOutputPlug) {
                        result = (*avcInterface)->allocateOutputPlug(avcInterface, self, writePlug, &outputPlug);
#ifdef DEBUG
                        printf("Result %x, output plug %ld\n", result, outputPlug);
#endif
                    }
                    if((*avcInterface)->readOutputPlug)
                        (*avcInterface)->readOutputPlug(avcInterface, outputPlug);
                    (*avcInterface)->addCallbackDispatcherToRunLoop(avcInterface, CFRunLoopGetCurrent());
                }                                
                busInfo.theCFPlugInInterface = theCFPlugInInterface;
                busInfo.theAVCInterface = avcInterface;
                //(*theCFPlugInInterface)->Stop(theCFPlugInInterface);
                //(*testInterface)->Release(testInterface);
                data = [NSData dataWithBytes:&busInfo length:sizeof(BusInfo)];
                [_busses addObject:data];    
            }
        }
    
        // we're done with the enumerator, so release it.
        if (enumerator)
            IOObjectRelease(enumerator) ;
	} while(0);
	
	// get a registry enumerator for all "IOFireWireAVCUnit" devices
    gNotifyPort = IONotificationPortCreate(gMasterDevicePort);
    gNotifySource = IONotificationPortGetRunLoopSource(gNotifyPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), gNotifySource, kCFRunLoopDefaultMode);

    result = IOServiceAddMatchingNotification(
                gNotifyPort,
                kIOMatchedNotification,
                IOServiceMatching( "IOFireWireAVCUnit" ),
                serviceMatchingCallback, self,
                & enumerator );

    if( kIOReturnSuccess != result )
	{
        NSLog(@"IOServiceGetMatchingServices = %x\n", result);
        return self;
	}
	
    serviceMatchingCallback(self, enumerator);
    
	// we need to watch device removed notes
	[ (NSNotificationCenter*)[ NSNotificationCenter defaultCenter ] 
		addObserver:self 
		selector:@selector(message:) 
		name:[ (NSString*)AVCMessageName mutableCopy ] 
		object:nil ] ;

    return self;
}

- (void) addDevice:(AVCDevice *)device
{
    [_devices addObject:device];

    [_devList reloadData];
    [_devCount setIntValue:[ _devices count ] ];
}

- (void) removeDevice:(AVCDevice *)device
{
	printf("remove device %p\n", device) ;
	[_devices removeObjectAtIndex:[ _devices indexOfObject:device ] ] ;

    [_devList reloadData];
    [_devCount setIntValue:[ _devices count ]];	
}

- (void)awakeFromNib
{
    [_devCount setIntValue:0];
}

- (int)numberOfRowsInTableView:(NSTableView *)aView
{
    return [ _devices count ] ;
}

- (id)tableView:(NSTableView *)aView 
    objectValueForTableColumn:(NSTableColumn *)aCol row:(int)index
{
    NSString *key = [aCol identifier];
    return [[_devices objectAtIndex:index] valueForKey:key];
}
#if 0
static int compareKeys(id a, id b, void *context)
{
    NSString *key = (NSString *)context;
    id valA = [a valueForKey:key];
    id valB = [b valueForKey:key];
    
    return [valA compare:valB];
}
#endif
- (IBAction)openDevice:(id)sender
{
    int index = [_devList selectedRow];
    if(index != -1)
	{
#ifdef DEBUG
        printf("Opening device %d\n", index);
#endif
        [ [ _devices objectAtIndex:index ] open ] ;
    }
}

- (void)message:(NSNotification*)note
{
	if ( [ (NSNumber*)[ [ note userInfo ] objectForKey:@"messageType" ] unsignedLongValue ] == kIOFWMessageServiceIsRequestingClose )
		[ self removeDevice:[ note object ] ] ;
}

@end
