/*
	File:		AVCBrowser/PlugBrowserController.mm

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

	$Log: PlugBrowserController.mm,v $
	Revision 1.4  2004/07/12 18:23:08  ayanowit
	Added support for EIA-775 descriptor parsing on supported devices.
	
	Revision 1.3  2003/05/23 17:38:53  firewire
	Fixed a build problem due to a missing header include.
	
	Revision 1.2  2003/04/25 20:57:31  firewire
	Added new features to PlugBrowserController
	
	Revision 1.1  2002/11/14 22:22:47  noggin
	fix minor crasher
	
	Revision 1.2  2002/11/08 22:46:48  firewire
	first checkin of install project
	
*/

#import <IOKit/IOReturn.h>
#import <IOKit/avc/IOFireWireAVCConsts.h>

#import "PlugBrowserController.h"
#import "Plug.h"

void avcCommandVerboseLog(UInt8 *command, UInt32 cmdLen, PlugBrowserController *userIntf);
void avcResponseVerboseLog(	UInt8 *response, UInt32 *responseLen, PlugBrowserController *userIntf);

@implementation PlugBrowserController

/***********************************************************************
**
**
**
*/
- (id)initWithDevice:(AVCDevice*)device
{
	if ( self != [ super init ] )
		return nil ;

	avcDevice = device;
	avcInterface = [device getAVCInterface];

	[self rereadPlugs:self];
	
	return self ;
}

/***********************************************************************
**
**
**
*/
+(PlugBrowserController *)withDevice:(AVCDevice *)device
{
    return [ [ [ self alloc ] initWithDevice:device ] autorelease ] ;
}

/***********************************************************************
**
**
**
*/
- (void)awakeFromNib
{
	unsigned int i;
	CFMutableDictionaryRef matchingDict;
	CFNumberRef GUIDDesc;
	CFNumberRef SpecIDDesc;
	CFNumberRef SWVersDesc;
	io_object_t eia775Unit ;
	UInt64 guid;
	SInt32 specID = 0x5068;
	SInt32 swVers = 0x10101;

	[ _inPlugCount setIntValue:[ _inPlugs count ] ] ;
	[ _outPlugCount setIntValue:[ _outPlugs count ] ] ;

	[subunitInfoPage setIntValue: 0];
	[outputPlugSigFmtPlugNum setIntValue: 0];
	[inputPlugSigFmtPlugNum setIntValue: 0];

	for (i=1;i<64;i++)
	{
		[inputPlugChannel addItemWithTitle:[NSString stringWithFormat:@"%d",i]];
		[outputPlugChannel addItemWithTitle:[NSString stringWithFormat:@"%d",i]];
	}

	avcLogString = [[NSMutableString stringWithCapacity:4096] retain];
	[avcLog setString:avcLogString];

	[deviceControllerWindow setTitle:[NSString stringWithFormat: @"AV/C Device: %@",[avcDevice name]]];

	// Enable or Disable EIA-775 Info Button, based on whether or not
	// this device has a EIA-775 unit directory in its config ROM
	matchingDict = IOServiceMatching("IOFireWireUnit");
	guid = [avcDevice guidVal];
	GUIDDesc = CFNumberCreate(nil,kCFNumberSInt64Type,&guid);
	
	SpecIDDesc = CFNumberCreate(nil,kCFNumberSInt32Type,&specID);
	SWVersDesc = CFNumberCreate(nil,kCFNumberSInt32Type,&swVers);
	
	CFDictionarySetValue(matchingDict, CFSTR("GUID"), GUIDDesc);
	CFDictionarySetValue(matchingDict, CFSTR("Unit_Spec_ID"), SpecIDDesc);
	CFDictionarySetValue(matchingDict, CFSTR("Unit_SW_Version"), SWVersDesc);
	eia775Unit = IOServiceGetMatchingService(kIOMasterPortDefault,matchingDict);
	if (eia775Unit)
	{
		NSLog(@"Found EIA-775 Unit Directory on Device\n");
		[eia775Info setEnabled:YES];
	}
	else
	{
		[eia775Info setEnabled:NO];
	}
	
	CFRelease(GUIDDesc);
	CFRelease(SpecIDDesc);
	CFRelease(SWVersDesc);
}

/***********************************************************************
**
**
**
*/
- (int)numberOfRowsInTableView:(NSTableView *)aView
{
    if(aView == _inPlugList)
        return [ _inPlugs count ] ;
    else
        return [ _outPlugs count ] ;
}

/***********************************************************************
**
**
**
*/
- (id)tableView:(NSTableView *)aView 
    objectValueForTableColumn:(NSTableColumn *)aCol row:(int)index
{
    NSString *key = [aCol identifier];
    //NSLog(@"key %@ index %d", key, index);
    if(aView == _inPlugList)
        return [ [ _inPlugs objectAtIndex:index ] valueForKey:key ] ;
    else
        return [ [ _outPlugs objectAtIndex:index ] valueForKey:key ] ;
}

/***********************************************************************
**
**
**
*/
- (void) addToAVCLog:(NSString*)string
{
	NSRange strRange;
	unsigned int strLen;

	[avcLogString appendString:string];
	[avcLog setString:avcLogString];

	// Set the range to point to the end of the string
	strRange.length=1;
	strLen = [avcLogString length];
	strRange.location=(strLen > 0) ? strLen-1 : 0;

	// Scroll the view to the end
	[avcLog scrollRangeToVisible:strRange];	
}

