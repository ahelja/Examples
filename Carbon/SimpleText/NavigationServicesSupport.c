/*
	File:		NavigationServicesSupport.c

	Contains:	Code to support Navigation Services in SimpleText

	Version:	Mac OS X

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

	Copyright © 1997-2001 Apple Computer, Inc., All Rights Reserved
*/

#include "NavigationServicesSupport.h"
#include "SimpleText.h"
#if !defined(USE_UMBRELLA_HEADERS) || !USE_UMBRELLA_HEADERS
#include <CodeFragments.h>
#include <Finder.h>
#include <Dialogs.h>
#include <LowMem.h>
#include <Processes.h>
#endif

#ifndef nrequire
	#define nrequire(CONDITION, LABEL) if (true) {if ((CONDITION)) goto LABEL; }
#endif
#ifndef require
#define require(CONDITION, LABEL) if (true) {if (!(CONDITION)) goto LABEL; }
#endif


static NavDialogRef gOpenFileDialog = NULL;


static pascal void MyEventProc( const NavEventCallbackMessage callbackSelector, 
								NavCBRecPtr callbackParms, 
								NavCallBackUserData callbackUD );


static NavEventUPP GetEventUPP()
{
	static NavEventUPP	eventUPP = NULL;				
	if ( eventUPP == NULL )
	{
		eventUPP = NewNavEventUPP( MyEventProc );
	}
	return eventUPP;
}


static pascal void MyPrivateEventProc( const NavEventCallbackMessage callbackSelector, 
									   NavCBRecPtr callbackParms, 
									   NavCallBackUserData callbackUD );


static NavEventUPP GetPrivateEventUPP()
{
	static NavEventUPP	privateEventUPP = NULL;				
	if ( privateEventUPP == NULL )
	{
		privateEventUPP = NewNavEventUPP( MyPrivateEventProc );
	}
	return privateEventUPP;
}


static Handle NewOpenHandle(OSType applicationSignature, short numTypes, OSType typeList[])
{
	Handle hdl = NULL;
	
	if ( numTypes > 0 )
	{
	
		hdl = NewHandle(sizeof(NavTypeList) + numTypes * sizeof(OSType));
	
		if ( hdl != NULL )
		{
			NavTypeListHandle open		= (NavTypeListHandle)hdl;
			
			(*open)->componentSignature = applicationSignature;
			(*open)->osTypeCount		= numTypes;
			BlockMoveData(typeList, (*open)->osType, numTypes * sizeof(OSType));
		}
	}
	
	return hdl;
}


OSStatus SendOpenAE( AEDescList list )
{
	OSStatus		err;
	AEAddressDesc	theAddress;
	AppleEvent		dummyReply;
	AppleEvent		theEvent;
	
	theAddress.descriptorType	= typeNull;
	theAddress.dataHandle		= NULL;

    dummyReply.descriptorType	= typeNull;
    dummyReply.dataHandle		= NULL;

    theEvent.descriptorType		= typeNull;
    theEvent.dataHandle			= NULL;

	do {
		ProcessSerialNumber psn;

		err = GetCurrentProcess(&psn);
		if ( err != noErr) break;
		
		err =AECreateDesc(typeProcessSerialNumber, &psn, sizeof(ProcessSerialNumber), &theAddress);
		if ( err != noErr) break;

		err = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments, &theAddress, kAutoGenerateReturnID, kAnyTransactionID, &theEvent);
		if ( err != noErr) break;
		
		err = AEPutParamDesc(&theEvent, keyDirectObject, &list);
		if ( err != noErr) break;
		
		err = AESend(&theEvent, &dummyReply, kAENoReply, kAENormalPriority, kAEDefaultTimeout, NULL, NULL);
		if ( err != noErr) break;
		
        
	} while (false);
	
    if ( theAddress.dataHandle != NULL )
    {
        AEDisposeDesc( &theAddress );
    }

    if ( dummyReply.dataHandle != NULL )
    {
        AEDisposeDesc( &dummyReply );
    }

    if ( theEvent.dataHandle != NULL )
    {
        AEDisposeDesc( &theEvent );
    }

	return err;
}


