/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

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
/*=============================================================================
	AudioFileChooser.h
	
=============================================================================*/

#ifndef __CAFileChooser_h__
#define __CAFileChooser_h__

#include "XControl.h"
#include <AudioUnit/AudioUnit.h>
#include <map>

class ChooserAction {
public:
	virtual void SelectedFile (const FSRef& inFSRef) = 0;
};
	
class CAFileChooser : public XControl {
public:
	CAFileChooser(XWindow *aSuper, OSType signature, UInt32 controlID, ChooserAction* inResponder);
	virtual ~CAFileChooser() {}

	void AddFilesToPresent (FSRef* files, int numFiles, DataBrowserItemID inPosition = 0);

protected:
	virtual bool		IsAcceptableFile (const HFSFlavor& fileData, FSRef* outRef = NULL) = 0;
	
	
private:
	void 	SetItemText (DataBrowserItemID itemID, DataBrowserItemDataRef itemData);
	void	ItemSelected (DataBrowserItemID itemID);
	
	DataBrowserCallbacks mCallbacks;
	
	struct FileDetails {
		FSRef fileLocation;
		CFStringRef fileName;
		
		FileDetails () : fileName (0) {	memset(&fileLocation, 0, sizeof(fileLocation));	}
		~FileDetails () {	if (fileName) CFRelease (fileName); }
		
		FileDetails (const FileDetails& a) : fileName(0) { *this = a; }
		FileDetails& operator= (const FileDetails& a) 
		{
			::memcpy(&fileLocation, &(a.fileLocation), sizeof(fileLocation));
			if (fileName) CFRelease (fileName);
			fileName = a.fileName;
			if (fileName) CFRetain (fileName);
			return *this;
		}
	};
	typedef std::multimap<DataBrowserItemID, FileDetails, std::less<int> > FileMap;
	
	FileMap mFiles;
	int fileKey;
	
	ChooserAction* mResponder;

	static OSStatus SimpleDataCallback (ControlRef browser, 
									DataBrowserItemID itemID, 
									DataBrowserPropertyID property, 
									DataBrowserItemDataRef itemData,
									Boolean changeValue);
	
	static void 	SimpleItemNotification ( ControlRef browser,
									DataBrowserItemID itemID,
									DataBrowserItemNotification message);
	
	static Boolean 	SimpleAcceptDragProc(ControlRef browser,
									DragReference theDrag,
									DataBrowserItemID item);
	
	static Boolean 	SimpleReceiveDrag (ControlRef browser,
									DragReference theDrag,
									DataBrowserItemID item);
	
};

class AudioFileChooser : public CAFileChooser
{
public:
	AudioFileChooser(XWindow *aSuper, OSType signature, UInt32 controlID, ChooserAction* inResponder)
		: CAFileChooser (aSuper, signature, controlID, inResponder) {}

protected:
	bool		IsAcceptableFile (const HFSFlavor& fileData, FSRef* outRef = NULL)
	{
		return IsAudioFile (fileData, outRef);
	}

private:
	static bool		IsAudioFile (const HFSFlavor& flavour, FSRef* outRef);
};

class MIDIFileChooser : public CAFileChooser
{
public:
	MIDIFileChooser(XWindow *aSuper, OSType signature, UInt32 controlID, ChooserAction* inResponder)
		: CAFileChooser (aSuper, signature, controlID, inResponder) {}

protected:
	bool		IsAcceptableFile (const HFSFlavor& fileData, FSRef* outRef = NULL)
	{
		return IsMIDIFile (fileData, outRef);
	}

private:
	static bool 	IsMIDIFile (const HFSFlavor& flavour, FSRef* outRef);
};

#endif // __CAFileChooser_h__
