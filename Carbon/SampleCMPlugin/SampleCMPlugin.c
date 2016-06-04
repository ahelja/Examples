/*
	File:		SampleCMPlugin.c

	Contains:	Sample contextual menu plugin.

	Version:	Mac OS X

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Appleﾕs
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

	Copyright ｩ 2002-2003 Apple Computer, Inc., All Rights Reserved
*/

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CFPlugInCOM.h>

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------

#define kSampleCMPluginFactoryID	( CFUUIDGetConstantUUIDWithBytes( NULL,		\
	0xC5, 0x2C, 0x25, 0x66, 0x3D, 0xC1, 0x11, 0xD5, 		\
	0xBB, 0xA3, 0x00, 0x30, 0x65, 0xB3, 0x00, 0xBC ) )
	// "C52C2566-3DC1-11D5-BBA3-003065B300BC"

#define scm_require(condition,location)		\
			if ( !(condition) )	\
				goto location;
#define scm_require_noerr(value,location)	\
			scm_require((value)==noErr,location)

// -----------------------------------------------------------------------------
//	typedefs
// -----------------------------------------------------------------------------

// The layout for an instance of SampleCMPluginType.
typedef struct SampleCMPluginType
{
	ContextualMenuInterfaceStruct	*cmInterface;
	CFUUIDRef						factoryID;
	UInt32							refCount;
 } SampleCMPluginType;

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//	Forward declaration for the IUnknown implementation.
//
static void DeallocSampleCMPluginType(
		SampleCMPluginType	*thisInstance );
static OSStatus AddCommandToAEDescList(
		ConstStr255Param	inCommandString,
		TextEncoding		inEncoding,
		DescType			inDescType,
		SInt32				inCommandID,
		MenuItemAttributes	inAttributes,
		UInt32				inModifiers,
		AEDescList*			ioCommandList);
static OSStatus CreateSampleSubmenu(
		AEDescList*			ioCommandList);
static OSStatus CreateSampleDynamicItems(
		AEDescList*			ioCommandList);

// -----------------------------------------------------------------------------
//	SampleCMPluginQueryInterface
// -----------------------------------------------------------------------------
//	Implementation of the IUnknown QueryInterface function.
//
static HRESULT SampleCMPluginQueryInterface(
		void*		thisInstance,
		REFIID		iid,
		LPVOID*		ppv )
{
	// Create a CoreFoundation UUIDRef for the requested interface.
	CFUUIDRef	interfaceID = CFUUIDCreateFromUUIDBytes( NULL, iid );

	// Test the requested ID against the valid interfaces.
	if ( CFEqual( interfaceID, kContextualMenuInterfaceID ) )
	{
		// If the TestInterface was requested, bump the ref count,
		// set the ppv parameter equal to the instance, and
		// return good status.
		( ( SampleCMPluginType* ) thisInstance )->cmInterface->AddRef(
				thisInstance );
		*ppv = thisInstance;
		CFRelease( interfaceID );
		return S_OK;
	}
	else if ( CFEqual( interfaceID, IUnknownUUID ) )
	{
		// If the IUnknown interface was requested, same as above.
		( ( SampleCMPluginType* ) thisInstance )->cmInterface->AddRef(
			thisInstance );
		*ppv = thisInstance;
		CFRelease( interfaceID );
		return S_OK;
	}
	else
	{
		// Requested interface unknown, bail with error.
		*ppv = NULL;
		CFRelease( interfaceID );
		return E_NOINTERFACE;
	}
}

// -----------------------------------------------------------------------------
//	SampleCMPluginAddRef
// -----------------------------------------------------------------------------
//	Implementation of reference counting for this type. Whenever an interface
//	is requested, bump the refCount for the instance. NOTE: returning the
//	refcount is a convention but is not required so don't rely on it.
//
static ULONG SampleCMPluginAddRef( void *thisInstance )
{
	( ( SampleCMPluginType* ) thisInstance )->refCount += 1;
	return ( ( SampleCMPluginType* ) thisInstance)->refCount;
}

