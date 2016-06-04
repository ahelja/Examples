/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
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
 *  CarbonURLDownload.mm
 *  CarbonDownloader
 *
 *  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 */
 
#include "CarbonURLDownload.h"
#include "UCarbonEvent.h"

@implementation URLDownloadDelegate        

/* ----------------------------------------------- initWithCarbonURLDownload: */
- (id) initWithCarbonURLDownload: (CarbonURLDownload *) hostURLDownload
{
    id retVal = [super init];
    
    mHostURLDownload = hostURLDownload;
    
    return retVal;
}

/* ------------------------------------------------------ notifyThroughEvent: */
- (void) notifyThroughEvent: (EventRef) inCarbonEvent
{
    if(NULL != mHostURLDownload) {
        HIObjectRef objectToNotify = mHostURLDownload->GetObjectToNotify();
        if(NULL != objectToNotify) {
            SendEventToHIObject(inCarbonEvent, objectToNotify);
        }
    }
}

/* ------------------------- download:decideDestinationWithSuggestedFilename: */
- (void)download:(NSURLDownload *)download 
        decideDestinationWithSuggestedFilename:(NSString *)filename
{
    UCarbonEvent *suggestFilenameEvent = UCarbonEvent::CreateCarbonEvent(kEventClassCarbonURLDownload, kEventDownloadDecideDestination);
    if(NULL != suggestFilenameEvent) {
        CFURLRef fileURL = CFURLCreateWithFileSystemPath(NULL, (CFStringRef) filename, kCFURLPOSIXPathStyle, false);
        
        if(NULL != fileURL) {
            suggestFilenameEvent->SetParameter(kEventParamDirectObject, typeCarbonURLDownload, mHostURLDownload);
            suggestFilenameEvent->SetParameter(kEventParamSuggestedFileName, typeCFStringRef, (CFStringRef) filename);
            
            [self notifyThroughEvent: *suggestFilenameEvent];
            
            delete suggestFilenameEvent;
            suggestFilenameEvent = NULL;
            
            ::CFRelease(fileURL);
        }
    }
}

/* ------------------------------------------- download:didCreateDestination: */
- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{
    UCarbonEvent *destinationCreatedEvent = UCarbonEvent::CreateCarbonEvent(kEventClassCarbonURLDownload, kEventDownloadDestinationCreated);
    if(NULL != destinationCreatedEvent) {
        CFURLRef fileURL = CFURLCreateWithFileSystemPath(NULL, (CFStringRef) path, kCFURLPOSIXPathStyle, false);
        
        if(NULL != fileURL) {
            destinationCreatedEvent->SetParameter(kEventParamDirectObject, typeCarbonURLDownload, mHostURLDownload);
            destinationCreatedEvent->SetParameter(kEventParamFileObject, typeCFURLRef, fileURL);
            
            [self notifyThroughEvent: *destinationCreatedEvent];
            
            delete destinationCreatedEvent;
            destinationCreatedEvent = NULL;
            
            ::CFRelease(fileURL);
        }
    }
}

/* ----------------------------------------------- download:didFailWithError: */
- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
    UCarbonEvent *errorReceivedEvent = UCarbonEvent::CreateCarbonEvent(kEventClassCarbonURLDownload, kEventDownloadFailed);
    if(NULL != errorReceivedEvent) {
        CFStringRef errorString = reinterpret_cast<CFStringRef>([error localizedDescription]);
        errorReceivedEvent->SetParameter(kEventParamDirectObject, typeCarbonURLDownload, mHostURLDownload);
        errorReceivedEvent->SetParameter(kEventParamLocalizedDescription, typeCFStringRef, errorString);
        
        [self notifyThroughEvent: *errorReceivedEvent];
        
        delete errorReceivedEvent;
        errorReceivedEvent = NULL;
    }
}

