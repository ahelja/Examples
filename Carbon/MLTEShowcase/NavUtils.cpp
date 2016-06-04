/*
 *  NavUtils.cpp
 *  MLTEShowcase
 *
 *  Created on Wed Jun 09 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.

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

#include "NavUtils.h"
#include "MLTEShowcaseUtils.h"

OSStatus
GetFileSpecOutOfNavReply( NavReplyRecord& reply, FSSpec& gotFileSpec  )
{
    AEDesc actualDesc;
	
    FSRef parent;
    FSRef fileToOpen;
    FSCatalogInfo catInfo;
	
    HFSUniStr255 fileNameUStr;

    OSStatus status = paramErr;
    
    status = AECoerceDesc( &reply.selection, typeFSRef, &actualDesc );
    require( status == noErr, COERCE_FAIL );

    if( &actualDesc != NULL )
    {
        status = AEGetDescData( &actualDesc, (void*)(&fileToOpen), sizeof(FSRef));

        require( status == noErr, OPEN_FAIL_DISPOSEAEDESC );
        status = ::FSGetCatalogInfo( &fileToOpen, kFSCatInfoNone, &catInfo/*FSCatalogInfo*/,
                                &fileNameUStr, &gotFileSpec, &parent/*parent*/ );
        require( status == noErr, OPEN_FAIL_DISPOSEAEDESC );
    }
	
    OPEN_FAIL_DISPOSEAEDESC:
    AEDisposeDesc( &actualDesc );
    COERCE_FAIL:
	
    switch( status )
    {
        case nsvErr:
            WarnStatusString("\"no such volume\" error: %li",status);
            break;
        case noErr:
            break;
        default:
            WarnStatusString("GetFileSpecOutOfNavReply failed: %li",status);
    }
    return status;
}

OSStatus
GetFSRefOutOfNavReply( const NavReplyRecord& reply, FSRef* gotFSRef )
{
	AEDesc fsRefAEDesc;
    OSStatus status = paramErr;
	
	require( gotFSRef != NULL, FAIL );

	status = AECoerceDesc( &reply.selection, typeFSRef, &fsRefAEDesc );    
    require( status == noErr, COERCE_FAIL );
	status = AEGetDescData( &fsRefAEDesc, (void*)(gotFSRef), sizeof(FSRef));
    require( status == noErr, OPEN_FAIL_DISPOSEAEDESC );
    
    OPEN_FAIL_DISPOSEAEDESC:
    AEDisposeDesc( &fsRefAEDesc );
    COERCE_FAIL:
	FAIL:
    
    switch( status )
    {
        case nsvErr:
            WarnStatusString("\"no such volume\" error: %li",status);
            break;
        case noErr:
            break;
        default:
            WarnStatusString("GetFSRefOutOfNavReply failed: %li",status);
    }
	return status;
}

OSStatus
GetCFURLOutOfNavReply( const NavReplyRecord& reply, CFURLRef* gotURLRef )
{
    OSStatus status = paramErr;
	FSRef fsRef;
	
	status = GetFSRefOutOfNavReply( reply, &fsRef );
	require( status == noErr, FAIL );
	*gotURLRef = CFURLCreateFromFSRef(NULL, &fsRef);
    
	FAIL:
    if( status != noErr )
		WarnStatusString("GetCFURLOutOfNavReply failed: %li",status);
		
	return status;
}

OSStatus
GetFileInfoOutOfNavReply( const NavReplyRecord& reply, FSRef& gotFileRef,
    FSSpec& gotFileSpec, HFSUniStr255& hfsFileName  )
{
    AEDesc fsspecAEDesc;
    FSRef parent;

    OSStatus status = paramErr;
    
    status = AECoerceDesc( &reply.selection, typeFSRef, &fsspecAEDesc );
    require( status == noErr, COERCE_FAIL );

    status = AEGetDescData( &fsspecAEDesc, (void*)(&gotFileRef), sizeof(FSRef));

    require( status == noErr, OPEN_FAIL_DISPOSEAEDESC );
    status = ::FSGetCatalogInfo( &gotFileRef, kFSCatInfoNone, NULL/*FSCatalogInfo*/,
                                 &hfsFileName, &gotFileSpec, &parent/*parent*/ );
    require( status == noErr, OPEN_FAIL_DISPOSEAEDESC );
    
    OPEN_FAIL_DISPOSEAEDESC:
    AEDisposeDesc( &fsspecAEDesc );
    COERCE_FAIL:
    
    switch( status )
    {
        case nsvErr:
            WarnStatusString("\"no such volume\" error: %li",status);
            break;
        case noErr:
            break;
        default:
            WarnStatusString("GetFileSpecOutOfNavReply failed: %li",status);
    }
	return status;
}

void
GetHFSUniStrSaveFileName( const NavDialogRef navDialog, HFSUniStr255& hfsSaveFileName )
{
    CFStringRef cfStrSaveFileName = ::NavDialogGetSaveFileName( navDialog );
    
    hfsSaveFileName.length =
        ::CFStringGetBytes( cfStrSaveFileName,
                            ::CFRangeMake( 0, MIN( ::CFStringGetLength( cfStrSaveFileName ), 255)), 
                            kCFStringEncodingUnicode, 0, false,
                            (UInt8 *)(hfsSaveFileName.unicode), 255, NULL );
}
