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

/*------------------------------------------------------------------------------

    This sample code is the Carbon equivalent of the classic print loop
    documented in Tech Note 1092 "A Print Loop That Cares ...".  This code
    illustrates the use of functions defined in PMCore.h and PMApplication.h
    instead of Printing.h.
    
    This code is updated so that the sample print loop code draws exclusively
    with Quartz since QuickDraw is deprecated in Tiger and later. Note that 
    the portion of the code which handles the Print apple event has not been 
    updated.

    You may incorporate this sample code into your applications without
    restriction, though the sample code has been provided "AS IS" and the
    responsibility for its operation is 100% yours.  However, what you are
    not permitted to do is to redistribute the source as "Apple Sample Code"
    after having made changes. If you're going to re-distribute the source,
    we require that you make it clear in the source that the code was
    descended from Apple Sample Code, but that you've made changes.
    
    Version:	1.0.3
    
    Technology:	Carbon Printing for Mac OS X

    Copyright © 1998-2004 Apple Computer, Inc  ., All Rights Reserved
	
    Change History:

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
	Includes
------------------------------------------------------------------------------*/

#include <Carbon/Carbon.h>
#include "PDECommon.h"

#include <ApplicationServices/ApplicationServices.h>
//#include <PMPrintAETypes.h>

#define USING_PDE	1		// if we are using PDEs on MacOS X
#define NUMPAGES	10		// the number of pages in our sample "document" 

#define	PRINTSAMPLE	1
/*------------------------------------------------------------------------------
	Globals
------------------------------------------------------------------------------*/
static	Handle	gflatPageFormat = NULL;		// used in FlattenAndSavePageFormat
static	Boolean	gQuit=false;

/*------------------------------------------------------------------------------
	Prototypes
------------------------------------------------------------------------------*/
int		main(void);
void	Initialize(void);
void	DoEventLoop(void);
void	DoHighLevel(EventRecord *AERecord);
static 	OSErr	AEPrintDocument(const AppleEvent *inputEvent, AppleEvent *outputEvent, SInt32 handlerRefCon);
static 	OSErr	AEOpenHandler(const AppleEvent *inputEvent, AppleEvent *outputEvent, SInt32 handlerRefCon);
static 	OSErr	AEOpenDocHandler(const AppleEvent *inputEvent, AppleEvent *outputEvent, SInt32 handlerRefCon);
static 	OSErr	AEQuitHandler(const AppleEvent *inputEvent, AppleEvent *outputEvent, SInt32 handlerRefCon);
  
OSStatus 	DoPageSetupDialog(PMPrintSession printSession, PMPageFormat* pageFormat);
OSStatus 	DoPrintDialog(PMPrintSession printSession, PMPageFormat pageFormat,
				PMPrintSettings* printSettings);
OSStatus	DoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat,
				PMPrintSettings printSettings);
void		DoPrintSample(void);

OSStatus 	FlattenAndSavePageFormat(PMPageFormat pageFormat);
OSStatus 	LoadAndUnflattenPageFormat(PMPageFormat* pageFormat);
OSStatus	DetermineNumberOfPagesInDoc(PMPageFormat pageFormat, UInt32* numPages);
OSStatus 	DrawPage(CGContextRef context, UInt32 pageNumber);
void 		PostPrintingErrors(OSStatus status);


/*------------------------------------------------------------------------------
    Function:	main
	
    Parameters:
        <none>
	
    Description:
        Uses PMCreateSession/PMRelease instead of PMBegin/PMEnd.  Note that the two
        printing objects, PMPageSetup and PMPrintSettings are valid outside the
        printing session.  This was not the case with PMBegin/PMEnd in the previous
        version of this sample code.  Note also that no nesting of printing sessions
        is allowed for Carbon applications running under MacOS 8 or 9.
        In this sample code, we show where an old print record could be read in and
        converted.  This is followed by displaying the Page Setup dialog, then the
        Print dialog, and finally we print the pages in the document.
	
------------------------------------------------------------------------------*/
int main(void)
{
    OSStatus		status = noErr;

    //	Standard Toolbox initialization.
    Initialize();
    DoEventLoop();

    return status;
}	// main