// -----------------------------------------------------------------------------
// SampleCMPluginRelease
// -----------------------------------------------------------------------------
//	When an interface is released, decrement the refCount.
//	If the refCount goes to zero, deallocate the instance.
//
static ULONG SampleCMPluginRelease( void *thisInstance )
{
	( ( SampleCMPluginType* ) thisInstance )->refCount -= 1;
	if ( ( ( SampleCMPluginType* ) thisInstance )->refCount == 0)
	{
		DeallocSampleCMPluginType(
				( SampleCMPluginType* ) thisInstance );
		return 0;
	}
	else
	{
		return ( ( SampleCMPluginType*) thisInstance )->refCount;
	}
}

// -----------------------------------------------------------------------------
//	SampleCMPluginExamineContext
// -----------------------------------------------------------------------------
//	The implementation of the ExamineContext test interface function.
//
static OSStatus SampleCMPluginExamineContext(
	void*				thisInstance,
	const AEDesc*		inContext,
	AEDescList*			outCommandPairs )
{
	// Sequence the command ids
	SInt32	theCommandID = 1;
	SInt32	result;

	printf( "SampleCMPlugin->SampleCMPluginExamineContext(): instance 0x%x, inContext 0x%x, outCommandPairs 0x%x\n",
			( unsigned ) thisInstance,
			( const unsigned ) inContext,
			( unsigned ) outCommandPairs );

	// Verify that we've got an up-to-date CMM
	verify_noerr( Gestalt( gestaltContextualMenuAttr, &result ) );
	if ( ( result & ( 1 << gestaltContextualMenuHasAttributeAndModifierKeys ) ) != 0 )
		printf( "SampleCMPlugin: CMM supports Attributes and Modifiers keys\n" );
	else
		printf( "SampleCMPlugin: CMM DOES NOT support Attributes and Modifiers keys\n" );
	if ( ( result & ( 1 << gestaltContextualMenuHasUnicodeSupport ) ) != 0 )
		printf( "SampleCMPlugin: CMM supports typeUnicodeText and typeCFStringRef\n" );
	else
		printf( "SampleCMPlugin: CMM DOES NOT support typeUnicodeText and typeCFStringRef\n" );
	
	// this is a quick sample that looks for text in the context descriptor
	
	// make sure the descriptor isn't null
	if ( inContext != NULL )
	{
		AEDesc theTextDesc = { typeNull, NULL };
		Str15 theDescriptorType;
		
		AddCommandToAEDescList( "\pInside SampleCMPlugin!", kTextEncodingMacRoman, typeChar,
				theCommandID++, 0, 0, outCommandPairs );

		AddCommandToAEDescList( "\pキャンセル", kTextEncodingMacJapanese, typeIntlText, theCommandID++,
				0, 0, outCommandPairs );
		AddCommandToAEDescList( "\p-A separator item", kTextEncodingMacRoman, typeChar,
				0, 0, 0, outCommandPairs );
		AddCommandToAEDescList( "\p-A non-separator item", kTextEncodingMacRoman, typeChar, 
				theCommandID++, kMenuItemAttrIgnoreMeta, 0, outCommandPairs );
		AddCommandToAEDescList( NULL, 0, typeNull, 0, kMenuItemAttrSeparator, 0, outCommandPairs );
		AddCommandToAEDescList( NULL, 0, typeNull, 0, 0, 0, outCommandPairs );
		AddCommandToAEDescList( "\pキャンセル", kTextEncodingMacJapanese, typeUnicodeText, theCommandID++,
				0, 0, outCommandPairs );
		
		// tell the raw type of the descriptor
		theDescriptorType[0] = 4;
		*( DescType* )( &theDescriptorType[1] ) = inContext->descriptorType;
		AddCommandToAEDescList( theDescriptorType, kTextEncodingMacRoman, typeChar,
				theCommandID++, 0, 0, outCommandPairs );
		
		// try to get text out of the context descriptor; make sure to
		// coerce it, cuz the app may have passed an object specifier or
		// styled text, etc.
		if ( AECoerceDesc( inContext, typeChar, &theTextDesc ) == noErr )
		{
			printf( "SampleCMPluginExamineContext: Able to coerce to text object.\n" );
			// add a text only command to our command list
			AddCommandToAEDescList( "\pWe got text!", kTextEncodingMacRoman, typeChar, 
					theCommandID++, 0, 0, outCommandPairs );
		}
		else
		{
			printf( "SampleCMPluginExamineContext: Unable to coerce to text object.\n" );
			// add a text only command to our command list
			AddCommandToAEDescList( "\pCan't Coerce.  8(", kTextEncodingMacRoman, typeChar,
					theCommandID++, 0, 0, outCommandPairs );
		}
		AEDisposeDesc( &theTextDesc );
		
		// Just for kicks, let's create a submenu for our plugin
		CreateSampleSubmenu( outCommandPairs );
		
		// Let's also demonstrate dynamic items in a contextual menu
		CreateSampleDynamicItems( outCommandPairs );
	}
	else
	{
		printf( "SampleCMPluginExamineContext: Hey! What's up with the NULL descriptor?\n" );
		// we have a null descriptor
		AddCommandToAEDescList( "\pNULL Descriptor", kTextEncodingMacRoman, typeChar,
				theCommandID++, 0, 0, outCommandPairs );
	}

	return noErr;
}