OSStatus OpenFileDialog(
	OSType applicationSignature, 
	short numTypes, 
	OSType typeList[], 
	NavDialogRef *outDialog )
{
	OSStatus theErr = noErr;
	if ( gOpenFileDialog == NULL )
	{
		NavDialogCreationOptions	dialogOptions;
		NavTypeListHandle			openList	= NULL;
	
		NavGetDefaultDialogCreationOptions( &dialogOptions );
	
		dialogOptions.modality = kWindowModalityNone;
		dialogOptions.clientName = CFStringCreateWithPascalString( NULL, LMGetCurApName(), GetApplicationTextEncoding());
		
		openList = (NavTypeListHandle)NewOpenHandle( applicationSignature, numTypes, typeList );
		
		theErr = NavCreateGetFileDialog( &dialogOptions, openList, GetPrivateEventUPP(), NULL, NULL, NULL, &gOpenFileDialog );

		if ( theErr == noErr )
		{
			theErr = NavDialogRun( gOpenFileDialog );
			if ( theErr != noErr )
			{
				NavDialogDispose( gOpenFileDialog );
				gOpenFileDialog = NULL;
			}
		}

		if (openList != NULL)
		{
			DisposeHandle((Handle)openList);
		}
		
		if ( dialogOptions.clientName != NULL )
		{
			CFRelease( dialogOptions.clientName );
		}
	}
	else
	{
		if ( NavDialogGetWindow( gOpenFileDialog ) != NULL )
		{
			SelectWindow( NavDialogGetWindow( gOpenFileDialog ));
		}
	}
	
	if ( outDialog != NULL )
	{
		*outDialog = gOpenFileDialog;
	}

	return noErr;
}


void TerminateOpenFileDialog()
{
	if ( gOpenFileDialog != NULL )
	{
		TerminateDialog( gOpenFileDialog );
	}
}


void TerminateDialog( NavDialogRef inDialog )
{
	NavCustomControl( inDialog, kNavCtlTerminate, NULL );
}


static OSStatus UniversalConfirmSaveDialog(
	WindowRef parentWindow, 
	CFStringRef documentName, 
	Boolean quitting, 
	void* inContextData,
	NavDialogRef *outDialog,
	NavUserAction *outUserAction )
{
	OSStatus 			theErr 			= noErr;
	NavAskSaveChangesAction		action 			= 0;
	NavDialogRef			dialog 			= NULL;
	NavUserAction			userAction 		= kNavUserActionNone;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP			eventUPP;
	Boolean				disposeAfterRun;

	NavGetDefaultDialogCreationOptions( &dialogOptions );

	action = quitting ? kNavSaveChangesQuittingApplication : kNavSaveChangesClosingDocument;
	dialogOptions.modality = ( parentWindow != NULL ) ? kWindowModalityWindowModal : kWindowModalityAppModal;
	dialogOptions.parentWindow = parentWindow;

	dialogOptions.clientName = CFStringCreateWithPascalString( NULL, LMGetCurApName(), GetApplicationTextEncoding());
	if ( documentName != NULL )
	{
		dialogOptions.saveFileName = documentName;
	}

	eventUPP = ( inContextData == NULL ) ? GetPrivateEventUPP() : GetEventUPP();
	disposeAfterRun = ( dialogOptions.modality == kWindowModalityAppModal && inContextData == NULL );

	theErr = NavCreateAskSaveChangesDialog(	
				&dialogOptions,
				action,
				eventUPP,
				inContextData,
				&dialog );
	
	if ( theErr == noErr )
	{
		theErr = NavDialogRun( dialog );
		if ( theErr != noErr || disposeAfterRun )
		{
			userAction = NavDialogGetUserAction( dialog );
			NavDialogDispose( dialog );
			dialog = NULL;
		}
	}

	if ( dialogOptions.clientName != NULL )
	{
		CFRelease( dialogOptions.clientName );
	}
	if ( outDialog != NULL )
	{
		*outDialog = dialog;
	}
	if ( outUserAction != NULL )
	{
		*outUserAction = userAction;
	}
	return theErr;
}