/* ----------------------------------------- download:didReceiveDataOfLength: */
- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(unsigned)length
{
    UCarbonEvent *dataReceivedEvent = UCarbonEvent::CreateCarbonEvent(kEventClassCarbonURLDownload, kEventDownloadReceivedData);
    if(NULL != dataReceivedEvent) {
        dataReceivedEvent->SetParameter(kEventParamDirectObject, typeCarbonURLDownload, mHostURLDownload);
        dataReceivedEvent->SetParameter(kEventParamDownloadDataSize, typeUInt32, (UInt32) length);
        
        [self notifyThroughEvent: *dataReceivedEvent];
        
        delete dataReceivedEvent;
        dataReceivedEvent = NULL;
    }
}

/* --------------------------------------------- download:didReceiveResponse: */
- (void)download:(NSURLConnection *)download 
        didReceiveResponse:(NSURLResponse *)response
{
    if(NULL != mHostURLDownload) {
        // Set the response object on the host URL download object in case the 
        // HIObject notfied wants access to any of the data.
        mHostURLDownload->_SetResponse(response);
        
        // Send the carbon event indicating that the response was received.
        UCarbonEvent *responseEvent = UCarbonEvent::CreateCarbonEvent(kEventClassCarbonURLDownload, kEventDownloadReceivedResponse);
        if(NULL != responseEvent) {
            responseEvent->SetParameter(kEventParamDirectObject, typeCarbonURLDownload, mHostURLDownload);
            [self notifyThroughEvent: *responseEvent];
            delete responseEvent;
            responseEvent = NULL;
        }
    }
}

/* -------------------------------------------------------- downloadDidBegin: */
- (void)downloadDidBegin:(NSURLDownload *)download
{
    UCarbonEvent *beginEvent = UCarbonEvent::CreateCarbonEvent(kEventClassCarbonURLDownload, kEventDownloadDidBegin);
    if(NULL != beginEvent) {
        beginEvent->SetParameter(kEventParamDirectObject, typeCarbonURLDownload, mHostURLDownload);
        
        [self notifyThroughEvent: *beginEvent];
        
        delete beginEvent;
        beginEvent = NULL;
    }
}

/* ------------------------------------------------------- downloadDidFinish: */
- (void)downloadDidFinish:(NSURLDownload *)download
{
    UCarbonEvent *finishEvent = UCarbonEvent::CreateCarbonEvent(kEventClassCarbonURLDownload, kEventDownloadDidFinish);
    if(NULL != finishEvent) {
        finishEvent->SetParameter(kEventParamDirectObject, typeCarbonURLDownload, mHostURLDownload);
        
        [self notifyThroughEvent: *finishEvent];
        
        delete finishEvent;
        finishEvent = NULL;
    }
}

/* ------------------------------- download:shouldDecodeSourceDataOfMIMEType: */
- (BOOL)download:(NSURLDownload *)download 
        shouldDecodeSourceDataOfMIMEType:(NSString *)encodingType
{
    BOOL retVal = false;
    
    UCarbonEvent *shouldDecodeEvent = UCarbonEvent::CreateCarbonEvent(kEventClassCarbonURLDownload, kEventShouldDecodeMimeType);
    if(NULL != shouldDecodeEvent) {
        shouldDecodeEvent->SetParameter(kEventParamDirectObject, typeCarbonURLDownload, mHostURLDownload);
        shouldDecodeEvent->SetParameter(kEventParamMimeType, typeCFStringRef, (CFStringRef) encodingType);

        [self notifyThroughEvent: *shouldDecodeEvent];

        Boolean outValue;
        shouldDecodeEvent->GetParameter(kEventParamShouldDecode, typeBoolean, outValue);

        retVal = outValue;
        
        delete shouldDecodeEvent;
        shouldDecodeEvent = NULL;
    }
    
    return retVal;
}

@end