void		Initialize(void)
{
    OSStatus	err = noErr;
    Handle	menuBar;
    IBNibRef 	nibRef = NULL;

    InitCursor();
    
    err = CreateNibReference( CFSTR("AppSDKMenu"),&nibRef );
    
    err = CreateMenuBarFromNib( nibRef,CFSTR("MenuBar"),&menuBar);
    err = SetMenuBarFromNib( nibRef, CFSTR("MenuBar") );
    DisposeNibReference( nibRef );

    // install AE handlers
    err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerUPP(AEPrintDocument), 0, false);
    err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, NewAEEventHandlerUPP(AEOpenHandler), 0, false);
    err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerUPP(AEOpenDocHandler), 0, false);
    err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(AEQuitHandler), 0, false);
}

/*------------------------------------------------------------------------------

    Function:	DoPrintSample
    
    Parameters:
        None
    Description:
        Sample print loop.

------------------------------------------------------------------------------*/
void		DoPrintSample(void)
{
    OSStatus		status = noErr;
    PMPageFormat	pageFormat = kPMNoPageFormat;
    PMPrintSettings	printSettings = kPMNoPrintSettings;
    PMPrintSession	printSession = NULL;
    Handle		oldPrintRecord = NULL;	// place holder for old print record

    //	Initialize the printing manager and create a printing session.
    status = PMCreateSession(&printSession);
    if (status != noErr) return;	// pointless to continue if PMCreateSession fails
    
    //	If your application has an old print record, it can be converted into new
    //	PMPageFormat and PMPrintSettings objects.  In this sample code, we skip this
    //	step.
    if (oldPrintRecord != NULL)
        status = PMSessionConvertOldPrintRecord(printSession, oldPrintRecord,
                &printSettings, &pageFormat);
                                
#if USING_PDE
    /* This sample code demonstrates usage of the MacOS X Printer Dialog Extension (PDE) 
        functionality for both the Page Setup and Print dialogs. If you have no plans 
        to extend either the PageSetup or Print dialogs on MacOS X then you can 
        safely ignore this code and set USING_PDE to false.
        
        Here we register our plugins. They are installed so they are located in the same 
        directory as the MacOS executable.
    */
    if(status == noErr){
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        if(mainBundle){
            CFURLRef pluginsURL = CFBundleCopyBuiltInPlugInsURL(mainBundle);
            if(pluginsURL){
                CFURLRef myPluginURL = CFURLCreateCopyAppendingPathComponent(NULL, pluginsURL,
                                CFSTR("PageSetupPDE.plugin"), false);
                if(myPluginURL){
                    (void)CFPlugInCreate(NULL, myPluginURL);
                    CFRelease(myPluginURL);
                }
                myPluginURL = CFURLCreateCopyAppendingPathComponent(NULL, pluginsURL, 
                                CFSTR("PrintDialogPDE.plugin"), false);
                if(myPluginURL){
                    (void)CFPlugInCreate(NULL, myPluginURL);
                    CFRelease(myPluginURL);
                }
                CFRelease(pluginsURL);
            }
        }
    }
#endif

    //	Display the Page Setup dialog.
    if (status == noErr)
        status = DoPageSetupDialog(printSession, &pageFormat);
                            
    //	Display the Print dialog.
    if (status == noErr)
        status = DoPrintDialog(printSession, pageFormat, &printSettings);
    
#if USING_PDE
    // Get the data stored in the page format by our PageSetup PDE
    /*
        The technique used for storing and obtaining the application 
	specific page format and print settings data is appropriate for 
	data to store in a CFData that does not need to
	be 'flattened'. Such data would be a simple data type 
	(like the Boolean here) or a structure containing simple data.
	
	More complex structures would have to be turned into a 'flat'
	representation and then stored as CFData.
    */
    if(status == noErr){
        Boolean printTitles = kPrintTitlesDefault;
        UInt32 bytesNeeded;
        
        status = PMGetPageFormatExtendedData(pageFormat, kPDE_Creator, 
                                                        &bytesNeeded, NULL);
        if(status == noErr && bytesNeeded == sizeof(printTitles) ){
            status = PMGetPageFormatExtendedData(pageFormat, kPDE_Creator, 
                                        &bytesNeeded, &printTitles);
        }
#if qOurDebug
        if(status)
            printf("got an error from PMGetPageFormatExtendedData\n");            
        printf("value of printTitles is %d\n", printTitles);
#endif
     }   

     // Get the data stored in the page format by our PrintDialog PDE
    /*
        The technique used for storing and obtaining the application 
	specific page format and print settings data is appropriate for 
	data to store in a CFData that does not need to
	be 'flattened'. Such data would be a simple data type 
	(like the Boolean here) or a structure containing simple data.
	
	More complex structures would have to be turned into a 'flat'
	representation and then stored as CFData.
    */
    if(status == noErr){
        Boolean printSelection = kPrintSelectionOnlyDefault;
        UInt32 bytesNeeded;
        
        status = PMGetPrintSettingsExtendedData(printSettings, kPDE_Creator, 
                                                        &bytesNeeded, NULL);
        if(status == noErr && bytesNeeded == sizeof(printSelection) ){
            status = PMGetPrintSettingsExtendedData(printSettings, kPDE_Creator, 
                                        &bytesNeeded, &printSelection);
        }
#if qOurDebug
        if(status)
            printf("got an error from PMGetPrintSettingsExtendedData\n");            
        printf("value of printSelection is %d\n", printSelection);
#endif
     }   
#endif
    
    //	Execute the print loop.
    if (status == noErr)
        status = DoPrintLoop(printSession, pageFormat, printSettings);
 
    if(status == kPMCancel)
        status = noErr;

    if (status != noErr)
        PostPrintingErrors(status);
        
                                                             
    //	Release the PageFormat and PrintSettings objects.  PMRelease decrements the
    //	ref count of the allocated objects.  We let the Printing Manager decide when
    //	to release the allocated memory.
    if (pageFormat != kPMNoPageFormat)
        (void)PMRelease(pageFormat);
    if (printSettings != kPMNoPrintSettings)
        (void)PMRelease(printSettings);
    
    //	Terminate the current printing session. 
    (void) PMRelease(printSession);
    
    return;
}

