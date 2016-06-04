/*
	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.

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

#import "URLAccessHandle.h"
#import <Carbon/Carbon.h>

static NSMutableArray *urls;
static NSMutableDictionary *cachedURLs;
static NSString *kIdleTime = @"kIdleTime";
static URLNotifyUPP notifyUPP;

@interface URLAccessHandle(URLAccessEventHandling)
- (OSStatus)handleEvent:(URLEvent)event info:(URLCallbackInfo *)info;
- (void)processDownloadEvent;
@end

static OSStatus newURLEvent(void *self, URLEvent event, URLCallbackInfo *info) {
    return [(URLAccessHandle *)self handleEvent:event info:info];
}

static void downloadEvent(void *self) {
    [(URLAccessHandle *)self processDownloadEvent];
}

@implementation URLAccessHandle

+ (void)initialize {
    static BOOL done = NO;
    if (!done) {
        done = YES;
        urls = [NSMutableArray new];
        cachedURLs = [NSMutableDictionary new];
        [NSURLHandle registerURLHandleClass:self]; // This is in general not a safe way to register oneself.  However, we know that we will receive a +acceptURL: message beore we are first asked to handle a URL, so it's safe for us.
        notifyUPP = NewURLNotifyUPP(newURLEvent);
    }
}

+ (void)acceptURL:(NSURL *)url {
    [urls addObject:url];
}

+ (BOOL)canInitWithURL:(NSURL *)anURL {
    if ([urls indexOfObject:anURL] != NSNotFound) {
        return YES;
    } else {
        return NO;
    }
}

+ (NSURLHandle *)cachedHandleForURL:(NSURL *)anURL {
    NSURLHandle *handle = [cachedURLs objectForKey:anURL];
    if (!handle) {
        handle = [[self alloc] initWithURL:anURL cached:YES];
        [cachedURLs setObject:handle forKey:anURL];
    }
    return handle;
}

- initWithURL:(NSURL *)anURL cached:(BOOL)willCache {
    if (self = [super initWithURL:anURL cached:willCache]) {
        int index;
        const char *cString;
        _url = [[anURL absoluteURL] retain];
        cString = [[_url absoluteString] UTF8String];
        if (!cString || URLNewReference(cString, &_urlRef) != noErr) {
            [_url release];
            [self release];
            self = nil;
        } else {
            _lock = [[NSLock alloc] init];
            _rlSource = NULL;
            _messageQueue = [[NSMutableArray alloc] init];
        }
        // Do this regardless of whether initialization succeeded
        index = [urls indexOfObject:anURL];
        if (index != NSNotFound) {
            // Remove anURL from the urls array so that future requests won't always be handled by us
            [urls removeObjectAtIndex:index];
        }
    }
    return self;
}

- (void)dealloc {
    [_url release];
    URLDisposeReference(_urlRef);
    [_lock release];
    if (_rlSource) CFRelease(_rlSource);
    [_messageQueue release];
    [super dealloc];
}

// No properties supported
- (id)propertyForKey:(NSString *)propertyKey {
    return nil;
}

- (id)propertyForKeyIfAvailable:(NSString *)propertyKey {
    return nil;
}

- (BOOL)writeProperty:(id)propertyValue forKey:(NSString *)propertyKey {
    return NO;
}

- (BOOL)writeData:(NSData *)data {
    return NO;
}

- (NSData *)loadInForeground {
    Handle myHandle = NewHandle(512);
    NSData *result = NULL;
    if (URLSimpleDownload([[_url absoluteString] UTF8String], NULL, myHandle, 0, NULL, NULL) == noErr) {
        // Would be nice to avoid this copy, but I can't without writing a custom allocator....
        result = [[NSData alloc] initWithBytes:*myHandle length:GetHandleSize(myHandle)]; 
    }
    DisposeHandle(myHandle);
    return result;
}

- (void) cleanupBackgroundThread {
    [_lock lock];
    CFRunLoopSourceInvalidate(_rlSource);
    CFRelease(_rlSource);
    _rlSource = NULL;
    [_messageQueue release];
    _messageQueue = [[NSMutableArray alloc] init];
    URLDisposeReference(_urlRef);
    URLNewReference([[_url absoluteString] UTF8String], &_urlRef);
    [_lock unlock];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kIdleTime object:self];
}

- (void)processDownloadEvent {
    Boolean shutdown = NO;
    [_lock lock];
    while ([_messageQueue count] != 0) {
        id urlEvent = [[_messageQueue objectAtIndex:0] retain];
        [_messageQueue removeObjectAtIndex:0];
        [_lock unlock];
        if ([urlEvent isKindOfClass:[NSString class]]) {
            [self backgroundLoadDidFailWithReason:(NSString *)urlEvent];
            shutdown = YES;
        } else if ([urlEvent isKindOfClass:[NSData class]]) {
            shutdown = [(NSData *)urlEvent length] == 0 ? YES : NO;
            [self didLoadBytes:urlEvent loadComplete:shutdown];
        } else {
            NSLog(@"Unexpected URL event %@!", urlEvent);
        }
        [urlEvent release];
        [_lock lock];
    }
    [_lock unlock];
    if (shutdown) {
        [self cleanupBackgroundThread];
    }
}

- (void)useIdleTime:(NSNotification *)notif {
    URLIdle();
    [[NSNotificationQueue defaultQueue] enqueueNotification:[NSNotification notificationWithName:kIdleTime object:self] postingStyle:NSPostWhenIdle];
}

// Used to guarantee all events are signalled to the client on the main thread
- (void) reportEvent:(id)event {
    [_lock lock];
    [_messageQueue addObject:event];
    CFRunLoopSourceSignal(_rlSource);
    [_lock unlock];
}

- (OSStatus)handleEvent:(URLEvent)event info:(URLCallbackInfo *)info {
    URLReference urlRef = info->urlRef;
    switch (event) {
    case kURLDataAvailableEvent: {
        void *buffer;
        Size bufferSize;
        while (URLGetBuffer(urlRef, &buffer, &bufferSize) == noErr && bufferSize != 0) {
            // We could get around this memcopy if we used a custom deallocator and CFDataCreateWithBytesNoCopy
            NSData *data = [NSData dataWithBytes:buffer length:bufferSize];
            URLReleaseBuffer(urlRef, buffer);
            [self reportEvent:data]; 
        }
        break;
    }
    case kURLCompletedEvent:
        [self reportEvent:[NSData data]];
        break;
    case kURLErrorOccurredEvent: {
        OSStatus error;
        URLGetError(urlRef, &error);
        [self reportEvent:[NSString stringWithFormat:@"URLAccess error occurred <%d>", error]];
        break;
    }
    default:
        ;
    }
    return noErr;
}

- (void)beginLoadInBackground {
    OSStatus status;
    [_lock lock];
    if (!_rlSource) {
        CFRunLoopSourceContext rlCtxt = {0, self, CFRetain, CFRelease, CFCopyDescription, CFEqual, CFHash, NULL, NULL, downloadEvent};
        _rlSource = CFRunLoopSourceCreate(NULL, 0, &rlCtxt);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), _rlSource, kCFRunLoopCommonModes);
    }
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(useIdleTime:) name:kIdleTime object:self];
    [_lock unlock];
    if ((status = URLOpen(_urlRef, NULL, 0, notifyUPP, kURLAllEventsMask, self)) != noErr) {
        [self reportEvent:[NSString stringWithFormat:@"Could not start download of %@; status %d", _url, status]];
    }
    [[NSNotificationQueue defaultQueue] enqueueNotification:[NSNotification notificationWithName:kIdleTime object:self] postingStyle:NSPostWhenIdle];
}

- (void)endLoadInBackground {
    URLAbort(_urlRef);
    [self cleanupBackgroundThread];
}
@end
