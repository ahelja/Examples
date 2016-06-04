/*
 *  DownloadWindow.cpp
 *  CarbonDownloader
 *
 *  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 */
 
#include "DownloadWindow.h"

static const ControlID kURLFieldID  = { DownloadWindow::kURLFieldSignature, 0 };
static const ControlID kDownloadButtonID = { DownloadWindow::kDownloadButtonSignature, 0 };
static const ControlID kProgressBarID = { DownloadWindow::kProgressBarSignature, 0 };
static const ControlID kDecodeCheckboxID = { DownloadWindow::kDecodeSignature, 0 };

EventHandlerUPP DownloadWindow::gDownloadWindowEventHandlerUPP = 
    NewEventHandlerUPP((EventHandlerProcPtr)DownloadWindow::DownloadWindowEventHandler);

NavEventUPP DownloadWindow::gPutFileDialogEventUPP =
    NewNavEventUPP((NavEventProcPtr)DownloadWindow::SaveDialogEventProc);
    
/* ----------------------------------------------------- CreateDownloadWindow */
/*
    Factory method for creating download windows.  This method insures
    that the window is initialized before it is returned to the caller.
*/
DownloadWindow *DownloadWindow::CreateDownloadWindow()
{
    DownloadWindow *retVal = NULL;

    try {
        // create a new download window object and try to initialize it.
        // if initialization fails, then delete the object and return
        // NULL.
        retVal = new DownloadWindow();
        if(NULL != retVal) {
            OSStatus initStatus = retVal->Initialize();
            if(noErr != initStatus) {
                delete retVal;
                retVal = NULL;
            }
        }
    }

    catch(...) {
        if(NULL != retVal) {
            delete retVal;
            retVal = NULL;
        }
    }

    return retVal;
}

/* ---------------------------------------------------------- ~DownloadWindow */
DownloadWindow::~DownloadWindow()
{
    // Dispose of the macintosh window (if one exists... and it should)
    check(NULL != mMacWindow);
    if(NULL != mMacWindow) {
        ::DisposeWindow(mMacWindow);
        mMacWindow = NULL;
    }
}

/* ----------------------------------------------------------- DownloadWindow */
/*
    Constructor for download window objects.  This method is protected to
    make it more difficult to create instances of DownloadWindow directly.
    
    After creating a DownloadWindow with the constructor, you should call
    Initialize on that window object to finish the construction.
*/
DownloadWindow::DownloadWindow() :
    mMacWindow(NULL),
    mWindowEventHandler(0),
    mPromptForSaveLocation(false),
    mCurrentDownload(NULL)
{
}

/* --------------------------------------------------------------- Initialize */
/*
    Finish the construction of a DownloadWindow object by loading the actual
    window associated with the object from the NIB file and setting up the
    carbon event handlers for the window and its controls.
*/
OSStatus DownloadWindow::Initialize()
{
    IBNibRef nibRef = NULL;
    OSStatus retVal = noErr;
    
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    retVal = ::CreateNibReference(CFSTR("main"), &nibRef);
    if(noErr == retVal) {    
        // Then create a window. "MainWindow" is the name of the window object. This name is set in 
        // InterfaceBuilder when the nib is created.
        retVal = ::CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &mMacWindow);

        // We don't need the nib reference anymore.
        ::DisposeNibReference(nibRef);
        nibRef = NULL;
    }
    
    if(NULL != mMacWindow && noErr == retVal) {
        const EventTypeSpec handledEvents[] = {
            { kEventClassControl, kEventControlHit },
            { kEventClassCarbonURLDownload, kEventDownloadDidBegin },
            { kEventClassCarbonURLDownload, kEventDownloadDidFinish },
            { kEventClassCarbonURLDownload, kEventDownloadFailed },
            { kEventClassCarbonURLDownload, kEventDownloadReceivedData },
            { kEventClassCarbonURLDownload, kEventDownloadDestinationCreated },
            { kEventClassCarbonURLDownload, kEventDownloadDecideDestination },
            { kEventClassCarbonURLDownload, kEventDownloadReceivedResponse },
            { kEventClassCarbonURLDownload, kEventShouldDecodeMimeType },
        };

        // install the handler that will call our HandleCarbonEvent method
        retVal = InstallWindowEventHandler(
                        mMacWindow, 
                        gDownloadWindowEventHandlerUPP,
                        GetEventTypeCount(handledEvents),
                        handledEvents,
                        (void *) this,
                        &mWindowEventHandler);


        require_noerr(retVal, initFailed);

        // initialize the controls on the window
        retVal = SetupControls();
        require_noerr(retVal, initFailed);

        ::ShowWindow( mMacWindow );
    }

    return retVal;
    
