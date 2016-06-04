/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple’s copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.  */

/*******************************************************************************
	File:		PrintDialogPDE.cp
	Contains: 	Implementation of a sample application printing dialog extension
                            for the Print "Job" Dialog

	Copyright (C) 2000-2001 by Apple Computer, Inc., all rights reserved.

	Description:
		Implementation of the example application printing dialog extension
                
		
		This module is compiled as a CFPlugin, which is then loaded by
		print dialog(s). The application is responsible for loading the CFPlugin
		with the CFPluginCreate API call. The printing system will take it 
		from there.
		
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Pragmas
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Includes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <Carbon/Carbon.h>
#include <Print/PMPrintingDialogExtensions.h>
#include <PrintCore/PMTicket.h>


#include "PrintDialogPDE.h"
#include "PDECommon.h"

#if qOurDebug
#define	DebugMessage(message)		printf((message))
#define DebugPrintErr(err, message)	if( (err) != 0) printf(message, (int)(err) )

#else
#define	DebugMessage(message)
#define DebugPrintErr(err, message)
#endif

/*
 * The text and labels for the PDE are localized using a strings file.
 * You can use the command genstrings to create a strings file from this source. Use:
 *	genstrings -s CopyLocalizedStringFromPlugin PrintDialogPDE.cp
 *
 * You may get some complaints from genstrings about this macro, but it will create the file.
 */
#define CopyLocalizedStringFromPlugin(key, comment, pluginBundleRef) \
    CFBundleCopyLocalizedString((pluginBundleRef), (key), (key), NULL)

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// ---------------------------------------------------------------------------
// each application needs to customize these CFSTRs to identify their application 
#define kPMPrintingManager 			CFSTR("com.apple.examples.print.printingmanager")
#define kSampleAppUserOptionKindID		CFSTR("com.apple.examples.print.pde.PrintDialogPDEOnly")
#define kPrintDialogPDEBundleID 		CFSTR("com.apple.examples.print.pde.PrintDialogPDE")

// define the CFUUID for the factory of our plugin. This CFUUID is for this sample
// PDE and each PDE vendor must generate a unique CFUUID for each PDE they create.
#define kPrintDialogPDEIntfFactoryIDStr CFSTR("DFC01D58-0C4A-11D5-BB77-003065500EB8")

#define kMAXH			150			// max size of our drawing area
#define kMAXV			100			// should be calculated based on our needs

enum {
	kSyncDirectionSetTickets = false,		// Set Ticket(s) based on UI
	kSyncDirectionSetUserInterface = true		// Set UI to reflect Ticket(s)
};

const ResType kPlugInCreator = kPDE_Creator;		// should be set to an appropriate creator

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Type definitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
The following Structure maintains the context between the IUnknown plugin and 
the MAC OS X system (Core Foundation). By adding a context structure to hold 
variables that are global in nature we are adding support for multi threaded 
operation.
*/
typedef struct
{
    const IUnknownVTbl*	vtable;					// Pointer to the vtable:    
    IUnknownVTbl		vtableStorage;			// Our vtable storage:
    CFUUIDRef			factoryID;			// Factory ID this instance is for:
    ULONG			refCount;			// Reference counter:
} IUnknownInstance;

/*
The following Structure maintains the context between this plugin and the host.
By adding a context structure to hold variables that are global in nature we are
adding support for multi threaded operation.
*/

typedef struct
{
    PlugInIntfVTable*	vtable;
    UInt32		refCount;    				// Reference counter
} PrintSelOnlyPlugInInterface;

typedef struct
{
    SInt16	theResFile;				// Resource File of this PDE
    CFBundleRef theBundleRef;				// Our bundle reference
    CFStringRef		titleStringRef;			// Our PDE's title string ref
    ControlRef	thePrintSelTextOnlyControlRef;
} PrintDialogPDEOnlyContext, *PrintDialogPDEOnlyContextPtr;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Function prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus GetTicketRef(PMPrintSession printSession, CFStringRef ticket, 
                                PMTicketRef* printSettingsPtr);

// Global initialization function.
static OSStatus InitContext	( PrintDialogPDEOnlyContextPtr*	context);