/* -------------------------------------------------- CreateCarbonURLDownload */
/* 
    Create and initialize a download object.  Returns NULL if either creation, 
    or initialization fails.
*/
CarbonURLDownload *CarbonURLDownload::CreateCarbonURLDownload(HIObjectRef objectToNotify, CFURLRef urlToDownload)
{
    CarbonURLDownload *retVal = NULL;
    
    require(NULL != objectToNotify, badArgs);
    require(NULL != urlToDownload, badArgs);
    
    try {
        retVal = new CarbonURLDownload(objectToNotify, urlToDownload);
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
    
    badArgs:
        return NULL;
}

/* ------------------------------------------------------------------- Cancel */
/*
    Cancel the current download 
*/
void CarbonURLDownload::Cancel()
{
    if(NULL != mDownloadObject) {
        [mDownloadObject cancel];
    }
}

/* ----------------------------------------------------------- SetDestination */
/*
    Set the destination for this download to the specified URL.
*/
void CarbonURLDownload::SetDestination(CFURLRef destinationURL, bool allowOverwrite)
{
    require(NULL != destinationURL, badArgs);

    if(NULL != mDownloadObject) {
        CFStringRef pathString = CFURLCopyFileSystemPath(destinationURL, kCFURLPOSIXPathStyle);
        
        [mDownloadObject setDestination:(NSString *)pathString allowOverwrite:(BOOL)allowOverwrite];
        
        CFRelease(pathString);
    }
    
badArgs:
    return;
}

/* ---------------------------------------------------- ExpectedContentLength */
/*
    Valid only after a response object has been obtained... return the 
    expected length of the content of this download.
*/
long long CarbonURLDownload::ExpectedContentLength()
{
    if(NULL != mResponse) {
        return [mResponse expectedContentLength];
    }
    
    return 0;
}

/* ------------------------------------------------------------- _SetResponse */
/*
    even though this routine is public, it is really only designed to be called
    from the Objective-C Delegate for the NSURLDownload object that this 
    C++ object represents.
*/
void CarbonURLDownload::_SetResponse(NSURLResponse *newResponse)
{
    if(NULL != mResponse) {
        [mResponse release];
    }
    
    mResponse = newResponse;
    
    if(NULL != mResponse) {
        [mResponse retain];
    }
}

/* -------------------------------------------------------- CarbonURLDownload */
CarbonURLDownload::CarbonURLDownload(
    HIObjectRef objectToNotify, 
    CFURLRef urlToDownload) :
        mDownloadObject(NULL),
        mResponse(NULL),
        mDelegate(NULL),
        mObjectToNotify(objectToNotify)
{
    /* 
        Note my desire to have the HIObject stick around 
    */
    if(NULL != objectToNotify) {
        CFRetain(objectToNotify);
    }
    
    /*
        Convert the CFURLRef that the caller asked us to download into an
        NSURLRequest.  From this we also create the NSURLDownload object
        for which this object is the C++ adapter.
    */
    if(NULL != urlToDownload) {
        NSURLRequest *urlRequest = [NSURLRequest requestWithURL: (NSURL *)urlToDownload];
        mDelegate = [[URLDownloadDelegate alloc] initWithCarbonURLDownload: this];
        
        if(NULL != urlRequest && NULL != mDelegate) {
            mDownloadObject = [[NSURLDownload alloc] initWithRequest: urlRequest delegate: mDelegate];
        } else {
            if(NULL != urlRequest) {
                [urlRequest release];
            }
            
            if(NULL != mDelegate) {
                [mDelegate release];
                mDelegate = NULL;
            }
        }
    }
}

/* ------------------------------------------------------- ~CarbonURLDownload */
CarbonURLDownload::~CarbonURLDownload()
{
    if(NULL != mDownloadObject) {
        [mDownloadObject release];
        mDownloadObject = NULL;
    }
    
    if(NULL != mResponse) {	
        [mResponse release];
        mResponse = NULL;
    }
    
    if(NULL != mDelegate) {
        [mDelegate release];
        mDelegate = NULL;
    }
    
    if(NULL != mObjectToNotify) {
        ::CFRelease(mObjectToNotify);
        mObjectToNotify = NULL;
    }
}

/* --------------------------------------------------------------- Initialize */
OSStatus CarbonURLDownload::Initialize()
{
    OSStatus retVal = noErr;

    if(NULL != mResponse) {
        [mResponse release];
        mResponse = NULL;
    }
    
    if(NULL == mDownloadObject || NULL == mDelegate || NULL == mObjectToNotify) {
        retVal = -1;
    }
    
    return retVal;
}