/***********************************************************************
**
**
**
*/
- (IBAction)makeInputPlugConnection:(id)sender
{
	int plug = [_inPlugList selectedRow];
	int channel = [inputPlugChannel indexOfSelectedItem];
	UInt32 oldVal;
	UInt32 newVal;
	FWAddress addr;
	IOReturn status;
	UInt32 currentCount;
	
	// Handle case where no current selection
	if (plug < 0)
	{
		plug = 0;
		[_inPlugList selectRow:0 byExtendingSelection:NO];
	}

	// Read the current value of the plug
    addr.addressHi = 0xffff;
    addr.addressLo = 0xf0000984 + plug*4;
    oldVal = [avcDevice readQuad:addr];

	currentCount = ((oldVal & 0x3F000000) >> 24);

	if (currentCount == 63)
	{
		[self addToAVCLog:[NSString stringWithFormat:@"\nError: input plug %d already at maximum connection count\n\n",plug]];
		[self rereadPlugs:self];
		return;
	}

	// Create new Value
	newVal = oldVal & 0xFFC0FFFF;
	newVal |= (channel << 16);
	newVal += (1 << 24);

	status = [avcDevice compareSwap:addr withOld:oldVal andNew:newVal];

	if (status != kIOReturnSuccess)
		[self addToAVCLog:[NSString stringWithFormat:@"\nError trying to connect to input plug %d: 0x%08X\n\n",plug,status]];
	else
		[self addToAVCLog:[NSString stringWithFormat:@"\nCompare-swap to input plug %d successful. OldVal=0x%08X NewVal=0x%08X\n\n",plug,oldVal,newVal]];	
	
	[self rereadPlugs:self];
}

/***********************************************************************
**
**
**
*/
- (IBAction)breakInputPlugConnection:(id)sender
{
	int plug = [_inPlugList selectedRow];
	UInt32 oldVal;
	UInt32 newVal;
	FWAddress addr;
	IOReturn status;
	UInt32 currentCount;

	// Handle case where no current selection
	if (plug < 0)
	{
		plug = 0;
		[_inPlugList selectRow:0 byExtendingSelection:NO];
	}

	// Read the current value of the plug
    addr.addressHi = 0xffff;
    addr.addressLo = 0xf0000984 + plug*4;
    oldVal = [avcDevice readQuad:addr];

	currentCount = ((oldVal & 0x3F000000) >> 24);

	if (currentCount == 0)
	{
		[self addToAVCLog:[NSString stringWithFormat:@"\nError: input plug %d has no connections\n\n",plug]];
		[self rereadPlugs:self];
		return;
	}

	// Create new Value
	newVal = oldVal - (1 << 24);

	status = [avcDevice compareSwap:addr withOld:oldVal andNew:newVal];

	if (status != kIOReturnSuccess)
		[self addToAVCLog:[NSString stringWithFormat:@"\nError trying to disconnect from input plug %d: 0x%08X\n\n",plug,status]];
	else
		[self addToAVCLog:[NSString stringWithFormat:@"\nCompare-swap to input plug %d successful. OldVal=0x%08X NewVal=0x%08X\n\n",plug,oldVal,newVal]];	

	[self rereadPlugs:self];
}

/***********************************************************************
**
**
**
*/
- (IBAction)makeOutputPlugConnection:(id)sender
{
	int plug = [_outPlugList selectedRow];
	int channel = [outputPlugChannel indexOfSelectedItem];
	int rate =  [outputPlugRate indexOfSelectedItem];
	UInt32 oldVal;
	UInt32 newVal;
	FWAddress addr;
	IOReturn status;
	UInt32 currentCount;

	// Handle case where no current selection
	if (plug < 0)
	{
		plug = 0;
		[_outPlugList selectRow:0 byExtendingSelection:NO];
	}

	// Read the current value of the plug
    addr.addressHi = 0xffff;
    addr.addressLo = 0xf0000904 + plug*4;
    oldVal = [avcDevice readQuad:addr];

	currentCount = ((oldVal & 0x3F000000) >> 24);

	if (currentCount == 63)
	{
		[self addToAVCLog:[NSString stringWithFormat:@"\nError: output plug %d already at maximum connection count\n\n",plug]];
		[self rereadPlugs:self];
		return;
	}

	// Create new Value
	newVal = oldVal & 0xFFC03FFF;
	newVal |= (channel << 16);
	newVal |= (rate << 14);
	newVal += (1 << 24);

	status = [avcDevice compareSwap:addr withOld:oldVal andNew:newVal];

	if (status != kIOReturnSuccess)
		[self addToAVCLog:[NSString stringWithFormat:@"\nError trying to connect to output plug %d: 0x%08X\n\n",plug,status]];
	else
		[self addToAVCLog:[NSString stringWithFormat:@"\nCompare-swap to output plug %d successful. OldVal=0x%08X NewVal=0x%08X\n\n",plug,oldVal,newVal]];	

	[self rereadPlugs:self];
}

/***********************************************************************
**
**
**
*/
- (IBAction)breakOutputPlugConnection:(id)sender
{
	int plug = [_outPlugList selectedRow];
	UInt32 oldVal;
	UInt32 newVal;
	FWAddress addr;
	IOReturn status;
	UInt32 currentCount;

	// Handle case where no current selection
	if (plug < 0)
	{
		plug = 0;
		[_outPlugList selectRow:0 byExtendingSelection:NO];
	}

	// Read the current value of the plug
    addr.addressHi = 0xffff;
    addr.addressLo = 0xf0000904 + plug*4;
    oldVal = [avcDevice readQuad:addr];

	currentCount = ((oldVal & 0x3F000000) >> 24);

	if (currentCount == 0)
	{
		[self addToAVCLog:[NSString stringWithFormat:@"\nError: output plug %d has no connections\n\n",plug]];
		[self rereadPlugs:self];
		return;
	}

	// Create new Value
	newVal = oldVal - (1 << 24);

	status = [avcDevice compareSwap:addr withOld:oldVal andNew:newVal];

	if (status != kIOReturnSuccess)
		[self addToAVCLog:[NSString stringWithFormat:@"\nError trying to disconnect from output plug %d: 0x%08X\n\n",plug,status]];
	else
		[self addToAVCLog:[NSString stringWithFormat:@"\nCompare-swap to output plug %d successful. OldVal=0x%08X NewVal=0x%08X\n\n",plug,oldVal,newVal]];	

	[self rereadPlugs:self];
}

