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
 OF SUCH DAMAGE.


	InstallSpeakableItems.c
	
	Copyright (c) 2001-2005 Apple Computer, Inc. All rights reserved.
*/

#include <Carbon/Carbon.h>

// Prototypes
OSErr NewAliasFile( SInt16 inVRefNum, SInt32 inDirID, ConstStr31Param inFileName, OSType inFileType, OSType inFileCreator, FSSpec* inTargetFileSpec, Boolean inReplaceIfExists, Boolean inMakeInvisible );
void InstallSpeakableItemsFolderContents( const FSRef * inSourceFolderFSRef, const FSRef * inTargetFolderFSRef, OSType inFileType, OSType inFileCreator, Boolean inRsrcForkInDataFork );


/*
    InstallSpeakableItemsForThisApplication
    
    This routines creates an application-specific folder inside the user's Speakable Items directory hierarchy, then
    installs Command and AppleScript data files from the application's bundle into the application-specific folder.
    Installing the items at run time instead of during install time is preferrable because the application may be
    self-contained and not need a separate installer.  It also allows the application to easily accomodate new
    users being created after the initial installation.
    
	This routine only creates and installs the application's items if the folder doesn't already exist and the user
    has turned on Speakable Items at least once, thereby creating the "Speakable Items" folder hierarchy in their 
    "Library/Speech/" folder.  You can optionally force a reinstall of items if the application's items folder
    already exists, for example to reinstall items the user may have thrown away.  This allows you to either, call
    this routine during every launch to create the folder if necessary, or call only when the user indicates that
    they want the spoken command feature enabled
    
	When using Project Builder, the Command and AppleScript data files are added to the project and specified to be
    copied (using a Copy File Build Phase) into separate folders inside the "Resources" folder.  Since AppleScript 
    files have resource forks, the resource fork must be moved to the data fork before adding the file to the project.  
    This routine then recreates the AppleScript resource fork using the data in the data fork.
    
*/