/*------------------------------------------------------------------------------

    Function:	DoEventLoop
    
    Parameters:
        None
    Description:
        Main event loop.

------------------------------------------------------------------------------*/
void	DoEventLoop(void)
{
    EventRecord	event;
    WindowPtr	targetWindow=NULL;
    long		menuResult;

    do {
        WaitNextEvent(everyEvent, &event, 30, nil);
        switch (event.what) {
            case mouseDown:
                /* first see where the hit was */
                switch (FindWindow(event.where, &targetWindow)) {
                    
                    case inMenuBar:
                        menuResult = MenuSelect(event.where);
                        if( menuResult == PRINTSAMPLE )
                            DoPrintSample();
                        HiliteMenu(0);
                       break;
                        
                    case inDesk: 
                    case inSysWindow:
                    case inContent:
                    case inDrag:
                    case inGrow:
                    case inGoAway:
                        /* don't care */
                        break;
                        
                }
            case nullEvent:
            case updateEvt:
            case mouseUp:
            case keyDown:
            case autoKey:
            case keyUp:
            case activateEvt:
            case networkEvt:
            case driverEvt:
            case app4Evt:
                /* don't care */
                break;
            case kHighLevelEvent:
                DoHighLevel(&event);
                break;
            default:
                break;
                
        }
    } while (gQuit != true);

}

void DoHighLevel(EventRecord *AERecord)
{
    
    AEProcessAppleEvent(AERecord);
    
}

/*------------------------------------------------------------------------------

    Function:	AEOpenHandler
    
    Parameters:
        inputEvent	-	Apple Event to process
        outputEvent	-	returned event
        handlerRefCon - ref con
    Description:
       Does nothing.
------------------------------------------------------------------------------*/
OSErr
AEOpenHandler(const AppleEvent *inputEvent, AppleEvent *outputEvent, SInt32 handlerRefCon)
{
#pragma unused (inputEvent,outputEvent,handlerRefCon)
    return(noErr);
}

/*------------------------------------------------------------------------------

    Function:	AEOpenDocHandler
    
    Parameters:
        inputEvent	-	Apple Event to process
        outputEvent	-	returned event
        handlerRefCon - ref con
    Description:
       Does nothing.

------------------------------------------------------------------------------*/
OSErr
AEOpenDocHandler(const AppleEvent *inputEvent, AppleEvent *outputEvent, SInt32 handlerRefCon)
{
#pragma unused (inputEvent,outputEvent,handlerRefCon)
    return(errAEEventNotHandled); 
    
}

