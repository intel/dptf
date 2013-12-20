/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#ifndef _ESIF_UF_APP_IFACE_
#define _ESIF_UF_APP_IFACE_

#include "esif_uf_iface.h"

#define APP_INTERFACE_VERSION 1
#define APP_PARTICIPANT_VERSION 1
#define APP_DOMAIN_VERSION 1

/*
    INTERFACE Structures
    These structures are only used by the INTERFACE ESIF maintains a much
    more complex state for a participant and domain than it shares.
 */

/* Application METADATA */
typedef struct _t_AppData {
	/* Version 1 */
	EsifData  fPathHome;		/* Path to home e.g. c:\program files\intel\DPTF... */
} AppData, *AppDataPtr, **AppDataPtrLocation;

/* Particiapnt METADATA */
typedef struct _t_AppParticipantData {
	/* Common */
	UInt8     fVersion;				/* ESIF Participant version */
	UInt8     fReserved[3];			/* Pad / Align */
	EsifData  fDriverType;			/* Guid is obtained from Driver */
	EsifData  fDeviceType;			/* Guid is obtained from DSP */
	EsifData  fName;				/* Name May Come From Driver/DSP */
	EsifData  fDesc;				/* Descriptoin From Driver/DSP */
	EsifData  fDriverName;			/* Driver Name */
	EsifData  fDeviceName;			/* Device Name */
	EsifData  fDevicePath;			/* Devie Path */
	UInt8     fDomainCount;			/* Domain Count */
	UInt8     fReserved2[3];		/* Pad/Align */
	eParticipantBus  fBusEnumerator;	/* Enumeration Type pci, acpi, platform, conjure etc.*/

	/* ACPI */
	EsifData     fAcpiDevice;		/* ACPI Device */
	EsifData     fAcpiScope;		/* ACPI Scope/Object ID \_SB_.IETM */
	UInt32       fAcpiUID;			/* ACPI Unique ID */
	eDomainType  fAcpiType;			/* ACPI Domain/Participant Type e.g. THermalSensor, Power, Etc. */

	/* PCI */
	UInt16  fPciVendor;			/* Vendor */
	UInt16  fPciDevice;			/* Device */
	UInt8   fPciBus;			/* Bus */
	UInt8   fPciBusDevice;		/* Bus Device */
	UInt8   fPciFunction;		/* Function */
	UInt8   fPciRevision;		/* Revision */
	UInt8   fPciClass;			/* Class */
	UInt8   fPciSubClass;		/* Sub Class */
	UInt8   fPciProgIf;			/* Programming Interface */
	UInt8   fReserved3[1];		/* Pad/Align */
} AppParticipantData, *AppParticipantDataPtr, **AppParticipantDataPtrLocation;

/* Domain METADATA */
typedef struct _t_AppDomainData {
	UInt8        fVersion;				/* ESIF Domain version */
	UInt8        fReserved[3];			/* Pad/Align */
	EsifData     fName;					/* Name From DSP */
	EsifData     fDescription;			/* Description From DSP */
	EsifData     fGuid;					/* GUID From DSP */
	eDomainType  fType;					/* Domain Type From DSP */
	EsifFlags    fCapability;			/* Domain Capability From DSP */
	UInt8 fCapabilityBytes[32];	/* Domain Capability Bytes */
} AppDomainData, *AppDomainDataPtr, **AppDomainDataPtrLocation;

/*
    INTERFACE Enumerations
    These Enumerations are only used by the INTERFACE.
 */

/* Applicaiton State */
typedef enum _t_AppState {
	eAppStateDisabled = 0,	/* Disable Application */
	eAppStateEnabled		/* Enable Application */
} eAppState;

/*
    ESIF_APP_INTERFACE Callback Functions
    These functions are only used by the INTERFACE as method to loosely
    couple ESIF with its hosted application.  There are used for ESIF to
    request information and services from the applicaion.  There is a
    corresponding set of interfaces defined below for allow the application
    to provide key services that ESIF provides as well.
 */

/*

    These maybe called to gather Applicaion information before the Allocate
    Handle and subsequent create functions are called.
 */

/*
** ============================================================================
** APPLICATION Level CallBacks
** ============================================================================
*/

typedef eEsifError (*AppGetAboutFunction)       (EsifDataPtr appAbout);
typedef eEsifError (*AppGetDescriptionFunction) (EsifDataPtr appDescription);
typedef eEsifError (*AppGetGuidFunction)        (EsifDataPtr appGuid);
typedef eEsifError (*AppGetNameFunction)        (EsifDataPtr appName);
typedef eEsifError (*AppGetVersionFunction)     (EsifDataPtr appVersion);

/* Must be called and completed with ESIF_OK before create will be called */
typedef eEsifError (*AppAllocateHandle)(void * *appHandleLocation);