/***********************************************************************
**
**
**
*/
- (IBAction)rereadPlugs:(id)sender
{
	int 	numOutPlugs 		= [ [ avcDevice outNum ] intValue ] ;
    int 	numInPlugs			= [ [ avcDevice inNum ] intValue ] ;

	if (_outPlugs)
		[_outPlugs release];
	if (_inPlugs)
		[_inPlugs release];

	_outPlugs = [ [ NSMutableArray arrayWithCapacity:numOutPlugs ] retain ] ;
    _inPlugs = [ [ NSMutableArray arrayWithCapacity:numInPlugs ] retain ] ;

    for( int i = 0; i < numOutPlugs; ++i )
	{
        NSObject *descObj = [ OutputPlug withDevice:avcDevice index:i ] ;
        if(descObj)
		{
            [ _outPlugs addObject:descObj ] ;
        }
    }

    for(int i = 0; i < numInPlugs; ++i)
	{
        NSObject* descObj = [ InputPlug withDevice:avcDevice index:i ] ;
        if(descObj)
		{
            [ _inPlugs addObject:descObj ] ;
        }
    }

	[_inPlugList reloadData];
	[_outPlugList reloadData];
}

/***********************************************************************
**
**
**
*/
- (IBAction)UnitInfoCommand:(id)sender
{
    UInt32 size;
    UInt8 cmd[8],response[512];
    IOReturn res;

	cmd[0] = kAVCStatusInquiryCommand;
	cmd[1] = 0xFF;
	cmd[2] = 0x30;
	cmd[3] = 0xFF;
	cmd[4] = 0xFF;
	cmd[5] = 0xFF;
	cmd[6] = 0xFF;
	cmd[7] = 0xFF;
	size = 512;

	[avcCommandBytes setStringValue:@"01FF30FFFFFFFFFF"];
	
	avcCommandVerboseLog(cmd,8,self);
	res = (*avcInterface)->AVCCommand(avcInterface, cmd, 8, response, &size);
	if (res == kIOReturnSuccess)
		avcResponseVerboseLog(response,&size,self);
	else
		[self addToAVCLog:[NSString stringWithFormat:@"\nAVCCommand Returned Error: 0x%08X\n\n",res]];	
}

/***********************************************************************
**
**
**
*/
- (IBAction)PlugInfoCommand:(id)sender
{
    UInt32 size;
    UInt8 cmd[8],response[512];
    IOReturn res;

	cmd[0] = kAVCStatusInquiryCommand;
	cmd[1] = 0xFF;
	cmd[2] = 0x02;
	cmd[3] = 0x00;
	cmd[4] = 0xFF;
	cmd[5] = 0xFF;
	cmd[6] = 0xFF;
	cmd[7] = 0xFF;
	size = 512;

	[avcCommandBytes setStringValue:@"01FF0200FFFFFFFF"];
	
	avcCommandVerboseLog(cmd,8,self);
	res = (*avcInterface)->AVCCommand(avcInterface, cmd, 8, response, &size);
	if (res == kIOReturnSuccess)
		avcResponseVerboseLog(response,&size,self);
	else
		[self addToAVCLog:[NSString stringWithFormat:@"\nAVCCommand Returned Error: 0x%08X\n\n",res]];
}

/***********************************************************************
**
**
**
*/
- (IBAction)SubunitInfoCommand:(id)sender
{
    UInt32 size;
    UInt8 cmd[8],response[512];
    IOReturn res;
	unsigned int page = [subunitInfoPage intValue];

	if ((page < 0) || (page > 7))
	{
		page = 0;
		[subunitInfoPage setIntValue:0];
	}

	cmd[0] = kAVCStatusInquiryCommand;
	cmd[1] = 0xFF;
	cmd[2] = 0x31;
	cmd[3] = (page << 4) + 0x07;
	cmd[4] = 0xFF;
	cmd[5] = 0xFF;
	cmd[6] = 0xFF;
	cmd[7] = 0xFF;
	size = 512;

	[avcCommandBytes setStringValue:[NSString stringWithFormat:@"01FF31%02XFFFFFFFF",cmd[3]]];

	avcCommandVerboseLog(cmd,8,self);
	res = (*avcInterface)->AVCCommand(avcInterface, cmd, 8, response, &size);
	if (res == kIOReturnSuccess)
		avcResponseVerboseLog(response,&size,self);
	else
		[self addToAVCLog:[NSString stringWithFormat:@"\nAVCCommand Returned Error: 0x%08X\n\n",res]];
}

/***********************************************************************
**
**
**
*/
- (IBAction)outputPlugSigFmtCommand:(id)sender
{
    UInt32 size;
    UInt8 cmd[8],response[512];
    IOReturn res;
	unsigned int plug = [outputPlugSigFmtPlugNum intValue];

	if ((plug < 0) || (plug > 31))
	{
		plug = 0;
		[outputPlugSigFmtPlugNum setIntValue:0];
	}

	cmd[0] = kAVCStatusInquiryCommand;
	cmd[1] = 0xFF;
	cmd[2] = 0x18;
	cmd[3] = plug;
	cmd[4] = 0xFF;
	cmd[5] = 0xFF;
	cmd[6] = 0xFF;
	cmd[7] = 0xFF;
	size = 512;

	[avcCommandBytes setStringValue:[NSString stringWithFormat:@"01FF18%02XFFFFFFFF",plug]];

	avcCommandVerboseLog(cmd,8,self);
	res = (*avcInterface)->AVCCommand(avcInterface, cmd, 8, response, &size);
	if (res == kIOReturnSuccess)
		avcResponseVerboseLog(response,&size,self);
	else
		[self addToAVCLog:[NSString stringWithFormat:@"\nAVCCommand Returned Error: 0x%08X\n\n",res]];
}

/***********************************************************************
**
**
**
*/
- (IBAction)inputPlugSigFmtCommand:(id)sender
{
    UInt32 size;
    UInt8 cmd[8],response[512];
    IOReturn res;
	unsigned int plug = [inputPlugSigFmtPlugNum intValue];

	if ((plug < 0) || (plug > 31))
	{
		plug = 0;
		[inputPlugSigFmtPlugNum setIntValue:0];
	}

	cmd[0] = kAVCStatusInquiryCommand;
	cmd[1] = 0xFF;
	cmd[2] = 0x19;
	cmd[3] = plug;
	cmd[4] = 0xFF;
	cmd[5] = 0xFF;
	cmd[6] = 0xFF;
	cmd[7] = 0xFF;
	size = 512;

	[avcCommandBytes setStringValue:[NSString stringWithFormat:@"01FF19%02XFFFFFFFF",plug]];

	avcCommandVerboseLog(cmd,8,self);
	res = (*avcInterface)->AVCCommand(avcInterface, cmd, 8, response, &size);
	if (res == kIOReturnSuccess)
		avcResponseVerboseLog(response,&size,self);
	else
		[self addToAVCLog:[NSString stringWithFormat:@"\nAVCCommand Returned Error: 0x%08X\n\n",res]];
}

