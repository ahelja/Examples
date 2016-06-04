/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
 *  DownloadWindow.h
 *  CarbonDownloader
 *
 *  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>

#pragma once

#ifndef _H_DownloadWindow
#define _H_DownloadWindow

#include "UCarbonEvent.h"
#include "CarbonURLDownload.h"

class DownloadWindow
{
    public:
        /* identifiers for controls that appear on the download window */
        static const OSType kURLFieldSignature = 'URL ';
        static const OSType kProgressBarSignature = 'PROG';
        static const OSType kDownloadLocationSignature = 'DESK';
        static const OSType kDecodeSignature = 'DCOD';
        static const OSType kOpenWhenDoneSignature = 'OPNC';
        static const OSType kDownloadButtonSignature = 'GO  ';

        static DownloadWindow *CreateDownloadWindow();
        virtual ~DownloadWindow();
    
    protected:
        DownloadWindow();

        OSStatus Initialize();
        OSStatus SetupControls();
        OSStatus HandleCarbonEvent(EventHandlerCallRef inCallRef, UCarbonEvent &inEvent);
        OSStatus HandleControlEvent(EventHandlerCallRef inCallRef, UCarbonEvent &inEvent);
        OSStatus HandleURLDownloadEvent(EventHandlerCallRef inCallRef, UCarbonEvent &inEvent);
        OSStatus HandleControlHit(ControlRef controlHit, ControlPartCode partHit);
        
    private:
        WindowRef 		mMacWindow;
        EventHandlerRef 	mWindowEventHandler;
        Boolean 		mPromptForSaveLocation;
        Boolean			mDownloading;			// True while data is being received
        CarbonURLDownload 	*mCurrentDownload;
        long long		mDataReceived;			// How much data has been received so far.
        
        void SetPromptForSaveLocation(bool shouldPrompt);
        void SetDownloadInProgress(CarbonURLDownload *newDownload);
        void SetDownloadButtonTitle();
        
        void SetDownloadDestinationWithPutDialog(CFStringRef suggestedFilename);
        static NavEventUPP gPutFileDialogEventUPP;
        static pascal void SaveDialogEventProc(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, CarbonURLDownload *downloadToSet);
        
        CFURLRef CreateDesktopDestinationURL(CFStringRef suggestedFilename);
        
        static EventHandlerUPP gDownloadWindowEventHandlerUPP;
        static pascal OSStatus DownloadWindowEventHandler(EventHandlerCallRef inCallRef, EventRef inEvent, DownloadWindow *windowToNotify);
};

#endif // _H_DownloadWindow