Boolean InstallSpeakableItemsForThisApplication( CFStringRef inCommandFilesResourcePath, CFStringRef inCompliedASDataFilesResourcePath, Boolean inForceReinstallation )
{
    #define kInvisibleFileName	"\pTarget Application Alias\r"
	#define	kInvisibleFileType	'adrp'

	Boolean					successfullyCreated		= false;
    CFURLRef				theSIAppDirCFURLRef		= NULL;
    CFURLRef				thisAppsSIDirCFURLRef	= NULL;
    CFStringRef				thisAppsDisplayName		= NULL;
    FSSpec					thisAppsFSSpec;
    ProcessSerialNumber		thisAppsPSN;
    Str255					thisAppsName;
    FSSpec           		thisAppsSIDirFSSpec;
    UInt32	         		thisAppsSIDirID;
    ProcessInfoRec 			thisAppsProcessInfoRec;
    FSRef					theSIAppDirFSRef;
    FSRef					thisAppsSIDirFSRef;

    GetCurrentProcess( &thisAppsPSN );

    // Grab info about this process
    thisAppsProcessInfoRec.processNumber.highLongOfPSN 	= 0;
    thisAppsProcessInfoRec.processNumber.lowLongOfPSN 	= kNoProcess;
    thisAppsProcessInfoRec.processInfoLength 			= sizeof(ProcessInfoRec);
    thisAppsProcessInfoRec.processName 					= thisAppsName;
    thisAppsProcessInfoRec.processAppSpec 				= &thisAppsFSSpec;
    if (GetProcessInformation( &thisAppsPSN, &thisAppsProcessInfoRec) == noErr) {
        
        thisAppsDisplayName = CFStringCreateWithPascalString( NULL, thisAppsProcessInfoRec.processName, kCFStringEncodingMacRoman );
        if (thisAppsDisplayName) {

            // Get the user's library folder.
			FSRef userLibraryFolderFSRef;
			if (FSFindFolder( kUserDomain, kDomainLibraryFolderType, true, &userLibraryFolderFSRef ) == noErr) {
			
				CFURLRef userLibraryFolderCFURL = CFURLCreateFromFSRef( kCFAllocatorDefault, (const struct FSRef *)&userLibraryFolderFSRef );
				if (userLibraryFolderCFURL) {
				
                    CFStringRef	theDirPath = CFURLCopyPath( userLibraryFolderCFURL );
                    if (theDirPath) {
            
                        // Append the path to this application's Speakable Items folder
                        CFMutableStringRef theDirectoryPathStr = CFStringCreateMutable( NULL, 0 );
                    
                        if (theDirectoryPathStr) {
                            CFStringAppend( theDirectoryPathStr, theDirPath );
                            CFStringAppend( theDirectoryPathStr, CFSTR( "/Speech/Speakable Items/Application Speakable Items/" ) );
                            theSIAppDirCFURLRef = CFURLCreateWithFileSystemPath( NULL, theDirectoryPathStr, kCFURLPOSIXPathStyle, true );
        
                            CFStringAppend( theDirectoryPathStr, thisAppsDisplayName );
                            CFStringAppend( theDirectoryPathStr, CFSTR( "/" ) );
                            thisAppsSIDirCFURLRef = CFURLCreateWithFileSystemPath( NULL, theDirectoryPathStr, kCFURLPOSIXPathStyle, true );
        
                            CFRelease( theDirectoryPathStr );
                        }
        
                        CFRelease( theDirPath );
                    }
					
					CFRelease( userLibraryFolderCFURL );
                }
            }
        }
    }
    
    // If we found the user's "Application Speakable Items" folder, then look for this app's folder 
    if (theSIAppDirCFURLRef && thisAppsSIDirCFURLRef && CFURLGetFSRef( theSIAppDirCFURLRef, &theSIAppDirFSRef )) {
    
        // We'll create a new folder for our application's spekable items if it doesn't already exist.
        // NOTE:  If the "Speakable Items" folder hasn't be created, then the user hasn't run SI yet so there's no need to create our folder yet.
        if (! CFURLGetFSRef( thisAppsSIDirCFURLRef, &thisAppsSIDirFSRef )) {
    
            const CFIndex		maxNameLen = 300;
            UniChar				appNameAsUniChar[maxNameLen];
            UniCharCount      	appNameLength;

            appNameLength = CFStringGetLength( thisAppsDisplayName );
            if (appNameLength > maxNameLen)
                appNameLength = maxNameLen;
                
            CFStringGetCharacters( thisAppsDisplayName, CFRangeMake(0, appNameLength), appNameAsUniChar ); 
        
            // Create the Directory
            if (FSCreateDirectoryUnicode( &theSIAppDirFSRef, appNameLength, appNameAsUniChar, 0, NULL, &thisAppsSIDirFSRef, &thisAppsSIDirFSSpec, &thisAppsSIDirID ) == noErr) {

                FSRef	bundleLocationFSRef;
        
                // Since GetProcessInformation returns the actual executable of bundled applications, we must use
                // GetProcessBundleLocation to get the application bundle FSSpec.
                if (GetProcessBundleLocation( &thisAppsPSN, &bundleLocationFSRef) == noErr)
                    FSGetCatalogInfo( &bundleLocationFSRef, kFSCatInfoNone, NULL, NULL, &thisAppsFSSpec, NULL);
                
                // Create the invisible alias file
                if (NewAliasFile( thisAppsSIDirFSSpec.vRefNum, thisAppsSIDirID, kInvisibleFileName, kInvisibleFileType, thisAppsProcessInfoRec.processSignature, &thisAppsFSSpec, true, true ) == noErr)
                    successfullyCreated = true;
            }
            else if (inForceReinstallation) {
                successfullyCreated = true;
            }
        }
		else {
			// We reinstall our commands at every launch because Speakable Items in later Mac OS X releases automatically creates our application folder.
			// You can optimize this if you look in the folder first before deciding the install the commands.
			successfullyCreated = true;
		}
    }            
    
    
    if (successfullyCreated) {
    
        FSRef 		sourceFolderFSRef;
        CFURLRef	sourceCFURLRef;
        FSCatalogInfo  catalogInfo;

        // Install Command Files
        sourceCFURLRef = CFBundleCopyResourceURL( CFBundleGetMainBundle(), inCommandFilesResourcePath, NULL, NULL );
        if (sourceCFURLRef) {
            if (CFURLGetFSRef( sourceCFURLRef, &sourceFolderFSRef ))
                InstallSpeakableItemsFolderContents( &sourceFolderFSRef, &thisAppsSIDirFSRef, 'sicf', 'siax', false );
            
            CFRelease( sourceCFURLRef );
        }
    
        // Install AppleScript Files
        sourceCFURLRef = CFBundleCopyResourceURL( CFBundleGetMainBundle(), inCompliedASDataFilesResourcePath, NULL, NULL );
        if (sourceCFURLRef) {
            if (CFURLGetFSRef( sourceCFURLRef, &sourceFolderFSRef ))
                InstallSpeakableItemsFolderContents( &sourceFolderFSRef, &thisAppsSIDirFSRef, 'osas', 'ToyS', true );
            
            CFRelease( sourceCFURLRef );
        }
        
        // Touch the mod date of the application's folder so Speakable Item will update itself.
        if (GetUTCDateTime( &(catalogInfo.contentModDate), 0 ) == noErr)
            FSSetCatalogInfo( &thisAppsSIDirFSRef, kFSCatInfoContentMod, &catalogInfo);

    }
    
	return successfullyCreated;
}