/***********************************************************************
**
**
**
*/
- (IBAction)sendAVCCommandBytes:(id)sender
{
    UInt32 cmdSize = 0;
    UInt32 respSize = 512;
    UInt8 cmd[512],response[512];
    IOReturn res;
	NSString *bytesString = [avcCommandBytes stringValue];
	UInt32 strLen = [bytesString length];
	UInt32 i;
	unichar hexByteStr[3];
	UInt8 hexByteVal;
	NSRange strRange;
	int c;
	unichar *s;

	// Parse the command bytes string to determine the command
	strRange.length=2;
	for (i=0;i<(strLen/2);i++)
	{
		strRange.location = i*2;
		[bytesString getCharacters:hexByteStr range:strRange];
		hexByteStr[2] = 0;
		hexByteVal = 0;
		s = hexByteStr;
		while(isxdigit(c = *s))
		{
			hexByteVal *= 16;
			if(c <= '9')
				hexByteVal += c - '0';
			else
				hexByteVal += (c & 0x5F) - 'A' + 10;
			s++;
		}
		cmd[cmdSize] = hexByteVal;
		cmdSize += 1;
	}

	if (cmdSize < 1)
	{
		[self addToAVCLog:[NSString stringWithFormat:@"\nInvalid AV/C Command Bytes: %@\n\n",bytesString]];
		return;
	}
	
	respSize = 512;
	avcCommandVerboseLog(cmd,cmdSize,self);
	res = (*avcInterface)->AVCCommand(avcInterface, cmd, cmdSize, response, &respSize);
	if (res == kIOReturnSuccess)
		avcResponseVerboseLog(response,&respSize,self);
	else
		[self addToAVCLog:[NSString stringWithFormat:@"\nAVCCommand Returned Error: 0x%08X\n\n",res]];
}

/***********************************************************************
**
**
**
*/
#define kAVCCommandBufSize 16
#define KAVCDescriptorResponseBufSize 512