OSStatus ConfirmSaveDialog(
	WindowRef parentWindow, 
	CFStringRef documentName, 
	Boolean quitting, 
	void* inContextData,
	NavDialogRef *outDialog )
{
	return UniversalConfirmSaveDialog( parentWindow, documentName, quitting, inContextData, outDialog, NULL );
}


OSStatus ModalConfirmSaveDialog(
	CFStringRef documentName, 
	Boolean quitting, 
	NavUserAction *outUserAction )
{
	return UniversalConfirmSaveDialog( NULL, documentName, quitting, NULL, NULL, outUserAction );
}


OSStatus SaveFileDialog(
	WindowRef parentWindow, 
	CFStringRef documentName, 
	OSType filetype, 
	OSType fileCreator, 
	void *inContextData,
	NavDialogRef *outDialog )
{
	NavDialogCreationOptions	dialogOptions;
	OSStatus					theErr = noErr;

	NavGetDefaultDialogCreationOptions( &dialogOptions );

	dialogOptions.clientName = CFStringCreateWithPascalString( NULL, LMGetCurApName(), GetApplicationTextEncoding());
	dialogOptions.saveFileName = documentName;
	dialogOptions.modality = ( parentWindow != NULL ) ? kWindowModalityWindowModal : kWindowModalityAppModal;
	dialogOptions.parentWindow = parentWindow;

	theErr = NavCreatePutFileDialog( &dialogOptions, filetype, fileCreator, GetEventUPP(), inContextData, outDialog );
	
	if ( theErr == noErr )
	{
		theErr = NavDialogRun( *outDialog );
		if ( theErr != noErr )
		{
			NavDialogDispose( *outDialog );
		}
		if ( theErr != noErr || dialogOptions.modality == kWindowModalityAppModal )
		{
			*outDialog = NULL;	// The dialog has already been disposed.
		}
	}

	if ( dialogOptions.clientName != NULL )
	{
		CFRelease( dialogOptions.clientName );
	}

	return theErr;
}


OSStatus BeginSave( NavDialogRef inDialog, NavReplyRecord* outReply, FSRef* outFileRef )
{
	OSStatus status = paramErr;
	AEDesc		dirDesc;
	AEKeyword	keyword;
	CFIndex		len;

	require( outReply, Return );
	require( outFileRef, Return );

	status = NavDialogGetReply( inDialog, outReply );
	nrequire( status, Return );
	
	status = AEGetNthDesc( &outReply->selection, 1, typeWildCard, &keyword, &dirDesc );
	nrequire( status, DisposeReply );
	
	len = CFStringGetLength( outReply->saveFileName );

	if ( dirDesc.descriptorType == typeFSRef )
	{
		const UInt32	kMaxNameLen = 255;
		FSRef		dirRef;
		UniChar		name[ kMaxNameLen ];

		if ( len > kMaxNameLen )
		{
			len = kMaxNameLen;
		}
	
		status = AEGetDescData( &dirDesc, &dirRef, sizeof( dirRef ));
		nrequire( status, DisposeDesc );
		
		CFStringGetCharacters( outReply->saveFileName, CFRangeMake( 0, len ), &name[0] );
		
		status = FSMakeFSRefUnicode( &dirRef, len, &name[0], GetApplicationTextEncoding(), outFileRef );
		if (status == fnfErr )
		{
                        // file is not there yet - create it and return FSRef
			status = FSCreateFileUnicode( &dirRef, len, &name[0], 0, NULL, outFileRef, NULL );
		}
		else
		{
                        // looks like file is there. Just make sure there is no error
			nrequire( status, DisposeDesc );
		}
	}
	else if ( dirDesc.descriptorType == typeFSS )
	{
                FSSpec	theSpec;
		status = AEGetDescData( &dirDesc, &theSpec, sizeof( FSSpec ));
		nrequire( status, DisposeDesc );

		if ( CFStringGetPascalString( outReply->saveFileName, &(theSpec.name[0]), 
					sizeof( StrFileName ), GetApplicationTextEncoding()))
		{
                        status = FSpMakeFSRef(&theSpec, outFileRef);
			nrequire( status, DisposeDesc );
			status = FSpCreate( &theSpec, 0, 0, smSystemScript );
			nrequire( status, DisposeDesc );
		}
		else
		{
			status = bdNamErr;
			nrequire( status, DisposeDesc );
		}
	}

DisposeDesc:
	AEDisposeDesc( &dirDesc );

DisposeReply:
	if ( status != noErr )
	{
		NavDisposeReply( outReply );
	}

Return:
	return status;
}


