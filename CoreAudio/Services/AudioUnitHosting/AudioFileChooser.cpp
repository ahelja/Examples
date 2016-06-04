/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
/*=============================================================================
	AudioFileChooser.cpp
	
=============================================================================*/

#include "AudioFileChooser.h"
#include "AudioUnitHosting.h"


CAFileChooser::CAFileChooser(XWindow *aSuper, OSType signature, UInt32 controlID, ChooserAction* inResponder)
	: XControl (aSuper, signature, controlID),
	  fileKey (1),
	  mResponder (inResponder)
{
	SetAutomaticControlDragTrackingEnabledForWindow (aSuper->MacWindow(), true);

	mCallbacks.version = kDataBrowserLatestCallbacks;
	InitDataBrowserCallbacks(&mCallbacks);
	mCallbacks.u.v1.itemDataCallback = SimpleDataCallback;
	mCallbacks.u.v1.itemNotificationCallback = SimpleItemNotification;
	mCallbacks.u.v1.acceptDragCallback = SimpleAcceptDragProc;
	mCallbacks.u.v1.receiveDragCallback = SimpleReceiveDrag;
	
	SetDataBrowserCallbacks (MacControl(), &mCallbacks);
}
	
void CAFileChooser::AddFilesToPresent (FSRef* files, int numFiles, DataBrowserItemID inPosition)
{
    DataBrowserItemID* items = (DataBrowserItemID*)malloc (sizeof (DataBrowserItemID) * numFiles);
	
    for (int i = 0; i < numFiles ; ++i) {
		items[i] = fileKey++;
		FileDetails details;
        
		CFURLRef url = CFURLCreateFromFSRef(NULL, &files[i]);
        if (url) {
            details.fileName = CFURLCopyLastPathComponent(url);
            CFRelease(url);
        } else {
            continue;
        }
        memcpy(&(details.fileLocation), &files[i], sizeof(FSRef));
        
		mFiles.insert (mFiles.end(), FileMap::value_type (items[i], details));
	} 
			
	AddDataBrowserItems (MacControl(), kDataBrowserNoItem, numFiles, items, kDataBrowserItemNoProperty);
	
	free (items);
}

void 	CAFileChooser::SetItemText (DataBrowserItemID itemID, DataBrowserItemDataRef itemData)
{
	FileMap::iterator iter = mFiles.find (itemID);
	SetDataBrowserItemDataText (itemData, (*iter).second.fileName);
}

void	CAFileChooser::ItemSelected (DataBrowserItemID itemID)
{
	if (mResponder) {
		FileMap::iterator iter = mFiles.find (itemID);
		mResponder->SelectedFile ((*iter).second.fileLocation);
	}
}

bool	AudioFileChooser::IsAudioFile (const HFSFlavor& flavour, FSRef* outRef)
{
	if (flavour.fileType != 0) {
		if (flavour.fileType == kAudioFileSoundDesigner2Type
			|| flavour.fileType == kAudioFileAIFFType
			|| flavour.fileType == kAudioFileAIFCType	
			|| flavour.fileType == kAudioFileWAVEType)
		{
			if (outRef) {
				FSRef 					fsRef;
				OSStatus result = FSpMakeFSRef (&flavour.fileSpec, &fsRef);			
					if (result) return false;
				*outRef = fsRef;
			}
			
			return true;
		}
	}
		// if we don't match here we'll still check the extension
	OSStatus				result = noErr;
	LSItemInfoRecord		info;
	FSRef 					fsRef;
	
	result = FSpMakeFSRef (&flavour.fileSpec, &fsRef);			
		if (result) return false;
	
	info.extension = 0;

	result = LSCopyItemInfoForRef (&fsRef, kLSRequestExtension, &info);
	
	if (info.extension) {
		bool res = false;
		if (!CFStringCompare(info.extension, CFSTR("aif"), kCFCompareCaseInsensitive))
			res = true;
		else if (!CFStringCompare(info.extension, CFSTR("aifc"), kCFCompareCaseInsensitive))
			res = true;
		else if (!CFStringCompare(info.extension, CFSTR("aiff"), kCFCompareCaseInsensitive))
			res = true;
		else if (!CFStringCompare(info.extension, CFSTR("wav"), kCFCompareCaseInsensitive))
			res = true;
		else if (!CFStringCompare(info.extension, CFSTR("sd2"), kCFCompareCaseInsensitive))
			res = true;
		
		if (res && outRef)
			*outRef = fsRef;
		
		CFRelease (info.extension);
		
		return res;
	} else
		return false;
}