- (IBAction)getEIA775Info:(id)sender
{

	// Local Vars
	IOReturn result;
	UInt32 size;
    UInt8 cmd[kAVCCommandBufSize],response[kAVCCommandBufSize];
	UInt8 descriptorResponse[KAVCDescriptorResponseBufSize];
	UInt32 descriptorResponseSize;
	UInt8 *pDescriptor;
	UInt8 *pInfoBlock;
	UInt16 allInfoBlocksLen;
	UInt16 thisInfoBlockLen;
	UInt16 descriptorLen; 
	UInt16 remainingInfoBlockBytes;
	UInt16 infoBlockType;
	UInt16 fieldsLen;
	UInt32 i;
	
	[self addToAVCLog:[NSString stringWithFormat:@"\n\nEIA-775 Unit Identifier Descriptor\n"]];

	do
	{
		// Open Descriptor (for UID)
		cmd[0] = kAVCControlCommand;
		cmd[1] = 0xFF;
		cmd[2] = 0x08;
		cmd[3] = 0x00;
		cmd[4] = 0x01;
		cmd[5] = 0x00;
		size = 6;
		result = (*avcInterface)->AVCCommand(avcInterface, cmd, 6, response, &size);
		if (!((result == kIOReturnSuccess) && (response[0] == kAVCAcceptedStatus)))
		{
			[self addToAVCLog:[NSString stringWithFormat:@"Error Opening EIA-775 Unit Identifer Descriptor\n"]];
			break;
		}
								
		// Read Descriptor
		cmd[0] = kAVCControlCommand;
		cmd[1] = 0xFF;
		cmd[2] = 0x09;
		cmd[3] = 0x00;
		cmd[4] = 0xFF;
		cmd[5] = 0x00;
		cmd[6] = 0x00;
		cmd[7] = 0x00;
		cmd[8] = 0x00;
		cmd[9] = 0x00;
		descriptorResponseSize = KAVCDescriptorResponseBufSize;
		result = (*avcInterface)->AVCCommand(avcInterface, cmd, 10, descriptorResponse, &descriptorResponseSize);
		if (!((result == kIOReturnSuccess) && (descriptorResponse[0] == kAVCAcceptedStatus)))
		{
			[self addToAVCLog:[NSString stringWithFormat:@"Error Reading EIA-775 Unit Identifer Descriptor\n"]];
			break;
		}
		
		// Close Descriptor
		cmd[0] = kAVCControlCommand;
		cmd[1] = 0xFF;
		cmd[2] = 0x08;
		cmd[3] = 0x00;
		cmd[4] = 0x00;
		cmd[5] = 0x00;
		size = 6;
		result = (*avcInterface)->AVCCommand(avcInterface,cmd, 6, response, &size);
		if (!((result == kIOReturnSuccess) && (response[0] == kAVCAcceptedStatus)))
		{
			[self addToAVCLog:[NSString stringWithFormat:@"Error Closing EIA-775 Unit Identifer Descriptor\n"]];
		}
		
		// Parse Descriptor Bytes
		pDescriptor = &descriptorResponse[10];
		descriptorLen = (((UInt16)pDescriptor[0] << 8) | pDescriptor[1]);
		[self addToAVCLog:[NSString stringWithFormat:@"Unit Identifier Descriptor Length: %d\n",descriptorLen]];
		[self addToAVCLog:[NSString stringWithFormat:@"Generation ID: %d\n",pDescriptor[2]]];
		[self addToAVCLog:[NSString stringWithFormat:@"Size of List ID: %d\n",pDescriptor[3]]];
		[self addToAVCLog:[NSString stringWithFormat:@"Size of Object ID: %d\n",pDescriptor[4]]];
		[self addToAVCLog:[NSString stringWithFormat:@"Size of Object Position: %d\n",pDescriptor[5]]];
		[self addToAVCLog:[NSString stringWithFormat:@"Number of Root Object Lists: %d\n",(((UInt16)pDescriptor[6] << 8) | pDescriptor[7])]];
		[self addToAVCLog:[NSString stringWithFormat:@"Unit Dependent Length: %d\n",(((UInt16)pDescriptor[8] << 8) | pDescriptor[9])]];
		[self addToAVCLog:[NSString stringWithFormat:@"Well Defined Fields Length: %d\n",(((UInt16)pDescriptor[10] << 8) | pDescriptor[11])]];
		allInfoBlocksLen = ((((UInt16)pDescriptor[8] << 8) | pDescriptor[9]) - 2);
		if (pDescriptor[3] == 0)
			pInfoBlock = &pDescriptor[12];
		else
			pInfoBlock = &pDescriptor[12 + (pDescriptor[3] * (((UInt16)pDescriptor[6] << 8) | pDescriptor[7]))];
		
		remainingInfoBlockBytes = allInfoBlocksLen;
		while (remainingInfoBlockBytes > 0)
		{
			thisInfoBlockLen = (((UInt16)pInfoBlock[0] << 8) | pInfoBlock[1]);
			[self addToAVCLog:[NSString stringWithFormat:@"\nInfo Block Len: %d\n",thisInfoBlockLen]];
			infoBlockType = (((UInt16)pInfoBlock[2] << 8) | pInfoBlock[3]);
			[self addToAVCLog:[NSString stringWithFormat:@"Info Block Type: %d\n",infoBlockType]];
			fieldsLen = (((UInt16)pInfoBlock[4] << 8) | pInfoBlock[5]);
			[self addToAVCLog:[NSString stringWithFormat:@"Fields Length: %d\n",fieldsLen]];
			
			if (infoBlockType == 0)
			{
				if (fieldsLen > 5)
				{
					[self addToAVCLog:[NSString stringWithFormat:@"Specifier ID: 0x%02X%02X%02X\n",pInfoBlock[6],pInfoBlock[7],pInfoBlock[8]]];
					if ((pInfoBlock[6] == 0x00) && (pInfoBlock[7] == 0x50) && (pInfoBlock[8] == 0x68))
					{
						[self addToAVCLog:[NSString stringWithFormat:@"EIA-775 Block Type: %d\n",(((UInt16)pInfoBlock[9] << 8) | pInfoBlock[10])]];
						if (pInfoBlock[10] == 1)
						{
							[self addToAVCLog:[NSString stringWithFormat:@"EIA-775 Plug Info Block, Version: %d\n\n",pInfoBlock[11]]];
							[self addToAVCLog:[NSString stringWithFormat:@"OSD Input Plug: 0x%02X\n",pInfoBlock[12]]];
							[self addToAVCLog:[NSString stringWithFormat:@"OSD Output Plug: 0x%02X\n",pInfoBlock[13]]];
							[self addToAVCLog:[NSString stringWithFormat:@"Analog Input Plug: 0x%02X\n",pInfoBlock[14]]];
							[self addToAVCLog:[NSString stringWithFormat:@"Analog Output Plug: 0x%02X\n",pInfoBlock[15]]];
							[self addToAVCLog:[NSString stringWithFormat:@"Digital Input Plug: 0x%02X\n",pInfoBlock[16]]];
							[self addToAVCLog:[NSString stringWithFormat:@"Digital Output Plug: 0x%02X\n",pInfoBlock[17]]];
							[self addToAVCLog:[NSString stringWithFormat:@"Transport Stream Input Formats: "]];
							if (pInfoBlock[21] & 0x01)
								[self addToAVCLog:[NSString stringWithFormat:@"MPEG-2 "]];
							if (pInfoBlock[21] & 0x02)
								[self addToAVCLog:[NSString stringWithFormat:@"DV "]];
							if (pInfoBlock[21] & 0x04)
								[self addToAVCLog:[NSString stringWithFormat:@"DirectTV "]];
							if (pInfoBlock[21] & 0xF8)
								[self addToAVCLog:[NSString stringWithFormat:@"Other "]];
							[self addToAVCLog:[NSString stringWithFormat:@"\n"]];
							[self addToAVCLog:[NSString stringWithFormat:@"Transport Stream Output Formats: "]];
							if (pInfoBlock[25] & 0x01)
								[self addToAVCLog:[NSString stringWithFormat:@"MPEG-2 "]];
							if (pInfoBlock[25] & 0x02)
								[self addToAVCLog:[NSString stringWithFormat:@"DV "]];
							if (pInfoBlock[25] & 0x04)
								[self addToAVCLog:[NSString stringWithFormat:@"DirectTV "]];
							if (pInfoBlock[25] & 0xF8)
								[self addToAVCLog:[NSString stringWithFormat:@"Other "]];
							[self addToAVCLog:[NSString stringWithFormat:@"\n"]];
						}
						else if (pInfoBlock[10] == 2)
						{
							[self addToAVCLog:[NSString stringWithFormat:@"EIA-775 DTV Info Block, Profile Level: %d ",pInfoBlock[11]]];
							switch (pInfoBlock[11])
							{
								case 0:
									[self addToAVCLog:[NSString stringWithFormat:@"(Profile A)"]];
									break;
								case 1:
									[self addToAVCLog:[NSString stringWithFormat:@"(Profile B)"]];
									break;
								case 255:
									[self addToAVCLog:[NSString stringWithFormat:@"(Ala Carte)"]];
									break;
								default:
									[self addToAVCLog:[NSString stringWithFormat:@"(Reserved Profile Value)"]];
									break;
							};
							[self addToAVCLog:[NSString stringWithFormat:@"\n\n"]];
							[self addToAVCLog:[NSString stringWithFormat:@"OSD Formats Supported:\n"]];
							if ((pInfoBlock[12] == 0) && (pInfoBlock[13] == 0) && (pInfoBlock[14] == 0) && (pInfoBlock[15] == 0))
								[self addToAVCLog:[NSString stringWithFormat:@"    None\n"]];
							else
							{
								if (pInfoBlock[15] & 0x01)
									[self addToAVCLog:[NSString stringWithFormat:@"    OSD Layout 0, Pixel Format 1 %s (640x480x4, a:Y:Cb:Cr = 2:6:4:4)\n",
										   (pInfoBlock[19] & 0x01) ? "(Double Buffered)" : ""]];
								if (pInfoBlock[15] & 0x02)
									[self addToAVCLog:[NSString stringWithFormat:@"    OSD Layout 0, Pixel Format 2 %s (640x480x4, a:Y:Cb:Cr = 4:6:3:3)\n",
										   (pInfoBlock[19] & 0x02) ? "(Double Buffered)" : ""]];
								if (pInfoBlock[15] & 0x04)
									[self addToAVCLog:[NSString stringWithFormat:@"    OSD Layout 1, Pixel Format 1 %s (640x480x8, a:Y:Cb:Cr = 2:6:4:4)\n",
										   (pInfoBlock[19] & 0x04) ? "(Double Buffered)" : ""]];;
								if (pInfoBlock[15] & 0x08)
									[self addToAVCLog:[NSString stringWithFormat:@"    OSD Layout 1, Pixel Format 2 %s (640x480x8, a:Y:Cb:Cr = 4:6:3:3)\n",
										   (pInfoBlock[19] & 0x08) ? "(Double Buffered)" : ""]];
								if (pInfoBlock[15] & 0x10)
									[self addToAVCLog:[NSString stringWithFormat:@"    OSD Layout 1, Pixel Format 0 %s (640x480x8, Y:Cb:Cr = 6:5:5)\n",
										   (pInfoBlock[19] & 0x10) ? "(Double Buffered)" : ""]];
								if (pInfoBlock[15] & 0x20)
									[self addToAVCLog:[NSString stringWithFormat:@"    OSD Layout 2, Pixel Format 1 %s (640x480x16, a:Y:Cb:Cr = 2:6:4:4)\n",
										   (pInfoBlock[19] & 0x20) ? "(Double Buffered)" : ""]];
								if (pInfoBlock[15] & 0x40)
									[self addToAVCLog:[NSString stringWithFormat:@"    OSD Layout 2, Pixel Format 2 %s (640x480x16, a:Y:Cb:Cr = 4:6:3:3)\n",
										   (pInfoBlock[19] & 0x40) ? "(Double Buffered)" : ""]];
								if (pInfoBlock[15] & 0x80)
									[self addToAVCLog:[NSString stringWithFormat:@"    OSD Layout 2, Pixel Format 0 %s (640x480x16, Y:Cb:Cr = 6:5:5)\n",
										   (pInfoBlock[19] & 0x80) ? "(Double Buffered)" : ""]];
							}
							[self addToAVCLog:[NSString stringWithFormat:@"\n"]];
							
							[self addToAVCLog:[NSString stringWithFormat:@"Misc Features:\n"]];
							[self addToAVCLog:[NSString stringWithFormat:@"    Stretching of 640x480 grid to 14:9: %s\n",
								   (pInfoBlock[22] & 0x02) ? "Yes" : "No"]];
							[self addToAVCLog:[NSString stringWithFormat:@"    Stretching of 640x480 grid to 16:9: %s\n",
								   (pInfoBlock[22] & 0x04) ? "Yes" : "No"]];
							[self addToAVCLog:[NSString stringWithFormat:@"    OSD Fill Surround: %s\n",
								   (pInfoBlock[22] & 0x08) ? "Yes" : "No"]];
							[self addToAVCLog:[NSString stringWithFormat:@"    Source Driven Digital/Analog Selection: %s\n",
								   (pInfoBlock[22] & 0x10) ? "Yes" : "No"]];
							[self addToAVCLog:[NSString stringWithFormat:@"    OSD Over Analog Video: %s\n",
								   (pInfoBlock[22] & 0x20) ? "Yes" : "No"]];
							[self addToAVCLog:[NSString stringWithFormat:@"\n"]];
							
							[self addToAVCLog:[NSString stringWithFormat:@"Default Video Format: "]];
							switch (pInfoBlock[23])
							{
								case 0:
									[self addToAVCLog:[NSString stringWithFormat:@"Unknown\n"]];
									break;
								case 1:
									[self addToAVCLog:[NSString stringWithFormat:@"1920x1080 Interlaced\n"]];
									break;
								case 2:
									[self addToAVCLog:[NSString stringWithFormat:@"1920x1080 Progressive\n"]];
									break;
								case 3:
									[self addToAVCLog:[NSString stringWithFormat:@"1280x720 Progressive\n"]];
									break;
								case 4:
									[self addToAVCLog:[NSString stringWithFormat:@"704x480 (4x3) Interlaced\n"]];
									break;
								case 5:
									[self addToAVCLog:[NSString stringWithFormat:@"704x480 (4x3) Progressive\n"]];
									break;
								case 6:
									[self addToAVCLog:[NSString stringWithFormat:@"704x480 (16x9) Interlaced\n"]];
									break;
								case 7:
									[self addToAVCLog:[NSString stringWithFormat:@"704x480 (16x9) Progressive\n"]];
									break;
								case 8:
									[self addToAVCLog:[NSString stringWithFormat:@"640x480 Interlaced\n"]];
									break;
								case 9:
									[self addToAVCLog:[NSString stringWithFormat:@"640x480 Progressive\n"]];
									break;
								default:
									[self addToAVCLog:[NSString stringWithFormat:@"Reserved Value\n"]];
									break;
							};
							
							[self addToAVCLog:[NSString stringWithFormat:@"Analog Video Conversion Format: "]];
							switch (pInfoBlock[24])
							{
								case 0:
									[self addToAVCLog:[NSString stringWithFormat:@"Unknown\n"]];
									break;
								case 1:
									[self addToAVCLog:[NSString stringWithFormat:@"1920x1080 Interlaced\n"]];
									break;
								case 2:
									[self addToAVCLog:[NSString stringWithFormat:@"1920x1080 Progressive\n"]];
									break;
								case 3:
									[self addToAVCLog:[NSString stringWithFormat:@"1280x720 Progressive\n"]];
									break;
								case 4:
									[self addToAVCLog:[NSString stringWithFormat:@"704x480 (4x3) Interlaced\n"]];
									break;
								case 5:
									[self addToAVCLog:[NSString stringWithFormat:@"704x480 (4x3) Progressive\n"]];
									break;
								case 6:
									[self addToAVCLog:[NSString stringWithFormat:@"704x480 (16x9) Interlaced\n"]];
									break;
								case 7:
									[self addToAVCLog:[NSString stringWithFormat:@"704x480 (16x9) Progressive\n"]];
									break;
								case 8:
									[self addToAVCLog:[NSString stringWithFormat:@"640x480 Interlaced\n"]];
									break;
								case 9:
									[self addToAVCLog:[NSString stringWithFormat:@"640x480 Progressive\n"]];
									break;
								default:
									[self addToAVCLog:[NSString stringWithFormat:@"Reserved Value\n"]];
									break;
							};
							[self addToAVCLog:[NSString stringWithFormat:@"\n"]];
							[self addToAVCLog:[NSString stringWithFormat:@"Alignment Data:\n\n"]];
							for (i=0;i<9;i++)
							{
								switch (i)
								{
									case 0:
										[self addToAVCLog:[NSString stringWithFormat:@"    1920x1080 Interlaced:\n"]];
										break;
									case 1:
										[self addToAVCLog:[NSString stringWithFormat:@"    1920x1080 Progressive:\n"]];
										break;
									case 2:
										[self addToAVCLog:[NSString stringWithFormat:@"    1280x720 Progressive:\n"]];
										break;
									case 3:
										[self addToAVCLog:[NSString stringWithFormat:@"    704x480 (4x3) Interlaced:\n"]];
										break;
									case 4:
										[self addToAVCLog:[NSString stringWithFormat:@"    704x480 (4x3) Progressive:\n"]];
										break;
									case 5:
										[self addToAVCLog:[NSString stringWithFormat:@"    704x480 (16x9) Interlaced:\n"]];
										break;
									case 6:
										[self addToAVCLog:[NSString stringWithFormat:@"    704x480 (16x9) Progressive:\n"]];
										break;
									case 7:
										[self addToAVCLog:[NSString stringWithFormat:@"    640x480 Interlaced:\n"]];
										break;
									case 8:
									default:
										[self addToAVCLog:[NSString stringWithFormat:@"    640x480 Progressive:\n"]];
										break;
								};
								if (pInfoBlock[25+(i*6)] & 0x80)
									[self addToAVCLog:[NSString stringWithFormat:@"      Stretching to 14:9\n"]];
								if (pInfoBlock[25+(i*6)] & 0x40)
									[self addToAVCLog:[NSString stringWithFormat:@"      Stretching to 16:9\n"]];
								if (pInfoBlock[25+(i*6)] & 0x10)
									[self addToAVCLog:[NSString stringWithFormat:@"      Display as Interlaced\n"]];
								else
									[self addToAVCLog:[NSString stringWithFormat:@"      Display as Progressive\n"]];
								[self addToAVCLog:[NSString stringWithFormat:@"      Video Scan Lines: %d\n",(((UInt16)pInfoBlock[25+(i*6)] & 0x0F) << 8) | pInfoBlock[26+(i*6)]]];
								[self addToAVCLog:[NSString stringWithFormat:@"      OSD Scan Lines: %d\n",(((UInt16)pInfoBlock[27+(i*6)] & 0x0F) << 8) | pInfoBlock[28+(i*6)]]];
								[self addToAVCLog:[NSString stringWithFormat:@"      Pixel Aspect Ratio: %f\n\n",pInfoBlock[30+(i*6)] / 128.0]];
							}
						}
					}
				}
			}
			remainingInfoBlockBytes -= (thisInfoBlockLen+2);
			pInfoBlock = &pInfoBlock[thisInfoBlockLen+2];
		}
	}while(0);
}