initFailed:
    return retVal;
}

/* ------------------------------------------------------------ SetupControls */
/*
    Initialize the settings on the controls in a download window.
*/
OSStatus DownloadWindow::SetupControls()
{
    OSStatus 	retVal = noErr;
    ControlRef  urlEditField = NULL;
    Boolean 	singleLine = true;

    /* Set the single line control flag on the edit field */
    retVal = ::GetControlByID(mMacWindow, &kURLFieldID, &urlEditField);
    require_noerr(retVal, setupFailed);
    require(NULL != urlEditField, setupFailed);
    retVal = ::SetControlData(
        urlEditField,
        kControlEntireControl,
        kControlEditTextSingleLineTag,
        sizeof(Boolean),
        &singleLine);
    require_noerr(retVal, setupFailed);
    
    SetDownloadInProgress(NULL);

    return retVal;
    
setupFailed:
    return retVal;
}

/* -------------------------------------------------------- HandleCarbonEvent */
/*
    Carbon event handler for a specific instance of DownloadWindow.  This
    method is usually called from the static WindowEventHandler for the
    DownloadWindow class
*/
OSStatus DownloadWindow::HandleCarbonEvent(
    EventHandlerCallRef inCallRef, 
    UCarbonEvent &inEvent)
{
    OSStatus retVal = eventNotHandledErr;
    
    switch(::GetEventClass(inEvent)) {
        case kEventClassControl : 
            retVal = HandleControlEvent(inCallRef, inEvent);
        break;
        
        case kEventClassCarbonURLDownload :
            retVal = HandleURLDownloadEvent(inCallRef, inEvent);
        break;
        
        default:
            retVal = eventNotHandledErr;
    }
    
    return retVal;
}

/* ------------------------------------------------------- HandleControlEvent */
/*
    Handle an event of kEventClassControl.  Mostly this referrs to hits
    in the DownloadWindow's control
*/
OSStatus DownloadWindow::HandleControlEvent(
    EventHandlerCallRef inCallRef, 
    UCarbonEvent &inEvent)
{
    OSStatus retVal = eventNotHandledErr;

    switch(::GetEventKind(inEvent)) {
        case kEventControlHit : {
            ControlRef whichControl = NULL;
            ControlPartCode   whichPart = kControlNoPart;
            
            retVal = ::GetEventParameter(
                            inEvent,
                            kEventParamDirectObject,
                            typeControlRef, NULL,
                            sizeof(ControlRef), NULL,
                            &whichControl);
            require_noerr(retVal, errorOccurred);

            retVal = ::GetEventParameter(
                            inEvent,
                            kEventParamControlPart,
                            typeControlPartCode, NULL,
                            sizeof(ControlPartCode), NULL,
                            &whichPart);
            require_noerr(retVal, errorOccurred);

            retVal = HandleControlHit(whichControl, whichPart);
        } break;

        default:
            retVal = eventNotHandledErr;
    }

    return retVal;
    
errorOccurred:
    
    return retVal;
}