#if __cplusplus
extern "C" {
#endif

/*
The following define prototypes for the PDE API...
*/
static 
OSStatus MyPDEPrologue(PMPDEContext	*context,
                    OSType 		*creator,
                    CFStringRef		*userOptionKind,
                    CFStringRef 	*title, 
                    UInt32 		*maxH, 
                    UInt32 		*maxV);
static
OSStatus MyPDEInitialize(PMPDEContext	context, 
                        PMPDEFlags*	flags,
                        PMPDERef	ref,
                        ControlRef	parentUserPane,
                        PMPrintSession	printSession);
static 
OSStatus MyPDEGetSummaryText(PMPDEContext	context, 
                        CFArrayRef 	*titleArray,
                        CFArrayRef	*summaryArray);

static 
OSStatus MyPDESync(PMPDEContext	context,
                PMPrintSession	printSession,
                Boolean		reinitializePlugin);
static 
OSStatus MyPDEOpen(PMPDEContext	context);

static 
OSStatus MyPDEClose(PMPDEContext	context);

static 
OSStatus MyPDETerminate(PMPDEContext	context, 
                    OSStatus	status);

#if __cplusplus
}
#endif
					
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Global variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    CFPlugin Specific Routines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* 	The Info.plist file and how it relates to PDE's.

	Two associations need to be made when developing a print dialog extension.
 	This association is accomplished by the information contained in the
	Info.plist file. The Info.plist file for this PDE looks like
	this:

{
    CFBundleExecutable = "PrintDialogPDE";
    CFBundleIdentifier = "com.apple.examples.print.pde.PrintDialogPDE";
    CFPlugInDynamicRegistration = NO;
    CFPlugInFactories = 
    {
        "DFC01D58-0C4A-11D5-BB77-003065500EB8" = "PrintDialogPDEPluginFactory";
    };
    CFPlugInTypes = 
    {	// the string on the left is kAppPrintDialogTypeIDStr
        "BCB07250-E57F-11D3-8CA6-0050E4603277" = ("DFC01D58-0C4A-11D5-BB77-003065500EB8");
    };	
}

	The CFPlugInTypes entry associates the extensions type UUID with this plugins 
	factory UUID. And the CFPlugInFactories entry associates this plugins factory
	UUID with the factory function name.
	
*/


// Factory ID and routine.
// Plugin factory names must not be mangled by C++ compiler.

#if __cplusplus
extern "C" {
#endif
    // Factory function:
    void* PrintDialogPDEPluginFactory( CFAllocatorRef allocator, CFUUIDRef typeID );
#if __cplusplus
}
#endif

// More Prototypes
static OSStatus MyCreatePlugInInterface( PlugInIntf** objPtr );
static ULONG IUnknownAddRef( void* obj );
static ULONG IUnknownRelease( void* obj );

/*
	The next two routines(IUnknownAddRef, IUnknownRelease) and IUnknownQueryInterface are 
	required by CFPlugin to instantiate or create an instance of a base class 
	(to put it in C++ terminology).
*/

/* =============================================================================
    Name:	IUnknownAddRef()
	Description:
        This function adds a tick to a plugin instance's reference count.
	Input Parameters:
        obj			-	The 'this' pointer.
	Output Parameters:
		None.
	Return Value:
        ULONG			-	Updated value of the reference count, or zero
						in case of an error. 
* ========================================================================== */
static ULONG IUnknownAddRef(void* obj)
{   
    IUnknownInstance* instance = (IUnknownInstance*) obj;
    ULONG refCount = 0;    		// We can't do much with errors here since we can only
                                        // update reference count value.   
    if (instance != NULL)
    {
		// Get updated refCount value (should be under mutex):
		refCount = ++instance->refCount;
    }
    else
    {
		refCount = 0;
    }
	return refCount;
}
/* =============================================================================
    Name:	IUnknownRelease()
	Description:
        This function takes a tick from a plugin instance's reference count.
		When the reference count goes down to zero the object self-destructs.
	Input Parameters:
        obj			-	The 'this' pointer.
	Output Parameters:
        None.    Return Value:
        ULONG		-	Updated value of the reference count, or zero
                        in case of an error.
* ========================================================================== */
static ULONG IUnknownRelease(void* obj)
{
    IUnknownInstance* instance = (IUnknownInstance*) obj;
    ULONG refCount = 0;
    
    // We can't do much with errors here since we can only return
    // updated reference count value.
    if (instance != NULL)
    {
	    // Get updated refCount value (should be under mutex):
	    // Make sure refCount is non-zero:
	    if (0 == instance->refCount)
	    {
		    instance = NULL;
		    return(refCount);
	    }

	    refCount = --instance->refCount;
	    
	    // Is it time to self-destruct?
	    if (0 == refCount)
	    {	    
		    // Unregister 'instance for factory' with CoreFoundation:
		    CFPlugInRemoveInstanceForFactory(instance->factoryID);														
		    // Release used factoryID:				
		    CFRelease(instance->factoryID);
		    instance->factoryID = NULL;
		    
		    // Deallocate object's memory block:
		    free((void*) instance);
		    instance = NULL;
	    }
    }
    
    return refCount;    
}
/*
	The next three routines(MyPMRetain, MyPMRelease, MyPMGetAPIVersion) are required by 
	the Printing system to host the dialog extensions plugin.
*/

