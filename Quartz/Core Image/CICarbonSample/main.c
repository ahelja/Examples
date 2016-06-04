//
//  main.c
//  CICarbonSample
//
//  Created by frank on 2/11/05.
//  Copyright __MyCompanyName__ 2005. All rights reserved.
//

#include <Carbon/Carbon.h>
#include "CIDraw.h"

/* Constants */
#define kMyHIViewSignature 'ciHV'
#define kMyHIViewFieldID    128
#define kGammaSliderSignature 'gSLD'
#define kGammaSliderFieldID    128
#define	kAboutBoxStringKey		CFSTR("AboutString")	// these key the localizable strings

/* Private Prototypes */
static OSStatus MyDrawEventHandler(EventHandlerCallRef myHandler, EventRef event, void *userData);
static void MyGammaSliderProc( ControlHandle control, SInt16 part );
static pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
static PMPageFormat CreateDefaultPageFormat(void);
static OSStatus DoPageSetup(PMPageFormat pageFormat);
static OSStatus DoPrint(PMPageFormat pageFormat);
static OSStatus MyDoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat, PMPrintSettings printSettings);
static void DoAboutBox();

/* Global Data */
static HIViewRef    gMyHIView = NULL;
static HIViewRef    gGammaSliderView = NULL;
static PMPageFormat gPageFormat = NULL;

int main(int argc, char* argv[])
{
    static const HIViewID	kMyViewID = {kMyHIViewSignature,  kMyHIViewFieldID };      
    static const HIViewID	kGammaSliderID = {kGammaSliderSignature,  kGammaSliderFieldID };      

    IBNibRef			nibRef;
    WindowRef			window;
    EventTargetRef		myEventTarget;
    static const EventTypeSpec 	kMyViewEvents[] = {kEventClassControl, kEventControlDraw };	
    static const EventTypeSpec 	kMyCommandEvents[] = {kEventClassCommand, kEventCommandProcess };	
    OSStatus			err = noErr;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );
    // Get the HIView associated with the window.
    HIViewFindByID( HIViewGetRoot( window ), kMyViewID, &gMyHIView );
    
    // make the view opaque
    HIViewChangeFeatures(gMyHIView, kHIViewIsOpaque, 0);

    // Get the event target for the view.
    myEventTarget = GetControlEventTarget (gMyHIView);
			     
    // Install the event handler for the HIView.				
    err = InstallEventHandler (myEventTarget, 
						    NewEventHandlerUPP (MyDrawEventHandler), 
						    GetEventTypeCount(kMyViewEvents), 
						    kMyViewEvents, 
						    (void *) gMyHIView, 
						    NULL); 


    HIViewFindByID( HIViewGetRoot( window ), kGammaSliderID, &gGammaSliderView );
    SetControlAction( gGammaSliderView, NewControlActionUPP(MyGammaSliderProc) );

    // Install the handler for the menu commands.
    InstallApplicationEventHandler(NewEventHandlerUPP(DoAppCommandProcess), GetEventTypeCount(kMyCommandEvents), 
						kMyCommandEvents, NULL, NULL);


    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // The window was created hidden so show it.
    ShowWindow( window );
    
    // Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}


static OSStatus MyDrawEventHandler(EventHandlerCallRef myHandler, EventRef event, void *userData)
{
	// NOTE: GState is save/restored by the HIView system doing the callback, so the draw handler doesn't need to do it

	OSStatus status = noErr;
	CGContextRef context;
	HIRect		bounds;

	// Get the CGContextRef
	status = GetEventParameter (event, kEventParamCGContextRef, 
					typeCGContextRef, NULL, 
					sizeof (CGContextRef),
					NULL,
					&context);

	if(status != noErr){
		fprintf(stderr, "Got error %d getting the context!\n", status);
		return status;
	}		
						
	// Get the bounding rectangle
	HIViewGetBounds ((HIViewRef) userData, &bounds);
	
	// Flip the coordinates by translating and scaling. This produces a
	// coordinate system that matches the Quartz default coordinate system
	// with the origin in the lower-left corner with the y axis pointing up.
	
	CGContextTranslateCTM (context, 0, bounds.size.height);
	CGContextScaleCTM (context, 1.0, -1.0);

	DoDraw(context, bounds);
	return status;
   
}

static void MyGammaSliderProc( ControlHandle control, SInt16 part )
{	
	gGammaValue = GetControl32BitValue(control);
	HIViewSetNeedsDisplay(gMyHIView, true);
	HIViewRender(gMyHIView);   
}


// Handle command-process events at the application level
static pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
#pragma unused (nextHandler, userData)
    HICommand  aCommand;
    OSStatus   result = eventNotHandledErr;

    GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
    
    switch (aCommand.commandID)
    {

	case kHICommandPageSetup:
	    if(gPageFormat == NULL)
		gPageFormat = CreateDefaultPageFormat();
		
	    if(gPageFormat)
		(void)DoPageSetup(gPageFormat);
		
	    result = noErr;
	    break;

	case kHICommandPrint:
	    if(gPageFormat == NULL)
		gPageFormat = CreateDefaultPageFormat();
		
	    if(gPageFormat)
		(void)DoPrint(gPageFormat);

	    result = noErr;
	    break;

	case kHICommandAbout:
	    DoAboutBox();
	    result = noErr; 
	    break;

	default:
	    break;

	case kHICommandQuit:
		QuitApplicationEventLoop();
		result = noErr;
		break;
    }
    HiliteMenu(0);
    return result;
}

