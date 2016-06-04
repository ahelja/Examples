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
 *  URLDownload.h
 *  CarbonDownloader
 *
 *  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>
#include "RefCounted.h"

/* ============================================ CarbonURLDownload event suite */
/*
    Definitions for the carbon event suite that parallels
    NSURLDownload's Delegate profile
*/
const EventClass kEventClassCarbonURLDownload = 'UrlD';

enum {
    kEventParamLocalizedDescription = 'Desc',
    kEventParamDownloadDataSize = 'DDSz',
    kEventParamSuggestedFileName = 'SFNm',
    kEventParamFileObject = 'FilO',
    kEventParamMimeType = 'MIME',
    kEventParamShouldDecode = 'DCod'
};

enum {
    kEventDownloadDidBegin = 1001,
    kEventDownloadDidFinish,
    kEventDownloadFailed,
    kEventDownloadReceivedData,
    kEventDownloadDestinationCreated,
    kEventDownloadDecideDestination,
    kEventDownloadReceivedResponse,
    kEventShouldDecodeMimeType
};

enum {
    typeCarbonURLDownload = 'CULD',
    typeCFURLRef = 'cfUl'
};

/*
    Parameters for Download events:

    kEventDownloadDidBegin
        -->     kEventParamDirectObject     		typeCarbonURLDownload

    kEventDownloadDidFinish,
        -->     kEventParamDirectObject     		typeCarbonURLDownload
        
    kEventDownloadFailed,
        -->     kEventParamDirectObject     		typeCarbonURLDownload
        -->	kEventParamLocalizedDescription		typeCFStringRef
        
    kEventDownloadReceivedData
        -->     kEventParamDirectObject     		typeCarbonURLDownload
        -->	kEventParamDownloadDataSize		typeUInt32
        
    kEventDownloadDestinationCreated,
        -->     kEventParamDirectObject     		typeCarbonURLDownload
        -->	kEventParamFileObject			typeCFURLRef
        
    kEventDownloadDecideDestination
        -->     kEventParamDirectObject     		typeCarbonURLDownload
        -->     kEventParamSuggestedFileName     	typeCFStringRef
        
    kEventDownloadReceivedResponse
        -->     kEventParamDirectObject     		typeCarbonURLDownload
        
    kEventShouldDecodeMimeType
        --> 	kEventParamDirectObject			typeCarbonURLDownload
        -->	kEventParamMimeType			typeCFStringRef
        <--	kEventParamShouldDecode			typeBoolean
*/

/* ====================================================== URLDownloadDelegate */
/*
    This class is used as the delegate for the NSURLDownload object 
    contained within a CarbonURLDownload object.  The delegate methods
    transfer their parameters into Carbon events and forward those events
    to the HIObjectRef that is stored as the target of the CarbonURLDownload
    
    The __OBJC__ preprocessor construct is something of a workaround.  It
    allows this header file to be included into C++ files that are not 
    compiled with the Objective-C++ compiler.
*/
#if __OBJC__
    #include<WebKit/WebKit.h>

    class CarbonURLDownload;
    
    @interface URLDownloadDelegate : NSObject
        {
            CarbonURLDownload *mHostURLDownload;
        };
        
        - (id) initWithCarbonURLDownload: (CarbonURLDownload *) mHostURLDownload;
    
        /* Utility method for sending carbon events */
        - (void) notifyThroughEvent: (EventRef) inCarbonEvent;

        /* Delegate methods for NURLDownload */
        
        - (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename;
        - (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path;
        - (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error;
        - (void)download:(NSURLDownload *)download didReceiveDataOfLength:(unsigned)length;
        - (void)download:(NSURLConnection *)download didReceiveResponse:(NSURLResponse *)response;
        - (void)downloadDidBegin:(NSURLDownload *)download;
        - (void)downloadDidFinish:(NSURLDownload *)download;
        - (BOOL)download:(NSURLDownload *)download shouldDecodeSourceDataOfMIMEType:(NSString *)encodingType;
    @end
#else
    class NSURLDownload;
    class NSURLResponse;
    class URLDownloadDelegate;
#endif


/* ======================================================== CarbonURLDownlaod */
/* 
    A C++ facade for an NSURLDownlaod object and, it's delegate, and some of 
    the methods of the NSURLResponse object methods that is generated as part 
    of the download process. This object also stores an HIObjectRef for an
    object that is to receive the carbon suite of this class (defined above).
    
    CarbonURLDownload objects inherit reference counting through 
    ConcreteRefCount
*/
class CarbonURLDownload : public ConcreteRefCount
{
    public:
        static CarbonURLDownload *CreateCarbonURLDownload(HIObjectRef objectToNotify, CFURLRef urlToDownload);
        virtual ~CarbonURLDownload();
        
        HIObjectRef GetObjectToNotify() { return mObjectToNotify; }
        
        void Cancel();
        void SetDestination(CFURLRef destinationURL, bool allowOverwrite);
        
        /* 
            These methods only return valid data after the download has constructed
            the response objects.  Clients of the download can determine when this
            happens by looking for the kEventDownloadReceivedResponse notification 
        */
        bool	  HasResponse() { return mResponse != NULL; }
        long long ExpectedContentLength();

        /* 
            This method is called by the Objective-C delegate.  Normally it wouldn't be
            public, but must be to allow Objective-C to call it. 
        */
        void _SetResponse(NSURLResponse *newResponse);
        
    protected:
        CarbonURLDownload(HIObjectRef objectToNotify, CFURLRef urlToDownload);

        OSStatus Initialize();
        
    private:
        NSURLDownload 		*mDownloadObject;
        NSURLResponse 		*mResponse;
        URLDownloadDelegate 	*mDelegate;
        HIObjectRef 		mObjectToNotify;
};