/* =============================================================================
    Name:	MyPMRetain()
	
	Description:
        This function controls our plugins instance.
		This function adds a tick to a plugin instance's reference count.

	Input Parameters:
        obj			-	The 'this' pointer.

	Output Parameters:
        None.

	Return Value:
        OSStatus		- 	The error status
* ========================================================================== */
static OSStatus MyPMRetain(PMPlugInHeaderInterface* obj)
{
    if (obj != NULL)
    {
	PrintSelOnlyPlugInInterface* plugin = (PrintSelOnlyPlugInInterface*) obj;

	// Increment reference count:
	plugin->refCount++;    
    }
    return noErr;
}

/* =============================================================================
    Name:	MyPMRelease()

	Description:
        This function controls our plugins instance. It takes a tick from a 
        plugin instance's reference count. When the reference count goes down
        to zero the object self-destructs.

	Input Parameters:
        obj			-	The 'this' pointer.

	Output Parameters:
        None.

	Return Value:
        OSStatus	- 	The error status
* ========================================================================== */
static OSStatus MyPMRelease(PMPlugInHeaderInterface** objPtr)
{
    if (*objPtr != NULL)
    {
		PrintSelOnlyPlugInInterface* plugin = (PrintSelOnlyPlugInInterface*) *objPtr;
		
		// Clear caller's variable:
		*objPtr = NULL;
	
		// Decrement reference count:
		plugin->refCount--;
	
		// When reference count is zero it's time self-destruct:
		if (0 == plugin->refCount)
		{
			// Delete object's vtable:
			free((char *)plugin->vtable);
	
			// Delete object's memory block:
			free((char *)plugin);
		}
    }
    return noErr;
}
/* =============================================================================
    Name:	MyPMGetAPIVersion()

	Description:
        This routine returns the plugins interface version

	Input Parameters:
        obj			-	The 'this' pointer.

	Output Parameters:
        versionPtr	- 	The pointer to the API version information

	Return Value:
        OSStatus	- an error code
 * ========================================================================== */
static OSStatus  MyPMGetAPIVersion(PMPlugInHeaderInterface* obj, PMPlugInAPIVersion* versionPtr)
{
   // Return versioning info:
    versionPtr->buildVersionMajor = kPDEBuildVersionMajor;
    versionPtr->buildVersionMinor = kPDEBuildVersionMinor;
    versionPtr->baseVersionMajor  = kPDEBaseVersionMajor;
    versionPtr->baseVersionMinor  = kPDEBaseVersionMinor;
    
    return noErr;
}

/* =============================================================================
    Name:	MyCreatePlugInInterface()

	Description:
        This function creates an interface for our plugin. An interface in this
        sample code consists of both the vtable of pointer to functions and the
        variables that will be global to the routines in this plugin. In this
        sample code we are storing a ControlRef in the interface.

	Input Parameters:
        obj			-	The 'this' pointer.

	Output Parameters:
        None.

	Return Value:
        OSStatus	- an error code
* ========================================================================== */
static OSStatus MyCreatePlugInInterface( PlugInIntf** objPtr )
{
    PrintSelOnlyPlugInInterface*	intf = NULL;
    PlugInIntfVTable*				vtable = NULL;

    // Allocate object and clear it:
    intf = (PrintSelOnlyPlugInInterface*) calloc(1, sizeof( PrintSelOnlyPlugInInterface ));
	
	if (intf != NULL) 
	{
	    // Assign all plugin data members:
	    intf->refCount = 1;
	    
	    // Allocate object's vtable and clear it:
	    vtable = (PlugInIntfVTable*) calloc(1, sizeof( PlugInIntfVTable ));
		if (vtable != NULL)
		{
		    intf->vtable = vtable;
	   		
		    // Assign all plugin header methods:
		    vtable->plugInHeader.Retain 		= MyPMRetain;
		    vtable->plugInHeader.Release 		= MyPMRelease;
		    vtable->plugInHeader.GetAPIVersion 		= MyPMGetAPIVersion;

		    // Assign all plugin methods:
		    vtable->Prologue 				= MyPDEPrologue;
		    vtable->Initialize 				= MyPDEInitialize;
		    vtable->Sync 				= MyPDESync;
		    vtable->GetSummaryText 			= MyPDEGetSummaryText;
		    vtable->Open 				= MyPDEOpen;
		    vtable->Close 				= MyPDEClose;
		    vtable->Terminate 				= MyPDETerminate;
	    }
	}

    // Return results:
    *objPtr = (PlugInIntf*) intf;
    
    return noErr;
}

