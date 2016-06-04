/**
 *	filename: main.c
 *	created : Thu Nov  6 15:47:35 2003
 *	@author : Vince DeMarco <demarco@apple.com>
 *	LastEditDate Was "Tue Jun  1 05:50:15 2004"
 *
 */

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugInCOM.h>
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <unistd.h>

/* ================================== CONDUIT IMPLEMENTATION  ================================== */

/* === (1) First,  update PLUGIN_ID with a unique GUUID for your
 *         importer obtained by running uuidgen
 */

#define PLUGIN_ID    CFUUIDCreateFromString(kCFAllocatorDefault,CFSTR("98F8D6FE-3FC5-11D8-A76C-0003936726FC"))

/* === (2) Then edit Info.plist file and change the CFPlugInFactories
 *         and CFPlugInTypes entries.  Their value should be the GUUID
 *         you defined in step 1.
 *
 *         Also edit the UTI(s) of the file types your importer handles,
 *         its name, etc
 *
 */

/* === (3) Then implement the ImportFromFile function to extract the
 *         appropriate metadata from your file format and put them in
 *         the CFDictionary, attrDict, that is passed in to you.
 */

/* The ImportFromFile function's job is to extract useful information
 * your file format might support and return it as a dictionary
 *
 * This sample importer deals with Application bundles.
 */
Boolean ImportFromFile(void                   *thisInterface,
		       CFMutableDictionaryRef  attrDict,
		       CFStringRef             utiType,
		       CFStringRef             pathToFile)
{
    CFDictionaryRef        infoDict;
    CFURLRef               url;
    CFTypeRef              value;

    /* Get the CFBundle for the application */
    url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
					pathToFile,
					kCFURLPOSIXPathStyle,
					TRUE);
    if (url == NULL) {
	return FALSE;
    }

    infoDict = CFBundleCopyInfoDictionaryForURL(url);
    CFRelease(url);

    if (infoDict) {
	value = CFDictionaryGetValue(infoDict,CFSTR("CFBundleGetInfoString"));
	/* This may be a document which we have already imported.  It is possible that 
	   this attribute has been present before, but should no longer be (such as an 
	   MP3 file which has had its "Album" information stripped). If this value is null,
	   we should explicitly remove the attribute from the metadata by setting value
	   to kCFNull */
	if (value == nil) {
		value = kCFNull;
	}
	if (value){
            CFDictionaryAddValue(attrDict,kMDItemCopyright,value);
        }

	
	value = CFDictionaryGetValue(infoDict,CFSTR("CFBundleIdentifier"));
	if (value == nil) {
		value = kCFNull;
	}
	if (value){
            CFDictionaryAddValue(attrDict,CFSTR("CFBundleIdentifier"),value);
        }
	
	
	value = CFDictionaryGetValue(infoDict,CFSTR("CFBundleShortVersionString"));
        if (value == nil) {
		value = kCFNull;
	}
	if (value){
            CFDictionaryAddValue(attrDict,CFSTR("CFBundleShortVersionString"),value);
        }
        CFRelease(infoDict);
    }
    return TRUE;
}

/* ================================== CFPLUGIN SUPPORT FUNCTIONS  ================================== */

    /* The layout for an instance of SampleImporterPlugIn */
typedef struct __SampleImporterPluginType
{
    MDImporterInterfaceStruct *importerInterface;
    CFUUIDRef                 factoryID;
    UInt32                    refCount;
} SampleImporterPluginType;

    /* All of the function prototypes */
SampleImporterPluginType  *AllocSampleImporterPluginType(CFUUIDRef inFactoryID);
void                      DeallocSampleImporterPluginType(SampleImporterPluginType *thisInstance);
HRESULT                   SampleImporterQueryInterface(void *thisInstance,REFIID iid,LPVOID *ppv);
void                     *SampleImporterPluginFactory(CFAllocatorRef allocator,CFUUIDRef typeID);
ULONG                     SampleImporterPluginAddRef(void *thisInstance);
ULONG                     SampleImporterPluginRelease(void *thisInstance);

