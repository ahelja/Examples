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
 *  CAFileBrowser_Protected.h
 *  PlayPen
 *-----------------------------------------------------------------------------
 *
 *=============================================================================*/

@interface CAFileBrowser(Protected)
#pragma mark ____ PUBLIC FUNCTIONS ____
- (BOOL)writePropertyList:(CFPropertyListRef)inPropertyList withName:(NSString *)inName asChildOfItem:(id)parent;
	// typically called by subclasses in their overridden method:
	// - (BOOL)savePresetWithName:(NSString *)inPresetName asChildOf:(id)item;
	// where the subclass produces the CFPropertyListRef, and passes the other
	// arguments through for CAFileBrowser to handle.

- (void)setCAFileHandlingObject:(CAFileHandling *)inCAFileHandlingObject;
	// called by subclasses to set the CAFileHandling derivative object they've
	// created for display.  CAFileBrowser just keeps a reference to that object,
	// so it's the subclass' responsibility to keep that object alive for the
	// duration of the lifespan of this, its superclass.
	
- (void)unsetCAFileHandlingObject;
	// called by subclasses before deleting the CAFileHandling object previously set
	// on them.  It is essential that this call be made before subsequent calls to
	// setCAFileHandlingObject are made.
	
- (CAFileHandling *)CAFileHandlingObject;
	// get current CAFileHandling object;

- (void)setTitle:(NSString *)inTitle;
	// Pass in string to use for outlineView's title.
- (NSString *)title;
	// String being used for outlineView's title.

- (BOOL)itemWasActivated:(id)object;
	// subclasses should override to implement any desired functionality when a user
	// double-clicks on an item in the outline view.  Because of the way CAFileBrowser
	// handles events, clients should NOT override setTarget:, setAction:,
	// setDoubleAction:, or other event handling methods.
	
- (NSString *)fileExtension;
	// subclasses should override to return the name of the preset file extension
	// (without the '.')
	
- (NSString *)nameKeyString;
	// subclasses should override to return the name of the key (usually "name") whose
	// value is the name of the preset.

- (BOOL)shouldAllowItemRenaming:(id)item;
	// subclasses may override to allow presets to be (selectively) renamed.
	// default returns YES for all items

@end