/* =============================================================================

    Name:	IUnknownQueryInterface()

    Description:
        This function creates an interface for our plugin by calling 
        MyCreatePlugInInterface. If the interface requested is for the
        "base" IUnKnownInterface just bump the refcount.

    Input Parameters:
        obj		-	The 'this' pointer.
        iID		- 	The UUID that describes which interface to return

    Output Parameters:
        intfPtr - 	A Pointer to the requested interface

    Return Value:
        HRESULT	-	Tells Core Foundation our status.

 * ========================================================================== */
 
static HRESULT IUnknownQueryInterface( void* obj, REFIID iID, LPVOID* intfPtr )
{
    IUnknownInstance*	instance = (IUnknownInstance*) obj;
    CFUUIDRef			myIntfID = NULL, reqIntfID = NULL;
    HRESULT				err = E_UNEXPECTED;
    PlugInIntf*			interface;

    // Get IDs for requested and PDE interfaces:
    reqIntfID = CFUUIDCreateFromUUIDBytes( kCFAllocatorDefault, iID );
    myIntfID = CFUUIDCreateFromString( kCFAllocatorDefault, kDialogExtensionIntfIDStr );
    if(reqIntfID && myIntfID){
    
	    // If we are asked to return the interface for 
	    // the IUnknown vtable, which the system already has access to,
	    // just increment the refcount value
	if(CFEqual( reqIntfID, IUnknownUUID ) )
	{
	    instance->vtable->AddRef( (void*) instance );
	    *intfPtr = (LPVOID) instance;
		    err = S_OK;
	}
	else if (CFEqual(reqIntfID, myIntfID))	// if we are asked for the PDEs interface,
	    {								// lets make one and return it.
		    err = MyCreatePlugInInterface( &interface );
		    if ( noErr == err )
		    {
			    *intfPtr = (LPVOID) interface;
			    err = S_OK;
		    }else{
			*intfPtr = NULL;
			err = E_NOINTERFACE;
		    }
	    }
	    else // we will return the err = E_NOINTERFACE and a  *intfPtr of NULL;
	    {
		*intfPtr = NULL;
		err = E_NOINTERFACE;
	    }
    }else{	// we will return the err = E_NOINTERFACE and a  *intfPtr of NULL;
	*intfPtr = NULL;
	err = E_NOINTERFACE;
    }
    // Clean up and return status:
    if(reqIntfID)CFRelease( reqIntfID );
    if(myIntfID)CFRelease( myIntfID );

    return( err );
}

/* =============================================================================

    Name:	PrintDialogPDEPluginFactory()

    Description:
		This is the factory function which should be the only entry point of 
		this plugin. This factory function creates the "base class" for the system.
		The factory functions name (ie "PrintDialogPDEPluginFactory") needs to be
		listed in the Info.plist to associate the factory function name
		with the factory function UUID. For example, this is how this function
		is associated with the UUID in the Info.plist file.
		
		CFPlugInFactories = 
		{
       	 	"DFC01D58-0C4A-11D5-BB77-003065500EB8" = "PrintDialogPDEPluginFactory";
    	};

    Input Parameters:
        allocator	- the allocator function used by CoreFoundation
        reqTypeID	- requested instance type.

    Output Parameters:
        None.

    Return Value:
		

 * ========================================================================== */