static MDImporterInterfaceStruct testInterfaceFtbl = {
    NULL,
    SampleImporterQueryInterface,
    SampleImporterPluginAddRef,
    SampleImporterPluginRelease,
    ImportFromFile
};

    /* Allocate an instance of the SampleImporterPluginType
     * You can do some initial setup for the importer here if you wish.
     * like allocating globals etc...
     */
SampleImporterPluginType *AllocSampleImporterPluginType(CFUUIDRef inFactoryID)
{
    SampleImporterPluginType *theNewInstance;

    theNewInstance = (SampleImporterPluginType *)malloc(sizeof(SampleImporterPluginType));
    memset(theNewInstance,0,sizeof(SampleImporterPluginType));

        /* Point to the function table */
    theNewInstance->importerInterface = &testInterfaceFtbl;

        /*  Retain and keep an open instance refcount for each factory. */
    theNewInstance->factoryID = CFRetain(inFactoryID);
    CFPlugInAddInstanceForFactory(inFactoryID);

        /* This function returns the IUnknown interface so set the refCount to one. */
    theNewInstance->refCount = 1;
    return theNewInstance;
}

    /* Called when the importer is deallocated. in the current implementation
     * importer interfaces are never deallocated, but implement this as this might
     * change in the future
     */
void DeallocSampleImporterPluginType(SampleImporterPluginType *thisInstance)
{
    CFUUIDRef theFactoryID;

    theFactoryID = thisInstance->factoryID;
    free(thisInstance);
    if (theFactoryID){
        CFPlugInRemoveInstanceForFactory(theFactoryID);
        CFRelease(theFactoryID);
    }
}

HRESULT SampleImporterQueryInterface(void *thisInstance,REFIID iid,LPVOID *ppv)
{
    CFUUIDRef interfaceID;

    interfaceID = CFUUIDCreateFromUUIDBytes(kCFAllocatorDefault,iid);

    if (CFEqual(interfaceID,kMDImporterInterfaceID)){
            /* If the Right interface was requested, bump the ref count,
             * set the ppv parameter equal to the instance, and
             * return good status.
             */
        ((SampleImporterPluginType*)thisInstance)->importerInterface->AddRef(thisInstance);
        *ppv = thisInstance;
        CFRelease(interfaceID);
        return S_OK;
    }else{
        if (CFEqual(interfaceID,IUnknownUUID)){
                /* If the IUnknown interface was requested, same as above. */
            ((SampleImporterPluginType*)thisInstance )->importerInterface->AddRef(thisInstance);
            *ppv = thisInstance;
            CFRelease(interfaceID);
            return S_OK;
        }else{
                /* Requested interface unknown, bail with error. */
            *ppv = NULL;
            CFRelease(interfaceID);
            return E_NOINTERFACE;
        }
    }
}

ULONG SampleImporterPluginAddRef(void *thisInstance)
{
    ((SampleImporterPluginType *)thisInstance )->refCount += 1;
    return ((SampleImporterPluginType*) thisInstance)->refCount;
}

ULONG SampleImporterPluginRelease(void *thisInstance)
{
    ((SampleImporterPluginType*)thisInstance)->refCount -= 1;
    if (((SampleImporterPluginType*)thisInstance)->refCount == 0){
        DeallocSampleImporterPluginType((SampleImporterPluginType*)thisInstance );
        return 0;
    }else{
        return ((SampleImporterPluginType*) thisInstance )->refCount;
    }
}

    /* Implementation of the factory function for this type. */
void *SampleImporterPluginFactory(CFAllocatorRef allocator,CFUUIDRef typeID)
{
    SampleImporterPluginType *result;
    CFUUIDRef                 uuid;

        /* If correct type is being requested, allocate an
         * instance of TestType and return the IUnknown interface.
         */
    if (CFEqual(typeID,kMDImporterTypeID)){
        uuid = PLUGIN_ID;
        result = AllocSampleImporterPluginType(uuid);
        CFRelease(uuid);
        return result;
    }
        /* If the requested type is incorrect, return NULL. */
    return NULL;
}