/*
    NewAliasFile
    
    A utility routine used by InstallSpeakableItemsForThisApplication to create an alias file.
*/
    
OSErr NewAliasFile( SInt16 inVRefNum, SInt32 inDirID, ConstStr31Param inFileName, OSType inFileType, OSType inFileCreator, FSSpec* inTargetFileSpec, Boolean inReplaceIfExists, Boolean inMakeInvisible )
{
	OSErr			theErr	= noErr;
    SInt16			savedResFile;
    CInfoPBRec		filePB;
	FSSpec			theAliasFile;
    
	theAliasFile.vRefNum	= inVRefNum;
	theAliasFile.parID	= inDirID;
	PLstrcpy( theAliasFile.name, inFileName );		
	
	savedResFile = CurResFile();
	FSpCreateResFile( &theAliasFile, inFileCreator, inFileType, smSystemScript );
	theErr = ResError();
	
	if(! theErr) {
    
		SInt16 theAliasResFileNum = FSpOpenResFile ( &theAliasFile, fsRdWrPerm );
		
		if( theAliasResFileNum != -1 )
		{
			AliasHandle	theAliasHandle;
			if( NewAlias ( NULL, inTargetFileSpec, &theAliasHandle ) == noErr )
			{ 
				AddResource( (Handle)theAliasHandle, rAliasType, 0, inFileName );
				theErr = ResError();
			}
			
			CloseResFile( theAliasResFileNum );
		}
	}
	
	UseResFile( savedResFile );
	
	
	//
	// Set Finder flags on our invisible alias file
	//
    if (! theErr) {
    
        filePB.hFileInfo.ioCompletion 	= NULL;
        filePB.hFileInfo.ioNamePtr 		= theAliasFile.name;
        filePB.hFileInfo.ioVRefNum 		= inVRefNum;
        filePB.hFileInfo.ioFDirIndex 	= 0;
        filePB.hFileInfo.ioDirID 		= inDirID;
        
        if( PBGetCatInfoSync(&filePB) == noErr )
        {
            filePB.hFileInfo.ioFlFndrInfo.fdFlags |= kIsAlias;
            
            if( inMakeInvisible )
                filePB.hFileInfo.ioFlFndrInfo.fdFlags |= kIsInvisible;
                
            filePB.hFileInfo.ioDirID 		= inDirID;
            theErr = PBSetCatInfoSync(&filePB);
        }
    }
	
	return theErr;
}

/*
    InstallSpeakableItemsFolderContents
    
    A utility routine used by InstallSpeakableItemsForThisApplication to copy the contents of a folder.
*/