/*------------------------------------------------------------------------------

    Function:	AEQuitHandler
    
    Parameters:
        inputEvent	-	Apple Event to process
        outputEvent	-	returned event
        handlerRefCon - ref con
    Description:
       Process the high level kAEQuitApplication Apple Event. Sets our
       global quit variable to true.

------------------------------------------------------------------------------*/
OSErr
AEQuitHandler(const AppleEvent *inputEvent, AppleEvent *outputEvent, SInt32 handlerRefCon)
{
#pragma unused (inputEvent,outputEvent,handlerRefCon)
    gQuit = true;
    return(noErr);
}


/*------------------------------------------------------------------------------

    Function:	AEPrintDocument
    
    Parameters:
        inputEvent	-	Apple Event to process
        outputEvent	-	returned event
        handlerRefCon - ref con
    Description:
       Process the high level kAEPrintDocuments Apple Event. A Print Settings is
       extracted from the Apple Event and coerced to a PMPrintSettings and a PMPrinter. 
       The coerced PMPrintSettings is used to print each document in the document list.
       The PMPrinter, if any, represents the desired target printer. We set the sessions
       printer to be that printer.
       
       This sample is limited to processing only the first 1024 bytes of a text file.

------------------------------------------------------------------------------*/
OSErr
AEPrintDocument( const AppleEvent *inputEvent, AppleEvent *outputEvent, SInt32 handlerRefCon)
{
#pragma unused (outputEvent,handlerRefCon)

    OSStatus	err = noErr;
    AEDesc	printSettingsDesc = {};
    AEDesc	printerDesc = {};
    AEDesc	aePrintSettingsDesc = {};
    PMPrintSettings	coercePrintSettings=kPMNoPrintSettings;
    PMPrinter	coercePrinter=NULL;
    SInt16	fileRefNum;
    Rect	windowRect={0,0,500,500};
    WindowRef	windowRef=NULL;
    Boolean		showPrintDialog=false;
    AEDesc		booleanDesc={};

    // See if we need to show the print dialog.
    err = AEGetParamDesc(inputEvent, kPMShowPrintDialogAEType, typeBoolean, &booleanDesc);
    if( !err )
        err = AEGetDescData( &booleanDesc, &showPrintDialog, sizeof( Boolean));

    // Get the AE print record.
    err = AEGetParamDesc(inputEvent, keyAEPropData, typeAERecord, &aePrintSettingsDesc);

    // Coerce it to a PMPrintSettings.
    if(!err)
        err = AECoerceDesc(&aePrintSettingsDesc, kPMPrintSettingsAEType, &printSettingsDesc);

    // Retrieve the coerced PMPrintSettings from the AEDesc.
    if(!err) {
        err = AEGetDescData( &printSettingsDesc, &coercePrintSettings, sizeof( void*));
        // We can now get rid of the AEDesc.
        err = AEDisposeDesc(&printSettingsDesc);
    }

    // Coerce it to a PMPrinter.
    if(!err) {
        err = AECoerceDesc(&aePrintSettingsDesc, kPMPrinterAEType, &printerDesc);

        // They may not have requested a target printer. But thats ok, so reset the error .
        if( !err ) {
            err = AEGetDescData( &printerDesc, &coercePrinter, sizeof( void*));
            // We can now get rid of the AEDesc.
            err = AEDisposeDesc(&printerDesc);
        } else
            err = noErr;
    }

    if( !err ) {
        long		index, itemsInList;
        AEDescList	docList;
        char		buffer[1024];
        long		count=sizeof(buffer);
        PMPrintSession session = NULL;
        PMPageFormat pageFormat = kPMNoPageFormat;

        // Create the session we'll use to print.
        PMCreateSession( &session );
        
        // Set the output to the target printer.
        if( coercePrinter )
            PMSessionSetCurrentPMPrinter( session, coercePrinter );

        // Create a default pageformat.
        PMCreatePageFormat(&pageFormat);
        PMSessionDefaultPageFormat(session, pageFormat);
            
         // Get the file list.
        err = AEGetParamDesc( inputEvent, keyDirectObject, typeAEList, &docList);
        err = AECountItems( &docList, &itemsInList);			// how many files passed in
        
       // Walk the list of files.
        for (index = 1; index <= itemsInList; index++)
        {
            FSRef 		fileRef;
            AEKeyword	keywd;
            DescType	returnedType;
            Size		actualSize;
            HFSUniStr255	fileName;
            CFStringRef		fileNameRef=NULL;
            Boolean		printIt = true;
            
            // Get the file ref.
            err = AEGetNthPtr( &docList, index, typeFSRef, &keywd, &returnedType,
                    (Ptr)(&fileRef), sizeof( fileRef ), &actualSize );

            // Get the file name to use in the window's title.
            if( !err )
                err = FSGetCatalogInfo( &fileRef, 0, NULL, &fileName,NULL,NULL);
            if( !err )
                fileNameRef = CFStringCreateWithCharacters(kCFAllocatorDefault, &fileName.unicode[0], fileName.length);

            // Open the file for reading.
            if( !err )
            {
                err = FSOpenFork(&fileRef, 0, NULL, fsRdPerm, &fileRefNum);
                if( !err )
                {
                    // read the data (1024 max)
                    FSRead(fileRefNum, &count, &buffer[0]);
                    // Close file.
                    FSClose(fileRefNum);
        
                    // Create a window to display the file's data.
                    CreateNewWindow(kDocumentWindowClass, kWindowNoAttributes, &windowRect, &windowRef);
                    MacMoveWindow(windowRef,100,100,true);
                    MacShowWindow( windowRef );
        
                    // Use the file's name as the window's title.
                    if( fileNameRef ) {
                        SetWindowTitleWithCFString( windowRef, fileNameRef );
                        CFRelease( fileNameRef );
                    }
        
                    // Display the contents of the file.
                    SetPort( GetWindowPort( windowRef ) );
                    MoveTo( 10,10 );
                    MacDrawText(&buffer[0], 0, count );
        
                    // Show the print dialog?
                    printIt = true;
                    if( showPrintDialog )
                        PMSessionPrintDialog( session, coercePrintSettings, pageFormat, &printIt );
        
                    // Print the file.
                    if( printIt )
                    {
                        PMSessionBeginDocument(session, coercePrintSettings, pageFormat);
                        PMSessionBeginPage(session, pageFormat, NULL);
                        MoveTo( 10,10 );
                        MacDrawText(&buffer[0], 0, count );
                        PMSessionEndPage(session);
                        PMSessionEndDocument(session);
                    }
        
                    DisposeWindow( windowRef );
                }
            }
        }
       
        // Clean up.
        if( pageFormat != kPMNoPageFormat )
            PMRelease( pageFormat );
        if( session != NULL )
            PMRelease( session );
    }

    // We're done so get rid of everything.
    if( coercePrintSettings != kPMNoPrintSettings )
        PMRelease( coercePrintSettings );
    if( coercePrinter != NULL )
        PMRelease( coercePrinter );
    return err;
}