/* DEPENDENCY AppAllocateHandle must have previously returned a VALID handle */
typedef eEsifError (*AppCreateFunction)(EsifInterfacePtr appServiceInterface,	/* The App MUST fill in all pointers */
										const void *esifHandle,				/* ESIF will provide App MUST save for use with callbacks */
										const void *appHandle,				/* Allocated handle for application */
										const AppDataPtr appData,				/* Application Metadata */
										const eAppState initialAppState	/* Initial state of application */
										);

/*
    DEPENDENCY
    The remaining funcitons are dependent on application create.
 */

/* Destroy */
typedef eEsifError (*AppDestroyFunction)(void *appHandle);

/* Banner */
typedef eEsifError (*AppGetBannerFunction)(const void *appHandle,	/* Allocated handle for application */
										   EsifDataPtr appBanner/* Applicaiton banner / greeting to display for CLI */
										   );

/* Prompt */
typedef eEsifError (*AppGetPromptFunction)(const void *appHandle,	/* Allocated handle for application */
										   EsifDataPtr appPrompt/* Application prompt for CLI e.g. small state conveyance */
										   );

/* CLI Command */
typedef eEsifError (*AppCommandFunction)(const void *appHandle,	/* Allocated handle for application */
										 const EsifDataPtr request,				/* CLI command requested e.g. help, participants etc. */
										 EsifDataPtr response,			/* CLI response must be ESIF_DATA_STRING today */
										 esif_string appParseContext/* TODO: Fix this very ugly work around */
										 );

/* Set State */
typedef eEsifError (*AppSetStateFunction)(const void *appHandle,/* Allocated handle for application */
										  const eAppState appState			/* Desired application state */
										  );

/* Get State */
typedef eAppState (*AppGetStateFunction)(const void *appHandle);

/* Get Status */
typedef enum _t_eAppStatusCommand {
	eAppStatusCommandGetXSLT = 0,		/* Get the XSLT information */
	eAppStatusCommandGetGroups,			/* Get the group id/name pairs */
	eAppStatusCommandGetModulesInGroup,	/* Get the module id/name pairs for a given group id */
	eAppStatusCommandGetModuleData,		/* Get the data for a given group id and module id */
} eAppStatusCommand;

typedef eEsifError (*AppGetStatusFunction)(const void *appHandle,	/* Allocated handle for application */
										   const eAppStatusCommand command,		/* Command */
										   const UInt32 appStatusIn,			/* Command Data (High Word Group, Low Word Module) */
										   EsifDataPtr appStatusOut				/* Status output string if XML please use ESIF_DATA_XML */
										   );

/*
** ============================================================================
** PARTICIPANT Level CallBacks
** ============================================================================
*/

typedef enum _t_eParticipantState {
	eParticipantStateDisabled = 0,	/* Participant Disabled */
	eParticipantStateEnabled		/* Participant Enabled */
} eParticipantState;

/* Must be called and completed with ESIF_OK before participant create will be called */
typedef eEsifError (*AppParticipantAllocateHandle)(const void *appHandle,	/* Allocated handle for application */
												   void * *participantHandleLocation/* Participant handle set by App */
												   );

/* DEPENDENCY AppParticipantAllocateHandle must have previously returned a VALID handle */
typedef eEsifError (*AppParticipantCreateFunction)(const void *appHandle,	/* Allocated handle for application */
												   const void *particpantHandle,		/* An allocated particpant handle */
												   const AppParticipantDataPtr participantData,			/* Participant Metadata */
												   const eParticipantState particiapntInitialState	/* Participant inital State */
												   );

/* Destroy */
typedef eEsifError (*AppParticipantDestroyFunction)(const void *appHandle,	/* Allocated handle for application */
													const void *participantHandle				/* An allocated participant handle */
													);

/* Get State */
typedef eParticipantState (*AppParticipantGetStateFunction)(const void *appHandle,	/* Allocated handle for application */
															const void *participantHandle	/* An allocated participant handle */
															);

/* Set State */
typedef eEsifError (*AppParticipantSetStateFunction)(const void *appHandle,	/* Allocated handle for application */
													 const void *participantHandle,	/* Allocated handle for participant */
													 eParticipantState participantState	/* Desired participant state */
													 );

/*
** ============================================================================
** DOMAIN Level CallBacks
** ============================================================================
*/

typedef enum _t_eDomainState {
	eDomainStateDisabled = 0,	/* Domain Disabld */
	eDomainStateEnabled			/* Domain Enabled */
} eDomainState;

/* Must be called and completed with ESIF_OK before domain create will be called */
typedef eEsifError (*AppDomainAllocateHandle)(const void *appHandle,/* Allocated handle for application */
											  const void *particpantHandle,		/* Allocated participant handle */
											  void * *domainHandleLocation	/* Domain handle set by application */
											  );

