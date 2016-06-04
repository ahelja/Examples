/*
	File:		AGFile.h

	Contains:	Public interface to Apple Guide Database Files.
				Does not use or require the Apple Guide extension.

	Version:	Mac OS X

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

	Copyright © 1994-2001 Apple Computer, Inc., All Rights Reserved
*/

#ifndef __AGFILE__
#define __AGFILE__

#if defined(USE_UMBRELLA_HEADERS) && USE_UMBRELLA_HEADERS
        #include <Carbon.h>
#else
	#include <Files.h>
	#include <MacTypes.h>
#endif

			// typedef's

typedef FSSpec AGFileFSSpecType;
typedef short AGFileSelectorCountType;
typedef short AGFileSelectorIndexType;
typedef OSType AGFileSelectorType;
typedef long AGFileSelectorValueType;
typedef short AGFileDBType;
typedef ConstStr63Param AGFileDBMenuNamePtr;
typedef short AGFileDBScriptType;
typedef short AGFileDBRegionType;
typedef short AGFileMajorRevType;
typedef short AGFileMinorRevType;
typedef short AGFileCountType;

			// Database types (for AGFileDBType parameter).

enum
{
	kAGFileDBTypeAny =			0,
	kAGFileDBTypeHelp =			1,
	kAGFileDBTypeTutorial =		2,
	kAGFileDBTypeShortcuts =	3,
	kAGFileDBTypeAbout =		4,
	kAGFileDBTypeOther =		8
};

			// Functions.

		// Get the database menu item name.
pascal OSErr
AGFileGetDBMenuName(AGFileFSSpecType *fileSpec,
					AGFileDBMenuNamePtr menuItemNameString);

		// Get the database type.
pascal OSErr
AGFileGetDBType(AGFileFSSpecType *fileSpec,
				AGFileDBType *databaseType);

		// Get the version of the software
		// that created this database.
pascal OSErr
AGFileGetDBVersion(AGFileFSSpecType *fileSpec,
					AGFileMajorRevType *majorRev,
					AGFileMinorRevType *minorRev);

		// Get the database script and region information.
pascal OSErr
AGFileGetDBCountry(AGFileFSSpecType *fileSpec,
					AGFileDBScriptType *script,
					AGFileDBRegionType *region);

		// Return the number of selectors in database.
pascal AGFileSelectorCountType
AGFileGetSelectorCount(AGFileFSSpecType *fileSpec);

		// Get the i-th database selector (1 to AGFileSelectorCountType)
		// and its value.
pascal OSErr
AGFileGetSelector(AGFileFSSpecType *fileSpec,
					AGFileSelectorIndexType selectorNumber,
					AGFileSelectorType *selector,
					AGFileSelectorValueType *value);

		// Return true if database is mixin.
pascal Boolean
AGFileIsMixin(AGFileFSSpecType *fileSpec);

		// Return the number of database files
		// of the specified databaseType and main/mixin.
		// Any file creator is acceptible,
		// but type must be kAGFileMain or kAGFileMixin.
pascal AGFileCountType
AGFileGetDBCount(short vRefNum,
					long dirID,
					AGFileDBType databaseType,
					Boolean wantMixin);

		// Get the FSSpec for the dbIndex-th database
		// of the specified databaseType and main/mixin.
		// Any file creator is acceptible,
		// but type must be kAGFileMain or kAGFileMixin.
pascal OSErr
AGFileGetIndDB(short vRefNum,
				long dirID,
				AGFileDBType databaseType,
				Boolean wantMixin,
				short dbIndex,
				FSSpec *fileSpec);

		// This selector must match with the application
		// creator in order for this file to appear in the 
		// application's Help menu. Ignored for mixin files
		// because they never appear in the Help menu anyway.
		// If empty (zeros), will appear in the Help menu
		// of any host application.
pascal OSErr
AGFileGetHelpMenuAppCreator(AGFileFSSpecType *fileSpec,
								OSType *helpMenuAppCreator);

		// This selector must match in the main and mixin
		// files in order for the mixin to mix-in with the main.
		// Empty (zeros) selectors are valid matches.
		// A '****' selector will mix-in with any main.
pascal OSErr
AGFileGetMixinMatchSelector(AGFileFSSpecType *fileSpec,
								OSType *mixinMatchSelector);

		// This is the text of the balloon for the
		// Help menu item for this database.
pascal OSErr
AGFileGetHelpMenuBalloonText(AGFileFSSpecType *fileSpec,
								Str255 helpMenuBalloonString);

#endif /* __AGFILE__ */

