
#ifndef __ECHOCONTEXT__
#define __ECHOCONTEXT__

#include <CoreFoundation/CoreFoundation.h>


#if defined(__cplusplus)
extern "C" {
#endif


typedef struct __EchoContext* EchoContextRef;


EchoContextRef EchoContextCreate(CFAllocatorRef alloc, CFSocketNativeHandle nativeSocket);

EchoContextRef EchoContextRetain(EchoContextRef context);
void EchoContextRelease(EchoContextRef context);

Boolean EchoContextOpen(EchoContextRef context);

void EchoContextClose(EchoContextRef context);


#if defined(__cplusplus)
}
#endif

#endif	/* __ECHOCONTEXT__ */