/* --------------------------------------------------------- HandleControlHit */
/*
    Handle the behavior when one of the controls in the download window is
    manipulated by the user.
*/
OSStatus DownloadWindow::HandleControlHit(ControlRef controlHit, ControlPartCode partHit)
{
    ControlID controlHitID;
    OSStatus retVal = eventNotHandledErr;
    
    retVal = ::GetControlID(controlHit, &controlHitID);
    require_noerr(retVal, handlingFailed);
    
    switch(controlHitID.signature) {
        case kDownloadButtonSignature : {
            /*
                The download button's behavior is a bit convoluted because the same button 
                (relabeled) also serves as the cancel button for a given download.
            */
            if(mCurrentDownload != NULL) {
                /* 
                    If a download is already in progress, then we must be a cancel button
                   and should cancel the download.
                */
                mCurrentDownload->Cancel();
                SetDownloadInProgress(NULL);
            } else {
                ControlRef urlField = NULL;
                CFStringRef urlText = NULL;
                CFURLRef    urlToDownload = NULL;
                
                // grab the text of the URL field and try to create a URL request for it.
                retVal = ::GetControlByID(mMacWindow, &kURLFieldID, &urlField);
                require_noerr(retVal, handlingFailed);
                require(NULL != urlField, handlingFailed);
            
                ::GetControlData(
                    urlField,
                    kControlEditTextPart,
                    kControlEditTextCFStringTag,
                    sizeof(CFStringRef),
                    &urlText,
                    NULL);
    
                urlToDownload = CFURLCreateWithString(NULL, urlText, NULL);
                if(NULL != urlToDownload) {
                    CarbonURLDownload *urlDownloadObject = CarbonURLDownload::CreateCarbonURLDownload((HIObjectRef) mMacWindow, urlToDownload);
    
                    /* store the new object in this object */
                    SetDownloadInProgress(urlDownloadObject);
    
                    /* Release the ref count from the create call */
                    urlDownloadObject->Release();
                    
                    ::CFRelease(urlToDownload);
                }
                
                ::CFRelease(urlText);
            }
            
            retVal = noErr;
        } break;
        
        case kDownloadLocationSignature : {
            /* 
                We watch the state of the download location radio group and use
                it to manipulate the "prompt for location" behavior. 
            */
            SInt32 controlValue = ::GetControl32BitValue(controlHit);
            SetPromptForSaveLocation(controlValue == 2);
            
            retVal = noErr;
        } break;
    }
    
    return retVal;
    
handlingFailed:
    return retVal;
}

/* --------------------------------------------------- HandleURLDownloadEvent */
OSStatus DownloadWindow::HandleURLDownloadEvent(
    EventHandlerCallRef inCallRef, 
    UCarbonEvent &inEvent)
{
    OSStatus retVal = eventNotHandledErr;
    
    switch(GetEventKind(inEvent)) {
        case kEventDownloadDidBegin :
            mDataReceived = 0;
            retVal = noErr;
        break;
        
        case kEventDownloadDidFinish :
            SetDownloadInProgress(NULL);
            retVal = noErr;
        break;
        
        case kEventDownloadFailed : {
            /*
                Our download failed behavior is pretty cheesy.  It simply puts up a sheet
                that (hopefully) describes why the download failed.
            */
            DialogRef alertDialog = NULL;
            CFStringRef errorString = NULL;

            CFStringRef downloadFailedString = CFCopyLocalizedString(CFSTR("Download Failed"), CFSTR("Error title when a download fails"));
            
            inEvent.GetParameter(kEventParamLocalizedDescription, typeCFStringRef, errorString);
            
            OSStatus createDialogErr = CreateStandardSheet(
                                            kAlertStopAlert,
                                            downloadFailedString,
                                            errorString,
                                            NULL,
                                            NULL,
                                            &alertDialog);

            if(noErr == createDialogErr && NULL != alertDialog) {
                ShowSheetWindow(GetDialogWindow(alertDialog), mMacWindow);
            }
            
            /* if the download failed, I guess we don't have one anymore */
            SetDownloadInProgress(NULL);
            retVal = noErr;
        } break;
        
        case kEventDownloadReceivedData : {
            /* We got some data so we update the progress bar. */
            UInt32 newDataVolume = 0;
            
            inEvent.GetParameter(kEventParamDownloadDataSize, typeUInt32, newDataVolume);
            mDataReceived = mDataReceived + newDataVolume;
            
            if(newDataVolume > 0 && mCurrentDownload->HasResponse()) {
                ControlRef progressBar = NULL;
                OSStatus   err = noErr;
                long long expectedData = mCurrentDownload->ExpectedContentLength();

                err = ::GetControlByID(mMacWindow, &kProgressBarID, &progressBar);
                if(noErr == err && NULL != progressBar) {
                    double dataReceived = (double) mDataReceived / (double) expectedData;
                    SetControl32BitValue(progressBar, (long)(1000 * dataReceived));
                }
                
                retVal = noErr;
            }
        } break;
        
        case kEventDownloadDestinationCreated :
        break;
        
        case kEventDownloadDecideDestination : {
            /* 
                In response to the decide destination event, we need to 
                call SetDestination on the CarbonURLDownload object 
            */
            CarbonURLDownload *downloadToChange = NULL;
            CFStringRef suggestedName = NULL;
            
            inEvent.GetParameter(kEventParamDirectObject, typeCarbonURLDownload, downloadToChange);
            inEvent.GetParameter(kEventParamSuggestedFileName, typeCFStringRef, suggestedName);
            
            if(NULL != downloadToChange && NULL != suggestedName) {
                if(mPromptForSaveLocation) {
                    // Ask the user to select a destination
                    SetDownloadDestinationWithPutDialog(suggestedName);
                } else {
                    // Create a destination that is on the user's desktop
                    CFURLRef destination = CreateDesktopDestinationURL(suggestedName);
                    if(NULL != destination) {
                        downloadToChange->SetDestination(destination, true);
                    }                    
                }

                retVal = noErr;
            }
        } break;
        
        case kEventDownloadReceivedResponse : {
            /*
                After we receive a response we may be able to tell how much data we
                have to download.  To that end, we update the progress bar to from 
                indeterminate to determinate and begin showing actual progress 
                information
            */
            ControlRef progressBar;
            OSStatus   err;

            err = ::GetControlByID(mMacWindow, &kProgressBarID, &progressBar);
            if(noErr == err && NULL != progressBar) {
                long long expectedData = mCurrentDownload->ExpectedContentLength();

                if(expectedData > 0) {
                    Boolean shouldAnimate = true;
                    Boolean isIndeterminate = false;

                    ::SetControlData(
                        progressBar,
                        kControlEntireControl,
                        kControlProgressBarIndeterminateTag,
                        sizeof(Boolean),
                        &isIndeterminate);

                    // Turn on the progress animation
                    ::SetControlData(
                        progressBar,
                        kControlEntireControl,
                        kControlProgressBarAnimatingTag,
                        sizeof(Boolean),
                        &shouldAnimate);

                    double dataReceived = (double) mDataReceived / (double) expectedData;

                    SetControl32BitValue(progressBar, (long)(1000 * dataReceived));
                }

                retVal = noErr;
            } break;

            case kEventShouldDecodeMimeType : {
                // Normally one would extract the mime type parameter of the event
                // and examine it to determine if we want that type decoded... in this case
                // we just return the value of the decode checkbox.
                ControlRef decodeCheckbox = NULL;
                Boolean	   shouldDecode = false;
                OSErr getErr = ::GetControlByID(mMacWindow, &kDecodeCheckboxID, &decodeCheckbox);
                if(noErr == getErr) {
                    shouldDecode = GetControl32BitValue(decodeCheckbox);
                }

                inEvent.SetParameter(kEventParamShouldDecode, typeBoolean, shouldDecode);

                retVal = noErr;
            } break;
        } break;
    }

    return retVal;
}