/*------------------------------------------------------------------------------

    Function:	DoPageSetupDialog
    
    Parameters:
        printSession	-	current printing session
        pageFormat	-	a PageFormat object addr
    
    Description:
        If the caller passes in an empty PageFormat object, DoPageSetupDialog
        creates a new one, otherwise it validates the one provided by the caller.
        It then invokes the Page Setup dialog and checks for Cancel. Finally it
        flattens the PageFormat object so it can be saved with the document.
        Note that the PageFormat object is modified by this function.
	
------------------------------------------------------------------------------*/
OSStatus 	DoPageSetupDialog(PMPrintSession printSession, PMPageFormat* pageFormat)
{
	OSStatus	status = noErr;
	Boolean		accepted;
	
	//	Set up a valid PageFormat object.
	if (*pageFormat == kPMNoPageFormat)
            {
            status = PMCreatePageFormat(pageFormat);
		
            //	Note that PMPageFormat is not session-specific, but calling
            //	PMSessionDefaultPageFormat assigns values specific to the printer
            //	associated with the current printing session.
            if ((status == noErr) && (*pageFormat != kPMNoPageFormat))
                status = PMSessionDefaultPageFormat(printSession, *pageFormat);
            }
	else
            status = PMSessionValidatePageFormat(printSession, *pageFormat, kPMDontWantBoolean);

	//	Display the Page Setup dialog.	
	if (status == noErr)
            {
            status = PMSessionPageSetupDialog(printSession, *pageFormat, &accepted);
            if (status == noErr && !accepted)
                status = kPMCancel;		// user clicked Cancel button
            }	
				
	//	If the user did not cancel, flatten and save the PageFormat object
	//	with our document.
	if (status == noErr)
            status = FlattenAndSavePageFormat(*pageFormat);

	return status;
	
}	//	DoPageSetupDialog



