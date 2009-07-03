/*
	File:		NewDocumentPlugIn.c

	Contains:	Contextual menu plugin for creating new documents.

	Version:	Mac OS X 10.4 to 10.5

	Author:		KemenAran, 2009
 
	Origin:		SampleCMPlugin example from Apple
	
	Licence : MIT Licence
	
	Copyright (c) 2009 Kemenaran

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
*/

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CFPlugInCOM.h>

#include "NewDocumentPlugIn.h"


// -----------------------------------------------------------------------------
//	Global variables
// -----------------------------------------------------------------------------

// Cached reference to self bundle - use GetSelfBundle() to retrieve it.
static CFBundleRef gSelfBundle;

// Cached reference to the global scripting component - use GetScriptingComponent to
// retrieve it.
static ComponentInstance gScriptingComponent;


// -----------------------------------------------------------------------------
//	Implementation of the IUnknown interface
// -----------------------------------------------------------------------------

/*
 * NewDocumentPlugInQueryInterface
 *
 * Implementation of the IUnknown QueryInterface function.
 */
static HRESULT NewDocumentPlugInQueryInterface(void* thisInstance, REFIID iid,	LPVOID*	ppv)
{
	// Create a CoreFoundation UUIDRef for the requested interface.
	CFUUIDRef	interfaceID = CFUUIDCreateFromUUIDBytes( NULL, iid );

	// Test the requested ID against the valid interfaces.
	if ( CFEqual( interfaceID, kContextualMenuInterfaceID ) )
	{
		// If the ContextualMenuInterface was requested, bump the ref count,
		// set the ppv parameter equal to the instance, and
		// return good status.
		( ( NewDocumentPlugInType* ) thisInstance )->cmInterface->AddRef(
				thisInstance );
		*ppv = thisInstance;
		CFRelease( interfaceID );
		return S_OK;
	}
	else if ( CFEqual( interfaceID, IUnknownUUID ) )
	{
		// If the IUnknown interface was requested, same as above.
		( ( NewDocumentPlugInType* ) thisInstance )->cmInterface->AddRef(
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

/*
 * NewDocumentPlugInAddRef
 *
 * Implementation of reference counting for this type. Whenever an interface
 * is requested, bump the refCount for the instance. NOTE: returning the
 * refcount is a convention but is not required so don't rely on it.
 */
static ULONG NewDocumentPlugInAddRef(void *thisInstance)
{
	((NewDocumentPlugInType*)thisInstance)->refCount += 1;
	return ((NewDocumentPlugInType* )thisInstance)->refCount;
}

/*
 * NewDocumentPlugInRelease
 * 
 * When an interface is released, decrement the refCount.
 * If the refCount goes to zero, deallocate the instance.
 */
static ULONG NewDocumentPlugInRelease(void *thisInstance)
{
	NewDocumentPlugInType* typedInstance = (NewDocumentPlugInType*)thisInstance;
	
	typedInstance->refCount -= 1;
	if(typedInstance->refCount == 0) {
		DeallocNewDocumentPlugInType(typedInstance);
		return 0;
	}
	else {
		return typedInstance->refCount;
	}
}


// -----------------------------------------------------------------------------
//  Generic plugin handling functions
// -----------------------------------------------------------------------------

/*
 * AllocNewDocumentPlugInType
 * 
 * Utility function that allocates a new instance.
 */
static NewDocumentPlugInType* AllocNewDocumentPlugInType(CFUUIDRef inFactoryID)
{
	// Allocate memory for the new instance.
	NewDocumentPlugInType *theNewInstance;
	theNewInstance = (NewDocumentPlugInType*) malloc(sizeof(NewDocumentPlugInType));

	// Point to the function table
	theNewInstance->cmInterface = &newDocumentInterfaceFtbl;

	// Retain and keep an open instance refcount<
	// for each factory.
	theNewInstance->factoryID = CFRetain(inFactoryID);
	CFPlugInAddInstanceForFactory(inFactoryID);

	// This function returns the IUnknown interface
	// so set the refCount to one.
	theNewInstance->refCount = 1;
	return theNewInstance;
}

/*
 * DeallocNewDocumentPlugInType
 *
 * Utility function that deallocates the instance when
 * the refCount goes to zero.
 */
static void DeallocNewDocumentPlugInType(NewDocumentPlugInType* thisInstance)
{
	CFUUIDRef theFactoryID = thisInstance->factoryID;
	
	free(thisInstance);
	if (theFactoryID) {
		CFPlugInRemoveInstanceForFactory(theFactoryID);
		// Release the global caches
		CFRelease(gSelfBundle);
		// Release the factory
		CFRelease(theFactoryID);
	}
}

/*
 * NewDocumentPlugInFactory
 *
 * Implementation of the factory function for this type.
 */
void* NewDocumentPlugInFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
	// If correct type is being requested, allocate an
	// instance of NewDocumentPlugInType and return the IUnknown interface.
	if (CFEqual(typeID, kContextualMenuTypeID)) {
		NewDocumentPlugInType *result;
		result = AllocNewDocumentPlugInType(kNewDocumentPlugInFactoryID);
		return result;
	}
	else {
		// If the requested type is incorrect, return NULL.
		return NULL;
	}
}


// -----------------------------------------------------------------------------
//  Context Menu Manager entry points 
// -----------------------------------------------------------------------------

/*
 * NewDocumentPlugInExamineContext
 *
 * This function is called by the Context Menu Manager when a menu
 * is about to be displayed. If the context looks suitable, we add
 * our "New document" menu.
 */
static OSStatus NewDocumentPlugInExamineContext(void* thisInstance, const AEDesc* inContext, AEDescList* outCommandPairs)
{
	CFURLRef selectionURL = CopyFileURLFromAEDescList(inContext);
	
	
	if (selectionURL != NULL) {
		 
		// Convert the fileURL to a FSRef
		FSRef file;
		CFURLGetFSRef(selectionURL, &file);
		
		// We are in a directory : let's add our submenu
		if (FSIsDir(&file)) {
			AddNewDocumentMenu(outCommandPairs);
		}
		
		CFRelease(selectionURL);
	}
	
	return noErr;
}

/*
 * NewDocumentPlugInHandleSelection
 *
 * This function is called by the Context Menu Manager when one
 * of our custom menu items has been selected. We then create
 * a new document of the requested type.
 */
static OSStatus NewDocumentPlugInHandleSelection(void* thisInstance, AEDesc* inContext, SInt32 inCommandID)
{
	OSStatus err;
	CFArrayRef templates;
	CFURLRef destURL, templateURL;
	CFStringRef templateName, newDocumentPath;
	CFMutableStringRef newDocumentName;
	CFBundleRef theBundle;
	FSRef selectionPathFS, templateFilenameFS;
	
	// Retrieve the templates list and the destination directory
	templates = CopyTemplatesFilenames();
	destURL = CopyFileURLFromAEDescList(inContext);
	
	if (destURL != NULL && templates != NULL) {
		
		// Retrieve the URL of the selected template
		templateName = CFArrayGetValueAtIndex(templates, inCommandID);
		theBundle = GetPlugInBundleRef(CFSTR(kNewDocumentPlugInBundle));
		templateURL = CFBundleCopyResourceURL(theBundle, templateName, NULL, CFSTR(kNewDocumentPlugInTemplatesSubdir));
		
		// Define the name of the new document
		newDocumentName = CopyLocalizedTemplateName(templateName, false);
		newDocumentPath = CFURLCopyFileSystemPath(destURL, kCFURLPOSIXPathStyle);
		IncrementDocumentName(newDocumentPath, newDocumentName);
		
		// Convert URLs to FSRefs
		CFURLGetFSRef(templateURL, &templateFilenameFS);
		CFURLGetFSRef(destURL, &selectionPathFS);
		
		// schedule an async copy
		FSFileOperationRef fo = FSFileOperationCreate(NULL);
		FSFileOperationScheduleWithRunLoop(fo, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
		err = FSCopyObjectAsync(fo,
								&templateFilenameFS,
								&selectionPathFS,
								newDocumentName,
								kFSFileOperationDefaultOptions,
								TemplateCopyCallback,
								1,
								NULL
								);
		if (err != noErr)
			printf("NewDocumentPlugIn : File copy error (%d)\n", err);
		
		CFRelease(newDocumentName);
		CFRelease(newDocumentPath);
		CFRelease(fo);
		CFRelease(templateURL);
		CFRelease(destURL);
		CFRelease(templates);
	}
	
	return noErr;
}

/*
 * NewDocumentPlugInPostMenuCleanup
 *
 * This function is called by the Context Menu Manager when our attached
 * context menu closes. We have a chance to do some cleanup — but it isn't
 * required here.
 */
static void NewDocumentPlugInPostMenuCleanup(void *thisInstance)
{
	// No need to clean up.  We are a tidy folk.
	
	// (We could eventually release the globally cached references — the script component
	// or our own bundle, for instance — but all instances of the plugin
	// share the same, so we'll free them when unloading the plugin)
}


// -----------------------------------------------------------------------------
//	Menu-handling functions
// -----------------------------------------------------------------------------

/*
 * AddNewDocumentMenu
 *
 * Add the "new document" menu to command list.
 */
static OSErr AddNewDocumentMenu(AEDescList* ioCommandList)
{
	OSErr err;
	CFArrayRef templates;
	int i, count;
	CFStringRef POSIXpath;
	CFMutableStringRef cleanName;
	
	templates = CopyTemplatesFilenames();
	
	if (templates != NULL && (count = CFArrayGetCount(templates)) > 0) {
		
		// Create submenu
		AEDescList submenu;
		err = CreateSubmenu(&submenu);
		
		if (err == noErr) {
			
			// enumerate templates
			for(i = 0; i < count; i++) {
				POSIXpath = CFArrayGetValueAtIndex(templates, i);
				cleanName = CopyLocalizedTemplateName(POSIXpath, true);
				RemoveLastExtension(cleanName);
				
				// Add an entry in the menu
				AddMenuItemToAEDescList(cleanName, kTextEncodingMacRoman, typeChar, i, 0, 0, &submenu);
				
				// clean-up
				CFRelease(cleanName);
			}
		}
		
		// Close submenu
		CFStringRef submenuTitle = CFCopyLocalizedStringFromTableInBundle(CFSTR("submenuTitle"), NULL, GetSelfBundle(), CFSTR("Title of the submenu"));
		StickSubmenuInParent(&submenu, ioCommandList, submenuTitle, kTextEncodingMacRoman);
		CFRelease(submenuTitle);
		
	} else {
		err = -1;
	}
	
	if (templates != NULL)
		CFRelease(templates);
	
	return err;
}

/*
 * AddMenuItemToAEDescList
 *
 * Add a new menu item to an existing list of descriptors.
 * Adapted from the original SampleCMPlugin code.
 */
static OSStatus AddMenuItemToAEDescList(
									   CFStringRef			inCommandCFString,
									   TextEncoding			inEncoding,
									   DescType				inDescType,
									   SInt32				inCommandID,
									   MenuItemAttributes	inAttributes,
									   UInt32				inModifiers,
									   AEDescList*			ioCommandList)
{
	OSStatus theError = noErr;
	
	AERecord theCommandRecord = { typeNull, NULL };
	
	// create an apple event record for our command
	theError = AECreateList( NULL, kAEDescListFactorNone, true, &theCommandRecord );
	require_noerr( theError, AddCommandToAEDescList_fail );
	
	// create a reference Cstring
	Boolean doFreeCommandString = false;
	char* inCommandString;
	inCommandString = (char *) CFStringGetCStringPtr(inCommandCFString, inEncoding);
	if (inCommandString == NULL) {
		doFreeCommandString = true;
		int sizeOfBuffer = CFStringGetLength(inCommandCFString) + 1;
		inCommandString = (char*) malloc(sizeOfBuffer);
		CFStringGetCString(inCommandCFString, inCommandString, sizeOfBuffer, inEncoding);
	}
	
	// stick the command text into the AERecord
	if ( inCommandString != NULL ) {
		switch(inDescType) {
			case typeChar:
				{
				theError = AEPutKeyPtr(&theCommandRecord, keyAEName, typeChar,
									   inCommandString, strlen(inCommandString));
				require_noerr(theError, AddCommandToAEDescList_fail);
				}
				break;
		
			case typeStyledText:
				{
				AERecord	textRecord;
				WritingCode	writingCode;
				AEDesc		textDesc;
			
				theError = AECreateList( NULL, kAEDescListFactorNone, true, &textRecord );
				require_noerr( theError, AddCommandToAEDescList_fail );
				
				theError = AEPutKeyPtr(&textRecord, keyAEText, typeChar,
									   inCommandString, strlen(inCommandString));
				require_noerr(theError, AddCommandToAEDescList_fail);
			
				RevertTextEncodingToScriptInfo(inEncoding, &writingCode.theScriptCode,
											   &writingCode.theLangCode, NULL);
				theError = AEPutKeyPtr(&textRecord, keyAEScriptTag, typeIntlWritingCode,
									   &writingCode, sizeof(writingCode));
				require_noerr(theError, AddCommandToAEDescList_fail);
			
				theError = AECoerceDesc( &textRecord, typeStyledText, &textDesc );
				require_noerr( theError, AddCommandToAEDescList_fail );
			
				theError = AEPutKeyDesc( &theCommandRecord, keyAEName, &textDesc );
				require_noerr( theError, AddCommandToAEDescList_fail );
			
				AEDisposeDesc( &textRecord );
				}
				break;
				
			case typeIntlText:
				{
				IntlText*	intlText;
				ByteCount	size = sizeof( IntlText ) + StrLength( inCommandString ) - 1;
			
				// create an IntlText structure with the text and script
				intlText = (IntlText*) malloc(size);
				RevertTextEncodingToScriptInfo(inEncoding, &intlText->theScriptCode,
											   &intlText->theLangCode, NULL );
				BlockMoveData(inCommandString, &intlText->theText, strlen(inCommandString));
				
				theError = AEPutKeyPtr( &theCommandRecord, keyAEName, typeIntlText, intlText, size );
				free( (char*) intlText );
				require_noerr( theError, AddCommandToAEDescList_fail );
				}
				break;
			
			case typeUnicodeText:
				{
				Boolean doFree = false;
				CFIndex sizeInChars = CFStringGetLength(inCommandCFString);
				CFIndex sizeInBytes = sizeInChars * sizeof( UniChar );
				const UniChar* unicode = CFStringGetCharactersPtr(inCommandCFString);
				if (unicode == NULL) {
					doFree = true;
					unicode = (UniChar*) malloc( sizeInBytes );
					CFStringGetCharacters(inCommandCFString, CFRangeMake( 0, sizeInChars ), (UniChar*) unicode );
				}
			
				theError = AEPutKeyPtr( &theCommandRecord, keyAEName, typeUnicodeText, unicode, sizeInBytes );
				
				if (doFree)
					free((char*) unicode);
				
				require_noerr( theError, AddCommandToAEDescList_fail );
				}
				break;
			
			case typeCFStringRef:
				{
				CFStringRef str = CFStringCreateCopy(NULL, inCommandCFString);
				if (str != NULL) {
					theError = AEPutKeyPtr( &theCommandRecord, keyAEName, typeCFStringRef, &str, sizeof( str ) );
					require_noerr( theError, AddCommandToAEDescList_fail );
				}	
				// do not release the string; the Contextual Menu Manager will release it for us
				}
				break;
				
			default:
				printf("NewDocumentPlugIn->AddMenuItemToAEDesc: Unknow descType (%s)\n", CFSwapInt32BigToHost(inDescType));
				break;
		}
	}
	
	// stick the command ID into the AERecord
	if ( inCommandID != 0 ) {
		theError = AEPutKeyPtr( &theCommandRecord, keyContextualMenuCommandID,
							   typeLongInteger, &inCommandID, sizeof( inCommandID ) );
		require_noerr( theError, AddCommandToAEDescList_fail );
	}
	
	// stick the attributes into the AERecord
	if ( inAttributes != 0 ) {
		theError = AEPutKeyPtr( &theCommandRecord, keyContextualMenuAttributes,
							   typeLongInteger, &inAttributes, sizeof( inAttributes ) );
		require_noerr( theError, AddCommandToAEDescList_fail );
	}
	
	// stick the modifiers into the AERecord
	if ( inModifiers != 0 ) {
		theError = AEPutKeyPtr( &theCommandRecord, keyContextualMenuModifiers,
							   typeLongInteger, &inModifiers, sizeof( inModifiers ) );
		require_noerr( theError, AddCommandToAEDescList_fail );
	}
	
	// stick this record into the list of commands that we are
	// passing back to the CMM
	theError = AEPutDesc(ioCommandList, 0, &theCommandRecord );
	
AddCommandToAEDescList_fail:
	// clean up after ourself; dispose of the AERecord
	AEDisposeDesc( &theCommandRecord );
	if (doFreeCommandString)
		free(inCommandString);
	
    return theError;
    
}

/*
 * CreateSubmenu
 *
 * Init a new submenu into an empty commandsList.
 */
static OSErr CreateSubmenu(AEDescList* outSubmenuCommands)
{
	OSErr		theError = noErr;
	//*outSubmenuCommands = NULL;
	
	// set up an AEDescList of subcommands
	theError = AECreateList(NULL, 0, false, outSubmenuCommands);
	
	return theError;
}

/*
 * StickSubmenuInParent
 *
 * Add a submenu previously created with CreateSubmenu and populated using
 * AddMenuItemToAEDescList to a parent menu.
 * The function dispose the submenuCommands created by the CreateSubmenu function.
 */
static OSErr StickSubmenuInParent(AEDescList* submenuCommands,
								  AEDescList* parentMenuCommands,
								  CFStringRef submenuTitle,
								  CFStringEncoding submenuTitleEncoding)
{
	OSErr	theError = noErr;
	AERecord	theSupercommand = { typeNull, NULL };
	
	// check that the submenuCommands are valid
	if (submenuCommands == NULL)
		theError = -1;
	require_noerr( theError, CreateSampleSubmenu_Complete_fail );
	
	// convert the submenuTitle into a C string - we'll need it later.
	Str255 theSupercommandText;
	CFStringGetCString(submenuTitle, (char*) theSupercommandText, sizeof(Str255), submenuTitleEncoding);
	
	// now, we need to create the supercommand which will "own" the
	// subcommands.  The supercommand lives in the root command list.
	// this looks very much like the AddCommandToAEDescList function,
	// except that instead of putting a command ID in the record,
	// we put in the subcommand list.
	
	// create an apple event record for our supercommand
	theError = AECreateList( NULL, 0, true, &theSupercommand );
	require_noerr( theError, CreateSampleSubmenu_fail );
	
	// stick the command text into the aerecord
	theError = AEPutKeyPtr(&theSupercommand, keyAEName, typeChar, theSupercommandText, strlen((char*)theSupercommandText));
	require_noerr( theError, CreateSampleSubmenu_fail );
	
	// stick the subcommands into into the AERecord
	theError = AEPutKeyDesc(&theSupercommand, keyContextualMenuSubmenu,
							submenuCommands);
	require_noerr( theError, CreateSampleSubmenu_fail );
	
	// stick the supercommand into the list of commands that we are
	// passing back to the CMM
	theError = AEPutDesc(parentMenuCommands, 0, &theSupercommand);
	
	// clean up after ourself
CreateSampleSubmenu_fail:
	AEDisposeDesc(submenuCommands);
	
CreateSampleSubmenu_CreateDesc_fail:
	AEDisposeDesc(&theSupercommand);
	
CreateSampleSubmenu_Complete_fail:
    return theError;
}


// -----------------------------------------------------------------------------
//	File System and templates manipulations
// -----------------------------------------------------------------------------

/*
 * GetPugInBundleRef
 *
 * Retrieve a reference to the bundle contained in a plugin object, or NULL
 * if an error occurs.
 */
static CFBundleRef GetPlugInBundleRef(CFStringRef bundleIdentifier)
{
	CFBundleRef tmpBundle;
	CFBundleRef result = NULL;
	CFPlugInRef thePlugIn;
	
	// Get the bundle using the global identification string
	tmpBundle = (CFBundleRef)CFRetain(CFBundleGetBundleWithIdentifier(bundleIdentifier));
	
	// We are not supposed to get the plugin bundle from an identifier.
	// So we get the plugin of the retrieved bundle, and then the proper bundle
	// of the plugin.
	if (tmpBundle != NULL) {
		thePlugIn = (CFPlugInRef)CFRetain(CFBundleGetPlugIn(tmpBundle));
		if (thePlugIn != NULL) {
			result = (CFBundleRef)CFPlugInGetBundle(thePlugIn);
			CFRelease(thePlugIn);
		}
		CFRelease(tmpBundle);
	}
	
	return result;
}

/*
 * GetSelfBundle
 *
 * Retrieve a reference to the plugin own bundle object (from the
 * kNewDocumentPlugInBundle constant), or NULL if an error occurs.
 */
static CFBundleRef GetSelfBundle()
{
	if (gSelfBundle == NULL)
		gSelfBundle = GetPlugInBundleRef(CFSTR(kNewDocumentPlugInBundle));
	
	return gSelfBundle;
}

/*
 * FSFileExists
 *
 * Indicates wether a POSIX-style path points to an existing file
 * or not. "isDir" indicates wether the path is expected to point to
 * a directory or not.
 */
static bool FSFileExists(CFStringRef filename, bool isDir)
{
	bool result;
	CFURLRef tmpURL;
	FSRef tmpFSRef;
	
	tmpURL = CFURLCreateWithFileSystemPath(NULL,
										   filename,
										   kCFURLPOSIXPathStyle,
										   false);
	result = CFURLGetFSRef(tmpURL, &tmpFSRef);
	
	CFRelease(tmpURL);
	
	return result;
}

/*
 * FSIsDir
 *
 * Indicates wether an FSRef points to a directory.
 */
static bool FSIsDir(const FSRef *ref)
{
	FSCatalogInfo fileInfo;
	OSErr err;
	
	err = FSGetCatalogInfo(ref, kFSCatInfoNodeFlags, &fileInfo, NULL, NULL, NULL);
	if (err == noErr && (fileInfo.nodeFlags & kFSNodeIsDirectoryMask))
		return true;
	else
		return false;
}

/*
 * CopyFileURLFromAEDescList
 *
 * Extract the first item of argument list in inContext, as a CFURL.
 * If inContext is null, not a list, or cannot be coerced to a file
 * URL, returns NULL.
 */
static CFURLRef CopyFileURLFromAEDescList(const AEDesc* inContext)
{
	CFURLRef result = NULL;
	
	// Make sure the descriptor isn't null
	if (inContext != NULL)
	{
		// Examine list
		if (inContext->descriptorType == typeAEList) {
			
			long count = 0;
			AEDesc value;
			OSErr err;
			
			AECountItems(inContext, &count);
			
			
			if (count == 1) {
				
				// Get first (and unique) list item, and coerce it to an FSRef
				err = AEGetNthDesc(inContext, 1, typeFileURL, NULL, &value);
				
				if (err == noErr) {
					
					// Extract data
					Size dataSize = AEGetDescDataSize(&value);
					UInt8 *buffer = malloc(dataSize);
					AEGetDescData(&value, buffer, dataSize);
					
					// Create a file URL with the data
					result = CFURLCreateWithBytes(NULL, buffer, dataSize, kCFStringEncodingUTF8, NULL);
					
					// Clean-up
					free(buffer);
					AEDisposeDesc(&value);
				}
			}
			 
		}
	}
	
	return result;
}

/*
 * IncrementDocumentName
 *
 * Increment a document name until the document doesn't already exists at the
 * given location. The increment is inserted just before the templates' extensions
 * (i.e. before the first dot in the document name).
 */
static void IncrementDocumentName(CFStringRef documentPath, CFMutableStringRef documentName)
{
	int i, maxCount = 1000;
	bool fileExists;
	CFStringRef filename, extensions, loopFilename, docWithoutExtensions, filenameWithoutExtensions;
	CFRange firstDot;
	
	// Format a complete filename from path and name
	filename = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@"), documentPath, documentName);
	
	// If initial file does not exists, no need to go further
	if (!FSFileExists(filename, false)) {
		CFRelease(filename);
		return;
	}
	
	// Extract extensions
	firstDot = CFStringFind(documentName, CFSTR("."), 0);
	if (firstDot.location != kCFNotFound) {
		// separate extensions from the file name
		extensions = CFStringCreateWithSubstring(NULL,
												 documentName,
												 CFRangeMake(firstDot.location,
															 CFStringGetLength(documentName) - firstDot.location));
		docWithoutExtensions = CFStringCreateWithSubstring(NULL,
														   documentName,
														   CFRangeMake(0, firstDot.location));
		filenameWithoutExtensions = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@"), documentPath, docWithoutExtensions);
	}
	else {
		// file name doesn't have any extensions
		extensions = CFSTR("");
		filenameWithoutExtensions = documentName;
	}
	
	// Try to find a suitable filename by appending a number between the document
	// name and the extensions
	for (i = 2; i < maxCount; i++) {
		loopFilename = CFStringCreateWithFormat(NULL,
												NULL,
												CFSTR("%@ %i%@"),
												filenameWithoutExtensions, i, extensions);
		fileExists = FSFileExists(loopFilename, false);
		CFRelease(loopFilename);
		if (!fileExists)
			break;
	}
	
	// if we reached maxcount without finding a suitable filename…
	if (i == maxCount)
		printf("NewDocumentPlugin: Error: cannot find a suitable filename for document.\n");
	else {
		CFStringRef newDocumentName = CFStringCreateWithFormat(NULL,
															   NULL,
															   CFSTR("%@ %i%@"),
															   docWithoutExtensions,
															   i,
															   extensions);
		CFStringReplaceAll(documentName, newDocumentName);
		CFRelease(newDocumentName);
	}
	
	CFRelease(filenameWithoutExtensions);
	CFRelease(docWithoutExtensions);
	CFRelease(extensions);
	CFRelease(filename);
}

/*
 * TemplateCopyCallback
 *
 * Called during the various stages of an asynchronous copy operation.
 * Used here mainly to trigger a script when the copy is finished, and to
 * set the extension of the file as hidden by default.
 */
void TemplateCopyCallback(FSFileOperationRef fileOp,
						  const FSRef *currentItem,
						  FSFileOperationStage stage,
						  OSStatus error,
						  CFDictionaryRef statusDictionary,
						  void *info)
{
	// do stuff only when everything is done
	if (stage == kFSOperationStageComplete && error == noErr) {
		
		CFURLRef url;
		CFStringRef item;
		OSErr err;
		
		// Hide the extension of the item (which will not be shown,
		// except if the Finder is configured to show all extensions anyway)
		LSSetExtensionHiddenForRef(currentItem, true);
		
		// Retrieve item name
		url = CFURLCreateFromFSRef(NULL, currentItem);
		item = CFURLCopyLastPathComponent(url);
		
		// tell the Finder to select the item
		err = EditFinderItem(item);
		if (err != noErr) {
			printf("NewDocumentPlugIn: Error while executing the script (%d).\n", err);
		}
		
		CFRelease(item);
		CFRelease(url);
	}
}

/*
 * CopyTemplatesFilenames
 *
 * Returns a array that contains the POSIX filenames to the templates.
 */
static CFArrayRef CopyTemplatesFilenames()
{
	CFMutableArrayRef templatesURL = CFArrayCreateMutable(NULL, 5, &kCFTypeArrayCallBacks);
	
	// Retrieve our own plugin bundle
	CFBundleRef theBundle = (CFBundleRef)CFRetain( GetPlugInBundleRef(CFSTR(kNewDocumentPlugInBundle)) );
	
	if (theBundle != NULL) {
		
		CFArrayRef docTemplates;
		CFURLRef templateURL;
		CFStringRef POSIXFilename;
		CFIndex i, count;
		
		// Retrieve templates from bundle resources
		docTemplates = CFBundleCopyResourceURLsOfType(theBundle, NULL, CFSTR(kNewDocumentPlugInTemplatesSubdir));
		count = CFArrayGetCount(docTemplates);
		
		// Enumerate docTemplates in plugin resources
		for (i = 0; i < count; i++) {
			templateURL = (CFURLRef)CFArrayGetValueAtIndex(docTemplates, i);
			POSIXFilename = CFURLCopyFileSystemPath(templateURL, kCFURLPOSIXPathStyle);
			
			// Add a new entry to the dictionary
			CFArrayAppendValue(templatesURL, POSIXFilename);
			
			CFRelease(POSIXFilename);
		}
		
		CFRelease(docTemplates);
		CFRelease(theBundle);
	}
	else {
		printf("NewDocumentPlugIn: Error : cannot retrieve plugin bundle.");
		return NULL;
	}
	
	return templatesURL;
}

/*
 * CopyLocalizedTemplateName
 *
 * Return a localized version of the template filename.
 * The returned string is mutable, as it will typically be manipulated again.
 */
static CFMutableStringRef CopyLocalizedTemplateName(CFStringRef templateFilename, bool localizeForMenu)
{
	CFStringRef localized, templateName, templateExtensions, tmpStr;
	CFMutableStringRef filename;
	CFRange dotPosition;
	
	// Get localized format string for the template name
	if (localizeForMenu)
		localized = CFCopyLocalizedStringFromTableInBundle(CFSTR("templateMenuName"),
														   NULL,
														   GetSelfBundle(),
														   CFSTR("Name of templates as displayed in menus"));
	else
		localized = CFCopyLocalizedStringFromTableInBundle(CFSTR("templateDocumentName"),
														   NULL,
														   GetSelfBundle(),
														   CFSTR("Name of new documents created from templates"));

	// Split filename from extensions
	dotPosition = CFStringFind(templateFilename, CFSTR("."), 0);
	if (dotPosition.location != kCFNotFound) {
		templateName = CFStringCreateWithSubstring(NULL, templateFilename, CFRangeMake(0, dotPosition.location));
		templateExtensions = CFStringCreateWithSubstring(NULL,
														 templateFilename,
														 CFRangeMake(dotPosition.location,
																	 CFStringGetLength(templateFilename) - dotPosition.location
														 ));
	}
	else {
		templateName = CFStringCreateCopy(NULL, templateFilename);
		templateExtensions = CFSTR("");
	}
	
	// Check if there is a specific override for this template
	tmpStr = CFCopyLocalizedStringWithDefaultValue(templateName, NULL, GetSelfBundle(), templateName, "");
	CFRelease(templateName);  // Release the original (and immutable) templateName…
	templateName = tmpStr;    // …and replace it by the overriden one. 
	
	// Localize the template name, then append the extensions
	filename = CFStringCreateMutable(NULL, 0);
	CFStringAppendFormat(filename,
						 NULL,
						 localized,
						 templateName);
	CFStringAppend(filename, templateExtensions);
	
	// Clean-up
	CFRelease(localized);
	CFRelease(templateName);
	CFRelease(templateExtensions);
	
	return filename;
}

/*
 * RemoveLastExtension
 *
 * Return a localized version of the template filename.
 */
static void RemoveLastExtension(CFMutableStringRef filename)
{
	CFArrayRef dotResults;
	
	// Remove the last of remaining extensions, if any
	dotResults = CFStringCreateArrayWithFindResults(NULL,
													filename,
													CFSTR("."), 
													CFRangeMake(0, CFStringGetLength(filename)),
													0);
	if (dotResults != NULL && CFArrayGetCount(dotResults) > 0) {
		CFRange* dotRange = (CFRange*)CFArrayGetValueAtIndex(dotResults, CFArrayGetCount(dotResults) - 1);
		CFStringDelete(filename, CFRangeMake(dotRange->location, CFStringGetLength(filename) - dotRange->location));
	}
	
	CFRelease(dotResults);
}


// -----------------------------------------------------------------------------
// Scripting functions
// -----------------------------------------------------------------------------

/*
 * EditFinderItem
 *
 * Run the "EditFinderItem" Applescript to enable the renaming of a specific item
 * in the Finder front window.
 */
static OSAError EditFinderItem(CFStringRef itemName)
{
	OSAError err;
	OSAID scriptID;
	ComponentInstance scriptComponent;
	CFMutableStringRef itemProperty;
	
	// Get the Applescript scripting component
	scriptComponent = GetAppleScriptComponent();
	
	// Load and compile the script from resources
	err = LoadScriptFromResources(CFSTR("EditFinderItem.scpt"), &scriptID);
	
	if (err == noErr) {
		
		// Bind vars
		itemProperty = CFStringCreateMutable(NULL, 0);
		CFStringAppendFormat(itemProperty, NULL, CFSTR("\"%@\""), itemName); // Add double-quotes
		err = InjectPropertyIntoScript(scriptID, CFSTR("theItem"), itemProperty, kCFStringEncodingUTF8);
		CFRelease(itemProperty);
		
		// Execute the script
		if (err == noErr) {
			err = ExecuteScript(scriptID, NULL);					
		}
		
		// Dispose the script and close the connection to the scripting component
		DisposeScript(scriptID);
	}
	
	return err;
}

/*
 * GetApplescriptScriptingComponent
 *
 * Retrieve an application-wide AppleScript scripting component
 */
static ComponentInstance GetAppleScriptComponent()
{
	ComponentInstance AppleScriptComponent;
	
	if (gScriptingComponent == NULL) {
		
		// Get the generic scripting component
		gScriptingComponent = OpenDefaultComponent(kOSAComponentType, kOSAGenericScriptingComponentSubtype);
		
		if (gScriptingComponent == NULL)
			printf("NewDocumentPlugIn->getGenericComponent : No such component.");
		else if (gScriptingComponent == (ComponentInstance) badComponentInstance)
			printf("NewDocumentPlugIn->getGenericComponent : Bad component instance.");
		else if (gScriptingComponent == (ComponentInstance) badComponentSelector)
			printf("NewDocumentPlugIn->getGenericComponent : Bad component selector.");	
	}
	
	// We need to target AppleScript specifically for some operations
	// (As OSAGetScriptingComponent always returns the same instance, we don't need to cache it)
	if (gScriptingComponent != NULL)
		OSAGetScriptingComponent(gScriptingComponent, kAppleScriptSubtype, &AppleScriptComponent);
	
	return AppleScriptComponent;
}

/*
 * LoadScriptFromResources
 *
 * Use OSA services to load (and optionaly compile) a script.
 * When you don't need to use the script anymore, you must free it with
 * DisposeScript().
 */
static OSAError LoadScriptFromResources(CFStringRef scriptName, OSAID* outScriptID)
{
	OSAError err = noErr;
	CFURLRef scriptURL;
	FSRef scriptFile;
	ComponentInstance scriptComponent;
	
	// Retrieve Applescript script component (required by OSASetProperty)
	scriptComponent = GetAppleScriptComponent();
	
	// Get the script URL from the resources
	scriptURL = CFBundleCopyResourceURL(GetPlugInBundleRef(CFSTR(kNewDocumentPlugInBundle)),
										scriptName,
										NULL,
										NULL);
	
	if (scriptURL != NULL) {
		
		// Get a file reference from the URL
		CFURLGetFSRef(scriptURL, &scriptFile);
		
		// Load the script (and compile it if needed)
		err = OSALoadFile(scriptComponent,
						  &scriptFile,
						  NULL,
						  kOSANullMode,
						  outScriptID);
		CFRelease(scriptURL);
		
	}
	else {
		err = fnfErr;
	}
	
	return err;
}

/*
 * CreateAEDescFromCFString
 *
 * Create a new AEDesc of type typeChar, containing the given string.
 * When you don't need the created AEDesc anymore, you must release it with
 * AEDisposeDesc().
 */
static OSErr CreateAEDescFromString(CFStringRef string, CFStringEncoding encoding, AEDesc* outDesc)
{
	OSErr err;
	char* cString;
	bool doFree;
	
	// Convert argument to cstring
	if ((cString = (char*)CFStringGetCStringPtr(string, encoding)) == NULL) {
		doFree = true;
		int sizeOfBuffer = CFStringGetLength(string) + 1;
		cString = (char*) malloc(sizeOfBuffer);
		CFStringGetCString(string, (char*)cString, sizeOfBuffer, encoding);
	}
	
	err = AECreateDesc(typeChar, cString, strlen(cString), outDesc);
	
	if (doFree)
		free(cString);
	
	return err;
}

/*
 * InjectPropertyIntoScript
 *
 * Set a given property within a compiled script.
 * PropertyValue are evaluated before being set, so you can pass data of any type. For
 * instance, passing "3" will set the property to the number 3, and passing "\"3\""
 * will set a string litteral.
 */
static OSAError InjectPropertyIntoScript(OSAID scriptID,
									  CFStringRef propertyName,
									  CFStringRef propertyValue,
									  CFStringEncoding encoding)
{
	OSAError err;
	AEDesc propName, propValue;
	OSAID propValueID;
	ComponentInstance scriptComponent;
	
	// retrieve script component
	scriptComponent = GetAppleScriptComponent();
	
	// create the AEDesc matching the property name
	CreateAEDescFromString(propertyName, encoding, &propName);
	
	// create a script result containing the property value
	CreateAEDescFromString(propertyValue, encoding, &propValue);
	OSACompileExecute(scriptComponent, &propValue, kOSANullScript, kOSANullMode, &propValueID);
	
	// set the property into the scriptID context
	err = OSASetProperty(scriptComponent, kOSAModeNull, scriptID, &propName, propValueID);
	
	OSADispose(scriptComponent, propValueID);
	AEDisposeDesc(&propName);
	AEDisposeDesc(&propValue);

	return err;
}

/*
 * ExecuteScript
 *
 * Execute a compiled script. When compiled with DEBUG, prints a description
 * of any script error.
 * outResultingScriptId can be NULL - but if it is not, you must release it with
 * OSADispose() when you no longer need it.
 */
static OSAError ExecuteScript(OSAID scriptID, OSAID* outResultingScriptID)
{
	OSAError err;
	OSAID resultID;
	OSAID* resultIDPtr;
	ComponentInstance scriptComponent;
	
	// get the applescript scripting component
	scriptComponent = GetAppleScriptComponent();
	
	// return the resulting script ID only if requested
	if (outResultingScriptID != NULL)
		resultIDPtr = outResultingScriptID;
	else
		resultIDPtr = &resultID;
	
	// Execute the script
	err = OSAExecute(scriptComponent,
					 scriptID,
					 kOSANullScript,
					 kOSANullMode,
					 resultIDPtr);
#ifdef DEBUG
	if (err != noErr) {
		if (err == errOSAScriptError) {
			
			AEDesc scriptError;
			CFStringRef errorMsg;
			
			OSAScriptError(scriptComponent,
						   kOSAErrorMessage,
						   typeChar,
						   &scriptError);
			errorMsg = CopyStringFromDesc(&scriptError);
			
			CFShow(errorMsg);
			
			CFRelease(errorMsg);
		} else {
			printf("NewDocumentPlugIn->ExecuteScript : OSAExecute error (%d)\n", err);
		}
	}
#endif
	
	// If we didn't asked the result script ID, free it.
	if (outResultingScriptID == NULL)
		OSADispose(scriptComponent, *resultIDPtr);
				   
	return err;
}

/*
 * DisposeScript
 *
 * Release a scriptID created by LoadScriptFromResources().
 */
static OSAError DisposeScript(OSAID scriptID)
{
	OSAError err;
	ComponentInstance scriptComponent;
	
	// get the script component
	scriptComponent = GetAppleScriptComponent();
	
	// Release the scriptId
	err = OSADispose(scriptComponent, scriptID);
	
	return err;
}


//------------------------------------------------------------
// Debug functions
//------------------------------------------------------------

#ifdef DEBUG
/* CopyDescToString
 * Debug function that convert a typeChar (or coercible to text) AEDesc in a
 * CFString.
 */
static CFStringRef CopyDescToString(AEDesc* desc)
{
	UTF8Char* cString = NULL;
	AEDesc* toCoerce = desc;
	AEDesc coercer;
	CFStringRef result;
	Size size;
	
	if (desc->descriptorType != typeChar) {
		toCoerce = &coercer;
		
		if (AECoerceDesc(desc, typeChar, toCoerce) != noErr)
			toCoerce = NULL;
	}
	
	if (toCoerce != NULL) {
		size = AEGetDescDataSize(toCoerce);
		cString = (UTF8Char*) malloc(size + 1);
		if (cString != NULL) {
			cString[size] = 0;
			if (AEGetDescData(toCoerce, cString, size) != noErr) {
				free(cString);
				cString = NULL;
			}
		}
	}
	
	if (toCoerce == &coercer)
		AEDisposeDesc(&coercer);
	
	if (cString == NULL)
		return CFSTR("");
	
	result = CFStringCreateWithBytes(NULL, cString, strlen((char*)cString), kCFStringEncodingASCII, false);
	free(cString);
	return result;
}
#endif