void InstallSpeakableItemsFolderContents( const FSRef * inSourceFolderFSRef, const FSRef * inTargetFolderFSRef, OSType inFileType, OSType inFileCreator, Boolean inRsrcForkInDataFork )
{

    const	ItemCount	kMaxInteratorCount = 100;

    OSStatus		status = noErr;
    FSIterator      sourceIterator;
    ItemCount		sourceDirObjectCount = 0;    
    FSRef *			sourceFSRefArray = NULL;    
    FSCatalogInfo *	sourceFSCatInfoArray = NULL;
    HFSUniStr255 *	sourceHFSUniNameArray = NULL;
    UInt32			sourceFileIndex = 0;

    sourceFSRefArray = malloc( kMaxInteratorCount * sizeof(FSRef) );
    sourceFSCatInfoArray = malloc( kMaxInteratorCount * sizeof(FSCatalogInfo) );
    sourceHFSUniNameArray = malloc( kMaxInteratorCount * sizeof(HFSUniStr255) );
    
    status = FSOpenIterator( inSourceFolderFSRef, kFSIterateFlat, &sourceIterator );
   
    if (!status && sourceFSRefArray && sourceFSCatInfoArray && sourceHFSUniNameArray) {
            
        do {
    
            // Grab a batch of source files to process from the source directory
            status = FSGetCatalogInfoBulk( sourceIterator, kMaxInteratorCount, &sourceDirObjectCount, NULL, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo | kFSCatInfoDataSizes, sourceFSCatInfoArray, sourceFSRefArray, NULL, sourceHFSUniNameArray);
            if ((status == errFSNoMoreItems || status == noErr) && sourceDirObjectCount) {
                status = noErr;
    
                for (sourceFileIndex = 0; sourceFileIndex < sourceDirObjectCount; sourceFileIndex++ ) {
                                    
                    // Only copy files, not directories
                    if ( ! (sourceFSCatInfoArray[sourceFileIndex].nodeFlags & kFSNodeIsDirectoryMask)) {
            
                        FSRef			targetFileFSRef;
                        FSCatalogInfo	targetFileCat;
                        SInt16			itemToBeCopiedDataForkRefNum = -1;
            
                        // Open data and resource fork of "from" file
                        status = FSOpenFork( &(sourceFSRefArray[sourceFileIndex]), 0, NULL, fsRdPerm, &itemToBeCopiedDataForkRefNum);
                        
                        if (! status) {
                            memcpy( targetFileCat.finderInfo, sourceFSCatInfoArray[sourceFileIndex].finderInfo, sizeof(UInt8) * 16 );
                            
                            ((FileInfo *)targetFileCat.finderInfo)->fileType		= inFileType; 
                            ((FileInfo *)targetFileCat.finderInfo)->fileCreator 	= inFileCreator;

                            status = FSCreateFileUnicode( inTargetFolderFSRef, sourceHFSUniNameArray[sourceFileIndex].length, sourceHFSUniNameArray[sourceFileIndex].unicode,kFSCatInfoFinderInfo, &targetFileCat, &targetFileFSRef, NULL);
                        }
                                
                        if (! status && itemToBeCopiedDataForkRefNum != -1) {
                            
                            SInt32	theDataCount = sourceFSCatInfoArray[sourceFileIndex].dataLogicalSize;
                            void *	theDataBuffer = malloc( theDataCount );
                            if (theDataBuffer) {
                            
                                SInt16			newForkRefNum = -1;
                                HFSUniStr255	newForkName;
            
                                // Create the fork specified my the given flag
                                if (inRsrcForkInDataFork)
                                    status = FSGetResourceForkName( &newForkName );
                                else
                                    status = FSGetDataForkName( &newForkName );
                        
                                if (! status)
                                    status = FSCreateFork( &targetFileFSRef, newForkName.length, newForkName.unicode);
                                
                                if (! status)
                                    status = FSOpenFork( &targetFileFSRef, newForkName.length, newForkName.unicode, fsRdWrPerm, &newForkRefNum);
                                    
                                if (! status)
                                    status = FSReadFork( itemToBeCopiedDataForkRefNum, fsFromStart, 0, theDataCount, theDataBuffer, (ByteCount *) &theDataCount);
                                    
                                if (! status)
                                    status = FSWriteFork( newForkRefNum, fsFromStart, 0, theDataCount, theDataBuffer, (ByteCount *) &theDataCount);
                    
                                if (newForkRefNum != -1)
                                    FSCloseFork( newForkRefNum );
                            
                                free( theDataBuffer );
                            }
                        }
                        
                        // Close up our source files.
                        if (itemToBeCopiedDataForkRefNum != -1)
                            FSCloseFork( itemToBeCopiedDataForkRefNum );
                    }
                
                }
                
            }
            
        } while (! status);
        
        free( sourceFSRefArray );
        free( sourceFSCatInfoArray );
        free( sourceHFSUniNameArray );
    }
}