void* PrintDialogPDEPluginFactory( CFAllocatorRef allocator, CFUUIDRef reqTypeID )
{
    CFUUIDRef			myInstID;
    IUnknownInstance*	instance = NULL;

   // There is not much we can do with errors - just return NULL.
    myInstID = CFUUIDCreateFromString(kCFAllocatorDefault, kAppPrintDialogTypeIDStr);
    // If the requested type matches our plugin type (it should!)
    // have a plugin instance created which will query us for
    // interfaces:
    if (myInstID && CFEqual(reqTypeID, myInstID))
    {
	CFUUIDRef myFactoryID = CFUUIDCreateFromString(kCFAllocatorDefault, kPrintDialogPDEIntfFactoryIDStr);
	if(myFactoryID){
	    // allocate and clear our instance structure
	    instance = (IUnknownInstance*) calloc(1, sizeof(IUnknownInstance));
	    if (instance != NULL)
	    {
		    // Assign all members:
		    instance->vtable = &instance->vtableStorage;
		    
		    instance->vtableStorage.QueryInterface = IUnknownQueryInterface;
		    instance->vtableStorage.AddRef = IUnknownAddRef;
		    instance->vtableStorage.Release = IUnknownRelease;
		    
		    instance->factoryID = myFactoryID;
		    instance->refCount = 1;

		    // Register the newly created instance
		    CFPlugInAddInstanceForFactory(myFactoryID);
	    }
	}
    }
    if(myInstID)
        CFRelease(myInstID);
    
    return ((void*) instance);
}

/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	MyPDEPrologue
	
	Input Parameters:
		none

	Output Parameters:
		context			:	The plugins context
		creator			: 	The creator type for this plugin
		userOptionKind		: 	The extension kind for the plugin
		title			: 	The title of this plugin.
		maxH			:	Maximum horizontal dimension required by client.
		maxV			:	Maximum vertical dimension required by client.
		err			:	returns the error status

	Description:
		Returns dimensions of content region desired by the client.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static 
OSStatus MyPDEPrologue(		PMPDEContext	*context,
						OSType 			*creator,
						CFStringRef		*userOptionKind,
						CFStringRef 		*title, 
						UInt32 			*maxH, 
						UInt32 			*maxV)
{
    OSStatus err = noErr;
    PrintDialogPDEOnlyContextPtr myContext = NULL;	// Pointer to our context data.

    DebugMessage("PrintDialogPDE MyPDEPrologue called\n");
    
    err = InitContext(&myContext);
    
    if (noErr == err)
    {
	*context = (PMPDEContext) myContext;

	// calculate the maximum amount of screen real estate that this plugin needs.
	*maxH = kMAXH;
	*maxV = kMAXV;

	/*
	    The semantics of the CFStrings represented by *title and *userOptionKind
	    are 'Get' semantics: the caller will retain what it needs to retain.
	    
	    This means that we need to release this title string sometime after
	    this routine returns. We put our reference to the string into our context
	    data so we can release that string when we dispose of the context data.
	*/
	myContext->titleStringRef = CopyLocalizedStringFromPlugin(		
			CFSTR("Sample Application PDE"),
			CFSTR("the text of the popup menu"),
			myContext->theBundleRef);

	if (myContext->titleStringRef != NULL){
		*title = myContext->titleStringRef;
	}

	*userOptionKind = kSampleAppUserOptionKindID;
	*creator = kPlugInCreator;    
    }
    else
	err = kPMInvalidPDEContext;						// return an error

    DebugPrintErr(err, "PrintDialogPDE Error from MyPDEPrologue returned %d\n");
    
    return (err);
}

/******************************************************************************/

/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	MyPDEInitialize

	Input Parameters:
		context				:	The plugins context
		parentUserPane		:	the user pane to your your controls into
		ref					:	the reference to this PDE
		printSession		:	this holds the PM tickets
		
	Output Parameters:
		flags				:	feature flags that are supported by this PDE
		err					:	returns the error status

	Description:
		Initializes client interface. Creates controls and sets initial values


	
	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static 