/*------------------------------------------------------------------------------
	Function:	DoPrintDialog
		
	Parameters:
		printSession	-	current printing session
		pageFormat	-	a PageFormat object addr
		printSettings	-	a PrintSettings object addr
			
	Description:
		If the caller passes an empty PrintSettings object, DoPrintDialog creates
		a new one, otherwise it validates the one provided by the caller.
		It then invokes the Print dialog and checks for Cancel.
		Note that the PrintSettings object is modified by this function.
		
------------------------------------------------------------------------------*/
OSStatus 	DoPrintDialog(PMPrintSession printSession, PMPageFormat pageFormat,
				PMPrintSettings* printSettings)
{
	OSStatus	status = noErr;
	Boolean		accepted;
	UInt32		realNumberOfPagesinDoc;
	
	//	In this sample code the caller provides a valid PageFormat reference but in
	//	your application you may want to load and unflatten the PageFormat object
	//	that was saved at PageSetup time.  See LoadAndUnflattenPageFormat below.
	
	//	Set up a valid PrintSettings object.
	if (*printSettings == kPMNoPrintSettings)
            {
            status = PMCreatePrintSettings(printSettings);	

            //	Note that PMPrintSettings is not session-specific, but calling
            //	PMSessionDefaultPrintSettings assigns values specific to the printer
            //	associated with the current printing session.
            if ((status == noErr) && (*printSettings != kPMNoPrintSettings))
                status = PMSessionDefaultPrintSettings(printSession, *printSettings);
            }
	else
            status = PMSessionValidatePrintSettings(printSession, *printSettings, kPMDontWantBoolean);
	
	//	Before displaying the Print dialog, we calculate the number of pages in the
	//	document.  On Mac OS X this is useful because we can prime the Print dialog
	//	with the actual page range of the document and prevent the user from entering
	//	out-of-range numbers.  This is not possible on Mac OS 8 and 9 because the driver,
	//	not the printing manager, controls the page range fields in the Print dialog.

	//	Calculate the number of pages required to print the entire document.
	if (status == noErr)
            status = DetermineNumberOfPagesInDoc(pageFormat, &realNumberOfPagesinDoc);

	//	Set a valid page range before displaying the Print dialog
	if (status == noErr)
            status = PMSetPageRange(*printSettings, 1, realNumberOfPagesinDoc);

	//	Display the Print dialog.
	if (status == noErr)
            {
            status = PMSessionPrintDialog(printSession, *printSettings, pageFormat, &accepted);
            if (status == noErr && !accepted)
                status = kPMCancel;		// user clicked Cancel button
            }
		
	return status;
	
}	//	DoPrintDialog


#define BUILDINGONTIGER 1   // Change to 0 to build on Pre-Tiger systems.

static OSStatus MyPMSessionBeginCGDocument(PMPrintSession printSession, 
                PMPrintSettings printSettings, PMPageFormat pageFormat)
{
    OSStatus err = noErr;
    // Tell the printing system we want to print by drawing to a CGContext,
    // not a QD port. 

#if BUILDINGONTIGER    
    // Use the simpler call if it is present.
    if(&PMSessionBeginCGDocument != nil){
	err = PMSessionBeginCGDocument(printSession, printSettings, pageFormat);
    }else{
#endif
        CFStringRef s[1] = { kPMGraphicsContextCoreGraphics };
        CFArrayRef  graphicsContextsArray = CFArrayCreate(NULL, (const void**)s, 1, &kCFTypeArrayCallBacks);
        err = PMSessionSetDocumentFormatGeneration(printSession, kPMDocumentFormatPDF, graphicsContextsArray, NULL);
        CFRelease(graphicsContextsArray);
	if(!err)
	    err = PMSessionBeginDocument(printSession, printSettings, pageFormat);
#if BUILDINGONTIGER    
    }
#endif
    return err;
}

static OSStatus MyPMSessionGetCGGraphicsContext(PMPrintSession printSession, CGContextRef *printingContextP)
{
    OSStatus err = noErr;
#if BUILDINGONTIGER    
    // Use the simpler call if it is present.
    if(&PMSessionGetCGGraphicsContext != nil){
	err = PMSessionGetCGGraphicsContext(printSession, printingContextP);
    }else{
#endif
    err = PMSessionGetGraphicsContext(printSession, kPMGraphicsContextCoreGraphics, (void**)printingContextP);
#if BUILDINGONTIGER    
    }
#endif
    return err;
}