/***********************************************************************
**
**
**
*/
- (IBAction)clearLog:(id)sender
{
	// Start over with an empty log string
	[avcLogString autorelease];
	avcLogString = [[NSMutableString stringWithCapacity:4096] retain];
	[avcLog setString:avcLogString];
}

@end

typedef struct valueStringsStruct
{
	int val;
	const char *string;
}ValueStrings, *ValueStringsPtr;

ValueStrings cTypeStrings[] =
{
	{ 0,"Control"},
	{ 1,"Status"},
	{ 2,"Specific-Inquiry"},
	{ 3,"Notify"},
	{ 4,"General-Inquiry"},
	{ -1,nil}
};

ValueStrings responseStrings[] =
{
	{ 0x08,"Not Implemented"},
	{ 0x09,"Accepted"},
	{ 0x0A,"Rejected"},
	{ 0x0B,"In Transition"},
	{ 0x0C,"Implemented/Stable"},
	{ 0x0D,"Changed"},
	{ 0x0E,"Reserved Response"},
	{ 0x0F,"Interim"},
	{ -1,nil}
};

ValueStrings unitOpCodeStrings[] =
{
	// Unit Commands
	{ 0x00,"Vendor Unique"},
	{ 0x02,"Plug-Info"},
	{ 0x30,"Unit Info"},
	{ 0x31,"Sub-Unit Info"},
	{ 0xB2,"Power"},
	{ 0x24,"Connect"},
	{ 0x25,"Disconnect"},
	{ 0x18,"Output Plug Signal Format"},
	{ 0x19,"Input Plug Signal Format"},
	{ 0x0F,"DTCP"},

	// Descriptor Commands
	{ 0x06,"Read Info-Block"},
	{ 0x07,"Write Info-Block"},
	{ 0x08,"Open Descriptor"},
	{ 0x09,"Read Descriptor"},
	{ 0x0A,"Write Descriptor"},
	{ 0x0C,"Create Descriptor"},

	{ -1,nil}
};