// -----------------------------------------------------------------------------
//	HandleSelection
// -----------------------------------------------------------------------------
//	The implementation of the HandleSelection test interface function.
//
static OSStatus SampleCMPluginHandleSelection(
	void*				thisInstance,
	AEDesc*				inContext,
	SInt32				inCommandID )
{
	printf( "SampleCMPlugin->SampleCMPluginHandleSelection(): instance 0x%x, inContext 0x%x, inCommandID 0x%x\n",
			( unsigned ) thisInstance,
			( const unsigned ) inContext,
			( unsigned ) inCommandID );
	
	// Selecting it does nothing
	return noErr;
}

// -----------------------------------------------------------------------------
//	PostMenuCleanup
// -----------------------------------------------------------------------------
//	The implementation of the PostMenuCleanup test interface function.
//
static void SampleCMPluginPostMenuCleanup( void *thisInstance )
{
	printf( "SampleCMPlugin->SampleCMPluginFinishedExamining(): instance 0x%x\n",
			( unsigned ) thisInstance );

	// No need to clean up.  We are a tidy folk.
}

// -----------------------------------------------------------------------------
//	testInterfaceFtbl	definition
// -----------------------------------------------------------------------------
//	The TestInterface function table.
//
static ContextualMenuInterfaceStruct testInterfaceFtbl =
			{ 
				// Required padding for COM
				NULL,
		
				// These three are the required COM functions
				SampleCMPluginQueryInterface,
				SampleCMPluginAddRef, 
				SampleCMPluginRelease, 
		
				// Interface implementation
				SampleCMPluginExamineContext,
				SampleCMPluginHandleSelection,
				SampleCMPluginPostMenuCleanup
			}; 

// -----------------------------------------------------------------------------
//	AllocSampleCMPluginType
// -----------------------------------------------------------------------------
//	Utility function that allocates a new instance.
//
static SampleCMPluginType* AllocSampleCMPluginType(
		CFUUIDRef		inFactoryID )
{
	
	// Allocate memory for the new instance.
	SampleCMPluginType *theNewInstance;
	theNewInstance = ( SampleCMPluginType* ) malloc(
			sizeof( SampleCMPluginType ) );

	// Point to the function table
	theNewInstance->cmInterface = &testInterfaceFtbl;

	// Retain and keep an open instance refcount<
	// for each factory.
	theNewInstance->factoryID = CFRetain( inFactoryID );
	CFPlugInAddInstanceForFactory( inFactoryID );

	// This function returns the IUnknown interface
	// so set the refCount to one.
	theNewInstance->refCount = 1;
	return theNewInstance;
}