OSStatus CompleteSave( NavReplyRecord* inReply, FSRef* inFileRef, Boolean inDidWriteFile )
{
	OSStatus theErr = noErr;
	
	if ( inReply->validRecord )
	{
		if ( inDidWriteFile )
		{
			theErr = NavCompleteSave( inReply, kNavTranslateInPlace );
		}
		else if ( !inReply->replacing )
		{
			// Write failed, not replacing, so delete the file
			// that was created in BeginSave.
			FSDeleteObject( inFileRef );
		}

		theErr = NavDisposeReply( inReply );
	}

	return theErr;
}


//
// Callback to handle events that occur while navigation dialogs are up but really should be handled by the application
//

extern void HandleEvent( EventRecord * pEvent );
extern void HandleNavUserAction( NavDialogRef inNavDialog, NavUserAction inUserAction, void* inContextData );

static pascal void MyEventProc( const NavEventCallbackMessage callbackSelector, 
								NavCBRecPtr callbackParms, 
								NavCallBackUserData callbackUD )
// Callback to handle event passing betwwn the navigation dialogs and the applicatio
{
	switch ( callbackSelector )
	{	
		case kNavCBEvent:
		{
			switch (callbackParms->eventData.eventDataParms.event->what)
			{
				case updateEvt:
				case activateEvt:
					HandleEvent(callbackParms->eventData.eventDataParms.event);
				break;
			}
		}
		break;

		case kNavCBUserAction:
		{
			// Call HandleNavUserAction
			HandleNavUserAction( callbackParms->context, callbackParms->userAction, callbackUD );
		}
		break;
		
		case kNavCBTerminate:
		{
			// Auto-dispose the dialog
			NavDialogDispose( callbackParms->context );
		}
	}
}

static pascal void MyPrivateEventProc( const NavEventCallbackMessage callbackSelector, 
									   NavCBRecPtr callbackParms, 
									   NavCallBackUserData callbackUD )
{
	switch ( callbackSelector )
	{	
		case kNavCBEvent:
		{
			switch (callbackParms->eventData.eventDataParms.event->what)
			{
				case updateEvt:
				case activateEvt:
					HandleEvent(callbackParms->eventData.eventDataParms.event);
				break;
			}
		}
		break;

		case kNavCBUserAction:
		{
			if ( callbackParms->userAction == kNavUserActionOpen )
			{
				// This is an open files action, send an AppleEvent
				NavReplyRecord	reply;
				OSStatus		status;
				
				status = NavDialogGetReply( callbackParms->context, &reply );
				if ( status == noErr )
				{
					SendOpenAE( reply.selection );
					NavDisposeReply( &reply );
				}
			}
		}
		break;
		
		case kNavCBTerminate:
		{			
			if ( callbackParms->context == gOpenFileDialog )
			{
				NavDialogDispose( gOpenFileDialog );
				gOpenFileDialog = NULL;
			}
			
			// if after dismissing the dialog SimpleText has no windows open (so Activate event will not be sent) -
			// call AdjustMenus ourselves to have at right menus enabled
			if (FrontNonFloatingWindow() == nil) AdjustMenus(nil, true, false);
		}
		break;
	}
}
