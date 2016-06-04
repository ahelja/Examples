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

/* PDECommon.h 

Copyright (c) 1998-2001 Apple Computer. All rights reserved.

*/

#define kPDE_Creator		'paul'		

/* NOTE:
    The scheme for naming the keys used for storing the application specific PDE
    data relies on the way PMGetPageFormatExtendedData and PMGetPrintSettingsExtendedData
    retrieve application data from the PageFormat and PrintSettings respectively. The
    key for storing the page format data must be a CFString that begins with
    "com.apple.print.PageFormatTicket." and has a 4 byte identifier at the end. The
    key for storing the print settings data must be a CFString that begins with
    ""com.apple.print.PrintSettingsTicket." and has a 4 byte identifier at the end.
    Here we use 'paul' as the 4 byte identifier.
    
    In our PDEs, when we store or retrieve our data from the PrintSettingsTicket or
    PageFormatTicket, we use the fully qualified CFString. In our application, when
    we retrieve our data from the PMPageFormat or PMPrintSettings, we use
    PMGetPageFormatExtendedData or PMGetPrintSettingsExtendedData and supply our
    4 byte identifier.
    
    One additional note: the 4 byte identifier must NOT contain any characters outside
    the standard ASCII 7 bit character range 0x20-0x7F.
*/
// Our tag for the PageFormat ticket. This should be defined in an application
// header that is common to this file and the applications' files so that it
// can get access to the data set in the PageFormat.
#define kAppPageSetupPDEOnlyKey 		CFSTR("com.apple.print.PageFormatTicket.paul")

#define kPrintTitlesDefault	false	// the initial default value of our page format custom data

// Our tag for the Print Settings ticket. This should be defined in an application
// header that is common to this file and the applications' files so that it
// can get access to the data set in the Print Settings.
#define kAppPrintDialogPDEOnlyKey 		CFSTR("com.apple.print.PrintSettingsTicket.paul")

#define kPrintSelectionOnlyDefault	false	// the initial default value of custom print settings custom data