// -----------------------------------------------------------------------------
//	DeallocSampleCMPluginType
// -----------------------------------------------------------------------------
//	Utility function that deallocates the instance when
//	the refCount goes to zero.
//
static void DeallocSampleCMPluginType( SampleCMPluginType* thisInstance )
{
	CFUUIDRef	theFactoryID = thisInstance->factoryID;
	free( thisInstance );
	if ( theFactoryID )
	{
		CFPlugInRemoveInstanceForFactory( theFactoryID );
		CFRelease( theFactoryID );
	}
}

// -----------------------------------------------------------------------------
//	SampleCMPluginFactory
// -----------------------------------------------------------------------------
//	Implementation of the factory function for this type.
//
void* SampleCMPluginFactory(
		CFAllocatorRef		allocator,
		CFUUIDRef			typeID )
{
	// If correct type is being requested, allocate an
	// instance of TestType and return the IUnknown interface.
	if ( CFEqual( typeID, kContextualMenuTypeID ) )
	{
		SampleCMPluginType *result;
		result = AllocSampleCMPluginType( kSampleCMPluginFactoryID );
		return result;
	}
	else
	{
		// If the requested type is incorrect, return NULL.
		return NULL;
	}
}

// -----------------------------------------------------------------------------
//	AddCommandToAEDescList
// -----------------------------------------------------------------------------
static OSStatus AddCommandToAEDescList(
	ConstStr255Param		inCommandString,
	TextEncoding			inEncoding,
	DescType				inDescType,
	SInt32					inCommandID,
	MenuItemAttributes		inAttributes,
	UInt32					inModifiers,
	AEDescList*				ioCommandList)
{
	OSStatus theError = noErr;
	
	AERecord theCommandRecord = { typeNull, NULL };
	
	printf( "AddCommandToAEDescList: Trying to add an item.\n" );

	// create an apple event record for our command
	theError = AECreateList( NULL, kAEDescListFactorNone, true, &theCommandRecord );
	require_noerr( theError, AddCommandToAEDescList_fail );
	
	// stick the command text into the AERecord
	if ( inCommandString != NULL )
	{
		if ( inDescType == typeChar )
		{
			theError = AEPutKeyPtr( &theCommandRecord, keyAEName, typeChar,
				&inCommandString[1], StrLength( inCommandString ) );
			require_noerr( theError, AddCommandToAEDescList_fail );
		}
		else if ( inDescType == typeStyledText )
		{
			AERecord	textRecord;
			WritingCode	writingCode;
			AEDesc		textDesc;
			
			theError = AECreateList( NULL, kAEDescListFactorNone, true, &textRecord );
			require_noerr( theError, AddCommandToAEDescList_fail );
			
			theError = AEPutKeyPtr( &textRecord, keyAEText, typeChar,
				&inCommandString[1], StrLength( inCommandString ) );
			require_noerr( theError, AddCommandToAEDescList_fail );
			
			RevertTextEncodingToScriptInfo( inEncoding, &writingCode.theScriptCode,
				&writingCode.theLangCode, NULL );
			theError = AEPutKeyPtr( &textRecord, keyAEScriptTag, typeIntlWritingCode,
				&writingCode, sizeof( writingCode ) );
			require_noerr( theError, AddCommandToAEDescList_fail );

			theError = AECoerceDesc( &textRecord, typeStyledText, &textDesc );
			require_noerr( theError, AddCommandToAEDescList_fail );
			
			theError = AEPutKeyDesc( &theCommandRecord, keyAEName, &textDesc );
			require_noerr( theError, AddCommandToAEDescList_fail );
			
			AEDisposeDesc( &textRecord );
		}
		else if ( inDescType == typeIntlText )
		{
			IntlText*	intlText;
			ByteCount	size = sizeof( IntlText ) + StrLength( inCommandString ) - 1;
			
			// create an IntlText structure with the text and script
			intlText = (IntlText*) malloc( size );
			RevertTextEncodingToScriptInfo( inEncoding, &intlText->theScriptCode,
				&intlText->theLangCode, NULL );
			BlockMoveData( &inCommandString[1], &intlText->theText, StrLength( inCommandString ) );
			
			theError = AEPutKeyPtr( &theCommandRecord, keyAEName, typeIntlText, intlText, size );
			free( (char*) intlText );
			require_noerr( theError, AddCommandToAEDescList_fail );
		}
		else if ( inDescType == typeUnicodeText )
		{
			CFStringRef str = CFStringCreateWithPascalString( NULL, inCommandString, inEncoding );
			if ( str != NULL )
			{
				Boolean doFree = false;
				CFIndex sizeInChars = CFStringGetLength( str );
				CFIndex sizeInBytes = sizeInChars * sizeof( UniChar );
				const UniChar* unicode = CFStringGetCharactersPtr( str );
				if ( unicode == NULL )
				{
					doFree = true;
					unicode = (UniChar*) malloc( sizeInBytes );
					CFStringGetCharacters( str, CFRangeMake( 0, sizeInChars ), (UniChar*) unicode );
				}
				
				theError = AEPutKeyPtr( &theCommandRecord, keyAEName, typeUnicodeText, unicode, sizeInBytes );
					
				CFRelease( str );
				if ( doFree )
					free( (char*) unicode );
				
				require_noerr( theError, AddCommandToAEDescList_fail );
			}
		}
		else if ( inDescType == typeCFStringRef )
		{
			CFStringRef str = CFStringCreateWithPascalString( NULL, inCommandString, inEncoding );
			if ( str != NULL )
			{
				theError = AEPutKeyPtr( &theCommandRecord, keyAEName, typeCFStringRef, &str, sizeof( str ) );
				require_noerr( theError, AddCommandToAEDescList_fail );
				
				// do not release the string; the Contextual Menu Manager will release it for us
			}
		}
	}
		
	// stick the command ID into the AERecord
	if ( inCommandID != 0 )
	{
		theError = AEPutKeyPtr( &theCommandRecord, keyContextualMenuCommandID,
				typeLongInteger, &inCommandID, sizeof( inCommandID ) );
		require_noerr( theError, AddCommandToAEDescList_fail );
	}
	
	// stick the attributes into the AERecord
	if ( inAttributes != 0 )
	{
		theError = AEPutKeyPtr( &theCommandRecord, keyContextualMenuAttributes,
				typeLongInteger, &inAttributes, sizeof( inAttributes ) );
		require_noerr( theError, AddCommandToAEDescList_fail );
	}
	
	// stick the modifiers into the AERecord
	if ( inModifiers != 0 )
	{
		theError = AEPutKeyPtr( &theCommandRecord, keyContextualMenuModifiers,
				typeLongInteger, &inModifiers, sizeof( inModifiers ) );
		require_noerr( theError, AddCommandToAEDescList_fail );
	}
	
	// stick this record into the list of commands that we are
	// passing back to the CMM
	theError = AEPutDesc(
			ioCommandList, 			// the list we're putting our command into
			0, 						// stick this command onto the end of our list
			&theCommandRecord );	// the command I'm putting into the list
	
AddCommandToAEDescList_fail:
	// clean up after ourself; dispose of the AERecord
	AEDisposeDesc( &theCommandRecord );

    return theError;
    
} // AddCommandToAEDescList