/* ------------------------------------------------- SetPromptForSaveLocation */
/*
    Remember whether or not we're supposed to save the location and set the 
    title on the download button appropriately
*/
void DownloadWindow::SetPromptForSaveLocation(bool shouldSave)
{
    mPromptForSaveLocation = shouldSave;
    SetDownloadButtonTitle();
}

/* ---------------------------------------------------- SetDownloadInProgress */
/*
    Change the object that is tracking the download we are currently processing
    
    We may be changing the object to "NULL" (meaning there is no current
    download in progress).
*/
void DownloadWindow::SetDownloadInProgress(CarbonURLDownload *newDownload)
{
    ControlRef progressBar = NULL;
    OSStatus   err = noErr;
    Boolean shouldAnimate = false;
    Boolean isIndeterminate = false;
  
    if(NULL != mCurrentDownload) {
        mCurrentDownload->Release();
        mCurrentDownload = NULL;
    }
    
    mCurrentDownload = newDownload;
    
    if(mCurrentDownload) {
        mCurrentDownload->Retain();
    }
        
    shouldAnimate = NULL != mCurrentDownload;
    isIndeterminate = NULL != mCurrentDownload;

    /* set up the progress bar to reflect the current state of the download. */
    err = ::GetControlByID(mMacWindow, &kProgressBarID, &progressBar);
    if(noErr == err && NULL != progressBar) {
        SetControl32BitMinimum(progressBar, 0);
        SetControl32BitMaximum(progressBar, 1000);
        SetControl32BitValue(progressBar, 0);

        ::SetControlData(
            progressBar,
            kControlEntireControl,
            kControlProgressBarIndeterminateTag,
            sizeof(Boolean),
            &isIndeterminate);

        // Turn on the progress animation
        ::SetControlData(
            progressBar,
            kControlEntireControl,
            kControlProgressBarAnimatingTag,
            sizeof(Boolean),
            &shouldAnimate);

        if(isIndeterminate) {
            SetControl32BitValue(progressBar, 500);
        }
    }
    
    SetDownloadButtonTitle();
}