ValueStrings discOpCodeStrings[] =
{
	// Unit/Subunit Commands
	{ 0x00,"Vendor Unique"},
	{ 0x02,"Plug-Info"},

	// Descriptor Commands
	{ 0x06,"Read Info-Block"},
	{ 0x07,"Write Info-Block"},
	{ 0x08,"Open Descriptor"},
	{ 0x09,"Read Descriptor"},
	{ 0x0A,"Write Descriptor"},
	{ 0x0C,"Create Descriptor"},

	// Disc Subunit Commands
	{ 0xC5,"Stop"},
	{ 0xC3,"Play"},
	{ 0xC2,"Record"},
	{ 0xD1,"Configure"},
	{ 0x40,"Erase"},
	{ 0xD3,"Set Plug Association"},

	{ -1,nil}
};

ValueStrings tapeOpCodeStrings[] =
{
	// Unit/Subunit Commands
	{ 0x00,"Vendor Unique"},
	{ 0x02,"Plug-Info"},

	// Tape Subunit Commands
	{ 0x70,"Analog Audio Output Mode"},
	{ 0x72,"Area Mode"},
	{ 0x52,"Absolute Track Number"},
	{ 0x71,"Audio Mode"},
	{ 0x56,"Backward"},
	{ 0x5A,"Binary Group"},
	{ 0x40,"Edit Mode"},
	{ 0x55,"Forward"},
	{ 0x79,"Input Signal Mode"},
	{ 0xC1,"Load Medium"},
	{ 0xCA,"Marker"},
	{ 0xDA,"Medium Info"},
	{ 0x60,"Open MIC"},
	{ 0x78,"Output Signal Mode"},
	{ 0xC3,"Play"},
	{ 0x45,"Preset"},
	{ 0x61,"Read MIC"},
	{ 0xC2,"Record"},
	{ 0x53,"Recording Date"},
	{ 0xDB,"Recording Speed"},
	{ 0x54,"Recording Time"},
	{ 0x57,"Relative Time Counter"},
	{ 0x50,"Search Mode"},
	{ 0x5C,"SMPTE/EBU Recording Time"},
	{ 0x59,"SMPTE/EBU Time Code"},
	{ 0xD3,"Tape Playback Format"},
	{ 0xD2,"Tape Recording Format"},
	{ 0x51,"Time Code"},
	{ 0xD0,"Transport State"},
	{ 0xC4,"Wind"},
	{ 0x62,"Write MIC"},

	{ -1,nil}
};