OSStatus MyPDEInitialize(	PMPDEContext	context, 
						PMPDEFlags*		flags,
						PMPDERef		ref,
						ControlRef		parentUserPane,
						PMPrintSession	printSession)
{
    OSStatus err = noErr;
    PrintDialogPDEOnlyContextPtr myContext = NULL;	// Pointer to our context data.

    DebugMessage("PrintDialogPDE MyPDEInitialize called\n");
    
    myContext = (PrintDialogPDEOnlyContextPtr) context;
    
    if ((myContext != NULL) && (printSession != NULL))
    {		
		WindowRef theWindow = NULL;
		short savedResFile = CurResFile();
		UseResFile(myContext->theResFile);
		theWindow = GetControlOwner(parentUserPane);	// get the windowref from the user pane
		
		// the user panes rect is the rect we should use to draw our
		// controls into. The printing system calculates the user pane
		// size based on the maxh and maxv sizes returned from the 
		// Prologue function
		
		// Note that we are using the AutoToggleProc variant of the Radio Button control
		// This allows a hit on this control to be automatically processed by the ControlMgr
	
		// get controls
		myContext->thePrintSelTextOnlyControlRef = GetNewControl(kPrintDialogPDEControlID, theWindow);
	
		// embed controls
		EmbedControl(myContext->thePrintSelTextOnlyControlRef, parentUserPane);
	
		// set controls as visible
		SetControlVisibility(myContext->thePrintSelTextOnlyControlRef, true, false);
	
		// Set default value
		SetControlValue(myContext->thePrintSelTextOnlyControlRef, 0);
		
		// Set flags
		*flags = kPMPDENoFlags;
		
		// Initialize this plugins controls based on the information in the 
		// PageSetup or PrintSettings ticket.
		err = MyPDESync(context, printSession, kSyncDirectionSetUserInterface);
		if (err == kPMKeyNotFound)
			err = noErr;

		UseResFile(savedResFile);
    }
    else
	err = kPMInvalidPDEContext;

    DebugPrintErr(err, "PrintDialogPDE Error from MyPDEInitialize returned %d\n");

    return (err);
}

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	MyPDEGetSummaryText

	Input Parameters:
		context			:	The plugins context
		titleArray 		:	an array to store the title of the summary text
		summaryArray	:	an array to store the summary text
		
	Output Parameters:
		titleArray 		:	updated with this plugins summary text title
		summaryArray	:	updated with this plugins summary text
		err				:	returns the error status

	Description:
		Returns the status/state of the plugin in textual form

	Change History (most recent first):

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static 
OSStatus MyPDEGetSummaryText(PMPDEContext	context, 
						CFArrayRef 		*titleArray,
						CFArrayRef		*summaryArray)
{
	OSStatus err = noErr;
        CFMutableArrayRef theTitleArray = NULL;				// Init CF strings
        CFMutableArrayRef theSummaryArray = NULL;
        CFStringRef titleStringRef = NULL;
        CFStringRef summaryStringRef = NULL;
	PrintDialogPDEOnlyContextPtr myContext = NULL;	// Pointer to our context data.
    
        myContext = (PrintDialogPDEOnlyContextPtr) context;
	*titleArray = NULL;
        *summaryArray = NULL;

        DebugMessage("PrintDialogPDE MyPDEGetSummaryText called\n");

	if (myContext != NULL)
	{
		//  NOTE: if the second parameter to CFArrayCreateMutable 
		//		  is not 0 then the array is a FIXED size
		theTitleArray = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		theSummaryArray = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    
		if ((theTitleArray != NULL) && (theSummaryArray != NULL))
		{
			SInt16	theControlValue = -1;
			titleStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" Print Selected Text Only"),
						CFSTR("Summary Title"),
						myContext->theBundleRef);
		
			theControlValue = GetControlValue(myContext->thePrintSelTextOnlyControlRef);
			switch (theControlValue)
			{
				case 0:
					summaryStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" No, Print All Text"),
						CFSTR("Summary Text"),
						myContext->theBundleRef);
					break;
	
				case 1:
					summaryStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" Yes"),
						CFSTR("Summary Text"),
						myContext->theBundleRef);
					break;
			}
                        
                        if(titleStringRef && summaryStringRef){
                            CFArrayAppendValue(theTitleArray, titleStringRef);
                            CFArrayAppendValue(theSummaryArray, summaryStringRef);
                        }else
                            err = memFullErr;
                }else
                    err = memFullErr;
        }
        else
            err = kPMInvalidPDEContext;

        // we release these because we've added them already to the title and summary array
        // or we don't need them because there was an error
        if (titleStringRef)
                CFRelease(titleStringRef);
        if (summaryStringRef)
                CFRelease(summaryStringRef);
        
        // update the data passed in.
        if(!err){
            *titleArray = theTitleArray;
            *summaryArray = theSummaryArray;

        }else{
            if (theTitleArray)
                    CFRelease(theTitleArray);
            if (theSummaryArray)
                    CFRelease(theSummaryArray);
        }

        DebugPrintErr(err, "PrintDialogPDE Error from MyPDEGetSummaryText returned %d\n");

	return (err);
}