/*------------------------------------------------------------------------------
	Function:
		DoPrintLoop
	
	Parameters:
		printSession	-	current printing session
		pageFormat	-	a PageFormat object addr
		printSettings	-	a PrintSettings object addr
	
	Description:
		DoPrintLoop calculates which pages to print and executes the print
		loop, calling DrawPage for each page.
				
------------------------------------------------------------------------------*/
OSStatus DoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat,
            PMPrintSettings printSettings)
{
    OSStatus	status = noErr, tempErr;
    UInt32	realNumberOfPagesinDoc,
                pageNumber,
                firstPage,
                lastPage;
    CFStringRef	jobName = CFSTR("Carbon Printing Sample");

    //	Since this sample code doesn't have a window, give the spool file a name.
    status = PMSetJobNameCFString(printSettings, jobName);

    //	Get the user's Print dialog selection for first and last pages to print.
    if (status == noErr)
        {
        status = PMGetFirstPage(printSettings, &firstPage);
        if (status == noErr)
            status = PMGetLastPage(printSettings, &lastPage);
        }

    //	Check that the selected page range does not exceed the actual number of
    //	pages in the document.
    if (status == noErr)
        {
        status = DetermineNumberOfPagesInDoc(pageFormat, &realNumberOfPagesinDoc);
        if (realNumberOfPagesinDoc < lastPage)
            lastPage = realNumberOfPagesinDoc;
        }

    //	Before executing the print loop, tell the Carbon Printing Manager which pages
    //	will be spooled so that the progress dialog can reflect an accurate page count.
    //	This is recommended on Mac OS X.  On Mac OS 8 and 9, we have no control over
    //	what the printer driver displays.
	
    if (status == noErr)
        status = PMSetFirstPage(printSettings, firstPage, false);
    if (status == noErr)
        status = PMSetLastPage(printSettings, lastPage, false);
    	
    //	Note, we don't have to worry about the number of copies.  The printing
    //	manager handles this.  So we just iterate through the document from the
    //	first page to be printed, to the last.
    if (status == noErr)
        {
            //	Begin a new print job.
            status = MyPMSessionBeginCGDocument(printSession, printSettings, pageFormat);
            if (status == noErr)
            {
                //	Print the selected range of pages in the document.		
                pageNumber = firstPage;
            
                /* Note that we check PMSessionError immediately before beginning a new
                    page. This handles user cancelling appropriately. Also, if we got
		    an error on any previous iteration of the print loop, we break
		    out of the loop.
                */
                while ( (pageNumber <= lastPage) && 
		    (status == noErr) && (PMSessionError(printSession) == noErr) )
                {
                //	Note, we don't have to deal with the classic Printing Manager's
                //	128-page boundary limit.
				
                //	Set up a page for printing.  Under the classic Printing Manager, applications
                //	could provide a page rect different from the one in the print record to achieve
                //	scaling. This is no longer recommended and on Mac OS X, the PageRect argument
                //	is ignored.
                status = PMSessionBeginPage(printSession, pageFormat, NULL);
                if (status == noErr){
		    CGContextRef printingContext;
                        
                    //	Get the printing graphics context, in this case a Quartz context,
                    //	for drawing the page. The origin is the lower left corner of the sheet with
		    //  the y axis going up the page.
                    status = MyPMSessionGetCGGraphicsContext(printSession, &printingContext);
                    if (status == noErr) {
                        //	Draw the page.
                        status = DrawPage(printingContext, pageNumber);
                     }
                                    
                    //	Close the page.
                    tempErr = PMSessionEndPage(printSession);
		    if(status == noErr)
			status = tempErr;
                }
                    
                //	And loop.
                    pageNumber++;
                } // end while loop
			
                // Close the print job.  This dismisses the progress dialog on Mac OS X.
            	tempErr = PMSessionEndDocument(printSession);
                if(status == noErr)
                    status = tempErr;
            }
        }
		
	//	Only report a printing error once we have completed the print loop. This
	//	ensures that every PMBeginXXX call that returns no error is followed by
        //	a matching PMEndXXX call, so the Printing Manager can release all temporary 
        //	memory and close properly.
	tempErr = PMSessionError(printSession);
        if(status == noErr)
            status = tempErr;
	if (status != noErr && status != kPMCancel)
            PostPrintingErrors(status);
		
    return status;
}	//	DoPrintLoop



