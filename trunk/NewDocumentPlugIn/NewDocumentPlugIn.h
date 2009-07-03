/*
 *  NewDocumentPlugIn.h
 *  NewDocumentPlugIn
 *
 *  Copyright 2009 Kemenaran.
 */
#if !defined(__NEWDOCUMENTPLUGIN__)
#define __NEWDOCUMENTPLUGIN__


// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------

// The CFBundleIdentifier, as defined in Info.plist
#define kNewDocumentPlugInBundle "com.kemenaran.Finder.NewDocumentPlugIn"

// The Resources subdirectory in which document templates are stored.
// If templates are not in a subdirectory, replace the name by NULL.
#define kNewDocumentPlugInTemplatesSubdir "Templates"

#define kNewDocumentPlugInFactoryID	( CFUUIDGetConstantUUIDWithBytes( NULL,		\
0x67, 0x06, 0x3B, 0xEC, 0xF0, 0x42, 0x4C, 0x5F, 	\
0xA3, 0xD3, 0x35, 0x2A, 0x8D, 0x28, 0x17, 0xEF ) )
// "67063BEC-F042-4C5F-A3D3-352A8D2817EF"

#define scm_require(condition,location)		\
if ( !(condition) )	\
goto location;
#define scm_require_noerr(value,location)	\
scm_require((value)==noErr,location)


// -----------------------------------------------------------------------------
//	typedefs
// -----------------------------------------------------------------------------

// The layout for an instance of NewDocumentPlugInType.
typedef struct NewDocumentPlugInType
{
	ContextualMenuInterfaceStruct	*cmInterface;
	CFUUIDRef						factoryID;
	UInt32							refCount;
} NewDocumentPlugInType;


// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------

// IUnknow forward declaration
static HRESULT	NewDocumentPlugInQueryInterface(void* thisInstance, REFIID iid,	LPVOID*	ppv);
static ULONG	NewDocumentPlugInAddRef(void *thisInstance);
static ULONG	NewDocumentPlugInRelease(void *thisInstance);

//  Generic plugin handling functions
static NewDocumentPlugInType* AllocNewDocumentPlugInType(CFUUIDRef inFactoryID);
static void		DeallocNewDocumentPlugInType(NewDocumentPlugInType *thisInstance);
void*			NewDocumentPlugInFactory(CFAllocatorRef allocator, CFUUIDRef typeID);

//  Context Menu Manager entry points 
static OSStatus NewDocumentPlugInExamineContext(void* thisInstance, const AEDesc* inContext, AEDescList* outCommandPairs);
static OSStatus NewDocumentPlugInHandleSelection(void* thisInstance, AEDesc* inContext, SInt32 inCommandID);
static void NewDocumentPlugInPostMenuCleanup(void *thisInstance);

//	Menu-handling functions
static OSErr		AddNewDocumentMenu(AEDescList* ioCommandList);
static OSStatus	AddMenuItemToAEDescList(CFStringRef		inCommandCFString,
									   TextEncoding		inEncoding,
									   DescType			inDescType,
									   SInt32				inCommandID,
									   MenuItemAttributes	inAttributes,
									   UInt32				inModifiers,
									   AEDescList*			ioCommandList);
static OSErr CreateSubmenu(AEDescList* outSubmenuCommands);
static OSErr StickSubmenuInParent(AEDescList* submenuCommands,
								  AEDescList* parentMenuCommands,
								  CFStringRef submenuTitle,
								  CFStringEncoding submenuTitleEncoding);

//	File System and templates manipulations
static CFBundleRef	GetPlugInBundleRef(CFStringRef bundleIdentifier);
static CFBundleRef GetSelfBundle();
static bool			FSIsDir(const FSRef *ref);
static CFURLRef		CopyFileURLFromAEDescList(const AEDesc* inContext);
static void IncrementDocumentName(CFStringRef documentPath, CFMutableStringRef documentName);
void TemplateCopyCallback(FSFileOperationRef fileOp,
						  const FSRef *currentItem,
						  FSFileOperationStage stage,
						  OSStatus error,
						  CFDictionaryRef statusDictionary,
						  void *info);
static CFArrayRef	CopyTemplatesFilenames();
static CFMutableStringRef CopyLocalizedTemplateName(CFStringRef templateFilename, bool localizeForMenu);
static void RemoveLastExtension(CFMutableStringRef filename);

// Scripting functions
static OSAError EditFinderItem(CFStringRef itemName);
static ComponentInstance GetAppleScriptComponent();
static OSAError LoadScriptFromResources(CFStringRef scriptName, OSAID* outScriptID);
static OSErr	CreateAEDescFromString(CFStringRef string, CFStringEncoding encoding, AEDesc* outDesc);
static OSAError InjectPropertyIntoScript(OSAID scriptID,
										 CFStringRef propertyName,
										 CFStringRef propertyValue,
										 CFStringEncoding encoding);
static OSAError ExecuteScript(OSAID scriptID, OSAID* outResultingScriptID);
static OSAError DisposeScript(OSAID scriptID);

// Debug functions
#ifdef DEBUG
static CFStringRef CopyStringFromDesc(AEDesc* desc);
#endif


// -----------------------------------------------------------------------------
//	Interfaces definitions
// -----------------------------------------------------------------------------

// The newDocumentInterface function table.
static ContextualMenuInterfaceStruct newDocumentInterfaceFtbl =
{ 
	// Required padding for COM
	NULL,
	
	// These three are the required COM functions
	NewDocumentPlugInQueryInterface,
	NewDocumentPlugInAddRef, 
	NewDocumentPlugInRelease, 
	
	// ContextualMenu interface implementation
	NewDocumentPlugInExamineContext,
	NewDocumentPlugInHandleSelection,
	NewDocumentPlugInPostMenuCleanup
};

#endif