// -----------------------------------------------------------------------------
//	CreateSampleSubmenu
// -----------------------------------------------------------------------------
static OSStatus CreateSampleSubmenu(
	AEDescList*		ioCommandList)
{
	OSStatus	theError = noErr;
	
	AEDescList	theSubmenuCommands = { typeNull, NULL };
	AERecord	theSupercommand = { typeNull, NULL };
	Str255		theSupercommandText = "\pSubmenu Here";
	
	// the first thing we should do is create an AEDescList of
	// subcommands

	// set up the AEDescList
	theError = AECreateList( NULL, 0, false, &theSubmenuCommands );
	require_noerr( theError, CreateSampleSubmenu_Complete_fail );

	// stick some commands in this subcommand list
	theError = AddCommandToAEDescList( "\pSubcommand 1", kTextEncodingMacRoman, typeChar,
			1001, 0, 0, &theSubmenuCommands );
	require_noerr( theError, CreateSampleSubmenu_CreateDesc_fail );
	
	// another
	theError = AddCommandToAEDescList( "\pAnother Subcommand", kTextEncodingMacRoman, typeChar,
			1002, 0, 0, &theSubmenuCommands );
	require_noerr( theError, CreateSampleSubmenu_fail );
	
	// yet another
	theError = AddCommandToAEDescList( "\pLast One", kTextEncodingMacRoman, typeChar, 
			1003, 0, 0, &theSubmenuCommands);
	require_noerr( theError, CreateSampleSubmenu_fail );
		
	// now, we need to create the supercommand which will "own" the
	// subcommands.  The supercommand lives in the root command list.
	// this looks very much like the AddCommandToAEDescList function,
	// except that instead of putting a command ID in the record,
	// we put in the subcommand list.

	// create an apple event record for our supercommand
	theError = AECreateList( NULL, 0, true, &theSupercommand );
	require_noerr( theError, CreateSampleSubmenu_fail );
	
	// stick the command text into the aerecord
	theError = AEPutKeyPtr(&theSupercommand, keyAEName, typeChar,
		&theSupercommandText[1], StrLength( theSupercommandText ) );
	require_noerr( theError, CreateSampleSubmenu_fail );
	
	// stick the subcommands into into the AERecord
	theError = AEPutKeyDesc(&theSupercommand, keyContextualMenuSubmenu,
		&theSubmenuCommands);
	require_noerr( theError, CreateSampleSubmenu_fail );
	
	// stick the supercommand into the list of commands that we are
	// passing back to the CMM
	theError = AEPutDesc(
		ioCommandList,		// the list we're putting our command into
		0,					// stick this command onto the end of our list
		&theSupercommand);	// the command I'm putting into the list
	
	// clean up after ourself
CreateSampleSubmenu_fail:
	AEDisposeDesc(&theSubmenuCommands);

CreateSampleSubmenu_CreateDesc_fail:
	AEDisposeDesc(&theSupercommand);

CreateSampleSubmenu_Complete_fail:
    return theError;
    
} // CreateSampleSubmenu

// -----------------------------------------------------------------------------
//	CreateSampleDynamicItems
// -----------------------------------------------------------------------------
static OSStatus CreateSampleDynamicItems(
		AEDescList*			ioCommandList)
{
	OSStatus	theError = noErr;
	
	// add a command
	theError = AddCommandToAEDescList( "\pClose", 2001, kTextEncodingMacRoman, typeChar,
			kMenuItemAttrDynamic, 0, ioCommandList );
	require_noerr( theError, CreateSampleDynamicItems_fail );
	
	// another
	theError = AddCommandToAEDescList( "\pClose All", kTextEncodingMacRoman, typeChar,
			2002, kMenuItemAttrDynamic, kMenuOptionModifier, ioCommandList );
	require_noerr( theError, CreateSampleDynamicItems_fail );
	
	// yet another
	theError = AddCommandToAEDescList( "\pClose All Without Saving", kTextEncodingMacRoman, typeChar,
			2003, kMenuItemAttrDynamic, kMenuOptionModifier | kMenuShiftModifier, ioCommandList );
	require_noerr( theError, CreateSampleDynamicItems_fail );
	
CreateSampleDynamicItems_fail:
	return theError;
	
} // CreateSampleDynamicItems