/*------------------------------------------------------------------------------
	Function:
		FlattenAndSavePageFormat
	
	Parameters:
		pageFormat	-	a PageFormat object
	
	Description:
		Flattens a PageFormat object so it can be saved with the document.
		Assumes caller passes a validated PageFormat object.
		
------------------------------------------------------------------------------*/
OSStatus FlattenAndSavePageFormat(PMPageFormat pageFormat)
{
    OSStatus	status;
    Handle	flatFormatHandle = NULL;
	
    //	Flatten the PageFormat object to memory.
    status = PMFlattenPageFormat(pageFormat, &flatFormatHandle);

    if(status == noErr){
        //	Write the PageFormat data to file.
        //	In this sample code we simply copy it to a global.	
        gflatPageFormat = flatFormatHandle;
    }

    return status;
}	//	FlattenAndSavePageFormat



/*------------------------------------------------------------------------------
    Function:	LoadAndUnflattenPageFormat
	
    Parameters:
        pageFormat	- PageFormat object read from document file
	
    Description:
        Gets flattened PageFormat data from the document and returns a PageFormat
        object.
        The function is not called in this sample code but your application
        will need to retrieve PageFormat data saved with documents.
		
------------------------------------------------------------------------------*/
OSStatus	LoadAndUnflattenPageFormat(PMPageFormat* pageFormat)
{
    OSStatus	status = noErr;
    Handle	flatFormatHandle = NULL;

    //	Read the PageFormat flattened data from file.
    //	In this sample code we simply copy it from a global.
    flatFormatHandle = gflatPageFormat;
    if(flatFormatHandle){
        //	Convert the PageFormat flattened data into a PageFormat object.
        status = PMUnflattenPageFormat(flatFormatHandle, pageFormat);
    }else{
        *pageFormat = kPMNoPageFormat;
    }
	
    return status;
}	//	LoadAndUnflattenPageFormat



/*------------------------------------------------------------------------------
    Function:	DetermineNumberOfPagesInDoc
	
    Parameters:
    	pageFormat	- a PageFormat object addr
        numPages	- on return, the size of the document in pages
			
    Description:
    	Calculates the number of pages needed to print the entire document.
		
------------------------------------------------------------------------------*/
OSStatus	DetermineNumberOfPagesInDoc(PMPageFormat pageFormat, UInt32* numPages)
{
    OSStatus	status;
    PMRect	pageRect;

    //	PMGetAdjustedPageRect returns the page size taking into account rotation,
    //	resolution and scaling settings.
    status = PMGetAdjustedPageRect(pageFormat, &pageRect);

    //	In this sample code we simply return a hard coded number.  In your application,
    //	you will need to figure out how many page rects are needed to image the
    //	current document.
    *numPages = NUMPAGES;

    return status;
    
}	//	DetermineNumberOfPagesinDoc



/*------------------------------------------------------------------------------
    Function:	DrawPage
	
    Parameters:
        printSession	- current printing session
        pageNumber	- the logical page number in the document
	
    Description:
        Draws the contents of a single page.
		
------------------------------------------------------------------------------*/
OSStatus DrawPage(CGContextRef context, UInt32 pageNumber)
{
    char text[1024];
    float fontSize = 24;
    //	In this sample code we do some very simple text drawing.    
    CGContextSelectFont(context, "Times-Roman", fontSize, 
					    kCGEncodingMacRoman);
    snprintf(text, sizeof(text), "Drawing Page Number %d", (int)pageNumber);
    // Draw 72 units in y below the top of a letter size page.
    CGContextShowTextAtPoint(context, 72, 792-72, text, strlen(text));
			
    return noErr;
}	//	DrawPage

/*------------------------------------------------------------------------------
    Function:	PostPrintingErrors
	
    Parameters:
        status	-	error code
	
    Description:
        This is where we could post an alert to report any problem reported
        by the Printing Manager.
		
------------------------------------------------------------------------------*/
void 	PostPrintingErrors(OSStatus status)
{
#pragma unused (status)	
}	//	PostPrintingErrors