/***********************************************************************
**
**
**
*/
const char* valToString(ValueStringsPtr pStringTable, int val)
{
	ValueStringsPtr pValueString = pStringTable;

	while (pValueString->val != -1)
	{
		if (pValueString->val == val)
			return pValueString->string;
		else
			pValueString = pValueString + 1; // Index to the next value string pair
	}
	return "Unknown";
}


/***********************************************************************
**
**
**
*/
void avcCommandVerboseLog(UInt8 *command, UInt32 cmdLen, PlugBrowserController *userIntf)
{
	UInt8 cType = command[0] & 0x0F;
    UInt8 subUnit = command[1];
    UInt8 opCode = command[2];
	//    UInt8 *pOperands = (UInt8*) &command[3];
	unsigned int i;

	[userIntf addToAVCLog:[NSString stringWithFormat:@"\n=============== Sent AVC Command ===============\n"]];

	[userIntf addToAVCLog:[NSString stringWithFormat:@"cType:   %s\n",valToString(cTypeStrings,cType)]];
	[userIntf addToAVCLog:[NSString stringWithFormat:@"subUnit: 0x%02X\n",subUnit]];

	if (subUnit == 0x20)
		[userIntf addToAVCLog:[NSString stringWithFormat:@"opCode:  %s (0x%02X)\n",valToString(tapeOpCodeStrings,opCode),opCode]];
	else if (subUnit == 0x18)
		[userIntf addToAVCLog:[NSString stringWithFormat:@"opCode:  %s (0x%02X)\n",valToString(discOpCodeStrings,opCode),opCode]];
	else
		[userIntf addToAVCLog:[NSString stringWithFormat:@"opCode:  %s (0x%02X)\n",valToString(unitOpCodeStrings,opCode),opCode]];

	[userIntf addToAVCLog:[NSString stringWithFormat:@"FCP Command Frame:"]];
	for (i=0;i<cmdLen;i++)
	{
		if ((i % 16) == 0)
			[userIntf addToAVCLog:[NSString stringWithFormat:@"\n"]];

		[userIntf addToAVCLog:[NSString stringWithFormat:@"%02X ",command[i]]];
	}
	[userIntf addToAVCLog:[NSString stringWithFormat:@"\n"]];

	return;
}


/***********************************************************************
**
**
**
*/
void avcResponseVerboseLog(	UInt8 *response, UInt32 *responseLen, PlugBrowserController *userIntf)
{
    UInt8 subUnit = response[1];
    UInt8 opCode = response[2];
	//    UInt8 *pResponseOperands = (UInt8*) &response[3];
    UInt8 *pResponseType = (UInt8*) &response[0];
	unsigned int i;

	[userIntf addToAVCLog:[NSString stringWithFormat:@"\n=============== Received AVC Response ===============\n"]];

	[userIntf addToAVCLog:[NSString stringWithFormat:@"response: %s\n",valToString(responseStrings,*pResponseType)]];
	[userIntf addToAVCLog:[NSString stringWithFormat:@"subUnit: 0x%02X\n",subUnit]];

	if (subUnit == 0x20)
		[userIntf addToAVCLog:[NSString stringWithFormat:@"opCode:  %s (0x%02X)\n",valToString(tapeOpCodeStrings,opCode),opCode]];
	else if (subUnit == 0x18)
		[userIntf addToAVCLog:[NSString stringWithFormat:@"opCode:  %s (0x%02X)\n",valToString(discOpCodeStrings,opCode),opCode]];
	else
		[userIntf addToAVCLog:[NSString stringWithFormat:@"opCode:  %s (0x%02X)\n",valToString(unitOpCodeStrings,opCode),opCode]];

	[userIntf addToAVCLog:[NSString stringWithFormat:@"FCP Response Frame:"]];

	for (i=0;i<*responseLen;i++)
	{
		if ((i % 16) == 0)
			[userIntf addToAVCLog:[NSString stringWithFormat:@"\n"]];

		[userIntf addToAVCLog:[NSString stringWithFormat:@"%02X ",response[i]]];
	}
	[userIntf addToAVCLog:[NSString stringWithFormat:@"\n"]];

	return;
}