/* --------------------------------------------------- SetDownloadButtonTitle */
/*
    If there is a download in progress, the Download button's title changes
    to "Cancel".  Otherwise it will show Download with or without an elipses
    depending on whether or not clicking the button will bring up a save file
    dialog.
*/
void DownloadWindow::SetDownloadButtonTitle()
{
    CFStringRef buttonTitle;    
    ControlRef 	downloadButton = NULL;

    ::GetControlByID(mMacWindow, &kDownloadButtonID, &downloadButton);

    if(NULL != mCurrentDownload) {
        SetWindowCancelButton(mMacWindow, downloadButton);
        SetWindowDefaultButton(mMacWindow, NULL);
        buttonTitle = CFCopyLocalizedString(CFSTR("Cancel"), CFSTR("Download button Title during download"));
    } else {
        SetWindowCancelButton(mMacWindow, NULL);
        SetWindowDefaultButton(mMacWindow, downloadButton);

        /* 
            Note.  Because CFSTR gripes if we use an elipses here, we use three periods in the "key" string 
            for CFCopyLocalizedString. When the string is actually loaded, however, the replacement SHOULD 
            have an elipses in the place of the three 
            periods.
        */
        if(mPromptForSaveLocation) {
            buttonTitle = CFCopyLocalizedString(CFSTR("Download..."), CFSTR("Download Button Title using a PutFile dialog to get the destination"));
        } else {
            buttonTitle = CFCopyLocalizedString(CFSTR("Download"), CFSTR("Download Button Title using the Desktop as the destination"));
        }
    }
    
    if(NULL != buttonTitle && NULL != downloadButton) {
        ::SetControlTitleWithCFString(downloadButton, buttonTitle);
        ::CFRelease(buttonTitle);
        buttonTitle = NULL;
    }
}

/* -------------------------------------- SetDownloadDestinationWithPutDialog */
/*
    Put up a "put file" dialog to select a destination for a download.
*/
void DownloadWindow::SetDownloadDestinationWithPutDialog(
    CFStringRef suggestedFilename)
{
    if(NULL != mCurrentDownload) {
        NavDialogRef putFileDialog = NULL;
        NavDialogCreationOptions dialogOptions;
    
        memset(&dialogOptions, 0, sizeof(NavDialogCreationOptions));
        
        dialogOptions.version = 0;
        dialogOptions.optionFlags = 0;
        dialogOptions.location.h = dialogOptions.location.v = -1;
        dialogOptions.clientName = CFSTR("Carbon Downloader");
        dialogOptions.windowTitle = NULL;
        dialogOptions.actionButtonLabel = NULL;
        dialogOptions.cancelButtonLabel = NULL;
        dialogOptions.saveFileName = suggestedFilename;
        dialogOptions.message = NULL;
        dialogOptions.preferenceKey = 1;
        dialogOptions.popupExtension = NULL;
        dialogOptions.modality = kWindowModalityWindowModal;
        dialogOptions.parentWindow = mMacWindow;
        
        /*
            The nav dialog is going to talk to the download object in it's 
            completion proc (the PutFileDialogEventUPP).  We add a refcount
            to the download object here and that refcount is removed when the
            event proc finishes with the download object.
        */
        mCurrentDownload->Retain();
        
        OSStatus createDialogErr = ::NavCreatePutFileDialog(
                                        &dialogOptions,
                                        0,
                                        0,
                                        gPutFileDialogEventUPP,
                                        mCurrentDownload,			// even proc should fill in the URL for us when the dialog is dismissed
                                        &putFileDialog);
    
        if(noErr == createDialogErr && NULL != putFileDialog) {
            NavDialogRun(putFileDialog);
        }
    }
}