/* DEPENDENCY AppDomainAllocateHandle must have previously returned a VALID handle */
typedef eEsifError (*AppDomainCreateFunction)(const void *appHandle,/* Allocated handle for application */
											  const void *participantHandle,/* Allocated participant handle by application */
											  const void *domainHandle,		/* Allocated domain handle by application */
											  const AppDomainDataPtr domainDataPtr,		/* Domain Metadata and configuration details */
											  const eDomainState domainInitialState	/* Domain initial state */
											  );

/* Destroy */
typedef eEsifError (*AppDomainDestroyFunction)(const void *appHandle,	/* Allocated handle for application */
											   const void *participantHandle,	/* An allocated handle for participant */
											   const void *domainHandle			/* An allocated handle for domain */
											   );

/* Get State */
typedef eDomainState (*AppDomainGetStateFunction)(const void *appHandle,/* Allocated handle for application */
												  const void *participantHandle,/* An allocated handle for participant */
												  const void *domainHandle			/* An allocated handle for domain */
												  );

/* Set State */
typedef eEsifError (*AppDomainSetStateFunction)(const void *appHandle,	/* Allocated handle for application */
												const void *participantHandle,	/* An allocated handle for participant */
												const void *domainHandle,		/* An allocated handle for domain */
												const eDomainState domainState	/* Desired domain state */
												);

/*
** ============================================================================
** EVENT CallBacks
** ============================================================================
*/

typedef eEsifError (*AppEventFunction)(const void *appHandle,	/* Allocated handle for application */
									   const void *participantHandle,	/* Optional Participant ESIF_NO_HANDLE indicates App Level Event */
									   const void *domainHandle,		/* Optional Domain ESIF_NO_HANDLE indicates non-domain Level Event */
									   const EsifDataPtr eventData,			/* Data included with the event if any MAY Be NULL */
									   const EsifDataPtr eventGuid	/* Event GUID */
									   );

/*
    ESIF Application Interface ESIF --> APPLICATION
    Forward declared and typedef in esif_uf_iface.h
 */

struct _t_AppInterface {
	/* Interface */
	eIfaceType  fIfaceType;
	UInt16      fIfaceVersion;
	UInt16      fIfaceSize;

	/* Version 1 */
	/* Application */
	AppAllocateHandle      fAppAllocateHandleFuncPtr;
	AppCreateFunction      fAppCreateFuncPtr;
	AppDestroyFunction     fAppDestroyFuncPtr;

	AppCommandFunction     fAppCommandFuncPtr;
	AppGetAboutFunction    fAppGetAboutFuncPtr;
	AppGetBannerFunction   fAppGetBannerFuncPtr;
	AppGetDescriptionFunction fAppGetDescriptionFuncPtr;
	AppGetGuidFunction     fAppGetGuidFuncPtr;
	AppGetNameFunction     fAppGetNameFuncPtr;
	AppGetPromptFunction   fAppGetPromptFuncPtr;
	AppGetStatusFunction   fAppGetStatusFuncPtr;
	AppGetVersionFunction  fAppGetVersionFuncPtr;
	AppSetStateFunction    fAppSetStateFuncPtr;

	/* Participant */
	AppParticipantAllocateHandle    fParticipantAllocateHandleFuncPtr;
	AppParticipantCreateFunction    fParticipantCreateFuncPtr;
	AppParticipantDestroyFunction   fParticipantDestroyFuncPtr;
	AppParticipantSetStateFunction  fParticipantSetStateFuncPtr;

	/* Domain */
	AppDomainAllocateHandle    fDomainAllocateHandleFuncPtr;
	AppDomainCreateFunction    fDomainCreateFuncPtr;
	AppDomainDestroyFunction   fDomainDestroyFuncPtr;
	AppDomainSetStateFunction  fDomainSetStateFuncPtr;

	/* Event */
	AppEventFunction  fAppEventFuncPtr;

	/* Version 2 */
};

/*
    C++
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
    Entry function this symbol must be available when the app.so/dll modules is
    loaded.  For Unix/Linux nothing special is required for widnows you must
    export this symbol see the REFERENCE IMPLEMENATION for the optional methods
    for doing this.  Note this is the only function that is linked to. So when
    you declare all of your other functions make the static and well access them
    via Function Pointers.  This will remove the linker cruft trom the static
    funcions.  And provide a simple private type of encapsulation for the module.
 */
eEsifError GetApplicationInterface (AppInterfacePtr theIface);

#ifdef __cplusplus
}
#endif

#define GET_APPLICATION_INTERFACE_FUNCTION "GetApplicationInterface"

#endif	// _ESIF_UF_APP_IFACE_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/