/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	MyPDESync

	Input Parameters: 	
		context			:	The plugins context
		printSession		:	this holds the tickets
		syncDirection		:	A boolean that tells the plugin that it needs to
                                                do one of two functions. 
                                                    If true the plugin
							fetches the values from the tickets and 
							sets the plugins control values.
                                                    if false the plugin
                                                        does the opposite, it takes the values out of the 
                                                        plugins' controls and sets them into the
                                                        tickets
			
	Output Parameters:
		err				returns the error status

	Description:
		Sets/Gets values in the PageFormat or PrintSettings tickets

	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static 
OSStatus MyPDESync(			PMPDEContext	context,
						PMPrintSession	printSession,
						Boolean			syncDirection)
{
    OSStatus err = noErr;
    PrintDialogPDEOnlyContextPtr myContext = NULL;		// Pointer to our context data.
    CFDataRef ourPDEPrintSettingsDataRef = NULL;

    DebugMessage("PrintDialogPDE MyPDESync called\n");

    myContext = (PrintDialogPDEOnlyContextPtr) context;
    
    if ((myContext != NULL) && (printSession != NULL))
	{
            PMTicketRef printSettingsContainer = NULL;
            err = GetTicketRef(printSession, kPDE_PMPrintSettingsRef, &printSettingsContainer);

            if (noErr == err)		
            {
                Boolean printSelectionOnly = false;
                SInt16 theControlValue = -1;

                switch (syncDirection)
                {
                    case kSyncDirectionSetUserInterface:
                        err = PMTicketGetCFData(printSettingsContainer, kPMTopLevel, 
				kPMTopLevel, kAppPrintDialogPDEOnlyKey, 
				&ourPDEPrintSettingsDataRef);
                        if(!err){
                            if(CFDataGetLength(ourPDEPrintSettingsDataRef) == sizeof(printSelectionOnly) ){
                                printSelectionOnly = *(Boolean *)
                                                CFDataGetBytePtr(ourPDEPrintSettingsDataRef);
                            }else
                                printSelectionOnly = kPrintSelectionOnlyDefault;	// default value
                            CFRelease(ourPDEPrintSettingsDataRef);
                        }else{
                            // set to default value
                            printSelectionOnly = kPrintSelectionOnlyDefault;
                            err = noErr;
                        }
                        if (noErr == err)
                        {
                                theControlValue = (printSelectionOnly) ? 1 : 0;
                                SetControlValue(myContext->thePrintSelTextOnlyControlRef, theControlValue);
                        }
                        break;

                    case kSyncDirectionSetTickets:
                        theControlValue = GetControlValue(myContext->thePrintSelTextOnlyControlRef);
                        printSelectionOnly = theControlValue != 0;
                        ourPDEPrintSettingsDataRef = CFDataCreate(kCFAllocatorDefault, 
                                                &printSelectionOnly, sizeof(printSelectionOnly));
                        if(ourPDEPrintSettingsDataRef){
			    err = PMTicketSetCFData(printSettingsContainer, kPMPrintingManager, 
                                            kAppPrintDialogPDEOnlyKey, ourPDEPrintSettingsDataRef,
					    kPMUnlocked);
			    CFRelease(ourPDEPrintSettingsDataRef);
			}else
			    err = memFullErr;
                        break;
                }
            }
	}
    else
	err = kPMInvalidPDEContext;
	
    DebugPrintErr(err, "PrintDialogPDE Error from MyPDESync returned %d\n");

    return (err);
}

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	MyPDEOpen

	Input Parameters:
		context			:	The plugins context
		
	Output Parameters:
		err				:	returns the error status
		
	Description:
		Do something before the plugins controls are shown in the print/page
		setup dialogs panel

	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus MyPDEOpen( PMPDEContext	context )
{
	OSStatus err = noErr;
	PrintDialogPDEOnlyContextPtr myContext = NULL;	// Pointer to our context data.

	DebugMessage("PrintDialogPDE MyPDEOpen called\n");
	
	myContext = (PrintDialogPDEOnlyContextPtr) context;
    if (myContext != NULL)
    {
	// make sure you make yourself the current resource file if you need
	// access to your resources
	
	//short savedResFile = CurResFile();
	//UseResFile(myContext->theResFile);

	//  Do something useful here
	//UseResFile(savedResFile);
    }
    else
	err = kPMInvalidPDEContext;

    DebugPrintErr(err, "PrintDialogPDE Error from MyPDEOpen returned %d\n");

    return err;
}
/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	MyPDEClose

	Input Parameters:
		context			:	The plugins context
		
	Output Parameters:
		err				:	returns the error status

	Description:
		Do something before control is shifted to another plugin

	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus MyPDEClose( PMPDEContext context )
{
    OSStatus err = noErr;
    PrintDialogPDEOnlyContextPtr myContext = NULL;	// Pointer to our context data.

    DebugMessage("PrintDialogPDE MyPDEClose called\n");
    
    myContext = (PrintDialogPDEOnlyContextPtr) context;
    if (myContext != NULL)
    {
	//  Do something useful here
    }
    else
	err = kPMInvalidPDEContext;

    DebugPrintErr(err, "PrintDialogPDE Error from MyPDEClose returned %d\n");

    return ( err );

}

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	MyPDETerminate

	Input Parameters:
		context			:	The plugins context
		status			:	Tells us why we are going away
		
	Output Parameters:
		err			:	returns the error status
		
	Description:
		Disposes of controls, and other "things" created by this plugin.

	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static OSStatus MyPDETerminate( PMPDEContext context, OSStatus status  )
{
	OSStatus err = noErr;
	PrintDialogPDEOnlyContextPtr myContext = NULL;	// Pointer to our context data.

	DebugMessage("PrintDialogPDE MyPDETerminate called\n");
	
	myContext = (PrintDialogPDEOnlyContextPtr) context;
	
	if (myContext != NULL)
    {		
	if (myContext->theResFile != -1)
	{	// Close the resource fork
		CFBundleCloseBundleResourceMap(myContext->theBundleRef, myContext->theResFile);
		myContext->theResFile = -1;
	}

	if(myContext->titleStringRef)
	    CFRelease(myContext->titleStringRef);
	    
	// Free the global context.
	free((char*) myContext);
	myContext = NULL;
    }
    else
	err = kPMInvalidPDEContext;

    DebugPrintErr(err, "PrintDialogPDE Error from MyPDETerminate returned %d\n");

    return ( err );
}

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	InitContext

	Input Parameters:
		
	Output Parameters:
		context		:	Pointer to global context.
		<function>	:	Status code.

	Description:
		Allocates and initializes the context data for this module.

	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus InitContext(PrintDialogPDEOnlyContextPtr* context)
{	
	OSStatus	err = noErr;		// Error condition.
		
	/*
	Allocate the global context.
	*/
	*context =  (PrintDialogPDEOnlyContextPtr) calloc(1, sizeof(PrintDialogPDEOnlyContext));

	if (NULL != *context)
	{
		CFBundleRef theBundleRef = NULL;
		/*
		    the calloc above initializes our context data to 0.

		    (*context)->thePrintSelTextOnlyControlRef = NULL;
		    (*context)->theBundleRef = NULL;
		    (*context)->titleStringRef = NULL;
		*/
		
		(*context)->theResFile = -1;

		//  Open the resource fork
		theBundleRef = CFBundleGetBundleWithIdentifier(kPrintDialogPDEBundleID);
                if(theBundleRef){
                    (*context)->theResFile =  CFBundleOpenBundleResourceMap(theBundleRef);
                    if ((*context)->theResFile == -1)
                            err = kPMGeneralError;
    
                    (*context)->theBundleRef = theBundleRef;
                }else
                    err = kPMInvalidPDEContext;			// this really needs a better error code

	}
	else
		err = kPMInvalidPDEContext;
	
	return (err);
	
}	// InitContext


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	GetTicketRef

	Input Parameters
		printSession	:	this holds the PM tickets		
		ticket			:	Ticket we want	

	Output Parameters:
		printSettingsPtr:	Ticket reference to the ticket
	
	Return Value:
			err			:	an error code

	Description:
	    Get the ticket refence from a printsession and ticket
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus GetTicketRef(PMPrintSession printSession, CFStringRef ticket, PMTicketRef* printSettingsPtr)
{
	OSStatus err = noErr;
	CFTypeRef cfTicketRef = NULL;

	err = PMSessionGetDataFromSession(printSession, ticket, &cfTicketRef);

	if ((noErr == err) && (cfTicketRef))
	{
		// This returns a Boolean (lossy data) that we don't care about
		Boolean lossy;
		lossy = CFNumberGetValue((CFNumberRef)cfTicketRef, kCFNumberSInt32Type, (void*)printSettingsPtr);

		if (NULL == *printSettingsPtr)
			err = kPMInvalidValue;
	}
	else
	{
		err = kPMInvalidTicket;
	}
	return err;
}