/* ------------------------------------------------------ SaveDialogEventProc */
/*
    Navigation Services event proc used by the "put file" dialog which 
    the application presents the user when selecting a download location
*/
pascal void DownloadWindow::SaveDialogEventProc(
    NavEventCallbackMessage callBackSelector, 
    NavCBRecPtr callBackParms, 
    CarbonURLDownload *downloadToSet)
{
    switch(callBackSelector) {
        case kNavCBUserAction : {
            NavUserAction userAction = NavDialogGetUserAction(callBackParms->context);
            switch(userAction) {
                /* 
                    when the user cancels the save we need to be sure to clean up the
                    ref count that the dialog had on the download object that 
                    prompted it.
                */
                case kNavUserActionCancel : {
                    if(NULL != downloadToSet) {
                        downloadToSet->Cancel();
                        downloadToSet->Release();
                        downloadToSet = NULL;
                    }
                } break;
                
                /* User hit the save button */
                case kNavUserActionSaveAs: {
                    NavReplyRecord reply;
                    ::NavDialogGetReply(callBackParms->context, &reply);
                    if(reply.validRecord) {
                        OSStatus gotFSRefErr = noErr;
                        AEDesc directoryDesc = { typeNull, NULL };
                        FSRef fileDirectory;
                        
                        /* 
                            Since this is a save dialog, the directory into which we should save is
                            the first item in the list of selected items returned.
                        */
                            
                        OSStatus getDescErr = ::AEGetNthDesc(&reply.selection, 1, typeFSRef, NULL, &directoryDesc);
                        if(noErr == getDescErr) {
                            gotFSRefErr = ::AEGetDescData(&directoryDesc, &fileDirectory, sizeof(FSRef));
                            ::AEDisposeDesc(&directoryDesc);
                        }
        
                        /*
                            If we successfully found an output directory then combine that with the 
                            file name to create a complete destination file.
                        */
                        if(noErr == gotFSRefErr && NULL != downloadToSet) {
                            CFURLRef destinationFolderURL = ::CFURLCreateFromFSRef(NULL, &fileDirectory);
                            CFURLRef destination = ::CFURLCreateCopyAppendingPathComponent(NULL, destinationFolderURL, reply.saveFileName, false);
                            
                            downloadToSet->SetDestination(destination, reply.replacing);
                            
                            // Undo the retain that was done when the nav dialog was brought up.
                            // (see SetDownloadDestinationWithPutDialog)
                            downloadToSet->Release();

                            ::CFRelease(destination);
                            ::CFRelease(destinationFolderURL);
                        }
                    }

                    ::NavDisposeReply(&reply);
                } break;
            }            
        } break;
    }
}

/* ---------------------------------------------- CreateDesktopDestinationURL */
/*
    Look up the current users's desktop folder and create a CFURL for a file
    on the desktop with the suggested file name.
*/
CFURLRef DownloadWindow::CreateDesktopDestinationURL(CFStringRef suggestedFilename)
{
    CFURLRef retVal = NULL;
    if(NULL != suggestedFilename) {
        FSRef desktopFolderRef;

        OSStatus findDesktopErr = ::FSFindFolder(
                                        kUserDomain,
                                        kDesktopFolderType,
                                        false,
                                        &desktopFolderRef);

        if(noErr == findDesktopErr) {
            CFURLRef desktopFolderURL = ::CFURLCreateFromFSRef(NULL, &desktopFolderRef);
            if(NULL != desktopFolderURL) {

                retVal = ::CFURLCreateCopyAppendingPathComponent(NULL, desktopFolderURL, suggestedFilename, false);
                check(NULL != retVal);

                ::CFRelease(desktopFolderURL);
            } 
        }
    }

    return retVal;
}

/* ------------------------------------------------------- WindowEventHandler */
/*
    Static class method for catching carbon events.   By in large, the 
    carbon events are simply forwarded to as specicific instance of 
    DownloadWindow.
*/
pascal OSStatus DownloadWindow::DownloadWindowEventHandler(
    EventHandlerCallRef inCallRef, 
    EventRef inEvent, 
    DownloadWindow *windowToNotify)
{
    OSStatus retVal = eventNotHandledErr;
    
    // pass the carbon event on to the specific instance that needs
    // to know about it.
    if(NULL != windowToNotify) {
        UCarbonEvent eventToHandle(inEvent);
        retVal = windowToNotify->HandleCarbonEvent(inCallRef, eventToHandle);
    }
    
    return retVal;
}