static void DoAboutBox()
{	
    CFStringRef outString = NULL;
    SInt16      alertItemHit = 0;
    Str255      stringBuf;

    outString =  CFCopyLocalizedString(kAboutBoxStringKey, NULL);
    if (outString != NULL)
    {
		if (CFStringGetPascalString (outString, stringBuf, sizeof(stringBuf), GetApplicationTextEncoding()))
		{
			StandardAlert(kAlertStopAlert, stringBuf, NULL, NULL, &alertItemHit);
		}
		CFRelease (outString);                             
    }
}

// -----------------------------------------------------------------------
static PMPageFormat CreateDefaultPageFormat(void)
{
    OSStatus err = noErr, tempErr;
    PMPageFormat pageFormat = NULL;
    PMPrintSession printSession;
    err = PMCreateSession(&printSession);
    if(!err){
	err = PMCreatePageFormat(&pageFormat);	// we own a reference to this page format
	if(err == noErr)
	    err = PMSessionDefaultPageFormat(printSession, pageFormat);

	tempErr = PMRelease(printSession);
	if(!err)err = tempErr;
    }
    if(err){
	fprintf(stderr, "got an error = %d creating the default page format\n", err);
    }
    return pageFormat;
}

// -----------------------------------------------------------------
static OSStatus DoPageSetup(PMPageFormat pageFormat)
{
    OSStatus		err = noErr;
    PMPrintSession printSession;
    err = PMCreateSession(&printSession);
    if(!err){
	Boolean accepted;
	if(!err) 	// validate the page format we're going to pass to the dialog code
	    err = PMSessionValidatePageFormat(printSession, pageFormat, kPMDontWantBoolean);
	if(!err){
	    err = PMSessionPageSetupDialog(printSession, pageFormat, &accepted);
	}
       (void)PMRelease(printSession);
    }
    
    if(err && err != kPMCancel)
	fprintf(stderr, "Got an error %d in Page Setup\n", err);

    return err;
} // DoPageSetup

// -------------------------------------------------------------------------------
static OSStatus DoPrint(PMPageFormat pageFormat)
{
    OSStatus err = noErr;
    UInt32 minPage = 1, maxPage = 1;
    PMPrintSession printSession;
    err = PMCreateSession(&printSession);
    if(err == noErr){
	// validate the page format we're going to use
	err = PMSessionValidatePageFormat(printSession, 
			    pageFormat,
			    kPMDontWantBoolean);
        if (err == noErr)
        {
	    PMPrintSettings printSettings = NULL;
            err = PMCreatePrintSettings(&printSettings);
            if(err == noErr)
                err = PMSessionDefaultPrintSettings(printSession, printSettings);

            if (err == noErr)
		err = PMSetPageRange(printSettings, minPage, maxPage);

            if (err == noErr)
            {
                Boolean accepted;
		err = PMSessionPrintDialog(printSession, printSettings, 
				pageFormat,
				&accepted);
		if(accepted){
		    err = MyDoPrintLoop(printSession, pageFormat, printSettings);
		}
            }
	    if(printSettings)
		(void)PMRelease(printSettings);
        }

        (void)PMRelease(printSession);   // ignoring error since we already have one 
    }
    
    if(err && err != kPMCancel)
	fprintf(stderr, "Got an error %d in Print\n", err);
    return err;
}

// --------------------------------------------------------------------------------------
static OSStatus MyDoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat, PMPrintSettings printSettings)
{
    OSStatus err = noErr;
    OSStatus tempErr = noErr;
    UInt32 firstPage, lastPage, totalDocPages = 1;
    
    if(!err)
	err = PMGetFirstPage(printSettings, &firstPage);
	
    if (!err)
        err = PMGetLastPage(printSettings, &lastPage);

    if(!err && lastPage > totalDocPages){
        // don't draw more than the number of pages in our document
        lastPage = totalDocPages;
    }

    if (!err)		// tell the printing system the number of pages we are going to print
        err = PMSetLastPage(printSettings, lastPage, false);

    if (!err)
    {
        err = PMSessionBeginCGDocument(printSession, printSettings, pageFormat);
        if (!err){
	    UInt32 pageNumber = firstPage;
	    // need to check errors from our print loop and errors from the session for each
	    // time around our print loop before calling our BeginPageProc
            while(pageNumber <= lastPage && err == noErr && PMSessionError(printSession) == noErr)
            {
                err = PMSessionBeginPage(printSession, pageFormat, NULL);
                if (!err){
                    CGContextRef printingContext = NULL;
                    err = PMSessionGetCGGraphicsContext(printSession, &printingContext);
                    if(!err){
			PMRect       pageRect;
			
			PMGetAdjustedPaperRect(pageFormat, &pageRect);
			DoDraw(printingContext, CGRectMake(pageRect.left, pageRect.top, pageRect.right - pageRect.left, pageRect.bottom - pageRect.top));
                    }
                    // we must call EndPage if BeginPage returned noErr
		    tempErr = PMSessionEndPage(printSession);
                        
		    if(!err)err = tempErr;
                }
		pageNumber++;
            }	// end while loop
            
            // we must call EndDocument if BeginDocument returned noErr
	    tempErr = PMSessionEndDocument(printSession);

	    if(!err)err = tempErr;
	    if(!err)
		err = PMSessionError(printSession);
        }
    }
    return err;
}