bool	MIDIFileChooser::IsMIDIFile (const HFSFlavor& flavour, FSRef* outRef)
{
	if (flavour.fileType != 0) {
		if (flavour.fileType == 'Midi')
		{
			if (outRef) {
				FSRef 					fsRef;
				OSStatus result = FSpMakeFSRef (&flavour.fileSpec, &fsRef);			
					if (result) return false;
				*outRef = fsRef;
			}
			
			return true;
		}
	}
		// if we don't match here we'll still check the extension
	OSStatus				result = noErr;
	LSItemInfoRecord		info;
	FSRef 					fsRef;
	
	result = FSpMakeFSRef (&flavour.fileSpec, &fsRef);			
		if (result) return false;
	
	info.extension = 0;

	result = LSCopyItemInfoForRef (&fsRef, kLSRequestExtension, &info);
	
	if (info.extension) {
		bool res = false;
		if (!CFStringCompare(info.extension, CFSTR("mid"), kCFCompareCaseInsensitive))
			res = true;
		else if (!CFStringCompare(info.extension, CFSTR("smf"), kCFCompareCaseInsensitive))
			res = true;
		else if (!CFStringCompare(info.extension, CFSTR("midi"), kCFCompareCaseInsensitive))
			res = true;
		else if (!CFStringCompare(info.extension, CFSTR("rmid"), kCFCompareCaseInsensitive))
			res = true;
		
		if (res && outRef)
			*outRef = fsRef;
		
		CFRelease (info.extension);
		
		return res;
	} else
		return false;
}

OSStatus CAFileChooser::SimpleDataCallback (ControlRef browser, 
									DataBrowserItemID itemID, 
									DataBrowserPropertyID property, 
									DataBrowserItemDataRef itemData,
									Boolean changeValue)
{
	if (!changeValue && (property == 'file')) {
		CAFileChooser* THIS = (CAFileChooser*)::GetControlReference(browser);
		THIS->SetItemText (itemID, itemData);
	}
	return 0;
}

void CAFileChooser::SimpleItemNotification ( ControlRef browser,
									DataBrowserItemID itemID,
									DataBrowserItemNotification message)
{
	if (message == kDataBrowserItemSelected) {
		CAFileChooser* THIS = (CAFileChooser*)::GetControlReference(browser);
		THIS->ItemSelected (itemID);
	}
}

Boolean CAFileChooser::SimpleAcceptDragProc(ControlRef browser,
									DragReference theDrag,
									DataBrowserItemID item)
{
	UInt16 items;
	CountDragItems(theDrag, &items);

	CAFileChooser* THIS = (CAFileChooser*)::GetControlReference(browser);
		
	for (int i = 1; i <= items; i++) {
		DragItemRef theItemRef;
		GetDragItemReferenceNumber (theDrag, i, &theItemRef);
		FlavorFlags theFlags = 0;
		OSStatus result = GetFlavorFlags(theDrag, theItemRef, kDragFlavorTypeHFS, &theFlags);
		
		if (result == noErr) {
			HFSFlavor fileData;
			Size size = sizeof(fileData);
			GetFlavorData (theDrag, theItemRef, kDragFlavorTypeHFS, &fileData, &size, 0L);

			if (THIS->IsAcceptableFile (fileData))
				return true; //there's at least one audio file here, so we'll accept the drag
		}
	}

	return false;
}
  
Boolean CAFileChooser::SimpleReceiveDrag (ControlRef browser,
									DragReference theDrag,
									DataBrowserItemID item)
{
	UInt16 items;
	UInt16 validItems = 0;
	
	CAFileChooser* THIS = (CAFileChooser*)::GetControlReference(browser);

	CountDragItems(theDrag, &items);
	
	FSRef* thePaths = (FSRef*)malloc (items * sizeof (FSRef));
	
	for (int i = 1; i <= items; i++) {
		DragItemRef theItemRef;
		GetDragItemReferenceNumber (theDrag, i, &theItemRef);
		FlavorFlags theFlags = 0;
		OSStatus result = GetFlavorFlags(theDrag, theItemRef, kDragFlavorTypeHFS, &theFlags);
		
		if (result == noErr) {
			HFSFlavor fileData;
			Size size = sizeof(fileData);
			GetFlavorData (theDrag, theItemRef, kDragFlavorTypeHFS, &fileData, &size, 0L);
			FSRef ref;
			if (THIS->IsAcceptableFile (fileData, &ref)) {
                memcpy(&thePaths[validItems], &ref, sizeof(FSRef));
                ++validItems;
			}
		}
	}
    
	THIS->AddFilesToPresent (thePaths, validItems, item);
	
	free (thePaths);
	
	return (validItems != 0);
}					
